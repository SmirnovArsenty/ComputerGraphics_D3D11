#include "particle_system.h"

#include "render/render.h"
#include "render/d3d11_common.h"

ParticleSystem::ParticleSystem(uint32_t max_particles, Vector3 origin)
    : max_particles_count_{ max_particles }
{

}

ParticleSystem::~ParticleSystem()
{

}

void ParticleSystem::initialize()
{
    auto device = Game::inst()->render().device();

    uniform_buffer_.initialize(sizeof(uniform_data_));

    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    buffer_desc.StructureByteStride = sizeof(Particle);
    buffer_desc.ByteWidth = sizeof(Particle) * max_particles_count_;
    D3D11_CHECK(device->CreateBuffer(&buffer_desc, nullptr, &particle_pool_));

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_UNKNOWN;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srv_desc.Buffer.ElementOffset = 0;
    srv_desc.Buffer.ElementWidth = max_particles_count_;
    D3D11_CHECK(device->CreateShaderResourceView(particle_pool_, &srv_desc, &particle_pool_SRV_));

    D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
    uav_desc.Format = DXGI_FORMAT_UNKNOWN;
    uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uav_desc.Buffer.FirstElement = 0;
    uav_desc.Buffer.NumElements = max_particles_count_;
    uav_desc.Buffer.Flags = 0;
    D3D11_CHECK(device->CreateUnorderedAccessView(particle_pool_, &uav_desc, &particle_pool_UAV_));

    buffer_desc.StructureByteStride = sizeof(uint32_t);
    buffer_desc.ByteWidth = sizeof(uint32_t) * max_particles_count_;
    D3D11_CHECK(device->CreateBuffer(&buffer_desc, nullptr, &dead_list_));

    uav_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    D3D11_CHECK(device->CreateUnorderedAccessView(dead_list_, &uav_desc, &dead_list_UAV_));

    buffer_desc.StructureByteStride = sizeof(float) * 2;
    buffer_desc.ByteWidth = (sizeof(float) * 2) * max_particles_count_;
    device->CreateBuffer(&buffer_desc, nullptr, &sort_list_);

    D3D11_CHECK(device->CreateShaderResourceView(sort_list_, &srv_desc, &sort_list_SRV_));

    uav_desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
    D3D11_CHECK(device->CreateUnorderedAccessView(sort_list_, &uav_desc, &sort_list_UAV_));

    ZeroMemory(&buffer_desc, sizeof(buffer_desc));
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    buffer_desc.ByteWidth = 5 * sizeof(uint32_t);
    buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    D3D11_CHECK(device->CreateBuffer(&buffer_desc, nullptr, &indirect_args_));

    ZeroMemory(&uav_desc, sizeof(uav_desc));
    uav_desc.Format = DXGI_FORMAT_R32_UINT;
    uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uav_desc.Buffer.FirstElement = 0;
    uav_desc.Buffer.NumElements = 5;
    uav_desc.Buffer.Flags = 0;
    D3D11_CHECK(device->CreateUnorderedAccessView(indirect_args_, &uav_desc, &indirect_args_UAV_));

    ZeroMemory(&buffer_desc, sizeof(buffer_desc));
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.ByteWidth = 4 * sizeof(UINT);
    D3D11_CHECK(device->CreateBuffer(&buffer_desc, nullptr, &dead_list_const_buffer_));
    D3D11_CHECK(device->CreateBuffer(&buffer_desc, nullptr, &sort_list_const_buffer_));

    D3D11_BLEND_DESC blend_desc;
    ZeroMemory(&blend_desc, sizeof(D3D11_BLEND_DESC));
    blend_desc.AlphaToCoverageEnable = false;
    blend_desc.IndependentBlendEnable = false;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blend_desc.RenderTarget[0].BlendEnable = true;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    D3D11_CHECK(device->CreateBlendState(&blend_desc, &blend_state_));

    init_dead_list_.set_compute_shader_from_file("./resources/shaders/particles/init_dead_list.hlsl", "CSMain");
    emit_.set_compute_shader_from_file("./resources/shaders/particles/emit.hlsl", "CSMain");
    simulate_.set_compute_shader_from_file("./resources/shaders/particles/simulate.hlsl", "CSMain");
    reset_.set_compute_shader_from_file("./resources/shaders/particles/reset.hlsl", "CSMain");

    render_.set_vs_shader_from_file("./resources/shaders/particles/render.hlsl", "VSMain");
    render_.set_ps_shader_from_file("./resources/shaders/particles/render.hlsl", "PSMain");

    blit_.set_vs_shader_from_file("./resources/shaders/particles/blit.hlsl", "VSMain");
    blit_.set_ps_shader_from_file("./resources/shaders/particles/blit.hlsl", "PSMain");

    sort_lib_.init();

    reset_flag_ = true;
}

void ParticleSystem::draw()
{
    auto context = Game::inst()->render().context();

    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11DepthStencilView* dsv = nullptr;
    context->OMGetRenderTargets(1, &rtv, &dsv);
    context->OMSetRenderTargets(0, nullptr, nullptr);

    if (reset_flag_)
    {
        init_dead_list();

        UINT initial_count = (UINT)-1;
        context->CSSetUnorderedAccessViews(0, 1, &particle_pool_UAV_, &initial_count);
        reset_.use();
        context->Dispatch(align(max_particles_count_, 256) / 256, 1, 1);

        reset_flag_ = false;
    }

    EmitterParams params;
    params.origin = Vector3(0, 0, 0);
    params.num_to_emit = 1000;
    params.velocity = Vector3(0, 1, 0);
    params.particle_life_span = 10;
    params.start_size = 10;
    params.end_size = 1;
    params.mass = 1;
    params.velocity_variance = 0;
    emit(params);

    simulate();

    context->CopyStructureCount(sort_list_const_buffer_, 0, sort_list_UAV_);

    sort();

    render_.use();

    context->OMSetRenderTargets(1, &rtv, dsv);
}

void ParticleSystem::init_dead_list()
{
    auto context = Game::inst()->render().context();

    init_dead_list_.use();
    UINT initial_count = 0;
    context->CSSetUnorderedAccessViews(0, 1, &dead_list_UAV_, &initial_count);

    context->Dispatch(align(max_particles_count_, 256) / 256, 1, 1);
}

void ParticleSystem::emit(const ParticleSystem::EmitterParams& emitter_params)
{
    auto context = Game::inst()->render().context();
}

void ParticleSystem::simulate()
{
    auto context = Game::inst()->render().context();

    ID3D11UnorderedAccessView* uavs[] = { particle_pool_UAV_, dead_list_UAV_, sort_list_UAV_, indirect_args_UAV_ };
    UINT initialCounts[] = { (UINT)-1, (UINT)-1, 0, (UINT)-1 };
    context->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, initialCounts);

    // ID3D11ShaderResourceView* srvs[] = { depth_SRV };
    // context->CSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

    simulate_.use();
    context->Dispatch(align(max_particles_count_, 256) / 256, 1, 1);

    // ZeroMemory(srvs, sizeof(srvs));
    // context->CSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

    ZeroMemory(uavs, sizeof(uavs));
    context->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavs), uavs, nullptr);
}

void ParticleSystem::sort()
{
    auto context = Game::inst()->render().context();

    sort_lib_.run(max_particles_count_, sort_list_UAV_, sort_list_const_buffer_);
}
