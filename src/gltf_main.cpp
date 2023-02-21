#include <Windows.h>
#include <dxgidebug.h>

#include "core/game.h"
#include "components/gltfmodel/gltfmodel_component.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    auto sponza = std::make_unique<GLTFModelComponent>("models/Sponza/Sponza.gltf");
    Game::inst()->add_component(sponza.get());

    Game::inst()->initialize(800, 800);
    Game::inst()->run();
    Game::inst()->destroy();

    return 0;
}
