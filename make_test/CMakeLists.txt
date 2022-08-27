cmake_minimum_required(VERSION 3.5)
project(test_project)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/out/lib)

set(CMAKE_CXX_FLAGS "-Wno-error=deprecated-declarations -Wno-deprecated-declarations ")
add_definitions(-g -Wl,--whole-archive   -Wl,--no-whole-archive -O2 -DPCAPP=1 -Wdeprecated-declarations)

link_directories(${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(lib_a_so)
add_subdirectory(lib_b_so)
add_subdirectory(app_a)
add_subdirectory(app_b)
