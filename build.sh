#!/bin/sh

# Developer use only
# Either run build_linux_x64.sh, or use CMake

# Silence output of a succesful build
# Only display it in case we fail
ninja -C build_linux_x64 install > log.txt
rc=$?

if [ $rc -ne 0 ]; then
    cat log.txt
fi

gcc -g3 test_raw_audio.c -L./bin_linux_x64 -lsound_lib -o current -Wl,-rpath,.
cp bin_linux_x64/libsound_lib.so .

