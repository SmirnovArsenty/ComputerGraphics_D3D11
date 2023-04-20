#include <sstream>
#include <d3dcompiler.h>

#include "core/game.h"
#include "render/render.h"
#include "shader.h"
#include "render/d3d11_common.h"

#pragma region GraphicsShader

GraphicsShader::GraphicsShader() {}

GraphicsShader::~GraphicsShader()
{
    assert(input_layout_ == nullptr);
    assert(vertex_shader_ == nullptr);
    assert(vertex_bc_ == nullptr);
    assert(hull_shader_ == nullptr);
    assert(hull_bc_ == nullptr);
    assert(domain_shader_ == nullptr);
    assert(domain_bc_ == nullptr);
    assert(geometry_shader_ == nullptr);
    assert(geometry_bc_ == nullptr);
    assert(pixel_shader_ == nullptr);
    assert(pixel_bc_ == nullptr);
}

void GraphicsShader::set_name(const std::string& name)
{
    std::string il_name = name + "_input_layout";
    std::string vs_name = name + "_vs";
    std::string hs_name = name + "_hs";
    std::string ds_name = name + "_ds";
    std::string gs_name = name + "_gs";
    std::string ps_name = name + "_ps";
    if (input_layout_) {
        input_layout_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(il_name.size()), il_name.c_str());
    }
    if (vertex_shader_) {
        vertex_shader_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(vs_name.size()), vs_name.c_str());
    }
    if (hull_shader_) {
        hull_shader_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(hs_name.size()), hs_name.c_str());
    }
    if (domain_shader_) {
        domain_shader_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(ds_name.size()), ds_name.c_str());
    }
    if (geometry_shader_) {
        geometry_shader_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(gs_name.size()), gs_name.c_str());
    }
    if (pixel_shader_) {
        pixel_shader_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(ps_name.size()), ps_name.c_str());
    }
}

void GraphicsShader::set_vs_shader_from_file(const std::string& filename,
                                     const std::string& entrypoint,
                                     D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    assert(vertex_bc_ == nullptr);
    assert(vertex_shader_ == nullptr);

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
                                        entrypoint.c_str(), "vs_5_0",
                                        compile_flags, 0,
                                        &vertex_bc_, &error_code);
    if (error_code) {
        std::stringstream err;
        err << (char*)(error_code->GetBufferPointer());
        OutputDebugString(err.str().c_str());
    }
    if (FAILED(status)) {
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateVertexShader(vertex_bc_->GetBufferPointer(),
                                           vertex_bc_->GetBufferSize(),
                                           nullptr, &vertex_shader_));
}

void GraphicsShader::set_gs_shader_from_file(const std::string& filename,
                                     const std::string& entrypoint,
                                     D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    assert(geometry_bc_ == nullptr);
    assert(geometry_shader_ == nullptr);

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
                                        entrypoint.c_str(), "gs_5_0",
                                        compile_flags, 0,
                                        &geometry_bc_, &error_code);
    if (error_code) {
        std::stringstream err;
        err << (char*)(error_code->GetBufferPointer());
        OutputDebugString(err.str().c_str());
    }
    if (FAILED(status)) {
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateGeometryShader(geometry_bc_->GetBufferPointer(),
                                             geometry_bc_->GetBufferSize(),
                                             nullptr, &geometry_shader_));
}

void GraphicsShader::set_ps_shader_from_file(const std::string& filename,
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
    if (error_code) {
        std::stringstream err;
        err << (char*)(error_code->GetBufferPointer());
        OutputDebugString(err.str().c_str());
    }
    if (FAILED(status)) {
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreatePixelShader(pixel_bc_->GetBufferPointer(),
                                          pixel_bc_->GetBufferSize(),
                                          nullptr, &pixel_shader_));
}

void GraphicsShader::set_vs_shader_from_memory(const std::string& data,
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

    if (error_code) {
        std::stringstream err;
        err << (char*)(error_code->GetBufferPointer());
        OutputDebugString(err.str().c_str());
    }
    if (FAILED(status)) {
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateVertexShader(vertex_bc_->GetBufferPointer(),
                                          vertex_bc_->GetBufferSize(),
                                          nullptr, &vertex_shader_));
}

void GraphicsShader::set_gs_shader_from_memory(const std::string& data,
                                       const std::string& entrypoint,
                                       D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    assert(geometry_bc_ == nullptr);
    assert(geometry_shader_ == nullptr);

    ID3DBlob* error_code = nullptr;
    unsigned int compile_flags = 0;
#ifndef NDEBUG
    compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    HRESULT status = D3DCompile(data.data(), data.size(), nullptr,
                                macro, include,
                                entrypoint.c_str(), "gs_5_0",
                                compile_flags, 0,
                                &geometry_bc_, &error_code);

    if (error_code) {
        std::stringstream err;
        err << (char*)(error_code->GetBufferPointer());
        OutputDebugString(err.str().c_str());
    }
    if (FAILED(status)) {
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateGeometryShader(geometry_bc_->GetBufferPointer(),
                                             geometry_bc_->GetBufferSize(),
                                             nullptr, &geometry_shader_));
}

