#pragma once

#include <cstdint>
#include <memory>

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;
#include <d3d11.h>

#include "component/game_component.h"

#include "render/resource/texture.h"
#include "render/resource/shader.h"
#include "render/resource/buffer.h"

class Light : public GameComponent
{
public:
    constexpr static uint32_t shadow_cascade_count = 3;
    constexpr static uint32_t shadow_map_resolution = 2048;

    enum class Type : uint32_t
    {
        undefined = 0,
        direction,
        point,
        spot,
        area,
    };

    virtual ~Light() = default;

protected:
    Light() = default;
};

class AmbientLight : public Light
{
public:
    AmbientLight(Vector3 color);

    void initialize() override;
    void draw() override;
    void imgui() override {};
    void reload() override {};
    void update() override;
    void destroy_resources() override;
private:
    struct {
        Vector4 color;
    } ambient_data_;
    ConstBuffer ambient_buffer_;
    Vector3 color_;

    Shader* shader_;
};

class DirectionLight : public Light
{
public:
    struct LightData {
        Light::Type type;
        Vector3 color;

        Vector3 origin;
        float angle;

        Vector3 direction;
        float dummy;

        Matrix transform[Light::shadow_cascade_count];
        Vector4 distances;
    };

    DirectionLight(const Vector3& color, const Vector3& direction);

    void initialize() override;
    void draw() override;
    void imgui() override {};
    void reload() override {};
    void update() override;
    void destroy_resources() override;

    void set_color(const Vector3& color);
    void set_direction(const Vector3& direction);

private:
    struct
    {
        Vector4 color;
        Vector4 direction;
    } direction_data_;
    ConstBuffer direction_buffer_;

    Vector3 color_;
    Vector3 direction_;

    ID3D11Texture2D* ds_buffer_{ nullptr };
    ID3D11ShaderResourceView* ds_buffer_view_{ nullptr };
    ID3D11DepthStencilView* ds_view_{ nullptr };

    // deferred
    ID3D11DepthStencilState* ds_state_{ nullptr };
    ID3D11BlendState* blend_state_{ nullptr };

    Shader* shader_;
};

class PointLight : public Light
{
public:
    PointLight(const Vector3& color, const Vector3& position, float radius);

    void initialize() override;
    void draw() override;
    void imgui() override {};
    void reload() override {};
    void update() override;
    void destroy_resources() override;

private:
    struct {
        Matrix transform;
        Vector4 color;
        Vector4 position_radius;
    } point_data_;
    ConstBuffer point_buffer_;

    Vector3 color_;
    Vector3 position_;
    float radius_;

    std::vector<Vector3> vertices_;
    std::vector<uint32_t> indices_;
    Buffer vertex_buffer_;
    Buffer index_buffer_;

    ID3D11RasterizerState* rasterizer_state_{ nullptr };

    Shader* shader_;
};
