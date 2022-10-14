#!/bin/bash -e

source ../common.sh

build_wpa_supplicant()
{
	cd ../libnl
	[ ! -d libnl-3.2.25 ] && ./autobuild.sh
	cd -
	rm -rf wpa_supplicant-2.5
	setup http://w1.fi/releases/wpa_supplicant-2.5.tar.gz
	patch -d wpa_supplicant-2.5 -i ../wpa_supplicant-2.5.patch -p1 &> /dev/null
	patch -d wpa_supplicant-2.5 -i ../wpa_supplicant-2.5_01.patch -p1 &> /dev/null
	pushd wpa_supplicant-2.5/wpa_supplicant
	echo "[wpa_supplicant] make"
	make &> make.log
	DESTDIR=$INSTALL_DIR/wpa_supplicant
	mkdir -p $DESTDIR
	cp wpa_supplicant $DESTDIR
	chmod +x wifi_wps
	cp wifi_wps $DESTDIR
	cp wpa_cli $DESTDIR
	echo "[wpa_supplicant] done"
	popd
}

clean_wpa_supplicant()
{
	rm -rf wpa_supplicant-2.5
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_wpa_supplicant
elif [ "$1" = "clean" ]; then
	clean_wpa_supplicant
fi
