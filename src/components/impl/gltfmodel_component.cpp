#include <cassert>
#include <vector>
#include <fstream>
#include <algorithm>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/render.h"
#include "render/camera.h"
#include "render/annotation.h"
#include "core/game.h"
#include "render/d3d11_common.h"
#include "gltfmodel_component.h"

GLTFModelComponent::GLTFModelComponent(const std::string& filename, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : gltf_filename_{ filename }
{
    // must not be found
    assert(gltf_filename_.find(resource_path_) == std::string::npos);

    // initalize model transform matrix
    (void)scale; // unused
    model_transform_ = glm::identity<glm::mat4>();
    model_transform_ = glm::translate(model_transform_, position) * glm::scale(model_transform_, scale);
}

GLTFModelComponent::~GLTFModelComponent()
{
}

namespace {
namespace gltf_loader_callbacks
{
static bool TINYGLTF_FileExistsFunction(const std::string& abs_filename, void* user_data)
{ // just stub
    return true;
}
static std::string TINYGLTF_ExpandFilePathFunction(const std::string& file_path, void* user_data)
{
    return file_path;
}
static bool TINYGLTF_ReadWholeFileFunction(std::vector<unsigned char>* data, std::string* fileerr, const std::string& file_name, void* user_data)
{
    std::vector<uint8_t> data_;

    std::ifstream file;

    file.open(file_name, std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("read_binary_file: Failed to open file: " + file_name);
        return false;
    }

    file.seekg(0, std::ios::end);
    uint64_t read_count = static_cast<uint64_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    data_.resize(static_cast<size_t>(read_count));
    file.read(reinterpret_cast<char *>(data_.data()), read_count);
    file.close();
    
    *data = data_;
    return true;
}
static bool TINYGLTF_WriteWholeFileFunction(std::string*, const std::string&, const std::vector<unsigned char>&, void*)
{
    // do not write any gltf
    assert(false);
    return true;
}
}
}

void GLTFModelComponent::load_node(tinygltf::Model* model, tinygltf::Node* input_node, GLTFModelComponent::Node* parent)
{
    nodes_.push_back(GLTFModelComponent::Node{});
    auto& new_node = nodes_.back();
    new_node.matrix = glm::mat4(1.f);
    new_node.parent = parent;

    if (input_node->translation.size() == 3) {
        new_node.matrix = glm::translate(new_node.matrix, glm::vec3(glm::make_vec3(input_node->translation.data())));
    }
    if (input_node->rotation.size() == 4) {
        glm::quat q = glm::make_quat(input_node->rotation.data());
        new_node.matrix *= glm::mat4(q);
    }
    if (input_node->scale.size() == 3) {
        new_node.matrix *= glm::scale(new_node.matrix, glm::vec3(glm::make_vec3(input_node->scale.data())));
    }
    if (input_node->matrix.size() == 16) {
        new_node.matrix = glm::make_mat4x4(input_node->matrix.data());
    }

    for (auto& child : input_node->children)
    {
        load_node(model, &(model->nodes[child]), &new_node);
    }

    auto get_attribute_data = [&model](uint32_t id) -> float* {
        auto& accessor = model->accessors[id];
        auto& buffer_view = model->bufferViews[accessor.bufferView];
        auto& buffer = model->buffers[buffer_view.buffer];

        size_t start_byte = accessor.byteOffset + buffer_view.byteOffset;

        return reinterpret_cast<float*>(&(buffer.data[start_byte]));
    };

    // check mesh data
    if (input_node->mesh > -1)
    {
        auto& mesh = model->meshes[input_node->mesh];
        for (auto& primitive : mesh.primitives) {
            uint32_t first_index = static_cast<uint32_t>(indices_.index_buffer_raw.size());
            uint32_t vertex_start = static_cast<uint32_t>(vertices_.vertex_buffer_raw.size());
            uint32_t index_count = 0;

            const float* position_buffer{ nullptr };
            const float* normals_buffer{ nullptr };
            const float* tex_coords_buffer{ nullptr };
            const float* color_buffer{ nullptr };
            size_t vertex_count = 0;

            for (auto& attribute : primitive.attributes)
            {
                std::string attribute_name = attribute.first;
                std::transform(attribute_name.begin(), attribute_name.end(), attribute_name.begin(), ::tolower);

                auto attribute_data = get_attribute_data(attribute.second);

                if (attribute_name == "position") {
                    vertex_count = model->accessors[attribute.second].count;
                    position_buffer = attribute_data;
                } else if (attribute_name == "normal") {
                    normals_buffer = attribute_data;
                } else if (attribute_name == "texcoord_0") {
                    tex_coords_buffer = attribute_data;
                } else if (attribute_name == "joints_0") {
                    /// TODO
                } else if (attribute_name == "weights_0") {
                    /// TODO
                } else if (attribute_name == "tangent") {
                    /// TODO
                } else if (attribute_name == "color_0") {
                    color_buffer = attribute_data;
                }
            }

            // Assemble vertex data
            for (size_t i = 0; i < vertex_count; ++i)
            {
                Vertex v{};
                glm::vec2 uv = tex_coords_buffer ? glm::make_vec2(&tex_coords_buffer[i * 2]) : glm::vec2(0.f);
                v.pos_uv_x = glm::vec4(glm::make_vec3(&position_buffer[i * 3]), uv.x);
                v.normal_uv_y = glm::vec4(glm::normalize(normals_buffer ? glm::make_vec3(&normals_buffer[i * 3]) : glm::vec3(0.f)), uv.y);
                /// TODO: remove this sin cos color
                v.color = glm::vec4(1.f); // color_buffer ? glm::make_vec4(&color_buffer[i * 4]) : glm::vec4(sinf(vertex_count), cosf(vertex_count), sinf(vertex_count * 1.3), 1.f);
                vertices_.vertex_buffer_raw.push_back(v);
            }

            // indices
            {
                auto& index_accessor = model->accessors[primitive.indices];
                auto& index_buffer_view = model->bufferViews[index_accessor.bufferView];
                auto& index_buffer = model->buffers[index_buffer_view.buffer];

                index_count += static_cast<uint32_t>(index_accessor.count);
                switch (index_accessor.componentType)
                {
                    case TINYGLTF_COMPONENT_TYPE_INT:
                    {
                        const uint32_t* buf = reinterpret_cast<uint32_t*>(&index_buffer.data[index_accessor.byteOffset + index_buffer_view.byteOffset]);
                        for (size_t index = 0; index < index_accessor.count; ++index) {
                            indices_.index_buffer_raw.push_back(buf[index] + vertex_start);
                        }
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* buf = reinterpret_cast<uint16_t*>(&index_buffer.data[index_accessor.byteOffset + index_buffer_view.byteOffset]);
                        for (size_t index = 0; index < index_accessor.count; ++index) {
                            indices_.index_buffer_raw.push_back(buf[index] + vertex_start);
                        }
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    {
                        const uint8_t* buf = reinterpret_cast<uint8_t*>(&index_buffer.data[index_accessor.byteOffset + index_buffer_view.byteOffset]);
                        for (size_t index = 0; index < index_accessor.count; ++index) {
                            indices_.index_buffer_raw.push_back(buf[index] + vertex_start);
                        }
                        break;
                    }
                    default:
                    {
                        throw std::runtime_error("Index component type " + std::to_string(index_accessor.componentType) + " not supported!");
                        return;
                    }
                }
            }
            Primitive prim{};
            prim.firstIndex = first_index;
            prim.indexCount = index_count;
            prim.materialIndex = primitive.material;
            new_node.mesh.primitives.push_back(prim);
        }
    }
}

void GLTFModelComponent::initialize()
{
    // load model from gltf_filename_
    tinygltf::TinyGLTF gltf_loader;
    tinygltf::FsCallbacks gltf_fs_callbacks = {
        gltf_loader_callbacks::TINYGLTF_FileExistsFunction,
        gltf_loader_callbacks::TINYGLTF_ExpandFilePathFunction,
        gltf_loader_callbacks::TINYGLTF_ReadWholeFileFunction,
        gltf_loader_callbacks::TINYGLTF_WriteWholeFileFunction,
        nullptr
    };
    gltf_loader.SetFsCallbacks(gltf_fs_callbacks);

    tinygltf::Model import_model;
    std::string err, warn;
    bool status = gltf_loader.LoadASCIIFromFile(&import_model, &err, &warn, resource_path_ + gltf_filename_, 0u);
    if (!status) {
        throw std::runtime_error("Can not load gltf file: " + err);
    }

    load_node(&import_model, &import_model.nodes[0], nullptr);

    vertices_.buffer.initialize(D3D11_BIND_VERTEX_BUFFER, vertices_.vertex_buffer_raw.data(), sizeof(vertices_.vertex_buffer_raw[0]), static_cast<UINT>(vertices_.vertex_buffer_raw.size()));
    indices_.buffer.initialize(D3D11_BIND_INDEX_BUFFER, indices_.index_buffer_raw.data(), sizeof(indices_.index_buffer_raw[0]), static_cast<UINT>(indices_.index_buffer_raw.size()));
    vertices_.buffer.set_name("gltf_vertex_buffer");
    indices_.buffer.set_name("gltf_index_buffer");

    shader_.set_vs_shader((std::string(resource_path_) + "shaders/shader.hlsl").c_str(),
                          "VSMain", nullptr, nullptr);
    shader_.set_ps_shader((std::string(resource_path_) + "shaders/shader.hlsl").c_str(),
                          "PSMain", nullptr, nullptr);

    D3D11_INPUT_ELEMENT_DESC inputs[] = {
        D3D11_INPUT_ELEMENT_DESC {
            "POSITION_UV_X", 0,
            DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        D3D11_INPUT_ELEMENT_DESC {
            "NORMAL_UV_Y", 0,
            DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        D3D11_INPUT_ELEMENT_DESC {
            "COLOR", 0,
            DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    shader_.set_input_layout(inputs, static_cast<uint32_t>(std::size(inputs)));

    shader_.set_name("gltf_shader");

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateRasterizerState(&rastDesc, &rasterizer_state_));

    uniform_buffer_.initialize(sizeof(UniformData), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    uniform_buffer_.set_name("gltf_uniform_buffer");
}

void GLTFModelComponent::draw()
{
    Annotation annotation("gltf model draw");
    auto context = Game::inst()->render().context();
    auto device = Game::inst()->render().device();

    shader_.use();

    context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    vertices_.buffer.bind();
    indices_.buffer.bind();

    const Camera* camera = Game::inst()->render().camera();
    UniformData data = {};
    data.model = model_transform_;
    data.view_proj = camera->view_proj();
    data.camera_pos = camera->position();
    data.camera_dir = camera->direction();
    uniform_buffer_.update_data(&data);
    uniform_buffer_.bind(0);

    context->RSSetState(rasterizer_state_);

    for (auto& node : nodes_)
    {
        for (auto& primitive: node.mesh.primitives)
        {
            context->DrawIndexed(primitive.indexCount, primitive.firstIndex, 0);
        }
    }
}

void GLTFModelComponent::reload()
{
}

void GLTFModelComponent::update()
{
}

void GLTFModelComponent::destroy_resources()
{
    vertices_.buffer.destroy();
    indices_.buffer.destroy();

    SAFE_RELEASE(rasterizer_state_);
    uniform_buffer_.destroy();
}
