#!/bin/bash

if [ -f .env ]; then
    source .env
	source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project"
    exit 1
fi

echo "build camera tag:"

SOC_TYPE=t40
TAG_PATH=${ZRT_ENV_TOOLS_DIR}/make_tag/tag_generator/
SRC_FILE=${TAG_PATH}/tag.bin
BUILD_FILE=${TAG_PATH}/build_tag_${SOC_TYPE}.sh


if [ -f ${BUILD_FILE} ]; then
    mkdir -p ${ZRT_ENV_OUT_CAMERA_DIR}
	cd ${TAG_PATH}
	${BUILD_FILE}
	if [ $? != 0 ]; then
		echo "error: build tag failed"
		exit 1
	fi

    cp ${SRC_FILE} ${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH}

	echo
	echo "==============================================================="
    echo "dump "${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH}":"
    
if [ ${ZRT_ENV_SENSOR_NUM} = two ]; then
	./tag_generator_${SOC_TYPE}_2sensor_host -d ${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH}
else
	./tag_generator_${SOC_TYPE}_host -d ${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH}
fi
	cp ${ZRT_ENV_TOP_DIR}/70mai/app/tools/iq_ota_firmware/t40n_get_iqInfo ${ZRT_ENV_OUT_CAMERA_DIR}
	cd ${ZRT_ENV_OUT_CAMERA_DIR}
	./t40n_get_iqInfo
	cd -

    echo "Generate Camera tag: "${ZRT_ENV_OUT_CAMERA_TAG_FILE_PATH}
    exit 0
else
    echo "error: "${TAG_FILE}" is not found"
    exit 1
fi
