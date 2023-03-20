#include <cassert>

#include <WICTextureLoader.h>
using namespace DirectX;

#include "core/game.h"
#include "render/render.h"
#include "render/d3d11_common.h"
#include "texture.h"

Texture::Texture()
{
}

void Texture::load(const std::string& path)
{
    // load texture data
    assert(!path.empty());
    assert(texture_ == nullptr);
    auto device = Game::inst()->render().device();
    auto context = Game::inst()->render().context();
    std::wstring filenamew(path.begin(), path.end());
    CreateWICTextureFromFile(device, filenamew.c_str(), (ID3D11Resource**)&texture_, &resource_view_);

    assert(texture_ != nullptr);
    assert(resource_view_ != nullptr);
}

void Texture::initialize(uint32_t width, uint32_t height, DXGI_FORMAT format, void* pixel_data)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Format = format;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = pixel_data;
    assert(format == DXGI_FORMAT_B8G8R8A8_UNORM);
    subresourceData.SysMemPitch = width * 4;
    subresourceData.SysMemSlicePitch = width * height * 4;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateTexture2D(&desc, &subresourceData, &texture_));
    D3D11_CHECK(device->CreateShaderResourceView(texture_, nullptr, &resource_view_));

    assert(texture_ != nullptr);
    assert(resource_view_ != nullptr);
}

void Texture::destroy()
{
    texture_->Release();
    texture_ = nullptr;

    resource_view_->Release();
    resource_view_ = nullptr;
}

void Texture::bind(UINT slot)
{
    auto context = Game::inst()->render().context();
    context->PSSetShaderResources(slot, 1, &resource_view_);
}

ID3D11Resource* Texture::resource() const
{
    return (ID3D11Resource*)texture_;
}
