#!/bin/bash -e

source ../common.sh

OPENSSL=../target/openssl

build_lighttpd()
{
	setup http://download.lighttpd.net/lighttpd/releases-1.4.x/lighttpd-1.4.45.tar.gz

	pushd lighttpd-1.4.45
	echo "[lighttpd] configure"
	./configure CC=${CROSS_COMPILE}gcc AR=${CROSS_COMPILE}ar LD=${CROSS_COMPILE}ld --host=$HOST \
	--prefix=/ --enable-static --enable-shared --disable-ipv6 --disable-lfs \
	--without-zlib --without-bzip2 --without-pcre --with-openssl=$INSTALL_DIR/openssl &> configure.log

	echo "[lighttpd] make"
	make &> make.log
	make install DESTDIR=$INSTALL_DIR/lighttpd &> install.log

	echo "[lighttpd] done"
	popd
}

clean_lighttpd()
{
	rm -rf lighttpd-1.4.45
}


rootfs_install_lighttpd()
{
	echo "[liblighttpd] rootfs_install"


#mkdir -p $ROOTFS/lib/lighttpd

	mkdir -p $ROOTFS/html


#	mkdir -p $ROOTFS/{html,lib}

	cp -a html/* $ROOTFS/html
	

	#cp $INSTALL_DIR/lighttpd/lib/mod_access.so $ROOTFS/html/httpd
	#cp $INSTALL_DIR/lighttpd/lib/mod_dirlisting.so $ROOTFS/html/httpd
	#cp $INSTALL_DIR/lighttpd/lib/mod_indexfile.so $ROOTFS/html/httpd
	#cp $INSTALL_DIR/lighttpd/lib/mod_staticfile.so $ROOTFS/html/httpd
	#cp $INSTALL_DIR/lighttpd/lib/mod_cgi.so $ROOTFS/html/httpd


	cp $INSTALL_DIR/lighttpd/lib/mod_access.so $ROOTFS/lib
	cp $INSTALL_DIR/lighttpd/lib/mod_dirlisting.so $ROOTFS/lib
	cp $INSTALL_DIR/lighttpd/lib/mod_indexfile.so $ROOTFS/lib
	cp $INSTALL_DIR/lighttpd/lib/mod_staticfile.so $ROOTFS/lib
	cp $INSTALL_DIR/lighttpd/lib/mod_cgi.so $ROOTFS/lib


	cp $INSTALL_DIR/lighttpd/sbin/lighttpd $ROOTFS/html/httpd


	#${CROSS_COMPILE}strip             $ROOTFS/html/httpd/*.so
	#${CROSS_COMPILE}strip             $ROOTFS/html/httpd/lighttpd



	echo "Copy openssl"
	chmod u+w $OPENSSL/lib/libcrypto.so.* $OPENSSL/lib/libssl.so.*
	cp -d $OPENSSL/lib/*.so*          $ROOTFS/lib
#cp $OPENSSL/ssl/openssl.cnf       $ROOTFS/ssl
#	mkdir -p $ROOTFS/ssl
#	cp ../openssl/openssl-1.1.1c/apps/openssl.cnf $ROOTFS/ssl/
	${CROSS_COMPILE}strip             $ROOTFS/lib/libcrypto.so.*
	${CROSS_COMPILE}strip             $ROOTFS/lib/libssl.so.*
	echo "Finish"



#	cp -a $INSTALL_DIR/lighttpd/lib/*.so $ROOTFS/lib/lighttpd
#	cp -a $INSTALL_DIR/lighttpd/lib/*.la $ROOTFS/lib/lighttpd

#	mkdir -p $CONFIG_TOP/app/70mai/lib/lighttpd
#	cp -a $INSTALL_DIR/lighttpd/lib/*.so $CONFIG_TOP/app/70mai/lib/lighttpd
#	cp -a $INSTALL_DIR/lighttpd/lib/*.la $CONFIG_TOP/app/70mai/lib/lighttpd 

#	mkdir -p $CONFIG_TOP/app/70mai/include/lighttpd
#	cp lighttpd-1.4.45/src/*.h $CONFIG_TOP/app/70mai/include/lighttpd


#cp -fa $INSTALL_DIR/librtmp/include/* $CONFIG_TOP/app/70mai/include

}
#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_lighttpd
elif type -t $1_lighttpd 2> /dev/null >&2 ; then
        $1_lighttpd
fi
