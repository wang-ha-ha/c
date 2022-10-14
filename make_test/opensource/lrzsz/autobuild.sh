#!/bin/bash -e

source ../common.sh

export CFLAGS='-mthumb'
export CC=${CROSS_COMPILE}gcc

build_lrzsz()
{
	setup https://www.ohse.de/uwe/releases/lrzsz-0.12.20.tar.gz

	pushd lrzsz-0.12.20
    echo "[lrzsz] patch"
    patch -p0 < ../lrzsz-0.12.20.patch

	echo "[lrzsz] configure"
	./configure --disable-timesync --disable-mkdir --disable-nls &> configure.log
	sed -i 's/^LIBS = -lnsl/#LIBS = -lnsl/' src/Makefile

	echo "[lrzsz] make"
	make &> make.log
	${CROSS_COMPILE}strip src/{lrz,lsz}
	mkdir -p $INSTALL_DIR/lrzsz
	cp src/{lrz,lsz} $INSTALL_DIR/lrzsz

	echo "[lrzsz] done"
	popd
}

clean_lrzsz()
{
	rm -rf lrzsz-0.12.20
}

rootfs_install_lrzsz()
{
	echo "[lrzsz] rootfs_install"
	mkdir -p $ROOTFS/usr/bin
	cp -a $INSTALL_DIR/lrzsz/lrz $ROOTFS/usr/bin
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_lrzsz
elif type -t $1_lrzsz 2> /dev/null >&2 ; then
        $1_lrzsz
fi
