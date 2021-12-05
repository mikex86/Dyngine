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

option(LLGL_BUILD_RENDERER_VULKAN "" ON) # Vulkan SDK required
add_subdirectory("${CMAKE_SOURCE_DIR}/libraries/LLGL")

# Reset the executable and library output path (changed by LLGL submodule)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_compile_definitions(LLGL_ENABLE_UTILITY) # Enable the Utility functions

# Link to LLGL library
target_link_libraries(Dyngine PUBLIC LLGL LLGL_Vulkan)

# Build Engine Resources

# Compile Spir-V shaders
set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_BINARY_DIR}/shaders/spirv")

file(GLOB_RECURSE SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.vert" "${SHADER_SOURCE_DIR}/*.frag")

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
        DEPENDS "${SHADER_BINARY_DIR}" "${ENGINE_WORKING_DIRECTORY}" "${CMAKE_BINARY_DIR}/DpacTools/DpacDeflate" ${SPV_SHADERS}
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
set(LLGL_LIBRARY_FILES "${CMAKE_BINARY_DIR}/libraries/LLGL/build/LLGL${LLGL_LIBRARY_POST_FIX}.dll;${CMAKE_BINARY_DIR}/libraries/LLGL/build/LLGL_Vulkan${LLGL_LIBRARY_POST_FIX}.dll;")
add_custom_command(
        COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${LLGL_LIBRARY_FILES} ${ENGINE_WORKING_DIRECTORY}
        DEPENDS "${ENGINE_WORKING_DIRECTORY}" LLGL LLGL_Vulkan
        COMMENT "Copying LLGL libraries to working directory"
        OUTPUT "LLGL_LIB_COPY"
)

add_custom_target(Dyngine_SetupWorkingDirectory ALL DEPENDS Dyngine_PackageResources LLGL_LIB_COPY)

add_dependencies(Dyngine Dyngine_SetupWorkingDirectory)
