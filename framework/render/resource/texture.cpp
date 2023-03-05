#include "core/game.h"
#include "render/render.h"
#include "texture.h"

Texture::Texture(const std::string& path) : path_{ path }
{
}

void Texture::initialize(uint32_t width, uint32_t height)
{
    texture_desc_.Width = width;
    texture_desc_.Height = height;
    texture_desc_.MipLevels = 0;
    texture_desc_.ArraySize = 1;
    texture_desc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // set format
    texture_desc_.SampleDesc.Count = 1;
    texture_desc_.SampleDesc.Quality = 0;
    texture_desc_.Usage = D3D11_USAGE_DEFAULT;
    texture_desc_.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texture_desc_.CPUAccessFlags = 0;
    texture_desc_.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    D3D11_SUBRESOURCE_DATA subresource_data;
    subresource_data.pSysMem = 0; // pixels
    subresource_data.SysMemPitch = 0; // row pitch
    subresource_data.SysMemSlicePitch = 0; // image size
    Game::inst()->render().device()->CreateTexture2D(&texture_desc_, &subresource_data, &texture_);
}

void Texture::destroy()
{

}
