cmake_minimum_required(VERSION 3.4)

project(yaml_test)

set(CMAKE_CXX_STANDARD 11)

add_executable(yaml_test main.cpp)

target_link_libraries(yaml_test yaml-cpp)
