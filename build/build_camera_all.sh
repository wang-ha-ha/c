#!/bin/bash -e

ROOT_PATH=`pwd`
export PATH=$ROOT_PATH/tools/toolchain/gcc_720/mips-linux-gnu-ingenic-gcc7.2.0-glibc2.29-fp64-r5.1.1.sr02/bin:$PATH

if [ -f .env ]; then
    source .env
	source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project"
    exit 1
fi

# Setup the variables
typeset -u NAME

if [ ${ZRT_ENV_SENSOR_NUM} = two ]; then
	NAME=ZRT_"$ZRT_ENV_SOC_TYPE"_"$ZRT_ENV_SENSOR"_"$ZRT_ENV_SENSOR1"_FW
else
	NAME=ZRT_"$ZRT_ENV_SOC_TYPE"_"$ZRT_ENV_SENSOR"_"$ZRT_ENV_BOARD_VERSION"_"$ZRT_ENV_HARDWARE_VERSION"_FW
fi

BIN_NAME="$NAME"_NOR.bin
NAND_BIN_NAME="$NAME"_NAND.bin

if [ $ZRT_ENV_CUSTOM == 70MAI ]; then
    case ${ZRT_ENV_BOARD_VERSION} in
        HM3001)
        if [ ${ZRT_ENV_FLASH_TYPE} = NOR ]; then
		BOOT_OFFSET=0
		BOOT_SIZE_KBYTES=256
		TAG_OFFSET=256
		TAG_SIZE_KBYTES=288
		KERNEL_OFFSET=544
		KERNEL_SIZE_KBYTES=3168
		ROOTFS_OFFSET=3712
		ROOTFS_KBYTES=4416
		ENV_OFFSET=8128
		ENV_SIZE_KBYTES=64
		FLASH_TOTAL_SIZE=8192
	else
		BOOT_OFFSET=0
		BOOT_SIZE_KBYTES=1024
		TAG_OFFSET=1024
		TAG_SIZE_KBYTES=1024
		KERNEL_OFFSET=2048
		KERNEL_SIZE_KBYTES=4096
		ROOTFS_OFFSET=6144
		ROOTFS_KBYTES=5120
		ENV_OFFSET=11264
		ENV_KBYTES=1024
		SYSTEM_OFFSET=12288
		SYSTEM_SIZE_KBYTES=17408
		MODE_A_OFFSET=29696
		MODE_A_SIZE_KBYTES=14336
		MCU_FW_OFFSET=44032
		MCU_FW_SIZE_KBYTES=1024
		SYSTEM_B_OFFSET=45056
		SYSTEM_B_SIZE_KBYTES=17408
		USERFS_OFFSET=62464
		USERFS_SIZE_KBYTES=45056
		MODE_B_OFFSET=107520
		MODE_B_SIZE_KBYTES=14336
		KERNEL_B_OFFSET=121856
		KERNEL_B_SIZE_KBYTES=4096
		ROOTFS_B_OFFSET=125952
		ROOTFS_B_SIZE_KBYTES=5120
		FLASH_TOTAL_SIZE=131072
	fi
            ;;
        *) 
            echo "Unknown board version"
            exit 1
            ;;
    esac
