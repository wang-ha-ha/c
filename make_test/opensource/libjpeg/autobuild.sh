#!/bin/bash -e

source ../common.sh

build_libjpeg()
{
	echo "[libjpeg] build"
	echo ${CONFIG_CROSS_COMPILE}gcc
	tar xvf jpegsrc.v9e.tar.gz

	cd jpeg-9e
	./configure CC=${CONFIG_CROSS_COMPILE}gcc --prefix=$PWD/_install --enable-shared --enable-static --host=arm-linux
	mkdir -p _install/bin _install/include _install/lib _install/man/man1 
	make && make install
	${CONFIG_CROSS_COMPILE}strip _install/lib/libjpeg.so.9.5.0

	echo "[libjpeg] done"
}

clean_libjpeg()
{
	rm -rf jpeg-9e
}

rootfs_install_libjpeg()
{
	echo "[libjpeg] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -a $INSTALL_DIR/jpeg-9e/_install/lib/libjpeg.so* $ROOTFS/lib
}

install_libjpeg()
{
    rootfs_install_libjpeg
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "libjpeg" ]; then
	build_libjpeg
elif type -t $1_libjpeg 2> /dev/null >&2 ; then
	$1_libjpeg
fi
