#!/bin/bash

if [ -f .env ]; then
    source .env
	source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project"
    exit 1
fi

VERSION=`awk 'BEGIN{print ENVIRON["ZRT_ENV_FW_VERSION"]}'`

if [ ! -n "${VERSION}" ]; then
	echo -e "please cd to zeratul top and run \"source build/env_setup_zeratul.sh\" first"
	exit 1
else
	exit 0
fi

