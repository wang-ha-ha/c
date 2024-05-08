#!/bin/bash

if [ -f .env ]; then
    source .env
	source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project"
    exit 1
fi

if [ ! -d ${ZRT_ENV_OUT_CAMERA_DIR}/BL616 ]; then
	mkdir -p ${ZRT_ENV_OUT_CAMERA_DIR}/BL616
fi

echo -e "*\n*Start build camera bl616\n*"


BL616_MAKEFILE=$ZRT_ENV_TOP_DIR/70mai/bl616/customer_app/bl616_demo_sdiowifi/genlp

if [ -f $BL616_MAKEFILE ]; then
	cd $ZRT_ENV_TOP_DIR/70mai/bl616/customer_app/bl616_demo_sdiowifi/
else
    logger_info error_info "Makefile not found"
    exit 1
fi

./genlp mjac
cp build_out/FW_OTA.bin.xz.ota ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
cp build_out/bl616_demo_sdiowifi.bin ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
cp build_out/bl616_demo_sdiowifi.elf ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
cp build_out/images/whole_flash_data_MJAC_4M_5* ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
cp build_out/images/whole_flash_data_MJAC_4M_FAC_5* ${ZRT_ENV_OUT_CAMERA_DIR}/BL616/ -rf
cd -
