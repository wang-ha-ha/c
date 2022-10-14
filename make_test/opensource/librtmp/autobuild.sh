#!/bin/bash -e

source ../common.sh

build_librtmp()
{

	echo "[librtmp] build"
	make #&> make.log
	make install-strip DESTDIR=$INSTALL_DIR/librtmp &> install.log

	echo "[librtmp] done"
}

clean_librtmp()
{
	make clean #&> make.log
}

rootfs_install_librtmp()
{
	echo "[librtmp] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -a $INSTALL_DIR/librtmp/lib/librtmp.so* $ROOTFS/lib
	cp -a $INSTALL_DIR/librtmp/lib/librtmp.so* $CONFIG_TOP/app/70mai/lib 
	cp -fa $INSTALL_DIR/librtmp/include/* $CONFIG_TOP/app/70mai/include

}

install_librtmp()
{
        rootfs_install_librtmp
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "librtmp" ]; then
	build_librtmp
elif type -t $1_librtmp 2> /dev/null >&2 ; then
	$1_librtmp
fi
