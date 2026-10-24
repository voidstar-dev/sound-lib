# Generates simple, one line build scripts that don't rely on external build tools
import os

class string_builder:
    def __init__(self):
        self.buf = ""

    def write(self, text, *args):
        self.buf += sprintf(text, *args)

    def data(self):
        return self.buf

def write_entire_file(name, data):
    f = open(name, "w")
    f.write(data)
    f.close()


def sprintf(fmt, *args):
    s = fmt % args
    return s


def make_executable(fname):
    # Note: Time of use / access bug-prone
    mode = os.stat(fname).st_mode
    mode |= 0o700
    os.chmod(fname, mode)


LIB_NAME = "sound_lib"
SOURCES = "sound_lib.c miniaudio.c extras/stb_vorbis.c"

def generate_script_linux(config):
    CC = config["CC"]
    fname = config["FNAME"]
    CFLAGS = config["CFLAGS"]
    LDFLAGS = config["LDFLAGS"]
    PREFIX = config["PREFIX"]
    SUFFIX = config["SUFFIX"]

    sb = string_builder()
    sb.write("#!/bin/sh\n")
    sb.write("CC=%s\n", CC)
    sb.write("$CC %s %s %s -o %s%s%s\n", CFLAGS, LDFLAGS, SOURCES, PREFIX, LIB_NAME, SUFFIX)

    write_entire_file(fname, sb.data())
    make_executable(fname)

def generate_script_win(config):
    fname = config["FNAME"]

    # Takes advantage of the fact that "cl" will output the DLL and import library based on the first filename argument
    sb = string_builder()
    sb.write("@echo off\n")
    sb.write("cl /LD /O2 %s\n", SOURCES)
    write_entire_file(fname, sb.data())


def generate_build_scripts():
    linux_config = {
        "CC": "gcc",
        "FNAME": "build_linux_x64.sh",
        "CFLAGS": "-shared -fPIC -O2",
        "LDFLAGS": "",
        "PREFIX": "lib",
        "SUFFIX": ".so",
    }

    mingw_config = {
        "CC": "x86_64-w64-mingw32-gcc",
        "FNAME": "build_mingw_x64.sh",
        "CFLAGS": "-shared -O2 -static-libgcc",
        "LDFLAGS": sprintf("-Wl,--out-implib,%s.lib", LIB_NAME),
        "PREFIX": "",
        "SUFFIX": ".dll",
    }

    msvc_config = {
        "FNAME": "build_cl_x64.bat"
    }

    generate_script_linux(linux_config)
    generate_script_linux(mingw_config)
    generate_script_win(msvc_config)

if __name__ == "__main__":
    generate_build_scripts()
