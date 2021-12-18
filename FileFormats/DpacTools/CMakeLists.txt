project(DpacTools)

set(CMAKE_CXX_STANDARD 17)

add_executable(DpacDeflate src/DpacDeflate.cpp)
add_executable(DpacList src/DpacList.cpp)
add_executable(DpacGet src/DpacGet.cpp)

# Depends on Dpac Module
target_link_libraries(DpacDeflate PUBLIC Dyngine_Dpac)
target_link_libraries(DpacList PUBLIC Dyngine_Dpac)
target_link_libraries(DpacGet PUBLIC Dyngine_Dpac)

# Depends on Utils Module
target_link_libraries(DpacDeflate PUBLIC Dyngine_Utils)
target_link_libraries(DpacList PUBLIC Dyngine_Dpac)
target_link_libraries(DpacGet PUBLIC Dyngine_Dpac)