#pragma once

#include <cstdint>

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;
#include <d3d11.h>

#include "render/resource/texture.h"
#include "render/resource/shader.h"
#include "render/resource/buffer.h"

class Light
{
public:
    constexpr static uint32_t shadow_cascade_count = 3;

    enum class Type : uint32_t
    {
        undefined = 0,
        direction,
        point,
        spot,
        area,
    };

    struct LightData {
        Type type;
        Vector3 color;

        Vector3 origin;
        float angle;

        Vector3 direction;
        float dummy;
    };

    struct CascadeData {
        Matrix transform[shadow_cascade_count];
        Vector4 distances;
    };

    Light();
    ~Light();

    LightData& get_data();
    CascadeData& get_cascade_data();

    void set_type(Type type);
    Type get_type() const;

    void set_color(Vector3 color);
    Vector3 get_color() const;

    void setup_direction(Vector3 direction);
    void setup_point(Vector3 position);
    void setup_spot(Vector3 origin, Vector3 direction, float angle);
    void setup_area(); // TODO

    ID3D11Texture2D* get_depth_buffer();
    ID3D11ShaderResourceView* get_depth_view();
    ID3D11DepthStencilView* get_depth_map();
    void set_transform(uint32_t index, Matrix transform, float distance);
    Matrix get_transform(uint32_t index);
private:
    LightData data_;
    CascadeData cascade_data_;

    ID3D11Texture2D* ds_buffer_{ nullptr };
    ID3D11ShaderResourceView* ds_buffer_view_{ nullptr };
    ID3D11DepthStencilView* ds_view_{ nullptr };
};
