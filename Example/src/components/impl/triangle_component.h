#pragma once

#include "components/common/game_component.h"
#include "render/resource/shader.h"
#include "render/resource/buffer.h"

class TriangleComponent : public GameComponent
{
private:
    constexpr static char* resource_path_{ "./resources/triangle_component/" };

    Shader shader_;

    Buffer vertex_buffer_;
    Buffer index_buffer_;

    ID3D11RasterizerState* rasterizer_state_{ nullptr };

public:
    TriangleComponent();
    ~TriangleComponent();

    void initialize() override;
    void draw() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
};
