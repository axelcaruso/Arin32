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

# Position Independent Code for both static and shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Source files for Arin32
set(ARIN_SOURCES
    src/types.cpp
    src/theme.cpp
    src/font.cpp
    src/texture.cpp
    src/image.cpp
    src/icon.cpp
    src/svg.cpp
    src/renderer/shader_util.cpp
    src/renderer/rect_pipeline.cpp
    src/renderer/text_pipeline.cpp
    src/renderer/progress_pipeline.cpp
    src/renderer/image_pipeline.cpp
    src/renderer/renderer.cpp
    src/button.cpp
    src/checkbox.cpp
    src/text_input.cpp
    src/progress_bar.cpp
    src/list_box.cpp
    src/layout.cpp
    src/context_menu.cpp
    src/window.cpp
    src/app.cpp
)

# Core Arin32 Static Library Target (.a)
add_library(arin32 STATIC ${ARIN_SOURCES})
set_target_properties(arin32 PROPERTIES OUTPUT_NAME "arin32")

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
    -O3
)

# Core Arin32 Shared Library Target (.so)
add_library(arin32_shared SHARED ${ARIN_SOURCES})
set_target_properties(arin32_shared PROPERTIES OUTPUT_NAME "arin32")

target_include_directories(arin32_shared
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(arin32_shared
    PUBLIC
        OpenGL::GL
        GLEW::GLEW
        glfw
        Threads::Threads
)

target_compile_options(arin32_shared PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O3
)

# Alias for consistent CMake target usage
add_library(Arin32::arin32 ALIAS arin32)
add_library(Arin32::arin32_shared ALIAS arin32_shared)

# Automatically copy built libraries and assets to dist/
add_custom_command(TARGET arin32 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:arin32> ${CMAKE_CURRENT_SOURCE_DIR}/dist/libarin32.a
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/dist/include
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_SOURCE_DIR}/dist/assets
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/scripts/package.py ${CMAKE_CURRENT_SOURCE_DIR}/dist/package.py
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/dist/LICENSE
    COMMENT "Synchronizing static library, headers, and package script to dist/"
)

add_custom_command(TARGET arin32_shared POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:arin32_shared> ${CMAKE_CURRENT_SOURCE_DIR}/dist/libarin32.so
    COMMENT "Synchronizing shared library (.so) to dist/"
)

# Custom target to package the distribution into .7z and produce SHA256 checksums
add_custom_target(dist
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/dist/package.py
    DEPENDS arin32 arin32_shared
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist
    COMMENT "Executing maximum 7z compression packaging and generating SHA256 manifest"
)

# Installation Rules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS arin32 arin32_shared
    EXPORT Arin32Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY assets/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/arin32/assets
)

install(EXPORT Arin32Targets
    FILE Arin32Targets.cmake
    NAMESPACE Arin32::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Arin32
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Arin32Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Arin32Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Arin32
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/Arin32ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Arin32Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Arin32ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Arin32
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
