#!/bin/bash -e

source ../../common.sh

build_spawn-fcgi()
{
	rm -rf spawn-fcgi-1.6.4
	setup http://nginx.org/download/spawn-fcgi-1.6.4.tar.gz
	patch -d spawn-fcgi-1.6.4 -i ../spawn-fcgi-1.6.4.patch -p1 &> /dev/null
	pushd spawn-fcgi-1.6.4
	echo "[spawn-fcgi] configure"
	CC=${CONFIG_CROSS_COMPILE}gcc ./configure --host=${CROSS_PREFIX%-} &> make.log
	echo "[spawn-fcgi] make"
	make &> make.log
	DESTDIR=$INSTALL_DIR/nginx
	${CONFIG_CROSS_COMPILE}strip src/spawn-fcgi
	cp src/spawn-fcgi $DESTDIR/usr/local/nginx/sbin
	echo "[spawn-fcgi] done"
	popd
}

clean_spawn-fcgi()
{
	rm -rf spawn-fcgi-1.6.4
}


build_fcgi()
{
	rm -rf fcgi-2.4.1-SNAP-0910052249
	setup http://nginx.org/download/fcgi-2.4.1.tar.gz
	patch -d fcgi-2.4.1-SNAP-0910052249 -i ../fcgi-2.4.1.patch -p1 &> /dev/null
	pushd fcgi-2.4.1-SNAP-0910052249
	echo "[fcgi] configure"
	CC=${CONFIG_CROSS_COMPILE}gcc ./configure --host=${CROSS_PREFIX%-} &> make.log
	echo "[fcgi] make"
	make &> make.log
	rm libfcgi/.libs/*.so* -f
	echo "[fcgi] done"	
	popd
}

clean_fcgi()
{
	rm -rf fcgi-2.4.1-SNAP-0910052249
}


build_fcgiwrap()
{
	rm -rf fcgiwrap
	setup http://nginx.org/download/fcgiwrap.tar.gz
	patch -d fcgiwrap -i ../fcgiwrap.patch -p1 &> /dev/null
	pushd fcgiwrap
	echo "[fcgiwrap] configure"
	chmod +x configure
	LDFLAGS=-L../fcgi-2.4.1-SNAP-0910052249/libfcgi/.libs/ CPPFLAGS=-I../fcgi-2.4.1-SNAP-0910052249/include ./configure --host=${CROSS_PREFIX%-} &> make.log
	echo "[fcgiwrap] make"
	${CONFIG_CROSS_COMPILE}gcc -I../fcgi-2.4.1-SNAP-0910052249/include  fcgiwrap.c -L../fcgi-2.4.1-SNAP-0910052249/libfcgi/.libs/ -lfcgi  -o fcgiwrap  &> make.log
	${CONFIG_CROSS_COMPILE}strip fcgiwrap
	DESTDIR=$INSTALL_DIR/nginx
	cp fcgiwrap $DESTDIR/usr/local/nginx/sbin	
	echo "[fcgiwrap] done"
	popd	
}

clean_fcgiwrap()
{
	rm -rf fcgiwrap
}

build_ncgic()
{
	pushd ncgic
	echo "[ncgic] make"
	CC=${CONFIG_CROSS_COMPILE}gcc make &> make.log
	${CONFIG_CROSS_COMPILE}strip *.cgi
	DESTDIR=$INSTALL_DIR/nginx
	mv *.cgi $DESTDIR/usr/local/nginx/html/cgi-bin/
	echo "[ncgic] done"
	popd	
}

clean_ncgic()
{
	pushd ncgic
	rm *.cgi -f
	popd
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_spawn-fcgi
	build_fcgi
	build_fcgiwrap
	build_ncgic
elif [ "$1" = "clean" ]; then
	clean_spawn-fcgi
	clean_fcgi
	clean_fcgiwrap
	clean_ncgic
elif [ "$1" = "ncgic" ]; then
	build_ncgic
fi
