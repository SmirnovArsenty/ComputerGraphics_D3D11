#include "core/game.h"
#include "render/render.h"
#include "render/camera.h"
#include "render/d3d11_common.h"

#include "light.h"

Light::Light() : data_{ Type::undefined }
{
    auto device = Game::inst()->render().device();
    auto context = Game::inst()->render().context();
    auto camera = Game::inst()->render().camera();

    // setup depth stencil array
    {
        D3D11_TEXTURE2D_DESC depth_desc{};
        depth_desc.Width = 512;
        depth_desc.Height = 512;
        depth_desc.Format = DXGI_FORMAT_R32_TYPELESS;
        depth_desc.ArraySize = shadow_cascade_count;
        depth_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
        depth_desc.MipLevels = 1;
        depth_desc.SampleDesc.Count = 1;
        depth_desc.SampleDesc.Quality = 0;
        D3D11_CHECK(device->CreateTexture2D(&depth_desc, nullptr, &ds_buffer_));

        D3D11_DEPTH_STENCIL_VIEW_DESC ds_desc{};
        ds_desc.Format = DXGI_FORMAT_D32_FLOAT;
        ds_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        ds_desc.Texture2DArray.MipSlice = 0;
        ds_desc.Texture2DArray.FirstArraySlice = 0;
        ds_desc.Texture2DArray.ArraySize = shadow_cascade_count;
        D3D11_CHECK(device->CreateDepthStencilView(ds_buffer_, &ds_desc, &ds_view_));

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srv_desc.Texture2DArray.MostDetailedMip = 0;
        srv_desc.Texture2DArray.MipLevels = 1;
        srv_desc.Texture2DArray.FirstArraySlice = 0;
        srv_desc.Texture2DArray.ArraySize = 3;
        D3D11_CHECK(device->CreateShaderResourceView(ds_buffer_, &srv_desc, &ds_buffer_view_));
    }
}

Light::~Light()
{
    if (ds_buffer_view_ != nullptr) {
        ds_buffer_view_->Release();
        ds_buffer_view_ = nullptr;
    }
    if (ds_view_ != nullptr) {
        ds_view_->Release();
        ds_view_ = nullptr;
    }
    if (ds_buffer_ != nullptr) {
        ds_buffer_->Release();
        ds_buffer_ = nullptr;
    }
}

Light::LightData& Light::get_data()
{
    return data_;
}

Light::CascadeData& Light::get_cascade_data()
{
    return cascade_data_;
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

ID3D11Texture2D* Light::get_depth_buffer()
{
    return ds_buffer_;
}

ID3D11ShaderResourceView* Light::get_depth_view()
{
    return ds_buffer_view_;
}

ID3D11DepthStencilView* Light::get_depth_map()
{
    assert(ds_view_ != nullptr);
    return ds_view_;
}

void Light::set_transform(uint32_t index, Matrix transform, float distance)
{
    cascade_data_.transform[index] = transform;
    if (index == 0) {
        cascade_data_.distances.x = distance;
    } else if (index == 1) {
        cascade_data_.distances.y = distance;
    } else if (index == 2) {
        cascade_data_.distances.z = distance;
    } else if (index == 3) {
        cascade_data_.distances.w = distance;
    }
}

Matrix Light::get_transform(uint32_t index)
{
    return cascade_data_.transform[index];
}
