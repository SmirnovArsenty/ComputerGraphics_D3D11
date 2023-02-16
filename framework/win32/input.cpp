#include <Windows.h>
#include <hidusage.h>
#include <stdexcept>
#include <string>
#include "input.h"
#include "core/game.h"

Input::Input(HWND hWnd) {
    RAWINPUTDEVICE raw_input_devices[2];
    raw_input_devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    raw_input_devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    raw_input_devices[0].dwFlags = 0; // RIDEV_NOLEGACY ?
    raw_input_devices[0].hwndTarget = hWnd;

    raw_input_devices[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    raw_input_devices[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    raw_input_devices[1].dwFlags = 0; // RIDEV_NOLEGACY ?
    raw_input_devices[1].hwndTarget = hWnd;

    // HID_USAGE_GENERIC_GAMEPAD ?

    if (!RegisterRawInputDevices(raw_input_devices, static_cast<UINT>(std::size(raw_input_devices)), sizeof(RAWINPUTDEVICE)))
    {
        OutputDebugString(("Can not register raw input device. Error: " + std::to_string(GetLastError())).c_str());
        throw std::runtime_error("Can not register raw input device");
    }
}

Input::~Input() {}

void Input::handle_win_input(WPARAM wparam, LPARAM lparam)
{
    UINT dwSize;
    GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    BYTE* buffer = (BYTE*)alloca(dwSize);
    if (!buffer)
    {
        throw std::runtime_error("Can not allocate memory buffer");
    }

    if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, buffer, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
    {
        throw std::runtime_error("Error getting raw input data: wrong size");
    }

    RAWINPUT* raw_input = (RAWINPUT*)buffer;
    if (raw_input->header.dwType == RIM_TYPEKEYBOARD)
    {
        RAWKEYBOARD keyboard = raw_input->data.keyboard;

        KeyboardState::ButtonState button_state;
        button_state.pressed = (keyboard.Message == WM_KEYDOWN);
        button_state.released = (keyboard.Message == WM_KEYUP);

        InputEvent input_event{};
        input_event.type = InputEvent::InputEventType::KEYBOARD;
        input_event.keyboard = { keyboard.VKey, button_state };
        event_queue_.push(input_event);
    }
    else if (raw_input->header.dwType == RIM_TYPEMOUSE)
    {
        RAWMOUSE mouse = raw_input->data.mouse;
        InputEvent input_event{};
        input_event.type = InputEvent::InputEventType::MOUSE;
        MouseState mouse_state;
        {
            if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
                mouse_state.lbutton = true;
            } else if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
                mouse_state.lbutton = false;
            }

            if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
                mouse_state.mbutton = true;
            } else if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
                mouse_state.mbutton = false;
            }

            if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
                mouse_state.rbutton = true;
            } else if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
                mouse_state.rbutton = false;
            }

            mouse_state.delta_x = float(mouse.lLastX);
            mouse_state.delta_y = float(mouse.lLastY);
        }

        input_event.mouse = mouse_state;
        event_queue_.push(input_event);
    }

    DefRawInputProc(&raw_input, 1, sizeof(RAWINPUTHEADER));
}

void Input::process_win_input()
{
    if (event_queue_.empty()) {
        return;
    }
    InputEvent input_event = event_queue_.front();
    event_queue_.pop();
    switch (input_event.type)
    {
        case InputEvent::InputEventType::MOUSE:
        {
            mouse_state_ = input_event.mouse;
            break;
        }
        case InputEvent::InputEventType::KEYBOARD:
        {
            auto button = input_event.keyboard;

            if (button.key_code == VK_ESCAPE) {
                Game::inst()->set_animating(false);
                Game::inst()->set_destroy();
            }
#define HANDLE_INPUT_EVENT(key, code)                               \
    do {                                                            \
        if (button.key_code == code) {                              \
            keyboard_state_.key.pressed = button.state.pressed;     \
            keyboard_state_.key.released = button.state.released;   \
        }                                                           \
    } while ((void)0, 0)

            FOR_EACH_BUTTON(HANDLE_INPUT_EVENT);
#undef HANDLE_INPUT_EVENT
            break;
        }
        default:
        {
            throw std::runtime_error("unknown input event type from event queue");
            break;
        }
    }
}

void Input::clear_after_process()
{
    // clear mouse deltas
    mouse_state_.delta_x = 0;
    mouse_state_.delta_y = 0;
    mouse_state_.delta_z = 0;

    // clear keyboard release states
#define CLEAR_RELEASED(key, code)               \
    do {                                        \
        keyboard_state_.key.released = false;   \
    } while ((void)0, 0)

    FOR_EACH_BUTTON(CLEAR_RELEASED);

#undef CLEAR_RELEASED
}


const Input::MouseState& Input::mouse() const
{
    return mouse_state_;
}

const Input::KeyboardState& Input::keyboard() const
{
    return keyboard_state_;
}
