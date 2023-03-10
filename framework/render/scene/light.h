#pragma once

#include <cstdint>

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

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
    

    Light();
    ~Light();

    void set_type(Type type);
    Type get_type() const;

    void set_color(Vector3 color);
    Vector3 get_color() const;

    void setup_direction(Vector3 direction);
    void setup_point(Vector3 position);
    void setup_spot(Vector3 origin, Vector3 direction, float angle);
    void setup_area(); // TODO

private:
    Type type_;
    Vector3 color_;
    // 4 bytes

    Vector3 origin_;
    float angle_;
    // 4 bytes
    Vector3 direction_;
    float dummy_;
};
