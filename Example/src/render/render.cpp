#include <directxmath.h>

#include "render.h"
#include "core/game_engine.h"
#include "win32/win.h"
#include "d3d11_common.h"
#include "shader.h"

class GameObject
{
private:
    Shader shader_;

    // Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;
public:
    GameObject()
    {        
        shader_.set_vs_shader(L"./shaders/shader.hlsl", "VSMain",
                              nullptr, nullptr);
        D3D_SHADER_MACRO pixel_shader_macros[] = {
            "TEST", "1",
            "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)",
            nullptr, nullptr
        };
        shader_.set_ps_shader(L"./shaders/shader.hlsl", "PSMain",
                              pixel_shader_macros, nullptr);

        D3D11_INPUT_ELEMENT_DESC inputs[] = {
            D3D11_INPUT_ELEMENT_DESC {
                "POSITION",
                0,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                0,
                0,
                D3D11_INPUT_PER_VERTEX_DATA,
                0},
            D3D11_INPUT_ELEMENT_DESC {
                "COLOR",
                0,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                0,
                D3D11_APPEND_ALIGNED_ELEMENT,
                D3D11_INPUT_PER_VERTEX_DATA,
                0}
        };
        shader_.set_input_layout(inputs, 2);

        // create buffers
        auto device = GameEngine::inst()->render().device();
        // DirectX::XMFLOAT4 points[] = {
        //     DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
        //     DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
        //     DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
        //     DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        // };

        // D3D11_BUFFER_DESC vertexBufDesc = {};
        // vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
        // vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        // vertexBufDesc.CPUAccessFlags = 0;
        // vertexBufDesc.MiscFlags = 0;
        // vertexBufDesc.StructureByteStride = 0;
        // vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * std::size(points);

        // D3D11_SUBRESOURCE_DATA vertexData = {};
        // vertexData.pSysMem = points;
        // vertexData.SysMemPitch = 0;
        // vertexData.SysMemSlicePitch = 0;

        // D3D11_CHECK(device->CreateBuffer(&vertexBufDesc, &vertexData, &vertex_buffer_));

        int indices[] = { 0,1,2, 1,0,3 };
        D3D11_BUFFER_DESC indexBufDesc = {};
        indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufDesc.CPUAccessFlags = 0;
        indexBufDesc.MiscFlags = 0;
        indexBufDesc.StructureByteStride = 0;
        indexBufDesc.ByteWidth = UINT(sizeof(int) * std::size(indices));

        D3D11_SUBRESOURCE_DATA indexData = {};
        indexData.pSysMem = indices;
        indexData.SysMemPitch = 0;
        indexData.SysMemSlicePitch = 0;

        D3D11_CHECK(device->CreateBuffer(&indexBufDesc, &indexData, &index_buffer_));
    }
    ~GameObject()
    {
    }

    void draw()
    {
        shader_.use();
        auto context = GameEngine::inst()->render().context();
        context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
        // UINT strides[] = { 32 };
        // UINT offsets[] = { 0 };
        // context->IASetVertexBuffers(0, 1, &vertex_buffer, strides, offsets);

        context->DrawIndexed(6, 0, 0);
    }

};

HRESULT Render::init()
{
    GameEngine* engine = GameEngine::inst();

    HWND hWnd = engine->win().get_window();
    if (hWnd == NULL)
    {
        OutputDebugString(L"Window is not initialized!");
        return -1;
    }
    RECT rc;
    GetWindowRect(hWnd, &rc);

    // create device and swapchain
    D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
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
    Microsoft::WRL::ComPtr<IDXGIFactory> factory;
    D3D11_CHECK(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));
    int32_t best_adapter_index = 0;
    int32_t adapter_number = 0;
    size_t best_memory = 0;
    while (true) {
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

    // pull main render target view
    D3D11_CHECK(swapchain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer_texture_));
    D3D11_CHECK(device_->CreateRenderTargetView(backbuffer_texture_.Get(), nullptr, &render_target_view_));

    CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;

	D3D11_CHECK(device_->CreateRasterizerState(&rastDesc, &rasterizer_state_));
	context_->RSSetState(rasterizer_state_.Get());

    game_objects_.push_back(new GameObject());

    return 0; // success
}

void Render::run()
{
    // temporary draw scene here
    context_->ClearState();

    context_->RSSetState(rasterizer_state_.Get());

    RECT rc;
    GetWindowRect(GameEngine::inst()->win().get_window(), &rc);
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(rc.right - rc.left);
    viewport.Height = static_cast<float>(rc.bottom - rc.top);
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;

    context_->RSSetViewports(1, &viewport);

    ID3D11RenderTargetView* rtv[1] = { render_target_view_.Get() };
    context_->OMSetRenderTargets(1, rtv, nullptr);
    float clear_color[4] = {};
    context_->ClearRenderTargetView(render_target_view_.Get(), clear_color);

    // GameObject draw
    for (GameObject* game_object : game_objects_)
    {
        game_object->draw();
    }

    context_->OMSetRenderTargets(0, nullptr, nullptr);

    D3D11_CHECK(swapchain_->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0));
}

void Render::destroy()
{
    for (GameObject* game_object : game_objects_)
    {
        delete game_object;
    }
    game_objects_.clear();
}

ID3D11Device* Render::device() const
{
    return device_.Get();
}

ID3D11DeviceContext* Render::context() const
{
    return context_.Get();
}
