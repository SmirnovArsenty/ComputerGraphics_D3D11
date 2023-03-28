#include "core/game.h"
#include "win32/win.h"
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
    light_shader_.set_gs_shader_from_memory(light_shader_source_, "GSMain", nullptr, nullptr);
    light_shader_.set_input_layout(inputs, std::size(inputs));
#ifndef NDEBUG
    light_shader_.set_name("scene_light_shader");
#endif

    std::string shadow_cascade_size = std::to_string(Light::shadow_cascade_count);
    std::string light_count = std::to_string(lights_.size());
    D3D_SHADER_MACRO macro[] = {
        "SHADOW_CASCADE_SIZE", shadow_cascade_size.c_str(),
        "LIGHT_COUNT", light_count.c_str(),
        nullptr, nullptr
    };
    shader_.set_vs_shader_from_memory(shader_source_, "VSMain", macro, nullptr);
    shader_.set_ps_shader_from_memory(shader_source_, "PSMain", macro, nullptr);
    shader_.set_input_layout(inputs, std::size(inputs));
#ifndef NDEBUG
    shader_.set_name("scene_draw_shader");
#endif

    CD3D11_RASTERIZER_DESC rast_desc = {};
    rast_desc.CullMode = D3D11_CULL_NONE;
    rast_desc.FillMode = D3D11_FILL_SOLID;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateRasterizerState(&rast_desc, &rasterizer_state_));

    CD3D11_RASTERIZER_DESC light_rast_desc = {};
    light_rast_desc.CullMode = D3D11_CULL_NONE;
    light_rast_desc.FillMode = D3D11_FILL_SOLID;
    light_rast_desc.DepthBias = 0;
    D3D11_CHECK(device->CreateRasterizerState(&light_rast_desc, &light_rasterizer_state_));

    uniform_buffer_.initialize(sizeof(uniform_data_), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    assert(lights_.size() > 0);
    std::vector<Light::LightData> lights_data;
    for (auto& light : lights_) {
        lights_data.push_back(light->get_data());
    }
    assert(lights_data.size() > 0);
    lights_buffer_.initialize(D3D11_BIND_SHADER_RESOURCE,
                                lights_data.data(), sizeof(Light::LightData), static_cast<UINT>(lights_data.size()),
                                D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    light_data_buffer_.initialize(sizeof(Light::LightData), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    // light_transform_buffer_.initialize(sizeof(Light::CascadeData), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    D3D11_SAMPLER_DESC depth_sampler_desc{};
    depth_sampler_desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    depth_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    depth_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    depth_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    depth_sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    depth_sampler_desc.MinLOD = 0;
    depth_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
    D3D11_CHECK(Game::inst()->render().device()->CreateSamplerState(&depth_sampler_desc, &depth_sampler_state_));

    // deferred initialize
    RECT rc;
    GetWindowRect(Game::inst()->win().window(), &rc);
    UINT width = UINT(rc.right - rc.left);
    UINT height = UINT(rc.bottom - rc.top);

    D3D11_TEXTURE2D_DESC rtv_tex_desc{};
    rtv_tex_desc.Width = width;
    rtv_tex_desc.Height = height;
    rtv_tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtv_tex_desc.MipLevels = 1;
    rtv_tex_desc.ArraySize = 1;
    rtv_tex_desc.SampleDesc.Count = 1;
    rtv_tex_desc.SampleDesc.Quality = 0;
    rtv_tex_desc.Usage = D3D11_USAGE_DEFAULT;
    rtv_tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    rtv_tex_desc.CPUAccessFlags = 0;
    rtv_tex_desc.MiscFlags = 0;

    for (uint32_t i = 0; i < gbuffer_count_; ++i) {
        D3D11_CHECK(device->CreateTexture2D(&rtv_tex_desc, nullptr, &deferred_gbuffers_[i]));
        D3D11_CHECK(device->CreateRenderTargetView((ID3D11Resource*)deferred_gbuffers_[i], nullptr, &deferred_gbuffers_target_view_[i]));
        D3D11_CHECK(device->CreateShaderResourceView((ID3D11Resource*)deferred_gbuffers_[i], nullptr, &deferred_gbuffers_view_[i]));
    }

    D3D11_TEXTURE2D_DESC depth_desc{};
    depth_desc.Width = width;
    depth_desc.Height = height;
    depth_desc.Format = DXGI_FORMAT_R32_TYPELESS;
    depth_desc.MipLevels = 1;
    depth_desc.ArraySize = 1;
    depth_desc.SampleDesc.Count = 1;
    depth_desc.SampleDesc.Quality = 0;
    depth_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    depth_desc.CPUAccessFlags = 0;
    depth_desc.MiscFlags = 0;
    D3D11_CHECK(device->CreateTexture2D(&depth_desc, nullptr, &deferred_depth_buffer_));

    D3D11_DEPTH_STENCIL_VIEW_DESC ds_desc{};
    ds_desc.Format = DXGI_FORMAT_D32_FLOAT;
    ds_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ds_desc.Texture2D.MipSlice = 0;
    D3D11_CHECK(device->CreateDepthStencilView(deferred_depth_buffer_, &ds_desc, &deferred_depth_target_view_));

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srv_desc.Texture2DArray.MostDetailedMip = 0;
    srv_desc.Texture2DArray.MipLevels = 1;
    srv_desc.Texture2DArray.FirstArraySlice = 0;
    srv_desc.Texture2DArray.ArraySize = 1;
    D3D11_CHECK(device->CreateShaderResourceView(deferred_depth_buffer_, &srv_desc, &deferred_depth_view_));
}

void Scene::destroy()
{
    models_.clear();
    lights_.clear();
    light_data_buffer_.destroy();
    lights_buffer_.destroy();
    uniform_buffer_.destroy();
    light_rasterizer_state_->Release();
    light_rasterizer_state_ = nullptr;
    rasterizer_state_->Release();
    rasterizer_state_ = nullptr;
    depth_sampler_state_->Release();
    depth_sampler_state_ = nullptr;
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
    context->RSSetState(light_rasterizer_state_);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = Light::shadow_map_resolution;
    viewport.Height = Light::shadow_map_resolution;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;

    // calculate shadows
    std::vector<Vector4> frustum_corners[Light::shadow_cascade_count];
    auto view_frustums = camera->cascade_view_proj();
    for (uint32_t i = 0; i < Light::shadow_cascade_count; ++i) {
        frustum_corners[i].reserve(8);
        Matrix inv_view_proj = view_frustums[i].first.Invert();
        for (uint32_t x = 0; x < 2; ++x){
            for (uint32_t y = 0; y < 2; ++y) {
                for (uint32_t z = 0; z < 2; ++z) {
                    Vector4 pt = Vector4::Transform(
                                    Vector4(2.f * x - 1.f, 2.f * y - 1.f, z * 1.f, 1.f),
                                    inv_view_proj);
                    frustum_corners[i].push_back(pt / pt.w);
                }
            }
        }
    }
    Vector3 center[Light::shadow_cascade_count]{ };
    for (uint32_t i = 0; i < Light::shadow_cascade_count; ++i) {
        center[i] = Vector3::Zero;
        for (const auto &v : frustum_corners[i]) {
            center[i] += Vector3(v.x, v.y, v.z);
        }
        center[i] /= float(frustum_corners[i].size());
    }

    light_shader_.use();
    for (auto& light : lights_)
    {
        context->OMSetRenderTargets(0, nullptr, light->get_depth_map());
        context->ClearDepthStencilView(light->get_depth_map(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0xFF);
        context->RSSetViewports(1, &viewport);
        for (uint32_t i = 0; i < Light::shadow_cascade_count; ++i) {
            Light::LightData& light_data = light->get_data();
            light_data_buffer_.update_data(&light_data);
            light_data_buffer_.bind(0);

            Matrix light_view = Matrix::CreateLookAt(center[i] - light_data.direction, center[i], Vector3(0.f, 1.f, 0.f));

            float minX = -std::numeric_limits<float>::lowest();
            float maxX = std::numeric_limits<float>::lowest();
            float minY = -std::numeric_limits<float>::lowest();
            float maxY = std::numeric_limits<float>::lowest();
            float minZ = -std::numeric_limits<float>::lowest();
            float maxZ = std::numeric_limits<float>::lowest();
            for (auto &v : frustum_corners[i]) {
                Vector4 trf = Vector4::Transform(v, light_view);
                minX = minX > trf.x ? trf.x : minX;
                maxX = maxX < trf.x ? trf.x : maxX;
                minY = minY > trf.y ? trf.y : minY;
                maxY = maxY < trf.y ? trf.y : maxY;
                minZ = minZ > trf.z ? trf.z : minZ;
                maxZ = maxZ < trf.z ? trf.z : maxZ;
            }
            Matrix light_proj = Matrix::Identity;
            switch (light_data.type)
            {
            case Light::Type::direction:
            {
                constexpr float zMult = 10.f;
                minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
                maxZ = (maxZ < 0) ? maxZ / zMult : maxZ * zMult;
                light_proj = Matrix::CreateOrthographicOffCenter(minX, maxX, minY, maxY, minZ, maxZ);
                break;
            }
            default:
            {
                // light_proj = Matrix::CreatePerspectiveOffCenter(minX, maxX, minY, maxY, minZ, maxZ);
                break;
            }
            }
            Matrix light_transform = light_view * light_proj;
            light->set_transform(i, light_transform, view_frustums[i].second);
        }

        for (auto& model : models_) {
            model->draw();
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

    std::vector<Light::LightData> lights_data;
    for (auto& light : lights_) {
        lights_data.push_back(light->get_data());
    }
    lights_buffer_.update_data(lights_data.data());
    lights_buffer_.bind(0);

    std::vector<ID3D11ShaderResourceView*> tex_array;
    for (auto& light : lights_)
    {
        tex_array.push_back(light->get_depth_view());
    }
    context->PSSetShaderResources(10, UINT(tex_array.size()), tex_array.data());

    context->PSSetSamplers(1, 1, &depth_sampler_state_);

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
    float3 world_model_pos : POSITION0;
    float3 mvp_model_pos : POSITION1;
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

    float4x4 cascade_view_proj[SHADOW_CASCADE_SIZE];
    float4 distances;
};

StructuredBuffer<Light> lights  : register(t0);
Texture2D<float4> texture_1     : register(t1);
Texture2D<float4> texture_2     : register(t2);
Texture2D<float4> texture_3     : register(t3);
Texture2D<float4> texture_4     : register(t4);
Texture2D<float4> texture_5     : register(t5);
Texture2D<float4> texture_6     : register(t6);
SamplerState tex_sampler : register(s0);
SamplerComparisonState depth_sampler : register(s1);

Texture2DArray<float> shadow_cascade[LIGHT_COUNT] : register(t10);

PS_IN VSMain(VS_IN input)
{
    PS_IN res = (PS_IN)0;
    float4 world_model_pos = mul(transform, float4(input.pos_uv_x.xyz, 1.f));
    res.world_model_pos = world_model_pos.xyz / world_model_pos.w;
    res.pos = mul(view_proj, float4(res.world_model_pos, 1.f));
    res.mvp_model_pos = res.pos.xyz / res.pos.w;
    res.normal = mul(inverse_transpose_transform, float4(input.normal_uv_y.xyz, 0.f)).xyz;
    res.uv = float2(input.pos_uv_x.w, input.normal_uv_y.w);

    return res;
}

PS_OUT PSMain(PS_IN input)
{
    PS_OUT res = (PS_OUT)0;

    float3 normal = normalize(input.normal);
    float3 world_pos = input.world_model_pos;

    float depth_distance = length(world_pos - camera_pos);

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

        if ((material_flags & 7) == 0) { // no material provided - draw gray
            diffuse_color = float4(.2f, .2f, .2f, 1.f);
            specular_color = (0.5).xxxx;
            ambient_color = (0.1).xxxx;
        }

        res.color = (0).xxxx;
        for (uint i = 0; i < LIGHT_COUNT; ++i)
        {
            int cascade_index = 0;
            float depth = input.mvp_model_pos.z;
            // distances.z = far plane
            if (depth_distance > lights[i].distances.x) {
                cascade_index++;
            }
            if (depth_distance > lights[i].distances.y) {
                cascade_index++;
            }

            float4 pos_in_light_space = mul(lights[i].cascade_view_proj[cascade_index], float4(world_pos, 1.f));
            pos_in_light_space = pos_in_light_space / pos_in_light_space.w;
            // shadow value: 0 - darkest shadow; 1 - no shadow
            float shadow_value = shadow_cascade[i].SampleCmp(depth_sampler,
                        float3(pos_in_light_space.xy * (0.5).xx + (0.5).xx, cascade_index) * float3(1, -1, 1),
                        pos_in_light_space.z - 0.0003);

            float3 diffuse_component = lights[i].color * diffuse_color.xyz * max(dot(normal, -normalize(lights[i].direction)), 0);
            float3 reflected_view = reflect(normalize(lights[i].direction), normal);
            float cos_phi = dot(reflected_view, normalize(camera_pos - world_pos));
            float3 specular_component = lights[i].color * specular_color * pow(max(cos_phi, 0), 5);

            res.color.xyz += (diffuse_component + specular_component) * shadow_value;
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
R"(cbuffer LightData : register(b0)
{
    uint type;
    float3 color;

    float3 origin;
    float angle;

    float3 direction;
    float dummy;

    float4x4 cascade_view_proj[3];
    float4 distances;
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

struct VS_IN
{
    float4 pos_uv_x : POSITION_UV_X0;
    float4 normal_uv_y : NORMAL_UV_Y0;
};

struct GS_IN
{
    float4 pos: POSITION;
};

GS_IN VSMain(VS_IN input)
{
    GS_IN res = (GS_IN)0;
    res.pos = mul(transform, float4(input.pos_uv_x.xyz, 1.f));
    // res.position = mul(light_transform, res.position);
    return res;
}

struct GS_OUT
{
    float4 pos : SV_POSITION;
    uint arr_ind : SV_RenderTargetArrayIndex;
};

[instance(3)]
[maxvertexcount(3)]
void GSMain(triangle GS_IN p[3],
            in uint id : SV_GSInstanceID,
            inout TriangleStream<GS_OUT> stream)
{
    [unroll]
    for (int i = 0; i < 3; ++i) {
        GS_OUT gs = (GS_OUT)0;
        gs.pos = mul(cascade_view_proj[id], float4(p[i].pos.xyz, 1.f));
        gs.arr_ind = id;
        stream.Append(gs);
    }
}
)";
