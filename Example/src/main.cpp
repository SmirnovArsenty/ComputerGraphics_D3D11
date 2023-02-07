#include <Windows.h>
#include "core/game_engine.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    GameEngine::inst()->init(800, 800);
    GameEngine::inst()->run();
    GameEngine::inst()->destroy();

    return 0;
}
