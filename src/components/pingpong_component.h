#pragma once

#include <glm/glm.hpp>

#include "component/game_component.h"
#include "render/resource/shader.h"
#include "render/resource/buffer.h"

class PingpongComponent : public GameComponent
{
private:
    static std::string brick_shader_source_;
    static std::string circle_shader_source_;

    // game info
    struct BrickInfo
    {
        glm::vec2 position{ 0.f, 0.f }; // first - setup padding (const), second - setup vertical position [0 + width / 2; 1 - width / 2]

        // relative to window size
        float width;
        float height;
    };

    struct CircleInfo
    {
        glm::vec2 position{ 0.f, 0.f };
        float radius;
        uint32_t triangle_count;
    } circle_, default_circle_;
    glm::vec2 circle_move_direction_{ 0.f, 0.f }, default_circle_move_direction_{ 0.f, 0.f };

    BrickInfo player_, default_player_;
    BrickInfo opponent_, default_opponent_;
    std::pair<uint32_t, uint32_t> score_; // player points; opponent points
    bool paused_{ false }; // 'P'
    bool self_mode_{ true }; // 'S'

    // resources
    Buffer brick_index_buffer_;
    Buffer circle_index_buffer_;

    ConstBuffer player_brick_info_buffer_;
    ConstBuffer opponent_brick_info_buffer_;
    ConstBuffer circle_info_buffer_;

    Shader brick_shader_;
    Shader circle_shader_;

    ID3D11RasterizerState* rasterizer_state_{ nullptr };
public:
    PingpongComponent();

    void initialize() override;
    void draw() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
};