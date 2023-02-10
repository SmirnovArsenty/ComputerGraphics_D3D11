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

    bool destroy_{ false };
    bool animating_{ false };
    bool fullscreen_{ false };

    std::unordered_set<GameComponent*> game_components_;

    Game();
    Game(Game&) = delete;
    Game(const Game&&) = delete;
public:
    static Game* inst();
    ~Game();

    virtual void add_component(GameComponent*);

    virtual bool initialize(uint32_t, uint32_t);
    virtual void run();
    virtual void destroy();

    void set_animating(bool);
    void set_destroy();
    void resize();
    void toggle_fullscreen();

    void mouse_drag(float delta_x, float delta_y);

    const Win& win() const;
    const Render& render() const;
};