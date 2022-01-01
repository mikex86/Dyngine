cmake_minimum_required(VERSION 3.20)
project(Dyngine)

set(CMAKE_CXX_STANDARD 20)

# Enable exceptions
if (CMAKE_COMPILER_IS_GNUCC)
    add_compile_options("-fexceptions")
elseif (MSVC)
    add_compile_options("/EHsc")
endif (CMAKE_COMPILER_IS_GNUCC)

# Reset the executable and library output path (changed by LLGL submodule)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

# Initialize submodules and link libraries

# Google Test
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(libraries/googletest)

# zstd compression
set(ZSTD_BUILD_STATIC ON)
add_subdirectory("libraries/zstd/build/cmake")

# stb image
add_library(STB_LIBRARY INTERFACE)
target_include_directories(STB_LIBRARY INTERFACE "libraries/stb")
target_compile_definitions(STB_LIBRARY INTERFACE "STB_IMAGE_IMPLEMENTATION" "STB_IMAGE_WRITE_IMPLEMENTATION")

# ErrorHandling Module
add_subdirectory("ErrorHandling")

# Utils Module
add_subdirectory("Utils")

# Stream Module
add_subdirectory("Stream")

# Dpac Module
add_subdirectory("FileFormats/Dpac")

# Dpac CLI tools
add_subdirectory("FileFormats/DpacTools")

# DShader Module
add_subdirectory("FileFormats/DShader")

# DShader CLI tools
add_subdirectory("FileFormats/DShaderTools")

# DAsset Module
add_subdirectory("FileFormats/DAsset")

# DAsset CLI tools
add_subdirectory("FileFormats/DAssetTools")

# Reset the executable and library output path (changed by DpacTools submodule)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

# OpenGL Math Library
add_subdirectory("libraries/glm")

# glTF loader
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory("libraries/tinygltf")

# Low Level Graphics Library
set(LLGL_GRAPHICS_API OpenGL)

if (${LLGL_GRAPHICS_API} MATCHES "OpenGL")
    set(LLGL_BUILD_RENDERER_OPENGL ON)
elseif (${LLGL_GRAPHICS_API} MATCHES "Vulkan")
    set(LLGL_BUILD_RENDERER_VULKAN ON) # Vulkan SDK required
elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D12")
    set(LLGL_BUILD_RENDERER_DIRECT3D12 ON)
elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D11")
    set(LLGL_BUILD_RENDERER_DIRECT3D11 ON)
endif ()

# LLGL library
add_subdirectory("libraries/LLGL")

# Disable warnings and errors on CLANG
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (${LLGL_GRAPHICS_API} MATCHES "Vulkan")
        target_compile_options(LLGL_Vulkan PRIVATE -Wno-switch)
    elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D11")
        target_compile_options(LLGL_Direct3D11 PRIVATE -Wno-address-of-temporary -Wno-class-conversion -Wno-switch)
    elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D12")
        # Disable warning about taking the address of a temporary variable in Clang
        # The use cases in LLGL are perfectly safe
        target_compile_options(LLGL_Direct3D12 PRIVATE -Wno-address-of-temporary -Wno-class-conversion -Wno-switch)
    endif ()
endif ()

add_compile_definitions(LLGL_ENABLE_UTILITY) # Enable the Utility functions

# Engine Module
add_subdirectory("Engine")

#Editor Module
add_subdirectory("Editor")