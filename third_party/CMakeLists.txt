cmake_minimum_required(VERSION 3.8)

project(third_party)

# tinygltf
add_library(tinygltf INTERFACE)
set(TINYGLTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)
target_sources(tinygltf INTERFACE ${TINYGLTF_DIR}/tiny_gltf.h ${TINYGLTF_DIR}/json.hpp)
target_include_directories(tinygltf INTERFACE ${TINYGLTF_DIR})

# directxtk
set(directxtk_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/directxtk/Inc/SimpleMath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/directxtk/Inc/SimpleMath.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/directxtk/Src/SimpleMath.cpp
)
add_library(directxtk STATIC ${directxtk_sources})
target_include_directories(directxtk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/directxtk/Inc)

# imgui
set(imgui_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_dx11.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_dx11.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.h
)
add_library(imgui STATIC ${imgui_sources})
target_include_directories(imgui
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)
