project(Dyngine_Dpac)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(Dyngine_Dpac STATIC ${SOURCE_FILES})
target_include_directories(Dyngine_Dpac PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public")

# Depends on ErrorHandling module
target_link_libraries(Dyngine_Dpac PUBLIC Dyngine_ErrorHandling)

# Depends on Buffer module
target_link_libraries(Dyngine_Dpac PUBLIC Dyngine_Buffer)