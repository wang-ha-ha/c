#!/bin/bash -e

source ../common.sh

build_yaml-cpp() {
    echo "[yaml-cpp] make"
    [ ! -e build ] && mkdir build
    cd build
    cmake  -D BUILD_SHARED_LIBS=ON \
    -DCMAKE_C_COMPILER=mips-linux-gnu-gcc -DCMAKE_CXX_COMPILER=mips-linux-gnu-g++ .. &> cmake.log
    make &> make.log

    [ ! -e $INSTALL_DIR/yaml-cpp ] && mkdir $INSTALL_DIR/yaml-cpp  $INSTALL_DIR/yaml-cpp/lib
    cp -rf ../include $INSTALL_DIR/yaml-cpp/
    cp -rf ../build/libyaml-cpp.so*  $INSTALL_DIR/yaml-cpp/lib
    
    echo "[yaml-cpp] done"
}

clean_yaml-cpp()
{
    echo "[yaml-cpp] clean"
    rm -rf build
}

rootfs_install_yaml-cpp()
{
    echo "[yaml-cpp] rootfs_install"
    cp -rf $INSTALL_DIR/yaml-cpp/lib/libyaml-cpp.so* $ROOTFS/lib
}

install_yaml-cpp()
{
    rootfs_install_yaml-cpp
}

#
# main
#
if [ $# -eq 0 ] || [ "$1" = "build" ]; then
    build_yaml-cpp
elif type -t $1_yaml-cpp 2> /dev/null >&2 ; then
    $1_yaml-cpp
fi
