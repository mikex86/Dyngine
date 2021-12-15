project(Dyngine_InputOutput)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_InputOutput STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_InputOutput PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")
target_include_directories(Dyngine_InputOutput PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private")

# Depends on ErrorHandling Module
target_link_libraries(Dyngine_InputOutput PUBLIC Dyngine_ErrorHandling)

# Depends on Utils Module
target_link_libraries(Dyngine_InputOutput PUBLIC Dyngine_Utils)

# Depends on zstd module
target_link_libraries(Dyngine_InputOutput PRIVATE libzstd_static)

# Tests
file(GLOB_RECURSE TEST_SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/tests/*.cpp")
add_executable(Dyngine_InputOutput_Test ${TEST_SOURCE_FILES})
target_link_libraries(Dyngine_InputOutput_Test PRIVATE Dyngine_InputOutput)
# Depends on Google Test
target_link_libraries(Dyngine_InputOutput_Test PUBLIC gtest_main)
add_test(NAME FileDataReadStream COMMAND Open)