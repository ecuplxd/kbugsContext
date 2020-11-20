#!/bin/bash

if [ -d "build" ]; then
    rm -rf build
fi

mkdir build && cd build

INSTALL_PATH="/usr"
cmake .. -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH}
make -j4

if [ "$1" == "install" ]; then
    sudo make install
    sudo rm -rf ${INSTALL_PATH}/bin/kbugscontext
    sudo cp ./kbugscontext ${INSTALL_PATH}/bin/kbugscontext
fi
