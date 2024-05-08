#!/bin/bash

if [ -f .env ]; then
    source .env
	source ${ZRT_ENV_BUILD_DIR}/logger_info.sh
else
    echo "Please run source at top of zeratul project"
    exit 1
fi


echo -e "*\n*Start build camera N32G031\n*"

#build boot
cd $ZRT_ENV_TOP_DIR/70mai/app/N32G031K8/
make all

#build app
#region `FLASH' overflowed by 1912 bytes
#cd $ZRT_ENV_TOP_DIR/70mai/app/N32G031K8/Nationstech.N32G031_Library.1.0.3/projects/n32g031_EVAL/app/GCC
#make TARGET_BUILD_TYPE=release


