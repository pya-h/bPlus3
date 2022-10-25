set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR})
add_library(BP3 STATIC bp3.cpp)
target_include_directories(BP3 PUBLIC ${PROJECT_SOURCE_DIR}/include)
