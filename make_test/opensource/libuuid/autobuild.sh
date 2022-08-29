#!/bin/bash -e

source ../common.sh

build_libuuid()
{

	pushd libuuid-1.0.3
	echo "[libuuid] libuuid $HOST"
	./configure --enable-cross-compile --enable-shared --without-ssl --without-zlib --host=$HOST &> configure.log

	echo "[libuuid] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/libuuid &> install.log

	echo "[libuuid] done"
	popd
}

clean_libuuid()
{
	pushd libuuid-1.0.3
	make clean &> make.log
	popd
}

rootfs_install_libuuid()
{
	echo "[libuuid] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -a $INSTALL_DIR/libuuid/usr/local/lib/libuuid.so* $ROOTFS/lib
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libuuid
elif type -t $1_libuuid 2> /dev/null >&2 ; then
	$1_libuuid
fi
