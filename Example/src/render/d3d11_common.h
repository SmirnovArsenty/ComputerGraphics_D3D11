#pragma once

#include <cassert>

#define D3D11_CHECK(command)                                                                                        \
    do {                                                                                                            \
        HRESULT status = command;                                                                                   \
        if (FAILED(status)) {                                                                                       \
            size_t count = wsprintf(nullptr, L"D3D11 error at " __FILE__ L":%d : %d", __LINE__, status);            \
            std::wstring info;                                                                                      \
            info.resize(count);                                                                                     \
            wsprintf(const_cast<wchar_t*>(info.data()), L"D3D11 error at " __FILE__ L":%d : %d", __LINE__, status); \
            OutputDebugString(info.c_str());                                                                        \
            assert(false);                                                                                          \
        }                                                                                                           \
    } while(0, 0)