#!/bin/sh
CC=x86_64-w64-mingw32-gcc
$CC -shared -O2 -static-libgcc -Wl,--out-implib,sound_lib.lib sound_lib.c miniaudio.c extras/stb_vorbis.c -o sound_lib.dll
