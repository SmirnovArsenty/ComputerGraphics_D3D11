#include "core/game.h"
#include "render/scene/model.h"
#include "components/katamari/katamari_component.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, char*, int)
{
    auto katamari = std::make_unique<KatamariComponent>();
    Game::inst()->add_component(katamari.get());

    Game::inst()->initialize(800, 800);
    Game::inst()->run();
    Game::inst()->destroy();

    return 0;
}
