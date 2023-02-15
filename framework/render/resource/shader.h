#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <string>
#include <vector>

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

    void set_name(const std::string& name);

    void set_vs_shader_from_file(const std::string& filename,
                                 const std::string& entrypoint,
                                 D3D_SHADER_MACRO*, ID3DInclude*);
    void set_ps_shader_from_file(const std::string& filename,
                                 const std::string& entrypoint,
                                 D3D_SHADER_MACRO*, ID3DInclude*);

    void set_vs_shader_from_memory(const std::string& data,
                                   const std::string& entrypoint,
                                   D3D_SHADER_MACRO*, ID3DInclude*);
    void set_ps_shader_from_memory(const std::string& data,
                                   const std::string& entrypoint,
                                   D3D_SHADER_MACRO*, ID3DInclude*);

    void set_input_layout(D3D11_INPUT_ELEMENT_DESC*, size_t);

    void use();

    void destroy();
};