else
	if [ ${ZRT_ENV_FLASH_TYPE} = NOR ]; then
		if [ ${ZRT_ENV_SENSOR_NUM} = two ]; then
			BOOT_OFFSET=0
			BOOT_SIZE_KBYTES=256
			TAG_OFFSET=256
			TAG_SIZE_KBYTES=512
			KERNEL_OFFSET=768
			KERNEL_SIZE_KBYTES=3072
			ROOTFS_OFFSET=3840
			ROOTFS_KBYTES=4096
			RECOVERY_OFFSET=7936
			RECOVERY_KBYTES=3584
			SYSTEM_OFFSET=11520
			SYSTEM_SIZE_KBYTES=4352
			FLASH_TOTAL_SIZE=16384
		else
			BOOT_OFFSET=0
			BOOT_SIZE_KBYTES=256
			TAG_OFFSET=256
			TAG_SIZE_KBYTES=288
			KERNEL_OFFSET=544
			KERNEL_SIZE_KBYTES=3072
			ROOTFS_OFFSET=3616
			ROOTFS_KBYTES=4096
			RECOVERY_OFFSET=7712
			RECOVERY_KBYTES=3584
			SYSTEM_OFFSET=11296
			SYSTEM_SIZE_KBYTES=4576
			FLASH_TOTAL_SIZE=16384
		fi
	else
		if [ ${ZRT_ENV_SENSOR_NUM} = two ]; then
			BOOT_OFFSET=0
			BOOT_SIZE_KBYTES=1024
			TAG_OFFSET=1024
			TAG_SIZE_KBYTES=1024
			KERNEL_OFFSET=2048
			KERNEL_SIZE_KBYTES=3072
			ROOTFS_OFFSET=5120
			ROOTFS_KBYTES=4096
			RECOVERY_OFFSET=9216
			RECOVERY_KBYTES=5120
			SYSTEM_OFFSET=14336
			SYSTEM_SIZE_KBYTES=20480
			CONFIG_OFFSET=34816
			CONFIG_SIZE_KBYTES=8192
			FLASH_TOTAL_SIZE=131072
		else
			BOOT_OFFSET=0
			BOOT_SIZE_KBYTES=1024
			TAG_OFFSET=1024
			TAG_SIZE_KBYTES=1024
			KERNEL_OFFSET=2048
			KERNEL_SIZE_KBYTES=3072
			ROOTFS_OFFSET=5120
			ROOTFS_KBYTES=4096
			RECOVERY_OFFSET=9216
			RECOVERY_KBYTES=5120
			SYSTEM_OFFSET=14336
			SYSTEM_SIZE_KBYTES=20480
			CONFIG_OFFSET=34816
			CONFIG_SIZE_KBYTES=8192
			FLASH_TOTAL_SIZE=131072
		fi
	fi
fi

# check root permission
sudo echo
if [ $? -ne 0 ]; then
    echo "please check root permission."
    exit 1
fi

#check toolchain
${ZRT_ENV_BUILD_DIR}/check_toolchain.sh
if [ $? -ne 0 ]; then
    logger_info error_info "check toolchain error"
    exit 1
fi

#check env
${ZRT_ENV_BUILD_DIR}/check_env.sh
if [ $? -ne 0 ]; then
	echo "check env error"
	exit 1
fi

#find build jobs
ALL_BUILD_JOBS="1"
PACK_OTA="0"
while getopts "j:o" arg
do
	case $arg in
	j)
		ALL_BUILD_JOBS=$OPTARG
		;;
	o)
		PACK_OTA="1"
		;;
	?)
		echo "unkonw argument"
		exit 1
		;;
	esac
done
if [ ${ALL_BUILD_JOBS} = "1" ]; then
	ALL_BUILD_JOBS=${ZRT_ENV_BUILD_JOBS}
fi
echo "build all jobs is ${ALL_BUILD_JOBS}"

#build step by step
if [ -f ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH} ]; then
    echo "Camera boot have already been built"
else
    ${ZRT_ENV_BUILD_DIR}/build_camera_boot.sh
    if [ $? -ne 0 ]; then
	echo "Camera build boot error"
	exit 1
    fi
fi

if [ -f ${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH} ]; then
    echo "Camera tag have already been built"
else
    ${ZRT_ENV_BUILD_DIR}/build_camera_tag.sh
    if [ $? -ne 0 ]; then
		echo "Camera build tag error"
		exit 1
    fi
fi



if [ -f ${ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH} ]; then
    echo "Camera kernel have already been built"
else
    ${ZRT_ENV_BUILD_DIR}/build_camera_uImage.sh -f -j${ALL_BUILD_JOBS}
	if [ $? -ne 0 ]; then
	echo "Camera build kernel error"
	exit 1
    fi
fi


if [ -f ${ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH} ]; then
    echo "Camera rootfs have already been built"
