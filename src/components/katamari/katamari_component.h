#pragma once

#include <vector>
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "component/game_component.h"

#include "render/scene/model.h"
#include "render/scene/light.h"

#include "render/resource/buffer.h"
#include "render/resource/shader.h"

class KatamariComponent : public GameComponent
{
public:
    KatamariComponent();
    ~KatamariComponent() = default;

    void initialize() override;
    void draw() override;
    void imgui() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
private:
    struct AttachedEntity
    {
        Model* model;
        Vector3 attach_position;
        Quaternion initial_rotation;
    };
    std::vector<AttachedEntity> attached_models_;
    std::vector<Model*> free_models_;
    std::vector<Light*> lights_;
    Model* plane_{ nullptr };

    float radius_a_{ 0.f };
    float radius_b_{ 0.f };
    float radius_t_{ 0.f };
};
