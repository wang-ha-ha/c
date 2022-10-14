#!/bin/bash -e

source ../../make/config.sh

# Instructions
#
# 1. SDK needs to be built once first
#
# 2. Build busybox with `<chip>_defconfig`
#      ./autobuild.sh
#      ./autobuild.sh install
#
# 3. Menuconfig and make
#      ./autobuild.sh menuconfig
#      ./autobuild.sh
#
# 4. Preserve updated .config at `busybox-1.27.2/OUT/.config`

# Configuration for this busybox
KBUILD_OUTPUT=OUT
DEFCONFIG=${CONFIG_ROOTFS_DIR#rootfs.}_defconfig

# compiler
export CFLAGS=-mthumb

# setup source - https://busybox.net/downloads/busybox-1.27.2.tar.bz2
export KBUILD_OUTPUT
busybox=busybox-1.27.2
if [ ! -e $busybox/$KBUILD_OUTPUT ]; then
	tar xf $busybox.tar.bz2
	patch -p1 < busybox-nc.patch
	mkdir $busybox/$KBUILD_OUTPUT
	cp $DEFCONFIG $busybox/configs
	make -C $busybox $DEFCONFIG
fi

# build
make -C $busybox $*
