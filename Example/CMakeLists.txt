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

set(sources 
    src/main.cpp
    
    src/core/game.cpp
    src/core/game.h

    src/render/render.cpp
    src/render/render.h
    
    src/render/buffer.cpp
    src/render/buffer.h
    src/render/shader.cpp
    src/render/shader.h
    src/render/camera.cpp
    src/render/camera.h

    src/components/game_component.h
    src/components/game_component_decl.h

    src/components/triangle_component.cpp
    src/components/gltfmodel_component.cpp

    src/win32/win.cpp
    src/win32/win.h
)

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
