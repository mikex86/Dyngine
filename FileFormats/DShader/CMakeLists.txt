project(Dyngine_DShader)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_DShader STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_DShader PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")

# Depends on ErrorHandling module
target_link_libraries(Dyngine_DShader PUBLIC Dyngine_ErrorHandling)

# Depends on Stream module
target_link_libraries(Dyngine_DShader PUBLIC Dyngine_Stream)

# Tests
file(GLOB_RECURSE TEST_SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/tests/*.cpp")
add_executable(Dyngine_DShader_Test ${TEST_SOURCE_FILES})
target_link_libraries(Dyngine_DShader_Test PRIVATE Dyngine_DShader)
# Depends on Google Test
target_link_libraries(Dyngine_DShader_Test PUBLIC gtest_main)
# Depends on Stream
target_link_libraries(Dyngine_DShader_Test PUBLIC Dyngine_Stream)

add_test(NAME FileDataReadStream COMMAND Open)