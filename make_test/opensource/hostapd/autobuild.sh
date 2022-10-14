#!/bin/bash -e

source ../common.sh

build_hostapd()
{
	cd ../libnl
	[ ! -d libnl-3.2.25 ] && ./autobuild.sh
	cd -
	rm -rf hostapd-2.9
	setup https://w1.fi/releases/hostapd-2.9.tar.gz
	patch -d hostapd-2.9 -i ../hostapd-2.9.patch -p1 &> /dev/null
	patch -d hostapd-2.9 -i ../hostapd-2.9_forceht40.patch -p1 &> /dev/null
	pushd hostapd-2.9/hostapd
	echo "[hostapd] make"
	make &> make.log
	DESTDIR=$INSTALL_DIR/hostapd
	mkdir -p $DESTDIR
	cp hostapd $DESTDIR
	cp hostapd_cli $DESTDIR
	echo "[hostapd] done"
	popd
}

clean_hostapd()
{
	rm -rf hostapd-2.9
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_hostapd
elif [ "$1" = "clean" ]; then
	clean_hostapd
fi
