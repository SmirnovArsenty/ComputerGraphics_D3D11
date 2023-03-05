#pragma once

#include <string>

class Texture
{
public:
    Texture(const std::string& path);

    inline const std::string& path() const { return path_; }

    void initialize(uint32_t width, uint32_t height);
    void destroy();

private:
    std::string path_;

    D3D11_TEXTURE2D_DESC texture_desc_;
    ID3D11Texture2D* texture_;
};
