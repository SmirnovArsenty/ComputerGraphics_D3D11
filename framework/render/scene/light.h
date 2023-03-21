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

    Light();
    ~Light();

    LightData& get_data();

    void set_type(Type type);
    Type get_type() const;

    void set_color(Vector3 color);
    Vector3 get_color() const;

    void setup_direction(Vector3 direction);
    void setup_point(Vector3 position);
    void setup_spot(Vector3 origin, Vector3 direction, float angle);
    void setup_area(); // TODO

    uint32_t get_depth_map_count() const;
    const Texture& get_depth_buffer(uint32_t index);
    ID3D11DepthStencilView* get_depth_map(uint32_t index);

private:
    constexpr static uint32_t shadow_cascade_count_ = 3;

    LightData data_;

    Texture ds_buffer_[shadow_cascade_count_];
    ID3D11DepthStencilView* ds_view_[shadow_cascade_count_ ]{ nullptr };
};
