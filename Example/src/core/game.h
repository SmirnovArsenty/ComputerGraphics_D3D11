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

    float delta_time_{ 0.f };

    struct
    {
        bool w = false;
        bool a = false;
        bool s = false;
        bool d = false;
        bool c = false;

        bool shift = false;
        bool space = false;
    } keyboard_state_;

    struct
    {
        float delta_x = 0.f;
        float delta_y = 0.f;

        bool lbutton = false;
        bool mbutton = false;
        bool rbutton = false;
    } mouse_state_;

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

    float delta_time() const;

    void set_animating(bool);
    void set_destroy();
    void resize();
    void toggle_fullscreen();

    void handle_keyboard(uint16_t key, uint32_t message);
    void handle_mouse(float delta_x, float delta_y, uint16_t flags);

    const Win& win() const;
    const Render& render() const;
};