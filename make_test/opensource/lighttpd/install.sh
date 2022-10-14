#!/bin/bash

ROOTFS=../../../filesystem/rootfs
HTML=$ROOTFS/html
OPENSSL=../openssl/target

mkdir -p $ROOTFS/{html,lib,ssl}
cp -a html/* $ROOTFS/html

source ../../../make/config.sh

echo "Copy lighttpd"
cp ./target/lib/mod_access.so     $ROOTFS/html/httpd
cp ./target/lib/mod_dirlisting.so $ROOTFS/html/httpd
cp ./target/lib/mod_indexfile.so  $ROOTFS/html/httpd
cp ./target/lib/mod_staticfile.so $ROOTFS/html/httpd
cp ./target/lib/mod_cgi.so        $ROOTFS/html/httpd
cp ./target/sbin/lighttpd         $ROOTFS/html/httpd
${CROSS_COMPILE}strip             $ROOTFS/html/httpd/*.so
${CROSS_COMPILE}strip             $ROOTFS/html/httpd/lighttpd

echo "Copy openssl"
chmod u+w $OPENSSL/lib/libcrypto.so.* $OPENSSL/lib/libssl.so.*
cp -d $OPENSSL/lib/*.so*          $ROOTFS/lib
cp $OPENSSL/ssl/openssl.cnf       $ROOTFS/ssl
${CROSS_COMPILE}strip             $ROOTFS/lib/libcrypto.so.*
${CROSS_COMPILE}strip             $ROOTFS/lib/libssl.so.*

echo "Finish"
