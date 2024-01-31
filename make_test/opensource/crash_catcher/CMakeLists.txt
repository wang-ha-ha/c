cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILEON ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER mips-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER mips-linux-gnu-g++)

project(crash_catcher_handler)
add_definitions(-muclibc)
link_directories(${70MAI_DIR}/lib/uclibc)
include_directories(${70MAI_DIR}/include)
add_library(${PROJECT_NAME} SHARED crash_catcher_signal_handler.c crash_catcher_collect_info.cpp)
target_compile_options(${PROJECT_NAME} 
    PUBLIC -fPIC 
    PUBLIC -DCOLLECT_LOCAL111)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC unwind
    PUBLIC dl)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})


project(crash_catcher)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build)
add_executable(${PROJECT_NAME} main.cpp CrashDumper.cpp)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC unwind 
    PUBLIC unwind-ptrace 
    PUBLIC unwind-generic)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

project(crash_catcher_test)
message("WCQ==${CMAKE_CURRENT_SOURCE_DIR}/build")
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build)
link_directories(${70MAI_DIR}/lib/uclibc)
add_executable(${PROJECT_NAME} EXCLUDE_FROM_ALL test_main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC crash_catcher_handler)
