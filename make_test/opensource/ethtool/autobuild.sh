#!/bin/bash -e

source ../common.sh

build_ethtool()
{
	setup  https://mirrors.edge.kernel.org/pub/software/network/ethtool/ethtool-4.6.tar.gz

	pushd ethtool-4.6
	
	echo "[ethtool] configure"
	./configure --host=$HOST CC=$CC LDFLAGS=-static &> configure.log

	echo "[ethtool] make"
	make &> make.log
	${CROSS_COMPILE}strip ethtool
	
	mkdir -p $INSTALL_DIR/ethtool
	cp ethtool $INSTALL_DIR/ethtool

	echo "[ethtool] done"
	popd
}

rootfs_install_ethtool()
{
	echo "[ethtool] rootfs_install"
	mkdir -p $ROOTFS/usr/sbin
	cp -a $INSTALL_DIR/ethtool/ethtool $ROOTFS/usr/sbin
}

clean_ethtool()
{
	rm -rf ethtool-4.6
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_ethtool
elif type -t $1_ethtool 2> /dev/null >&2 ; then
        $1_ethtool
fi
