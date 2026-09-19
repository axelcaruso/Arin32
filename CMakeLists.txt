cmake_minimum_required(VERSION 3.16)
project(Arin32 VERSION 0.1.0 LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(ARIN_BUILD_EXAMPLES "Build Arin32 examples and demos" ON)
option(ARIN_BUILD_TESTS "Build Arin32 test suite" ON)

# Find Required Dependencies
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(Threads REQUIRED)

# Core Arin32 Library Target
add_library(arin32
    src/types.cpp
    src/theme.cpp
    src/font.cpp
    src/renderer/shader_util.cpp
    src/renderer/rect_pipeline.cpp
    src/renderer/text_pipeline.cpp
    src/renderer/progress_pipeline.cpp
    src/renderer/renderer.cpp
    src/button.cpp
    src/progress_bar.cpp
    src/window.cpp
    src/app.cpp
)

target_include_directories(arin32
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(arin32
    PUBLIC
        OpenGL::GL
        GLEW::GLEW
        glfw
        Threads::Threads
)

target_compile_options(arin32 PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Optional Examples
if(ARIN_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Optional Tests
if(ARIN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
