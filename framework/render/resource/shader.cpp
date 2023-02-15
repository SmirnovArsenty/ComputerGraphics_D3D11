#include <sstream>
#include <d3dcompiler.h>

#include "core/game.h"
#include "render/render.h"
#include "shader.h"
#include "render/d3d11_common.h"

Shader::Shader() {}

Shader::~Shader() {}

void Shader::set_name(const std::string& name)
{
    std::string vs_name = name + "_vs";
    std::string ps_name = name + "_ps";
    vertex_shader_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(vs_name.size()), vs_name.c_str());
    pixel_shader_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(ps_name.size()), ps_name.c_str());

    std::string il_name = name + "_input_layout";
    input_layout_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(il_name.size()), il_name.c_str());
}

void Shader::set_vs_shader_from_file(const std::string& filename,
                                     const std::string& entrypoint,
                                     D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    // translate filename to wstring
    std::wstringstream wfilename;
    wfilename << filename.c_str();

    ID3DBlob* error_code = nullptr;
    // don't use D3D11_CHECK for this call, need to know compilation error message
    HRESULT status = D3DCompileFromFile(wfilename.str().c_str(), macro, include,
                                        entrypoint.c_str(), "vs_5_0",
                                        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
                                        0, &vertex_bc_, &error_code);
    if (FAILED(status))
    {
        if (error_code)
        {
            std::stringstream err;
            err << (char*)(error_code->GetBufferPointer());
            OutputDebugString(err.str().c_str());
        }
        else
        {
            OutputDebugString("Missing shader file");
        }
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateVertexShader(vertex_bc_->GetBufferPointer(),
                                           vertex_bc_->GetBufferSize(),
                                           nullptr, &vertex_shader_));
}

void Shader::set_ps_shader_from_file(const std::string& filename,
                                     const std::string& entrypoint,
                                     D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    assert(pixel_bc_ == nullptr);
    assert(pixel_shader_ == nullptr);

    // translate filename to wstring
    std::wstringstream wfilename;
    wfilename << filename.c_str();

    ID3DBlob* error_code = nullptr;

    unsigned int compile_flags = 0;
#ifndef NDEBUG
    compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // don't use D3D11_CHECK for this call, need to know compilation error message
    HRESULT status = D3DCompileFromFile(wfilename.str().c_str(), macro, include,
                                        entrypoint.c_str(), "ps_5_0",
                                        compile_flags, 0,
                                        &pixel_bc_, &error_code);
    if (FAILED(status))
    {
        if (error_code)
        {
            std::stringstream err;
            err << (char*)(error_code->GetBufferPointer());
            OutputDebugString(err.str().c_str());
        }
        else
        {
            OutputDebugString("Missing shader file");
        }
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreatePixelShader(pixel_bc_->GetBufferPointer(),
                                          pixel_bc_->GetBufferSize(),
                                          nullptr, &pixel_shader_));
}

void Shader::set_vs_shader_from_memory(const std::string& data,
                                       const std::string& entrypoint,
                                       D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    assert(vertex_bc_ == nullptr);
    assert(vertex_shader_ == nullptr);

    ID3DBlob* error_code = nullptr;
    unsigned int compile_flags = 0;
#ifndef NDEBUG
    compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    HRESULT status = D3DCompile(data.data(), data.size(), nullptr,
                                macro, include,
                                entrypoint.c_str(), "vs_5_0",
                                compile_flags, 0,
                                &vertex_bc_, &error_code);

    if (FAILED(status))
    {
        if (error_code)
        {
            std::stringstream err;
            err << (char*)(error_code->GetBufferPointer());
            OutputDebugString(err.str().c_str());
        }
        else
        {
            OutputDebugString("Missing shader file");
        }
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateVertexShader(vertex_bc_->GetBufferPointer(),
                                          vertex_bc_->GetBufferSize(),
                                          nullptr, &vertex_shader_));
}

void Shader::set_ps_shader_from_memory(const std::string& data,
                                       const std::string& entrypoint,
                                       D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    assert(pixel_bc_ == nullptr);
    assert(pixel_shader_ == nullptr);

    ID3DBlob* error_code = nullptr;
    unsigned int compile_flags = 0;
#ifndef NDEBUG
    compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    HRESULT status = D3DCompile(data.data(), data.size(), nullptr,
                                macro, include,
                                entrypoint.c_str(), "ps_5_0",
                                compile_flags, 0,
                                &pixel_bc_, &error_code);

    if (FAILED(status))
    {
        if (error_code)
        {
            std::stringstream err;
            err << (char*)(error_code->GetBufferPointer());
            OutputDebugString(err.str().c_str());
        }
        else
        {
            OutputDebugString("Missing shader file");
        }
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreatePixelShader(pixel_bc_->GetBufferPointer(),
                                          pixel_bc_->GetBufferSize(),
                                          nullptr, &pixel_shader_));
}

void Shader::set_input_layout(D3D11_INPUT_ELEMENT_DESC* inputs, size_t count)
{
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateInputLayout(
                inputs, static_cast<uint32_t>(count),
                vertex_bc_->GetBufferPointer(),
                vertex_bc_->GetBufferSize(),
                &input_layout_));
}

void Shader::use()
{
    auto context = Game::inst()->render().context();
    if (input_layout_ != nullptr) {
        context->IASetInputLayout(input_layout_);
    }
    context->VSSetShader(vertex_shader_, nullptr, 0);
    context->PSSetShader(pixel_shader_, nullptr, 0);
}

void Shader::destroy()
{
    SAFE_RELEASE(vertex_shader_);
    SAFE_RELEASE(vertex_bc_);
    SAFE_RELEASE(pixel_shader_);
    SAFE_RELEASE(pixel_bc_);

    SAFE_RELEASE(input_layout_);
}
