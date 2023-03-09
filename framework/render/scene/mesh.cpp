#include "core/game.h"
#include "render/render.h"
#include "mesh.h"

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Material* material) :
    vertices_{ vertices }, indices_{ indices }, material_{ material }
{
}

Mesh::~Mesh()
{
    if (material_ != nullptr) {
        material_->destroy();
        delete material_;
        material_ = nullptr;
    }
}

void Mesh::initialize()
{
    vertex_buffer_.initialize(D3D11_BIND_VERTEX_BUFFER, vertices_.data(), sizeof(vertices_[0]), static_cast<UINT>(vertices_.size()));
    index_buffer_.initialize(D3D11_BIND_INDEX_BUFFER, indices_.data(), sizeof(indices_[0]), static_cast<UINT>(indices_.size()));
#ifndef NDEBUG
    vertex_buffer_.set_name("vertex_buffer");
    index_buffer_.set_name("index_buffer");
#endif

    if (material_->get_diffuse()) {
        uniform_data_.material_flags |= 1 << 0;
    }

    if (material_->get_specular()) {
        uniform_data_.material_flags |= 1 << 1;
    }

    if (material_->get_ambient()) {
        uniform_data_.material_flags |= 1 << 2;
    }

    uniform_buffer_.initialize(sizeof(uniform_data_), D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    uniform_buffer_.update_data(&uniform_data_);
}

void Mesh::destroy()
{
    uniform_buffer_.destroy();
    index_buffer_.destroy();
    vertex_buffer_.destroy();
}

void Mesh::draw()
{
    uniform_buffer_.bind(2);

    vertex_buffer_.bind(0);
    index_buffer_.bind(0);

    material_->bind();

    auto context = Game::inst()->render().context();
    context->DrawIndexed(index_buffer_.count(), 0, 0);
}
