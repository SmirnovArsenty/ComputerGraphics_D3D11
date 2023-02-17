#include <imgui/imgui.h>
#include "core/game.h"
#include "render/render.h"
#include "win32/win.h"
#include "win32/input.h"
#include "pingpong_component.h"
#include "render/d3d11_common.h"

PingpongComponent::PingpongComponent()
{
    // setup game data
    default_player_.position = Vector2(-1.f + 0.02f, .0f);
    default_player_.width = 0.3f;
    default_player_.height = 0.02f;

    default_opponent_.position = Vector2(1.f - 0.02f, .0f);
    default_opponent_.width = 0.3f;
    default_opponent_.height = 0.02f;

    default_circle_.triangle_count = 10;
    default_circle_.position = Vector2(0.f, 0.f);
    default_circle_.radius = 0.02f;

    default_circle_move_direction_ = Vector2(1.f, 0.f); // initially moves to opponent
}

void PingpongComponent::initialize()
{
    reload();

    // setup shaders
    net_shader_.set_vs_shader_from_memory(net_shader_source_, "VSMain", nullptr, nullptr);
    net_shader_.set_ps_shader_from_memory(net_shader_source_, "PSMain", nullptr, nullptr);
#ifndef NDEBUG
    net_shader_.set_name("net_shader");
#endif

    brick_shader_.set_vs_shader_from_memory(brick_shader_source_, "VSMain", nullptr, nullptr);
    brick_shader_.set_ps_shader_from_memory(brick_shader_source_, "PSMain", nullptr, nullptr);

#ifndef NDEBUG
    brick_shader_.set_name("brick_shader");
#endif

    circle_shader_.set_vs_shader_from_memory(circle_shader_source_, "VSMain", nullptr, nullptr);
    circle_shader_.set_ps_shader_from_memory(circle_shader_source_, "PSMain", nullptr, nullptr);
#ifndef NDEBUG
    circle_shader_.set_name("circle_shader");
#endif
    // input layout is not required: only index buffer used

    std::vector<uint16_t> brick_index_data = { 0,1,2, 0,1,3 };
    brick_index_buffer_.initialize(D3D11_BIND_INDEX_BUFFER, brick_index_data.data(), sizeof(brick_index_data[0]), UINT(std::size(brick_index_data)));
#ifndef NDEBUG
    brick_index_buffer_.set_name("brick_index_buffer");
#endif

    std::vector<uint16_t> circle_index_data; // = { 0,1,2, 0,2,3 };
    for (uint16_t i = 0; i < max_triangle_count_; ++i)
    {
        circle_index_data.push_back(0); // first index is 0 - circle center
        circle_index_data.push_back(i + 1);
        circle_index_data.push_back(i + 2);
    }
    circle_index_data.back() = 1; // link with first triangle
    circle_index_buffer_.initialize(D3D11_BIND_INDEX_BUFFER, circle_index_data.data(), sizeof(circle_index_data[0]), UINT(circle_index_data.size()));

#ifndef NDEBUG
    circle_index_buffer_.set_name("circle_index_buffer");
#endif

    player_brick_info_buffer_.initialize(sizeof(BrickInfo), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    opponent_brick_info_buffer_.initialize(sizeof(BrickInfo), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    circle_info_buffer_.initialize(sizeof(CircleInfo), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    net_info_buffer_.initialize(sizeof(NetInfo), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

#ifndef NDEBUG
    player_brick_info_buffer_.set_name("player_uniform_buffer");
    opponent_brick_info_buffer_.set_name("opponent_uniform_buffer");
    circle_info_buffer_.set_name("circle_uniform_buffer");
#endif

    net_info_buffer_.update_data(&net_info_);

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateRasterizerState(&rastDesc, &rasterizer_state_));

#ifndef NDEBUG
    std::string rasterizer_state_name = "pingpong_rasterizer_state";
    rasterizer_state_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(rasterizer_state_name.size()), rasterizer_state_name.c_str());
#endif
}

void PingpongComponent::draw()
{
    auto context = Game::inst()->render().context();

    context->RSSetState(rasterizer_state_);

    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw net
    net_shader_.use();
    brick_index_buffer_.bind();
    net_info_buffer_.bind(0);
    context->DrawIndexedInstanced(brick_index_buffer_.count(), net_info_.net_elements_count, 0, 0, 0);

    // draw bricks
    brick_shader_.use();
    brick_index_buffer_.bind();

    // player brick
    player_brick_info_buffer_.update_data(&player_);
    player_brick_info_buffer_.bind(0);
    context->DrawIndexed(brick_index_buffer_.count(), 0, 0);

    // opponent brick
    opponent_brick_info_buffer_.update_data(&opponent_);
    opponent_brick_info_buffer_.bind(0);
    context->DrawIndexed(brick_index_buffer_.count(), 0, 0);

    // draw circle
    circle_shader_.use();
    circle_index_buffer_.bind();
    circle_info_buffer_.update_data(&circle_);
    circle_info_buffer_.bind(0);
    context->DrawIndexed(circle_index_buffer_.count(), 0, 0);
}

void PingpongComponent::imgui()
{
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();

    uint32_t window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoCollapse;

    constexpr uint32_t gamers_window_width = 160;

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x / 4 - gamers_window_width, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(gamers_window_width, 85), ImGuiCond_FirstUseEver);
    ImGui::Begin("Player", nullptr, window_flags);
    {
        ImGui::Text("Score");
        ImGui::SameLine();
        ImGui::Text("%d", score_.first);

        ImGui::Text("Size");
        ImGui::SameLine();
        ImGui::Text("%.2fx%.2f", player_.width, player_.height);

        ImGui::Text("Position");
        ImGui::SameLine();
        ImGui::Text("%.2fx%.2f", player_.position.x, player_.position.y);
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x * 3 / 4, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(gamers_window_width, 85), ImGuiCond_FirstUseEver);
    ImGui::Begin("Opponent", nullptr, window_flags);
    {
        ImGui::Text("Score");
        ImGui::SameLine();
        ImGui::Text("%d", score_.second);

        ImGui::Text("Size");
        ImGui::SameLine();
        ImGui::Text("%.2fx%.2f", opponent_.width, opponent_.height);

        ImGui::Text("Position");
        ImGui::SameLine();
        ImGui::Text("%.2fx%.2f", opponent_.position.x, opponent_.position.y);
    }
    ImGui::End();

    // show circle info
    constexpr uint32_t common_info_window_width = 300;
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x / 2 - common_info_window_width / 2, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(common_info_window_width, 125), ImGuiCond_FirstUseEver);
    ImGui::Begin("Circle", nullptr, window_flags);
    {
        ImGui::Text("Radius");
        ImGui::SameLine();
        ImGui::Text("%.2f", circle_.radius);

        ImGui::Text("Position");
        ImGui::SameLine();
        ImGui::Text("%.2fx%.2f", circle_.position.x, circle_.position.y);

        ImGui::Text("Triangle count");
        ImGui::SameLine();
        ImGui::SliderInt("triangle_count", &circle_.triangle_count, 3, max_triangle_count_);

        ImGui::Text("Direction");
        ImGui::SameLine();
        ImGui::Text("%.2fx%.2f", circle_move_direction_.x, circle_move_direction_.y);

        ImGui::Text("Speed");
        ImGui::SameLine();
        ImGui::Text("%.2f", circle_move_speed_);
    }
    ImGui::End();
}

