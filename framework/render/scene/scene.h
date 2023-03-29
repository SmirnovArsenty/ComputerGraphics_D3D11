#pragma once

#include <vector>
#include <string>

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "light.h"

#include "render/resource/buffer.h"
#include "render/resource/shader.h"

class Scene
{
public:
    Scene();
    ~Scene();

    void initialize();
    void destroy();

    // TODO: load scene from xml description

    void add_model(class Model* model);

    void add_light(Light* light);

    void update();
    void draw();
private:
    std::vector<class Model*> models_;
    std::vector<Light*> lights_;

    ConstBuffer uniform_buffer_;
    struct
    {
        Matrix view_proj;
        Vector3 camera_pos;
        float time;
        Vector3 camera_dir;
        uint32_t lights_count;
    } uniform_data_;

    // generate gbuffers
    Shader generate_gbuffers_shader_;
    ID3D11RasterizerState* rasterizer_state_{ nullptr };

    // light pass
    Shader light_shader_;
    ConstBuffer light_data_buffer_;
    StructuredBuffer lights_buffer_;
    ID3D11RasterizerState* light_rasterizer_state_{ nullptr };
    ID3D11SamplerState* depth_sampler_state_{ nullptr };

    // assemble
    Shader assemble_gbuffers_shader_;
    ID3D11SamplerState* texture_sampler_state_{ nullptr };

    constexpr static uint32_t gbuffer_count_ = 4;
    // normal
    // diffuse
    // specular
    // ambient
    ID3D11Texture2D* deferred_gbuffers_[gbuffer_count_]{ nullptr };
    ID3D11ShaderResourceView* deferred_gbuffers_view_[gbuffer_count_]{ nullptr };
    ID3D11RenderTargetView* deferred_gbuffers_target_view_[gbuffer_count_]{ nullptr };
    // depth
    ID3D11DepthStencilState* deferred_depth_state_{ nullptr };
    ID3D11Texture2D* deferred_depth_buffer_{ nullptr };
    ID3D11ShaderResourceView* deferred_depth_view_{ nullptr };
    ID3D11DepthStencilView* deferred_depth_target_view_{ nullptr };
};
