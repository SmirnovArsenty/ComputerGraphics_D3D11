cmake_minimum_required(VERSION 3.8)

project(ComputerGraphics_D3D11)

### dependencies
add_subdirectory(third_party)
add_subdirectory(framework)

### setup triangle draw build
set(group_component_triangle
    src/components/triangle_component.cpp
    src/components/triangle_component.h

    src/triangle_main.cpp
)
add_executable(triangle WIN32 ${group_component_triangle})
target_include_directories(triangle
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework)
target_link_libraries(triangle
    framework
)
set_property(TARGET triangle PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
add_custom_command(TARGET triangle POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:triangle> ${CMAKE_CURRENT_SOURCE_DIR})

### setup gltf draw build
set(group_component_gltf
    src/components/gltfmodel_component.cpp
    src/components/gltfmodel_component.h

    src/gltf_main.cpp
)
add_executable(gltf WIN32 ${group_component_gltf})
target_include_directories(gltf
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework)
target_link_libraries(gltf
    framework
)
set_property(TARGET gltf PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
add_custom_command(TARGET gltf POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gltf> ${CMAKE_CURRENT_SOURCE_DIR})

### setup pingpong game build
set(group_component_pingpong
    src/components/pingpong_component.cpp
    src/components/pingpong_component.h

    src/pingpong_main.cpp
)
add_executable(pingpong WIN32 ${group_component_pingpong})
target_include_directories(pingpong
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework)
target_link_libraries(pingpong
    framework
)
set_property(TARGET pingpong PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
add_custom_command(TARGET pingpong POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pingpong> ${CMAKE_CURRENT_SOURCE_DIR})
