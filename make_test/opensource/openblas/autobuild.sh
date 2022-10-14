#!/bin/bash -e

source ../common.sh

build_openblas()
{
	if [ ! -e OpenBLAS-0.3.10 ]; then
		[ ! -e v0.3.10.tar.gz ] && wget https://github.com/xianyi/OpenBLAS/archive/v0.3.10.tar.gz
		tar xf v0.3.10.tar.gz
	fi

	pushd OpenBLAS-0.3.10
	echo "[openblas] make"
	make BINARY=32 CC=${CROSS_COMPILE}gcc HOSTCC=gcc TARGET=ARMV7 &> make.log
	make PREFIX=$INSTALL_DIR/openblas install &> install.log

	echo "[openblas] Finish"
	popd
}


clean_openblas()
{
	rm -rf OpenBLAS-0.3.10
}



if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_openblas
elif [ "$1" = "clean" ]; then
	clean_openblas
fi
