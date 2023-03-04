#pragma once

#include <string>

class Material
{
public:
    Material(const std::string& path);

    void set_albedo(const class Texture*);
    const class Texture* get_albedo() const;

    void set_normal(const class Texture*);
    const class Texture* get_normal() const;

    void set_metallic(const class Texture*);
    const class Texture* get_metallic() const;

    void set_roughness(const class Texture*);
    const class Texture* get_roughness() const;

    void set_diffuse(const class Texture*);
    const class Texture* get_diffuse() const;

    void set_specular(const class Texture*);
    const class Texture* get_specular() const;

private:
    std::string path_;

    const class Texture* albedo_;
    const class Texture* normal_;

    const class Texture* metallic_;
    const class Texture* roughness_;

    const class Texture* diffuse_;
    const class Texture* specular_;
};
