#include <cassert>

#include "core/game.h"
#include "render/render.h"
#include "render/d3d11_common.h"
#include "orbit_component.h"

OrbitComponent::Sphere::Sphere(float radius, float distance, Vector3 color)
    : parent_{ nullptr }, radius_{ radius },
    distance_{ distance }, local_rotation_{ Quaternion::Identity },
    color_{ color }
{
}

void OrbitComponent::Sphere::add_child(Sphere* child)
{
    child->parent_ = this;
    children_.push_back(child);
}

void OrbitComponent::Sphere::clear()
{
    for (auto& child : children_) {
        child->clear();
        delete child;
    }
}

Quaternion OrbitComponent::Sphere::rotation() const
{
    if (parent_ != nullptr) {
        return parent_->rotation() * local_rotation_;
    }
    return local_rotation_;
}

void OrbitComponent::Sphere::draw()
{
    
}

OrbitComponent::OrbitComponent()
{
    system_root_ = nullptr;
}

void OrbitComponent::initialize()
{
    assert(system_root_ == nullptr);
    // setup GPU things
    sphere_draw_shader_.set_vs_shader_from_memory(sphere_draw_shader_source_, "VSMain", nullptr, nullptr);
    sphere_draw_shader_.set_ps_shader_from_memory(sphere_draw_shader_source_, "PSMain", nullptr, nullptr);
    D3D11_INPUT_ELEMENT_DESC inputs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    sphere_draw_shader_.set_input_layout(inputs, std::size(inputs));

    {
        struct Vertex
        {
            Vector3 pos;
            Vector3 normal;
        };
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        constexpr uint32_t horizontal_segments_count = 10;
        constexpr uint32_t vertical_segments_count = 10;

        for (uint32_t i = 0; i < vertical_segments_count; ++i) {
            float vertical_angle = i * 3.14159265359f / vertical_segments_count;
            float next_vertical_angle = (i + 1) * 3.14159265359f / vertical_segments_count;
            for (uint32_t j = 0; j < horizontal_segments_count; ++j) {
                float horizontal_angle = j * 2 * 3.14159265359f / horizontal_segments_count;
                float next_horizontal_angle = (j + 1) * 2 * 3.14159265359f / horizontal_segments_count;
                if (i == 0) {
                    Vector3 pos1{ sinf(vertical_angle) * cosf(horizontal_angle), cosf(vertical_angle), sinf(vertical_angle) * cosf(horizontal_angle) };
                    Vector3 pos2{ sinf(next_vertical_angle) * cosf(horizontal_angle), cosf(next_vertical_angle), sinf(next_vertical_angle) * sinf(horizontal_angle) };
                    Vector3 pos3{ sinf(next_vertical_angle) * cosf(next_horizontal_angle), cosf(next_vertical_angle), sinf(next_vertical_angle) * sinf(next_horizontal_angle) };
                    Vector3 v1, v2;
                    (pos1 - pos2).Normalize(v1);
                    (pos1 - pos3).Normalize(v2);
                    Vector3 normal = v1.Cross(v2);
                    Vertex ver1{ pos1, normal };
                    Vertex ver2{ pos2, normal };
                    Vertex ver3{ pos3, normal };
                    indices.push_back(uint32_t(vertices.size()));
                    vertices.push_back(ver1);
                    indices.push_back(uint32_t(vertices.size()));
                    vertices.push_back(ver2);
                    indices.push_back(uint32_t(vertices.size()));
                    vertices.push_back(ver3);
                } else if (i == vertical_segments_count - 1) {
                    Vector3 pos1{ sinf(next_vertical_angle) * cosf(horizontal_angle), cosf(next_vertical_angle), sinf(next_vertical_angle) * cosf(horizontal_angle) };
                    Vector3 pos2{ sinf(vertical_angle) * cosf(horizontal_angle), cosf(vertical_angle), sinf(vertical_angle) * sinf(horizontal_angle) };
                    Vector3 pos3{ sinf(vertical_angle) * cosf(next_horizontal_angle), cosf(vertical_angle), sinf(vertical_angle) * sinf(next_horizontal_angle) };
                    Vector3 v1, v2;
                    (pos1 - pos2).Normalize(v1);
                    (pos1 - pos3).Normalize(v2);
                    Vector3 normal = v1.Cross(v2);
                    Vertex ver1{ pos1, normal };
                    Vertex ver2{ pos2, normal };
                    Vertex ver3{ pos3, normal };
                    indices.push_back(uint32_t(vertices.size()));
                    vertices.push_back(ver1);
                    indices.push_back(uint32_t(vertices.size()));
                    vertices.push_back(ver2);
                    indices.push_back(uint32_t(vertices.size()));
                    vertices.push_back(ver3);
                } else {
                    uint32_t index_offset = uint32_t(vertices.size());
                    Vector3 pos1{ sinf(vertical_angle) * cosf(horizontal_angle), cosf(vertical_angle), sinf(vertical_angle) * cosf(horizontal_angle) };
                    Vector3 pos2{ sinf(next_vertical_angle) * cosf(next_horizontal_angle), cosf(next_vertical_angle), sinf(next_vertical_angle) * cosf(next_horizontal_angle) };
                    Vector3 pos3{ sinf(vertical_angle) * cosf(next_horizontal_angle), cosf(vertical_angle), sinf(vertical_angle) * cosf(next_horizontal_angle) };
                    Vector3 pos4{ sinf(next_vertical_angle) * cosf(horizontal_angle), cosf(next_vertical_angle), sinf(next_vertical_angle) * cosf(horizontal_angle) };
                    Vector3 v1, v2;
                    (pos1 - pos2).Normalize(v1);
                    (pos1 - pos3).Normalize(v2);
                    Vector3 normal = v1.Cross(v2);
                    Vertex ver1{ pos1, normal };
                    Vertex ver2{ pos2, normal };
                    Vertex ver3{ pos3, normal };
                    Vertex ver4{ pos4, normal };
                    vertices.push_back(ver1);
                    vertices.push_back(ver2);
                    vertices.push_back(ver3);
                    vertices.push_back(ver4);
                    indices.push_back(index_offset + 0);
                    indices.push_back(index_offset + 1);
                    indices.push_back(index_offset + 2);
                    indices.push_back(index_offset + 0);
                    indices.push_back(index_offset + 1);
                    indices.push_back(index_offset + 3);
                }
            }
        }

        sphere_vertex_buffer_.initialize(D3D11_BIND_VERTEX_BUFFER, vertices.data(), sizeof(Vertex), uint32_t(vertices.size()));
        sphere_index_buffer_.initialize(D3D11_BIND_INDEX_BUFFER, indices.data(), sizeof(uint32_t), uint32_t(indices.size()));
        sphere_info_buffer_.initialize(sizeof(SphereInfo), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    }

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateRasterizerState(&rastDesc, &rasterizer_state_));

    // setup CPU
    system_root_ = new Sphere(100, 0, Vector3(1.f, 1.f, 0.f));
    system_root_->add_child(new Sphere(10, 200, Vector3(1.f, 1.f, 1.f)));
}

