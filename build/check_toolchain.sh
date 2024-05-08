#!/bin/bash

if [ -f .env ]; then
    source .env
    source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project"
    exit 1
fi

if [ ! -d ${ZRT_ENV_TOOLS_DIR}/toolchain/gcc_720/mips-linux-gnu-ingenic-gcc7.2.0-glibc2.29-fp64-r5.1.1.sr02 ];then
    cd ${ZRT_ENV_TOOLS_DIR}/toolchain/gcc_720/
    tar xjf mips-linux-gnu-ingenic-gcc7.2.0-glibc2.29-fp64-r5.1.1.sr02.tar.bz2
fi

if [ ! -d ${ZRT_ENV_TOOLS_DIR}/toolchain/gcc_mcu/gcc-arm-none-eabi-10.3-2021.10 ];then
    cd ${ZRT_ENV_TOOLS_DIR}/toolchain/gcc_mcu/
    tar xjf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
fi
CMP_TOOLCHAIN=`mips-linux-gnu-gcc --version | grep  "${ZRT_ENV_TOOL_CHAIN}"`

if [ ! -n "${CMP_TOOLCHAIN}" ]; then
	logger_info error_info "Toolchain version is not ${ZRT_ENV_TOOL_CHAIN}"
	exit 1
else
	logger_info normal_info "ZRT_ENV_TOOL_CHAIN is ${ZRT_ENV_TOOL_CHAIN}"
	exit 0
fi
