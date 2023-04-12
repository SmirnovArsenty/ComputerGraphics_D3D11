#include "core/game.h"
#include "render/render.h"

#include "render/scene/light.h"

std::unique_ptr<Shader> AmbientLight::shader_{ nullptr };

AmbientLight::AmbientLight(Vector3 color) : color_{ color }
{
    if (shader_.get() == nullptr) {
        // initialize shader_
        shader_ = std::make_unique<Shader>();
        shader_->set_vs_shader_from_file("./resources/shaders/deferred/light_pass/ambient.hlsl", "VSMain", nullptr, nullptr);
        shader_->set_ps_shader_from_file("./resources/shaders/deferred/light_pass/ambient.hlsl", "PSMain", nullptr, nullptr);
#ifndef NDEBUG
        shader_->set_name("ambient");
#endif
    }
}

void AmbientLight::initialize()
{
    ambient_buffer_.initialize(sizeof(ambient_data_), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void AmbientLight::destroy_resources()
{
    ambient_buffer_.destroy();
}

void AmbientLight::update()
{
    ambient_data_.color.x = color_.x;
    ambient_data_.color.y = color_.y;
    ambient_data_.color.z = color_.z;
    ambient_buffer_.update_data(&ambient_data_);
}

void AmbientLight::draw()
{
    assert(shader_ != nullptr);

    shader_->use();

    ambient_buffer_.bind(1U);

    Game::inst()->render().context()->Draw(3, 0);
}
