function print_build_type()
{
    echo "Build types:"
    echo "  [1] release"
    echo "  [2] debug"
    echo
}

function print_build_soc_type()
{
	echo "Supproted SOC Types:"
	echo "  [1] SOC_T40XP"
	echo "  [2] SOC_T40N"
}

function print_build_flash_type()
{
	echo "Supproted SOC Types:"
	echo "  [1] NOR"
	echo "  [2] NAND"
}

function print_build_cpu_num()
{
	echo "Supproted cpu nums:"
	echo "  [1] one"
	echo "  [2] two"
}

function print_build_sensor_num()
{
	echo "Supproted sensor nums:"
	echo "  [1] one"
	echo "  [2] two"
}

function print_sensors()
{
    echo "Supproted sensors:"
    echo "  [1] jxf37"
    echo "  [2] gc2063"
    echo "  [3] sc3235"
    echo "  [4] jxf37s1"
    echo "  [5] sc8238"
    echo "  [6] sc5235"
    echo "  [7] sc201cs"
    echo "  [8] imx327"
    echo "  [9] gc4653"
    echo "  [10] sc402ai"
    echo "  [11] sc530ai"
    echo "  [12] sc301IoT"
    echo
}

function print_sensor1s()
{
    echo "Supproted sensor1s:"
    echo "  [1] jxf37"
    echo "  [2] gc2063"
    echo "  [3] sc3235"
    echo "  [4] jxf37s1"
    echo "  [5] sc8238"
    echo "  [6] sc5235"
    echo "  [7] sc201cs"
    echo "  [8] imx327"
    echo "  [9] gc4653"
    echo "  [10] sc402ai"
    echo "  [11] sc530ai"
    echo "  [12] sc301IoT"
    echo
}

