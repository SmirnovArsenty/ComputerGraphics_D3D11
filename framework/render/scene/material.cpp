#include "material.h"
#include "render/resource/texture.h"

Material::Material(const std::string& path) : path_{ path }
{
}

void Material::set_albedo(const Texture* albedo)
{
    albedo_ = albedo;
}

const Texture* Material::get_albedo() const
{
    return albedo_;
}

void Material::set_normal(const Texture* normal)
{
    normal_ = normal;
}

const Texture* Material::get_normal() const
{
    return normal_;
}

void Material::set_metallic(const Texture* metallic)
{
    metallic_ = metallic;
}

const Texture* Material::get_metallic() const
{
    return metallic_;
}

void Material::set_roughness(const Texture* roughness)
{
    roughness_ = roughness;
}

const Texture* Material::get_roughness() const
{
    return roughness_;
}

void Material::set_diffuse(const Texture* diffuse)
{
    diffuse_ = diffuse;
}

const Texture* Material::get_diffuse() const
{
    return diffuse_;
}
void Material::set_specular(const Texture* specular)
{
    specular_ = specular;
}

const Texture* Material::get_specular() const
{
    return specular_;
}
