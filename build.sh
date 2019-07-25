#!/bin/bash

if [ -d "build" ]; then
    rm -rf build
fi

mkdir build && cd build

cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j4

if [ $1 == "install" ]; then
    sudo make install
fi