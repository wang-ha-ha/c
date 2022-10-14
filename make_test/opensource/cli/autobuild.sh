#!/bin/bash -e

source ../common.sh

build_cli() {
	echo "[cli] make"
	make #&> make.log
	make install-strip DESTDIR=$INSTALL_DIR/cli &> install.log
	echo "[cli] done"
}

clean_cli()
{
	make clean &>make.log
}

rootfs_install_cli()
{
	echo "[cli] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -rf $INSTALL_DIR/cli/lib/libCLILib.so $ROOTFS/lib
	cp -rf $INSTALL_DIR/cli/lib/libCLILib.so $CONFIG_TOP/app/70mai/lib
	cp -rf $INSTALL_DIR/cli/include/*  $CONFIG_TOP/app/70mai/include
}

install_cli()
{
	rootfs_install_cli
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_cli
elif type -t $1_cli 2> /dev/null >&2 ; then
        $1_cli
fi
