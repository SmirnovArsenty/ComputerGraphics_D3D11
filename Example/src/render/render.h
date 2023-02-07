#pragma once

#include <Windows.h>
#include <wrl.h>
#include <d3d11.h>
#include <string>
#include <vector>

class GameObject;

class Render
{
private:
    // main D3D11 units
    Microsoft::WRL::ComPtr<ID3D11Device> device_{ nullptr };
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_{ nullptr };
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain_{ nullptr };

    // backbuffer[0] texture and render target
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer_texture_{ nullptr };
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view_{ nullptr };

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state_{ nullptr };

    std::vector<GameObject*> game_objects_;

public:
    Render() = default;

    HRESULT init();
    void run();
    void destroy();

    ID3D11Device* device() const;
    ID3D11DeviceContext* context() const;
};
