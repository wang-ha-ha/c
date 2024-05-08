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

echo -e "*\n*Start compiling config\n*"

cd ${ZRT_ENV_OUT_CAMERA_DIR}

# generate camera config
rm -rf ./config
rm -f ${ZRT_ENV_OUT_CAMERA_CONFIG_FILE_PATH}
mkdir config
if [ ${ZRT_ENV_FLASH_TYPE} = NOR ]; then
	echo "nor none"
else
	if [ -f config.ubifs ]; then
		rm -f config.ubifs
	fi
	mkfs.ubifs -e 0x1f000 -c 64 -m 2048 -d config -o config.ubifs -v
	ls -l config.ubifs

	if [ -f config_ubinize.cfg ]; then
		rm -f config_ubinize.cfg
	fi
	touch config_ubinize.cfg
	echo [ubifs] >> config_ubinize.cfg
	echo mode=ubi >> config_ubinize.cfg
	echo image=config.ubifs >> config_ubinize.cfg
	echo vol_id=0 >> config_ubinize.cfg
	echo vol_type=dynamic >> config_ubinize.cfg
	echo vol_name=config >> config_ubinize.cfg
	echo vol_flags=autoresize >> config_ubinize.cfg
	echo vol_alignment=1 >> config_ubinize.cfg

	if [ -f config.bin ]; then
		rm -f config.bin
	fi
	ubinize -o ${ZRT_ENV_OUT_CAMERA_CONFIG_FILE_PATH} -m 2048 -p 128KiB -s 2048 config_ubinize.cfg -v
	ls -l config.bin
	rm -f config.ubifs
	rm -f config_ubinize.cfg
fi
if [ $? -ne 0 ]; then
    echo "make config.bin error, please check install the tools first."
    exit 1
fi

echo -e "*\n*Generate Camera config: ${ZRT_ENV_OUT_CAMERA_CONFIG_FILE_PATH}\n*"
