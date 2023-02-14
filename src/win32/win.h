#pragma once

#include <Windows.h>
#include <cstdint>

class Win final
{
private:
    HWND hWnd_{ NULL };

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
public:
    Win() = default;
    ~Win() = default;

    bool initialize(uint32_t, uint32_t);
    void run();
    void destroy();

    HWND get_window() const;
};
