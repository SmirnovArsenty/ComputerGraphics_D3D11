#include "buffer.h"
#include "d3d11_common.h"
#include "core/game.h"
#include "render/render.h"

#include <cassert>

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
    assert(resource_ == nullptr);
}

void Buffer::initialize(D3D11_BIND_FLAG bind_flags, void* data, UINT stride, UINT count, D3D11_USAGE usage)
{
    assert(resource_ == nullptr);

    strides_.push_back(stride);
    offsets_.push_back(0); /// TODO: handle offsets

    if (bind_flags == D3D11_BIND_INDEX_BUFFER && stride < 2) { // invalid input
        assert(false);
    }

    buffer_desc_.Usage = usage;
    buffer_desc_.BindFlags = bind_flags;
    buffer_desc_.CPUAccessFlags = 0;
    buffer_desc_.MiscFlags = 0;
    buffer_desc_.StructureByteStride = 0;
    buffer_desc_.ByteWidth = stride * count;

    subresource_data_.pSysMem = data;
    subresource_data_.SysMemPitch = 0;
    subresource_data_.SysMemSlicePitch = 0;

    auto device = Game::inst()->render().device();
    D3D11_CHECK(device->CreateBuffer(&buffer_desc_, &subresource_data_, &resource_));
}

void Buffer::bind(UINT slot)
{
    auto context = Game::inst()->render().context();

    if (buffer_desc_.BindFlags == D3D11_BIND_VERTEX_BUFFER)
    {
        context->IASetVertexBuffers(slot, 1, &resource_, strides_.data(), offsets_.data());
    }
    else if (buffer_desc_.BindFlags == D3D11_BIND_INDEX_BUFFER)
    {
        if (strides_[0] == sizeof(uint16_t)) {
            context->IASetIndexBuffer(resource_, DXGI_FORMAT_R16_UINT, 0);
        } else if (strides_[0] == sizeof(uint32_t)) {
            context->IASetIndexBuffer(resource_, DXGI_FORMAT_R32_UINT, 0);
        }
    }
}

void Buffer::destroy()
{
    SAFE_RELEASE(resource_);
}

UINT Buffer::count() const
{
    return buffer_desc_.ByteWidth / strides_[0];
}
