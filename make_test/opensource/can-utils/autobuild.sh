#!/bin/bash -e

source ../common.sh

build_canu()
{
	tar xf can-utils.tgz
	pushd can-utils
	echo "[can-utils] make"
	make CC=${CROSS_COMPILE}gcc AR=${CROSS_COMPILE}ar LD=${CROSS_COMPILE}ld STRIP=${CROSS_COMPILE}strip &> make.log
	mkdir -p $INSTALL_DIR/can-utils
	cp candump canfdtest cangen cansend $INSTALL_DIR/can-utils
	echo "[can-utils] done"
	popd
}
clean_canu()
{
	rm -rf can-utils
}

rootfs_install_canu()
{
	echo "[can-utils] rootfs_install"
	mkdir -p $ROOTFS/usr/bin
	cp -a $INSTALL_DIR/can-utils/{candump,canfdtest,cangen,cansend} $ROOTFS/usr/bin
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_canu
elif type -t $1_canu 2> /dev/null >&2 ; then
        $1_canu
fi
