#include <chrono>
#include <thread>
#include "render.h"
#include "core/game.h"
#include "win32/win.h"
#include "d3d11_common.h"
#include "shader.h"

void Render::initialize()
{
    Game* engine = Game::inst();

    HWND hWnd = engine->win().get_window();
    if (hWnd == NULL)
    {
        OutputDebugString(L"Window is not initialized!");
        assert(false);
        return;
    }
    RECT rc;
    GetWindowRect(hWnd, &rc);

    // create device and swapchain
    D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = swapchain_buffer_count_;
    swapDesc.BufferDesc.Width = rc.right - rc.left;
    swapDesc.BufferDesc.Height = rc.bottom - rc.top;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hWnd;
    swapDesc.Windowed = true;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;

    // choose best adapter by memory
    {
        Microsoft::WRL::ComPtr<IDXGIFactory> factory;
        D3D11_CHECK(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));
        int32_t best_adapter_index = 0;
        int32_t adapter_number = 0;
        size_t best_memory = 0;
        while (true)
        {
            Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
            HRESULT enum_result = factory->EnumAdapters(adapter_number, &adapter);
            if (enum_result != S_OK) {
                // enumerating finnished
                break;
            }
            DXGI_ADAPTER_DESC adapter_desc;
            adapter->GetDesc(&adapter_desc);
            if (best_memory < adapter_desc.DedicatedVideoMemory) {
                best_adapter_index = adapter_number;
                best_memory = adapter_desc.DedicatedVideoMemory;
            }
            ++adapter_number;
        }
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        factory->EnumAdapters(best_adapter_index, &adapter);

        D3D11_CHECK(D3D11CreateDeviceAndSwapChain(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN,
                                                  nullptr, D3D11_CREATE_DEVICE_DEBUG,
                                                  featureLevel, 1, D3D11_SDK_VERSION,
                                                  &swapDesc, &swapchain_,
                                                  &device_, nullptr, &context_));
    }

    // pull main render target view
    D3D11_CHECK(swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer_texture_));
    D3D11_CHECK(device_->CreateRenderTargetView(backbuffer_texture_, nullptr, &render_target_view_));

    // setup depth stencil

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
    depth_stencil_desc.DepthEnable = true;
    depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depth_stencil_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; // reversed depth
    depth_stencil_desc.StencilEnable = false;
    depth_stencil_desc.StencilReadMask = 0;
    depth_stencil_desc.StencilWriteMask = 0;
    depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    D3D11_CHECK(device_->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state_));

    create_depth_stencil_texture_and_view();
}

void Render::resize()
{
    if (!swapchain_) {
        return;
    }

    // static auto last_resize_time = std::chrono::steady_clock::now();
    // auto actual_time = std::chrono::steady_clock::now();

    // if ((actual_time - last_resize_time).count() / 1e9f < 1) { // less than one second
    //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // }

    destroy_render_target_view();
    destroy_depth_stencil_texture_and_view();

    {
        Game* engine = Game::inst();
        RECT rc;
        GetWindowRect(Game::inst()->win().get_window(), &rc);
        swapchain_->ResizeBuffers(swapchain_buffer_count_, rc.right - rc.left, rc.bottom - rc.top,
                                  DXGI_FORMAT_R8G8B8A8_UNORM,
                                  DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
    }

    create_render_target_view();
    create_depth_stencil_texture_and_view();
}

void Render::fullscreen(bool is_fullscreen)
{
    if (!swapchain_) {
        return;
    }

    swapchain_->SetFullscreenState(is_fullscreen, nullptr);
}

void Render::prepare_frame()
{
    context_->ClearState();

    RECT rc;
    GetWindowRect(Game::inst()->win().get_window(), &rc);
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(rc.right - rc.left);
    viewport.Height = static_cast<float>(rc.bottom - rc.top);
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;

    context_->RSSetViewports(1, &viewport);
}

void Render::prepare_resources()
{
    context_->OMSetRenderTargets(1, &render_target_view_, depth_stencil_view_);
    float clear_color[4] = { 0.f, 0.f, 0.f, 1.f };
    context_->ClearRenderTargetView(render_target_view_, clear_color);

    context_->OMSetDepthStencilState(depth_stencil_state_, 1);
    context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.f, 0);
}

void Render::restore_targets()
{
    context_->OMSetRenderTargets(0, nullptr, nullptr);
}

void Render::end_frame()
{
    D3D11_CHECK(swapchain_->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0));
}

void Render::destroy_resources()
{
    SAFE_RELEASE(depth_stencil_state_);

    destroy_depth_stencil_texture_and_view();

    destroy_render_target_view();
}

ID3D11Device* Render::device() const
{
    return device_.Get();
}

ID3D11DeviceContext* Render::context() const
{
    return context_.Get();
}

void Render::create_render_target_view()
{
    destroy_render_target_view();
    D3D11_CHECK(swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer_texture_));
    D3D11_CHECK(device_->CreateRenderTargetView(backbuffer_texture_, nullptr, &render_target_view_));
}

void Render::destroy_render_target_view()
{
    SAFE_RELEASE(backbuffer_texture_);
    SAFE_RELEASE(render_target_view_);
}

void Render::create_depth_stencil_texture_and_view()
{
    destroy_depth_stencil_texture_and_view();

    RECT rc;
    GetWindowRect(Game::inst()->win().get_window(), &rc);
    D3D11_TEXTURE2D_DESC depth_texture_desc;
    depth_texture_desc.Width = rc.right - rc.left;
    depth_texture_desc.Height = rc.bottom - rc.top;
    depth_texture_desc.MipLevels = 1;
    depth_texture_desc.ArraySize = 1;
    depth_texture_desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    depth_texture_desc.SampleDesc.Count = 1;
    depth_texture_desc.SampleDesc.Quality = 0;
    depth_texture_desc.Usage = D3D11_USAGE_DEFAULT;
    depth_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depth_texture_desc.CPUAccessFlags = 0;
    depth_texture_desc.MiscFlags = 0;
    D3D11_CHECK(device_->CreateTexture2D(&depth_texture_desc, nullptr, &depth_stencil_texture_));

    D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
    depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depth_stencil_view_desc.Texture2D.MipSlice = 0;
    D3D11_CHECK(device_->CreateDepthStencilView(depth_stencil_texture_, &depth_stencil_view_desc, &depth_stencil_view_));
}

void Render::destroy_depth_stencil_texture_and_view()
{
    SAFE_RELEASE(depth_stencil_texture_);
    SAFE_RELEASE(depth_stencil_view_);
}
