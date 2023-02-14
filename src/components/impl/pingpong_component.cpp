#include "core/game.h"
#include "render/render.h"
#include "pingpong_component.h"
#include "render/d3d11_common.h"

PingpongComponent::PingpongComponent()
{
    // setup game data
    player_.position = glm::vec2(-1.f + 0.02f, .0f);
    player_.width = 0.3f;
    player_.height = 0.02f;

    opponent_.position = glm::vec2(1.f - 0.02f, .0f);
    opponent_.width = 0.3f;
    opponent_.height = 0.02f;

    circle_.triangle_count = 10;
    circle_.position = glm::vec2(0.f, 0.f);
    circle_.radius = 0.02f;

    circle_move_direction_ = glm::vec2(1.f, 0.f); // initially moves to opponent
}

void PingpongComponent::initialize()
{
    // setup shaders
    brick_shader_.set_vs_shader((std::string(resource_path_) + "shaders/brick.hlsl").c_str(),
                                "VSMain", nullptr, nullptr);
    brick_shader_.set_ps_shader((std::string(resource_path_) + "shaders/brick.hlsl").c_str(),
                                "PSMain", nullptr, nullptr);

    circle_shader_.set_vs_shader((std::string(resource_path_) + "shaders/circle.hlsl").c_str(),
                                 "VSMain", nullptr, nullptr);
    circle_shader_.set_ps_shader((std::string(resource_path_) + "shaders/circle.hlsl").c_str(),
                                 "PSMain", nullptr, nullptr);
    // input layout is not required: only index buffer used

    std::vector<uint16_t> brick_index_data = { 0,1,2, 0,1,3 };
    brick_index_buffer_.initialize(D3D11_BIND_INDEX_BUFFER, brick_index_data.data(), sizeof(brick_index_data[0]), UINT(std::size(brick_index_data)));

    std::vector<uint16_t> circle_index_data; // = { 0,1,2, 0,2,3 };
    for (uint16_t i = 0; i < circle_.triangle_count; ++i)
    {
        circle_index_data.push_back(0); // first index is 0 - circle center
        circle_index_data.push_back(i + 1);
        circle_index_data.push_back(i + 2);
    }
    circle_index_data.back() = 1; // link with first triangle
    circle_index_buffer_.initialize(D3D11_BIND_INDEX_BUFFER, circle_index_data.data(), sizeof(circle_index_data[0]), UINT(std::size(circle_index_data)));

    player_brick_info_buffer_.initialize(sizeof(BrickInfo), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    opponent_brick_info_buffer_.initialize(sizeof(BrickInfo), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    circle_info_buffer_.initialize(sizeof(CircleInfo), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateRasterizerState(&rastDesc, &rasterizer_state_));
}

void PingpongComponent::draw()
{
    auto context = Game::inst()->render().context();

    context->RSSetState(rasterizer_state_);

    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
}

void PingpongComponent::update()
{
    const Game::KeyboardState& keyboard = Game::inst()->keyboard_state();
    const float brick_move_delta = Game::inst()->delta_time() * 1e0f * 1.5f;
    const float circle_move_delta = Game::inst()->delta_time() * 1e0f * 2;

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
    {
        player_.position.y += brick_move_delta * keyboard.up;
        player_.position.y -= brick_move_delta * keyboard.down;

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

    // check intersections
    {
        // horizontal
        {
            if (circle_move_direction_.x > 0) // move to AI
            {
                if (circle_.position.x > opponent_.position.x)
                {
                    MessageBox(NULL, "You won!", "Winner!", MB_OK | MB_ICONEXCLAMATION);
                    Game::inst()->set_destroy();
                }
                if (circle_.position.x + circle_.radius > opponent_.position.x - opponent_.height / 2.f &&
                    circle_.position.y + circle_.radius < opponent_.position.y + opponent_.width / 2.f &&
                    circle_.position.y - circle_.radius > opponent_.position.y - opponent_.width / 2.f)
                { // brick hit
                    // reflect circle
                    srand(static_cast<unsigned>(Game::inst()->delta_time() * 1e9f));
                    circle_move_direction_ = glm::normalize(circle_.position - opponent_.position);
                    circle_move_direction_.y += (2.f * rand() / RAND_MAX - 1.f) * 1e-2f;
                    circle_move_direction_ = glm::normalize(circle_move_direction_);

                    // TODO: reduce size by every hit
                    opponent_.width *= 0.99f;
                }
            }
            else // move to player
            {
                if (circle_.position.x < player_.position.x)
                {
                    MessageBox(NULL, "You lose!", "Loser!", MB_OK | MB_ICONEXCLAMATION);
                    Game::inst()->set_destroy();
                }
                if (circle_.position.x - circle_.radius < player_.position.x + player_.height / 2.f &&
                    circle_.position.y + circle_.radius < player_.position.y + player_.width / 2.f &&
                    circle_.position.y - circle_.radius > player_.position.y - player_.width / 2.f)
                { // brick hit
                    srand(static_cast<unsigned>(Game::inst()->delta_time() * 1e9f));
                    circle_move_direction_ = glm::normalize(circle_.position - player_.position);
                    circle_move_direction_.y += (2.f * rand() / RAND_MAX - 1.f) * 1e-2f;
                    circle_move_direction_ = glm::normalize(circle_move_direction_);

                    player_.width *= 0.99f;
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

    brick_shader_.destroy();
    circle_shader_.destroy();
}
