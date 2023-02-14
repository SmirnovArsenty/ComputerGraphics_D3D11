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

    bool destroy_{ false };
    bool animating_{ false };
    bool fullscreen_{ false };

    std::unordered_set<GameComponent*> game_components_;

    Game();
    Game(Game&) = delete;
    Game(const Game&&) = delete;
public:
    struct KeyboardState {
        bool q = false;
        bool w = false;
        bool e = false;
        bool r = false;
        bool t = false;
        bool y = false;
        bool u = false;
        bool i = false;
        bool o = false;
        bool p = false;
        bool a = false;
        bool s = false;
        bool d = false;
        bool f = false;
        bool g = false;
        bool h = false;
        bool j = false;
        bool k = false;
        bool l = false;
        bool z = false;
        bool x = false;
        bool c = false;
        bool v = false;
        bool b = false;
        bool n = false;
        bool m = false;

        bool shift = false;
        bool ctrl = false;
        bool alt = false;
        bool space = false;
        bool enter = false;
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
    };

    struct MouseState
    {
        float delta_x = 0.f;
        float delta_y = 0.f;
        float delta_z = 0.f; // scroll

        bool lbutton = false;
        bool mbutton = false;
        bool rbutton = false;
    };

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

    const KeyboardState& keyboard_state() const;
    const MouseState& mouse_state() const;

private:
    struct KeyboardState keyboard_state_;
    struct MouseState mouse_state_;
};
