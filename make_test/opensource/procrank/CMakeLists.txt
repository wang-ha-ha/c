cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILEON ON)
set(CMAKE_C_COMPILER mips-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER mips-linux-gnu-g++)


PROJECT (pagemap)
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/libpagemap/*.c")
add_definitions (-D_LARGEFILE64_SOURCE)
include_directories (${PROJECT_SOURCE_DIR}/libpagemap/include)
add_library(${PROJECT_NAME} STATIC ${SOURCES})
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})


project(procmem)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/procmem/procmem.c)
target_link_libraries(${PROJECT_NAME} pagemap)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

project(procrank)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/procrank/procrank.c ${PROJECT_SOURCE_DIR}/procrank/strlcpy.c)
target_link_libraries(${PROJECT_NAME} pagemap)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
