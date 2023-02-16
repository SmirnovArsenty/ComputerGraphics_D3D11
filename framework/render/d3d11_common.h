#pragma once

#include <cassert>

#define D3D11_CHECK(command)                                                                                           \
    do {                                                                                                               \
        HRESULT status = command;                                                                                      \
        if (FAILED(status)) {                                                                                          \
            size_t count = sprintf_s(nullptr, 0, "D3D11 error at " __FILE__ ":%d : %d", __LINE__, status);       \
            std::string info;                                                                                          \
            info.resize(count);                                                                                        \
            sprintf_s(const_cast<char*>(info.data()), count, "D3D11 error at " __FILE__ ":%d : %d", __LINE__, status); \
            OutputDebugString(info.c_str());                                                                           \
            assert(false);                                                                                             \
        }                                                                                                              \
    } while(0, 0)

#define SAFE_RELEASE(ptr)       \
    do {                        \
        if (ptr != nullptr) {   \
            (ptr)->Release();   \
        }                       \
        (ptr = nullptr);        \
    } while (0, 0)
