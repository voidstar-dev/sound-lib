#!/bin/sh
#CC=x86_64-w64-mingw32-gcc
#
## I don't know why, but we must compile these separately.
#$CC -c extras/stb_vorbis.c
#$CC -x c -c miniaudio.h -DMINIAUDIO_IMPLEMENTATION
#$CC miniaudio.o stb_vorbis.o -shared -o lib_sound.dll

ninja -C build_win32 install
