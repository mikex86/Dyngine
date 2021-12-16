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

# Google Test
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libraries/googletest)

# zstd compression
set(ZSTD_BUILD_STATIC ON)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/libraries/zstd/build/cmake")

# ErrorHandling Module
add_subdirectory("${CMAKE_SOURCE_DIR}/ErrorHandling")
target_link_libraries(Dyngine PRIVATE Dyngine_ErrorHandling)

# Utils Module
add_subdirectory("${CMAKE_SOURCE_DIR}/Utils")
target_link_libraries(Dyngine PRIVATE Dyngine_Utils)

# InputOutput Module
add_subdirectory("${CMAKE_SOURCE_DIR}/InputOutput")

# Dpac Module
add_subdirectory("${CMAKE_SOURCE_DIR}/Dpac")
target_link_libraries(Dyngine PRIVATE Dyngine_Dpac)

# Dpac CLI tools
add_subdirectory("${CMAKE_SOURCE_DIR}/DpacTools")

# AssetLoader Module
add_subdirectory("${CMAKE_SOURCE_DIR}/AssetLoader")
target_link_libraries(Dyngine PRIVATE Dyngine_AssetLoader)

# Reset the executable and library output path (changed by DpacTools submodule)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

# OpenGL Math Library
add_subdirectory("${CMAKE_SOURCE_DIR}/libraries/glm")
target_link_libraries(Dyngine PUBLIC glm)

# glTF loader
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/libraries/tinygltf")

# Low Level Graphics Library
set(LLGL_GRAPHICS_API OpenGL)

if (${LLGL_GRAPHICS_API} MATCHES "OpenGL")
    target_compile_definitions(Dyngine PRIVATE DYNGINE_USE_OPENGL_API)
    set(LLGL_BUILD_RENDERER_OPENGL ON)
elseif (${LLGL_GRAPHICS_API} MATCHES "Vulkan")
    target_compile_definitions(Dyngine PRIVATE DYNGINE_USE_VULKAN_API)
    set(LLGL_BUILD_RENDERER_VULKAN ON) # Vulkan SDK required
elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D12")
    set(LLGL_BUILD_RENDERER_DIRECT3D12 ON)
    target_compile_definitions(Dyngine PRIVATE DYNGINE_USE_DIRECT3D12_API)
elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D11")
    set(LLGL_BUILD_RENDERER_DIRECT3D11 ON)
    target_compile_definitions(Dyngine PRIVATE DYNGINE_USE_DIRECT3D11_API)
endif ()

add_subdirectory("${CMAKE_SOURCE_DIR}/libraries/LLGL")

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

# Reset the executable and library output path (changed by LLGL submodule)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_compile_definitions(LLGL_ENABLE_UTILITY) # Enable the Utility functions

# Link to LLGL library
target_link_libraries(Dyngine PUBLIC LLGL LLGL_${LLGL_GRAPHICS_API})

# Build Engine Resources

set(RESOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/resources")
set(RESOURCES_BINARY_DIRECTORY "${CMAKE_BINARY_DIR}/resources")

# Create RESOURCES_BINARY_DIRECTORY
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" -E make_directory "${RESOURCES_BINARY_DIRECTORY}"
        OUTPUT "${RESOURCES_BINARY_DIRECTORY}"
        COMMENT "Creating ${RESOURCES_BINARY_DIRECTORY}"
)

# Compile shaders
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${RESOURCES_BINARY_DIRECTORY}/shaders")

# Create SHADER_BINARY_DIR
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" -E make_directory "${SHADER_BINARY_DIR}"
        OUTPUT "${SHADER_BINARY_DIR}"
        COMMENT "Creating ${SHADER_BINARY_DIR}"
)

