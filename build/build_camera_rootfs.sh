#!/bin/bash

if [ -f .env ]; then
    source .env
	source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project"
    exit 1
fi

if [ ! -d ${ZRT_ENV_OUT_CAMERA_DIR} ]; then
	mkdir -p ${ZRT_ENV_OUT_CAMERA_DIR}
fi

sudo echo

BUILD_DRIVERS=false
BUILD_APPS=false
BUILD_CLEAN=false
BUILD_OUT_CAMERA_ROOTFS=false
#Remove the unnecessary Ko in rootfs and put the system partition,Please and `PUT_SYSTEM_KO` keep the same
REMOVE_ROOTFS_KO=(nfs.ko nfsve.ko nfsv3.ko lockd.ko sunrpc.ko grace.ko ${ZRT_ENV_KERNEL_MODULE_70MAI})
while getopts "fcbo" arg
do
    case $arg in
	f)
	    BUILD_DRIVERS=true
	    ;;
	c)
	    BUILD_CLEAN=true
	    ;;
	b)
	    BUILD_APPS=true
	    ;;
	o)
		BUILD_OUT_CAMERA_ROOTFS=true
		;;
	?)
        echo "unkonw argument"
        exit 1
	    ;;
    esac
done

echo -e "*\n*Start build camera rootfs\n*"
#Separate compressed file system
if [ ${BUILD_OUT_CAMERA_ROOTFS} = true ]; then
	echo "build out/camera/_rootfs_camera"
	if [ -d ${ZRT_ENV_OUT_CAMERA_DIR}/_rootfs_camera ]; then
		echo "${ZRT_ENV_OUT_CAMERA_DIR}/_rootfs_camera"
		# pack and compress
		cd ${ZRT_ENV_OUT_CAMERA_DIR}
		rm -f rootfs_camera.cpio.lzo
		cd ${ZRT_ENV_OUT_CAMERA_DIR}/_rootfs_camera
		find . | cpio -H newc -o > ../rootfs_camera.cpio
		cd ..

		lzop -9 -f rootfs_camera.cpio -o rootfs_camera.cpio.lzo
		${ZRT_ENV_TOP_DIR}/build/mark_rootfs_pc rootfs_camera.cpio.lzo
		rm rootfs_camera.cpio
		echo "Generate Camera rootfs: "${ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH}
		exit 0
	else
		echo "No out/camera/_rootfs_camera ,build fail"
		exit 1
	fi
fi


if [ ${BUILD_DRIVERS} = true ]; then
    echo "rebuild camera drivers again"
    ${ZRT_ENV_BUILD_DIR}/build_camera_drivers.sh -f
    if [ $? -ne 0 ]; then
	echo "Camera build drivers error"
	exit 1
    fi
else
    echo "use the drivers that already been built"
fi

if [ ${BUILD_APPS} = true ]; then
    if [ $ZRT_ENV_CUSTOM == 70MAI ]; then
        logger_info normal_info "rebuild 70mai app"
        ${ZRT_ENV_BUILD_DIR}/build_70mai_app.sh ${BUILD_CLEAN}
        if [ $? -ne 0 ]; then
            logger_info normal_info "build app error"
            exit 1
        fi
    fi
fi

cd ${ZRT_ENV_OUT_CAMERA_DIR}

# generate camera rootfs,copy to out/camera/
rm -rf ./_rootfs_camera
cp ${ZRT_ENV_TOP_DIR}/os/rootfs/7.2.0/camera/rootfs_camera ./_rootfs_camera -r
find ./_rootfs_camera -name .git\* | xargs rm -rf
#enter the rootfs directory
cd _rootfs_camera

