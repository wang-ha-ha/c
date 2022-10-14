#!/bin/bash -e

source ../common.sh
INSTALL_DIR=$INSTALL_DIR/exfat

build_libfuse()
{
	setup https://github.com/libfuse/libfuse/releases/download/fuse-2.9.9/fuse-2.9.9.tar.gz

	pushd fuse-2.9.9
	echo "[libfuse] configure"
	./configure --host=$HOST &> configure.log

	echo "[libfuse] make"
	make &> make.log

	# Workaround of `make install-strip` issue "Unable to recognise the format of the input file `...sbin/_inst.15671_'"
	make install DESTDIR=$INSTALL_DIR &> install.log
	${CROSS_COMPILE}strip $INSTALL_DIR/sbin/* $INSTALL_DIR/usr/local/{bin/fusermount,bin/ulockmgr_server,lib/*.so.?.?.?}

	echo "[libfuse] done"
	popd
}

rootfs_install_libfuse()
{
	echo "[libfuse] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -a $INSTALL_DIR/usr/local/lib/{libfuse.so*,libulockmgr.so*} $ROOTFS/lib
}

build_fuse_exfat()
{
	setup https://github.com/relan/exfat/releases/download/v1.3.0/fuse-exfat-1.3.0.tar.gz

	pushd fuse-exfat-1.3.0
	echo "[fuse-exfat] configure"
	./configure --host=$HOST FUSE_CFLAGS="-I$INSTALL_DIR/usr/local/include" FUSE_LIBS="-L$INSTALL_DIR/usr/local/lib/ -lfuse" &> configure.log

	echo "[fuse-exfat] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR &> install.log

	echo "[fuse-exfat] done"
	popd
}

rootfs_install_fuse_exfat()
{
	echo "[fuse-exfat] rootfs_install"
	mkdir -p $ROOTFS/usr/sbin
	cp -a $INSTALL_DIR/usr/local/sbin/{mount.exfat,mount.exfat-fuse} $ROOTFS/usr/sbin
}

build_exfat_utils()
{
	setup https://github.com/relan/exfat/releases/download/v1.3.0/exfat-utils-1.3.0.tar.gz

	pushd exfat-utils-1.3.0
	echo "[exfat-utils] configure"
	./configure --host=$HOST &> configure.log

	echo "[exfat-utils] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR &> install.log

	echo "[exfat-utils] done"
	popd
}

rootfs_install_exfat_utils()
{
	echo "[exfat-utils] rootfs_install"
	mkdir -p $ROOTFS/usr/sbin
	cp -a $INSTALL_DIR/usr/local/sbin/{exfatfsck,fsck.exfat} $ROOTFS/usr/sbin
	cp -a $INSTALL_DIR/usr/local/sbin/{mkexfatfs,mkfs.exfat} $ROOTFS/usr/sbin
}


build_exfat()
{
	#build_libfuse
	#build_fuse_exfat
	build_exfat_utils
}

clean_exfat()
{
	#rm -rf fuse-2.9.9
	#rm -rf fuse-exfat-1.3.0
	rm -rf exfat-utils-1.3.0
}

rootfs_install_exfat()
{
	#rootfs_install_libfuse
	#rootfs_install_fuse_exfat
	rootfs_install_exfat_utils
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_exfat
elif type -t $1_exfat 2> /dev/null >&2 ; then
        $1_exfat
fi
