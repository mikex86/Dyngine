project(Dyngine_Engine)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_Engine STATIC ${SOURCE_FILES})
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
target_link_libraries(Dyngine_Engine PRIVATE Dyngine_DShader)
target_link_libraries(Dyngine_Engine PRIVATE Dyngine_DAsset)
target_link_libraries(Dyngine_Engine PRIVATE glm)
target_link_libraries(Dyngine_Engine PRIVATE STB_LIBRARY)

# Build Engine Resources

set(RESOURCE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/resources")
set(RESOURCES_BINARY_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/resources")

# Create RESOURCES_BINARY_DIRECTORY
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" -E make_directory "${RESOURCES_BINARY_DIRECTORY}"
        OUTPUT "${RESOURCES_BINARY_DIRECTORY}"
        COMMENT "Creating ${RESOURCES_BINARY_DIRECTORY}"
)

# Compile shaders
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/compiled_shaders")

# Create SHADER_BINARY_DIR
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" -E make_directory "${SHADER_BINARY_DIR}"
        OUTPUT "${SHADER_BINARY_DIR}"
        COMMENT "Creating ${SHADER_BINARY_DIR}"
)

set(SHADER_PACKAGE_DST_DIR "${RESOURCES_BINARY_DIRECTORY}/shaders")

# Create SHADER_PACKAGE_DST_DIR
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" -E make_directory "${SHADER_PACKAGE_DST_DIR}"
        OUTPUT "${SHADER_PACKAGE_DST_DIR}"
        COMMENT "Creating ${SHADER_PACKAGE_DST_DIR}"
)

# Find GLSLC shader compiler
find_package(Vulkan REQUIRED COMPONENTS glslc) # Vulkan SDK required
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
if (NOT glslc_executable)
    message("Spir-V Shader compiler (glslc) was not found. Please install the Vulkan Development SDK")
endif ()