sed -i "s/export TRANSFER_MODE=/export TRANSFER_MODE=${ZRT_ENV_TRANSFER_MODE}/g" etc/init.d/rcS
sed -i "s/export PRODUCT_MODE=/export PRODUCT_MODE=${ZRT_ENV_PRODUCT_MODE}/g" etc/init.d/rcS
sed -i "s/export SENSOR=/export SENSOR=${ZRT_ENV_SENSOR}/g" etc/init.d/rcS
sed -i "s/export SOC_TYPE=/export SOC_TYPE=${ZRT_ENV_SOC_TYPE}/g" etc/init.d/rcS
sed -i "s/export FLASH_TYPE=/export FLASH_TYPE=${ZRT_ENV_FLASH_TYPE}/g" etc/init.d/rcS
sed -i "s/export BOARD_TYPE=/export BOARD_TYPE=${ZRT_ENV_BOARD_VERSION}/g" etc/init.d/rcS
sed -i "s/export HARDWARE_VERSION=/export HARDWARE_VERSION=${ZRT_ENV_HARDWARE_VERSION}/g" etc/init.d/rcS

#sudo mknod -m 660 ./dev/mem c 1 1
if [ $? -ne 0 ]; then
    echo "mknod error, please check root permission."
    exit 1
fi

if [[ ${ZRT_ENV_SOC_TYPE} = SOC_T40N ]]; then

