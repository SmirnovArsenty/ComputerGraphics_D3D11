#include <cassert>

#include <assimp/Importer.hpp>

#include "model.h"
#include "mesh.h"

// public
Model::Model(const std::string& filename) : filename_{ filename }, meshes_{}
{
}

void Model::load()
{
    // check not initialized
    assert(meshes_.empty());

    Assimp::Importer importer;
    auto scene = importer.ReadFile(filename_, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
    assert(scene != nullptr);
    load_node(scene->mRootNode, scene);
}

void Model::unload()
{
    // check initialized
    assert(!meshes_.empty());
}

// private
void Model::load_node(aiNode* node, const aiScene* scene)
{
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        auto mesh = scene->mMeshes[node->mMeshes[i]];
        load_mesh(mesh, scene);
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        load_node(node->mChildren[i], scene);
    }
}

void Model::load_mesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        if (mesh->mTextureCoords[0]) {
            vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
            vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        auto& face = mesh->mFaces[i];

        for (uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex >= 0) {
        auto mat = scene->mMaterials[mesh->mMaterialIndex];
        Material material(mat->GetName().C_Str());

        // diffuse
        for (uint32_t i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); ++i) {
            aiString str;
            mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
            auto texture = new Texture(str.C_Str());
            auto embedded_texture = scene->GetEmbeddedTexture(str.C_Str());
            if (embedded_texture != nullptr) {
                
            } else {
                std::string tex_filename(str.C_Str());
            }

            material.set_diffuse(texture);

        }

        // specular


        for (uint32_t i = 0; i < mat->mNumProperties; ++i) {
            auto prop = mat->mProperties[i];

        }
    }
}
