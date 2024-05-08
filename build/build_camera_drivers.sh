#!/bin/bash

if [ -f .env ]; then
    source .env
	source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project,E.g ./build/build_camera_drivers.sh"
    exit 1
fi

DEF_CONFIG=false
BUILD_JOBS=1
#The default driver to delete
DEFCONFIG_DRIVERS=(jz_sfc.ko lockd.ko nfs.ko nfsv2.ko nfsv3.ko grace.ko sunrpc.ko squashfs.ko jffs2.ko ${ZRT_ENV_KERNEL_MODULE_70MAI})
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

cd ${ZRT_ENV_TOP_DIR}/os/kernel/
if [ ${DEF_CONFIG} = true ]; then
	if [ -f .config ]; then
		cp .config config_drivers
	fi

	if [[ $ZRT_ENV_CUSTOM == *"70MAI"* ]];then    
		logger_info normal_info "make ${ZRT_ENV_KERNEL_CONFIG_70MAI}"        
		make ${ZRT_ENV_KERNEL_CONFIG_70MAI}
    else    
        make zeratul_SOC_T40_camera_defconfig
    fi

else
	echo "build without make defconfig"
fi

# build in kernel source drivers
cd ${ZRT_ENV_TOP_DIR}/os/kernel

INSTALL_MOD_PATH=${PWD}/_modules
export INSTALL_MOD_PATH=${INSTALL_MOD_PATH}

for i in ${DEFCONFIG_DRIVERS[*]}; do
    find ./ -name $i | xargs rm  > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "Delete existing drivers :"$i
    fi
done

rm -rf ${INSTALL_MOD_PATH}
#start make modules
echo "Start make modules:"
make modules -j${BUILD_JOBS}
if [ $? -ne 0 ]; then
    echo "Camera build in kernel source drivers error"
    exit 1
fi

make modules_install

cd $ZRT_ENV_TOP_DIR/70mai/app/lkm
make clean
make
make release
cd -

cd $ZRT_ENV_DRIVERS_DIR/wifi/build/
./build_camera_wifi.sh
cd -

if [ ${DEF_CONFIG} = true ]; then
	if [ -f config_drivers ]; then
		cp config_drivers .config
		rm config_drivers
	fi
fi

