#include "core/game.h"
#include "render/render.h"
#include "render/camera.h"

#include "light.h"

Light::Light() : data_{ Type::undefined }
{
    auto device = Game::inst()->render().device();
    auto context = Game::inst()->render().context();
    auto camera = Game::inst()->render().camera();

    // setup depth stencil array
    {
        for (uint32_t i = 0; i < shadow_cascade_count_; ++i) {
            ds_buffer_[i].initialize(512, 512, DXGI_FORMAT_R32_TYPELESS, nullptr, D3D11_BIND_FLAG(D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE));

            D3D11_DEPTH_STENCIL_VIEW_DESC ds_desc{};
            ds_desc.Format = DXGI_FORMAT_D32_FLOAT;
            ds_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            device->CreateDepthStencilView(ds_buffer_[i].resource(), &ds_desc, &(ds_view_[i]));
        }
    }
}

Light::~Light()
{
    if (ds_view_ != nullptr) {
        for (uint32_t i = 0; i < shadow_cascade_count_; ++i) {
            ds_view_[i]->Release();
        }
    }
    for (uint32_t i = 0; i < shadow_cascade_count_; ++i) {
        ds_buffer_[i].destroy();
    }
}

Light::LightData& Light::get_data()
{
    return data_;
}

void Light::set_type(Light::Type type)
{
    data_.type = type;
}

Light::Type Light::get_type() const
{
    return data_.type;
}

void Light::set_color(Vector3 color)
{
    data_.color = color;
}

Vector3 Light::get_color() const
{
    return data_.color;
}

void Light::setup_direction(Vector3 direction)
{
    assert(data_.type == Light::Type::direction);
    direction.Normalize(data_.direction);
}

void Light::setup_point(Vector3 position)
{
    assert(data_.type == Light::Type::point);
    data_.origin = position;
}

void Light::setup_spot(Vector3 origin, Vector3 direction, float angle)
{
    assert(data_.type == Light::Type::spot);
    data_.origin = origin;
    data_.direction = direction;
    data_.angle = angle;
}

void Light::setup_area()
{
    assert(data_.type == Light::Type::area);
    // TODO
}

uint32_t Light::get_depth_map_count() const
{
    return shadow_cascade_count_;
}

const Texture& Light::get_depth_buffer(uint32_t index)
{
    return ds_buffer_[index];
}

ID3D11DepthStencilView* Light::get_depth_map(uint32_t index)
{
    assert(index < shadow_cascade_count_);
    assert(ds_view_ != nullptr);
    return ds_view_[index];
}
