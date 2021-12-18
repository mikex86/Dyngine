project(Dyngine_Stream)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_Stream STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_Stream PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")
target_include_directories(Dyngine_Stream PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private")

# Depends on ErrorHandling Module
target_link_libraries(Dyngine_Stream PUBLIC Dyngine_ErrorHandling)

# Depends on Utils Module
target_link_libraries(Dyngine_Stream PUBLIC Dyngine_Utils)

# Depends on zstd module
target_link_libraries(Dyngine_Stream PRIVATE libzstd_static)
target_include_directories(Dyngine_Stream PRIVATE "${CMAKE_SOURCE_DIR}/libraries/zstd/lib")

# Tests
file(GLOB_RECURSE TEST_SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/tests/*.cpp")
add_executable(Dyngine_Stream_Test ${TEST_SOURCE_FILES})
target_link_libraries(Dyngine_Stream_Test PRIVATE Dyngine_Stream)
# Depends on Google Test
target_link_libraries(Dyngine_Stream_Test PUBLIC gtest_main)
add_test(NAME FileDataReadStream COMMAND Open)