void GraphicsShader::set_ps_shader_from_memory(const std::string& data,
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
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreatePixelShader(pixel_bc_->GetBufferPointer(),
                                          pixel_bc_->GetBufferSize(),
                                          nullptr, &pixel_shader_));
}

void GraphicsShader::set_input_layout(D3D11_INPUT_ELEMENT_DESC* inputs, size_t count)
{
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateInputLayout(
                inputs, static_cast<uint32_t>(count),
                vertex_bc_->GetBufferPointer(),
                vertex_bc_->GetBufferSize(),
                &input_layout_));
}

void GraphicsShader::use()
{
    auto context = Game::inst()->render().context();
    context->IASetInputLayout(input_layout_);
    context->VSSetShader(vertex_shader_, nullptr, 0);
    context->HSSetShader(hull_shader_, nullptr, 0);
    context->DSSetShader(domain_shader_, nullptr, 0);
    context->GSSetShader(geometry_shader_, nullptr, 0);
    context->PSSetShader(pixel_shader_, nullptr, 0);
    context->CSSetShader(nullptr, nullptr, 0);
}

void GraphicsShader::destroy()
{
    SAFE_RELEASE(input_layout_);
    SAFE_RELEASE(vertex_shader_);
    SAFE_RELEASE(vertex_bc_);
    SAFE_RELEASE(geometry_shader_);
    SAFE_RELEASE(geometry_bc_);
    SAFE_RELEASE(pixel_shader_);
    SAFE_RELEASE(pixel_bc_);
}

#pragma endregion

#pragma region ComputeShader

ComputeShader::ComputeShader() {}

ComputeShader::~ComputeShader()
{
    assert(compute_shader_ == nullptr);
    assert(compute_bc_ == nullptr);
}

void ComputeShader::set_name(const std::string& name)
{
    std::string cs_name = name + "_cs";
    if (compute_shader_) {
        compute_shader_->SetPrivateData(WKPDID_D3DDebugObjectName, UINT(cs_name.size()), cs_name.c_str());
    }
}

void ComputeShader::set_compute_shader_from_file(const std::string& filename,
    const std::string& entrypoint,
    D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    assert(compute_bc_ == nullptr);
    assert(compute_shader_ == nullptr);

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
        entrypoint.c_str(), "cs_5_0",
        compile_flags, 0,
        &compute_bc_, &error_code);
    if (error_code) {
        std::stringstream err;
        err << (char*)(error_code->GetBufferPointer());
        OutputDebugString(err.str().c_str());
    }
    if (FAILED(status)) {
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateComputeShader(compute_bc_->GetBufferPointer(),
        compute_bc_->GetBufferSize(),
        nullptr, &compute_shader_));
}

void ComputeShader::set_compute_shader_from_memory(const std::string& data,
    const std::string& entrypoint,
    D3D_SHADER_MACRO* macro, ID3DInclude* include)
{
    assert(compute_bc_ == nullptr);
    assert(compute_shader_ == nullptr);

    ID3DBlob* error_code = nullptr;
    unsigned int compile_flags = 0;
#ifndef NDEBUG
    compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    HRESULT status = D3DCompile(data.data(), data.size(), nullptr,
        macro, include,
        entrypoint.c_str(), "cs_5_0",
        compile_flags, 0,
        &compute_bc_, &error_code);
    if (error_code) {
        std::stringstream err;
        err << (char*)(error_code->GetBufferPointer());
        OutputDebugString(err.str().c_str());
    }
    if (FAILED(status))
    {
        assert(false);
    }
    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateComputeShader(compute_bc_->GetBufferPointer(),
        compute_bc_->GetBufferSize(),
        nullptr, &compute_shader_));
}

void ComputeShader::use()
{
    auto context = Game::inst()->render().context();
    context->IASetInputLayout(nullptr);
    context->VSSetShader(nullptr, nullptr, 0);
    context->HSSetShader(nullptr, nullptr, 0);
    context->DSSetShader(nullptr, nullptr, 0);
    context->GSSetShader(nullptr, nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);
    context->CSSetShader(compute_shader_, nullptr, 0);
}

void ComputeShader::destroy()
{
    SAFE_RELEASE(compute_shader_);
    SAFE_RELEASE(compute_bc_);
}

#pragma endregion
