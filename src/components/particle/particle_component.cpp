#include "particle_component.h"

#include "core/game.h"
#include "win32/win.h"
#include "render/render.h"
#include "render/camera.h"

ParticleComponent::ParticleComponent()
{
}

ParticleComponent::~ParticleComponent()
{
}

void ParticleComponent::initialize()
{
    auto device = Game::inst()->render().device();

    // Create the global particle pool. Each particle is split into two parts for better cache coherency. The first half contains the data more 
    // relevant to rendering while the second half is more related to simulation
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeof( GPUParticlePartA ) * g_maxParticles;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof( GPUParticlePartA );

    device->CreateBuffer( &desc, nullptr, &m_pParticleBufferA );

    desc.ByteWidth = sizeof( GPUParticlePartB ) * g_maxParticles;
    desc.StructureByteStride = sizeof( GPUParticlePartB );

    device->CreateBuffer( &desc, nullptr, &m_pParticleBufferB );

    D3D11_SHADER_RESOURCE_VIEW_DESC srv;
    srv.Format = DXGI_FORMAT_UNKNOWN;
    srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srv.Buffer.ElementOffset = 0;
    srv.Buffer.ElementWidth = g_maxParticles;

    device->CreateShaderResourceView( m_pParticleBufferA, &srv, &m_pParticleBufferA_SRV );

    D3D11_UNORDERED_ACCESS_VIEW_DESC uav;
    uav.Format = DXGI_FORMAT_UNKNOWN;
    uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uav.Buffer.FirstElement = 0;
    uav.Buffer.NumElements = g_maxParticles;
    uav.Buffer.Flags = 0;
    device->CreateUnorderedAccessView( m_pParticleBufferA, &uav, &m_pParticleBufferA_UAV );
    device->CreateUnorderedAccessView( m_pParticleBufferB, &uav, &m_pParticleBufferB_UAV );

    // The view space positions of particles are cached during simulation so allocate a buffer for them
    desc.ByteWidth = 16 * g_maxParticles;
    desc.StructureByteStride = 16;
    device->CreateBuffer( &desc, 0, &m_pViewSpaceParticlePositions );
    device->CreateShaderResourceView( m_pViewSpaceParticlePositions, &srv, &m_pViewSpaceParticlePositionsSRV );
    device->CreateUnorderedAccessView( m_pViewSpaceParticlePositions, &uav, &m_pViewSpaceParticlePositionsUAV );

    // The maximum radii of each particle is cached during simulation to avoid recomputing multiple times later. This is only required
    // for streaked particles as they are not round so we cache the max radius of X and Y
    desc.ByteWidth = 4 * g_maxParticles;
    desc.StructureByteStride = 4;
    device->CreateBuffer( &desc, 0, &m_pMaxRadiusBuffer );
    device->CreateShaderResourceView( m_pMaxRadiusBuffer, &srv, &m_pMaxRadiusBufferSRV );
    device->CreateUnorderedAccessView( m_pMaxRadiusBuffer, &uav, &m_pMaxRadiusBufferUAV );

    // The dead particle index list. Created as an append buffer
    desc.ByteWidth = sizeof( UINT ) * g_maxParticles;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof( UINT );

    device->CreateBuffer( &desc, nullptr, &m_pDeadListBuffer );

    uav.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    device->CreateUnorderedAccessView( m_pDeadListBuffer, &uav, &m_pDeadListUAV );

    // Create constant buffers to copy the dead and alive list counters into
    ZeroMemory( &desc, sizeof( desc ) );
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.ByteWidth = 4 * sizeof( UINT );
    device->CreateBuffer( &desc, nullptr, &m_pDeadListConstantBuffer );
    device->CreateBuffer( &desc, nullptr, &m_pActiveListConstantBuffer );

    // Create the particle billboard index buffer required for the rasterization VS-only path
    ZeroMemory( &desc, sizeof( desc ) );
    desc.ByteWidth = g_maxParticles * 6 * sizeof( UINT );
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA data;

    UINT* indices = new UINT[ g_maxParticles * 6 ];
    data.pSysMem = indices;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    UINT base = 0;
    for ( int i = 0; i < g_maxParticles; i++ )
    {
        indices[ 0 ] = base + 0;
        indices[ 1 ] = base + 1;
        indices[ 2 ] = base + 2;

        indices[ 3 ] = base + 2;
        indices[ 4 ] = base + 1;
        indices[ 5 ] = base + 3;

        base += 4;
        indices += 6;
    }

    device->CreateBuffer( &desc, &data, &m_pIndexBuffer );

    delete[] data.pSysMem;

    m_Shader.set_vs_shader_from_file("ParticleRender.hlsl", "VS_StructuredBuffer", nullptr, nullptr);
    m_Shader.set_gs_shader_from_file("ParticleRender.hlsl", "GS", nullptr, nullptr);
    m_Shader.set_ps_shader_from_file("ParticleRender.hlsl", "PS_Billboard", nullptr, nullptr);

    m_QuadShader.set_vs_shader_from_file("ParticleRenderQuad.hlsl", "QuadVS", nullptr, nullptr);
    m_QuadShader.set_ps_shader_from_file("ParticleRenderQuad.hlsl", "QuadPS", nullptr, nullptr);

    m_CSSimulate.set_compute_shader_from_file("ParticleSimulation.hlsl", "CS_Simulate", nullptr, nullptr);
    m_CSInitDeadList.set_compute_shader_from_file("InitDeadList.hlsl", "CS_InitDeadList", nullptr, nullptr);
    m_CSEmit.set_compute_shader_from_file("ParticleEmit.hlsl", "CS_Emit", nullptr, nullptr);
    m_CSResetParticles.set_compute_shader_from_file("ParticleSimulation.hlsl", "CS_Reset", nullptr, nullptr);

    // Create the emitter constant buffer
    ZeroMemory( &desc, sizeof( desc ) );
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.ByteWidth = sizeof( EmitterConstantBuffer );
    device->CreateBuffer( &desc, nullptr, &m_pEmitterConstantBuffer );

    struct IndexBufferElement
    {
        float distance; // distance squared from the particle to the camera
        float index; // global index of the particle
    };

    // Create the index buffer of alive particles that is to be sorted (at least in the rasterization path).
    // For the tiled rendering path this could be just a UINT index buffer as particles are not globally sorted
    desc.ByteWidth = sizeof( IndexBufferElement ) * g_maxParticles;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof( IndexBufferElement );

    device->CreateBuffer( &desc, nullptr, &m_pAliveIndexBuffer );

    srv.Format = DXGI_FORMAT_UNKNOWN;
    srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srv.Buffer.ElementOffset = 0;
    srv.Buffer.ElementWidth = g_maxParticles;
    
    device->CreateShaderResourceView( m_pAliveIndexBuffer, &srv, &m_pAliveIndexBufferSRV );

    uav.Buffer.NumElements = g_maxParticles;
    uav.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
    uav.Format = DXGI_FORMAT_UNKNOWN;
    device->CreateUnorderedAccessView( m_pAliveIndexBuffer, &uav, &m_pAliveIndexBufferUAV );

    // Allocate the tiled rendering buffer as RGBA16F
    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory( &BufferDesc, sizeof(BufferDesc) );
    int numPixels = 0; // w * h
    BufferDesc.ByteWidth = 8 * numPixels;
    BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    device->CreateBuffer( &BufferDesc, nullptr, &m_pRenderingBuffer );

    DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ZeroMemory( &SRVDesc, sizeof( SRVDesc ) );
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Buffer.ElementOffset = 0;
    SRVDesc.Format = format;
    SRVDesc.Buffer.ElementWidth = numPixels;
    device->CreateShaderResourceView( m_pRenderingBuffer, &SRVDesc, &m_pRenderingBufferSRV );

    D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
    ZeroMemory( &UAVDesc, sizeof( UAVDesc ) );
    UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    UAVDesc.Buffer.FirstElement = 0;
    UAVDesc.Format = format;
    UAVDesc.Buffer.NumElements = numPixels;
    device->CreateUnorderedAccessView( m_pRenderingBuffer, &UAVDesc, &m_pRenderingBufferUAV );

    // Create the buffer to store the indirect args for the DrawInstancedIndirect call
    ZeroMemory( &desc, sizeof( desc ) );
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    desc.ByteWidth = 5 * sizeof( UINT );
    desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    device->CreateBuffer( &desc, nullptr, &m_pIndirectDrawArgsBuffer );

    ZeroMemory( &uav, sizeof( uav ) );
    uav.Format = DXGI_FORMAT_R32_UINT;
    uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uav.Buffer.FirstElement = 0;
    uav.Buffer.NumElements = 5;
    uav.Buffer.Flags = 0;
    device->CreateUnorderedAccessView( m_pIndirectDrawArgsBuffer, &uav, &m_pIndirectDrawArgsBufferUAV );

    // Create a blend state for compositing the particles onto the render target
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    device->CreateBlendState( &blendDesc, &m_pCompositeBlendState );

    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory( &cbDesc, sizeof(cbDesc) );
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof( m_GlobalConstantBuffer );
    device->CreateBuffer( &cbDesc, nullptr, &m_pPerFrameConstantBuffer );

    m_SortLib.init();

    fill_random_texture();
}

