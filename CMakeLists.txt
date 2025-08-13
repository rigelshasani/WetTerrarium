cmake_minimum_required(VERSION 3.24)
project(WetTerrarium LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Prefer config packages (vcpkg) over Find*.cmake
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# --- SFML from vcpkg (CONFIG mode) ---
# Do NOT pin version here; let vcpkg provide it.
find_package(SFML COMPONENTS System Window Graphics Audio CONFIG REQUIRED)

add_executable(wet_terrarium src/main.cpp)
target_link_libraries(wet_terrarium PRIVATE
  SFML::System SFML::Window SFML::Graphics SFML::Audio)

# macOS runtime search path so the app can find vcpkg dylibs
set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)
# Use the vcpkg “installed” under the current build dir when in manifest mode
# Fallback to global VCPKG_ROOT if that path doesn't exist.
if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/arm64-osx/lib")
  set(CMAKE_INSTALL_RPATH "${CMAKE_BINARY_DIR}/vcpkg_installed/arm64-osx/lib")
elseif(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/installed/arm64-osx/lib")
  set(CMAKE_INSTALL_RPATH "$ENV{VCPKG_ROOT}/installed/arm64-osx/lib")
endif()

message(STATUS "SFML_DIR = ${SFML_DIR}")
