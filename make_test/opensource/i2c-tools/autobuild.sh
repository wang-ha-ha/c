#!/bin/bash -e

source ../common.sh

build_i2c_tools()
{
	setup https://www.kernel.org/pub/software/utils/i2c-tools/i2c-tools-4.2.tar.xz

	pushd i2c-tools-4.2

	echo "[i2c-tools] make"
	make CC=${CROSS_COMPILE}gcc AR=${CROSS_COMPILE}ar LD=${CROSS_COMPILE}ld STRIP=${CROSS_COMPILE}strip &> make.log
	make strip CC=${CROSS_COMPILE}gcc AR=${CROSS_COMPILE}ar LD=${CROSS_COMPILE}ld STRIP=${CROSS_COMPILE}strip &>> make.log
	make install PREFIX=$INSTALL_DIR/i2c-tools &> install.log

	echo "[i2c-tools] done"
}

clean_i2c_tools()
{
	rm -rf i2c-tools-4.2
}

rootfs_install_i2c_tools()
{
	echo "[i2c-tools] rootfs_install"
	mkdir -p $ROOTFS/usr/bin $ROOTFS/lib
	cp -a $INSTALL_DIR/i2c-tools/sbin/{i2cdetect,i2cdump,i2cset,i2cget,i2ctransfer} $ROOTFS/usr/bin
	cp -a $INSTALL_DIR/i2c-tools/lib/libi2c.so* $ROOTFS/lib
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_i2c_tools
elif type -t $1_i2c_tools 2> /dev/null >&2 ; then
	$1_i2c_tools
fi
