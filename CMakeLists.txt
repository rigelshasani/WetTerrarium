cmake_minimum_required(VERSION 3.24)
project(WetTerrarium LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

find_package(SFML COMPONENTS System Window Graphics Audio CONFIG REQUIRED)

# --- engine_render (for Camera) ---
add_library(engine_render
  engine/render/Camera.cpp
  engine/render/Camera.hpp
)
target_link_libraries(engine_render PUBLIC SFML::Graphics SFML::Window SFML::System)
target_include_directories(engine_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(wet_terrarium src/main.cpp)
target_link_libraries(wet_terrarium PRIVATE engine_render SFML::Audio)

# rpath for vcpkg dylibs on macOS
set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)
if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/arm64-osx/lib")
  set(CMAKE_INSTALL_RPATH "${CMAKE_BINARY_DIR}/vcpkg_installed/arm64-osx/lib")
elseif(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/installed/arm64-osx/lib")
  set(CMAKE_INSTALL_RPATH "$ENV{VCPKG_ROOT}/installed/arm64-osx/lib")
endif()

message(STATUS "SFML_DIR = ${SFML_DIR}")
