#!/bin/bash -e

source ../common.sh

build_tinyalsa()
{
	if [ ! -e "tinyalsa-2.0.0" ]; then
		echo "[tinyalsa-2.0.0] untar"
		tar xzvf tinyalsa-2.0.0.tar.gz
		patch -p0 < tinyalsa.diff
	fi	

	pushd tinyalsa-2.0.0
	make
	make install DESTDIR=$INSTALL_DIR/tinyalsa #&> install.log

	popd
	
	cp -rf $INSTALL_DIR/tinyalsa/usr/local/bin/* ../../filesystem/rootfs.oa8600/bin/
	cp -rf $INSTALL_DIR/tinyalsa/usr/local/lib/* ../../filesystem/rootfs.oa8600/lib/
	echo "[tinyalsa-2.0.0] done"
	rm -rf tinyalsa-2.0.0
}


#
# main
#
build_tinyalsa
