if [ -f build/env_setup_zeratul_hm3001_a1_1.sh ]; then
    echo "Setup Zeratul build environment(devp)":

    if [ "${SENSOR_NUM}" = "one" ] ; then
        FW_VERSION="\"ZRT_${BUILD_TYPE}_`date +%Y%m%d%H%M`_`whoami`_`hostname`;[VERSION];ver=CAMERA_${SOC_TYPE}_${SENSOR}_V${VERSION};\""
    else
        FW_VERSION="\"ZRT_${BUILD_TYPE}_`date +%Y%m%d%H%M`_`whoami`_`hostname`;[VERSION];ver=CAMERA_${SOC_TYPE}_${SENSOR}_${SENSOR1}_V${VERSION};\""
    fi

    export ZRT_ENV_USER_MODE_70MAI=${USER_MODE_70MAI}
    export ZRT_ENV_FACTORY_MODE_70MAI=${FACTORY_MODE_70MAI}
    export ZRT_ENV_FW_VERSION=${FW_VERSION}
    export ZRT_ENV_UPDATE_VERSION=CAMERA_${SOC_TYPE}_${SENSOR}_${TOOL_CHAIN}_V${VERSION}
    export ZRT_ENV_KERNEL_RELEASE=4.4.94
    export ZRT_ENV_BUILD_TYPE=${BUILD_TYPE}
    export ZRT_ENV_BOARD_VERSION=${BOARD_VERSION}
    export ZRT_ENV_SOC_TYPE=${SOC_TYPE}
    export ZRT_ENV_FLASH_TYPE=${FLASH_TYPE}
    export ZRT_ENV_SENSOR_NUM=${SENSOR_NUM}
    export ZRT_ENV_SENSOR=${SENSOR}
    export ZRT_ENV_SENSOR1=${SENSOR1}
    export ZRT_ENV_TOOL_CHAIN=${TOOL_CHAIN}	
    export ZRT_ENV_BUILD_JOBS=${JOBS}
    export ZRT_ENV_TRANSFER_MODE=${TRANSFER_MODE}	
    export ZRT_ENV_PRODUCT_MODE=SINGLE   
    export ZRT_ENV_TOP_DIR=$(pwd)
    export ZRT_ENV_OS_DIR=${ZRT_ENV_TOP_DIR}/os
    export ZRT_ENV_KERNEL_DIR=${ZRT_ENV_OS_DIR}/kernel/
    export ZRT_ENV_LINUX_VERSION=${ZRT_ENV_KERNEL_RELEASE}
    export ZRT_ENV_BUILD_DIR=${ZRT_ENV_TOP_DIR}/build
    export ZRT_ENV_OUT_DIR=${ZRT_ENV_TOP_DIR}/out
    export ZRT_ENV_DRIVERS_DIR=${ZRT_ENV_TOP_DIR}/os/drivers
    export ZRT_ENV_USERLAND_DIR=${ZRT_ENV_TOP_DIR}/userland
    export ZRT_ENV_TOOLS_DIR=${ZRT_ENV_TOP_DIR}/tools
    export ZRT_ENV_OUT_CAMERA_DIR=${ZRT_ENV_OUT_DIR}/${BOARD_VERSION}/${HARDWARE_VERSION}
    export ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/boot.bin
    export ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/tag.bin
    export ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/uImage.lzo
    export ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/rootfs_camera.cpio.lzo
    export ZRT_ENV_OUT_CAMERA_RECOVERY_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/recovery.bin
    export ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/system.bin
    export ZRT_ENV_OUT_CAMERA_CONFIG_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/config.bin
    export ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/env.bin  #add
    export PATH=${ZRT_ENV_TOOLS_DIR}/toolchain/gcc_720/mips-linux-gnu-ingenic-gcc7.2.0-glibc2.29-fp64-r5.1.1.sr02/bin:$PATH
    export PATH=${ZRT_ENV_TOOLS_DIR}/toolchain/gcc_mcu/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH
    #新增
    export ZRT_ENV_HARDWARE_VERSION=${HARDWARE_VERSION}
    export ZRT_ENV_CUSTOM=${CUSTOM}
    export ZRT_ENV_BOARD_VERSION_LOWER=${BOARD_VERSION_LOWER}
    export ZRT_ENV_WIFI_CHIP=${WIFI_CHIP}
    export ZRT_ENV_WDR_ENABLED=${WDR_ENABLED}
    export ZRT_ENV_KERNEL_CONFIG_70MAI=${KERNEL_CONFIG_70MAI}
    export ZRT_ENV_KERNEL_MODULE_70MAI=${KERNEL_MODULE_70MAI}
    export ZRT_ENV_KERNEL_NECESSARY_MODULE_70MAI="\"${KERNEL_NECESSARY_MODULE_70MAI}\""
    export ZRT_ENV_OUT_CAMERA_WIFI_OTA_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/BL616/FW_OTA.bin.xz.ota
    export ZRT_ENV_OUT_CAMERA_MCU_N32G_OTA_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/N32G031K8/N32G031K8.bin
    export ZRT_ENV_OUT_CAMERA_IQ_FILE_OTA_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/sc301IoT_t40.bin
    export ZRT_ENV_AI_ALG=$AI_ALG
    export ZRT_ENV_IS_HASH=$IS_HASH

    echo ============================================================

    echo ZRT_ENV_FW_VERSION=${ZRT_ENV_FW_VERSION} > .env
    echo ZRT_ENV_USER_MODE_70MAI=${USER_MODE_70MAI} >> .env
    echo ZRT_ENV_FACTORY_MODE_70MAI=${FACTORY_MODE_70MAI} >> .env
    echo ZRT_ENV_UPDATE_VERSION=${ZRT_ENV_UPDATE_VERSION} >> .env
    echo ZRT_ENV_KERNEL_RELEASE=${ZRT_ENV_KERNEL_RELEASE} >> .env
    echo ZRT_ENV_BUILD_TYPE=${ZRT_ENV_BUILD_TYPE} >> .env
    echo ZRT_ENV_BOARD_VERSION=${ZRT_ENV_BOARD_VERSION} >> .env
    echo ZRT_ENV_SOC_TYPE=${ZRT_ENV_SOC_TYPE} >> .env
    echo ZRT_ENV_FLASH_TYPE=${ZRT_ENV_FLASH_TYPE} >> .env
    echo ZRT_ENV_SENSOR_NUM=${ZRT_ENV_SENSOR_NUM} >> .env
    echo ZRT_ENV_SENSOR=${ZRT_ENV_SENSOR} >> .env
    echo ZRT_ENV_SENSOR1=${ZRT_ENV_SENSOR1} >> .env
    echo ZRT_ENV_BUILD_JOBS=${ZRT_ENV_BUILD_JOBS} >> .env
    echo ZRT_ENV_LINUX_VERSION=${ZRT_ENV_KERNEL_RELEASE} >> .env
    echo ZRT_ENV_TRANSFER_MODE=${ZRT_ENV_TRANSFER_MODE} >> .env
    echo ZRT_ENV_PRODUCT_MODE=${ZRT_ENV_PRODUCT_MODE} >> .env
    echo ZRT_ENV_TOP_DIR=${ZRT_ENV_TOP_DIR} >> .env
    echo ZRT_ENV_BUILD_DIR=${ZRT_ENV_BUILD_DIR} >> .env
    echo ZRT_ENV_OS_DIR=${ZRT_ENV_OS_DIR} >> .env
    echo ZRT_ENV_OUT_DIR=${ZRT_ENV_OUT_DIR} >> .env
    echo ZRT_ENV_DRIVERS_DIR=${ZRT_ENV_DRIVERS_DIR} >> .env
    echo ZRT_ENV_USERLAND_DIR=${ZRT_ENV_USERLAND_DIR} >> .env
    echo ZRT_ENV_TOOLS_DIR=${ZRT_ENV_TOOLS_DIR} >> .env
    echo ZRT_ENV_OUT_CAMERA_DIR=${ZRT_ENV_OUT_CAMERA_DIR} >> .env
    echo ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH=${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH} >> .env
    echo ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH=${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH} >> .env
    echo ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH=${ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH} >> .env
    echo ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH=${ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH} >> .env
    echo ZRT_ENV_OUT_CAMERA_RECOVERY_FILE_PATH=${ZRT_ENV_OUT_CAMERA_RECOVERY_FILE_PATH} >> .env
    echo ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH=${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} >> .env
    echo ZRT_ENV_OUT_CAMERA_CONFIG_FILE_PATH=${ZRT_ENV_OUT_CAMERA_CONFIG_FILE_PATH} >> .env
    echo ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/env.bin >> .env   #add

    #新增
    echo ZRT_ENV_HARDWARE_VERSION=${HARDWARE_VERSION} >> .env
    echo ZRT_ENV_CUSTOM=${CUSTOM} >> .env
    echo ZRT_ENV_BOARD_VERSION_LOWER=${BOARD_VERSION_LOWER} >> .env
    echo ZRT_ENV_WIFI_CHIP=${WIFI_CHIP} >> .env
    echo ZRT_ENV_WDR_ENABLED=${WDR_ENABLED} >> .env
    echo ZRT_ENV_KERNEL_CONFIG_70MAI=${KERNEL_CONFIG_70MAI} >> .env
    echo ZRT_ENV_KERNEL_MODULE_70MAI=${KERNEL_MODULE_70MAI} >> .env
    echo ZRT_ENV_KERNEL_NECESSARY_MODULE_70MAI="\"${KERNEL_NECESSARY_MODULE_70MAI}\"" >> .env
    echo PATH=${ZRT_ENV_TOOLS_DIR}/toolchain/gcc_720/mips-linux-gnu-ingenic-gcc7.2.0-glibc2.29-fp64-r5.1.1.sr02/bin:$PATH >> .env
    echo PATH=${ZRT_ENV_TOOLS_DIR}/toolchain/gcc_mcu/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH >> .env
    echo ZRT_ENV_OUT_CAMERA_WIFI_OTA_FILE_PATH=${ZRT_ENV_OUT_CAMERA_WIFI_OTA_FILE_PATH} >> .env
    echo ZRT_ENV_OUT_CAMERA_MCU_N32G_OTA_FILE_PATH=${ZRT_ENV_OUT_CAMERA_MCU_N32G_OTA_FILE_PATH} >> .env
    echo ZRT_ENV_OUT_CAMERA_IQ_FILE_OTA_FILE_PATH=${ZRT_ENV_OUT_CAMERA_IQ_FILE_OTA_FILE_PATH} >> .env
    echo ZRT_ENV_AI_ALG=$AI_ALG >> .env
    echo ZRT_ENV_IS_HASH=$IS_HASH >> .env
    cat .env

    echo ============================================================

else

    echo "Please run source at top of isvp"
fi