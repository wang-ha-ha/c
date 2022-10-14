#!/bin/bash -e

source ../common.sh

build_nginx()
{
	rm -rf nginx-1.21.0
	setup http://nginx.org/download/nginx-1.21.0.tar.gz
	patch -d nginx-1.21.0 -i ../nginx-1.21.0.patch -p1 &> /dev/null
	pushd nginx-1.21.0
	echo "[nginx] configure"
	./configure --with-cc=${CONFIG_CROSS_COMPILE}gcc --prefix=/usr/local/nginx --user=root --group=root  --with-http_mp4_module --with-http_flv_module --add-module=../nginx-http-flv-module #&> make.log
	echo "[nginx] make"
	make #&> make.log
	DESTDIR=$INSTALL_DIR/nginx/
	rm $DESTDIR -rf
	mkdir -p $DESTDIR
	cp ../conf/* $DESTDIR -rf
	arm-linux-strip objs/nginx
	cp objs/nginx $DESTDIR/usr/local/nginx/sbin
	echo "[nginx] done"
	popd
}

clean_nginx()
{
	rm -rf nginx-1.21.0
}


rootfs_install_nginx()
{
	echo "[nginx] rootfs_install"
	cp $INSTALL_DIR/nginx/* $ROOTFS/ -rf
}


#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	pushd pcre
	./autobuild.sh
	popd
	build_nginx
	pushd nginx-cgi
	./autobuild.sh
	popd
elif [ "$1" = "clean" ]; then
	pushd pcre
	./autobuild.sh clean
	popd
	clean_nginx
	pushd nginx-cgi
	./autobuild.sh clean
	popd
elif [ "$1" = "html" ]; then
	pushd nginx-cgi
	./autobuild.sh ncgic
	popd
	DESTDIR=$INSTALL_DIR/nginx/
	cp conf/* $DESTDIR -rf
elif type -t $1_nginx 2> /dev/null >&2 ; then
	$1_nginx
fi
