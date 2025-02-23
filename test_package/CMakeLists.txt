cmake_minimum_required(VERSION 3.31 FATAL_ERROR)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
project(PackageTest CXX)

find_package(xpp CONFIG REQUIRED)

add_executable(example src/example.cpp)
target_link_libraries(example PUBLIC xpp::xpp)
add_dependencies(example xpp::xpp)