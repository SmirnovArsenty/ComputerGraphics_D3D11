#include <cassert>

#include <assimp/Importer.hpp>

#include "core/game.h"
#include "render/render.h"
#include "render/camera.h"
#include "render/annotation.h"
#include "render/resource/texture.h"
#include "model.h"
#include "mesh.h"
#include "render/d3d11_common.h"

// public
Model::Model(const std::string& filename) :
    filename_{ filename },
    meshes_{},
    uniform_data_{ Matrix::Identity, Matrix::Identity }
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

    uniform_buffer_.initialize(sizeof(uniform_data_), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

void Model::unload()
{
    // check initialized
    assert(!meshes_.empty());

    uniform_buffer_.destroy();

    for (auto& mesh : meshes_) {
        mesh->destroy();
        delete mesh;
    }
    meshes_.clear();
}

void Model::set_transform(Vector3 position, Vector3 scale, Quaternion rotation)
{
    // uniform_data_.transform = ;
    uniform_data_.inverse_transpose_transform = uniform_data_.transform.Invert().Transpose();
    uniform_buffer_.update_data(&uniform_data_);
}

void Model::draw()
{
    Annotation annotation("draw:" + filename_);

    uniform_buffer_.bind(1);

    for (auto& mesh : meshes_) {
        mesh->draw();
    }
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

        vertex.position_uv_x.x = mesh->mVertices[i].x;
        vertex.position_uv_x.y = mesh->mVertices[i].y;
        vertex.position_uv_x.z = mesh->mVertices[i].z;

        vertex.normal_uv_y.x = mesh->mNormals[i].x;
        vertex.normal_uv_y.y = mesh->mNormals[i].y;
        vertex.normal_uv_y.z = mesh->mNormals[i].z;

        if (mesh->mTextureCoords[0]) {
            vertex.position_uv_x.w = (float)mesh->mTextureCoords[0][i].x;
            vertex.normal_uv_y.w = (float)mesh->mTextureCoords[0][i].y;
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        auto& face = mesh->mFaces[i];

        for (uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    Material* material = nullptr;

    if (mesh->mMaterialIndex >= 0) {
        auto mat = scene->mMaterials[mesh->mMaterialIndex];
        material = new Material(mat->GetName().C_Str());

        auto load_texture = [&scene](aiString& str) -> Texture* {
            auto texture = new Texture();
            auto embedded_texture = scene->GetEmbeddedTexture(str.C_Str());
            if (embedded_texture != nullptr) {
                if (embedded_texture->mHeight != 0) {
                    texture->initialize(embedded_texture->mWidth, embedded_texture->mHeight, DXGI_FORMAT_B8G8R8A8_UNORM, embedded_texture->pcData);
                }
            } else {
                texture->load(str.C_Str());
            }
            return texture;
        };

        // diffuse
        for (uint32_t i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); ++i) {
            aiString str;
            mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
            material->set_diffuse(load_texture(str));
        }

        // specular
        for (uint32_t i = 0; i < mat->GetTextureCount(aiTextureType_SPECULAR); ++i) {
            aiString str;
            mat->GetTexture(aiTextureType_SPECULAR, i, &str);
            material->set_specular(load_texture(str));
        }

        // ambient
        for (uint32_t i = 0; i < mat->GetTextureCount(aiTextureType_AMBIENT); ++i) {
            aiString str;
            mat->GetTexture(aiTextureType_AMBIENT, i, &str);
            material->set_ambient(load_texture(str));
        }
    }

    meshes_.push_back(new Mesh(vertices, indices, material));
}
