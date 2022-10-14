#!/bin/bash -e

source ../common.sh

build_dnsmasq()
{
	rm -rf dnsmasq-2.86
	setup http://w1.fi/releases/dnsmasq-2.86.tar.xz
	pushd dnsmasq-2.86
	echo "[dnsmasq] make"
	CC=${CROSS_COMPILE}gcc make COPTS=-DNO_INOTIFY &> make.log
	DESTDIR=$INSTALL_DIR/dnsmasq
	mkdir -p $DESTDIR
	${CROSS_COMPILE}strip src/dnsmasq	
	cp src/dnsmasq $DESTDIR
	cp ../dnsmasq.conf $DESTDIR
	echo "[dnsmasq] done"
	popd
}

clean_dnsmasq()
{
	rm -rf dnsmasq-2.86
}

rootfs_install_dnsmasq()
{
	echo "[dnsmasq] rootfs_install"
	cp -a $INSTALL_DIR/dnsmasq/dnsmasq $ROOTFS/usr/bin/
	cp -a $INSTALL_DIR/dnsmasq/dnsmasq.conf $ROOTFS/etc/
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_dnsmasq
elif [ "$1" = "clean" ]; then
	clean_dnsmasq
elif type -t $1_dnsmasq 2> /dev/null >&2 ; then
	$1_dnsmasq
fi
