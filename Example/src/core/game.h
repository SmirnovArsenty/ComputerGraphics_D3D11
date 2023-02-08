#pragma once

#include <cstdint>
#include <memory>
#include <unordered_set>

class Win;
class Render;
class GameComponent;

class Game
{
private:
    std::unique_ptr<Win> win_;
    std::unique_ptr<Render> render_;

    bool animating_{ false };
    bool fullscreen_{ false };

    std::unordered_set<GameComponent*> game_components_;

    Game();
    Game(Game&) = delete;
    Game(const Game&&) = delete;
public:
    static Game* inst();

    virtual bool initialize(uint32_t, uint32_t);
    virtual void run();
    virtual void destroy();

    void set_animating(bool);
    void resize();
    void toggle_fullscreen();

    const Win& win() const;
    const Render& render() const;
};