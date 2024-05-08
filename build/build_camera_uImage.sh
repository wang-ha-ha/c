#!/bin/bash
if [ -f .env ]; then
    source .env
	source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project"
    exit 1
fi

DEF_CONFIG=false
BUILD_JOBS="1"
KERNEL_TYPE=""
#user configuration,You can add it according to your own configuration
if [ $ZRT_ENV_CUSTOM == 70MAI ] ; then
    USER_CONFIG_KERNEL=${ZRT_ENV_KERNEL_CONFIG_70MAI}
else
    USER_CONFIG_KERNEL=""
fi

#defconfig
	DEF_CONFIG_KERNEL="zeratul_SOC_T40_camera_defconfig"
#use configuration
if [ $USER_CONFIG_KERNEL ]; then
    CONFIG_KERNEL=$USER_CONFIG_KERNEL
else
    CONFIG_KERNEL=$DEF_CONFIG_KERNEL
fi

while getopts "fj:" arg
do
	case $arg in
	f)
		DEF_CONFIG=true
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



echo -e "*\n*Start compiling the kernel:$CONFIG_KERNEL\n*"

cd ${ZRT_ENV_OS_DIR}/kernel/
rm ${ZRT_ENV_OS_DIR}/kernel/arch/mips/xburst2/lib/isp-t40/lib.a
#if mandatory
if [ ${DEF_CONFIG} = true ]; then
	make $CONFIG_KERNEL
    echo "build make defconfig"
else
    echo "build without make defconfig"
fi
#Determin kernel type
if [ -f .config ]; then
    KERNEL_TYPE=`cat .config | grep  "CONFIG_INITRAMFS_SOURCE" |  sed 's/CONFIG_INITRAMFS_SOURCE=//'`
    if [ $KERNEL_TYPE ]; then
        if [ $KERNEL_TYPE != "\"\"" ]; then
            echo "Warning: .config may be a 'recovery' configuration and needs to use the default configuration!!!"
            make distclean
            make $CONFIG_KERNEL
        fi
    fi
fi
#if not config
if [ ! -f .config ]; then
	echo "Warning:Use default configuration:$CONFIG_KERNEL"
	make $CONFIG_KERNEL
fi

cat ${ZRT_ENV_TOP_DIR}/.env | xargs make uImage -j${BUILD_JOBS}
if [ $? -ne 0 ]; then
    echo "Camera build uImage error"
    exit 1
fi

cp arch/mips/boot/uImage.lzo ${ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH}

echo -e "*\n*Compile the kernel successfully\n*"
