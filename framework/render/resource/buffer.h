#pragma once

#include <d3d11.h>
#include <vector>
#include <string>

class Buffer
{
protected:
    ID3D11Buffer* resource_{ nullptr };

    D3D11_BUFFER_DESC buffer_desc_{};
    D3D11_SUBRESOURCE_DATA subresource_data_{};

    std::vector<UINT> strides_{};
    std::vector<UINT> offsets_{};

public:
    Buffer();
    ~Buffer();

    void initialize(D3D11_BIND_FLAG bind_flags, void* data, UINT stride, UINT count, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, D3D11_CPU_ACCESS_FLAG cpu_access = (D3D11_CPU_ACCESS_FLAG)0);
    void set_name(const std::string& name);
    void bind(UINT slot = 0);
    void destroy();

    UINT count() const;
};

class ConstBuffer : public Buffer
{
public:
    ConstBuffer() = default;
    void initialize(UINT size, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, D3D11_CPU_ACCESS_FLAG cpu_access = D3D11_CPU_ACCESS_WRITE);
    void update_data(void* data);
};
