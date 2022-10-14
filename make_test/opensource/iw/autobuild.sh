#!/bin/bash -e

source ../common.sh

build_iw() {
	setup https://mirrors.edge.kernel.org/pub/software/network/iw/iw-5.9.tar.xz

	pushd iw-5.9

	echo "[iw] patch"
	patch -p1 < ../iw-5.9.patch

	echo "[iw] make"
	CC=${CROSS_COMPILE}gcc make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/iw &> install.log

	echo "[iw] done"
}

clean_iw()
{
	rm -rf iw-5.9
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_iw
elif [ "$1" = "clean" ]; then
	clean_iw
fi
