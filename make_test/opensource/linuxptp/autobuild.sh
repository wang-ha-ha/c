#!/bin/bash -e

source ../common.sh

build_linuxptp()
{
	SRC_URL=https://nchc.dl.sourceforge.net/project/linuxptp/v3.1/linuxptp-3.1.1.tgz
	
	SRC_TGZ=$(basename $SRC_URL)
	SRC_DIR=${SRC_TGZ%.tgz}
	
	if [ ! -e $SRC_DIR ]; then
		[ ! -e $SRC_TGZ ] && wget $SRC_URL
		tar xf $SRC_TGZ
	fi

	pushd linuxptp-3.1.1
	
	echo "[linuxptp] make"
	make &> make.log

	echo "[linuxptp] make install"
	make install prefix=$INSTALL_DIR/linuxptp &> install.log
	${CROSS_COMPILE}strip $INSTALL_DIR/linuxptp/sbin/{hwstamp_ctl,nsm,phc2sys,phc_ctl,pmc,ptp4l,timemaster,ts2phc}

	echo "[linuxptp] done"
	popd
}

rootfs_install_linuxptp()
{
	echo "[linuxptp] rootfs_install"
	mkdir -p $ROOTFS/bin
	cp -a $INSTALL_DIR/linuxptp/sbin/ptp4l $ROOTFS/bin
}

clean_linuxptp()
{
	rm -rf linuxptp-3.1.1
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_linuxptp
elif type -t $1_linuxptp 2> /dev/null >&2 ; then
        $1_linuxptp
fi
