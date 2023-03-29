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
    auto device = Game::inst()->render().device();

    // setup shaders
    D3D11_INPUT_ELEMENT_DESC inputs[] = {
        { "POSITION_UV_X", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL_UV_Y", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    // light_shader_.set_vs_shader_from_memory(light_shader_source_, "VSMain", nullptr, nullptr);
    // light_shader_.set_gs_shader_from_memory(light_shader_source_, "GSMain", nullptr, nullptr);
    // light_shader_.set_input_layout(inputs, std::size(inputs));
#ifndef NDEBUG
    // light_shader_.set_name("scene_light_shader");
#endif

    std::string shadow_cascade_size = std::to_string(Light::shadow_cascade_count);
    std::string light_count = std::to_string(lights_.size());
    D3D_SHADER_MACRO macro[] = {
        "SHADOW_CASCADE_SIZE", shadow_cascade_size.c_str(),
        "LIGHT_COUNT", light_count.c_str(),
        nullptr, nullptr
    };
    generate_gbuffers_shader_.set_vs_shader_from_file("./resources/shaders/deferred/generate_gbuffers.hlsl", "VSMain", nullptr, nullptr);
    generate_gbuffers_shader_.set_ps_shader_from_file("./resources/shaders/deferred/generate_gbuffers.hlsl", "PSMain", nullptr, nullptr);
    generate_gbuffers_shader_.set_input_layout(inputs, std::size(inputs));
#ifndef NDEBUG
    generate_gbuffers_shader_.set_name("generate_gbuffers");
#endif

    assemble_gbuffers_shader_.set_vs_shader_from_file("./resources/shaders/deferred/assemble_gbuffers.hlsl", "VSMain", nullptr, nullptr);
    assemble_gbuffers_shader_.set_ps_shader_from_file("./resources/shaders/deferred/assemble_gbuffers.hlsl", "PSMain", nullptr, nullptr);

    CD3D11_RASTERIZER_DESC rast_desc = {};
    rast_desc.CullMode = D3D11_CULL_NONE;
    rast_desc.FillMode = D3D11_FILL_SOLID;
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

    D3D11_SAMPLER_DESC tex_sampler_desc{};
    tex_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    tex_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    tex_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    tex_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    tex_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    tex_sampler_desc.MinLOD = 0;
    tex_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
    D3D11_CHECK(device->CreateSamplerState(&tex_sampler_desc, &texture_sampler_state_));

    D3D11_SAMPLER_DESC depth_sampler_desc{};
    depth_sampler_desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    depth_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    depth_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    depth_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    depth_sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    depth_sampler_desc.MinLOD = 0;
    depth_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
    D3D11_CHECK(device->CreateSamplerState(&depth_sampler_desc, &depth_sampler_state_));

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

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    depth_stencil_desc.DepthEnable = true;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

    depth_stencil_desc.StencilEnable = false;
    depth_stencil_desc.StencilReadMask = 0;
    depth_stencil_desc.StencilWriteMask = 0;

    depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    D3D11_CHECK(device->CreateDepthStencilState(&depth_stencil_desc, &deferred_depth_state_));

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
    SAFE_RELEASE(light_rasterizer_state_);
    SAFE_RELEASE(rasterizer_state_);
    SAFE_RELEASE(texture_sampler_state_);
    SAFE_RELEASE(depth_sampler_state_);
    SAFE_RELEASE(deferred_depth_state_);
    generate_gbuffers_shader_.destroy();
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
    // generate G-Buffers
    {
        // restore default render target and depth stencil
        {
            context->OMSetRenderTargets(gbuffer_count_, deferred_gbuffers_target_view_, deferred_depth_target_view_);

            float clear_color[4] = { 0.f, 0.f, 0.f, 1.f };
            for (uint32_t i = 0; i < gbuffer_count_; ++i) {
                context->ClearRenderTargetView(deferred_gbuffers_target_view_[i], clear_color);
            }

            context->OMSetDepthStencilState(deferred_depth_state_, 0);
            context->ClearDepthStencilView(deferred_depth_target_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0xFF);

            RECT rc;
            GetWindowRect(Game::inst()->win().window(), &rc);
            D3D11_VIEWPORT viewport = {};
            viewport.Width = static_cast<float>(rc.right - rc.left);
            viewport.Height = static_cast<float>(rc.bottom - rc.top);
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.MinDepth = 0;
            viewport.MaxDepth = 1.0f;

            context->RSSetViewports(1, &viewport);
        }

        context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->RSSetState(rasterizer_state_);

        // draw models
        generate_gbuffers_shader_.use();
        uniform_data_.view_proj = camera->view_proj();
        uniform_buffer_.update_data(&uniform_data_);
        uniform_buffer_.bind(0);

        for (auto& model : models_) {
            model->draw();
        }
    }

    // lights pass
    {

    }

    // assemble gbuffers
    {
        // restore default render target and depth stencil
        Game::inst()->render().prepare_resources();
        context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->RSSetState(rasterizer_state_);

        assemble_gbuffers_shader_.use();
        context->PSSetShaderResources(0, gbuffer_count_, deferred_gbuffers_view_);
        context->PSSetShaderResources(gbuffer_count_, 1, &deferred_depth_view_);

        context->PSSetSamplers(0, 1, &texture_sampler_state_);
        context->PSSetSamplers(1, 1, &depth_sampler_state_);

        // restore pos from depth
        uniform_data_.view_proj = uniform_data_.view_proj.Invert();
        uniform_buffer_.update_data(&uniform_data_);
        uniform_buffer_.bind(0);

        context->Draw(3, 0);
    }
}
