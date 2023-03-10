#include <chrono>
#include "game.h"
#include "win32/win.h"
#include "win32/input.h"
#include "render/render.h"
#include "render/annotation.h"
#include "render/camera.h"
#include "component/game_component.h"

Game::Game()
{
    win_ = std::make_unique<Win>();
    render_ = std::make_unique<Render>();
}

// static
Game* Game::inst()
{
    static Game instance;
    return &instance;
}

Game::~Game()
{
    win_.release();
    render_.release();
}

void Game::add_component(GameComponent* game_component)
{
    game_components_.emplace(game_component);
}

bool Game::initialize(uint32_t w, uint32_t h)
{
    win_->initialize(w, h);
    render_->initialize();

    for (auto game_component : game_components_)
    {
        game_component->initialize();
    }

    animating_ = true; // initialized
    return animating_;
}

void Game::run()
{
    while (!destroy_)
    {
        // handle win messages
        win_->run();

        // process input queue
        Input* input = win_->input();
        {
            input->process_win_input();
        }

        if (!animating_) {
            continue;
        }

        {
            // prepares
            {
                Annotation annotation("prepare resources");
                render_->prepare_frame();
                render_->prepare_resources();
            }

            { // update components
                for (auto game_component : game_components_)
                {
                    game_component->update();
                }
            }

            {
                Annotation annotation("draw components");
                for (auto game_component : game_components_)
                {
                    game_component->draw();
                }
            }

            // Handle components imgui
            {
                Annotation annotation("draw imgui for components");
                render_->prepare_imgui();
                for (auto game_component : game_components_)
                {
                    game_component->imgui();
                }
                render_->end_imgui();
            }
            {
                Annotation annotation("after draw");
                render_->restore_targets();
                render_->end_frame();
            }
        }

        // clear keyboard
        {
            win_->input()->clear_after_process();
        }

        // handle FPS
        {
            static auto prev_time {
                std::chrono::steady_clock::now()
            };
            static float total_time = 0;
            static uint32_t frame_count = 0;
            auto cur_time = std::chrono::steady_clock::now();
            delta_time_ = std::chrono::duration_cast<std::chrono::microseconds>(cur_time - prev_time).count() / 1e6f;
            prev_time = cur_time;

            total_time += delta_time_;
            frame_count++;

            if (total_time > 1.0f) {
                OutputDebugString(("FPS: " + std::to_string(frame_count / total_time) + "\n").c_str());
                total_time -= 1.0f;
                frame_count = 0;
            }
        }
    }
    // next step - destroy
}

void Game::destroy()
{
    for (auto game_component : game_components_)
    {
        game_component->destroy_resources();
    }
    game_components_.clear();

    render_->destroy_resources();
    win_->destroy();
}

float Game::delta_time() const
{
    return delta_time_;
}

void Game::set_animating(bool animating)
{
    animating_ = animating;
}

void Game::set_destroy()
{
    destroy_ = true;
}

void Game::resize()
{
    render_->resize();
}

void Game::toggle_fullscreen()
{
    fullscreen_ = !fullscreen_;
    render_->fullscreen(fullscreen_);
}

const Win& Game::win() const
{
    return *win_;
}

const Render& Game::render() const
{
    return *render_;
}
