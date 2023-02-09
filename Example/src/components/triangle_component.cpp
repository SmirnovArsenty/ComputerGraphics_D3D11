#include <directxmath.h>

#include "render/render.h"
#include "core/game.h"
#include "render/d3d11_common.h"
#include "components/game_component_decl.h"

TriangleComponent::TriangleComponent() {}
TriangleComponent::~TriangleComponent() {}

void TriangleComponent::initialize()
{
    shader_.set_vs_shader((std::string(resource_path_) + "shaders/shader.hls").c_str(),
                          "VSMain", nullptr, nullptr);
    // D3D_SHADER_MACRO pixel_shader_macros[] = {
    //     "TEST", "1",
    //     "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)",
    //     nullptr, nullptr
    // };
    shader_.set_ps_shader((std::string(resource_path_) + "shaders/shader.hls").c_str(),
                          "PSMain", nullptr /* pixel_shader_macros */, nullptr);

    D3D11_INPUT_ELEMENT_DESC inputs[] = {
        D3D11_INPUT_ELEMENT_DESC {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0},
        D3D11_INPUT_ELEMENT_DESC {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            D3D11_APPEND_ALIGNED_ELEMENT, // 32 // sizeof(DirectX::XMFLOAT4)
            D3D11_INPUT_PER_VERTEX_DATA,
            0}
    };
    shader_.set_input_layout(inputs, 2);

    // create buffers
    auto device = Game::inst()->render().device();
    DirectX::XMFLOAT4 points[] = { // full-screen triangle
        DirectX::XMFLOAT4(3.f, -1.f, 0.f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
        DirectX::XMFLOAT4(-1.f, 3.f, 0.f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
        DirectX::XMFLOAT4(-1.f, -1.f, 0.f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
    };

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = UINT(sizeof(DirectX::XMFLOAT4) * std::size(points));

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = points;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    D3D11_CHECK(device->CreateBuffer(&vertexBufDesc, &vertexData, &vertex_buffer_));

    int indices[] = { 0,1,2 };
    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.CPUAccessFlags = 0;
    indexBufDesc.MiscFlags = 0;
    indexBufDesc.StructureByteStride = 0;
    indexBufDesc.ByteWidth = UINT(sizeof(int) * std::size(indices));

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    D3D11_CHECK(device->CreateBuffer(&indexBufDesc, &indexData, &index_buffer_));
}

void TriangleComponent::draw()
{
    shader_.use();
    auto context = Game::inst()->render().context();
    auto device = Game::inst()->render().device();
    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);
    UINT strides[] = { 32 };
    UINT offsets[] = { 0 };
    context->IASetVertexBuffers(0, 1, &vertex_buffer_, strides, offsets);

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;

    D3D11_CHECK(device->CreateRasterizerState(&rastDesc, &rasterizer_state_));
    context->RSSetState(rasterizer_state_);

    context->DrawIndexed(3, 0, 0);
}

void TriangleComponent::reload()
{
    // stub
}

void TriangleComponent::update()
{
    // stub
}

void TriangleComponent::destroy_resources()
{
    SAFE_RELEASE(vertex_buffer_);
    SAFE_RELEASE(index_buffer_);

    SAFE_RELEASE(rasterizer_state_);
}
