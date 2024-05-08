#!/bin/bash

if [ $# -ne 2 ] ; then
    echo "Usage: sinature_70mai.sh rsa_private_key.pem code_dir"
    exit 1
fi

filepath=$(cd "$(dirname "$0")"; pwd) 
echo $filepath
mkdir -p out
export LD_LIBRARY_PATH=$filepath/openssl-1.1.1:$LD_LIBRARY_PATH

if [ $ZRT_ENV_IS_HASH == 1 ]; then
    echo 1111
    $filepath/sinature_hash_tool $2/uImage.lzo
    echo 2222
    $filepath/sinature_hash_tool $2/rootfs_camera.cpio.lzo 
    echo 3333
else
    $filepath/sinature_tool $2/uImage.lzo $1 
    $filepath/sinature_tool $2/rootfs_camera.cpio.lzo $1 
fi

if [ ! -e $2/uboot_scboot.bin ];then
    echo "Unable to find uboot_scboot.bin!"
    exit 1
fi

# 需要 padding normal boot
if [ -e $2/u-boot.bin ];then
    $filepath/sinature_tool $2/u-boot.bin $1
    ${ZRT_ENV_TOP_DIR}/build/pad_camera_u-boot.sh -i $2/u-boot.bin-signed -o $2/uboot.bin-signed -b $2/uboot_scboot.bin
    $filepath/scboot_tool $2/out/camera/uboot.bin-signed $1 1
else
    $filepath/scboot_tool $2/uboot_scboot.bin $1 1
fi

sudo mv ${ZRT_ENV_TOP_DIR}/out/u-boot-with-spl-signed.bin $2

$filepath/scboot_tool $2/boot.bin $1 1 

sudo mv out/u-boot-with-spl-signed.bin $2/u-boot-with-spl-noencrypt-signed.bin
sudo mv ${ZRT_ENV_TOP_DIR}/out/rsa_n.bin $2
sudo mv ${ZRT_ENV_TOP_DIR}/out/rsa_mod_n_sha256.bin $2

