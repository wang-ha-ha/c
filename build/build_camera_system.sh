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


echo -e "*\n*Start compiling system\n*"

BUILD_DRIVERS=false
BUILD_APPS=false
#Add the required drivers to the system partition,Please and `REMOVE_ROOTFS_KO` keep the same
#PUT_SYSTEM_KO=(nfs.ko nfsv2.ko nfsv3.ko sunrpc.ko lockd.ko grace.ko)
if [ ${ZRT_ENV_BUILD_TYPE} == debug ]; then
    PUT_SYSTEM_KO=(${ZRT_ENV_KERNEL_MODULE_70MAI})
else 
    PUT_SYSTEM_KO=(${ZRT_ENV_KERNEL_NECESSARY_MODULE_70MAI})
fi

while getopts "fb" arg
do
    case $arg in
	f)
		BUILD_DRIVERS=true
		;;
	b)
        BUILD_APPS=true
        ;;
	?)
		echo "unkonw argument"
		exit 1
		;;
    esac
done

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

#编译70mai的app
if [ ${BUILD_APPS} = true ]; then
    if [ $ZRT_ENV_CUSTOM == 70MAI ]; then
        logger_info normal_info "rebuild 70mai app"
        ${ZRT_ENV_BUILD_DIR}/build_70mai_app.sh
        if [ $? -ne 0 ]; then
            logger_info normal_info "build app error"
            exit 1
        fi
    fi
fi

cd ${ZRT_ENV_OUT_CAMERA_DIR}

# generate camera system
rm -rf ./system
cp ${ZRT_ENV_TOP_DIR}/os/rootfs/7.2.0/camera/system . -r
find ./system -name .git\* | xargs rm -rf

# copy the file you want here
mkdir -p system/lib/modules
for i in ${PUT_SYSTEM_KO[*]}; do
    find ${ZRT_ENV_TOP_DIR}/os/kernel/_modules/$i > /dev/null 2>&1
    file_list+=$i" "
done
set -e
if [ "$file_list" = "" ]; then
    echo "Warning: No file found, try try entering './build/build_camera_system -f'"
else
    for i in ${PUT_SYSTEM_KO[*]}; do
        find ${ZRT_ENV_TOP_DIR}/os/kernel/_modules -name $i | xargs -i cp {} ./system/lib/modules
        if [ -f ./system/lib/modules/$i ]; then
            echo Copy $i drive to system partition
        else
            echo Copy $i drive to system partition failed,Please check if the file exists
        fi
    done
fi

#cp ${ZRT_ENV_TOP_DIR}/os/drivers/eeprom/eeprom_at24.ko system/lib/modules/${KERNELRELEASE}/
cp ${ZRT_ENV_TOP_DIR}/os/drivers/jz_aes/aes_dev.ko system/lib/modules/

# 编译时拷贝文件到system
if [ $ZRT_ENV_CUSTOM == 70MAI ]; then
    cd ${ZRT_ENV_TOP_DIR}
    ${ZRT_ENV_TOP_DIR}/70mai/configs/script/setup.sh system
    cd -
fi

rm -f ${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH}

