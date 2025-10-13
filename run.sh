#!/bin/bash

function usage {
    echo "Usage: $0 <model>"
    echo "Available models: bpi-r3 bpi-r4"
}

[ $# -ne 1 ] && usage && exit 1

model=$1

case $model in
    bpi-r3) ;;
    bpi-r4) ;;
    *) echo "Unsupported model: $model" && usage && exit 1 ;;
esac

time {
    sudo losetup -D
    make clean
    echo -e "board=${model}\ndevice=emmc" > build.conf
    ./build.sh importconfig
    ./build.sh
    ./build.sh rename
    git checkout -f mtk-atf-2025
    echo -e "board=${model}\ndevice=emmc" > build.conf
    ./build.sh
    ./build.sh rename
    ./build.sh createimg non-interactive
    if [ $model = "bpi-r4" ]; then
        make realclean
        echo -e "extraflags=DDR4_4BG_MODE=1" >> build.conf
        ./build.sh
        ./build.sh rename
    fi
    rm build.conf
    git checkout -f 2025-10-bpi
}
