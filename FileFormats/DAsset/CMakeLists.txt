project(Dyngine_DAsset)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_DAsset STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_DAsset PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")

target_link_libraries(Dyngine_DAsset PUBLIC Dyngine_Stream)
target_link_libraries(Dyngine_DAsset PUBLIC glm)