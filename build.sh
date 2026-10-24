#!/bin/sh

# Silence output of a succesful build
# Only display it in case we fail
ninja -C build_linux_x64 install > log.txt
rc=$?

if [ $rc -ne 0 ]; then
    cat log.txt
fi
