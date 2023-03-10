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
        Matrix view_proj;
        Matrix transform;
        Matrix inverse_transp_transform;
        Vector3 color;
        float angle;
    };

    class Sphere
    {
    public:
        Sphere(float radius, float distance, float speed, float local_speed, Vector3 color);
        void add_child(class Sphere* next);
        void clear();
        size_t child_count() const;
        Matrix transform() const;
        void draw(Buffer& sphere_index_buffer, ConstBuffer& sphere_info_buffer, const Matrix& view_proj);
        void update();
        void get_position(int& depth, Vector3& res);
    private:
        class Sphere* parent_;
        std::vector<class Sphere*> children_;
        float radius_;
        float distance_;
        float angle_;
        float angle_speed_;
        float local_angle_;
        float local_speed_;
        Vector3 color_;
    };
    Sphere* system_root_;

    int horizontal_segments_count_{ 10 };
    int vertical_segments_count_{ 10 };

    Buffer sphere_vertex_buffer_;
    Buffer sphere_index_buffer_;
    ConstBuffer sphere_info_buffer_;

    static std::string sphere_draw_shader_source_;
    Shader sphere_draw_shader_;

    ID3D11RasterizerState* rasterizer_state_{ nullptr };

    // imgui controls
    bool camera_perspective_;
    bool focus_on_;
    int focus_target_;
};
