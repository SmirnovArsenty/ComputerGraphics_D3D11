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
    number_shader_.set_vs_shader_from_memory(number_shader_source_, "VSMain", nullptr, nullptr);
    number_shader_.set_ps_shader_from_memory(number_shader_source_, "PSMain", nullptr, nullptr);
#ifndef NDEBUG
    number_shader_.set_name("number_shader");
#endif
    D3D11_INPUT_ELEMENT_DESC inputs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    number_shader_.set_input_layout(inputs, std::size(inputs));

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
    for (uint16_t i = 0; i < circle_.triangle_count; ++i)
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

    // setup vertex buffers for numbers
    {
        // 0
        {
        }

        // 1
        {
        }

    }

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

void PingpongComponent::reload()
{
    // setup game components
    player_ = default_player_;
    opponent_ = default_opponent_;
    circle_ = default_circle_;
    circle_move_direction_ = default_circle_move_direction_;
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

    std::string score_string;
    score_string += "Player: " + std::to_string(score_.first);
    score_string += " | Opponent: " + std::to_string(score_.second);
    SetWindowText(Game::inst()->win().window(), score_string.c_str());

    // toggle (AI vs AI) and (user vs AI) modes
    if (keyboard.s.released) {
        self_mode_ = !self_mode_;
    }

    // handle circle fly
    {
        circle_.position += circle_move_direction_ * circle_move_delta;
    }

    // handle AI
    {
        // update AI position when circle moves in its direction
        if (circle_move_direction_.x > 0) {
            if (opponent_.position.y > circle_.position.y) {
                opponent_.position.y -= brick_move_delta;
            }
            if (opponent_.position.y < circle_.position.y) {
                opponent_.position.y += brick_move_delta;
            }
        }

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
        // update AI position when circle moves in its direction
        if (circle_move_direction_.x < 0) {
            if (player_.position.y > circle_.position.y) {
                player_.position.y -= brick_move_delta;
            }
            if (player_.position.y < circle_.position.y) {
                player_.position.y += brick_move_delta;
            }
        }

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
            if (circle_move_direction_.x > 0) // move to AI
            {
                if (circle_.position.x > opponent_.position.x + opponent_.height / 2)
                {
                    // add point to player
                    score_.first++;
                    reload();
                }
                if (circle_.position.x + circle_.radius > opponent_.position.x - opponent_.height / 2.f &&
                    circle_.position.y + circle_.radius < opponent_.position.y + opponent_.width / 2.f &&
                    circle_.position.y - circle_.radius > opponent_.position.y - opponent_.width / 2.f)
                { // brick hit
                    // reflect circle
                    circle_move_direction_.x *= -1.f;
                    srand(static_cast<unsigned>(Game::inst()->delta_time() * 1e9f));
                    circle_move_direction_.y = (2.f * rand() / RAND_MAX - 1.f) * .5f;
                    circle_move_direction_.Normalize();

                    // reduce size by every hit
                    opponent_.width *= 0.95f;
                    if (opponent_.width < 0.04f) {
                        opponent_.width = 0.04f;
                    }
                }
            }
            else // move to player
            {
                if (circle_.position.x < player_.position.x - player_.height / 2)
                {
                    // add point to opponent
                    score_.second++;
                    reload();
                }
                if (circle_.position.x - circle_.radius < player_.position.x + player_.height / 2.f &&
                    circle_.position.y + circle_.radius < player_.position.y + player_.width / 2.f &&
                    circle_.position.y - circle_.radius > player_.position.y - player_.width / 2.f)
                { // brick hit
                    circle_move_direction_.x *= -1.f;
                    srand(static_cast<unsigned>(Game::inst()->delta_time() * 1e9f));
                    circle_move_direction_.y = (2.f * rand() / RAND_MAX - 1.f) * .5f;
                    circle_move_direction_.Normalize();

                    player_.width *= 0.95f;
                    if (player_.width < 0.04f) {
                        player_.width = 0.04f;
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

    brick_index_buffer_.destroy();
    circle_index_buffer_.destroy();

    for (size_t i = 0; i < std::size(number_index_buffers_); ++i)
    {
        number_index_buffers_[i].destroy();
    }

    for (size_t i = 0; i < std::size(number_vertex_buffers_); ++i)
    {
        number_vertex_buffers_[i].destroy();
    }

    brick_shader_.destroy();
    circle_shader_.destroy();

    net_shader_.destroy();
    number_shader_.destroy();
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

std::string PingpongComponent::number_shader_source_{
R"(struct VS_IN
{
    float2 pos : POSITION0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
};

cbuffer NumberInfo
{
    float2 position;
    float size;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN res = (PS_IN)0;
    res.pos = float4(position + input.pos * size, 0.5f, 1.f);
    return res;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    return float4(1.f, 1.f, 1.f, 1.f);
}
)"};
