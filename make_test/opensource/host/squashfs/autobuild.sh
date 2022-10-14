#!/bin/bash -e

BUILD_DIR=$(readlink -f ../../build/)
MKSQUASHFS=squashfs4.3/squashfs-tools/mksquashfs

if [ ! -e $BUILD_DIR/$MKSQUASHFS ]; then
	tar xf ./squashfs4.3.tar.gz -C $BUILD_DIR
	pushd $BUILD_DIR
	make -C squashfs4.3/squashfs-tools
	popd
fi