# Package shader variants into the dshader format
# Compile DShaderPackage executable
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target DShaderPackage --config "${CMAKE_BUILD_TYPE}"
        COMMENT "Building DShaderPackage Tool"
        OUTPUT "${CMAKE_BINARY_DIR}/DShaderPackage"
)
# Run DShaderPackage to package shader variants
file(GLOB SHADER_DIRECTORIES LIST_DIRECTORIES true "${SHADER_SOURCE_DIR}/*")
foreach (SHADER_DIRECTORY IN LISTS SHADER_DIRECTORIES)
    if (NOT IS_DIRECTORY ${SHADER_DIRECTORY})
        continue()
    endif ()

    file(GLOB SHADER_VARIANTS "${SHADER}/*")
    if (NOT SHADER_VARIANTS) # Only directories with files in them are shader directories.
        continue()
    endif ()
    file(RELATIVE_PATH SHADER_RELATIVE_PATH ${SHADER_SOURCE_DIR} ${SHADER_DIRECTORY})

    # Copy GLSL shaders for platforms not supporting precompiled shaders
    # Copy files SHADER_DIRECTORY/*.glsl to SHADER_BINARY_DIR
    file(GLOB_RECURSE GLSL_FILES CONFIGURE_DEPENDS "${SHADER_DIRECTORY}/*.glsl")
    foreach (GLSL_FILE IN LISTS GLSL_FILES)
        file(RELATIVE_PATH GLSL_FILE_RELATIVE ${SHADER_DIRECTORY} ${GLSL_FILE})
        get_filename_component(FILENAME ${GLSL_FILE} NAME)
        add_custom_command(
                COMMAND
                "${CMAKE_COMMAND}" -E copy_if_different "${GLSL_FILE}" "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${GLSL_FILE_RELATIVE}"
                OUTPUT "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${GLSL_FILE_RELATIVE}"
                DEPENDS "${GLSL_FILE}" "${SHADER_BINARY_DIR}"
                COMMENT "Copying GLSL Shader \"${SHADER_RELATIVE_PATH}/${GLSL_FILE_RELATIVE}\""
        )
        list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${GLSL_FILE_RELATIVE}")
    endforeach ()

    # Compile Spir-V shaders
    file(GLOB_RECURSE GLSL_VERTEX_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_DIRECTORY}/*.vert.glsl")
    file(GLOB_RECURSE GLSL_FRAGMENT_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_DIRECTORY}/*.frag.glsl")

    # Compile GLSL vertex shaders to SPIR-V
    foreach (VERT_SHADER_FILE IN LISTS GLSL_VERTEX_SHADER_SOURCE_FILES)
        file(RELATIVE_PATH VERT_SHADER_FILE_RELATIVE ${SHADER_DIRECTORY} ${VERT_SHADER_FILE})
        get_filename_component(FILENAME ${VERT_SHADER_FILE} NAME)
        add_custom_command(
                COMMAND
                "${glslc_executable}"
                -fshader-stage=vertex
                -o "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${VERT_SHADER_FILE_RELATIVE}.spv"
                ${VERT_SHADER_FILE}
                OUTPUT "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${VERT_SHADER_FILE_RELATIVE}.spv"
                DEPENDS "${VERT_SHADER_FILE}" "${SHADER_BINARY_DIR}"
                COMMENT "Compiling Spir-V Vertex Shader \"${SHADER_RELATIVE_PATH}/${VERT_SHADER_FILE_RELATIVE}.spv\""
        )
        list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${VERT_SHADER_FILE_RELATIVE}.spv")
    endforeach ()

    # Compile GLSL fragment shaders to SPIR-V
    foreach (FRAG_SHADER_FILE IN LISTS GLSL_FRAGMENT_SHADER_SOURCE_FILES)
        file(RELATIVE_PATH FRAG_SHADER_FILE_RELATIVE ${SHADER_DIRECTORY} ${FRAG_SHADER_FILE})
        get_filename_component(FILENAME ${FRAG_SHADER_FILE} NAME)
        add_custom_command(
                COMMAND
                "${glslc_executable}"
                -fshader-stage=fragment
                -o "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${FRAG_SHADER_FILE_RELATIVE}.spv"
                ${FRAG_SHADER_FILE}
                OUTPUT "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${FRAG_SHADER_FILE_RELATIVE}.spv"
                DEPENDS "${FRAG_SHADER_FILE}" "${SHADER_BINARY_DIR}"
                COMMENT "Compiling Spir-V Fragment Shader \"${SHADER_RELATIVE_PATH}/${FRAG_SHADER_FILE_RELATIVE}.spv\""
        )
        list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${FRAG_SHADER_FILE_RELATIVE}.spv")
    endforeach ()

    # Compile HLSL shaders
    file(GLOB_RECURSE HLSL_VERTEX_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_DIRECTORY}/*.vert.hlsl")
    file(GLOB_RECURSE HLSL_FRAGEMENT_SHADER_SOURCE_FILES CONFIGURE_DEPENDS "${SHADER_DIRECTORY}/*.frag.hlsl")

    # Compile HLSL vertex shaders to Compiled Shader Objects
    foreach (VERT_SHADER_FILE IN LISTS HLSL_VERTEX_SHADER_SOURCE_FILES)
        file(RELATIVE_PATH VERT_SHADER_FILE_RELATIVE ${SHADER_DIRECTORY} ${VERT_SHADER_FILE})
        get_filename_component(FILENAME ${VERT_SHADER_FILE} NAME)
        add_custom_command(
                COMMAND
                dxc "${VERT_SHADER_FILE}" -E VS -T vs_4_0 -Fo "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${VERT_SHADER_FILE_RELATIVE}.cso"
                OUTPUT "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${VERT_SHADER_FILE_RELATIVE}.cso"
                DEPENDS "${VERT_SHADER_FILE}" "${SHADER_BINARY_DIR}"
                COMMENT "Compiling HLSL Shader \"${SHADER_RELATIVE_PATH}/${VERT_SHADER_FILE_RELATIVE}\""
        )
        list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${VERT_SHADER_FILE_RELATIVE}.cso")
    endforeach ()

    # Compile HLSL fragment shaders to Compiled Shader Objects
    foreach (FRAG_SHADER_FILE IN LISTS HLSL_FRAGEMENT_SHADER_SOURCE_FILES)
        file(RELATIVE_PATH FRAG_SHADER_FILE_RELATIVE ${SHADER_DIRECTORY} ${FRAG_SHADER_FILE})
        get_filename_component(FILENAME ${FRAG_SHADER_FILE} NAME)
        add_custom_command(
                COMMAND
                dxc "${FRAG_SHADER_FILE}" -E PS -T ps_4_0 -Fo "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${FRAG_SHADER_FILE_RELATIVE}.cso"
                OUTPUT "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${FRAG_SHADER_FILE_RELATIVE}.cso"
                DEPENDS "${FRAG_SHADER_FILE}" "${SHADER_BINARY_DIR}"
                COMMENT "Compiling HLSL Shader \"${SHADER_RELATIVE_PATH}/${FRAG_SHADER_FILE_RELATIVE}\""
        )
        list(APPEND COMPILED_SHADERS "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}/${FRAG_SHADER_FILE_RELATIVE}.cso")
    endforeach ()

    # Package the variants into a DShaderPackage file
    set(SHADER_PACKAGE "${SHADER_PACKAGE_DST_DIR}/${SHADER_RELATIVE_PATH}.dsp")
    add_custom_command(
            COMMAND
            "${CMAKE_BINARY_DIR}/DShaderPackage"
            "${SHADER_BINARY_DIR}/${SHADER_RELATIVE_PATH}"
            "${SHADER_PACKAGE}"
            DEPENDS "${SHADER_PACKAGE_DST_DIR}" "${CMAKE_BINARY_DIR}/DShaderPackage" ${COMPILED_SHADERS}
            COMMENT "Packaging shader ${SHADER_RELATIVE_PATH}"
            OUTPUT "${SHADER_PACKAGE}"
    )
    list(APPEND PACKAGED_SHADERS "${SHADER_PACKAGE}")
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

# Compile DpacDeflate executable
add_custom_command(
        COMMAND
        "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target DpacDeflate --config "${CMAKE_BUILD_TYPE}"
        COMMENT "Building DpacDeflate Tool"
        OUTPUT "${CMAKE_BINARY_DIR}/DpacDeflate"
)

# Use DpacDeflate to package the Engine Resources
add_custom_command(
        COMMAND
        "${CMAKE_BINARY_DIR}/DpacDeflate"
        "${RESOURCES_BINARY_DIRECTORY}"
        "${ENGINE_RESOURCE_PACKAGE}"
        DEPENDS "${SHADER_BINARY_DIR}" "${ENGINE_WORKING_DIRECTORY}" "${CMAKE_BINARY_DIR}/DpacDeflate" ${COPIED_RESOURCES} ${PACKAGED_SHADERS}
        COMMENT "Packaging engine resources"
        OUTPUT "${ENGINE_RESOURCE_PACKAGE}"
)
add_custom_target(Dyngine_Engine_PackageResources ALL DEPENDS ${ENGINE_RESOURCE_PACKAGE})
add_dependencies(Dyngine_Engine_PackageResources DpacDeflate)

# Setup Working Directory
# Copy library files
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(LLGL_LIBRARY_POST_FIX "D")
else ()
    set(LLGL_LIBRARY_POST_FIX "")
endif ()

# Get library extension
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(LIBRARY_EXTENSION "dll")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LIBRARY_EXTENSION "so")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LIBRARY_EXTENSION "dylib")
else ()
    message(FATAL_ERROR "Unsupported system: ${CMAKE_SYSTEM_NAME}")
endif ()

# Determine shared library files to copy into engine working directory
# Copy LLGL dll files
set(LIBRARY_FILES "${CMAKE_BINARY_DIR}/libraries/LLGL/build/LLGL${LLGL_LIBRARY_POST_FIX}.${LIBRARY_EXTENSION}" ; "${CMAKE_BINARY_DIR}/libraries/LLGL/build/LLGL_${LLGL_GRAPHICS_API}${LLGL_LIBRARY_POST_FIX}.${LIBRARY_EXTENSION}")

message(STATUS "Library files to copy: ${LIBRARY_FILES}")

add_custom_command(
        COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${LIBRARY_FILES} ${ENGINE_WORKING_DIRECTORY}
        DEPENDS "${ENGINE_WORKING_DIRECTORY}" LLGL LLGL_${LLGL_GRAPHICS_API}
        COMMENT "Copying engine libraries to engine working directory"
        OUTPUT "LIB_COPY"
)

add_custom_target(Dyngine_Engine_SetupWorkingDirectory ALL DEPENDS Dyngine_Engine_PackageResources LIB_COPY)

add_dependencies(Dyngine_Engine Dyngine_Engine_SetupWorkingDirectory)