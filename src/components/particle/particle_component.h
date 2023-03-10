#pragma once

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "render/resource/shader.h"
#include "render/resource/buffer.h"
#include "component/game_component.h"

class ParticleComponent : public GameComponent
{
private:
    struct Particle
    {
        Vector2 size;
        float rotation_rate;
        float scale_rate;
        float alpha_rate;
    };

    struct Emitter
    {
        Vector3 position;
        Vector3 direction;
        float rate;
    };

    Shader particle_shader_;
public:
    ParticleComponent(Vector3 position, float rate);

    void initialize() override;
    void draw() override;
    void imgui() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
};
