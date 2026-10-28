#!/bin/sh
CC=gcc
$CC -shared -fPIC -O2  sound_lib.c miniaudio.c extras/stb_vorbis.c -o libsound_lib.so
