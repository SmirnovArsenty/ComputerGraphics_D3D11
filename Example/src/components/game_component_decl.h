#pragma once
#include <string>

#include "game_component.h"
#include "render/shader.h"

class TriangleComponent : public GameComponent
{
private:
    constexpr static char* resource_path_{ "./resources/triangle_component/" };

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

class GLTFModelComponent : public GameComponent
{
private:
    constexpr static char* resource_path_{ "./resources/gltfmodel_component/" };

    // path is relative to resource_path_
    std::string gltf_filename_;

    // The vertex layout for the samples' model
    /*struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec3 color;
    };

    // Single vertex buffer for all primitives
    struct {
        ID3D11Buffer* buffer;
    } vertices;

    // Single index buffer for all primitives
    struct {
        int count;
        ID3D11Buffer* buffer;
    } indices;*/

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
