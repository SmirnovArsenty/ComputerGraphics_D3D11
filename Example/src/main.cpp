#include <Windows.h>
#include "core/game.h"
#include <dxgidebug.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    Game::inst()->initialize(800, 800);
    Game::inst()->run();
    Game::inst()->destroy();

    return 0;
}
