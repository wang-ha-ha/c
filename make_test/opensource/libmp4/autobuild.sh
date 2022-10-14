#!/bin/bash -e

source ../common.sh

build_libmp4() {
	if [ ! -e libmp4 ]; then
		tar xf libmp4.tar.gz
		pushd libmp4
		tar xf ../libfutils.tar.gz
		patch -p1 < ../libmp4.patch
		popd
	fi

	pushd libmp4
	echo "[libmp4] make"
	make &> make.log

	echo "[libmp4] done"
	popd
}

build_libmp4
