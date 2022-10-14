#!/bin/bash -e

source ../common.sh

build_curl()
{
	setup https://curl.haxx.se/download/curl-7.55.1.tar.gz

	pushd curl-7.55.1

	echo "[curl] configure"
	CPPFLAGS="-I$INSTALL_DIR/openssl/include" LDFLAGS="-L$INSTALL_DIR/openssl/lib" LIBS="-ldl -lssl -lcrypto" ./configure --host=$HOST  --enable-cross-compile --with-ssl --enable-shared --disable-dict --disable-ftp --disable-imap --disable-ldap --disable-ldaps --disable-pop3 --disable-proxy --disable-rtsp --disable-smtp --disable-telnet --disable-tftp --disable-zlib --without-ca-bundle --without-gnutls --without-libidn --without-librtmp --without-libssh2 --without-nss --without-zlib  &> configure.log

	# ./configure --enable-cross-compile --enable-shared --with-ssl=$INSTALL_DIR/openssl/lib --without-zlib --host=$HOST &> configure.log

	echo "[curl] make"
	make &> make.log
	make install-strip DESTDIR=$INSTALL_DIR/curl &> install.log

	echo "[curl] done"
	popd
}

clean_curl()
{
	rm -rf curl-7.55.1
}

rootfs_install_curl()
{
	echo "[curl] rootfs_install"
	mkdir -p $ROOTFS/lib
	cp -a $INSTALL_DIR/curl/usr/local/lib/libcurl.so* $ROOTFS/lib
}

install_curl()
{
	rootfs_install_curl
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_curl
elif type -t $1_curl 2> /dev/null >&2 ; then
	$1_curl
fi
