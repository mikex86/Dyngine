# ATTENTION: VULKAN SDK REQUIRED
# Download here: https://vulkan.lunarg.com/

# DirectX Shader Compiler (dxc) required to be in PATH

cmake_minimum_required(VERSION 3.20)
project(Dyngine)

set(CMAKE_CXX_STANDARD 17)

# Enable exceptions
if (CMAKE_COMPILER_IS_GNUCC)
    add_compile_options("-fexceptions")
elseif (MSVC)
    add_compile_options("/EHsc")
endif (CMAKE_COMPILER_IS_GNUCC)

# Compile *.cpp in src/ folder
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(Dyngine ${SOURCE_FILES})

# Include private/ folder as internal headers
target_include_directories(Dyngine PRIVATE private/)

# Initialize submodules and link libraries

# ErrorHandling Module
add_subdirectory("${CMAKE_SOURCE_DIR}/ErrorHandling")
target_link_libraries(Dyngine PUBLIC Dyngine_ErrorHandling)

# Utils Module
add_subdirectory("${CMAKE_SOURCE_DIR}/Utils")
target_link_libraries(Dyngine PUBLIC Dyngine_Utils)

# Buffer Module
add_subdirectory("${CMAKE_SOURCE_DIR}/Buffer")

# Dpac Module
add_subdirectory("${CMAKE_SOURCE_DIR}/Dpac")
target_link_libraries(Dyngine PRIVATE Dyngine_Dpac)

# Dpac CLI tools
add_subdirectory("${CMAKE_SOURCE_DIR}/DpacTools")

# Reset the executable and library output path (changed by DpacTools submodule)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

# Low Level Graphics Library
set(LLGL_GRAPHICS_API Direct3D12)

if (${LLGL_GRAPHICS_API} MATCHES "Vulkan")
    target_compile_definitions(Dyngine PRIVATE DYNGINE_USE_VULKAN_API)
    set(LLGL_BUILD_RENDERER_VULKAN ON) # Vulkan SDK required
elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D12")
    set(LLGL_BUILD_RENDERER_DIRECT3D12 ON)
    target_compile_definitions(Dyngine PRIVATE DYNGINE_USE_DIRECT3D12_API)
endif ()

add_subdirectory("${CMAKE_SOURCE_DIR}/libraries/LLGL")

# Disable warnings and errors on CLANG
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (${LLGL_GRAPHICS_API} MATCHES "Vulkan")
        target_compile_options(LLGL_Direct3D12 PRIVATE -Wno-switch)
    elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D12")
        # Disable warning about taking the address of a temporary variable in Clang
        # The use cases in LLGL are perfectly safe
        target_compile_options(LLGL_Direct3D12 PRIVATE -Wno-address-of-temporary -Wno-class-conversion -Wno-switch)
    endif ()
endif ()

# Reset the executable and library output path (changed by LLGL submodule)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_compile_definitions(LLGL_ENABLE_UTILITY) # Enable the Utility functions

# Link to LLGL library
target_link_libraries(Dyngine PUBLIC LLGL LLGL_${LLGL_GRAPHICS_API})

# Build Engine Resources

# Compile Spir-V shaders
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_BINARY_DIR}/shaders/spirv")

