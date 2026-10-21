#!/bin/sh

argv=$*
options=${argv[0]}


if [ "$options" == "release" ]; then
    build_type="-DCMAKE_BUILD_TYPE=Release"
    build_type="-DCMAKE_BUILD_TYPE=RelWithDebInfo"
fi


cmake -S . -B build_linux -G Ninja -DCMAKE_INSTALL_PREFIX=. -DCMAKE_INSTALL_RPATH=. $build_type
cmake -S . -B build_win32 -G Ninja -DCMAKE_INSTALL_PREFIX=. -DCMAKE_INSTALL_RPATH=. -DCMAKE_TOOLCHAIN_FILE=toolchain_w32_x64.cmake $build_type
