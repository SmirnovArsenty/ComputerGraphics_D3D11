cmake_minimum_required(VERSION 3.8)

project(framework)

### dependencies

### end of dependencies

set(group_core
    core/game.cpp
    core/game.h
)

set(group_render
    render/render.cpp
    render/render.h

    render/annotation.cpp
    render/annotation.h

    render/camera.cpp
    render/camera.h
)

set(group_render_resource
    render/resource/buffer.cpp
    render/resource/buffer.h
    render/resource/shader.cpp
    render/resource/shader.h
    render/resource/texture.cpp
    render/resource/texture.h
)

set(group_component_common
    component/game_component.h
)

set(group_win32
    win32/input.cpp
    win32/input.h
    win32/win.cpp
    win32/win.h
)

set(sources
    ${group_core}

    ${group_render}
    ${group_render_resource}

    ${group_component_common}

    ${group_component_impl}

    ${group_win32}
)

source_group("core" FILES ${group_core})
source_group("render" FILES ${group_render})
source_group("render/resource" FILES ${group_render_resource})
source_group("component/common" FILES ${group_component_common})
source_group("win32" FILES ${group_win32})

add_library(framework STATIC ${sources})
target_link_libraries(framework
    tinygltf
    directxtk
    imgui
)
target_include_directories(framework
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/
)
