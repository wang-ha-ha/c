#!/bin/bash -e

# Download dosfstools-84a8d1c0b71943d4a20b50ceab18b9a836cc471f.zip
#    from https://github.com/dosfstools/dosfstools/tree/84a8d1c0b71943d4a20b50ceab18b9a836cc471f
#
# Note
# - Commit 84a8d1c0b71943d4a20b50ceab18b9a836cc471f is required for `--without-iconv`.
# - Requires `/opt/arm-ov-linux-20190531` that supports wchar,
#   and build it as statically linked.
# - `dosfstools.patch` is to unset salvage_files that requires a large amount of memory.
#
# Usage Example
#   if ! ./fsck.fat -b /dev/mmcblk0p1; then
#           ./fsck.fat -a /dev/mmcblk0p1
#   fi
#

SRC=dosfstools-84a8d1c0b71943d4a20b50ceab18b9a836cc471f

build_dosfstools()
{
	[ ! -e $SRC ] &&  unzip $SRC.zip
	cd $SRC

	./autogen.sh 
	# Don't set these before autogen to use autoreconf in host system instead of toolchain's.
	export PATH=/opt/arm-ov-linux-20190531/bin:$PATH
	export LD_LIBRARY_PATH=/opt/arm-ov-linux-20190531/lib
	export CFLAGS='-static -mthumb'

	echo "Configuring dosfstools"
	./configure --host=arm-linux --without-iconv &> 0.configure.log
	patch -p1 < ../dosfstools.patch

	echo "Building dosfstools"
	make &> 1.build.log
	arm-linux-strip src/fsck.fat -o ../fsck.fat
	echo "Finish building dosfstools"
}

build_dosfstools
