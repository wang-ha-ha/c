#!/bin/bash -e

source ../common.sh

CUR=$(dirname $(readlink -f $0))

build_ltp()
{
	setup https://github.com/linux-test-project/ltp/releases/download/20220121/ltp-full-20220121.tar.bz2

	rm -rf target
	mkdir -p target/log

	pushd ltp-full-20220121

	echo "[ltp] configure"
	./configure	CC=${CROSS_COMPILE}gcc --host=$HOST --prefix=$CUR/target/ltp &>../target/log/configure.log

	# Fix the "error: unknown type name ‘__s32’" in fanotify.h
	patch -p1 < ../ltp-config.patch &>/dev/null

	echo "[ltp] make"
	MAKE_JOBS=$(getconf _NPROCESSORS_ONLN)
	make -j$MAKE_JOBS &>../target/log/make.log

	make install &> ../target/log/install.log

	for item in `find $CUR/target/ltp/testcases/bin/ -type f `; do 
		if file $item | grep -q 'ELF 32-bit LSB executable'; then 
			${CROSS_COMPILE}strip  $item
		fi; 
	done

	echo "[ltp] build done"
	popd

	echo "Copy dir \"$CUR/target/ltp\" to SD Card to use it"
}

clean_ltp()
{
	echo "[ltp] clean"

	rm -rf ltp-full-20220121
	rm -rf target

	echo "[ltp] clean done"
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_ltp
elif type -t $1_ltp 2> /dev/null >&2 ; then
	$1_ltp
fi
