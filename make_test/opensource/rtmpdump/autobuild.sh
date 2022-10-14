#!/bin/bash -e

source ../common.sh
export CONFIG_CHIP
export CONFIG_OUT
export CONFIG_PRJ
export CONFIG_TOP
build_rtmpdump()
{
	[ ! -d rtmpdump ] && git clone git://git.ffmpeg.org/rtmpdump
	pushd rtmpdump
	rm .git -rf
	#patch -d rtmpdump -i ../rtmpdump.patch -p1 &> /dev/null
	echo "[rtmpdump] make"
	CROSS_COMPILE=${CONFIG_CROSS_COMPILE} CC=gcc make CRYPTO= &> make.log
	DESTDIR=$INSTALL_DIR/rtmpdump
	rm $DESTDIR -rf
	mkdir -p $DESTDIR
	cp rtmpdump $DESTDIR
	cp librtmp/librtmp.so* $DESTDIR -R
	${CONFIG_CROSS_COMPILE}strip $DESTDIR/* 
	mkdir -p $DESTDIR/include
	cp librtmp/*.h $DESTDIR/include
	echo "[rtmpdump] done"
	popd
}

clean_rtmpdump()
{
	pushd rtmpdump
	make clean
	popd
}

rootfs_install_rtmpdump()
{
	echo "[librtmp] rootfs_install"
	mkdir -p $ROOTFS/lib
    cp $INSTALL_DIR/rtmpdump/librtmp.so* $ROOTFS/lib -R
}


#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_rtmpdump
elif [ "$1" = "clean" ]; then
	clean_rtmpdump
elif type -t $1_rtmpdump 2> /dev/null >&2 ; then
	$1_rtmpdump
fi
