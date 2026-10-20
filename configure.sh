#!/bin/sh
cmake -S . -B build -G Ninja -DCMAKE_INSTALL_PREFIX=. -DCMAKE_INSTALL_RPATH=.
