#include <Windows.h>
#include <dxgidebug.h>

#include "core/game.h"
#include "components/common/game_component_decl.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    auto sponza = std::make_unique<GLTFModelComponent>("models/Sponza/Sponza.gltf");
    // Game::inst()->add_component(sponza.get());

    auto traingle = std::make_unique<TriangleComponent>();
    // Game::inst()->add_component(traingle.get());

    auto pingpong = std::make_unique<PingpongComponent>();
    Game::inst()->add_component(pingpong.get());

    Game::inst()->initialize(800, 800);
    Game::inst()->run();
    Game::inst()->destroy();

    return 0;
}
