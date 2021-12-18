project(Dyngine_Engine)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_executable(Dyngine_Engine ${SOURCE_FILES})
target_include_directories(Dyngine_Engine PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private")
target_include_directories(Dyngine_Engine PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")

# Link libraries

# LLGL

if (${LLGL_GRAPHICS_API} MATCHES "OpenGL")
    target_compile_definitions(Dyngine_Engine PRIVATE DYNGINE_USE_OPENGL_API)
elseif (${LLGL_GRAPHICS_API} MATCHES "Vulkan")
    target_compile_definitions(Dyngine_Engine PRIVATE DYNGINE_USE_VULKAN_API)
elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D12")
    target_compile_definitions(Dyngine_Engine PRIVATE DYNGINE_USE_DIRECT3D12_API)
elseif (${LLGL_GRAPHICS_API} MATCHES "Direct3D11")
    target_compile_definitions(Dyngine_Engine PRIVATE DYNGINE_USE_DIRECT3D11_API)
endif ()
target_link_libraries(Dyngine_Engine PRIVATE LLGL LLGL_${LLGL_GRAPHICS_API})

target_link_libraries(Dyngine_Engine PRIVATE Dyngine_ErrorHandling)
target_link_libraries(Dyngine_Engine PRIVATE Dyngine_Utils)
target_link_libraries(Dyngine_Engine PRIVATE Dyngine_Dpac)
target_link_libraries(Dyngine_Engine PRIVATE Dyngine_AssetLoader)
target_link_libraries(Dyngine_Engine PUBLIC glm)

# Build Engine Resources

set(RESOURCE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/resources")
set(RESOURCES_BINARY_DIRECTORY "${CMAKE_BINARY_DIR}/resources")

# Create RESOURCES_BINARY_DIRECTORY
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" -E make_directory "${RESOURCES_BINARY_DIRECTORY}"
        OUTPUT "${RESOURCES_BINARY_DIRECTORY}"
        COMMENT "Creating ${RESOURCES_BINARY_DIRECTORY}"
)

# Compile shaders
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/shaders")
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
set(ENGINE_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/working_directory")
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

add_dependencies(Dyngine_Engine Dyngine_SetupWorkingDirectory)