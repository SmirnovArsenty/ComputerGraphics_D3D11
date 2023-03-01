#include <cassert>
#include <imgui/imgui.h>

#include "core/game.h"
#include "render/render.h"
#include "render/camera.h"
#include "render/d3d11_common.h"
#include "orbit_component.h"

OrbitComponent::Sphere::Sphere(float radius, float distance, float speed, float local_speed, Vector3 color)
    : parent_{ nullptr }, radius_{ radius }, angle_{ 0.f }, angle_speed_{ speed }, local_speed_{ local_speed },
    distance_{ distance }, local_angle_{ 0.f },
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

size_t OrbitComponent::Sphere::child_count() const
{
    size_t res = children_.size();

    for (auto& child : children_) {
        res += child->child_count();
    }

    return res;
}

Matrix OrbitComponent::Sphere::transform() const
{
    if (parent_ == nullptr)
    {
        return Matrix::Identity;
    }
    Vector4 translation;
    Vector4::Transform(Vector4(1.f, 0.f, 0.f, 1.f), Matrix::CreateRotationY(angle_), translation);
    translation.x /= translation.w;
    translation.y /= translation.w;
    translation.z /= translation.w;
    translation.w = 1.f;

    translation.x *= distance_;
    translation.y *= distance_;
    translation.z *= distance_;

    return parent_->transform() * Matrix::CreateTranslation(translation.x, translation.y, translation.z);
}

void OrbitComponent::Sphere::draw(Buffer& sphere_index_buffer, ConstBuffer& sphere_info_buffer, const Matrix& view_proj)
{
    SphereInfo info;
    info.color = color_;
    info.transform = Matrix::CreateRotationY(local_angle_) * Matrix::CreateScale(radius_) * transform();
    info.inverse_transp_transform = info.transform.Invert().Transpose();
    info.view_proj = view_proj;
    sphere_info_buffer.update_data(&info);
    sphere_info_buffer.bind(0);

    auto context = Game::inst()->render().context();
    context->DrawIndexed(sphere_index_buffer.count(), 0, 0);

    for (auto& child : children_)
    {
        child->draw(sphere_index_buffer, sphere_info_buffer, view_proj);
    }
}

void OrbitComponent::Sphere::update()
{
    float delta_time = Game::inst()->delta_time();
    angle_ += delta_time * angle_speed_;
    local_angle_ += delta_time * local_speed_;

    for (auto& child : children_) {
        child->update();
    }
}

void OrbitComponent::Sphere::get_position(int &depth, Vector3 &res)
{
    if (depth < 0) {
        return;
    }
    if (depth == 0) {
        Vector3 scale;
        Quaternion rot;
        transform().Decompose(scale, rot, res);
    }

    --depth;
    for (auto& child : children_)
    {
        child->get_position(depth, res);
    }
}

