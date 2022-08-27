set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g)

aux_source_directory(src/  DIR_SRCS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_library(${CURRENT_FOLDER} SHARED  ${DIR_SRCS})

install(TARGETS ${CURRENT_FOLDER} DESTINATION lib)