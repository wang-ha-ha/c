#!/bin/bash -e

source ../common.sh

build_quirc() {
	pushd quirc

	echo "[quirc] make"
	make CC=mips-linux-gnu-gcc libquirc.so &> make.log
	make CC=mips-linux-gnu-gcc install DESTDIR=$INSTALL_DIR/quirc &> install.log
	echo "[quirc] done"

	popd
}

clean_quirc()
{
	pushd quirc

	make CC=mips-linux-gnu-gcc clean &>make.log

	popd
}

rootfs_install_quirc()
{
	echo "[quirc] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -rf $INSTALL_DIR/quirc/lib/libquirc.so $ROOTFS/lib
}

install_quirc()
{
	rootfs_install_quirc
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_quirc
elif type -t $1_quirc 2> /dev/null >&2 ; then
        $1_quirc
fi
