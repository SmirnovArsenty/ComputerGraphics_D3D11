#pragma once
#include <string>

#include "game_component.h"

class TriangleComponent : public GameComponent
{
private:
    constexpr static wchar_t* resource_path_{ L"./resources/triangle_component/" };

    Shader shader_;

    ID3D11Buffer* vertex_buffer_{ nullptr };
    ID3D11Buffer* index_buffer_{ nullptr };

    ID3D11RasterizerState* rasterizer_state_{ nullptr };

public:
    TriangleComponent();
    ~TriangleComponent();

    void initialize() override;
    void draw() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
};

class GLTFModelComponent : GameComponent
{
private:
    constexpr static wchar_t* resource_path_{ L"./resources/gltfmodel_component/" };

    // path is relative to resource_path_
    std::wstring gltf_filename_;

    Shader shader_;

    ID3D11Buffer* vertex_buffer_{ nullptr };
    ID3D11Buffer* index_buffer_{ nullptr };

    ID3D11RasterizerState* rasterizer_state_{ nullptr };
public:
    GLTFModelComponent(const std::string& filename);
    ~GLTFModelComponent();

    void initialize() override;
    void draw() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;

};
