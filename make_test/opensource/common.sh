set -e

INSTALL_DIR=$ZRT_ENV_USERLAND_DIR/70mai/opensource/target/
CROSS_PREFIX=mips-linux-gnu-
HOST=${CROSS_PREFIX%-}

[ ! -n "$ROOTFS" ] \
	&& ROOTFS=$ZRT_ENV_OS_DIR/rootfs/7.2.0/camera/rootfs_camera

[ ! -n "$USERFS" ] \
	&& USERFS=$ZRT_ENV_OS_DIR/rootfs/7.2.0/camera/system

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

setup()
{
	SRC_URL=$1
	SRC_TGZ=$(basename $SRC_URL)
	SRC_DIR=${SRC_TGZ%.tar.*}
	if [ ! -e $SRC_DIR ]; then
		[ ! -e $SRC_TGZ ] && wget $SRC_URL
		tar xf $SRC_TGZ
	fi
}
