cmake_minimum_required(VERSION 3.8)

project(D3D11_Example)

set(sources 
    src/main.cpp
    
    src/core/game.cpp
    src/core/game.h

    src/render/render.cpp
    src/render/render.h
    src/render/shader.cpp
    src/render/shader.h

    src/components/game_component.h

    src/components/triangle_component.cpp
    src/components/triangle_component.h

    src/win32/win.cpp
    src/win32/win.h
)

add_executable(${CMAKE_PROJECT_NAME} WIN32 ${sources})

target_compile_definitions(${CMAKE_PROJECT_NAME}
    PRIVATE -DUNICODE)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# it is important to find 'shaders' folder
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR})
