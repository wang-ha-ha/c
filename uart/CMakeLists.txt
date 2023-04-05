get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${CURRENT_FOLDER})
set(CMAKE_C_COMPILER mips-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER mips-linux-gnu-g++)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/  DIR_SRCS)
aux_source_directory(${70MAI_DIR}/../communication/wifi_soc/module/nanopb/ DIR_SRCS)
aux_source_directory(${70MAI_DIR}/../communication/wifi_soc/module/stream/ DIR_SRCS)
aux_source_directory(${70MAI_DIR}/../communication/wifi_soc/module/comm_protocol/iot/board/t41/ DIR_SRCS)
aux_source_directory(${70MAI_DIR}/../communication/wifi_soc/module/comm_protocol/iot/pb/ DIR_SRCS)
aux_source_directory(${70MAI_DIR}/../communication/wifi_soc/module/comm_protocol/port/t41/ DIR_SRCS)
aux_source_directory(${70MAI_DIR}/../communication/wifi_soc/module/port/arch_os/linux/ DIR_SRCS)
aux_source_directory(${70MAI_DIR}/../communication/wifi_soc/module/utils/ DIR_SRCS)

if("${CONFIG_UCLIBC_BUILD}" STREQUAL "y")
    set(CMAKE_CXX_FLAGS "-Wno-error=deprecated-declarations -Wno-deprecated-declarations -muclibc")
    add_definitions(-g -Wl,--whole-archive   -Wl,--no-whole-archive -O2 -std=c++1y -Wdeprecated-declarations -muclibc)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_DIR}/bin/uclibc)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/uclibc)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/uclibc)
    link_directories(${70MAI_DIR}/zeratul-sdk/lib/uclibc)
    link_directories(${70MAI_DIR}/lib/uclibc)
    link_directories(${70MAI_DIR}/lib/uclibc/platform-media)
else()
    set(CMAKE_CXX_FLAGS "-Wno-error=deprecated-declarations -Wno-deprecated-declarations ")
    add_definitions(-g -Wl,--whole-archive   -Wl,--no-whole-archive -O2 -std=c++1y -Wdeprecated-declarations)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_DIR}/bin/glibc)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/glibc)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/glibc)
    link_directories(${70MAI_DIR}/zeratul-sdk/lib/glibc)
    link_directories(${70MAI_DIR}/lib/glibc)
endif()

include_directories(${70MAI_DIR}/platform-osp)
include_directories(${70MAI_DIR}/platform-osp/libosp/include)
include_directories(${70MAI_DIR}/zeratul-sdk/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../platform-media/mips32/zeratul_interface_T41/include  DIR_SRCS)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libcore/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libcrypt/include)
include_directories(${70MAI_DIR}/include)
include_directories(${70MAI_DIR}/../communication/wifi_soc/module/nanopb)
include_directories(${70MAI_DIR}/../communication/wifi_soc/module/stream)
include_directories(${70MAI_DIR}/../communication/wifi_soc/module/comm_protocol/iot/board/t41)
include_directories(${70MAI_DIR}/../communication/wifi_soc/module/comm_protocol/iot/pb)
include_directories(${70MAI_DIR}/../communication/wifi_soc/module/comm_protocol/port)
include_directories(${70MAI_DIR}/../communication/wifi_soc/module/comm_protocol/port/t41)
include_directories(${70MAI_DIR}/../communication/wifi_soc/module/port/arch_os/linux)
include_directories(${70MAI_DIR}/../communication/wifi_soc/module/utils)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cli_agent_client_config.hpp
    COMMAND  python ${CMAKE_CURRENT_SOURCE_DIR}/res/cli2cpp.py --cli-class-name "cli_agent_client" ${CMAKE_CURRENT_SOURCE_DIR}/res/cli_agent_client_config.xml --output ${CMAKE_CURRENT_SOURCE_DIR}/include/cli_agent_client_config.hpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/cli_agent_client_config.xml
    COMMENT "CLIConfigTAG build"
)

add_executable(${CURRENT_FOLDER}  ${DIR_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/cli_agent_client_config.hpp)
target_link_libraries(${CURRENT_FOLDER} pthread json uuid dl core osp rt alog imp zeratul_interface_T41)

# CLILib
target_link_libraries(${CURRENT_FOLDER} CLILib ncurses)

install(TARGETS ${CURRENT_FOLDER} DESTINATION bin)

