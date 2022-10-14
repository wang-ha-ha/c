#!/bin/bash -e

source ../common.sh

build_mosquitto()
{
	setup https://mosquitto.org/files/source/mosquitto-2.0.14.tar.gz
	rm -rf mosquitto-2.0.14
	tar vxf mosquitto-2.0.14.tar.gz &> /dev/null
	pushd mosquitto-2.0.14
	patch -p1 -i ../mosquitto_01.patch
	echo "[mosquitto] make"
	CROSS_COMPILE=${CONFIG_CROSS_COMPILE} CC=gcc make &> make.log
	DESTDIR=$INSTALL_DIR/mosquitto
	rm $DESTDIR -rf
	mkdir -p $DESTDIR
	cp client/mosquitto_pub $DESTDIR	
	cp client/mosquitto_sub $DESTDIR	
	cp lib/libmosquitto.so.1 $DESTDIR/libmosquitto.so.1
	arm-linux-strip $DESTDIR/*
	ln -s libmosquitto.so.1 $DESTDIR/libmosquitto.so
	cp include $DESTDIR -rf
	echo "[mosquitto] done"
	popd
}

clean_mosquitto()
{
	rm -rf mosquitto-2.0.14
}


rootfs_install_mosquitto()
{
	echo "[mosquitto] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp $INSTALL_DIR/mosquitto/libmosquitto.so* $ROOTFS/lib -R
#	cp $INSTALL_DIR/mosquitto/mosquitto_pub $ROOTFS/usr/bin	
#	cp $INSTALL_DIR/mosquitto/mosquitto_sub $ROOTFS/usr/bin
}


#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_mosquitto
elif [ "$1" = "clean" ]; then
	clean_mosquitto
elif type -t $1_mosquitto 2> /dev/null >&2 ; then
	$1_mosquitto
fi