file(GLOB_RECURSE GLSL_VERTEX_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.vert.glsl")
file(GLOB_RECURSE GLSL_FRAGMENT_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.frag.glsl")

find_package(Vulkan REQUIRED COMPONENTS glslc) # Vulkan SDK required
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
if (NOT glslc_executable)
    message("Spir-V Shader compiler (glslc) was not found. Please install the Vulkan Development SDK")
endif ()
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" -E make_directory "${SHADER_BINARY_DIR}"
        OUTPUT "${SHADER_BINARY_DIR}"
        COMMENT "Creating ${SHADER_BINARY_DIR}"
)

# Compile GLSL vertex shaders to SPIR-V
foreach (vert_shader_file IN LISTS GLSL_VERTEX_SHADER_SOURCE_FILES)
    get_filename_component(FILENAME ${vert_shader_file} NAME)
    add_custom_command(
            COMMAND
            "${glslc_executable}"
            -fshader-stage=vertex
            -o "${SHADER_BINARY_DIR}/${FILENAME}.spv"
            ${vert_shader_file}
            OUTPUT "${SHADER_BINARY_DIR}/${FILENAME}.spv"
            DEPENDS "${spirv_file}" "${SHADER_BINARY_DIR}"
            COMMENT "Compiling Spir-V Shader \"${FILENAME}\""
    )
    list(APPEND SPV_SHADERS "${SHADER_BINARY_DIR}/${FILENAME}.spv")
endforeach ()

# Compile GLSL fragment shaders to SPIR-V
foreach (frag_shader_file IN LISTS GLSL_FRAGMENT_SHADER_SOURCE_FILES)
    get_filename_component(FILENAME ${frag_shader_file} NAME)
    add_custom_command(
            COMMAND
            "${glslc_executable}"
            -fshader-stage=fragment
            -o "${SHADER_BINARY_DIR}/${FILENAME}.spv"
            ${frag_shader_file}
            OUTPUT "${SHADER_BINARY_DIR}/${FILENAME}.spv"
            DEPENDS "${spirv_file}" "${SHADER_BINARY_DIR}"
            COMMENT "Compiling Spir-V Shader \"${FILENAME}\""
    )
    list(APPEND SPV_SHADERS "${SHADER_BINARY_DIR}/${FILENAME}.spv")
endforeach ()

# Compile HLSL shaders
file(GLOB_RECURSE HLSL_VERTEX_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.vert.hlsl")
file(GLOB_RECURSE HLSL_FRAGEMENT_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.frag.hlsl")

# Compile HLSL vertex shaders to Compiled Shader Objects
foreach (vert_shader_file IN LISTS HLSL_VERTEX_SHADER_SOURCE_FILES)
    get_filename_component(FILENAME ${vert_shader_file} NAME)
    add_custom_command(
            COMMAND
            dxc "${vert_shader_file}" -E VS -T vs_4_0 -Fo "${SHADER_BINARY_DIR}/${FILENAME}.cso"
            OUTPUT "${SHADER_BINARY_DIR}/${FILENAME}.cso"
            DEPENDS "${hlsl_file}" "${SHADER_BINARY_DIR}"
            COMMENT "Compiling HLSL Shader \"${FILENAME}\""
    )
    list(APPEND HLSL_SHADERS "${SHADER_BINARY_DIR}/${FILENAME}.cso")
endforeach ()

# Compile HLSL fragment shaders to Compiled Shader Objects
foreach (frag_shader_file IN LISTS HLSL_FRAGEMENT_SHADER_SOURCE_FILES)
    get_filename_component(FILENAME ${frag_shader_file} NAME)
    add_custom_command(
            COMMAND
            dxc "${frag_shader_file}" -E PS -T ps_4_0 -Fo "${SHADER_BINARY_DIR}/${FILENAME}.cso"
            OUTPUT "${SHADER_BINARY_DIR}/${FILENAME}.cso"
            DEPENDS "${hlsl_file}" "${SHADER_BINARY_DIR}"
            COMMENT "Compiling HLSL Shader \"${FILENAME}\""
    )
    list(APPEND HLSL_SHADERS "${SHADER_BINARY_DIR}/${FILENAME}.cso")
endforeach ()

# Package Engine Resources
set(ENGINE_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/working_directory")
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" -E make_directory "${ENGINE_WORKING_DIRECTORY}"
        OUTPUT "${ENGINE_WORKING_DIRECTORY}"
        COMMENT "Creating ${ENGINE_WORKING_DIRECTORY}"
)
set(ENGINE_RESOURCE_PACKAGE ${ENGINE_WORKING_DIRECTORY}/EngineResources.dpac)

add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target DpacDeflate --config "${CMAKE_BUILD_TYPE}"
        COMMENT "Building DpacDeflate Tool"
        OUTPUT "${CMAKE_BINARY_DIR}/DpacTools/DpacDeflate"
)

add_custom_command(
        COMMAND
        "${CMAKE_BINARY_DIR}/DpacTools/DpacDeflate"
        "${SHADER_BINARY_DIR}"
        "${ENGINE_RESOURCE_PACKAGE}"
        DEPENDS "${SHADER_BINARY_DIR}" "${ENGINE_WORKING_DIRECTORY}" "${CMAKE_BINARY_DIR}/DpacTools/DpacDeflate" ${SPV_SHADERS} ${HLSL_SHADERS}
        COMMENT "Packaging engine resources"
        OUTPUT "${ENGINE_RESOURCE_PACKAGE}"
)
add_custom_target(Dyngine_PackageResources ALL DEPENDS "${ENGINE_RESOURCE_PACKAGE}")
add_dependencies(Dyngine_PackageResources DpacDeflate)

# Setup Working Directory
# Copy library files
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(LLGL_LIBRARY_POST_FIX "D")
else ()
    set(LLGL_LIBRARY_POST_FIX "")
endif ()
set(LLGL_LIBRARY_FILES "${CMAKE_BINARY_DIR}/libraries/LLGL/build/LLGL${LLGL_LIBRARY_POST_FIX}.dll;${CMAKE_BINARY_DIR}/libraries/LLGL/build/LLGL_${LLGL_GRAPHICS_API}${LLGL_LIBRARY_POST_FIX}.dll;")
add_custom_command(
        COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${LLGL_LIBRARY_FILES} ${ENGINE_WORKING_DIRECTORY}
        DEPENDS "${ENGINE_WORKING_DIRECTORY}" LLGL LLGL_${LLGL_GRAPHICS_API}
        COMMENT "Copying LLGL libraries to working directory"
        OUTPUT "LLGL_LIB_COPY"
)

add_custom_target(Dyngine_SetupWorkingDirectory ALL DEPENDS Dyngine_PackageResources LLGL_LIB_COPY)

add_dependencies(Dyngine Dyngine_SetupWorkingDirectory)
