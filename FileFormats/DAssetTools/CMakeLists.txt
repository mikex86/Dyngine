project(DpacTools)

set(CMAKE_CXX_STANDARD 20)

add_executable(DAssetConvert src/DAssetConvert.cpp)
add_executable(DAssetPrint src/DAssetPrint.cpp)

target_link_libraries(DAssetConvert PRIVATE tinygltf)
target_link_libraries(DAssetConvert PRIVATE Dyngine_DAsset)
target_link_libraries(DAssetConvert PRIVATE Dyngine_ErrorHandling)

target_link_libraries(DAssetPrint PRIVATE tinygltf)
target_link_libraries(DAssetPrint PRIVATE Dyngine_DAsset)
target_link_libraries(DAssetPrint PRIVATE Dyngine_ErrorHandling)