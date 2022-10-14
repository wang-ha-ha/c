#!/bin/bash -e

source ../common.sh

build_paho_mqtt_cpp() {
    echo "[paho_mqtt_cpp] make"
    cd code
    cmake -Bbuild -H. -DOVSDK_DIR=$CONFIG_TOP -DOV_OPENSSL=${INSTALL_DIR}openssl -DOV_CROSS_COMPILE=$CONFIG_CROSS_COMPILE -DOV_PAHO_MQTT_C=${INSTALL_DIR}paho-mqtt-c\
    -DPAHO_BUILD_STATIC=ON -DPAHO_BUILD_SHARED=ON -DPAHO_WITH_SSL=ON -DCMAKE_INSTALL_PREFIX=./build/_install > make.log
    cmake --build build/ --target install >> make.log

    mkdir -p ${INSTALL_DIR}paho-mqtt-cpp/include
    mkdir -p ${INSTALL_DIR}paho-mqtt-cpp/lib

    cp -rf ./build/_install/include/* ${INSTALL_DIR}paho-mqtt-cpp/include
    cp -rf ./build/_install/lib/libpaho-mqttpp3.so* ${INSTALL_DIR}paho-mqtt-cpp/lib

    echo "[paho_mqtt_cpp] done"
}

clean_paho_mqtt_cpp()
{
    echo "[paho_mqtt_cpp] clean"
    rm -rf code/build
}

rootfs_install_paho_mqtt_cpp()
{
    echo "[paho_mqtt_cpp] rootfs_install"
    cp -rf ${INSTALL_DIR}paho-mqtt-cpp/lib/libpaho-mqttpp3.so* $ROOTFS/lib
    cp -rf ${INSTALL_DIR}paho-mqtt-cpp/lib/libpaho-mqttpp3.so* $CONFIG_TOP/app/70mai/lib
    cp -rf ${INSTALL_DIR}paho-mqtt-cpp/include/* $CONFIG_TOP/app/70mai/include
}

install_paho_mqtt_cpp()
{
    rootfs_install_paho_mqtt_cpp
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
    build_paho_mqtt_cpp
elif type -t $1_paho_mqtt_cpp 2> /dev/null >&2 ; then
    $1_paho_mqtt_cpp
fi