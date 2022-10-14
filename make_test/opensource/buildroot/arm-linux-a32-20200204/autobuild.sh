#!/bin/bash -e

TOP=$(dirname $(readlink -f $0))
BUILDROOT=buildroot-2020.02.4
unset PERL_MM_OPT

pushd () {
	command pushd "$@" > /dev/null
}

popd () {
	command popd "$@" > /dev/null
}


build_buildroot()
{
	tar xf $BUILDROOT.tar.bz2
	pushd $BUILDROOT
	cp $TOP/config .config
	make
	popd
}

install_buildroot()
{
	pushd $BUILDROOT/output
	tar --transform='s/host/arm-linux-a32-20200204/' -czf $TOP/arm-linux-a32-20200204.tar.gz host/*
	cp images/rootfs.tar $TOP/rootfs.tar
	popd
}

build_buildroot
install_buildroot