void OrbitComponent::draw()
{
    auto context = Game::inst()->render().context();
    context->RSSetState(rasterizer_state_);

    sphere_draw_shader_.use();

    sphere_vertex_buffer_.bind(0);
    sphere_index_buffer_.bind();
    system_root_->draw();
}

void OrbitComponent::imgui()
{

}

void OrbitComponent::reload()
{

}
void OrbitComponent::update()
{

}
void OrbitComponent::destroy_resources()
{
    rasterizer_state_->Release();

    system_root_->clear();
    delete system_root_;
    system_root_ = nullptr;

    sphere_vertex_buffer_.destroy();
    sphere_index_buffer_.destroy();
    sphere_info_buffer_.destroy();
    sphere_draw_shader_.destroy();
}

std::string OrbitComponent::sphere_draw_shader_source_{
R"(cbuffer SphereData : register(b0)
{
    float4x4 transform;
    float3 color;
};

struct VS_IN
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal;
}

PS_IN VSMain(VS_IN input)
{
    PS_IN res = (PS_IN)0;
    res.pos = mul(float4(input.position, 1.f), transform);
    res.normal = mul(float4(input.normal, 0.f), transpose(inverse(transform)));
}

float4 PSMain(PS_IN input) : SV_Target
{
    return float4(res.normal.xyz, 1.f);
}

)"};