void ParticleComponent::draw()
{
    auto context = Game::inst()->render().context();
    auto camera = Game::inst()->render().camera();

    // Increment the time IF we aren't paused
    m_GlobalConstantBuffer.m_ElapsedTime += Game::inst()->delta_time();

    // Wrap the timer so the numbers don't go too high
    const float wrapPeriod = 10.0f;
    if ( m_GlobalConstantBuffer.m_ElapsedTime > wrapPeriod )
    {
        m_GlobalConstantBuffer.m_ElapsedTime -= wrapPeriod;
    }

    m_GlobalConstantBuffer.m_FrameIndex++;
    m_GlobalConstantBuffer.m_FrameIndex %= 1000;
    // Clear the backbuffer and depth stencil
    float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 1.0f };

    // context->ClearRenderTargetView( (ID3D11RenderTargetView*)DXUTGetD3D11RenderTargetView(), ClearColor );
    // context->ClearRenderTargetView( (ID3D11RenderTargetView*)g_RenderTargetRTV, ClearColor );
    // context->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0 );

    // Compute some matrices for our per-frame constant buffer
    DirectX::XMMATRIX mView = camera->view();
    DirectX::XMMATRIX mProj = camera->proj();
    DirectX::XMMATRIX mViewProjection = camera->view_proj();

    m_GlobalConstantBuffer.m_ViewProjection = DirectX::XMMatrixTranspose( mViewProjection );
    m_GlobalConstantBuffer.m_View  = DirectX::XMMatrixTranspose( mView );
    m_GlobalConstantBuffer.m_Projection = DirectX::XMMatrixTranspose( mProj );

    DirectX::XMMATRIX viewProjInv = DirectX::XMMatrixInverse( nullptr, mViewProjection );
    m_GlobalConstantBuffer.m_ViewProjInv = DirectX::XMMatrixTranspose( viewProjInv );

    DirectX::XMMATRIX viewInv = DirectX::XMMatrixInverse( nullptr, mView );
    m_GlobalConstantBuffer.m_ViewInv= DirectX::XMMatrixTranspose( viewInv );

    DirectX::XMMATRIX projInv = DirectX::XMMatrixInverse( nullptr, mProj );
    m_GlobalConstantBuffer.m_ProjectionInv = DirectX::XMMatrixTranspose( projInv );

    m_GlobalConstantBuffer.m_SunDirectionVS = DirectX::XMVector4Transform( m_GlobalConstantBuffer.m_SunDirection, mView );

    m_GlobalConstantBuffer.m_EyePosition = { camera->position().x, camera->position().y, camera->position().z, 1 };

    m_GlobalConstantBuffer.m_FrameTime = 0.1f;

    m_GlobalConstantBuffer.m_AlphaThreshold = (float)97 / 100.0f;
    m_GlobalConstantBuffer.m_CollisionThickness = (float)40 * 0.1f;

    m_GlobalConstantBuffer.m_CollisionsEnabled = false;
    m_GlobalConstantBuffer.m_EnableSleepState = false;
    m_GlobalConstantBuffer.m_ShowSleepingParticles = false;

    // Update the per-frame constant buffer
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    context->Map( m_pPerFrameConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
    memcpy( MappedResource.pData, &m_GlobalConstantBuffer, sizeof( m_GlobalConstantBuffer ) );
    context->Unmap( m_pPerFrameConstantBuffer, 0 );

    // Switch off alpha blending
    // float BlendFactor[1] = { 0.0f };
    // context->OMSetBlendState( g_pOpaqueState, BlendFactor, 0xffffffff );

	// Render the scene if the shader cache has finished compiling shaders
    ID3D11ShaderResourceView* srv = 0;
    context->CSSetShaderResources( 0, 1, &srv );

    context->RSSetState( g_pRasterState );

    // Set the per-frame constant buffer on all shader stages
    context->VSSetConstantBuffers( 0, 1, &m_pPerFrameConstantBuffer );
    context->PSSetConstantBuffers( 0, 1, &m_pPerFrameConstantBuffer );
    context->GSSetConstantBuffers( 0, 1, &m_pPerFrameConstantBuffer );
    context->CSSetConstantBuffers( 0, 1, &m_pPerFrameConstantBuffer );

    // Set the viewport to be the full screen area
    D3D11_VIEWPORT vp;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = Game::inst()->win().screen_width();
    vp.Height = Game::inst()->win().screen_height();
    context->RSSetViewports( 1, &vp );

    // Make the render target our intermediate buffer (ie not the back buffer)
    context->OMSetRenderTargets( 1, &g_RenderTargetRTV, g_pDepthStencilView );
    
    // Set the global samplers
    context->PSSetSamplers( 0, 1, &g_pSamWrapLinear );
    context->PSSetSamplers( 1, 1, &g_pSamClampLinear );

    context->CSSetSamplers( 0, 1, &g_pSamWrapLinear );
    context->CSSetSamplers( 1, 1, &g_pSamClampLinear );

    // Switch depth writes off and alpha blending on
    context->OMSetDepthStencilState( g_pDepthTestState, 0 );
    context->OMSetBlendState( g_pAlphaState, BlendFactor, 0xffffffff );
    
    // Set the particle texture atlas
    // context->PSSetShaderResources( 0, 1, &g_pTextureAtlas );
    // context->CSSetShaderResources( 6, 1, &g_pTextureAtlas );
        
    // Fill in array of emitters that we will send to the particle system
    // IParticleSystem::EmitterParams emitters[ 5 ];
    // int numEmitters = 0;
    // PopulateEmitters( numEmitters, emitters, ARRAYSIZE( emitters ), fElapsedTime );
    EmitterParams params;
    params.m_Position = Vector4(0, 0, 0, 1);
    params.m_Velocity = DirectX::XMVectorSet( 0.0f, 5.0f, 0.0f, 0.0f );
    params.m_NumToEmit = 0;
    params.m_ParticleLifeSpan = 150.0f;
    params.m_StartSize = 5.0f;
    params.m_EndSize = 22.0f;
    params.m_PositionVariance = DirectX::XMVectorSet( 2.0f, 0.0f, 2.0f, 1.0f );
    params.m_VelocityVariance = 0.6f;
    params.m_Mass = 0.0003f;
    params.m_TextureIndex = 0;
    params.m_Streaks = false;
    emit(1, &params);

    // Unbind the depth buffer because we don't need it and we are going to be using it as shader input
    context->OMSetRenderTargets( 1, &g_RenderTargetRTV, nullptr );
    
    // Convert our UI options into the particle system flags
    int flags = 0;

    // Render the GPU particles system
    g_pGPUParticleSystem->Render( fFrameTime, flags, g_Technique, g_CoarseCullingMode, emitters, numEmitters, g_pDepthStencilSRV );

    //  Unset the GS in-case we have been using it previously
    context->GSSetShader( nullptr, nullptr, 0 );
}

