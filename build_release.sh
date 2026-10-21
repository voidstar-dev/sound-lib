#!/bin/sh
./configure.sh release

targets="build_win32 build_linux"

ninja -C build_linux clean
ninja -C build_win32 clean

ninja -C build_linux install &
ninja -C build_win32 install &

wait

