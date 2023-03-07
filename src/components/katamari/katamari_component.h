#pragma once

#include <vector>
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "component/game_component.h"
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
    class Scene* scene_{ nullptr };
};
