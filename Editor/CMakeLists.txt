project(Dyngine_Editor)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_PREFIX_PATH $ENV{QT_DIRECTORY})

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE UI_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/ui/*.ui")

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(Dyngine_Editor ${SOURCE_FILES})

# Generate Qt wrappers from ui files
qt5_wrap_ui(UI_HEADERS ${UI_FILES})
set(UI_HEADERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/uiheaders")
file(MAKE_DIRECTORY "${UI_HEADERS_DIR}")

# copy generated headers into folder
add_custom_target(COPY_QT_WRAPPER_HEADERS
        DEPENDS ${UI_HEADERS} ${UI_FILES}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${UI_HEADERS} ${UI_HEADERS_DIR}
        COMMENT "Copying generated qt wrapper headers"
        OUTPUT "COPY_QT_WRAPPER_HEADERS")

add_dependencies(Dyngine_Editor "COPY_QT_WRAPPER_HEADERS")

# include that directory
target_include_directories(Dyngine_Editor PRIVATE "${UI_HEADERS_DIR}")

target_include_directories(Dyngine_Editor PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private")
target_include_directories(Dyngine_Editor PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")

# Set Editor working directory
set(EDITOR_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/working_directory")

# Create Editor working directory
file(MAKE_DIRECTORY "${EDITOR_WORKING_DIRECTORY}")

# Link libraries
target_link_libraries(Dyngine_Editor PRIVATE Dyngine_ErrorHandling)

target_link_libraries(Dyngine_Editor PRIVATE Dyngine_Engine) # Dyngine Engine dependency

# QT

# Required QT libraries list
set(REQUIRED_QT_LIBRARIES Gui;Core;Widgets)


find_package(Qt5 COMPONENTS ${REQUIRED_QT_LIBRARIES} REQUIRED)

# Link against required QT libraries
if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(QT_LIBRARY_POST_FIX "d")
else ()
    set(QT_LIBRARY_POST_FIX "")
endif ()
foreach (LIB IN LISTS REQUIRED_QT_LIBRARIES)
    target_link_libraries(Dyngine_Editor PRIVATE Qt5::${LIB})
    get_property(QT_LIB_LOCATION TARGET Qt5::${LIB} PROPERTY LOCATION)

    # Append QT_LIBRARY_POST_FIX to QT_LIB_LOCATION before the library extension
    get_filename_component(QT_LIB_FILE_DIR "${QT_LIB_LOCATION}" DIRECTORY)
    get_filename_component(QT_LIB_FILE_NAME "${QT_LIB_LOCATION}" NAME_WE)
    get_filename_component(QT_LIB_FILE_EXT "${QT_LIB_LOCATION}" EXT)
    set(QT_LIB_LOCATION "${QT_LIB_FILE_DIR}/${QT_LIB_FILE_NAME}${QT_LIBRARY_POST_FIX}${QT_LIB_FILE_EXT}")
    message(STATUS "QT_LIBRARY_FILE_NAME: ${QT_LIB_LOCATION}")
    list(APPEND QT_LIBRARY_FILES "${QT_LIB_LOCATION}")
endforeach ()

message(STATUS "QT libraries: ${QT_LIBRARY_FILES}")


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

# Add library files for required QT libraries
foreach (QT_LIBRARY_FILE IN LISTS QT_LIBRARY_FILES)
    list(APPEND LIBRARY_FILES "${QT_LIBRARY_FILE}")
endforeach ()

message(STATUS "Library files to copy: ${LIBRARY_FILES}")


add_custom_command(
        COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${LIBRARY_FILES} ${EDITOR_WORKING_DIRECTORY}
        DEPENDS "${EDITOR_WORKING_DIRECTORY}" LLGL LLGL_${LLGL_GRAPHICS_API}
        COMMENT "Copying editor libraries to editor working directory"
        OUTPUT "LIB_COPY"
)

add_custom_target(Dyngine_Editor_SetupWorkingDirectory ALL DEPENDS LIB_COPY)

add_dependencies(Dyngine_Editor Dyngine_Editor_SetupWorkingDirectory)