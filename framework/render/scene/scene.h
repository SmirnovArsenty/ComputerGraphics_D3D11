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
    Shader shader_;
    static std::string shader_source_;

    ConstBuffer light_data_buffer_;
    Shader light_shader_;
    static std::string light_shader_source_;

    Buffer lights_buffer_;

    ID3D11RasterizerState* rasterizer_state_{ nullptr };

    ID3D11RenderTargetView* render_target_view_{};
};
