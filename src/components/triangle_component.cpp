#include <directxmath.h>

#include "render/render.h"
#include "core/game.h"
#include "render/d3d11_common.h"
#include "triangle_component.h"

TriangleComponent::TriangleComponent() {}
TriangleComponent::~TriangleComponent() {}

void TriangleComponent::initialize()
{
    shader_.set_vs_shader_from_memory(shader_source_, "VSMain", nullptr, nullptr);
    shader_.set_ps_shader_from_memory(shader_source_, "PSMain", nullptr, nullptr);

    D3D11_INPUT_ELEMENT_DESC inputs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    shader_.set_input_layout(inputs, std::size(inputs));
#ifndef NDEBUG
    shader_.set_name("triangle_shader");
#endif

    // create buffers
    auto device = Game::inst()->render().device();
    glm::vec4 points[] = { // full-screen triangle
        glm::vec4(3.f, -1.f, 0.f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        glm::vec4(-1.f, 3.f, 0.f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
        glm::vec4(-1.f, -1.f, 0.f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
    };

    vertex_buffer_.initialize(D3D11_BIND_VERTEX_BUFFER, points, sizeof(glm::vec4) * 2, 3);
#ifndef NDEBUG
    vertex_buffer_.set_name("triangle_vertex_buffer");
#endif

    std::vector<uint16_t> index_data = { 0, 1, 2 };
    index_buffer_.initialize(D3D11_BIND_INDEX_BUFFER, index_data.data(), sizeof(index_data[0]), UINT(std::size(index_data)));
#ifndef NDEBUG
    index_buffer_.set_name("triangle_index_buffer");
#endif

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;

    D3D11_CHECK(device->CreateRasterizerState(&rastDesc, &rasterizer_state_));

#ifndef NDEBUG
    std::string rasterizer_state_name = "triangle_rasterizer_state";
    rasterizer_state_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(rasterizer_state_name.size()), rasterizer_state_name.c_str());
#endif

    // D3D11_BUFFER_DESC vertexBufDesc = {};
    // vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    // vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // vertexBufDesc.CPUAccessFlags = 0;
    // vertexBufDesc.MiscFlags = 0;
    // vertexBufDesc.StructureByteStride = 0;
    // vertexBufDesc.ByteWidth = UINT(sizeof(DirectX::XMFLOAT4) * std::size(points));

    // D3D11_SUBRESOURCE_DATA vertexData = {};
    // vertexData.pSysMem = points;
    // vertexData.SysMemPitch = 0;
    // vertexData.SysMemSlicePitch = 0;

    // D3D11_CHECK(device->CreateBuffer(&vertexBufDesc, &vertexData, &vertex_buffer_));

    // int indices[] = { 0,1,2 };
    // D3D11_BUFFER_DESC indexBufDesc = {};
    // indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    // indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    // indexBufDesc.CPUAccessFlags = 0;
    // indexBufDesc.MiscFlags = 0;
    // indexBufDesc.StructureByteStride = 0;
    // indexBufDesc.ByteWidth = UINT(sizeof(int) * std::size(indices));

    // D3D11_SUBRESOURCE_DATA indexData = {};
    // indexData.pSysMem = indices;
    // indexData.SysMemPitch = 0;
    // indexData.SysMemSlicePitch = 0;

    // D3D11_CHECK(device->CreateBuffer(&indexBufDesc, &indexData, &index_buffer_));
}

void TriangleComponent::draw()
{
    auto context = Game::inst()->render().context();
    auto device = Game::inst()->render().device();

    // bind current shader
    shader_.use();

    // setup tiangles draw
    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // bind draw pipeline input buffers
    index_buffer_.bind();
    vertex_buffer_.bind();

    context->RSSetState(rasterizer_state_);

    context->DrawIndexed(index_buffer_.count(), 0, 0);
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
    vertex_buffer_.destroy();
    index_buffer_.destroy();

    SAFE_RELEASE(rasterizer_state_);
}

std::string TriangleComponent::shader_source_{
R"(struct VS_IN
{
    float4 pos : POSITION0;
    float4 col : COLOR0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN output = (PS_IN)0;

    output.pos = input.pos;
    output.col = input.col;
    
    return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    float4 col = input.col;
    return col;
}
)"};
