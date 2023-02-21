#include <Windows.h>
#include <dxgidebug.h>

#include "core/game.h"
#include "components/pingpong/pingpong_component.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    auto pingpong = std::make_unique<PingpongComponent>();
    Game::inst()->add_component(pingpong.get());

    Game::inst()->initialize(800, 800);
    Game::inst()->run();
    Game::inst()->destroy();

    return 0;
}
