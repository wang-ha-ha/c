#!/bin/bash -e

source ../common.sh

build_RCF() {
	echo "[RCF] make"
	make #&> make.log
	make install-strip DESTDIR=$INSTALL_DIR/RCF &> install.log
	echo "[RCF] done"
}

clean_RCF()
{
	make clean &>make.log
}

rootfs_install_RCF()
{
	echo "[RCF] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -rf $INSTALL_DIR/RCF/lib/libRCF.so $ROOTFS/lib
}

install_RCF()
{
	rootfs_install_RCF
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_RCF
elif type -t $1_RCF 2> /dev/null >&2 ; then
        $1_RCF
fi
