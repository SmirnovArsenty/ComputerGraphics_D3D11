#include <sstream>
#include <d3dcompiler.h>

#include "core/game_engine.h"
#include "render.h"
#include "shader.h"
#include "d3d11_common.h"

Shader::Shader() {}

Shader::~Shader()
{
}

void Shader::set_vs_shader(const std::wstring& filename,
                           const std::string& entrypoint,
                           D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    ID3DBlob* error_code = nullptr;
    // don't use D3D11_CHECK for this call, need to know compilation error message
    HRESULT status = D3DCompileFromFile(filename.c_str(), macro, include,
                                        entrypoint.c_str(), "vs_5_0",
                                        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
                                        0, &vertex_bc_, &error_code);
    if (FAILED(status))
    {
        if (error_code)
        {
            std::wstringstream err;
            err << (char*)(error_code->GetBufferPointer());
            OutputDebugString(err.str().c_str());
        }
        else
        {
            OutputDebugString(L"Missing shader file");
        }
        assert(false);
    }
    auto device = GameEngine::inst()->render().device();
    D3D11_CHECK(device->CreateVertexShader(vertex_bc_->GetBufferPointer(),
                                           vertex_bc_->GetBufferSize(),
                                           nullptr, &vertex_shader_));
}

void Shader::set_ps_shader(const std::wstring& filename,
                           const std::string& entrypoint,
                           D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    ID3DBlob* error_code = nullptr;
    // don't use D3D11_CHECK for this call, need to know compilation error message
    HRESULT status = D3DCompileFromFile(filename.c_str(), macro, include,
                                        entrypoint.c_str(), "ps_5_0",
                                        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
                                        0, &pixel_bc_, &error_code);
    if (FAILED(status))
    {
        if (error_code)
        {
            std::wstringstream err;
            err << (char*)(error_code->GetBufferPointer());
            OutputDebugString(err.str().c_str());
        }
        else
        {
            OutputDebugString(L"Missing shader file");
        }
        assert(false);
    }
    auto device = GameEngine::inst()->render().device();
    D3D11_CHECK(device->CreatePixelShader(pixel_bc_->GetBufferPointer(),
                                          pixel_bc_->GetBufferSize(),
                                          nullptr, &pixel_shader_));
}

void Shader::set_input_layout(D3D11_INPUT_ELEMENT_DESC* inputs, uint32_t count)
{
    auto device = GameEngine::inst()->render().device();
    D3D11_CHECK(device->CreateInputLayout(
                inputs, count,
                vertex_bc_->GetBufferPointer(),
                vertex_bc_->GetBufferSize(),
                &input_layout_));
}

void Shader::use()
{
    auto context = GameEngine::inst()->render().context();
    context->IASetInputLayout(input_layout_.Get());
    context->VSSetShader(vertex_shader_.Get(), nullptr, 0);
    context->PSSetShader(pixel_shader_.Get(), nullptr, 0);
}