void PingpongComponent::reload()
{
    // setup game components
    player_ = default_player_;
    opponent_ = default_opponent_;
    int32_t triangle_count_save = circle_.triangle_count;
    circle_ = default_circle_;
    circle_.triangle_count = triangle_count_save;
    circle_move_direction_ = default_circle_move_direction_;
    circle_move_speed_ = default_circle_move_speed_;
}

void PingpongComponent::update()
{
    const auto& keyboard = Game::inst()->win().input()->keyboard();
    const float brick_move_delta = Game::inst()->delta_time() * 1e0f * 1.3f;
    const float circle_move_delta = Game::inst()->delta_time() * 1e0f * 2.f;

    // toggle pause
    if (keyboard.p.released) {
        paused_ = !paused_;
    }

    if (paused_) {
        return;
    }

    // toggle (AI vs AI) and (user vs AI) modes
    if (keyboard.s.released) {
        self_mode_ = !self_mode_;
    }

    // handle circle fly
    {
        circle_.position += circle_move_direction_ * circle_move_speed_ * circle_move_delta;
    }

    // handle AI
    {
        float target_y = circle_.position.y;
        if (circle_move_direction_.x > 0) { // to opponent
            target_y = (circle_move_direction_.y / circle_move_direction_.x) *
                             (opponent_.position.x - circle_.position.x) +
                             circle_.position.y;
        }
        while (target_y > 1.f || target_y < -1.f) {
            if (target_y > 1.f) {
                target_y = 2 - target_y;
            }
            if (target_y < -1.f) {
                target_y = -2 - target_y;
            }
        }
        // target_y = circle_.position.y;
        float delta_move = target_y - opponent_.position.y;
        if (abs(delta_move) > 0.02f) {
            delta_move = delta_move / abs(delta_move);
        }
        opponent_.position.y += delta_move * brick_move_delta;

        // clamp opponent brick position
        const float half_width = opponent_.width / 2.f;
        if (opponent_.position.y < -1.f + half_width)
        {
            opponent_.position.y = -1.f + half_width;
        }
        if (opponent_.position.y > 1.f - half_width)
        {
            opponent_.position.y = 1.f - half_width;
        }
    }

    // handle user input
    if (!self_mode_)
    {
        player_.position.y += brick_move_delta * keyboard.up.pressed;
        player_.position.y -= brick_move_delta * keyboard.down.pressed;

        // clamp player brick position
        const float half_width = player_.width / 2.f;
        if (player_.position.y < -1.f + half_width)
        {
            player_.position.y = -1.f + half_width;
        }
        if (player_.position.y > 1.f - half_width)
        {
            player_.position.y = 1.f - half_width;
        }
    }
    else // AI as player
    {
        float target_y = circle_.position.y;
        if (circle_move_direction_.x < 0) { // to player
            target_y = (circle_move_direction_.y / circle_move_direction_.x) *
                        (player_.position.x - circle_.position.x) +
                        circle_.position.y;
        }
        while (target_y > 1.f || target_y < -1.f) {
            if (target_y > 1.f) {
                target_y = 2 - target_y;
            }
            if (target_y < -1.f) {
                target_y = -2 - target_y;
            }
        }

        float delta_move = target_y - player_.position.y;
        if (abs(delta_move) > 0.02f) {
            delta_move = delta_move / abs(delta_move);
        }
        player_.position.y += delta_move * brick_move_delta;

        // clamp opponent brick position
        const float half_width = player_.width / 2.f;
        if (player_.position.y < -1.f + half_width)
        {
            player_.position.y = -1.f + half_width;
        }
        if (player_.position.y > 1.f - half_width)
        {
            player_.position.y = 1.f - half_width;
        }
    }

    // check intersections
    {
        // horizontal
        {
            const float circle_tg = circle_move_direction_.y / circle_move_direction_.x;
            if (circle_move_direction_.x > 0) // move to AI
            {
                if (circle_.position.x > opponent_.position.x - opponent_.height / 2 - circle_.radius)
                {
                    float intersect_y = circle_tg * ((opponent_.position.x - opponent_.height / 2) - circle_.position.x) + circle_.position.y;
                    float delta = opponent_.width / 2 + circle_.radius;
                    if (abs(intersect_y - opponent_.position.y) < delta)
                    { // brick hit
                        // reflect circle
                        srand(static_cast<unsigned>(Game::inst()->delta_time() * 1e9f));
                        circle_move_direction_.x *= (-1.f * rand() / RAND_MAX);
                        // circle_move_direction_.y = (intersect_y - opponent_.position.y) / opponent_.width;
                        circle_move_direction_.y = (2.f * rand() / RAND_MAX - 1.f) * .5f;
                        if (circle_move_direction_.x > -0.2f)
                        {
                            circle_move_direction_.x = -0.2f;
                        }
                        circle_move_direction_.Normalize();

                        // reduce size by every hit
                        opponent_.width *= 0.95f;
                        if (opponent_.width < 0.04f) {
                            opponent_.width = 0.04f;
                        }
                        circle_move_speed_ *= 1.01f;
                    } else { // miss
                        // add point to player
                        score_.first++;
                        reload();
                    }
                }
            }
            else // move to player
            {
                if (circle_.position.x < player_.position.x + player_.height / 2 + circle_.radius) {
                    float intersect_y = circle_tg * ((player_.position.x + player_.height / 2) - circle_.position.x) + circle_.position.y;
                    float delta = player_.width / 2 + circle_.radius;
                    if (abs(intersect_y - player_.position.y) < delta)
                    { // brick hit
                        srand(static_cast<unsigned>(Game::inst()->delta_time() * 1e9f));
                        circle_move_direction_.x *= (1.f * rand() / RAND_MAX);
                        // circle_move_direction_.y = -(intersect_y - player_.position.y) / player_.width;
                        circle_move_direction_.y = (2.f * rand() / RAND_MAX - 1.f) * .5f;

                        if (circle_move_direction_.x < 0.2f)
                        {
                            circle_move_direction_.x = 0.2f;
                        }
                        circle_move_direction_.Normalize();

                        player_.width *= 0.95f;
                        if (player_.width < 0.04f) {
                            player_.width = 0.04f;
                        }
                        circle_move_speed_ *= 1.01f;
                    } else { // miss
                        // add point to opponent
                        score_.second++;
                        reload();
                    }
                }
            }
        }
        // vertical
        {
            if (circle_.position.y + circle_.radius > 1.f)
            {
                circle_move_direction_.y *= -1.f;
            }
            if (circle_.position.y - circle_.radius < -1.f)
            {
                circle_move_direction_.y *= -1.f;
            }
        }
    }
}

