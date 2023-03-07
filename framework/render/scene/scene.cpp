#include "core/game.h"
#include "render/render.h"
#include "render/camera.h"
#include "render/d3d11_common.h"

#include "scene.h"
#include "model.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::initialize()
{
    // setup shader
    shader_.set_vs_shader_from_memory(shader_source_, "VSMain", nullptr, nullptr);
    shader_.set_ps_shader_from_memory(shader_source_, "PSMain", nullptr, nullptr);
    D3D11_INPUT_ELEMENT_DESC inputs[] = {
        { "POSITION_UV_X", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL_UV_Y", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    shader_.set_input_layout(inputs, std::size(inputs));
#ifndef NDEBUG
    shader_.set_name("scene_draw_shader");
#endif

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateRasterizerState(&rastDesc, &rasterizer_state_));

    uniform_buffer_.initialize(sizeof(uniform_data_), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void Scene::destroy()
{
    uniform_buffer_.destroy();
    rasterizer_state_->Release();
    rasterizer_state_ = nullptr;
    shader_.destroy();
}

void Scene::add_model(const std::string& filename)
{
    models_.push_back(new Model(filename));
}

void Scene::update()
{
    auto camera = Game::inst()->render().camera();
    uniform_data_.view_proj = camera->view_proj();
    uniform_data_.camera_pos = camera->position();
    uniform_data_.camera_dir = camera->direction();
    uniform_data_.time += Game::inst()->delta_time();
}

void Scene::draw()
{
    auto context = Game::inst()->render().context();

    shader_.use();
    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    uniform_buffer_.bind(0);

    for (auto& model : models_) {
        model->draw();
    }
}

std::string Scene::shader_source_ =
R"(struct VS_IN
{
    float4 pos_uv_x : POSITION_UV_X0;
    float4 normal_uv_y : NORMAL_UV_Y0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 model_pos;
    float3 normal;
    float2 uv;
};

struct PS_OUT
{
    float4 color: SV_Target;
};

cbuffer SceneData : register(b0)
{
    float4x4 view_proj;
    float3 camera_pos;
    float time;
    float3 camera_dir;
    float dummy;
};

cbuffer ModelData : register(b1)
{
    float4x4 transform;
    float4x4 inverse_transpose_transform;
};

cbuffer MeshData : register(b2)
{
    uint material_flags;
};

Texture2D<float4> diffuse   : register(t0);
Texture2D<float4> specular  : register(t1);
Texture2D<float4> ambient   : register(t2);

SamplerState sampler : register(s0);

PS_IN VSMain(VS_IN input)
{
    PS_IN res = (PI_IN)0;
    res.model_pos = mul(transform, float4(input.pos_uv_x.xyz, 1.f)).xyz;
    res.pos = mul(view_proj, float4(res.model_pos, 1.f));
    res.normal = mul(inverse_transpose_transform, float4(input.normal_uv_y.xyz, 0.f)).xyz;
    res.uv = float2(input.pos_uv_x.w, input.normal_uv_y.w);

    return res;
}

PS_OUT PSMain(PS_IN input)
{
    PS_OUT res = (PS_OUT)0;

    float3 normal = normalize(input.normal);
    float3 pos = input.model_pos;

    float4 diffuse_color = (0).xxxx;
    if (material_flags & 1) {
        diffuse_color = diffuse.Sample(sampler, input.uv);
    }
    float4 specular_color = (0).xxxx;
    if (material_flags & 2) {
        specular_color = specular.Sample(sampler, input.uv);
    }
    float4 ambient_color = (0).xxxx;
    if (material_flags & 4) {
        ambient_color = ambient.Sample(sampler, input.uv);
    }

    return diffuse_color; // debug
}

)";
