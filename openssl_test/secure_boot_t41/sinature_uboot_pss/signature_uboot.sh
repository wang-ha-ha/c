#!/bin/bash

NOR=1
SD=0

if [ $1 != ${SD} -a $1 != ${NOR} ];then
	echo error
	exit
fi

rm -rf out
if [ ! -f ./scboot_tool ];then
	make -j4
fi
mkdir -p out
./scboot_tool ./input_source/u-boot-with-spl.bin ./input_source/rsa_private_key.pem $1
md5sum ./input_source/u-boot-with-spl.bin
