#!/bin/bash
set -e

if [ -f .env ]; then
    source .env
    source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo -e "\033[31m"[ERROR]Please run source at top of zeratul project"\033[0m"
    exit 1
fi

mkdir -p ${ZRT_ENV_OUT_CAMERA_DIR}
cp ${ZRT_ENV_TOP_DIR}/70mai/env_config/user.txt ${ZRT_ENV_OUT_CAMERA_DIR}/user.txt
cp ${ZRT_ENV_TOP_DIR}/70mai/env_config/factory.txt ${ZRT_ENV_OUT_CAMERA_DIR}/factory.txt
cp ${ZRT_ENV_TOP_DIR}/70mai/env_config/point.txt ${ZRT_ENV_OUT_CAMERA_DIR}/point.txt

case ${ZRT_ENV_HARDWARE_VERSION} in
    a1_1)
        echo ---------------user----------------
        ${ZRT_ENV_TOP_DIR}/70mai/env_config/fw_genenv user ${ZRT_ENV_OUT_CAMERA_DIR}/user.txt ${ZRT_ENV_OUT_CAMERA_DIR}/env_user.bin
        echo ---------------factory----------------
        ${ZRT_ENV_TOP_DIR}/70mai/env_config/fw_genenv factory ${ZRT_ENV_OUT_CAMERA_DIR}/factory.txt ${ZRT_ENV_OUT_CAMERA_DIR}/env_factory.bin
        echo ---------------point----------------
        ${ZRT_ENV_TOP_DIR}/70mai/env_config/fw_genenv point ${ZRT_ENV_OUT_CAMERA_DIR}/point.txt ${ZRT_ENV_OUT_CAMERA_DIR}/env_point.bin

        mkdir -p ${ZRT_ENV_OUT_DIR}
        tr '\000' '\377' < /dev/zero | dd of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K count=64 > /dev/null
        dd if=${ZRT_ENV_OUT_CAMERA_DIR}/env_factory.bin of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K seek=8
        dd if=${ZRT_ENV_OUT_CAMERA_DIR}/env_user.bin of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K seek=12
        dd if=${ZRT_ENV_OUT_CAMERA_DIR}/env_user.bin of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K seek=32
        dd if=${ZRT_ENV_OUT_CAMERA_DIR}/env_point.bin of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K seek=48
    ;;
    b1_1)
        echo ---------------user----------------
        ${ZRT_ENV_TOP_DIR}/70mai/env_config/fw_genenv user ${ZRT_ENV_OUT_CAMERA_DIR}/user.txt ${ZRT_ENV_OUT_CAMERA_DIR}/env_user.bin
        echo ---------------factory----------------
        ${ZRT_ENV_TOP_DIR}/70mai/env_config/fw_genenv factory ${ZRT_ENV_OUT_CAMERA_DIR}/factory.txt ${ZRT_ENV_OUT_CAMERA_DIR}/env_factory.bin
        echo ---------------point----------------
        ${ZRT_ENV_TOP_DIR}/70mai/env_config/fw_genenv point ${ZRT_ENV_OUT_CAMERA_DIR}/point.txt ${ZRT_ENV_OUT_CAMERA_DIR}/env_point.bin

        mkdir -p ${ZRT_ENV_OUT_DIR}
        tr '\000' '\377' < /dev/zero | dd of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K count=64 > /dev/null
        dd if=${ZRT_ENV_OUT_CAMERA_DIR}/env_factory.bin of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K seek=8
        dd if=${ZRT_ENV_OUT_CAMERA_DIR}/env_user.bin of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K seek=12
        dd if=${ZRT_ENV_OUT_CAMERA_DIR}/env_user.bin of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K seek=32
        dd if=${ZRT_ENV_OUT_CAMERA_DIR}/env_point.bin of=${ZRT_ENV_OUT_CAMERA_ENV_FILE_PATH} bs=1K seek=48
    ;;
    *) 
        echo "Unknown hardware version"
        exit 1
    ;;
esac
