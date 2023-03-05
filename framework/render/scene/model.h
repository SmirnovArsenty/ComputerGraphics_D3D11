#pragma once

#include <string>
#include <vector>

#include "SimpleMath.h"
using namespace DirectX::SimpleMath;

#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
    Model(const std::string& filename);

    void load();
    void unload();

    void set_transform(Vector3 position, Vector3 scale, Quaternion rotation);
    void draw() const;

private:
    // https://github.com/assimp/assimp/blob/master/samples/SimpleTexturedDirectx11/SimpleTexturedDirectx11/ModelLoader.cpp
    void load_node(aiNode* node, const aiScene* scene);
    void load_mesh(aiMesh* mesh, const aiScene* scene);
    void load_material(aiMaterial* material, const aiScene* scene);

    const std::string filename_; // model filename

    Matrix transform_;

    std::vector<class Mesh*> meshes_;
};
