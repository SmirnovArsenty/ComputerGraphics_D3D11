#pragma once

#include <string>
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "component/game_component.h"
#include "render/resource/shader.h"
#include "render/resource/buffer.h"

namespace tinygltf
{
class Model;
class Node;
}

class GLTFModelComponent : public GameComponent
{
private:
    constexpr static char* resource_path_{ "./resources/gltfmodel_component/" };
    static std::string shader_source_;

    // path is relative to resource_path_
    std::string gltf_filename_;

    // following structures taken here:
    // https://github.com/SaschaWillems/Vulkan/blob/master/examples/gltfloading/gltfloading.cpp

    // The vertex layout for the samples' model
    struct Vertex {
        Vector4 pos_uv_x;
        Vector4 normal_uv_y;
        Vector4 color;
    };

    // Single vertex buffer for all primitives
    struct {
        Buffer buffer;
        std::vector<GLTFModelComponent::Vertex> vertex_buffer_raw;
    } vertices_;

    // Single index buffer for all primitives
    struct {
        Buffer buffer;
        std::vector<uint32_t> index_buffer_raw;
    } indices_;

    // A primitive contains the data for a single draw call
    struct Primitive {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t materialIndex;
    };

    // Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
    struct Mesh {
        std::vector<Primitive> primitives;
    };

    // A node represents an object in the glTF scene graph
    struct Node {
        Node* parent;
        std::vector<Node*> children;
        Mesh mesh;
        Matrix matrix;
        ~Node() {
            for (auto& child : children) {
                delete child;
            }
        }
    };

    // A glTF material stores information in e.g. the texture that is attached to it and colors
    // struct Material {
    //     Vector4 baseColorFactor = Vector4(1.0f);
    //     uint32_t baseColorTextureIndex;
    // };

    // // Contains the texture for a single glTF image
    // // Images may be reused by texture objects and are as such separated
    // struct Image {
    //     vks::Texture2D texture;
    //     // We also store (and create) a descriptor set that's used to access this texture from the fragment shader
    //     VkDescriptorSet descriptorSet;
    // };

    // // A glTF texture stores a reference to the image and a sampler
    // // In this sample, we are only interested in the image
    // struct Texture {
    //     int32_t imageIndex;
    // };

    // model data
    // std::vector<Image> images_;
    // std::vector<Texture> textures_;
    // std::vector<Material> materials_;
    std::vector<Node> nodes_;

    Shader shader_;

    ID3D11RasterizerState* rasterizer_state_{ nullptr };

    struct UniformData {
        Matrix model;
        Matrix view_proj;
        Vector3 camera_pos;
        float res_0;
        Vector3 camera_dir;
        float res_1;
    };

    ConstBuffer uniform_buffer_;

    Matrix model_transform_;

    void load_node(tinygltf::Model* model, tinygltf::Node* input_node, GLTFModelComponent::Node* parent);
public:
    GLTFModelComponent(const std::string& filename, Vector3 position = Vector3(0.f), Quaternion rotation = Quaternion::Identity, Vector3 scale = Vector3(1.f));
    ~GLTFModelComponent();

    void initialize() override;
    void draw() override;
    void imgui() override;
    void reload() override;
    void update() override;
    void destroy_resources() override;
};
