#!/bin/bash -e
#
# Script to build mtd-utils
#

source ../common.sh

DEPENDS="lzo zlib e2fsprogs"

build_lzo()
{
	setup https://www.oberhumer.com/opensource/lzo/download/lzo-2.10.tar.gz

	mod_name=lzo-2.10
	pushd $mod_name
	echo "[$mod_name] configure"
	./configure prefix=/ --build=i686-pc-linux --host=$HOST &> configure.log
	echo "[$mod_name] make"
	make &> make.log
	echo "[$mod_name] make install"
	make install-strip DESTDIR=$INSTALL_DIR/lzo &> install.log
	echo "[$mod_name] done"
	popd
}

rootfs_install_lzo()
{
	true
}

build_e2fsprogs()
{
	setup https://git.kernel.org/pub/scm/fs/ext2/e2fsprogs.git/snapshot/e2fsprogs-1.42.9.tar.gz

	mod_name=e2fsprogs-1.42.9
	pushd $mod_name
	echo "[$mod_name] configure"
	./configure --prefix=/  --build=i686-pc-linux --host=$HOST --enable-symlink-install &> configure.log
	echo "[$mod_name] make"
	patch -p2 < ../e2fsprogs_v1.42.9_devname.patch
	make -i &> make.log # ignore errors
	echo "[$mod_name] make install"
	make install DESTDIR=$INSTALL_DIR/e2fsprogs &> install.log
	make install-libs DESTDIR=$INSTALL_DIR/e2fsprogs &> install-libs.log
	echo "[$mod_name] done"
	popd
}

rootfs_install_e2fsprogs()
{
	true
}

build_mtd-utils()
{
	build_lzo
	build_e2fsprogs

	setup https://infraroot.at/pub/mtd/mtd-utils-2.0.2.tar.bz2

	mod_name=mtd-utils-2.0.2
	pushd $mod_name
	echo "[$mod_name] configure"
	ZLIB_CFLAGS="-I$INSTALL_DIR/zlib/include" ZLIB_LIBS="-L$INSTALL_DIR/zlib/lib -lz" \
	UUID_CFLAGS="-I$INSTALL_DIR/e2fsprogs/include/uuid" UUID_LIBS="-L$INSTALL_DIR/e2fsprogs/lib -luuid" \
	LZO_CFLAGS="-I$INSTALL_DIR/lzo/include" LZO_LIBS="-L$INSTALL_DIR/lzo/lib" \
	CPPFLAGS="-I$INSTALL_DIR/lzo/include -I$INSTALL_DIR/zlib/include" LIBS="-L$INSTALL_DIR/lzo/lib -L$INSTALL_DIR/zlib/lib" \
	./configure --prefix=/ --build=i686-pc-linux --host=$HOST &> configure.log
	echo "[$mod_name] make"
	make &> make.log
	echo "[$mod_name] make install"
	make install-strip DESTDIR=$INSTALL_DIR/mtd-utils &> install.log
	echo "[$mod_name] done"
	popd
}

rootfs_install_mtd-utils()
{
	rootfs_install_lzo
	rootfs_install_e2fsprogs

	mod_name=mtd-utils-2.0.2
        echo "[$mod_name] rootfs_install"
        mkdir -p $ROOTFS/sbin
	## needed tools
        cp -a $INSTALL_DIR/mtd-utils/sbin/{mtdinfo,ubiattach,ubidetach,ubiformat,ubimkvol,ubinfo,ubinize} $ROOTFS/sbin
	## tools used for performance test
	cp -a $INSTALL_DIR/mtd-utils/sbin/{ubirmvol,ubirsvol,ubiupdatevol} $ROOTFS/sbin
	## tools for debugging to read/write mtd partition. Make sure to read/write in block size!!!
	cp -a $INSTALL_DIR/mtd-utils/sbin/mtd_debug $ROOTFS/sbin
	## tools may not need, can remove from ROOTFS?
	cp -a $INSTALL_DIR/mtd-utils/sbin/mkfs.ubifs  $ROOTFS/sbin

}

rootfs_uninstall_mtd-utils()
{
	mod_name=mtd-utils-2.0.2
        echo "[$mod_name] rootfs_uninstall"
	rm -rf $ROOTFS/sbin/{mtdinfo,ubiattach,ubidetach,ubiformat,ubimkvol,ubinfo,ubinize}
	rm -rf $ROOTFS/sbin/{ubirmvol,ubirsvol,ubiupdatevol}
	rm -rf $ROOTFS/sbin/mtd_debug
	rm -rf $ROOTFS/sbin/mkfs.ubifs
}

clean_mtd-utils()
{
	rm -rf lzo-2.10
	rm -rf e2fsprogs-1.42.9
	rm -rf mtd-utils-2.0.2
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_mtd-utils
elif type -t $1_mtd-utils 2> /dev/null >&2 ; then
	$1_mtd-utils
fi
