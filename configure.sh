#!/bin/sh

argv=$*
options=${argv[0]}

build_type="-DCMAKE_BUILD_TYPE=Debug"

if [ "$options" == "release" ]; then
    build_type="-DCMAKE_BUILD_TYPE=RelWithDebInfo"
fi


cmake -S . -B build_linux_x64 -G Ninja -DCMAKE_INSTALL_PREFIX=. -DCMAKE_INSTALL_RPATH=. $build_type
cmake -S . -B build_mingw_x64 -G Ninja -DCMAKE_INSTALL_PREFIX=. -DCMAKE_INSTALL_RPATH=. -DCMAKE_TOOLCHAIN_FILE=toolchain_mingw_x64.cmake $build_type
#cmake -S . -B build_mingw_x86 -G Ninja -DCMAKE_INSTALL_PREFIX=. -DCMAKE_INSTALL_RPATH=. -DCMAKE_TOOLCHAIN_FILE=toolchain_w32_x86.cmake $build_type
