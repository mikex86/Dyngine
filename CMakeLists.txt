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

add_subdirectory("${CMAKE_SOURCE_DIR}/RenderLib")

# Reset the executable and library output path (changed by DpacTools submodule)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

# GLFW
add_subdirectory("${CMAKE_SOURCE_DIR}/libraries/glfw")

# Link to RenderLib
target_link_libraries(Dyngine PRIVATE Dyngine_RenderLib)

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
add_dependencies(Dyngine Dyngine_PackageResources)
