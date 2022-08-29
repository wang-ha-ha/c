#!/bin/bash

source ./common.sh

help()
{
    cat << EOF
autobuild.sh - top script to build opensource
Usage:
    autobuild.sh [make]         - Build opensource packages
    autobuild.sh clean          - Clean build directories
    autobuild.sh rootfs_install - Install to rootfs
    autobuild.sh userfs_install - Install to userfs
    help                        - print this message
EOF
}

# Packages with build support (tested)
PACKAGES_BUILD="RCF cJSON yaml-cpp"

# Packages with filesystem install support
PACKAGES_INSTALL="RCF cJSON yaml-cpp "

# Packages configured for filesystem install
#PACKAGES_INSTALL="${CONFIG_OPENSOURCE_APPS}"

do_build()
{
	for pkg in $PACKAGES_BUILD
	do
		(cd $pkg && ./autobuild.sh)
		echo ""
	done
}

do_clean()
{
	for pkg in $PACKAGES_BUILD
	do
		(cd $pkg && ./autobuild.sh clean)
	done
}

do_rootfs_install()
{
	for pkg in $PACKAGES_INSTALL
	do
		(cd $pkg && ./autobuild.sh rootfs_install)
	done
}

do_userfs_install()
{
	for pkg in $PACKAGES_INSTALL
	do
		(cd $pkg && ./autobuild.sh userfs_install)
	done
}

do_install()
{
	for pkg in $PACKAGES_INSTALL
	do
		(cd $pkg && ./autobuild.sh install)
	done
}

#
# main
#
if [ $# -eq 0 ]; then
	cmd="build"
else
	cmd=$1
fi

case "$cmd" in
	build|rootfs_install|userfs_install|install|clean)
		echo "[opensource] start $cmd"
		eval do_$cmd
		echo "[opensource] done"
		;;
	*)
		help
		;;
esac

exit 0
