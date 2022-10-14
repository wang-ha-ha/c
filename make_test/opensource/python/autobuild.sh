#!/bin/bash -e

source ../common.sh

setup_python()
{
	echo "[python] setup"

	tar xf ../buildroot/buildroot-2021.02/buildroot-2021.02.tar.bz2
}

build_python()
{
	echo "[python] make"

	rm -fr sysroot
	rm -fr target

	pushd buildroot-2021.02
	cp ../config .config
	unset PERL_MM_OPT
	make &> make.log
	popd
}

install_python()
{
	echo "[python] install to $INSTALL_DIR/python"

	cp -a buildroot-2021.02/output/host/arm-buildroot-linux-uclibcgnueabihf/sysroot .
	echo "Install dir " $INSTALL_DIR
	mkdir -p $INSTALL_DIR/python
	tar czf $INSTALL_DIR/python/sysroot.tar.gz sysroot

	mkdir -p target
	tar xf buildroot-2021.02/output/images/rootfs.tar -C target

	rm -f $INSTALL_DIR/python/opt_py.sqfs
	mksquashfs target $INSTALL_DIR/python/opt_py.sqfs
}

#
# main
#
setup_python
build_python
install_python

echo "[python] done"
