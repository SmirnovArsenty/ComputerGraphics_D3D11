#pragma once

#include <vector>
#include <string>
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "component/game_component.h"
#include "render/resource/buffer.h"
#include "render/resource/shader.h"

class OrbitComponent : public GameComponent
{
public:
    OrbitComponent();

    void initialize() override;
    void draw() override;
    void imgui() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
private:
    struct SphereInfo
    {
        Matrix transform;
        Vector3 color;
    };

    class Sphere
    {
    public:
        Sphere(float radius, float distance, Vector3 color);
        void add_child(class Sphere* next);
        void clear();
        Quaternion rotation() const; // calculate all rotations
        void draw();
    private:
        class Sphere* parent_;
        std::vector<class Sphere*> children_;
        float radius_;
        float distance_;
        Vector3 color_;
        Quaternion local_rotation_;
    };
    Sphere* system_root_;

    Buffer sphere_vertex_buffer_;
    Buffer sphere_index_buffer_;
    ConstBuffer sphere_info_buffer_;

    static std::string sphere_draw_shader_source_;
    Shader sphere_draw_shader_;

    ID3D11RasterizerState* rasterizer_state_{ nullptr };
};
