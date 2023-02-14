cmake_minimum_required(VERSION 3.8)

project(D3D11_Example)

### dependencies

# tinygltf
add_library(tinygltf INTERFACE)
set(TINYGLTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/tinygltf)
target_sources(tinygltf INTERFACE ${TINYGLTF_DIR}/tiny_gltf.h ${TINYGLTF_DIR}/json.hpp)
target_include_directories(tinygltf INTERFACE ${TINYGLTF_DIR})

# glm
add_library(glm INTERFACE)
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/glm)
target_sources(glm INTERFACE ${GLM_DIR}/glm/glm.hpp)
target_include_directories(glm INTERFACE ${GLM_DIR})

### end of dependencies

set(group_core
    src/core/game.cpp
    src/core/game.h
)

set(group_render
    src/render/render.cpp
    src/render/render.h

    src/render/annotation.cpp
    src/render/annotation.h

    src/render/camera.cpp
    src/render/camera.h
)

set(group_render_resource
    src/render/resource/buffer.cpp
    src/render/resource/buffer.h
    src/render/resource/shader.cpp
    src/render/resource/shader.h
    src/render/resource/texture.cpp
    src/render/resource/texture.h
)

set(group_component_common
    src/components/common/game_component.h
    src/components/common/game_component_decl.h
)

set(group_component_impl
    src/components/impl/triangle_component.cpp
    src/components/impl/triangle_component.h
    src/components/impl/gltfmodel_component.cpp
    src/components/impl/gltfmodel_component.h
    src/components/impl/pingpong_component.cpp
    src/components/impl/pingpong_component.h
)

set(group_win32
    src/win32/win.cpp
    src/win32/win.h
)

set(sources
    src/main.cpp

    ${group_core}

    ${group_render}
    ${group_render_resource}

    ${group_component_common}

    ${group_component_impl}

    ${group_win32}
)

source_group("" FILES src/main.cpp)
source_group("core" FILES ${group_core})
source_group("render" FILES ${group_render})
source_group("render/resource" FILES ${group_render_resource})
source_group("component/common" FILES ${group_component_common})
source_group("component/impl" FILES ${group_component_impl})
source_group("win32" FILES ${group_win32})

add_executable(${CMAKE_PROJECT_NAME} WIN32 ${sources})

target_link_libraries(${CMAKE_PROJECT_NAME}
    tinygltf
    glm
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# it is important to find 'shaders' folder
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR})
