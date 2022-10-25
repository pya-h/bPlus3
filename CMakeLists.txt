cmake_minimum_required(VERSION 3.8)
project(CS539_Project)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(src)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3 ")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(test test.cpp)
target_link_libraries(test BP3)
