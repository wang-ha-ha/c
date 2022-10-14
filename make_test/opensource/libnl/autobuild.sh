#!/bin/bash -e

source ../common.sh

build_libnl()
{
	setup https://www.infradead.org/~tgr/libnl/files/libnl-3.2.25.tar.gz

	pushd libnl-3.2.25

	echo "[libnl] configure"
	./configure CC=${CROSS_COMPILE}gcc LD=${CROSS_COMPILE}ld STRIP=${CROSS_COMPILE}strip \
		--host=$HOST --disable-debug --disable-pthreads &> configure.log

	echo "[libnl] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/libnl &> install.log

	echo "[libnl] done"
	popd
}

clean_libnl()
{
	rm -rf libnl-3.2.25
}

rootfs_install_libnl()
{
	echo "[libnl] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -a $INSTALL_DIR/libnl/usr/local/lib/libnl-3.so.* $ROOTFS/lib
	cp -a $INSTALL_DIR/libnl/usr/local/lib/libnl-genl-3.so.* $ROOTFS/lib
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_libnl
elif type -t $1_libnl 2> /dev/null >&2 ; then
	$1_libnl
fi
