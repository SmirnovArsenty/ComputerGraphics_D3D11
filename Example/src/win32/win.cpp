#include <chrono>
#include <string>
#include "win.h"
#include "core/game.h"
#include "components/game_component_decl.h"

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
        case WM_KEYUP:
        {
            // OutputDebugString((TEXT("Key pressed: ") + std::to_string(wparam) + TEXT("\n")).c_str());
            if (wparam == VK_ESCAPE) {
                Game::inst()->set_destroy();
            }

            if (wparam == 'F') {
                RECT fullscreen_rc;
                HDC hdc = GetDC(HWND_DESKTOP);
                GetClipBox(hdc, &fullscreen_rc);
                ReleaseDC(HWND_DESKTOP, hdc);
                static RECT old_rc = fullscreen_rc;

                RECT current_rc;
                GetWindowRect(hWnd, &current_rc);
                SetWindowPos(hWnd, nullptr,
                             old_rc.left, old_rc.top,
                             old_rc.right - old_rc.left, old_rc.bottom - old_rc.top,
                             SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                old_rc = current_rc;

                Game::inst()->toggle_fullscreen();
            }
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            if (wparam & MK_LBUTTON) // lbutton pressed
            { // drag handle
                int32_t x = LOWORD(lparam);
                int32_t y = HIWORD(lparam);
                if (old_x == 0 && old_y == 0) {
                    old_x = x;
                    old_y = y;
                    return 0;
                }
                float delta_x = float(x - old_x);
                if (abs(delta_x) > 20) {
                    delta_x = 0;
                }
                float delta_y = float(y - old_y);
                if (abs(delta_y) > 20) {
                    delta_y = 0;
                }
                Game::inst()->mouse_drag(delta_x, delta_y);
                old_x = x;
                old_y = y;
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        {
            old_x = 0;
            old_y = 0;
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
    return true;
}

void Win::run()
{
    static std::chrono::time_point<std::chrono::steady_clock> prev_time {
        std::chrono::steady_clock::now()
    };
    static float total_time = 0;
    static uint32_t frame_count = 0;

    MSG msg{};
    while (PeekMessage(&msg, hWnd_, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    auto cur_time = std::chrono::steady_clock::now();
    float delta_time = 
        std::chrono::duration_cast<std::chrono::microseconds>(cur_time - prev_time).count()
        / 1e6f;
    prev_time = cur_time;

    total_time += delta_time;
    frame_count++;

    if (total_time > 1.0f) {
        float fps = frame_count / total_time;

        total_time -= 1.0f;

        char text[256];
        sprintf_s(text, TEXT("FPS: %f\n"), fps);
        // SetWindowText(hWnd_, text);
        OutputDebugString(text);

        frame_count = 0;
    }
}

void Win::destroy()
{
}

HWND Win::get_window() const
{
    return hWnd_;
}
