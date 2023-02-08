#pragma once

#include "game_component.h"

class TriangleComponent : public GameComponent
{
private:
    Shader shader_;

    ID3D11Buffer* vertex_buffer_{ nullptr };
    ID3D11Buffer* index_buffer_{ nullptr };

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