void PingpongComponent::destroy_resources()
{
    rasterizer_state_->Release();

    player_brick_info_buffer_.destroy();
    opponent_brick_info_buffer_.destroy();
    circle_info_buffer_.destroy();
    net_info_buffer_.destroy();

    brick_index_buffer_.destroy();
    circle_index_buffer_.destroy();

    brick_shader_.destroy();
    circle_shader_.destroy();

    net_shader_.destroy();
}

std::string PingpongComponent::brick_shader_source_ = {
R"(struct VS_IN
{
    uint index : SV_VertexID;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
};

cbuffer BrickInfo : register(b0)
{
    float2 position;
    float width;
    float height;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN output = (PS_IN)0;

    float half_width = width / 2;
    float half_height = height / 2;
    if (input.index == 0) {
        output.pos = float4(position.x + half_height, position.y + half_width, .5f, 1.f);
    }
    if (input.index == 1) {
        output.pos = float4(position.x - half_height, position.y - half_width, .5f, 1.f);
    }
    if (input.index == 2) {
        output.pos = float4(position.x + half_height, position.y - half_width, .5f, 1.f);
    }
    if (input.index == 3) {
        output.pos = float4(position.x - half_height, position.y + half_width, .5f, 1.f);
    }

    return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    return (1.f).xxxx;
}
)"};

