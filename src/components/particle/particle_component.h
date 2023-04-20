#pragma once

#include <vector>

#include "core/game.h"
#include "component/game_component.h"

class Model;

class ParticleComponent : public GameComponent
{
private:
    class ParticleSystem* particle_system_{ nullptr };

    std::vector<class Light*> lights_;

    Model* plane_{ nullptr };
public:
    ParticleComponent();
    ~ParticleComponent();

    void initialize() override;
    void draw() override;
    void imgui() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
};
