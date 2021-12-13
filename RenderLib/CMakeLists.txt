project(Dyngine_RenderLib)

set(CMAKE_CXX_STANDARD 17)

set(RENDERLIB_BACKEND Vulkan)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/main/*.cpp")

if (RENDERLIB_BACKEND MATCHES "Vulkan")
    file(GLOB_RECURSE VULKAN_SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/vulkan/*.cpp")
endif ()

add_library(Dyngine_RenderLib STATIC ${SOURCE_FILES};${VULKAN_SOURCE_FILES})
target_include_directories(Dyngine_RenderLib PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include/public/main")

target_link_libraries(Dyngine_RenderLib PRIVATE glfw)

if (RENDERLIB_BACKEND MATCHES "Vulkan")
    find_package(Vulkan REQUIRED FATAL_ERROR)
    if (NOT Vulkan_FOUND)
        message(FATAL_ERROR "Vulkan not found")
    endif ()
    target_include_directories(Dyngine_RenderLib PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include/public/vulkan")
    target_include_directories(Dyngine_RenderLib PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include/private/vulkan")
    target_include_directories(Dyngine_RenderLib PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(Dyngine_RenderLib PRIVATE ${Vulkan_LIBRARIES})
endif ()

# Depends on ErrorHandling module
target_link_libraries(Dyngine_RenderLib PUBLIC Dyngine_ErrorHandling)