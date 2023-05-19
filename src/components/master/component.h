#pragma once

#include <vector>
#include <memory>

#include "core/game.h"
#include "component/game_component.h"

class Model;
class ParticleSystem;

class GraphicsShader;
class ComputeShader;

class Buffer;
class ConstBuffer;

class MasterComponent : public GameComponent
{
private:
    ParticleSystem* particle_system_{ nullptr };

    std::vector<class Light*> lights_;

    Model* plane_{ nullptr };

    std::unique_ptr<ComputeShader> init_bvh_{};
public:
    MasterComponent();
    ~MasterComponent();

    void initialize() override;
    void draw() override;
    void imgui() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
};
