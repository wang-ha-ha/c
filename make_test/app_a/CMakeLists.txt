set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

aux_source_directory(src/  DIR_SRCS)
aux_source_directory(m/  DIR_SRCS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib_a_so/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib_b_so/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${CURRENT_FOLDER}  ${DIR_SRCS})
target_link_libraries(${CURRENT_FOLDER} lib_a_so lib_b_so )

install(TARGETS ${CURRENT_FOLDER} DESTINATION bin)