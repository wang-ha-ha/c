#!/bin/bash -e

source ../common.sh

build_openssl()
{
	export CROSS=$(basename $CROSS_COMPILE)

	setup https://ftp.openssl.org/source/old/1.1.1/openssl-1.1.1c.tar.gz

	pushd openssl-1.1.1c
	echo "[openssl] configure"
	./Configure -fPIC --prefix=/ threads shared linux-armv4 &> configure.log 
	echo "[openssl] make"
	make &> make.log
	make install_sw INSTALLTOP=$INSTALL_DIR/openssl &> install.log

	echo "[openssl] done"
	popd
}

clean_openssl()
{
	rm -rf openssl-1.1.1c
}

rootfs_install_openssl()
{
    echo "[openssl] rootfs_install"
    # cp -rf ${INSTALL_DIR}openssl/lib/libcrypto.so* $ROOTFS/lib
	cp -rf ${INSTALL_DIR}openssl/lib/libssl.so* $ROOTFS/lib
}

install_openssl()
{
    rootfs_install_openssl
}


#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_openssl
elif type -t $1_openssl 2> /dev/null >&2 ; then
	$1_openssl
fi