function check_t40_branch()
{
	# os/uboot
	cd ${ZRT_ENV_TOP_DIR}/os/uboot/

	CAMERA_CUR_BRANCH=$(git branch -vv | grep "*" | grep "zeratul/zeratul_t40" | awk '{print $4}')
	if [[ "${CAMERA_CUR_BRANCH}" == [zeratul/zeratul_t40* ]]; then
		echo "os/uboot branch is right"
	else
		echo -e "\033[31m os/uboot invalid branch \033[0m"
		git branch -vv
		if [[ -n $(git diff --stat)  ]]
		then
			echo -e "\033[31m This branch is dirty, Please modify to zeratul_t40 branch it manually. \033[0m"
			git status
		else
			git branch -D zeratul_t40 > /dev/null 2>&1
			git checkout remotes/zeratul/zeratul_t40 -b zeratul_t40
		fi
	fi

	# os/drivers
	cd ${ZRT_ENV_TOP_DIR}/os/drivers/

	CAMERA_CUR_BRANCH=$(git branch -vv | grep "*" | grep "zeratul/zeratul_t40" | awk '{print $4}')
	if [[ "${CAMERA_CUR_BRANCH}" == [zeratul/zeratul_t40* ]]; then
		echo "os/drivers branch is right"
	else
		echo -e "\033[31m os/drivers invalid branch \033[0m"
		git branch -vv
		if [[ -n $(git diff --stat)  ]]
		then
			echo -e "\033[31m This branch is dirty, Please modify to zeratul_t40 branch it manually. \033[0m"
			git status
		else
			git branch -D zeratul_t40 > /dev/null 2>&1
			git checkout remotes/zeratul/zeratul_t40 -b zeratul_t40
		fi
	fi

	# Kernel
	cd ${ZRT_ENV_TOP_DIR}/os/kernel/

	KERNEL_CUR_BRANCH=$(git branch -vv | grep "*" | grep "zeratul/zrt_t40" | awk '{print $4}')
	if [[ "${KERNEL_CUR_BRANCH}" == [zeratul/zrt_t40* ]]; then
		echo "Kernel branch is right"
	else
		echo -e "\033[31m Kernel invalid branch \033[0m"
		git branch -vv
		if [[ -n $(git diff --stat)  ]]
		then
			echo -e "\033[31m This branch is dirty, Please modify to zrt_t40 branch it manually. \033[0m"
			git status
		else
			git branch -D zrt_t40 > /dev/null 2>&1
			git checkout remotes/zeratul/zrt_t40 -b zrt_t40
		fi
	fi

	# os/rootfs
	cd ${ZRT_ENV_TOP_DIR}/os/rootfs/

	CAMERA_CUR_BRANCH=$(git branch -vv | grep "*" | grep "zeratul/zeratul_t40" | awk '{print $4}')
	if [[ "${CAMERA_CUR_BRANCH}" == [zeratul/zeratul_t40* ]]; then
		echo "os/rootfs branch is right"
	else
		echo -e "\033[31m os/rootfs invalid branch \033[0m"
		git branch -vv
		if [[ -n $(git diff --stat)  ]]
		then
			echo -e "\033[31m This branch is dirty, Please modify to zeratul_t40 branch it manually. \033[0m"
			git status
		else
			git branch -D zeratul_t40 > /dev/null 2>&1
			git checkout remotes/zeratul/zeratul_t40 -b zeratul_t40
		fi
	fi

	# release
	cd ${ZRT_ENV_TOP_DIR}/release/

	CAMERA_CUR_BRANCH=$(git branch -vv | grep "*" | grep "zeratul/zeratul_t40" | awk '{print $4}')
	if [[ "${CAMERA_CUR_BRANCH}" == [zeratul/zeratul_t40* ]]; then
		echo "os/rootfs branch is right"
	else
		echo -e "\033[31m release invalid branch \033[0m"
		git branch -vv
		if [[ -n $(git diff --stat)  ]]
		then
			echo -e "\033[31m This branch is dirty, Please modify to zeratul_t40 branch it manually. \033[0m"
			git status
		else
			git branch -D zeratul_t40 > /dev/null 2>&1
			git checkout remotes/zeratul/zeratul_t40 -b zeratul_t40
		fi
	fi

	cd ${ZRT_ENV_TOP_DIR}
}

if [ -f build/env_setup_zeratul.sh ]; then
    echo "Setup Zeratul build environment(devp)":

    JOBS=""
    BUILD_TYPE=""
    VERSION=""
	SOC_TYPE=""
	FLASH_TYPE=""
	SENSOR_NUM=""
    SENSOR=""
    SENSOR1=""
    FLASH=""
    TRANSFER_MODE=""
    PRODUCT_MODE=""
	SUIT_RELAY_EN=""
    STATION_AP_MODE=""
    WIFI_CHIP=""
	FB_LCD=""
	BOARD_VERSION=""
    TOOL_CHAIN="7.2.0"

    while getopts "j:s:" arg
    do
	case $arg in
	j)
		JOBS=$OPTARG
		;;
	s)
		SENSOR=$OPTARG
		;;
	?)
		echo "unkonw argument"
		exit 1
		;;
	esac
    done

    if [ "${JOBS}" = "" ] ; then
	echo -n "please enter the num of jobs(build threads)(defalt 1): "
	read JOBS
	if [ "${JOBS}" = "" ] ; then
	    JOBS=1
	fi
    fi

    if [ "${BUILD_TYPE}" = "" ] ; then
	print_build_type
	echo -n "please enter the system build type(defalt 1): "
	read BUILD_TYPE
	if [ "${BUILD_TYPE}" = "1" ] ; then
	    BUILD_TYPE=release
	elif [ "${BUILD_TYPE}" = "2" ] ; then
	    BUILD_TYPE=debug
	else
	    BUILD_TYPE=release ##default
	fi
    fi

    if [ "${VERSION}" = "" ] ; then
	echo -n "please enter the FW sub version(pad to the end of the version string), or leave this empty as default: "
	read VERSION
	if [ ! "${VERSION}" = "" ] ; then
	    VERSION=${VERSION}
	fi
    fi

	if [ "${SOC_TYPE}" = "" ] ; then
	print_build_soc_type
	echo -n "please enter the SOC type index(defalt 1): "
	read SOC_TYPE
	if [ "${SOC_TYPE}" = "1" ] ; then
	    SOC_TYPE=SOC_T40XP
		BOARD_VERSION=V1
		TRANSFER_MODE=IIC
	elif [ "${SOC_TYPE}" = "2" ] ; then
	    SOC_TYPE=SOC_T40N
		BOARD_VERSION=V1
		TRANSFER_MODE=IIC
	else
	    SOC_TYPE=SOC_T40XP ##default
		BOARD_VERSION=V1
		TRANSFER_MODE=IIC
	fi
    fi

	if [ "${FLASH_TYPE}" = "" ] ; then
	print_build_flash_type
	echo -n "please enter the flash type index(defalt 1): "
	read FLASH_TYPE
	if [ "${FLASH_TYPE}" = "1" ] ; then
	    FLASH_TYPE=NOR
	elif [ "${FLASH_TYPE}" = "2" ] ; then
	    FLASH_TYPE=NAND
	else
	    FLASH_TYPE=NOR ##default
	fi
    fi

	if [ "${SENSOR_NUM}" = "" ] ; then
	print_build_sensor_num
	echo -n "please enter the SENSOR num(defalt 1): "
	read SENSOR_NUM
	if [ "${SENSOR_NUM}" = "1" ] ; then
		SENSOR_NUM=one
		if [ "${SENSOR}" = "" ] ; then
			print_sensors
			echo -n "please enter the Camera Sensor index(defalt 1): "
			read SENSOR
			if [ "${SENSOR}" = "1" ] ; then
				SENSOR=jxf37
			elif [ "${SENSOR}" = "2" ] ; then
				SENSOR=gc2063
			elif [ "${SENSOR}" = "3" ] ; then
				SENSOR=sc3235
			elif [ "${SENSOR}" = "4" ] ; then
				SENSOR=jxf37s1
			elif [ "${SENSOR}" = "5" ] ; then
				SENSOR=sc8238
			elif [ "${SENSOR}" = "6" ] ; then
				SENSOR=sc5235
			elif [ "${SENSOR}" = "7" ] ; then
				SENSOR=sc201cs
			elif [ "${SENSOR}" = "8" ] ; then
				SENSOR=imx327
			elif [ "${SENSOR}" = "9" ] ; then
				SENSOR=gc4653
			elif [ "${SENSOR}" = "10" ] ; then
				SENSOR=sc402ai
			elif [ "${SENSOR}" = "11" ] ; then
				SENSOR=sc530ai
			elif [ "${SENSOR}" = "12" ] ; then
				SENSOR=sc301IoT
			else
				SENSOR=jxf37 ##default
			fi
		fi
	elif [ "${SENSOR_NUM}" = "2" ] ; then
		SENSOR_NUM=two
		if [ "${SENSOR}" = "" ] ; then
			print_sensors
			echo -n "please enter the Camera Main Sensor index(defalt 1): "
			read SENSOR
			if [ "${SENSOR}" = "1" ] ; then
				SENSOR=jxf37
			elif [ "${SENSOR}" = "2" ] ; then
				SENSOR=gc2063
			elif [ "${SENSOR}" = "3" ] ; then
				SENSOR=sc3235
			elif [ "${SENSOR}" = "4" ] ; then
				SENSOR=jxf37s1
			elif [ "${SENSOR}" = "5" ] ; then
				SENSOR=sc8238
			elif [ "${SENSOR}" = "6" ] ; then
				SENSOR=sc5235
			elif [ "${SENSOR}" = "7" ] ; then
				SENSOR=sc201cs
			elif [ "${SENSOR}" = "8" ] ; then
				SENSOR=imx327
			elif [ "${SENSOR}" = "9" ] ; then
				SENSOR=gc4653
			elif [ "${SENSOR}" = "10" ] ; then
				SENSOR=sc402ai
			elif [ "${SENSOR}" = "11" ] ; then
				SENSOR=sc530ai
			elif [ "${SENSOR}" = "12" ] ; then
				SENSOR=sc301IoT
			else
				SENSOR=jxf37 ##default
			fi
		fi
		
		if [ "${SENSOR1}" = "" ] ; then
			print_sensor1s
			echo -n "please enter the Camera Sencond Sensor index(defalt 1): "
			read SENSOR1
			if [ "${SENSOR1}" = "1" ] ; then
				SENSOR1=jxf37
			elif [ "${SENSOR1}" = "2" ] ; then
				SENSOR1=gc2063
			elif [ "${SENSOR1}" = "3" ] ; then
				SENSOR1=sc3235
			elif [ "${SENSOR1}" = "4" ] ; then
				SENSOR1=jxf37s1
			elif [ "${SENSOR1}" = "5" ] ; then
				SENSOR1=sc8238
			elif [ "${SENSOR1}" = "6" ] ; then
				SENSOR1=sc5253
			elif [ "${SENSOR1}" = "7" ] ; then
				SENSOR1=sc201cs
			elif [ "${SENSOR1}" = "8" ] ; then
				SENSOR1=imx327
			elif [ "${SENSOR1}" = "9" ] ; then
				SENSOR1=gc4653
			elif [ "${SENSOR1}" = "10" ] ; then
				SENSOR1=sc402ai
			else
				SENSOR1=jxf37 ##default
			fi
		fi
	else
		SENSOR_NUM=one
	    if [ "${SENSOR}" = "" ] ; then
			print_sensors
			echo -n "please enter the Camera Sensor index(defalt 1): "
			read SENSOR
			if [ "${SENSOR}" = "1" ] ; then
				SENSOR=jxf37
			elif [ "${SENSOR}" = "2" ] ; then
				SENSOR=gc2063
			elif [ "${SENSOR}" = "3" ] ; then
				SENSOR=sc3235
			elif [ "${SENSOR}" = "4" ] ; then
				SENSOR=jxf37s1
			elif [ "${SENSOR}" = "5" ] ; then
				SENSOR=sc8238
			elif [ "${SENSOR}" = "6" ] ; then
				SENSOR=sc5235
			elif [ "${SENSOR}" = "7" ] ; then
				SENSOR=sc201cs
			elif [ "${SENSOR}" = "8" ] ; then
				SENSOR=imx327
			elif [ "${SENSOR}" = "9" ] ; then
				SENSOR=gc4653
			elif [ "${SENSOR}" = "10" ] ; then
				SENSOR=sc402ai
			elif [ "${SENSOR}" = "11" ] ; then
				SENSOR=sc530ai
			elif [ "${SENSOR}" = "12" ] ; then
				SENSOR=sc301IoT
			else
				SENSOR=jxf37 ##default
			fi
		fi
	fi
    fi

#    if [ "${SENSOR}" = "" ] ; then
#	print_sensors
#	echo -n "please enter the Camera Sensor index(defalt 1): "
#	read SENSOR
#	if [ "${SENSOR}" = "1" ] ; then
#	    SENSOR=jxf37
#	else
#	    SENSOR=jxf37 ##default
#	fi
#    fi
#
	if [ "${SENSOR_NUM}" = "1" ] ; then
		FW_VERSION="\"ZRT_${BUILD_TYPE}_`date +%Y%m%d%H%M`_`whoami`_`hostname`;[VERSION];ver=CAMERA_${SOC_TYPE}_${SENSOR}_V${VERSION};\""
	else
		FW_VERSION="\"ZRT_${BUILD_TYPE}_`date +%Y%m%d%H%M`_`whoami`_`hostname`;[VERSION];ver=CAMERA_${SOC_TYPE}_${SENSOR}_${SENSOR1}_V${VERSION};\""
	fi

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
	export ZRT_ENV_BUILD_DIR=${ZRT_ENV_TOP_DIR}/build
	export ZRT_ENV_OUT_DIR=${ZRT_ENV_TOP_DIR}/out
    export ZRT_ENV_DRIVERS_DIR=${ZRT_ENV_TOP_DIR}/os/drivers
    export ZRT_ENV_USERLAND_DIR=${ZRT_ENV_TOP_DIR}/userland
    export ZRT_ENV_TOOLS_DIR=${ZRT_ENV_TOP_DIR}/tools
    export ZRT_ENV_OUT_CAMERA_DIR=${ZRT_ENV_OUT_DIR}/camera
    export ZRT_ENV_OUT_CAMERA_BOOT_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/boot.bin
    export ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/tag.bin
    export ZRT_ENV_OUT_CAMERA_KERNEL_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/uImage.lzo
    export ZRT_ENV_OUT_CAMERA_ROOTFS_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/rootfs_camera.cpio.lzo
    export ZRT_ENV_OUT_CAMERA_RECOVERY_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/recovery.bin
    export ZRT_ENV_OUT_CAMERA_SYSTEM_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/system.bin
    export ZRT_ENV_OUT_CAMERA_CONFIG_FILE_PATH=${ZRT_ENV_OUT_CAMERA_DIR}/config.bin

    echo ============================================================

	echo ZRT_ENV_FW_VERSION=${ZRT_ENV_FW_VERSION} > .env
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

    cat .env

    echo ============================================================

else

    echo "Please run source at top of isvp"
fi