OrbitComponent::OrbitComponent()
{
    system_root_ = nullptr;
    focus_on_ = false;
    focus_target_ = 0;
    camera_perspective_ = true;
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

        for (int32_t i = 0; i < vertical_segments_count_; ++i) {
            float vertical_angle = i * 3.14159265359f / vertical_segments_count_;
            float next_vertical_angle = (i + 1) * 3.14159265359f / vertical_segments_count_;
            for (int32_t j = 0; j < horizontal_segments_count_; ++j) {
                float horizontal_angle = j * 2 * 3.14159265359f / horizontal_segments_count_;
                float next_horizontal_angle = (j + 1) * 2 * 3.14159265359f / horizontal_segments_count_;
                if (i == 0) {
                    Vector3 pos1{ sinf(vertical_angle) * cosf(horizontal_angle), cosf(vertical_angle), sinf(vertical_angle) * sinf(horizontal_angle) };
                    Vector3 pos2{ sinf(next_vertical_angle) * cosf(horizontal_angle), cosf(next_vertical_angle), sinf(next_vertical_angle) * sinf(horizontal_angle) };
                    Vector3 pos3{ sinf(next_vertical_angle) * cosf(next_horizontal_angle), cosf(next_vertical_angle), sinf(next_vertical_angle) * sinf(next_horizontal_angle) };
                    Vector3 v1, v2;
                    (pos1 - pos2).Normalize(v1);
                    (pos1 - pos3).Normalize(v2);
                    Vector3 normal = -v1.Cross(v2);
                    Vertex ver1{ pos1, normal };
                    Vertex ver2{ pos2, normal };
                    Vertex ver3{ pos3, normal };
                    indices.push_back(uint32_t(vertices.size()));
                    vertices.push_back(ver1);
                    indices.push_back(uint32_t(vertices.size()));
                    vertices.push_back(ver2);
                    indices.push_back(uint32_t(vertices.size()));
                    vertices.push_back(ver3);
                } else if (i == vertical_segments_count_ - 1) {
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
                    Vector3 pos1{ sinf(vertical_angle) * cosf(horizontal_angle), cosf(vertical_angle), sinf(vertical_angle) * sinf(horizontal_angle) };
                    Vector3 pos2{ sinf(next_vertical_angle) * cosf(next_horizontal_angle), cosf(next_vertical_angle), sinf(next_vertical_angle) * sinf(next_horizontal_angle) };
                    Vector3 pos3{ sinf(vertical_angle) * cosf(next_horizontal_angle), cosf(vertical_angle), sinf(vertical_angle) * sinf(next_horizontal_angle) };
                    Vector3 pos4{ sinf(next_vertical_angle) * cosf(horizontal_angle), cosf(next_vertical_angle), sinf(next_vertical_angle) * sinf(horizontal_angle) };
                    Vector3 v1, v2;
                    (pos1 - pos2).Normalize(v1);
                    (pos1 - pos3).Normalize(v2);
                    Vector3 normal = -v1.Cross(v2);
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
    rastDesc.FillMode = D3D11_FILL_SOLID;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateRasterizerState(&rastDesc, &rasterizer_state_));

    // setup CPU
    system_root_ = new Sphere(100, 0, 0.f, 10.f, Vector3(1.f, 1.f, 0.f));
    Sphere *one = new Sphere(50, 500, 1.f, 100.f, Vector3(0.f, 7.f, 0.f));
    system_root_->add_child(one);
    one->add_child(new Sphere(10, 100, 2.3f, 1.f, Vector3(0.3f, 0.3f, 0.3f)));

    Sphere* two = new Sphere(50, 800, -1.f, 1.f, Vector3(0.3f, 0.5f, 0.7f));
    system_root_->add_child(two);
    two->add_child(new Sphere(10, 100, 1.f, 1.f, Vector3(0.7f, 0.5f, 0.3f)));
    two->add_child(new Sphere(10, 150, -1.f, 1.f, Vector3(0.7f, 0.5f, 0.3f)));

    camera_perspective_ = (Game::inst()->render().camera()->type() == Camera::CameraType::perspective);
    focus_on_ = false;
    focus_target_ = 0;
}

void OrbitComponent::draw()
{
    auto context = Game::inst()->render().context();
    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->RSSetState(rasterizer_state_);

    sphere_draw_shader_.use();

    sphere_vertex_buffer_.bind(0);
    sphere_index_buffer_.bind();
    auto camera = Game::inst()->render().camera();
    system_root_->draw(sphere_index_buffer_, sphere_info_buffer_, camera->view_proj());
}

void OrbitComponent::imgui()
{
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();

    uint32_t window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x / 10, main_viewport->WorkSize.y / 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 150), ImGuiCond_FirstUseEver);

    ImGui::Begin("Info", nullptr, window_flags);
    {
        ImGui::Text("Perspective camera");
        ImGui::SameLine();
        ImGui::Checkbox("   ", &camera_perspective_);

        ImGui::Text("Focus");
        ImGui::SameLine();
        ImGui::Checkbox("  ", &focus_on_);

        if (focus_on_) {
            ImGui::Text("Focus target");
            ImGui::SameLine();
            ImGui::SliderInt(" ", &focus_target_, 0, (int)system_root_->child_count());
        }
    }
    ImGui::End();
}

void OrbitComponent::reload()
{

}

void OrbitComponent::update()
{
    system_root_->update();

    auto camera = Game::inst()->render().camera();
    camera->set_type(camera_perspective_ ? Camera::CameraType::perspective : Camera::CameraType::orthographic);

    if (focus_on_) {
        Vector3 focus_pos;
        int depth = focus_target_;
        system_root_->get_position(depth, focus_pos);
        camera->focus(focus_pos);
    } else {
        camera->reset_focus();
    }
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
    float4x4 view_proj;
    float4x4 transform;
    float4x4 inv_transp_transform;
    float3 color;
    float reserved;
};

struct VS_IN
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 model_pos : POSITION;
    float3 normal : NORMAL;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN res = (PS_IN)0;
    res.model_pos = mul(transform, float4(input.position, 1.f)).xyz;
    res.pos = mul(view_proj, float4(res.model_pos, 1.f));
    res.normal = input.normal;//normalize(mul(inv_transp_transform, float4(input.normal, 0.f)));
    return res;
}

float4 PSMain(PS_IN input) : SV_Target
{
    return float4(abs(input.normal), 1.f);
}

)"};