else
	make clean -C 70mai/app/
    ${ZRT_ENV_BUILD_DIR}/build_camera_rootfs.sh -f -c -b
    if [ $? -ne 0 ]; then
	echo "Camera build rootfs error"
	exit 1
    fi
fi

#编译BL616，并且将BL616的OTA包进行拷贝
if [ -f ${ZRT_ENV_OUT_CAMERA_WIFI_OTA_FILE_PATH} ]; then
    logger_info normal_info "70mai BL616 have already been built"
	cd $ZRT_ENV_TOP_DIR/70mai/bl616/customer_app/bl616_demo_sdiowifi/
	cp build_out/FW_OTA.bin.xz.ota ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
	cp build_out/bl616_demo_sdiowifi.bin ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
	cp build_out/bl616_demo_sdiowifi.elf ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
	cp build_out/images/whole_flash_data_MJAC_4M_5* ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
	cp build_out/images/whole_flash_data_MJAC_4M_FAC_5* ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
	cd -
else
    ${ZRT_ENV_BUILD_DIR}/build_70mai_bl616.sh
    if [ $? -ne 0 ]; then
	logger_info normal_info "Camera build 70mai bl616 error"
	exit 1
    fi
fi


#拷贝外机程序N32G031K8
if [ -f ${ZRT_ENV_OUT_CAMERA_MCU_N32G_OTA_FILE_PATH} ]; then
    logger_info normal_info "70mai N32G031K8 have already been built"
else
    ${ZRT_ENV_BUILD_DIR}/build_70mai_N32G031.sh
    mkdir -p ${ZRT_ENV_OUT_CAMERA_DIR}/N32G031K8
    cp ${ZRT_ENV_TOP_DIR}/70mai/app/N32G031K8/out/N32G031K8.bin ${ZRT_ENV_OUT_CAMERA_MCU_N32G_OTA_FILE_PATH}
    cp -rf ${ZRT_ENV_TOP_DIR}/70mai/app/N32G031K8/out/N32G031_BOOT_Project.bin ${ZRT_ENV_OUT_CAMERA_DIR}/N32G031K8/
    cp ${ZRT_ENV_TOP_DIR}/70mai/app/N32G031K8/out/N32G031K8_ALL.bin ${ZRT_ENV_OUT_CAMERA_DIR}/N32G031K8/
    cp ${ZRT_ENV_TOP_DIR}/70mai/app/N32G031K8/out/N32G031K8_ALL_FAC.bin ${ZRT_ENV_OUT_CAMERA_DIR}/N32G031K8/
fi

if [ -f ${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} ]; then
    echo "Camera system have already been built"
else
    ${ZRT_ENV_BUILD_DIR}/build_camera_system.sh -f
    if [ $? -ne 0 ]; then
	echo "Camera build system error"
	exit 1
    fi
fi

#70mai不编译出recovery分区
if [ $ZRT_ENV_CUSTOM == 70MAI ]; then
    logger_info normal_info "Recovery partition is not required"
else
	if [ -f ${ZRT_ENV_OUT_CAMERA_RECOVERY_FILE_PATH} ]; then
		echo "Camera recovery have already been built"
	else
		${ZRT_ENV_BUILD_DIR}/build_camera_recovery.sh -f -j${ALL_BUILD_JOBS}
		if [ $? -ne 0 ]; then
	 	echo "Camera build recovery error"
	 	exit 1
		fi
	fi
fi

#70mai生成相关env_factory.bin
if [ $ZRT_ENV_CUSTOM == 70MAI ]; then
    if [ -f ${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} ]; then
        logger_info normal_info "Camera ENV have already been built"
    else
        ${ZRT_ENV_BUILD_DIR}/build_70mai_env.sh
        if [ $? -ne 0 ]; then
            logger_info normal_info "Camera Generating ENV error"
        fi
    fi
elif [ ${ZRT_ENV_FLASH_TYPE} = NOR ]; then
	echo "Nor does not require build config"