# Copy GLSL shaders for platforms not supporting precompiled shaders
# Copy files SHADER_SOURCE_DIR/*.glsl to SHADER_BINARY_DIR
# Copy resources from RESOURCE_DIRECTORY into RESOURCES_BINARY_DIRECTORY
file(GLOB_RECURSE GLSL_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.glsl")
foreach (GLSL_FILE IN LISTS GLSL_FILES)
    file(RELATIVE_PATH GLSL_FILE_RELATIVE ${SHADER_SOURCE_DIR} ${GLSL_FILE})
    get_filename_component(FILENAME ${GLSL_FILE} NAME)
    add_custom_command(
            COMMAND
            "${CMAKE_COMMAND}" -E copy_if_different "${GLSL_FILE}" "${SHADER_BINARY_DIR}/${GLSL_FILE_RELATIVE}"
            OUTPUT "${SHADER_BINARY_DIR}/${GLSL_FILE_RELATIVE}"
            DEPENDS "${GLSL_FILE}" "${SHADER_BINARY_DIR}"
            COMMENT "Copying Resource \"${GLSL_FILE_RELATIVE}\""
    )
    list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${GLSL_FILE_RELATIVE}")
endforeach ()


# Compile Spir-V shaders

file(GLOB_RECURSE GLSL_VERTEX_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.vert.glsl")
file(GLOB_RECURSE GLSL_FRAGMENT_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.frag.glsl")

# Find GLSLC shader compiler
find_package(Vulkan REQUIRED COMPONENTS glslc) # Vulkan SDK required
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
if (NOT glslc_executable)
    message("Spir-V Shader compiler (glslc) was not found. Please install the Vulkan Development SDK")
endif ()

# Compile GLSL vertex shaders to SPIR-V
foreach (VERT_SHADER_FILE IN LISTS GLSL_VERTEX_SHADER_SOURCE_FILES)
    file(RELATIVE_PATH VERT_SHADER_FILE_RELATIVE ${SHADER_SOURCE_DIR} ${VERT_SHADER_FILE})
    get_filename_component(FILENAME ${VERT_SHADER_FILE} NAME)
    add_custom_command(
            COMMAND
            "${glslc_executable}"
            -fshader-stage=vertex
            -o "${SHADER_BINARY_DIR}/${VERT_SHADER_FILE_RELATIVE}.spv"
            ${VERT_SHADER_FILE}
            OUTPUT "${SHADER_BINARY_DIR}/${VERT_SHADER_FILE_RELATIVE}.spv"
            DEPENDS "${VERT_SHADER_FILE}" "${SHADER_BINARY_DIR}"
            COMMENT "Compiling Spir-V Shader \"${VERT_SHADER_FILE_RELATIVE}\""
    )
    list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${VERT_SHADER_FILE_RELATIVE}.spv")
endforeach ()

# Compile GLSL fragment shaders to SPIR-V
foreach (FRAG_SHADER_FILE IN LISTS GLSL_FRAGMENT_SHADER_SOURCE_FILES)
    file(RELATIVE_PATH FRAG_SHADER_FILE_RELATIVE ${SHADER_SOURCE_DIR} ${FRAG_SHADER_FILE})
    get_filename_component(FILENAME ${FRAG_SHADER_FILE} NAME)
    add_custom_command(
            COMMAND
            "${glslc_executable}"
            -fshader-stage=fragment
            -o "${SHADER_BINARY_DIR}/${FRAG_SHADER_FILE_RELATIVE}.spv"
            ${FRAG_SHADER_FILE}
            OUTPUT "${SHADER_BINARY_DIR}/${FRAG_SHADER_FILE_RELATIVE}.spv"
            DEPENDS "${FRAG_SHADER_FILE}" "${SHADER_BINARY_DIR}"
            COMMENT "Compiling Spir-V Shader \"${FRAG_SHADER_FILE_RELATIVE}\""
    )
    list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${FRAG_SHADER_FILE_RELATIVE}.spv")
endforeach ()

# Compile HLSL shaders
file(GLOB_RECURSE HLSL_VERTEX_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.vert.hlsl")
file(GLOB_RECURSE HLSL_FRAGEMENT_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.frag.hlsl")

# Compile HLSL vertex shaders to Compiled Shader Objects
foreach (VERT_SHADER_FILE IN LISTS HLSL_VERTEX_SHADER_SOURCE_FILES)
    file(RELATIVE_PATH VERT_SHADER_FILE_RELATIVE ${SHADER_SOURCE_DIR} ${VERT_SHADER_FILE})
    get_filename_component(FILENAME ${VERT_SHADER_FILE} NAME)
    add_custom_command(
            COMMAND
            dxc "${VERT_SHADER_FILE}" -E VS -T vs_4_0 -Fo "${SHADER_BINARY_DIR}/${VERT_SHADER_FILE_RELATIVE}.cso"
            OUTPUT "${SHADER_BINARY_DIR}/${VERT_SHADER_FILE_RELATIVE}.cso"
            DEPENDS "${VERT_SHADER_FILE}" "${SHADER_BINARY_DIR}"
            COMMENT "Compiling HLSL Shader \"${VERT_SHADER_FILE_RELATIVE}\""
    )
    list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${VERT_SHADER_FILE_RELATIVE}.cso")
endforeach ()

# Compile HLSL fragment shaders to Compiled Shader Objects
foreach (FRAG_SHADER_FILE IN LISTS HLSL_FRAGEMENT_SHADER_SOURCE_FILES)
    file(RELATIVE_PATH FRAG_SHADER_FILE_RELATIVE ${SHADER_SOURCE_DIR} ${FRAG_SHADER_FILE})
    get_filename_component(FILENAME ${FRAG_SHADER_FILE} NAME)
    add_custom_command(
            COMMAND
            dxc "${FRAG_SHADER_FILE}" -E PS -T ps_4_0 -Fo "${SHADER_BINARY_DIR}/${FRAG_SHADER_FILE_RELATIVE}.cso"
            OUTPUT "${SHADER_BINARY_DIR}/${FRAG_SHADER_FILE_RELATIVE}.cso"
            DEPENDS "${FRAG_SHADER_FILE}" "${SHADER_BINARY_DIR}"
            COMMENT "Compiling HLSL Shader \"${FRAG_SHADER_FILE_RELATIVE}\""
    )
    list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${FRAG_SHADER_FILE_RELATIVE}.cso")
endforeach ()

# Copy resources from RESOURCE_DIRECTORY into RESOURCES_BINARY_DIRECTORY
file(GLOB_RECURSE RESOURCE_FILES CONFIGURE_DEPENDS "${RESOURCE_DIRECTORY}/*")
foreach (RESOURCE_FILE IN LISTS RESOURCE_FILES)
    file(RELATIVE_PATH RESOURCE_FILE_RELATIVE ${RESOURCE_DIRECTORY} ${RESOURCE_FILE})
    get_filename_component(FILENAME ${RESOURCE_FILE} NAME)
    add_custom_command(
            COMMAND
            "${CMAKE_COMMAND}" -E copy_if_different "${RESOURCE_FILE}" "${RESOURCES_BINARY_DIRECTORY}/${RESOURCE_FILE_RELATIVE}"
            OUTPUT "${RESOURCES_BINARY_DIRECTORY}/${RESOURCE_FILE_RELATIVE}"
            DEPENDS "${RESOURCE_FILE}" "${RESOURCES_BINARY_DIRECTORY}"
            COMMENT "Copying Resource \"${RESOURCE_FILE_RELATIVE}\""
    )
    list(APPEND COPIED_RESOURCES "${RESOURCES_BINARY_DIRECTORY}/${RESOURCE_FILE_RELATIVE}")
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
        "${RESOURCES_BINARY_DIRECTORY}"
        "${ENGINE_RESOURCE_PACKAGE}"
        DEPENDS "${SHADER_BINARY_DIR}" "${ENGINE_WORKING_DIRECTORY}" "${CMAKE_BINARY_DIR}/DpacTools/DpacDeflate" ${COPIED_RESOURCES} ${COMPILED_SHADERS}
        COMMENT "Packaging engine resources"
        OUTPUT "${ENGINE_RESOURCE_PACKAGE}"
)
add_custom_target(Dyngine_PackageResources ALL DEPENDS ${ENGINE_RESOURCE_PACKAGE})
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
