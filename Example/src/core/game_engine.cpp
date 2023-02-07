#include "game_engine.h"
#include "win32/win.h"
#include "render/render.h"

GameEngine::GameEngine()
{
    win_ = std::make_unique<Win>();
    render_ = std::make_unique<Render>();
}

// static
GameEngine* GameEngine::inst()
{
    static GameEngine instance;
    return &instance;
}

bool GameEngine::init(uint32_t w, uint32_t h)
{
    animating_ = (win_->init(w, h) && SUCCEEDED(render_->init()));
    return animating_;
}

void GameEngine::run()
{
    while (animating_)
    {
        win_->run();
        render_->run();
    }
    // next step - destroy
}

void GameEngine::destroy()
{
    win_->destroy();
    render_->destroy();
}

void GameEngine::set_animating(bool animating)
{
    animating_ = animating;
}

const Win& GameEngine::win() const
{
    return *win_;
}

const Render& GameEngine::render() const
{
    return *render_;
}
