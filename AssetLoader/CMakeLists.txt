project(Dyngine_AssetLoader)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_AssetLoader STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_AssetLoader PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")