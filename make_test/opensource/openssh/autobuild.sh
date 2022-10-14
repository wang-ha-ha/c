#!/bin/bash -e

source ../common.sh

OPENSSH_SOURCE="openssh-8.5p1"

build_zlib()
{
	export CROSS=$CROSS_COMPILE

	setup http://www.zlib.net/zlib-1.2.11.tar.gz

	pushd zlib-1.2.11

	echo "[zlib] configure"

	CC=${CROSS}gcc AR=${CROSS}ar RANLIB=${CROSS}ranlib ./configure --prefix=/ &> log.config

	echo "[zlib] make"
	make &> make.log
	make install prefix=$INSTALL_DIR/zlib/ &> install.log

	echo "[zlib] done"

	popd
}


build_openssh()
{
	export CROSS=$CROSS_COMPILE

	setup https://fastly.cdn.openbsd.org/pub/OpenBSD/OpenSSH/portable/${OPENSSH_SOURCE}.tar.gz

	pushd ${OPENSSH_SOURCE}

	if [ ! -e $INSTALL_DIR/openssl ]; then
		echo "Error: $INSTALL_DIR/openssl doesn't exit"
		exit 1
	fi

	echo "[openssh] configure"
	./configure --prefix=/opt --host=$HOST --disable-strip --with-libs --with-zlib=$INSTALL_DIR/zlib --with-ssl-dir=$INSTALL_DIR/openssl --disable-etc-default-login CC=${CROSS}gcc AR=${CROSS}ar LD=${CROSS}ld RANLIB=${CROSS}ranlib STRIP=${CROSS}strip &> configure.log

	echo "[openssh] make"
	make &> make.log

	make install DESTDIR=$INSTALL_DIR/openssh &> install.log
	${CROSS}strip $INSTALL_DIR/openssh/opt/bin/* $INSTALL_DIR/openssh/opt/libexec/* $INSTALL_DIR/openssh/opt/sbin/*

	echo "[openssh] done"

	popd
}

clean_openssh()
{
	rm -rf target
	[ -n "${OPENSSH_SOURCE}" ] && rm -rf ${OPENSSH_SOURCE}
	rm -rf zlib-1.2.11
}

TARGET_ETC=../target/etc
TARGET_BIN=../target/bin
TARGET_LIB=../target/lib
TARGET_SBIN=../target/sbin
TARGET_LIBEXEC=../target/libexec

strip_exe()
{
	${CROSS_COMPILE}strip $1 -o $2/$(basename $1)
}

chmod_exe()
{
	chmod 777 $1
}

strip_chmod_exe()
{
	strip_exe $1 $2
	chmod_exe $2/$1
}

cp_exe()
{
	cp -a $1 $2
}

install_openssh()
{
	echo "[openssh] squashfs create"
	rm -rf target
	mkdir target
	(cd target && mkdir bin sbin libexec etc lib)

	pushd ${OPENSSH_SOURCE}
	strip_chmod_exe ssh $TARGET_BIN
	strip_chmod_exe scp $TARGET_BIN
	strip_chmod_exe ssh-keysign $TARGET_LIBEXEC

	strip_exe sshd $TARGET_SBIN

	cp -f ../ssh.key/* $TARGET_ETC

	cp_exe moduli $TARGET_ETC
	cp_exe sshd_config $TARGET_ETC
	sed -i "s/#PermitRootLogin\(.*\)/PermitRootLogin yes/g" $TARGET_ETC/sshd_config

	cp_exe $INSTALL_DIR/zlib/lib/libz.so $TARGET_LIB
	cp_exe $INSTALL_DIR/zlib/lib/libz.so.1 $TARGET_LIB
	cp_exe $INSTALL_DIR/openssl/lib/libcrypto.so $TARGET_LIB
	strip_exe $INSTALL_DIR/zlib/lib/libz.so.1.2.11 $TARGET_LIB
	strip_exe $INSTALL_DIR/openssl/lib/libcrypto.so.1.1 $TARGET_LIB
	popd

	rm -f $INSTALL_DIR/openssh/opt_ssh.sqfs
	mkdir -p $INSTALL_DIR/openssh
	mksquashfs target $INSTALL_DIR/openssh/opt_ssh.sqfs &> ${OPENSSH_SOURCE}/mksquashfs.log
	echo "[openssh] squashfs done"

	rebuild_singlebin=`rootfs_install_openssh`

	if [ "$rebuild_singlebin" = "1" ]; then
		(cd ../../../ && ./autobuild.sh)
	fi
}

rootfs_install_openssh()
{
	local rebuild_singlebin

	echo "[openssh] rootfs_install" >&2

	rebuild_singlebin=0

	if grep -q "root:x" $ROOTFS/etc/passwd; then
		sed -i "s/root:x/root:ORHbffmUxeeGE/g" $ROOTFS/etc/passwd
		rebuild_singlebin=1
	fi

	if ! grep -q Privilege $ROOTFS/etc/passwd; then
		echo "sshd:x:74:74:Privilege-separated SSH:/var/empty/sshd:/sbin/nologin" >> $ROOTFS/etc/passwd
		rebuild_singlebin=1
	fi

	if ! grep -q devpts $ROOTFS/etc/fstab; then
		echo "none /dev/pts devpts  gid=5,mode=620 0 0" >> $ROOTFS/etc/fstab
		rebuild_singlebin=1
	fi

	mkdir -p $ROOTFS/root/.ssh
	cp test-client.key/id_rsa.pub $ROOTFS/root/.ssh/authorized_keys

	# return value
	echo "$rebuild_singlebin"
}

userfs_install_openssh()
{
	[ ! -n "$USERFS" ] && return

	echo "[openssh] userfs_install"

	mkdir -p $USERFS/bin $USERFS/lib/libexec $USERFS/sbin $USERFS/etc/ssh $USERFS/root

	cp -a $INSTALL_DIR/openssl/lib/libcrypto.so* $USERFS/lib

	cp -a $INSTALL_DIR/openssh/opt/bin/{ssh,scp} $USERFS/bin
	cp -a $INSTALL_DIR/openssh//opt/libexec/ssh-keysign $USERFS/lib/libexec
	cp -a $INSTALL_DIR/openssh/opt/sbin/sshd $USERFS/sbin

	cp -a $INSTALL_DIR/openssh/opt/etc/{moduli,sshd_config} $USERFS/etc/ssh 
	sed -i "s;#HostKey /opt/etc\(.*\);HostKey /userfs/etc/ssh\1;g" $USERFS/etc/ssh/sshd_config
	sed -i "s;#PermitRootLogin\(.*\);PermitRootLogin yes;g" $USERFS/etc/ssh/sshd_config
	sed -i "s;Subsystem\(\s*\)sftp\(\s*\)/opt/libexec\(.*\);#Subsystem\1sftp\2/userfs/lib/libexec\3;g" $USERFS/etc/ssh/sshd_config
	cp -f ssh.key/* $USERFS/etc/ssh
	chmod -x $USERFS/etc/ssh/*
}

if [ $# -eq 0 ] || [ "$1" = "build" ]; then
	build_zlib
	build_openssh
	install_openssh
elif [ "$1" = "rootfs_install" ]; then
	rootfs_install_openssh > /dev/null
elif type -t $1_openssh 2> /dev/null >&2 ; then
	$1_openssh
fi