pack_ubifs()
{
    if [ $# != 3 ] && [ $# != 4 ];then
        logger_info error_info "Warning: pack_ubifs error"
        exit -1
    fi
    #system分区最大为20M，即(20*1024)/128 = 160, 最多逻辑可擦除块数目
    ubifs_bin_path=$1
    logic_name=$2
    logic_num=$3
    logic_num=$((logic_num*8))

    echo "pack_ubifs:$logic_name $logic_num"

    if [ -f $logic_name.ubifs ]; then
        rm -f $logic_name.ubifs
    fi
    
    if [ ! -e $logic_name ]; then
        mkdir -p $logic_name
    fi
    ls -l $logic_name
    if [ $# == 4 ];then
        $4
    fi

    mkfs.ubifs -e 124KiB -c $logic_num -m 2048 -d $logic_name -o $logic_name.ubifs -F
    ls -l $logic_name.ubifs

    if [ -f $logic_name_ubinize.cfg ]; then
        rm -f $logic_name_ubinize.cfg
    fi
    touch $logic_name_ubinize.cfg
    echo [ubifs] > $logic_name_ubinize.cfg
    echo mode=ubi >> $logic_name_ubinize.cfg
    echo image=$logic_name.ubifs >> $logic_name_ubinize.cfg
    echo vol_id=0 >> $logic_name_ubinize.cfg
    echo vol_type=dynamic >> $logic_name_ubinize.cfg
    echo vol_name=$logic_name >> $logic_name_ubinize.cfg
    echo vol_flags=autoresize >> $logic_name_ubinize.cfg
    echo vol_alignment=1 >> $logic_name_ubinize.cfg

    if [ -f $logic_name.bin ]; then
        rm -f $logic_name.bin
    fi
    ubinize -o $ubifs_bin_path -m 2048 -p 128KiB -s 2048 $logic_name_ubinize.cfg
    rm -f $logic_name.ubifs
    rm -f $logic_name_ubinize.cfg
}

a1_1_copy_userfs()
{
    echo a1_1_copy_userfs
    cp ${ZRT_ENV_TOP_DIR}/tools/make_tag/sensor_calibration_bin/sc301IoT-t40-0108.bin userfs
}

b1_1_copy_userfs()
{
    echo b1_1_copy_userfs
	#工厂调焦IQ
    cp ${ZRT_ENV_TOP_DIR}/tools/make_tag/sensor_calibration_bin/sc301IoT-t40-20240202.bin userfs
    cp -rf ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/FW_OTA.bin.xz.ota userfs
    cp -rf ${ZRT_ENV_OUT_CAMERA_DIR}/N32G031K8/N32G031K8.bin userfs/N32G031K8_backup.bin
}

#70mai需要将system目录放到nand中使用
if [ $ZRT_ENV_CUSTOM == 70MAI ] ; then
    case ${ZRT_ENV_BOARD_VERSION} in
        HM3001)
            rm -rf configfs userfs
            case ${ZRT_ENV_HARDWARE_VERSION} in
                a1_1)
                    pack_ubifs ${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} system 20
                    pack_ubifs ${ZRT_ENV_OUT_CAMERA_DIR}/userfs.bin userfs 48 a1_1_copy_userfs
                ;;
                b1_1)
                    # pack_ubifs ${ZRT_ENV_OUT_CAMERA_DIR}/configfs.bin configfs 3
                    pack_ubifs ${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} system 15
                    pack_ubifs ${ZRT_ENV_OUT_CAMERA_DIR}/userfs.bin userfs 49 b1_1_copy_userfs
                ;;
                *) 
                    echo "Unknown hardware version"
                    exit 1
                ;;
            esac
            ;;
        *) 
            echo "Unknown board version"
            exit 1
            ;;
    esac
elif [ ${ZRT_ENV_FLASH_TYPE} = NOR ]; then
	mksquashfs system ${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} -comp xz
else
	if [ -f system.ubifs ]; then
		rm -f system.ubifs
	fi
	mkfs.ubifs -e 0x1f000 -c 160 -m 2048 -d system -o system.ubifs -v
	ls -l system.ubifs

	if [ -f system_ubinize.cfg ]; then
		rm -f system_ubinize.cfg
	fi
	touch system_ubinize.cfg
	echo [ubifs] >> system_ubinize.cfg
	echo mode=ubi >> system_ubinize.cfg
	echo image=system.ubifs >> system_ubinize.cfg
	echo vol_id=0 >> system_ubinize.cfg
	echo vol_type=static >> system_ubinize.cfg
	echo vol_name=system >> system_ubinize.cfg
	echo vol_flags=autoresize >> system_ubinize.cfg
	echo vol_alignment=1 >> system_ubinize.cfg

	if [ -f system.bin ]; then
		rm -f system.bin
	fi
	ubinize -o ${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} -m 2048 -p 128KiB -s 2048 system_ubinize.cfg -v
	ls -l system.bin
	rm -f system.ubifs
	rm -f system_ubinize.cfg
fi
if [ $? -ne 0 ]; then
    echo "mksquashfs error, please check install the tools first."
    exit 1
fi

echo -e "*\n*Generate Camera system: ${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH}\n*"
