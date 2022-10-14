#!/bin/bash -e

source ../common.sh

build_iperf2() {
	setup https://astuteinternet.dl.sourceforge.net/project/iperf/iperf-2.0.5.tar.gz

	pushd iperf-2.0.5
	export ac_cv_func_malloc_0_nonnull=yes
	export CFLAGS='-static'

	echo "[iperf2] configure"
	./configure --host=${CROSS_PREFIX%-} --prefix=/ &> configure.log

	echo "[iperf2] patch"
	patch -p1 < ../iperf_20210322.patch

	echo "[iperf2] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/iperf2 &> install.log

	echo "[iperf2] done"
}

clean_iperf2()
{
	rm -rf iperf-2.0.5
}

rootfs_install_iperf2()
{
	echo "[iperf2] rootfs_install"
	mkdir -p $ROOTFS/bin
	cp -a $INSTALL_DIR/iperf2/bin/iperf $ROOTFS/bin
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_iperf2
elif type -t $1_iperf2 2> /dev/null >&2 ; then
        $1_iperf2
fi
