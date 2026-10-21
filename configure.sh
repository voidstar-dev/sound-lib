#!/bin/sh
cmake -S . -B build_linux -G Ninja -DCMAKE_INSTALL_PREFIX=. -DCMAKE_INSTALL_RPATH=.
cmake -S . -B build_win32 -G Ninja -DCMAKE_INSTALL_PREFIX=. -DCMAKE_INSTALL_RPATH=. -DCMAKE_TOOLCHAIN_FILE=toolchain_w32_x64.cmake
