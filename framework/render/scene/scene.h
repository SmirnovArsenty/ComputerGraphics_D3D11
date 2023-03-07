#pragma once

#include <vector>
#include <string>

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "render/resource/buffer.h"
#include "render/resource/shader.h"
class Scene
{
public:
    Scene();
    ~Scene();

    void initialize();
    void destroy();

    void add_model(const std::string& filename);

    void update();
    void draw();
private:
    std::vector<class Model*> models_;

    ConstBuffer uniform_buffer_;
    struct
    {
        Matrix view_proj;
        Vector3 camera_pos;
        float time;
        Vector3 camera_dir;
        float dummy;
    } uniform_data_;
    Shader shader_;
    static std::string shader_source_;

    ID3D11RasterizerState* rasterizer_state_{ nullptr };
};