else
	if [ -f ${ZRT_ENV_OUT_CAMERA_CONFIG_FILE_PATH} ]; then
		echo "Camera config have already been built"
	else
		${ZRT_ENV_BUILD_DIR}/build_camera_config.sh
		if [ $? -ne 0 ]; then
		echo "Camera build config error"
		exit 1
		fi
	fi
fi

if [ $ZRT_ENV_CUSTOM == 70MAI ]; then
	# pack each part
	mkdir -p ${ZRT_ENV_OUT_DIR}
	case ${ZRT_ENV_BOARD_VERSION} in
        HM3001)		
            case ${ZRT_ENV_HARDWARE_VERSION} in
                a1_1)
					#nor + nand 
					if [ $PACK_OTA == "1" ];then
						rm -rf ${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed
						${ZRT_ENV_BUILD_DIR}/sinature_70mai/sinature_70mai.sh ${ZRT_ENV_BUILD_DIR}/sinature_70mai/input_source/rsa_private_key.pem ${ZRT_ENV_OUT_CAMERA_DIR} 
						if [ -f ${ZRT_ENV_OUT_CAMERA_DIR}/u-boot-with-spl-signed.bin ] && 
							[ -f ${ZRT_ENV_OUT_CAMERA_DIR}/uImage.lzo-signed ] && 
							[ -f ${ZRT_ENV_OUT_CAMERA_DIR}/rootfs_camera.cpio.lzo-signed ]; then
							tr '\000' '\377' < /dev/zero | dd of=${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed bs=1K count=${FLASH_TOTAL_SIZE} > /dev/null

							dd if=${ZRT_ENV_OUT_CAMERA_DIR}/u-boot-with-spl-signed.bin of=${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed bs=1K seek=${BOOT_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed bs=1K seek=${TAG_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_DIR}/uImage.lzo-signed of=${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed bs=1K seek=${KERNEL_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_DIR}/rootfs_camera.cpio.lzo-signed of=${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed bs=1K seek=${ROOTFS_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed bs=1K seek=${ENV_OFFSET}

							logger_info normal_info "Generate Camera firmware at: "${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed
						else
							logger_info error_info "Generate Camera firmware at: "${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed" failure"
						fi
					fi
                    tr '\000' '\377' < /dev/zero | dd of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K count=${FLASH_TOTAL_SIZE} > /dev/null

					dd if=${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${BOOT_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${TAG_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${KERNEL_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${ROOTFS_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${ENV_OFFSET}

					logger_info normal_info "Generate Camera firmware at: "${ZRT_ENV_OUT_DIR}/${BIN_NAME}
					#NAND BIN
					tr '\000' '\377' < /dev/zero | dd of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1K count=128 > /dev/null

					#dd if=${ZRT_ENV_OUT_CAMERA_DIR}/mcu.bin of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}
					dd if=${ZRT_ENV_OUT_CAMERA_MCU_N32G_OTA_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}
					dd if=${ZRT_ENV_TOP_DIR}/70mai/configs/model/t40n_cateye2_${ZRT_ENV_AI_ALG}.bin of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1M seek=1
					dd if=${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}  bs=1M seek=16
					dd if=${ZRT_ENV_OUT_CAMERA_DIR}/userfs.bin of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}  bs=1M seek=36

					logger_info normal_info "Generate Camera firmware at: "${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}
                ;;
                b1_1)
					#nand
					BOOT_OFFSET=0
					BOOT_SIZE_KBYTES=1024
					TAG_OFFSET=$((BOOT_SIZE_KBYTES+BOOT_OFFSET))
					TAG_SIZE_KBYTES=1024
					ENV_OFFSET=$((TAG_SIZE_KBYTES+TAG_OFFSET))
					ENV_KBYTES=1024
					KERNEL_OFFSET=$((ENV_KBYTES+ENV_OFFSET))
					KERNEL_SIZE_KBYTES=$((4*1024))
					ROOTFS_OFFSET=$((KERNEL_SIZE_KBYTES+KERNEL_OFFSET))
					ROOTFS_KBYTES=$((5*1024))
					SYSTEM_OFFSET=$((ROOTFS_KBYTES+ROOTFS_OFFSET))
					SYSTEM_SIZE_KBYTES=$((15*1024))
					MODE_A_OFFSET=$((SYSTEM_SIZE_KBYTES+SYSTEM_OFFSET))
					MODE_A_SIZE_KBYTES=$((14*1024))
					USERFS_OFFSET=$((MODE_A_SIZE_KBYTES+MODE_A_OFFSET))
					USERFS_SIZE_KBYTES=$((49*1024))
					KERNEL_B_OFFSET=$((USERFS_SIZE_KBYTES+USERFS_OFFSET))
					KERNEL_B_SIZE_KBYTES=$((4*1024))
					ROOTFS_B_OFFSET=$((KERNEL_B_SIZE_KBYTES+KERNEL_B_OFFSET))
					ROOTFS_B_SIZE_KBYTES=$((5*1024))
					SYSTEM_B_OFFSET=$((ROOTFS_B_SIZE_KBYTES+ROOTFS_B_OFFSET))
					SYSTEM_B_SIZE_KBYTES=$((15*1024))
					MODE_B_OFFSET=$((SYSTEM_B_SIZE_KBYTES+SYSTEM_B_OFFSET))
					MODE_B_SIZE_KBYTES=$((14*1024))
					FLASH_TOTAL_SIZE=$((MODE_B_SIZE_KBYTES+MODE_B_OFFSET))

					if [ $FLASH_TOTAL_SIZE != $((128*1024)) ];then
						logger_info error_info "Partition error"
						exit 1
					fi

					file_len=`stat -c "%s" ${ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH}`
					if [ $((KERNEL_SIZE_KBYTES*1024)) -le $file_len ];then
						logger_info error_info "uImage too large"
						exit 1
					fi

					file_len=`stat -c "%s" ${ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH}`
					if [ $((ROOTFS_KBYTES*1024)) -le $file_len ];then
						logger_info error_info "rootfs too large"
						exit 1
					fi

					file_len=`stat -c "%s" ${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH}`
					if [ $((SYSTEM_SIZE_KBYTES*1024)) -le $file_len ];then
						logger_info error_info "system too large"
						exit 1
					fi

					if [ $PACK_OTA == "1" ];then
						rm -rf ${ZRT_ENV_OUT_DIR}/${BIN_NAME}-signed
						${ZRT_ENV_BUILD_DIR}/sinature_70mai/sinature_70mai.sh ${ZRT_ENV_BUILD_DIR}/sinature_70mai/input_source/rsa_private_key.pem ${ZRT_ENV_OUT_CAMERA_DIR}
						if [ -f ${ZRT_ENV_OUT_CAMERA_DIR}/u-boot-with-spl-signed.bin ] && 
							[ -f ${ZRT_ENV_OUT_CAMERA_DIR}/uImage.lzo-signed ] && 
							[ -f ${ZRT_ENV_OUT_CAMERA_DIR}/rootfs_camera.cpio.lzo-signed ]; then
							tr '\000' '\377' < /dev/zero | dd of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed bs=1K count=${FLASH_TOTAL_SIZE} > /dev/null

							dd if=${ZRT_ENV_OUT_CAMERA_DIR}/u-boot-with-spl-signed.bin of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed bs=1K seek=${BOOT_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed bs=1K seek=${TAG_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed bs=1K seek=${ENV_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_DIR}/uImage.lzo-signed of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed bs=1K seek=${KERNEL_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_DIR}/rootfs_camera.cpio.lzo-signed of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed bs=1K seek=${ROOTFS_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed bs=1K seek=${SYSTEM_OFFSET}
							dd if=${ZRT_ENV_TOP_DIR}/70mai/configs/model/t40n_cateye2_${ZRT_ENV_AI_ALG}.bin of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed bs=1K seek=${MODE_A_OFFSET}
							dd if=${ZRT_ENV_OUT_CAMERA_DIR}/userfs.bin of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed  bs=1K seek=${USERFS_OFFSET}

							logger_info normal_info "Generate Camera firmware at: "${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed
						else
							logger_info error_info "Generate Camera firmware at: "${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}-signed" failure"
						fi
					fi
                    tr '\000' '\377' < /dev/zero | dd of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1K count=128 > /dev/null

					dd if=${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1K seek=${BOOT_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1K seek=${TAG_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1K seek=${ENV_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1K seek=${KERNEL_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1K seek=${ROOTFS_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1K seek=${SYSTEM_OFFSET}
					dd if=${ZRT_ENV_TOP_DIR}/70mai/configs/model/t40n_cateye2_${ZRT_ENV_AI_ALG}.bin of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME} bs=1K seek=${MODE_A_OFFSET}
					dd if=${ZRT_ENV_OUT_CAMERA_DIR}/userfs.bin of=${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}  bs=1K seek=${USERFS_OFFSET}
				
					echo "Generate Camera firmware at: "${ZRT_ENV_OUT_DIR}/${NAND_BIN_NAME}
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
	if [ $PACK_OTA == "1" ];then
		# 打包ota升级包
		rm -rf ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp
		mkdir -p ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp
		cp ${ZRT_ENV_TOP_DIR}/70mai/configs/version ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp
		cp ${ZRT_ENV_TOP_DIR}/70mai/configs/model/t40n_cateye2_${ZRT_ENV_AI_ALG}.bin ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp/t40-5block.bin
		cp ${ZRT_ENV_OUT_CAMERA_DIR}/uImage.lzo-signed ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp/ota-uImage.bin-signed
		cp ${ZRT_ENV_OUT_CAMERA_DIR}/rootfs_camera.cpio.lzo-signed ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp/ota-rootfs.bin-signed
		cp ${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp/system.bin
		cp $ZRT_ENV_OUT_CAMERA_WIFI_OTA_FILE_PATH ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp/FW_OTA.bin.xz.ota
		cp $ZRT_ENV_OUT_CAMERA_MCU_N32G_OTA_FILE_PATH ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp/N32G031K8.bin
		cp ${ZRT_ENV_OUT_CAMERA_DIR}/${ZRT_ENV_SENSOR}_t40_ota.bin ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp/${ZRT_ENV_SENSOR}_t40.bin -rf
		cp ${ZRT_ENV_OUT_CAMERA_DIR}/ae_table_${ZRT_ENV_SENSOR}_ota.bin ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp/ae_table_${ZRT_ENV_SENSOR}_ota.bin -rf
		cp ${ZRT_ENV_OUT_CAMERA_DIR}/${ZRT_ENV_SENSOR}_t40_init_ota.bin ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp/${ZRT_ENV_SENSOR}_t40_init_ota.bin -rf
		${ZRT_ENV_TOP_DIR}/70mai/app/tools/fw_pack/ipc_ota_fw_pack ${ZRT_ENV_OUT_CAMERA_DIR}/ota_tmp ${ZRT_ENV_OUT_CAMERA_DIR}/ota.bin

		logger_info normal_info "Generate Camera OTA firmware at: "${ZRT_ENV_OUT_CAMERA_DIR}/ota.bin
	fi
else
	# pack each part
	mkdir -p ${ZRT_ENV_OUT_DIR}
	tr '\000' '\377' < /dev/zero | dd of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K count=${FLASH_TOTAL_SIZE} > /dev/null

	dd if=${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${BOOT_OFFSET}
	dd if=${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${TAG_OFFSET}
	dd if=${ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${KERNEL_OFFSET}
	dd if=${ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${ROOTFS_OFFSET}
	dd if=${ZRT_ENV_OUT_CAMERA_RECOVERY_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${RECOVERY_OFFSET}
	dd if=${ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${SYSTEM_OFFSET}
	if [ ${ZRT_ENV_FLASH_TYPE} = NOR ]; then
		echo "Nor does not require dd config"
	else
		dd if=${ZRT_ENV_OUT_CAMERA_CONFIG_FILE_PATH} of=${ZRT_ENV_OUT_DIR}/${BIN_NAME} bs=1K seek=${CONFIG_OFFSET}
	fi
fi

