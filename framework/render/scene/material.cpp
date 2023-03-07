#include "core/game.h"
#include "render/render.h"

#include "material.h"
#include "render/resource/texture.h"

Material::Material(const std::string& path) : path_{ path }
{
}

Material::~Material()
{
}

void Material::initialize()
{
    D3D11_SAMPLER_DESC sampler_desc;
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
    Game::inst()->render().device()->CreateSamplerState(&sampler_desc, &sampler_state_);
}

void Material::bind()
{
    auto context = Game::inst()->render().context();
    context->PSSetSamplers(0, 1, &sampler_state_);

    if (is_pbr()) {

    } else {
        diffuse_->bind(0);
        specular_->bind(1);
        ambient_->bind(2);
    }

}

bool Material::is_pbr()
{
    return base_color_ != nullptr;
}

void Material::destroy()
{
    sampler_state_->Release();
    sampler_state_ = nullptr;

#define DESTROY_MATERIAL_TYPE(material_type)    \
    if (material_type##_ != nullptr) {          \
        material_type##_->destroy();            \
        delete material_type##_;                \
        material_type##_ = nullptr;             \
    }

    MATERIALS(DESTROY_MATERIAL_TYPE)

#undef DESTROY_MATERIAL_TYPE
}

#define TEXTURE_TYPE_IMPL(texture_type) \
    void Material::set_##texture_type(Texture* texture) {   \
        texture_type##_ = texture;                          \
    }                                                       \
    const Texture* Material::get_##texture_type() const {   \
        return texture_type##_;                             \
    }

TEXTURE_TYPE_IMPL(diffuse)
TEXTURE_TYPE_IMPL(specular)
TEXTURE_TYPE_IMPL(ambient)
TEXTURE_TYPE_IMPL(emissive)
TEXTURE_TYPE_IMPL(height)
TEXTURE_TYPE_IMPL(normal)
TEXTURE_TYPE_IMPL(shininess)
TEXTURE_TYPE_IMPL(opacity)
TEXTURE_TYPE_IMPL(displacement)
TEXTURE_TYPE_IMPL(lightmap)
TEXTURE_TYPE_IMPL(reflection)
TEXTURE_TYPE_IMPL(base_color)
TEXTURE_TYPE_IMPL(normal_camera)
TEXTURE_TYPE_IMPL(emission_color)
TEXTURE_TYPE_IMPL(metalness)
TEXTURE_TYPE_IMPL(diffuse_roughness)
TEXTURE_TYPE_IMPL(ambient_occlusion)

#undef TEXTURE_TYPE_IMPL
