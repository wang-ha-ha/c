#!/bin/bash

if [ -f .env ]; then
    source .env
else
    echo "Please run source at top of zeratul project"
    exit 1
fi

if [ ! -d ${ZRT_ENV_TOP_DIR} ]; then
    echo "Please source env_setup.sh at first"
    exit 1
fi

7z > /dev/null

if [ $? -ne 0 ]; then
    echo "7z run failed, please check the command"
    exit 1
fi

7z x ${ZRT_ENV_TOP_DIR}/os/kernel.7z -o${ZRT_ENV_TOP_DIR}/os
7z x ${ZRT_ENV_TOP_DIR}/os/busybox.7z -o${ZRT_ENV_TOP_DIR}/os
7z x ${ZRT_ENV_TOP_DIR}/os/drivers.7z -o${ZRT_ENV_TOP_DIR}/os
7z x ${ZRT_ENV_TOP_DIR}/os/rootfs.7z -o${ZRT_ENV_TOP_DIR}/os
7z x ${ZRT_ENV_TOP_DIR}/os/uboot.7z -o${ZRT_ENV_TOP_DIR}/os
7z x ${ZRT_ENV_TOP_DIR}/userland.7z -o${ZRT_ENV_TOP_DIR}/
7z x ${ZRT_ENV_TOP_DIR}/firmware.7z -o${ZRT_ENV_TOP_DIR}/
7z x ${ZRT_ENV_TOP_DIR}/tools.7z -o${ZRT_ENV_TOP_DIR}/
7z x ${ZRT_ENV_TOP_DIR}/hdk.7z -o${ZRT_ENV_TOP_DIR}/

echo "Unpack archive done."
