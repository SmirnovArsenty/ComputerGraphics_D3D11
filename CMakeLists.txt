cmake_minimum_required(VERSION 3.8)

project(ComputerGraphics_D3D11)

### dependencies
add_subdirectory(third_party)
add_subdirectory(framework)

### setup katamari build
set(group_component_katamari
    src/components/katamari/katamari_component.cpp
    src/components/katamari/katamari_component.h
)
set(group_component_katamari_main
    src/katamari_main.cpp
)
set(katamari_sources
    ${group_component_katamari}
    ${group_component_katamari_main}
)
source_group("" FILES ${group_component_katamari_main})
source_group("component" FILES ${group_component_katamari})
add_executable(katamari WIN32 ${katamari_sources})
target_include_directories(katamari
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework)
target_link_libraries(katamari
    framework
)
set_property(TARGET katamari PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
add_custom_command(TARGET katamari POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:katamari> ${CMAKE_CURRENT_SOURCE_DIR})

### setup particle build
set(group_component_particle
    src/components/particle/particle_component.cpp
    src/components/particle/particle_component.h
)
set(group_component_particle_main
    src/particle_main.cpp
)
set(particle_sources
    ${group_component_particle}
    ${group_component_particle_main}
)
source_group("" FILES ${group_component_particle_main})
source_group("component" FILES ${group_component_particle})
add_executable(particle WIN32 ${particle_sources})
target_include_directories(particle
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/framework)
target_link_libraries(particle
    framework
)
set_property(TARGET particle PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
add_custom_command(TARGET particle POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:particle> ${CMAKE_CURRENT_SOURCE_DIR})
