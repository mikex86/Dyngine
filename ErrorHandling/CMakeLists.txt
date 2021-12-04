project(Dyngine_ErrorHandling)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_ErrorHandling STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_ErrorHandling PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")