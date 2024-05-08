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

echo -e "*\n*Start build camera boot\n*"

#第一阶段的uboot, 由君正开发，并且固定,代码不开源，生成的bin文件路径是：os/uboot/bin
BOOT_PATH=${ZRT_ENV_OS_DIR}/uboot/bin
UBOOT_PATH=${ZRT_ENV_OS_DIR}/uboot/u-boot/u-boot.bin

if [ $ZRT_ENV_CUSTOM == 70MAI ] ; then
	case ${ZRT_ENV_HARDWARE_VERSION} in
		a1_1)
			# BOOT_FILE=${BOOT_PATH}/boot_${ZRT_ENV_SOC_TYPE}_${ZRT_ENV_FLASH_TYPE}_${ZRT_ENV_BOARD_VERSION}.bin
			# #	if [ ! -f ${ZRT_ENV_OUT_CAMERA_DIR}/u-boot.bin ]; then
			# 		echo "hm3001_t40n_nor"
			# 		cd ${ZRT_ENV_OS_DIR}/uboot/u-boot/
			# 		make clean
			# 		#make hm3001_t40n_nor > /dev/null 2>&1
			# 		make hm3001_t40n_nor
			# 		cp ${UBOOT_PATH} ${ZRT_ENV_OUT_CAMERA_DIR}/u-boot.bin
			# 		cd -
			# #	fi
			# if [ -f ${BOOT_FILE} ]; then
			# 	#cp ${BOOT_FILE} ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH}
			# 	#将自己编译出来的第二阶段uboot  pad到boot.bin中
			# 	./build/pad_camera_u-boot.sh -i ${ZRT_ENV_OUT_CAMERA_DIR}/u-boot.bin -o ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH} -b ${BOOT_FILE}

			# 	echo "ZRT_ENV_OUT_CAMERA_DIR=$ZRT_ENV_OUT_CAMERA_DIR"

			# 	echo -e "*\n*Generate Camera boot: ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH}\n*"
			# 	exit 0

			# else
			# 	echo "error: "${BOOT_FILE}" is not found"
			# 	exit 1
			# fi
			cp $BOOT_PATH/boot_SOC_T40N_NOR_HM3001.bin ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH}
			cp $BOOT_PATH/boot_SOC_T40N_NOR_HM3001.bin ${ZRT_ENV_OUT_CAMERA_DIR}/uboot_scboot.bin
			echo -e "*\n*Generate Camera boot: ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH}\n*"
		;;
		b1_1)
			cp $BOOT_PATH/boot_SOC_T40N_NAND_HM3001.bin ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH}
			if [ $ZRT_ENV_IS_HASH == 1 ]; then
				echo aaa
				cp $BOOT_PATH/boot_SOC_T40N_NAND_HM3001_SCBOOT_HASH.bin ${ZRT_ENV_OUT_CAMERA_DIR}/uboot_scboot.bin
			else
				cp $BOOT_PATH/boot_SOC_T40N_NAND_HM3001_SCBOOT.bin ${ZRT_ENV_OUT_CAMERA_DIR}/uboot_scboot.bin
			fi
			echo -e "*\n*Generate Camera boot: ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH}\n*"
		;;
		*) 
			echo "Unknown hardware version"
			exit 1
		;;
	esac
else

	if [ ${ZRT_ENV_SENSOR_NUM} = two ]; then
		BOOT_FILE=${BOOT_PATH}/boot_${ZRT_ENV_SOC_TYPE}_${ZRT_ENV_FLASH_TYPE}_2SENSOR_${ZRT_ENV_BOARD_VERSION}.bin
	else
		BOOT_FILE=${BOOT_PATH}/boot_${ZRT_ENV_SOC_TYPE}_${ZRT_ENV_FLASH_TYPE}_${ZRT_ENV_BOARD_VERSION}.bin
	#	if [ ! -f ${ZRT_ENV_OUT_CAMERA_DIR}/u-boot.bin ]; then
			echo "hm3001_t40n_nor"
			cd ${ZRT_ENV_OS_DIR}/uboot/u-boot/
			make clean
			#make hm3001_t40n_nor > /dev/null 2>&1
			make hm3001_t40n_nor
			cp ${UBOOT_PATH} ${ZRT_ENV_OUT_CAMERA_DIR}/u-boot.bin
			cd -
	#	fi
	fi

	if [ -f ${BOOT_FILE} ]; then
		#cp ${BOOT_FILE} ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH}
		#将自己编译出来的第二阶段uboot  pad到boot.bin中
		./build/pad_camera_u-boot.sh -i ${ZRT_ENV_OUT_CAMERA_DIR}/u-boot.bin -o ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH} -b ${BOOT_FILE}

		echo "ZRT_ENV_OUT_CAMERA_DIR=$ZRT_ENV_OUT_CAMERA_DIR"

		echo -e "*\n*Generate Camera boot: ${ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH}\n*"
		exit 0

	else
		echo "error: "${BOOT_FILE}" is not found"
		exit 1
	fi

fi

