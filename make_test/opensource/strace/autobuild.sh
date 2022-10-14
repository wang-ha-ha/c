#!/bin/bash -e

source ../common.sh

build_strace()
{
	setup https://github.com/strace/strace/releases/download/v5.4/strace-5.4.tar.xz

	pushd strace-5.4
	echo "[strace] configure"
	./configure --host=$HOST &> configure.log

	echo "[strace] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/strace &> install.log

	echo "[strace] done"
	popd
}

clean_strace()
{
	rm -rf strace-5.4
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_strace
elif [ "$1" = "clean" ]; then
	clean_strace
fi