if [ ${ZRT_ENV_SENSOR_NUM} = two ]; then
sudo mknod -m 660 ./dev/avpu c 251, 0
sudo mknod -m 660 ./dev/bscaler0 c 10, 51
sudo mknod -m 660 ./dev/console c 5, 1
sudo mknod -m 660 ./dev/cpu_dma_latency c 10, 44
sudo mknod -m 660 ./dev/dbox c 10, 52
sudo mknod -m 660 ./dev/dsp c 10, 45
sudo mknod -m 660 ./dev/framechan0 c 10, 61
sudo mknod -m 660 ./dev/framechan1 c 10, 60
sudo mknod -m 660 ./dev/framechan2 c 10, 59
sudo mknod -m 660 ./dev/framechan3 c 10, 58
sudo mknod -m 660 ./dev/framechan4 c 10, 57
sudo mknod -m 660 ./dev/framechan5 c 10, 56
sudo mknod -m 660 ./dev/full c 1, 7
sudo mknod -m 660 ./dev/i2d c 10, 54
sudo mknod -m 660 ./dev/ipu c 10, 53
sudo mknod -m 660 ./dev/isp-m0 c 10, 62
sudo mknod -m 660 ./dev/kmsg c 1, 11
sudo mknod -m 660 ./dev/log_events c 10, 48
sudo mknod -m 660 ./dev/log_main c 10, 49
sudo mknod -m 660 ./dev/log_radio c 10, 47
sudo mknod -m 660 ./dev/log_system c 10, 46
sudo mknod -m 660 ./dev/loop-control c 10, 237
sudo mknod -m 660 ./dev/loop0 b 7, 0
sudo mknod -m 660 ./dev/loop1 b 7, 1
sudo mknod -m 660 ./dev/mem c 1, 1 
sudo mknod -m 660 ./dev/memory_bandwidth c 10, 41
sudo mknod -m 660 ./dev/mtd0 c 90, 0
sudo mknod -m 660 ./dev/mtd0ro c 90, 1
sudo mknod -m 660 ./dev/mtd1 c 90, 2
sudo mknod -m 660 ./dev/mtd1ro c 90, 3
sudo mknod -m 660 ./dev/mtd2 c 90, 4
sudo mknod -m 660 ./dev/mtd2ro c 90, 5
sudo mknod -m 660 ./dev/mtd3 c 90, 6
sudo mknod -m 660 ./dev/mtd3ro c 90, 7
sudo mknod -m 660 ./dev/mtd4 c 90, 8
sudo mknod -m 660 ./dev/mtd4ro c 90, 9
sudo mknod -m 660 ./dev/mtd5 c 90, 10
sudo mknod -m 660 ./dev/mtd5ro c 90, 11
sudo mknod -m 660 ./dev/mtd6 c 90, 12
sudo mknod -m 660 ./dev/mtd6ro c 90, 13
sudo mknod -m 660 ./dev/mtd7 c 90, 14
sudo mknod -m 660 ./dev/mtd7ro c 90, 15
sudo mknod -m 660 ./dev/mtdblock0 b 31, 0
sudo mknod -m 660 ./dev/mtdblock1 b 31, 1
sudo mknod -m 660 ./dev/mtdblock2 b 31, 2
sudo mknod -m 660 ./dev/mtdblock3 b 31, 3
sudo mknod -m 660 ./dev/mtdblock4 b 31, 4
sudo mknod -m 660 ./dev/mtdblock5 b 31, 5
sudo mknod -m 660 ./dev/mtdblock6 b 31, 6
sudo mknod -m 660 ./dev/mtdblock7 b 31, 7
sudo mknod -m 660 ./dev/network_latency c 10, 43
sudo mknod -m 660 ./dev/network_throughput c 10, 42
sudo mknod -m 660 ./dev/null c 1, 3
sudo mknod -m 660 ./dev/ptmx c 5, 2
sudo mknod -m 660 ./dev/ptyp0 c 2, 0
sudo mknod -m 660 ./dev/ptyp1 c 2, 1
sudo mknod -m 660 ./dev/ram0 b 1, 0
sudo mknod -m 660 ./dev/ram1 b 1, 1
sudo mknod -m 660 ./dev/ram10 b 1, 10
sudo mknod -m 660 ./dev/ram11 b 1, 11
sudo mknod -m 660 ./dev/ram12 b 1, 12
sudo mknod -m 660 ./dev/ram13 b 1, 13
sudo mknod -m 660 ./dev/ram14 b 1, 14
sudo mknod -m 660 ./dev/ram15 b 1, 15
sudo mknod -m 660 ./dev/ram2 b 1, 2
sudo mknod -m 660 ./dev/ram3 b 1, 3
sudo mknod -m 660 ./dev/ram4 b 1, 4
sudo mknod -m 660 ./dev/ram5 b 1, 5
sudo mknod -m 660 ./dev/ram6 b 1, 6
sudo mknod -m 660 ./dev/ram7 b 1, 7
sudo mknod -m 660 ./dev/ram8 b 1, 8
sudo mknod -m 660 ./dev/ram9 b 1, 9
sudo mknod -m 660 ./dev/random c 1, 8
sudo mknod -m 660 ./dev/rfkill c 10, 55
sudo mknod -m 660 ./dev/rmem c 10, 50
sudo mknod -m 660 ./dev/tisp c 253, 0
sudo mknod -m 660 ./dev/tty c 5, 0
sudo mknod -m 660 ./dev/ttyS1 c 4, 65
sudo mknod -m 660 ./dev/ttyp0 c 3, 0
sudo mknod -m 660 ./dev/ttyp1 c 3, 1
sudo mknod -m 660 ./dev/tx-isp c 10, 63
sudo mknod -m 660 ./dev/urandom c 1, 9
sudo mknod -m 660 ./dev/watchdog c 10, 130
sudo mknod -m 660 ./dev/watchdog0 c 252, 0
sudo mknod -m 660 ./dev/zero c 1, 5
sudo mknod -m 660 ./dev/zram0 b 254, 0
sudo mknod -m 660 ./dev/zram1 b 254, 1