std::string PingpongComponent::circle_shader_source_{
R"(struct VS_IN
{
    uint index : SV_VertexID;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
};

cbuffer CircleInfo : register(b0)
{
    float2 position;
    float radius;
    uint triangle_count;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN output = (PS_IN)0;

    // setup out pos by index
    if (input.index == 0)
    {
        output.pos = float4(position.xy, .5f, 1.f);
    }
    else
    {
        static const float PI = 3.14159265f;
        float angle = 2 * PI / triangle_count;
        float x_pos = position.x + radius * cos(angle * input.index);
        float y_pos = position.y + radius * sin(angle * input.index);
        output.pos = float4(x_pos, y_pos, .5f, 1.f);
    }

    return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    return float4(1.f, 1.f, 0.f, 1.f); // yellow circle
}
)"};

std::string PingpongComponent::net_shader_source_{
R"(struct VS_IN
{
    uint index : SV_VertexID;
    uint instance : SV_InstanceID;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
};

cbuffer NetInfo : register(b0)
{
    uint net_elements_count;
    float width;
    float height;
    uint unused;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN res = (PS_IN)0;

    float y_pos = (float(input.instance) / net_elements_count + .5f / net_elements_count) * 2.f - 1.f;
    float half_width = width / 2;
    float half_height = height / 2;
    float2 pos = (0).xx;
    if (input.index == 0) {
        pos = float2(-half_width, y_pos - half_height);
    }
    if (input.index == 1) {
        pos = float2(half_width, y_pos + half_height);
    }
    if (input.index == 2) {
        pos = float2(-half_width, y_pos + half_height);
    }
    if (input.index == 3) {
        pos = float2(half_width, y_pos - half_height);
    }

    res.pos = float4(pos, 0.5f, 1.f);
    return res;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    return float4(1.f, 0.f, 1.f, 1.f); // cyan net
}
)"};
