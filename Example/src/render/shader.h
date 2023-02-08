#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <string>

class Shader
{
private:
    ID3D11VertexShader* vertex_shader_{ nullptr };
    ID3DBlob* vertex_bc_{ nullptr };
    ID3D11PixelShader* pixel_shader_{ nullptr };
    ID3DBlob* pixel_bc_{ nullptr };

    ID3D11InputLayout* input_layout_{ nullptr };
public:
    Shader();
    ~Shader();

    void set_vs_shader(const std::wstring& filename,
                       const std::string& vs_entrypoint,
                       D3D_SHADER_MACRO*, ID3DInclude*);
    void set_ps_shader(const std::wstring& filename,
                       const std::string& ps_entrypoint,
                       D3D_SHADER_MACRO*, ID3DInclude*);

    void set_input_layout(D3D11_INPUT_ELEMENT_DESC*, uint32_t);

    void use();
};
