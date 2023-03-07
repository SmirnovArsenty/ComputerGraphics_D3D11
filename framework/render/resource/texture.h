#pragma once

#include <string>
#include <dxgiformat.h>
#include <d3d11.h>

class Texture
{
public:
    Texture();

    void load(const std::string& path);
    void initialize(uint32_t width, uint32_t height, DXGI_FORMAT format, void* pixel_data);
    void destroy();

    void bind(UINT slot);

private:
    ID3D11Texture2D* texture_{ nullptr };
    ID3D11ShaderResourceView* resource_view_{ nullptr };
};
