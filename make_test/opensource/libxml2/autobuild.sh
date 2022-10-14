#!/bin/bash -e

source ../common.sh




build_libxml2() {
    tar xzf libxml2-2.7.1.tar.gz
	cd libxml2-2.7.1
	export CROSS=$CROSS_COMPILE

	echo "[libxml2] configure" 
	./configure CROSS_COMPILE=${CROSS}  --prefix=$INSTALL_DIR/libxml2  &> log.config
	echo "[libxml2] make"

	make &> make.log
	echo "[libxml2] make install"
	make install &> install.log
	echo "[libxml2] done"
}

clean_libxml2()
{
	echo "[libxml2] clean"	
	rm -rf libxml2-2.7.1
}

rootfs_install_libxml2()
{
	echo "[libxml2] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp  $INSTALL_DIR/libxml2/lib/libxml2.so $ROOTFS/lib
	cp  $INSTALL_DIR/libxml2/lib/libxml2.so $CONFIG_TOP/app/70mai/lib
	cp -rf $INSTALL_DIR/libxml2/include/*  $CONFIG_TOP/app/70mai/include
}

install_libxml2()
{
	rootfs_install_libxml2
}
#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libxml2
elif type -t $1_libxml2 2> /dev/null >&2 ; then
	$1_libxml2
fi