void ParticleComponent::imgui()
{

}

void ParticleComponent::reload()
{

}

void ParticleComponent::update()
{

}

void ParticleComponent::destroy_resources()
{
    m_SortLib.release();
}

void ParticleComponent::emit( int numEmitters, const EmitterParams* emitters )
{
    auto context = Game::inst()->render().context();

    // Set resources but don't reset any atomic counters
    ID3D11UnorderedAccessView* uavs[] = { m_pParticleBufferA_UAV, m_pParticleBufferB_UAV, m_pDeadListUAV };
    UINT initialCounts[] = { (UINT)-1, (UINT)-1, (UINT)-1 };
    context->CSSetUnorderedAccessViews( 0, ARRAYSIZE( uavs ), uavs, initialCounts );

    ID3D11Buffer* buffers[] = { m_pEmitterConstantBuffer, m_pDeadListConstantBuffer };
    context->CSSetConstantBuffers( 1, ARRAYSIZE( buffers ), buffers );

    ID3D11ShaderResourceView* srvs[] = { m_pRandomTextureSRV };
    context->CSSetShaderResources( 0, ARRAYSIZE( srvs ), srvs );

    m_CSEmit.use();

    // Run CS for each emitter
    for ( int i = 0; i < numEmitters; i++ )
    {
        const EmitterParams& emitter = emitters[ i ];
    
        if ( emitter.m_NumToEmit > 0 )
        {    
            // Update the emitter constant buffer
            D3D11_MAPPED_SUBRESOURCE MappedResource;
            context->Map( m_pEmitterConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
            EmitterConstantBuffer* constants = (EmitterConstantBuffer*)MappedResource.pData;
            constants->m_EmitterPosition = emitter.m_Position;
            constants->m_EmitterVelocity = emitter.m_Velocity;
            constants->m_MaxParticlesThisFrame = emitter.m_NumToEmit;
            constants->m_ParticleLifeSpan = emitter.m_ParticleLifeSpan;
            constants->m_StartSize = emitter.m_StartSize;
            constants->m_EndSize = emitter.m_EndSize;
            constants->m_PositionVariance = emitter.m_PositionVariance;
            constants->m_VelocityVariance = emitter.m_VelocityVariance;
            constants->m_Mass = emitter.m_Mass;
            constants->m_Index = i;
            constants->m_Streaks = emitter.m_Streaks ? 1 : 0;
            constants->m_TextureIndex = emitter.m_TextureIndex;
            context->Unmap( m_pEmitterConstantBuffer, 0 );
        
            // Copy the current number of dead particles into a CB so we know how many new particles are available to be spawned
            context->CopyStructureCount( m_pDeadListConstantBuffer, 0, m_pDeadListUAV );
        
            // Dispatch enough thread groups to spawn the requested particles
            int numThreadGroups = align( emitter.m_NumToEmit, 1024 ) / 1024;
            context->Dispatch( numThreadGroups, 1, 1 );
        }
    }
}

void ParticleComponent::simulate( int flags, ID3D11ShaderResourceView* depthSRV )
{
    auto context = Game::inst()->render().context();

    // Set the UAVs and reset the alive index buffer's counter
    ID3D11UnorderedAccessView* uavs[] = { m_pParticleBufferA_UAV, m_pParticleBufferB_UAV, m_pDeadListUAV, m_pAliveIndexBufferUAV, m_pViewSpaceParticlePositionsUAV, m_pMaxRadiusBufferUAV, m_pIndirectDrawArgsBufferUAV };
    UINT initialCounts[] = { (UINT)-1, (UINT)-1, (UINT)-1, 0, (UINT)-1, (UINT)-1, (UINT)-1 };
    
    context->CSSetUnorderedAccessViews( 0, ARRAYSIZE( uavs ), uavs, initialCounts );
    
    // Bind the depth buffer as a texture for doing collision detection and response
    ID3D11ShaderResourceView* srvs[] = { depthSRV };
    context->CSSetShaderResources( 0, ARRAYSIZE( srvs ), srvs );

    // Dispatch enough thread groups to update all the particles
    m_CSSimulate.use();
    context->Dispatch( align( g_maxParticles, 256 ) / 256, 1, 1 );

    ZeroMemory( srvs, sizeof( srvs ) );
    context->CSSetShaderResources( 0, ARRAYSIZE( srvs ), srvs );

    ZeroMemory( uavs, sizeof( uavs ) );
    context->CSSetUnorderedAccessViews( 0, ARRAYSIZE( uavs ), uavs, nullptr );
}

void ParticleComponent::sort()
{
    m_SortLib.run( g_maxParticles, m_pAliveIndexBufferUAV, m_pActiveListConstantBuffer );
}

void ParticleComponent::render_quad()
{
    auto context = Game::inst()->render().context();

    // Set the blend state to do compositing
    context->OMSetBlendState( m_pCompositeBlendState, nullptr, 0xffffffff );

    // Set the quad shader
    m_QuadShader.use();

    // No vertex buffer or index buffer required. Just use vertexId to generate triangles
    context->IASetIndexBuffer( nullptr, DXGI_FORMAT_UNKNOWN, 0 );
    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Bind the tiled UAV to the pixel shader
    ID3D11ShaderResourceView* srvs[] = { m_pRenderingBufferSRV };
    context->PSSetShaderResources( 0, ARRAYSIZE( srvs ), srvs );

    // Draw one large triangle
    context->Draw( 3, 0 );
    
    ZeroMemory( srvs, sizeof( srvs ) );
    context->PSSetShaderResources( 0, ARRAYSIZE( srvs ), srvs );

    // Restore the default blend state
    context->OMSetBlendState( nullptr, nullptr, 0xffffffff );
}

void ParticleComponent::init_dead_list()
{
    auto context = Game::inst()->render().context();

    m_CSInitDeadList.use();

    UINT initialCount[] = { 0 };
    context->CSSetUnorderedAccessViews( 0, 1, &m_pDeadListUAV, initialCount );

    // Disaptch a set of 1d thread groups to fill out the dead list, one thread per particle
    context->Dispatch( align( g_maxParticles, 256 ) / 256, 1, 1 );
}

void ParticleComponent::fill_random_texture()
{
    auto device = Game::inst()->render().device();

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory( &desc, sizeof( desc ) );
    desc.Width = 1024;
    desc.Height = 1024;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    
    float* values = new float[ desc.Width * desc.Height * 4 ];
    float* ptr = values;
    for ( UINT i = 0; i < desc.Width * desc.Height; i++ )
    {
        ptr[ 0 ] = RandomVariance( 0.0f, 1.0f );
        ptr[ 1 ] = RandomVariance( 0.0f, 1.0f );
        ptr[ 2 ] = RandomVariance( 0.0f, 1.0f );
        ptr[ 3 ] = RandomVariance( 0.0f, 1.0f );
        ptr += 4;
    }

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = values;
    data.SysMemPitch = desc.Width * 16;
    data.SysMemSlicePitch = 0; 

    device->CreateTexture2D( &desc, &data, &m_pRandomTexture );

    delete[] values;

    D3D11_SHADER_RESOURCE_VIEW_DESC srv;
    srv.Format = desc.Format;
    srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv.Texture2D.MipLevels = 1;
    srv.Texture2D.MostDetailedMip = 0;
    
    device->CreateShaderResourceView( m_pRandomTexture, &srv, &m_pRandomTextureSRV );
}
