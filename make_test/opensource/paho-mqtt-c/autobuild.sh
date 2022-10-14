#!/bin/bash -e

source ../common.sh

build_paho_mqtt_c() {
    echo "[paho_mqtt_c] make"

    cd code
    cmake -Bbuild -H. -DOVSDK_DIR=$CONFIG_TOP -DOV_OPENSSL=${INSTALL_DIR}openssl -DOV_CROSS_COMPILE=$CONFIG_CROSS_COMPILE \
    -DPAHO_HIGH_PERFORMANCE=ON -DCMAKE_INSTALL_PREFIX=./build/_install > make.log
    cmake --build build/ --target install >> make.log

    mkdir -p ${INSTALL_DIR}paho-mqtt-c/include
    mkdir -p ${INSTALL_DIR}paho-mqtt-c/lib

    cp -rf ./build/_install/include/* ${INSTALL_DIR}paho-mqtt-c/include
    cp -rf ./build/_install/lib/libpaho-mqtt3as.so* ${INSTALL_DIR}paho-mqtt-c/lib

    echo "[paho_mqtt_c] done"
}

clean_paho_mqtt_c()
{
    echo "[paho_mqtt_c] clean"
    rm -rf code/build
}

rootfs_install_paho_mqtt_c()
{
    echo "[paho_mqtt_c] rootfs_install"

    cp -rf ${INSTALL_DIR}paho-mqtt-c/lib/libpaho-mqtt3as.so* $ROOTFS/lib
    cp -rf ${INSTALL_DIR}paho-mqtt-c/lib/libpaho-mqtt3as.so* $CONFIG_TOP/app/70mai/lib
    cp -rf ${INSTALL_DIR}paho-mqtt-c/include/* $CONFIG_TOP/app/70mai/include
}

install_paho_mqtt_c()
{
    rootfs_install_paho_mqtt_c
}


#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
    build_paho_mqtt_c
elif type -t $1_paho_mqtt_c 2> /dev/null >&2 ; then
    $1_paho_mqtt_c
fi