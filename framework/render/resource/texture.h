#pragma once

#include <string>

class Texture
{
public:
    Texture(const std::string& path);

    inline const std::string& path() const { return path_; }

private:
    std::string path_;
};
