#!/bin/bash

if [ -f .env ]; then
    source .env
    source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo -e "\033[31m"[ERROR]Please run source at top of zeratul project"\033[0m"
    exit 1
fi

if [ ! -d ${ZRT_ENV_OUT_CAMERA_DIR} ]; then
	mkdir -p ${ZRT_ENV_OUT_CAMERA_DIR}
fi

sudo echo

FORMAT_SD=false
SD_DEVICE=none
UBOOT_CONFIG=hm3001_t40n_msc0

while getopts "cfd:" arg
do
    case $arg in
    c)
        UBOOT_CONFIG=hm3001_sd_t40n
        ;;
    f)
        FORMAT_SD=true
        ;;
	d)
	    SD_DEVICE="$OPTARG"
	    ;;
	?)
        echo "unkonw argument"
        exit 1
	    ;;
    esac
done

if [ "$SD_DEVICE" == "none" ]; then
    logger_info error_info "Please specify a block device."
    exit 1
fi

if [ "$FORMAT_SD" == "true" ]; then
    set -e
    if [ -f $SD_DEVICE ]; then
        logger_info error_info "Please enter the correct block device. "
        exit 1
    fi
    # sudo fdisk -l $SD_DEVICE
    
    logger_info warning_info "!!!Please press Enter to confirm it is device $SD_DEVICE."
    read

    logger_info normal_info "Start formatting..."

    echo "o
    n
    p

    10240

    w
    " | sudo fdisk $SD_DEVICE

    logger_info normal_info "Compile the uboot."

    cd ${ZRT_ENV_OS_DIR}/uboot/u-boot
    make clean
	make ${UBOOT_CONFIG} > /dev/null 2>&1
	# make hm3001_t40n_nor
    
    logger_info normal_info "copy uboot to sdcard."

    sudo dd if=${ZRT_ENV_OS_DIR}/uboot/u-boot/u-boot-with-spl.bin of=$SD_DEVICE bs=1024 seek=17
    sync

    logger_info normal_info "Format sdcard as VFAT."
    sudo mkfs.vfat ${SD_DEVICE}1
fi
set -e

logger_info normal_info "Mount SD card."
cd ${ZRT_ENV_OUT_CAMERA_DIR}
sudo rm -rf mount_tmp
mkdir mount_tmp
sudo mount ${SD_DEVICE}1 mount_tmp 

logger_info normal_info "Copy the bin file to the SD card "

sudo cp *.bin mount_tmp
sudo cp ../../*.bin mount_tmp
sudo cp BL616/FW_OTA.bin.xz.ota mount_tmp
sudo cp rootfs_camera.cpio.lzo mount_tmp
sudo cp ${ZRT_ENV_TOP_DIR}/70mai/bl616/ota_fw/FW_OTA.bin.xz.ota mount_tmp
sudo cp uImage.lzo mount_tmp

ls mount_tmp

logger_info normal_info "Umount SD-Card."

sync
sudo umount mount_tmp
sudo rm -rf mount_tmp