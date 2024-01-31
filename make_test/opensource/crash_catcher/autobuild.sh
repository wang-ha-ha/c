#!/bin/bash -e

source ../common.sh

build_crash_catcher() {
	echo "[crash_catcher] make"
	rm -rf build*
	mkdir -p build
	mkdir -p build_out
	cd build
	cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/../build_out \
			-DCMAKE_C_FLAGS="-muclibc" \
			-DCMAKE_CXX_FLAGS="-muclibc" \
			-D70MAI_DIR=${ZRT_ENV_TOP_DIR}/70mai/app \
			.. > cmake.log
	make
	make install > install.log
	make clean
	echo "[crash_catcher] done"
}

clean_crash_catcher()
{
	make clean &>make.log
}

rootfs_install_crash_catcher()
{
	echo "[crash_catcher] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR
}

install_crash_catcher()
{
	rootfs_install_crash_catcher
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_crash_catcher
elif type -t $1_crash_catcher 2> /dev/null >&2 ; then
        $1_crash_catcher
fi