else
sudo mknod -m 660 ./dev/avpu c 251, 0
sudo mknod -m 660 ./dev/bscaler0 c 10, 54
sudo mknod -m 660 ./dev/console c 5, 1
sudo mknod -m 660 ./dev/cpu_dma_latency c 10, 47
sudo mknod -m 660 ./dev/dbox c 10, 55
sudo mknod -m 660 ./dev/dsp c 10, 48
# sudo mknod -m 660 ./dev/fb0 c 29, 0
# sudo mknod -m 660 ./dev/mpsys_data c 10, 41
# sudo mknod -m 660 ./dev/framechan0 c 10, 61
# sudo mknod -m 660 ./dev/framechan1 c 10, 60
# sudo mknod -m 660 ./dev/framechan2 c 10, 59
sudo mknod -m 660 ./dev/full c 1, 7
sudo mknod -m 660 ./dev/i2d c 10, 57
sudo mknod -m 660 ./dev/ipu c 10, 56
sudo mknod -m 660 ./dev/isp-m0 c 10, 62
sudo mknod -m 660 ./dev/kmsg c 1, 11
sudo mknod -m 660 ./dev/log_events c 10, 51
sudo mknod -m 660 ./dev/log_main c 10, 52
sudo mknod -m 660 ./dev/log_radio c 10, 50
sudo mknod -m 660 ./dev/log_system c 10, 49
sudo mknod -m 660 ./dev/loop-control c 10, 237
sudo mknod -m 660 ./dev/loop0 b 7, 0
sudo mknod -m 660 ./dev/loop1 b 7, 1
sudo mknod -m 660 ./dev/mem c 1, 1 
sudo mknod -m 660 ./dev/memory_bandwidth c 10, 44
sudo mknod -m 660 ./dev/mtd0 c 90, 0
sudo mknod -m 660 ./dev/mtd0ro c 90, 1
sudo mknod -m 660 ./dev/mtd1 c 90, 2
sudo mknod -m 660 ./dev/mtd1ro c 90, 3
sudo mknod -m 660 ./dev/mtd2 c 90, 4
sudo mknod -m 660 ./dev/mtd2ro c 90, 5
sudo mknod -m 660 ./dev/mtd3 c 90, 6
sudo mknod -m 660 ./dev/mtd3ro c 90, 7
sudo mknod -m 660 ./dev/mtd4 c 90, 8
sudo mknod -m 660 ./dev/mtd4ro c 90, 9
sudo mknod -m 660 ./dev/mtd5 c 90, 10
sudo mknod -m 660 ./dev/mtd5ro c 90, 11
sudo mknod -m 660 ./dev/mtd6 c 90, 12
sudo mknod -m 660 ./dev/mtd6ro c 90, 13
sudo mknod -m 660 ./dev/mtd7 c 90, 14
sudo mknod -m 660 ./dev/mtd7ro c 90, 15
sudo mknod -m 660 ./dev/mtd8 c 90, 16
sudo mknod -m 660 ./dev/mtd8ro c 90, 17
sudo mknod -m 660 ./dev/mtd9 c 90, 18
sudo mknod -m 660 ./dev/mtd9ro c 90, 19
sudo mknod -m 660 ./dev/mtd10 c 90, 20
sudo mknod -m 660 ./dev/mtd10ro c 90, 21
sudo mknod -m 660 ./dev/mtd11 c 90, 22
sudo mknod -m 660 ./dev/mtd11ro c 90, 23
sudo mknod -m 660 ./dev/mtd12 c 90, 24
sudo mknod -m 660 ./dev/mtd12ro c 90, 25
sudo mknod -m 660 ./dev/mtdblock0 b 31, 0
sudo mknod -m 660 ./dev/mtdblock1 b 31, 1
sudo mknod -m 660 ./dev/mtdblock2 b 31, 2
sudo mknod -m 660 ./dev/mtdblock3 b 31, 3
sudo mknod -m 660 ./dev/mtdblock4 b 31, 4
sudo mknod -m 660 ./dev/mtdblock5 b 31, 5
sudo mknod -m 660 ./dev/mtdblock6 b 31, 6
sudo mknod -m 660 ./dev/mtdblock7 b 31, 7
sudo mknod -m 660 ./dev/mtdblock8 b 31, 8
sudo mknod -m 660 ./dev/mtdblock9 b 31, 9
sudo mknod -m 660 ./dev/mtdblock10 b 31, 10
sudo mknod -m 660 ./dev/mtdblock11 b 31, 11
sudo mknod -m 660 ./dev/mtdblock12 b 31, 12
sudo mknod -m 660 ./dev/network_latency c 10, 46
sudo mknod -m 660 ./dev/network_throughput c 10, 45
sudo mknod -m 660 ./dev/null c 1, 3
sudo mknod -m 660 ./dev/ptmx c 5, 2
sudo mknod -m 660 ./dev/ptyp0 c 2, 0
sudo mknod -m 660 ./dev/ptyp1 c 2, 1
sudo mknod -m 660 ./dev/ram0 b 1, 0
sudo mknod -m 660 ./dev/ram1 b 1, 1
sudo mknod -m 660 ./dev/ram10 b 1, 10
sudo mknod -m 660 ./dev/ram11 b 1, 11
sudo mknod -m 660 ./dev/ram12 b 1, 12
sudo mknod -m 660 ./dev/ram13 b 1, 13
sudo mknod -m 660 ./dev/ram14 b 1, 14
sudo mknod -m 660 ./dev/ram15 b 1, 15
sudo mknod -m 660 ./dev/ram2 b 1, 2
sudo mknod -m 660 ./dev/ram3 b 1, 3
sudo mknod -m 660 ./dev/ram4 b 1, 4
sudo mknod -m 660 ./dev/ram5 b 1, 5
sudo mknod -m 660 ./dev/ram6 b 1, 6
sudo mknod -m 660 ./dev/ram7 b 1, 7
sudo mknod -m 660 ./dev/ram8 b 1, 8
sudo mknod -m 660 ./dev/ram9 b 1, 9
sudo mknod -m 660 ./dev/random c 1, 8
sudo mknod -m 660 ./dev/rfkill c 10, 58
sudo mknod -m 660 ./dev/rmem c 10, 53
sudo mknod -m 660 ./dev/tisp c 253, 0
sudo mknod -m 660 ./dev/tty c 5, 0
sudo mknod -m 660 ./dev/ttyS0 c 4, 64
sudo mknod -m 660 ./dev/ttyS1 c 4, 65
sudo mknod -m 660 ./dev/ttyS2 c 4, 66
sudo mknod -m 660 ./dev/ttyp0 c 3, 0
sudo mknod -m 660 ./dev/ttyp1 c 3, 1
sudo mknod -m 660 ./dev/tx-isp c 10, 63
sudo mknod -m 660 ./dev/urandom c 1, 9
sudo mknod -m 660 ./dev/watchdog c 10, 130
sudo mknod -m 660 ./dev/watchdog0 c 252, 0
sudo mknod -m 660 ./dev/zero c 1, 5
sudo mknod -m 660 ./dev/zram0 b 254, 0
sudo mknod -m 660 ./dev/zram1 b 254, 1
sudo mknod -m 660 ./dev/aes c 10, 38
sudo mknod -m 660 ./dev/env c 10, 39
sudo mknod -m 660 ./dev/i2c-0 c 89, 0
sudo mknod -m 660 ./dev/i2c-1 c 89, 1
sudo mknod -m 660 ./dev/i2c-2 c 89, 2
sudo mknod -m 660 ./dev/i2c-3 c 89, 3
sudo mknod -m 660 ./dev/ingenic_sfc c 249, 0
sudo mknod -m 660 ./dev/nna_lock c 10, 42
sudo mknod -m 660 ./dev/soc-nna c 10, 40
sudo mknod -m 660 ./dev/ubi0 c 248, 0
sudo mknod -m 660 ./dev/ubi0_0 c 248, 1
sudo mknod -m 660 ./dev/ubi1 c 247, 0
sudo mknod -m 660 ./dev/ubi1_0 c 247, 1
sudo mknod -m 660 ./dev/ubi_ctrl c 10, 43
fi

