#!/bin/bash -e

source ../common.sh

build_cJSON() {
	echo "[cJSON] make"
	make #&> make.log
	make install-strip DESTDIR=$INSTALL_DIR/cJSON &> install.log
	echo "[cJSON] done"
}

clean_cJSON()
{
	make clean &>make.log
}

rootfs_install_cJSON()
{
	echo "[cJSON] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -rf $INSTALL_DIR/cJSON/lib/* $ROOTFS/lib
}

install_cJSON()
{
	rootfs_install_cJSON
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_cJSON
elif type -t $1_cJSON 2> /dev/null >&2 ; then
        $1_cJSON
fi
