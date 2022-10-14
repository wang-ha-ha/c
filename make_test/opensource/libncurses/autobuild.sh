#!/bin/bash -e

source ../common.sh

build_libncurses() {
    tar xzf ncurses-5.9.tar.gz
	cd ncurses-5.9

	echo "[libncurses] configure" 
	./configure CC=${CROSS_COMPILE}gcc --host=arm-linux  --prefix=$INSTALL_DIR/libncurses --with-shared --without-cxx-binding &> log.config
	echo "[libncurses] make"

	make &> make.log
	echo "[libncurses] make install"
	make install &> install.log
	echo "[libncurses] done"
}

clean_libncurses()
{
	echo "[libncurses] clean"	
	rm -rf ncurses-5.9
}

rootfs_install_libncurses()
{
	echo "[libncurses] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -rf  $INSTALL_DIR/libncurses/lib/libncurses.so* $ROOTFS/lib
	cp -rf  $INSTALL_DIR/libncurses/lib/libncurses.so* $CONFIG_TOP/app/70mai/lib
	cp -rf $INSTALL_DIR/libncurses/include/*  $CONFIG_TOP/app/70mai/include
}

install_libncurses()
{
	rootfs_install_libncurses
}
#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libncurses
elif type -t $1_libncurses 2> /dev/null >&2 ; then
	$1_libncurses
fi
