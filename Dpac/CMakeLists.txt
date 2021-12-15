project(Dyngine_Dpac)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_Dpac STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_Dpac PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")

# Depends on ErrorHandling module
target_link_libraries(Dyngine_Dpac PUBLIC Dyngine_ErrorHandling)

# Depends on InputOutput module
target_link_libraries(Dyngine_Dpac PUBLIC Dyngine_InputOutput)

# Tests
file(GLOB_RECURSE TEST_SOURCE_FILES "${CMAKE_CURRENT_LIST_DIR}/tests/*.cpp")
add_executable(Dyngine_Dpac_Test ${TEST_SOURCE_FILES})
target_link_libraries(Dyngine_Dpac_Test PRIVATE Dyngine_Dpac)
# Depends on Google Test
target_link_libraries(Dyngine_Dpac_Test PUBLIC gtest_main)
# Depends on InputOutput
target_link_libraries(Dyngine_Dpac_Test PUBLIC Dyngine_InputOutput)

add_test(NAME FileDataReadStream COMMAND Open)