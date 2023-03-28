#include <Windows.h>
#include <dxgidebug.h>

#include "core/game.h"
#include "components/orbit/orbit_component.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    auto orbit = std::make_unique<OrbitComponent>();
    Game::inst()->add_component(orbit.get());

    Game::inst()->initialize(800, 800);
    Game::inst()->run();
    Game::inst()->destroy();

    return 0;
}
