#!/bin/bash -e

source ../common.sh

build_tcpdump()
{
	rm -rf tcpdump-4.9.3
	setup https://www.tcpdump.org/release/tcpdump-4.9.3.tar.gz
	pushd tcpdump-4.9.3
	echo "[tcpdump] configure"
	CC=${CROSS_COMPILE}gcc ./configure --host=arm-linux --prefix=`pwd`/out &> make.log
	echo "[tcpdump] make"
	make &> make.log
	echo "[tcpdump] make install"
	make install &> make.log
	DESTDIR=$INSTALL_DIR/tcpdump
	mkdir -p $DESTDIR
	${CROSS_COMPILE}strip out/sbin/tcpdump
	cp out/sbin/tcpdump $DESTDIR -R
	echo "[tcpdump] done"
	popd
}

clean_tcpdump()
{
	rm -rf tcpdump-4.9.3
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_tcpdump
elif [ "$1" = "clean" ]; then
	clean_tcpdump
fi
