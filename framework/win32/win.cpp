#include <Windows.h>
#include <hidusage.h>
#include <chrono>
#include <string>
#include <stdexcept>
#include <string>
#include "win.h"
#include "core/game.h"

// static
LRESULT CALLBACK Win::WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    static int32_t old_x = 0;
    static int32_t old_y = 0;
    switch (message)
    {
        case WM_GETMINMAXINFO:
        {   // need to handle it for fullscreen
            ((MINMAXINFO*)lparam)->ptMaxTrackSize.y =
                GetSystemMetrics(SM_CYMAXTRACK) +
                GetSystemMetrics(SM_CYCAPTION) +
                GetSystemMetrics(SM_CYMENU) +
                GetSystemMetrics(SM_CYBORDER) * 2;
            return 0;
        }
        case WM_INPUT:
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
                Game::inst()->handle_keyboard(keyboard.VKey, keyboard.Message);
            }
            else if (raw_input->header.dwType == RIM_TYPEMOUSE)
            {
                RAWMOUSE mouse = raw_input->data.mouse;
                Game::inst()->handle_mouse(float(mouse.lLastX), float(mouse.lLastY), mouse.usButtonFlags);
            }

            DefRawInputProc(&raw_input, 1, sizeof(RAWINPUTHEADER));
            return 0;
        }
        case WM_DESTROY:
        {
            Game::inst()->set_destroy();
            Game::inst()->set_animating(false);
            return 0;
        }
        case WM_ENTERSIZEMOVE:
        {
            Game::inst()->set_animating(false);
            return 0;
        }
        case WM_EXITSIZEMOVE:
        {
            // OutputDebugString(std::to_string(wparam).c_str());
            Game::inst()->resize();
            Game::inst()->set_animating(true);
            return 0;
        }
        default:
        {
            return DefWindowProc(hWnd, message, wparam, lparam);
        }
    }
}

bool Win::initialize(uint32_t w, uint32_t h)
{
    constexpr char* app_name = "SGame";

    HINSTANCE hInst = GetModuleHandle(NULL);

    WNDCLASSEX wc;
    
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = app_name;
    wc.cbSize = sizeof(WNDCLASSEX);

    // Register the window class.
    RegisterClassEx(&wc);

    auto screenWidth = 800;
    auto screenHeight = 800;

    RECT windowRect = { 0, 0, static_cast<LONG>(screenWidth), static_cast<LONG>(screenHeight) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_THICKFRAME; // WS_MINIMIZEBOX

    auto posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
    auto posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

    hWnd_ = CreateWindowEx(WS_EX_APPWINDOW, app_name, app_name,
        dwStyle,
        posX, posY,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInst, nullptr);

    ShowWindow(hWnd_, SW_SHOW);
    SetForegroundWindow(hWnd_);
    SetFocus(hWnd_);

    ShowCursor(true);

    // setup RawInput
    {
        RAWINPUTDEVICE raw_input_devices[2];
        raw_input_devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
        raw_input_devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
        raw_input_devices[0].dwFlags = 0; // RIDEV_NOLEGACY ?
        raw_input_devices[0].hwndTarget = hWnd_;

        raw_input_devices[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
        raw_input_devices[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
        raw_input_devices[1].dwFlags = 0; // RIDEV_NOLEGACY ?
        raw_input_devices[1].hwndTarget = hWnd_;

        // HID_USAGE_GENERIC_GAMEPAD ?

        if (!RegisterRawInputDevices(raw_input_devices, static_cast<UINT>(std::size(raw_input_devices)), sizeof(RAWINPUTDEVICE)))
        {
            OutputDebugString(("Can not register raw input device. Error: " + std::to_string(GetLastError())).c_str());
            throw std::runtime_error("Can not register raw input device");
        }
    }

    return true;
}

void Win::run()
{
    MSG msg{};
    while (PeekMessage(&msg, hWnd_, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Win::destroy()
{
}

HWND Win::get_window() const
{
    return hWnd_;
}
