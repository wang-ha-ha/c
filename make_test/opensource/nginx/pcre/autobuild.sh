#!/bin/bash -e

source ../../common.sh

build_pcre()
{
	rm -rf pcre-8.45
	setup http://nginx.org/download/pcre-8.45.tar.bz
	pushd pcre-8.45
	echo "[pcre] configure"
	CC=${CONFIG_CROSS_COMPILE}gcc ./configure --host=${CROSS_PREFIX%-} #&> make.log
	echo "[pcre] make"
	make #&> make.log
	rm .libs/*.so* -f
	echo "[pcre] done"
	popd
}

clean_pcre()
{
	rm -rf pcre-8.45
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_pcre
elif [ "$1" = "clean" ]; then
	clean_pcre
fi
