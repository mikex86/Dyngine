project(Dyngine_Buffer)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_Buffer STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_Buffer PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")

# Depends on ErrorHandling Module
target_link_libraries(Dyngine_Buffer PUBLIC Dyngine_ErrorHandling)

# Depends on Utils Module
target_link_libraries(Dyngine_Buffer PUBLIC Dyngine_Utils)