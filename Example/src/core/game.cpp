#include "game.h"
#include "win32/win.h"
#include "render/render.h"
#include "render/annotation.h"
#include "components/game_component.h"

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
        win_->run(); // placed here to check `animating_` immediatelly
        if (!animating_){
            continue;
        }

        {
            // prepares
            {
                Annotation annotation("prepare resources");
                render_->prepare_frame();
                render_->prepare_resources();
            }

            {
                Annotation annotation("draw components");
                for (auto game_component : game_components_)
                {
                    game_component->draw();
                }
            }

            {
                Annotation annotation("after draw");
                render_->restore_targets();
                render_->end_frame();
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

    win_->destroy();
    render_->destroy_resources();
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

void Game::mouse_drag(float delta_x, float delta_y)
{
    render_->camera_update(delta_x, delta_y);
}

const Win& Game::win() const
{
    return *win_;
}

const Render& Game::render() const
{
    return *render_;
}
