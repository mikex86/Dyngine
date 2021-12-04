# ATTENTION: VULKAN SDK REQUIRED
# Download here: https://vulkan.lunarg.com/

cmake_minimum_required(VERSION 3.20)
project(Dyngine)

set(CMAKE_CXX_STANDARD 17)

# Enable exceptions
if (CMAKE_COMPILER_IS_GNUCC)
    add_definitions("-fexceptions")
elseif (MSVC)
    add_definitions("/EHsc")
endif (CMAKE_COMPILER_IS_GNUCC)

# Compile *.cpp in src/ folder
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
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

# Low Level Graphics Library

option(LLGL_BUILD_RENDERER_VULKAN "" ON) # Vulkan SDK required
add_subdirectory("${CMAKE_SOURCE_DIR}/libraries/LLGL")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}") # Reset the executable output path which gets fucked by LLGL
add_compile_definitions(LLGL_ENABLE_UTILITY) # Enable the Utility functions

# Link to LLGL library
target_link_libraries(Dyngine PUBLIC LLGL LLGL_Vulkan)

# Build Engine Resources

# Compile Spir-V shaders
set(SHADER_BINARY_DIR "${CMAKE_BINARY_DIR}/shaders/spirv")
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shaders")

file(GLOB_RECURSE SHADER_SOURCE_FILES "${SHADER_SOURCE_DIR}/*.vert" "${SHADER_SOURCE_DIR}/*.frag")

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
foreach (shader_file IN LISTS SHADER_SOURCE_FILES) # Compile all shader files
    get_filename_component(FILENAME ${shader_file} NAME)
    add_custom_command(
            COMMAND
            "${glslc_executable}"
            -o "${SHADER_BINARY_DIR}/${FILENAME}.spv"
            ${shader_file}
            OUTPUT "${SHADER_BINARY_DIR}/${FILENAME}.spv"
            DEPENDS "${shader_file}" "${SHADER_BINARY_DIR}"
            COMMENT "Compiling Spir-V Shader \"${FILENAME}\""
    )
    list(APPEND SPV_SHADERS "${SHADER_BINARY_DIR}/${FILENAME}.spv")
endforeach ()
add_custom_target(Dyngine_Spvshaders ALL DEPENDS ${SPV_SHADERS})

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
        "${CMAKE_BINARY_DIR}/DpacTools/DpacDeflate"
        "${SHADER_BINARY_DIR}"
        "${ENGINE_RESOURCE_PACKAGE}"
        OUTPUT "${ENGINE_RESOURCE_PACKAGE}"
        DEPENDS "${ENGINE_WORKING_DIRECTORY}" "Dyngine_Spvshaders"
        COMMENT "Packaging engine resources"
)
add_custom_target(Dyngine_PackageResources ALL DEPENDS "${ENGINE_RESOURCE_PACKAGE}")
add_dependencies(Dyngine_PackageResources DpacDeflate)

# Setup Working Directory
file(GLOB LIBRARY_FILES "${CMAKE_BINARY_DIR}/libraries/LLGL/*.dll" "${CMAKE_BINARY_DIR}/libraries/LLGL/*.so" , "${CMAKE_BINARY_DIR}/libraries/LLGL/*.dylib")
add_custom_command(
        COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${LIBRARY_FILES}
        ${ENGINE_WORKING_DIRECTORY}
        DEPENDS "${ENGINE_WORKING_DIRECTORY}"
        OUTPUT LLGL_LIB_COPY
        COMMENT "Copying libraries to working directory"
)
add_custom_target(Dyngine_CopyLibraries ALL DEPENDS "LLGL_LIB_COPY")

add_custom_target(Dyngine_SetupWorkingDirectory ALL DEPENDS Dyngine_CopyLibraries Dyngine_PackageResources)

add_dependencies(Dyngine Dyngine_SetupWorkingDirectory)
