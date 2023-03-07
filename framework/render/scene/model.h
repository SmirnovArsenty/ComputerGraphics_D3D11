#pragma once

#include <string>
#include <vector>

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "render/resource/shader.h"
#include "render/resource/buffer.h"

class Model
{
public:
    Model(const std::string& filename);

    void load();
    void unload();

    void set_transform(Vector3 position, Vector3 scale, Quaternion rotation);
    void draw();

private:
    // https://github.com/assimp/assimp/blob/master/samples/SimpleTexturedDirectx11/SimpleTexturedDirectx11/ModelLoader.cpp
    void load_node(aiNode* node, const aiScene* scene);
    void load_mesh(aiMesh* mesh, const aiScene* scene);
    void load_material(aiMaterial* material, const aiScene* scene);

    const std::string filename_; // model filename

    std::vector<class Mesh*> meshes_;

    ConstBuffer uniform_buffer_;
    struct {
        Matrix transform;
        Matrix inverse_transpose_transform;
    } uniform_data_;
};
