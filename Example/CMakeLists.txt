cmake_minimum_required(VERSION 3.8)

project(D3D11_Example)

set(sources 
    src/MySuper3DApp.cpp
)

add_executable(${CMAKE_PROJECT_NAME} WIN32 ${sources})

# it is important to find 'shaders' folder
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR})
