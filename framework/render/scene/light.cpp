#include "light.h"

Light::Light() : type_{ Type::undefined }
{
}

Light::~Light()
{
}

void Light::set_type(Light::Type type)
{
    type_ = type;
}

Light::Type Light::get_type() const
{
    return type_;
}

void Light::set_color(Vector3 color)
{
    color_ = color;
}

Vector3 Light::get_color() const
{
    return color_;
}

void Light::setup_direction(Vector3 direction)
{
    assert(type_ == Light::Type::direction);
    direction.Normalize(direction_);
}

void Light::setup_point(Vector3 position)
{
    assert(type_ == Light::Type::point);
    origin_ = position;
}

void Light::setup_spot(Vector3 origin, Vector3 direction, float angle)
{
    assert(type_ == Light::Type::spot);
    origin_ = origin;
    direction_ = direction;
    angle_ = angle;
}

void Light::setup_area()
{
    assert(type_ == Light::Type::area);
    // TODO
}
