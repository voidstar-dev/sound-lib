#!/bin/sh
targets="build_mingw_x64 build_linux_x64"
export INSTALL_DIR="./sound_lib/"

# for t in $targets; do
    # rm -rf $t
# done


build_docs()
{
    DOCS_INSTALL_DIR="$(pwd)/$INSTALL_DIR"
    make -C docs install INSTALL_DIR=$DOCS_INSTALL_DIR
}

configure()
{
    ./configure.sh release
}

compile()
{
    for t in $targets; do
        ninja -C "$t" install &
    done
    wait
}

generate_build_scripts()
{
    python generate_build_scripts.py
}

zip_release_folder()
{
    zip -u -r sound_lib.zip sound_lib/*
}

generate_build_scripts
configure
compile
build_docs
zip_release_folder
