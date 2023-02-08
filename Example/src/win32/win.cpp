#include <chrono>
#include <string>
#include "win.h"
#include "core/game.h"

// static
LRESULT CALLBACK Win::WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
        case WM_KEYUP:
        {
            // OutputDebugString((L"Key pressed: " + std::to_wstring(wparam) + L"\n").c_str());
            if (wparam == VK_ESCAPE) {
                Game::inst()->set_animating(false);
            }
            return 0;
        }
        case WM_DESTROY:
        {
            Game::inst()->set_animating(false);
            return 0;
        }
        case WM_SIZE:
        {
            OutputDebugString(std::to_wstring(wparam).c_str());
            if (wparam == SIZE_RESTORED) {
                Game::inst()->resize();
            }
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
    constexpr wchar_t* app_name = L"SGame";

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

        WCHAR text[256];
        swprintf_s(text, TEXT("FPS: %f"), fps);
        SetWindowText(hWnd_, text);

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
