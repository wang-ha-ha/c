#!/bin/bash -e

source ../common.sh

build_libmbedtls() {
	echo "[libmbedtls] make"
	rm -rf build*
	mkdir -p build
	mkdir -p build_out
	cd build
	cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/../build_out \
			-DCMAKE_C_COMPILER=mips-linux-gnu-gcc \
			-DCMAKE_C_FLAGS="-muclibc -O3 -fPIC" \
			.. > cmake.log
	make > make.log
	make install > install.log
	make clean
	echo "[libmbedtls] done"
}

clean_libmbedtls()
{
	make clean &>make.log
}

rootfs_install_libmbedtls()
{
	echo "[libmbedtls] rootfs_install"
	mkdir -p $APP_LIB_DIR
	mkdir -p $APP_INCLUDE_DIR
	cp -rf mbedtls/build_out/lib/* $APP_DIR/libcrypt/lib/static_lib
}

install_libmbedtls()
{
	rootfs_install_libmbedtls
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libmbedtls
elif type -t $1_libmbedtls 2> /dev/null >&2 ; then
        $1_libmbedtls
fi
