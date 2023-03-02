#pragma once

#include <vector>

#include <SimpleMath.h>
using namespace DirectX::SimpleMath;

#include "render/resource/buffer.h"
#include "render/resource/texture.h"
#include "material.h"

struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
};

class Mesh
{
public:
    Mesh(const std::vector<Buffer>& vertices, const std::vector<Buffer>& indices, const std::vector<Texture>& textures);
private:

};
