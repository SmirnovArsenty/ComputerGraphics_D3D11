cmake_minimum_required(VERSION 3.8)

project(ComputerGraphics_D3D11)

### dependencies
add_subdirectory(third_party)
add_subdirectory(framework)

### setup triangle draw build
set(group_component_triangle
    src/components/triangle/triangle_component.cpp
    src/components/triangle/triangle_component.h
)
set(group_component_triangle_main
    src/triangle_main.cpp
)
set(triangle_sources
    ${group_component_triangle}
    ${group_component_triangle_main}
)
source_group("" FILES ${group_component_triangle_main})
source_group("component" FILES ${group_component_triangle})
add_executable(triangle WIN32 ${triangle_sources})
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
    src/components/gltfmodel/gltfmodel_component.cpp
    src/components/gltfmodel/gltfmodel_component.h
)
set(group_component_gltf_main
    src/gltf_main.cpp
)
set(gltf_sources
    ${group_component_gltf}
    ${group_component_gltf_main}
)
source_group("" FILES ${group_component_gltf_main})
source_group("component" FILES ${group_component_gltf})
add_executable(gltf WIN32 ${gltf_sources})
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
    src/components/pingpong/pingpong_component.cpp
    src/components/pingpong/pingpong_component.h
)
set(group_component_pingpong_main
    src/pingpong_main.cpp
)
set(pingpong_sources
    ${group_component_pingpong}
    ${group_component_pingpong_main}
)
source_group("" FILES ${group_component_pingpong_main})
source_group("component" FILES ${group_component_pingpong})
add_executable(pingpong WIN32 ${pingpong_sources})
target_include_directories(pingpong
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework)
target_link_libraries(pingpong
    framework
)
set_property(TARGET pingpong PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
add_custom_command(TARGET pingpong POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pingpong> ${CMAKE_CURRENT_SOURCE_DIR})

### setup orbit build
set(group_component_orbit
    src/components/orbit/orbit_component.cpp
    src/components/orbit/orbit_component.h
)
set(group_component_orbit_main
    src/orbit_main.cpp
)
set(orbit_sources
    ${group_component_orbit}
    ${group_component_orbit_main}
)
source_group("" FILES ${group_component_orbit_main})
source_group("component" FILES ${group_component_orbit})
add_executable(orbit WIN32 ${orbit_sources})
target_include_directories(orbit
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework)
target_link_libraries(orbit
    framework
)
set_property(TARGET orbit PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
add_custom_command(TARGET orbit POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:orbit> ${CMAKE_CURRENT_SOURCE_DIR})
