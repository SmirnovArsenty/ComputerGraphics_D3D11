#include <chrono>
#include "game.h"
#include "win32/win.h"
#include "render/render.h"
#include "render/annotation.h"
#include "render/camera.h"
#include "components/common/game_component.h"

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

        // handle device inputs
        {
            { // move camera
                Camera* camera = render_->camera();
                float camera_move_delta = delta_time_ * 1e2f;
                if (keyboard_state_.shift) {
                    camera_move_delta *= 1e1f;
                }
                camera->move_forward(camera_move_delta * keyboard_state_.w);
                camera->move_right(camera_move_delta * keyboard_state_.d);
                camera->move_forward(-camera_move_delta * keyboard_state_.s);
                camera->move_right(-camera_move_delta * keyboard_state_.a);
                camera->move_up(camera_move_delta * keyboard_state_.space);
                camera->move_up(-camera_move_delta * keyboard_state_.c);
            }
            { // rotate camera
                const float camera_rotate_delta = delta_time_ / 1e1f;
                if (mouse_state_.rbutton)
                { // camera rotation with right button pressed
                    render_->camera()->pitch(-mouse_state_.delta_y * camera_rotate_delta); // negate to convert from Win32 coordinates
                    render_->camera()->yaw(mouse_state_.delta_x * camera_rotate_delta);
                }
            }

            // clear mouse_state_ deltas
            mouse_state_.delta_x = 0;
            mouse_state_.delta_y = 0;
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

            {
                Annotation annotation("after draw");
                render_->restore_targets();
                render_->end_frame();
            }
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

void Game::handle_keyboard(uint16_t key, uint32_t message)
{
#define HANDLE_CHAR_KEY(code) \
    key == ((#code)[0] + 'A' - 'a') && (keyboard_state_.code = (message == WM_KEYDOWN))

#define HANDLE_VK_KEY(code, state_key) \
    key == (code) && (keyboard_state_.state_key = (message == WM_KEYDOWN))

    HANDLE_CHAR_KEY(q);
    HANDLE_CHAR_KEY(w);
    HANDLE_CHAR_KEY(e);
    HANDLE_CHAR_KEY(r);
    HANDLE_CHAR_KEY(t);
    HANDLE_CHAR_KEY(y);
    HANDLE_CHAR_KEY(u);
    HANDLE_CHAR_KEY(i);
    HANDLE_CHAR_KEY(o);
    HANDLE_CHAR_KEY(p);
    HANDLE_CHAR_KEY(a);
    HANDLE_CHAR_KEY(s);
    HANDLE_CHAR_KEY(d);
    HANDLE_CHAR_KEY(f);
    HANDLE_CHAR_KEY(g);
    HANDLE_CHAR_KEY(h);
    HANDLE_CHAR_KEY(j);
    HANDLE_CHAR_KEY(k);
    HANDLE_CHAR_KEY(l);
    HANDLE_CHAR_KEY(z);
    HANDLE_CHAR_KEY(x);
    HANDLE_CHAR_KEY(c);
    HANDLE_CHAR_KEY(v);
    HANDLE_CHAR_KEY(b);
    HANDLE_CHAR_KEY(n);
    HANDLE_CHAR_KEY(m);
    HANDLE_CHAR_KEY(d);
    HANDLE_VK_KEY(VK_SHIFT, shift);
    HANDLE_VK_KEY(VK_CONTROL, ctrl);
    HANDLE_VK_KEY(VK_MENU, alt);
    HANDLE_VK_KEY(VK_SPACE, space);
    HANDLE_VK_KEY(VK_RETURN, enter);
    HANDLE_VK_KEY(VK_UP, up);
    HANDLE_VK_KEY(VK_DOWN, down);
    HANDLE_VK_KEY(VK_LEFT, left);
    HANDLE_VK_KEY(VK_RIGHT, right);

    if (key == VK_ESCAPE)
    {
        animating_ = false;
        destroy_ = true;
    }

#undef HANDLE_CHAR_KEY
#undef HANDLE_VK_KEY
}

void Game::handle_mouse(float delta_x, float delta_y, uint16_t flags)
{
    if (flags & RI_MOUSE_LEFT_BUTTON_DOWN) {
        mouse_state_.lbutton = true;
    } else if (flags & RI_MOUSE_LEFT_BUTTON_UP) {
        mouse_state_.lbutton = false;
    }

    if (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
        mouse_state_.mbutton = true;
    } else if (flags & RI_MOUSE_MIDDLE_BUTTON_UP) {
        mouse_state_.mbutton = false;
    }

    if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
        mouse_state_.rbutton = true;
    } else if (flags & RI_MOUSE_RIGHT_BUTTON_UP) {
        mouse_state_.rbutton = false;
    }

    mouse_state_.delta_x += delta_x;
    mouse_state_.delta_y += delta_y;
}

const Win& Game::win() const
{
    return *win_;
}

const Render& Game::render() const
{
    return *render_;
}

const Game::KeyboardState& Game::keyboard_state() const
{
    return keyboard_state_;
}

const Game::MouseState& Game::mouse_state() const
{
    return mouse_state_;
}
