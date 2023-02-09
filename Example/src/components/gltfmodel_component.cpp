#include <cassert>
#include <vector>
#include <fstream>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include "render/render.h"
#include "core/game.h"
#include "render/d3d11_common.h"
#include "components/game_component_decl.h"

GLTFModelComponent::GLTFModelComponent(const std::string& filename) : gltf_filename_{ filename }
{
    // must not be found
    assert(gltf_filename_.find(resource_path_) == std::string::npos);
}

GLTFModelComponent::~GLTFModelComponent()
{
}

// gltf loader callbacks
bool TINYGLTF_FileExistsFunction(const std::string& abs_filename, void* user_data)
{ // just stub
    return true;
}
std::string TINYGLTF_ExpandFilePathFunction(const std::string& file_path, void* user_data)
{
    return file_path;
}
bool TINYGLTF_ReadWholeFileFunction(std::vector<unsigned char>* data, std::string* fileerr, const std::string& file_name, void* user_data)
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
bool TINYGLTF_WriteWholeFileFunction(std::string*, const std::string&, const std::vector<unsigned char>&, void*)
{
    // do not write any gltf
    assert(false);
    return true;
}

void GLTFModelComponent::initialize()
{
    // load model from gltf_filename_
    tinygltf::TinyGLTF gltf_loader;
    tinygltf::FsCallbacks gltf_fs_callbacks = {
        TINYGLTF_FileExistsFunction,
        TINYGLTF_ExpandFilePathFunction,
        TINYGLTF_ReadWholeFileFunction,
        TINYGLTF_WriteWholeFileFunction,
        nullptr
    };
    gltf_loader.SetFsCallbacks(gltf_fs_callbacks);

    tinygltf::Model import_model;
    std::string err, warn;
    bool status = gltf_loader.LoadASCIIFromFile(&import_model, &err, &warn, resource_path_ + gltf_filename_, 0u);

    for (auto& mesh : import_model.meshes)
    {
        for (auto& primitive : mesh.primitives)
        {
            primitive;
        }
    }
}

void GLTFModelComponent::draw()
{
}

void GLTFModelComponent::reload()
{
}

void GLTFModelComponent::update()
{
}

void GLTFModelComponent::destroy_resources()
{
}
