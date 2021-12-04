project(Dyngine_Utils)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_Utils STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_Utils PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")

# Depends on ErrorHandling module
target_link_libraries(Dyngine_Utils PUBLIC Dyngine_ErrorHandling)