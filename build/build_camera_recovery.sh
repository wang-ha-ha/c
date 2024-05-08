#!/bin/bash

if [ -f .env ]; then
    source .env
	source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project,then "./build/build_camera_recovery.sh""
    exit 1
fi

echo "build camera recovery:"

sudo echo

RESTORE_CONFIG=false
BUILD_JOBS="1"
#user configuration,You can add it according to your own configuration
USER_CONFIG_RECOVERY_KERNEL=""
#defconfig
if [ ${ZRT_ENV_FLASH_TYPE} = NOR ]; then
	DEF_CONFIG_RECOVERY_KERNEL="zeratul_SOC_T40_camera_recovery_defconfig"
else
	DEF_CONFIG_RECOVERY_KERNEL="zeratul_SOC_T40_camera_recovery_nand_defconfig"
fi

if [ $USER_CONFIG_RECOVERY_KERNEL ]; then
    CONFIG_RECOVERY_KERNEL=$USER_CONFIG_RECOVERY_KERNEL
else
    CONFIG_RECOVERY_KERNEL=$DEF_CONFIG_RECOVERY_KERNEL
fi


while getopts "fj:" arg
do
    case $arg in
	f)
		RESTORE_CONFIG=true
		;;
	j)
		BUILD_JOBS=$OPTARG
		;;
    ?)
		echo "unkonw argument"
		exit 1
		;;
    esac
done

#Select the number of threads
if [ ${BUILD_JOBS} = "1" ]; then
	BUILD_JOBS=${ZRT_ENV_BUILD_JOBS}
fi

echo -e "*\n*Start compiling $CONFIG_RECOVERY_KERNEL\n*"
cd ${ZRT_ENV_OUT_CAMERA_DIR}

# generate camera recovery rootfs
rm -rf ./_rootfs_camera_recovery
cp ${ZRT_ENV_TOP_DIR}/os/rootfs/7.2.0/camera/rootfs_camera_recovery ./_rootfs_camera_recovery -r
find ./_rootfs_camera_recovery -name .git\* | xargs rm -rf

cd _rootfs_camera_recovery

sudo mknod -m 600 ./dev/console c 5 1
sudo mknod -m 666 ./dev/null c 1 3

cd ${ZRT_ENV_TOP_DIR}/os/kernel/

if [ ${RESTORE_CONFIG} = true ]; then
    if [ -f .config ]; then
	echo "copy the config bak"
	mv .config _config_bak
	make distclean
	fi
fi


make $CONFIG_RECOVERY_KERNEL

cat ${ZRT_ENV_TOP_DIR}/.env | xargs make uImage -j${BUILD_JOBS}

if [ $? -ne 0 ]; then
    echo "build recovery uImage error"
    exit 1
fi
cp arch/mips/boot/uImage ${ZRT_ENV_OUT_CAMERA_RECOVERY_FILE_PATH}
echo -e "*\n*Compiled successfully,Generate Camera recovery: ${ZRT_ENV_OUT_CAMERA_RECOVERY_FILE_PATH}\n*"
if [ ${RESTORE_CONFIG} = true ]; then
    if [ -f _config_bak ]; then
	echo "restore the config"
	make distclean
	mv _config_bak .config
#	cat ${ZRT_ENV_TOP_DIR}/.env | xargs make uImage.zrt -j${BUILD_OBJS}
    fi

    if [ $? -ne 0 ]; then
	echo "restore uImage error"
	exit 1
    fi
fi
