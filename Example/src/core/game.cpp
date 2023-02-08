#include "game.h"
#include "win32/win.h"
#include "render/render.h"
#include "components/triangle_component.h"

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

bool Game::initialize(uint32_t w, uint32_t h)
{
    win_->initialize(w, h);
    render_->initialize();

    // add some game components
    game_components_.emplace(new TriangleComponent());

    for (auto game_component : game_components_)
    {
        game_component->initialize();
    }

    animating_ = true; // initialized
    return animating_;
}

void Game::run()
{
    while (animating_)
    {
        // prepares
        render_->prepare_frame();
        render_->prepare_resources();

        for (auto game_component : game_components_)
        {
            game_component->draw();
        }

        render_->restore_targets();
        render_->end_frame();

        
        // handle win messages
        win_->run(); // placed here to check `animating_` immediatelly
    }
    // next step - destroy
}

void Game::destroy()
{
    for (auto game_component : game_components_)
    {
        game_component->destroy_resources();
        delete game_component;
    }
    game_components_.clear();

    win_->destroy();
    render_->destroy_resources();
}

void Game::set_animating(bool animating)
{
    animating_ = animating;
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
