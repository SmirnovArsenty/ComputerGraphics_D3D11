#pragma once

#include <d3d11.h>
#include <vector>
#include <string>

class Buffer
{
private:
    ID3D11Buffer* resource_{ nullptr };

    D3D11_BUFFER_DESC buffer_desc_{};
    D3D11_SUBRESOURCE_DATA subresource_data_{};

    std::vector<UINT> strides_{};
    std::vector<UINT> offsets_{};

public:
    Buffer();
    ~Buffer();

    void initialize(D3D11_BIND_FLAG bind_flags, void* data, UINT stride, UINT count, D3D11_USAGE usage = D3D11_USAGE_DEFAULT);
    void set_name(const std::string& name);
    void bind(UINT slot = 0);
    void destroy();

    UINT count() const;
};
