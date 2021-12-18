project(DpacTools)

set(CMAKE_CXX_STANDARD 20)

add_executable(DShaderPackage src/DShaderPackage.cpp)

target_link_libraries(DShaderPackage PRIVATE Dyngine_DShader)
target_link_libraries(DShaderPackage PRIVATE Dyngine_ErrorHandling)