#!/bin/bash
set -e

if [ -f .env ]; then
    source .env
    source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo -e "\033[31m"[ERROR]Please run source at top of zeratul project"\033[0m"
    exit 1
fi

# cd $ZRT_ENV_TOP_DIR/70mai/alg/libimgalgo
# if [ $1 == true ]; then
# ./autobuild.sh clean
# fi
# ./autobuild.sh
# ./autobuild.sh install
# cd -
if [ -f $ZRT_ENV_TOP_DIR/70mai/app/Makefile ]; then
    cd $ZRT_ENV_TOP_DIR/70mai/app
elif [ -f $ZRT_ENV_TOP_DIR/70mai/Makefile ]; then
    cd $ZRT_ENV_TOP_DIR/70mai
else
    logger_info error_info "Makefile not found"
    exit 1
fi

if [ $1 == true ]; then
    rm -rf build
fi
make
cd -