fi

#enter the kernel directory
cd ${ZRT_ENV_TOP_DIR}/os/kernel
KERNELRELEASE=`cat include/config/kernel.release 2> /dev/null`
if [ "${KERNELRELEASE}" != "${ZRT_ENV_KERNEL_RELEASE}" ]; then
	echo "Please enter ./build/build_camera_rootfs -f "
	exit 1
fi
#return to the previous directory
cd -
rm ./lib/modules/* -rf
mkdir ./lib/modules/${KERNELRELEASE}

# copy in kernel source drivers
find ${ZRT_ENV_TOP_DIR}/os/kernel/_modules -name *.ko | xargs -i cp {} ./lib/modules/${KERNELRELEASE}/

# copy os/drivers
cp ${ZRT_ENV_TOP_DIR}/os/drivers/mpsys-driver/mpsys.ko ./lib/modules/${KERNELRELEASE}/

# remove the ko u dont' want,
for i in ${REMOVE_ROOTFS_KO[*]}; do
    rm ./lib/modules/${KERNELRELEASE}/$i > /dev/null 2>&1
    echo remove rootfs $i
done
# copy prebuilt mmc ko
# strip debug
mips-linux-gnu-strip --strip-debug ./lib/modules/${KERNELRELEASE}/*

# link
cd ./lib/modules/${KERNELRELEASE}/
for i in ${REMOVE_ROOTFS_KO[*]}; do
    ln -s /system/lib/modules/$i $i
    echo Create $i soft connection system
done
cd ..
ln -s ./${KERNELRELEASE}/* .

#70mai 将主控程序放到rootfs
set -e
# 编译时拷贝文件到rootfs
if [ $ZRT_ENV_CUSTOM == 70MAI ]; then
    if [ $ZRT_ENV_BUILD_TYPE == debug ]; then
        cd ${ZRT_ENV_OUT_CAMERA_DIR}
        rm -rf ./_rootfs_camera_debug
        cp -r $ZRT_ENV_TOP_DIR/os/rootfs/7.2.0/camera/rootfs_camera_debug $ZRT_ENV_OUT_CAMERA_DIR/_rootfs_camera_debug 
        cd _rootfs_camera_debug
        sed -i "s/export TRANSFER_MODE=/export TRANSFER_MODE=${ZRT_ENV_TRANSFER_MODE}/g" etc/init.d/rcS
        sed -i "s/export PRODUCT_MODE=/export PRODUCT_MODE=${ZRT_ENV_PRODUCT_MODE}/g" etc/init.d/rcS
        sed -i "s/export SENSOR=/export SENSOR=${ZRT_ENV_SENSOR}/g" etc/init.d/rcS
        sed -i "s/export SOC_TYPE=/export SOC_TYPE=${ZRT_ENV_SOC_TYPE}/g" etc/init.d/rcS
        sed -i "s/export FLASH_TYPE=/export FLASH_TYPE=${ZRT_ENV_FLASH_TYPE}/g" etc/init.d/rcS
        sed -i "s/export BOARD_TYPE=/export BOARD_TYPE=${ZRT_ENV_BOARD_VERSION}/g" etc/init.d/rcS
        sed -i "s/export HARDWARE_VERSION=/export HARDWARE_VERSION=${ZRT_ENV_HARDWARE_VERSION}/g" etc/init.d/rcS
    fi
    cd ${ZRT_ENV_TOP_DIR}
    ${ZRT_ENV_TOP_DIR}/70mai/configs/script/setup.sh rootfs
    cd -
fi

# pack and compress
cd ${ZRT_ENV_OUT_CAMERA_DIR}/_rootfs_camera
find . | cpio -H newc -o > ../rootfs_camera.cpio
cd ..

rm -f rootfs_camera.cpio.lzo
lzop -9 -f rootfs_camera.cpio -o rootfs_camera.cpio.lzo
cd ${ZRT_ENV_TOOLS_DIR}/mark_rootfs/
make
cd ${ZRT_ENV_OUT_CAMERA_DIR}
${ZRT_ENV_TOOLS_DIR}/mark_rootfs/mark_rootfs_pc rootfs_camera.cpio.lzo
rm rootfs_camera.cpio


echo -e "*\n*Generate Camera rootfs: ${ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH}\n*"
