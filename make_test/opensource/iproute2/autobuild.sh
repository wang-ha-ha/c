#!/bin/bash -e

source ../common.sh

build_iproute2()
{
	tar xf ${IPROUTE}.tar.[xg]z

	pushd ${IPROUTE}

	echo "[iproute2] patch for GCC10"
	sed -i 's/struct xfrm_filter filter;/extern struct xfrm_filter filter;/' ip/ipxfrm.c

	echo "[iproute2] make"
	make CC=${CROSS_COMPILE}gcc SUBDIRS="lib ip"&> make.log
#	make&> make.log

	mkdir -p $INSTALL_DIR/iproute2
	EXES="ip/ip"
	for exe in $EXES; do
		${CROSS_COMPILE}strip $exe
		cp $exe $INSTALL_DIR/iproute2
	done
	popd

	echo "[iproute2] done"
}
clean_iproute2()
{
	rm -rf ${IPROUTE}
}

rootfs_install_iproute2()
{
	echo "[iproute2] rootfs_install"
	mkdir -p $ROOTFS/bin
	cp -a $INSTALL_DIR/iproute2/ip $ROOTFS/bin
}

#
# main
#
IPROUTE=iproute2-3.19.0
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_iproute2
elif type -t $1_iproute2 2> /dev/null >&2 ; then
	$1_iproute2
fi
