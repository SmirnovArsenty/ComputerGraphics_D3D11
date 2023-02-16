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
