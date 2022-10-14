#!/bin/bash -e

source ../common.sh

build_iptables()
{
	rm -rf iptables-1.8.2
	setup https://www.netfilter.org/projects/iptables/files/iptables-1.8.2.tar.bz2
	pushd iptables-1.8.2
	echo "[iptables] configure"
	CC=${CROSS_COMPILE}gcc ./configure --host=arm-linux --disable-nftables --prefix=`pwd`/out --disable-ipv6 --enable-static --disable-shared &> make.log
	echo "[iptables] make"
	make &> make.log
	echo "[iptables] make install"
	make install &> make.log
	DESTDIR=$INSTALL_DIR/iptables
	mkdir -p $DESTDIR
	${CROSS_COMPILE}strip out/sbin/xtables-legacy-multi	
	cp out/sbin/{iptables,xtables-legacy-multi} $DESTDIR -R
	echo "[iptables] done"
	popd
}

clean_iptables()
{
	rm -rf iptables-1.8.2
}


rootfs_install_iptables()
{
	echo "[iptables] rootfs_install"
	mkdir -p $ROOTFS/lib
    cp -rf $INSTALL_DIR/iptables/* $ROOTFS/usr/bin
    if [ ! -e $ROOTFS/run ]; then
        mkdir $ROOTFS/run
    fi
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_iptables
elif [ "$1" = "clean" ]; then
	clean_iptables
elif type -t $1_iptables 2> /dev/null >&2 ; then
	$1_iptables
fi