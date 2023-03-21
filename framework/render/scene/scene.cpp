#include "core/game.h"
#include "render/render.h"
#include "render/camera.h"
#include "render/d3d11_common.h"

#include "scene.h"
#include "model.h"

Scene::Scene() : uniform_data_{}
{
}

Scene::~Scene()
{
}

void Scene::initialize()
{
    // setup shaders
    D3D11_INPUT_ELEMENT_DESC inputs[] = {
        { "POSITION_UV_X", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL_UV_Y", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    light_shader_.set_vs_shader_from_memory(light_shader_source_, "VSMain", nullptr, nullptr);
    light_shader_.set_ps_shader_from_memory(light_shader_source_, "PSMain", nullptr, nullptr);
    light_shader_.set_input_layout(inputs, std::size(inputs));
#ifndef NDEBUG
    light_shader_.set_name("scene_light_shader");
#endif

    shader_.set_vs_shader_from_memory(shader_source_, "VSMain", nullptr, nullptr);
    shader_.set_ps_shader_from_memory(shader_source_, "PSMain", nullptr, nullptr);
    shader_.set_input_layout(inputs, std::size(inputs));
#ifndef NDEBUG
    shader_.set_name("scene_draw_shader");
#endif

    CD3D11_RASTERIZER_DESC rast_desc = {};
    rast_desc.CullMode = D3D11_CULL_NONE;
    rast_desc.FillMode = D3D11_FILL_SOLID;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateRasterizerState(&rast_desc, &rasterizer_state_));

    uniform_buffer_.initialize(sizeof(uniform_data_), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    assert(lights_.size() > 0);
    std::vector<Light::LightData> lights_data;
    for (auto& light : lights_) {
        lights_data.push_back(light->get_data());
    }
    assert(lights_data.size() > 0);
    lights_buffer_.initialize(D3D11_BIND_SHADER_RESOURCE,
                                lights_data.data(), sizeof(Light::LightData), static_cast<UINT>(lights_data.size()),
                                D3D11_USAGE_IMMUTABLE, (D3D11_CPU_ACCESS_FLAG)0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);

    light_data_buffer_.initialize(sizeof(Light::LightData), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    // render target view
    ID3D11Texture2D* rtv_tex{ nullptr };
    D3D11_TEXTURE2D_DESC rtv_tex_desc{};
    rtv_tex_desc.Width = 512;
    rtv_tex_desc.Height = 512;
    rtv_tex_desc.MipLevels = 1;
    rtv_tex_desc.ArraySize = 1;
    rtv_tex_desc.SampleDesc.Count = 1;
    rtv_tex_desc.SampleDesc.Quality = 0;
    rtv_tex_desc.Usage = D3D11_USAGE_DEFAULT;
    rtv_tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtv_tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    rtv_tex_desc.CPUAccessFlags = 0;
    rtv_tex_desc.MiscFlags = 0;
    D3D11_CHECK(device->CreateTexture2D(&rtv_tex_desc, nullptr, &rtv_tex));
    D3D11_CHECK(device->CreateRenderTargetView((ID3D11Resource*)rtv_tex, nullptr, &render_target_view_));
}

void Scene::destroy()
{
    models_.clear();
    lights_.clear();
    light_data_buffer_.destroy();
    lights_buffer_.destroy();
    uniform_buffer_.destroy();
    rasterizer_state_->Release();
    rasterizer_state_ = nullptr;
    shader_.destroy();
    light_shader_.destroy();
}

void Scene::add_model(Model* model)
{
    models_.push_back(model);
}

void Scene::add_light(Light* light)
{
    lights_.push_back(light);
}

void Scene::update()
{
    auto camera = Game::inst()->render().camera();
    uniform_data_.view_proj = camera->view_proj();
    uniform_data_.camera_pos = camera->position();
    uniform_data_.camera_dir = camera->direction();
    uniform_data_.time += Game::inst()->delta_time();
    uniform_data_.lights_count = static_cast<uint32_t>(lights_.size());
}

void Scene::draw()
{
    auto context = Game::inst()->render().context();
    auto camera = Game::inst()->render().camera();

    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->RSSetState(rasterizer_state_);

    // calculate shadows
    light_shader_.use();
    for (auto& light : lights_)
    {
        uint32_t depth_map_count = light->get_depth_map_count();
        for (uint32_t i = 0; i < depth_map_count; ++i) {
            context->OMSetRenderTargets(0, nullptr, light->get_depth_map(i));
            context->ClearDepthStencilView(light->get_depth_map(i), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0xFF);

            Light::LightData& light_data = light->get_data();
            switch (light_data.type)
            {
            case Light::Type::direction:
            {
                light_data.origin = camera->position();
                break;
            }
            default:
            break;
            }
            light_data_buffer_.update_data(&light_data);
            light_data_buffer_.bind(0);

            for (auto& model : models_) {
                model->draw();
            }
        }
    }

    // restore default render target and depth stencil
    Game::inst()->render().prepare_resources();

    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->RSSetState(rasterizer_state_);

    // draw models
    shader_.use();
    uniform_buffer_.update_data(&uniform_data_);
    uniform_buffer_.bind(0);
    lights_buffer_.bind(0);

    ID3D11ShaderResourceView* tex_array[3] = {
        lights_[0]->get_depth_buffer(0).view(),
        lights_[0]->get_depth_buffer(1).view(),
        lights_[0]->get_depth_buffer(2).view()
    };

    context->PSSetShaderResources(10, 3, tex_array);

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
    float3 model_pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
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
    uint lights_count;
};

cbuffer ModelData : register(b1)
{
    float4x4 transform;
    float4x4 inverse_transpose_transform;
};

cbuffer MeshData : register(b2)
{
    uint is_pbr;
    uint material_flags;
    float2 MeshData_dummy;
};

struct Light
{
    uint type;
    float3 color;

    float3 origin;
    float angle;

    float3 direction;
    float dummy;
};

StructuredBuffer<Light> lights  : register(t0);
Texture2D<float4> texture_1     : register(t1);
Texture2D<float4> texture_2     : register(t2);
Texture2D<float4> texture_3     : register(t3);
Texture2D<float4> texture_4     : register(t4);
Texture2D<float4> texture_5     : register(t5);
Texture2D<float4> texture_6     : register(t6);
SamplerState tex_sampler : register(s0);

Texture2D<float> shadow_cascade[3] : register(t10);

PS_IN VSMain(VS_IN input)
{
    PS_IN res = (PS_IN)0;
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

    float depth0 = shadow_cascade[0].Sample(tex_sampler, float2(0, 0));
    float depth1 = shadow_cascade[1].Sample(tex_sampler, float2(0, 0));
    float depth2 = shadow_cascade[2].Sample(tex_sampler, float2(0, 0));
    if (depth0 < 0 || depth1 < 0 || depth2 < 0)
    {
        res.color.xyz = float3(1, 0, 0);
        res.color.w = 1.f;
        return res;
    }
    if (is_pbr)
    {
        float4 base_color = (0).xxxx;
        float4 normal_camera = (0).xxxx;
        float4 emission_color = (0).xxxx;
        float4 metalness = (0).xxxx;
        float4 diffuse_roughness = (0).xxxx;
        float4 ambient_occlusion = (0).xxxx;

        if (material_flags & 1) {
            base_color = texture_1.Sample(tex_sampler, input.uv);
        }
        if (material_flags & 2) {
            normal_camera = texture_2.Sample(tex_sampler, input.uv);
        }
        if (material_flags & 4) {
            emission_color = texture_3.Sample(tex_sampler, input.uv);
        }
        if (material_flags & 8) {
            metalness = texture_4.Sample(tex_sampler, input.uv);
        }
        if (material_flags & 16) {
            diffuse_roughness = texture_5.Sample(tex_sampler, input.uv);
        }
        if (material_flags & 32) {
            ambient_occlusion = texture_6.Sample(tex_sampler, input.uv);
        }

        res.color.xyz = base_color.xyz;
        res.color.w = 1.f;
    }
    else
    { // Phong light model
        float4 diffuse_color = (0).xxxx;
        float4 specular_color = (0).xxxx;
        float4 ambient_color = (0).xxxx;

        if (material_flags & 1) {
            diffuse_color = texture_1.Sample(tex_sampler, input.uv);
        }
        if (material_flags & 2) {
            specular_color = texture_2.Sample(tex_sampler, input.uv);
        }
        if (material_flags & 4) {
            ambient_color = texture_3.Sample(tex_sampler, input.uv);
        }

        if ((material_flags & 7) == 0) { // no material provided - draw magenta
            diffuse_color = float4(1.f, 0.f, 1.f, 1.f);
            specular_color = (0.5).xxxx;
            ambient_color = (0.1).xxxx;
        }

        res.color = (0).xxxx;
        for (uint i = 0; i < lights_count; ++i)
        {
            float3 diffuse_component = lights[i].color * diffuse_color.xyz * max(dot(normal, -normalize(lights[i].direction)), 0);
            float3 reflected_view = reflect(normalize(lights[i].direction), normal);
            float cos_phi = dot(reflected_view, normalize(camera_pos - pos));
            float3 specular_component = lights[i].color * specular_color * pow(max(cos_phi, 0), 5);

            res.color.xyz += diffuse_component + specular_component;
        }
        if (dot(ambient_color, ambient_color) > 0) {
            res.color += ambient_color * 0.2f;
        } else {
            res.color += diffuse_color * 0.2f;
        }
        res.color.w = 1.f; // TODO: look up to opacity map
    }

    // gamma correction
    res.color.xyz = pow(abs(res.color.xyz), 1/2.2f);
    return res;
}
)";


std::string Scene::light_shader_source_ =
R"(struct VS_IN
{
    float4 pos_uv_x : POSITION_UV_X0;
    float4 normal_uv_y : NORMAL_UV_Y0;
};

struct PS_IN
{
    float4 position : SV_POSITION;
};

cbuffer LightData : register(b0)
{
    uint type;
    float3 color;

    float3 origin;
    float angle;

    float3 direction;
    float dummy;
};

cbuffer ModelData : register(b1)
{
    float4x4 transform;
    float4x4 inverse_transpose_transform;
};

// not used
cbuffer MeshData : register(b2)
{
    uint is_pbr;
    uint material_flags;
    float2 MeshData_dummy;
};

float4x4 axis_matrix(float3 right, float3 up, float3 forward)
{
    float3 xaxis = right;
    float3 yaxis = up;
    float3 zaxis = forward;
    return float4x4(
        xaxis.x, yaxis.x, zaxis.x, 0,
        xaxis.y, yaxis.y, zaxis.y, 0,
        xaxis.z, yaxis.z, zaxis.z, 0,
        0, 0, 0, 1
    );
}

float4x4 look_at_matrix(float3 at, float3 eye, float3 up)
{
    float3 zaxis = normalize(at - eye);
    float3 xaxis = normalize(cross(up, zaxis));
    float3 yaxis = cross(zaxis, xaxis);
    return axis_matrix(xaxis, yaxis, zaxis);
}

float4x4 ortho_proj(float width, float height, float far, float near)
{
    return float4x4(2/width, 0, 0, 0,
                    0, 2/height, 0, 0,
                    0, 0, -2/(far-near), -(far+near)/(far-near),
                    0, 0, 0, 1);
}

float4x4 persp_proj(float far, float near)
{
    return float4x4(1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, -(far+near)/(far-near), -1,
                    0, 0, -(far*near)/(far-near), 0);
}

PS_IN VSMain(VS_IN input)
{
    PS_IN res = (PS_IN)0;
    res.position = mul(transform, float4(input.pos_uv_x.xyz, 1.f));
    res.position = mul(look_at_matrix(origin, origin - direction * 2.f, float3(0, 1, 0)), res.position);
    if (type == 1) // direction
    {
        res.position = mul(ortho_proj(1000, 1000, 1000, 0.1), res.position);
    }
    return res;
}

void PSMain(PS_IN input)
{
}
)";
