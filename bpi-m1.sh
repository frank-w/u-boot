#!/bin/bash

set -e

BPIBOARD=bpi-m1
MODE=$1

generate_board_config() {
        cat <<-EOT
	CONFIG_ARMV7_BOOT_SEC_DEFAULT=y
	CONFIG_OLD_SUNXI_KERNEL_COMPAT=y
	CONFIG_BPI_CHIP="a20"
	CONFIG_BPI_BOARD="${BPIBOARD}"
	CONFIG_BPI_SERVICE="linux"
	CONFIG_BPI_PATH="bananapi/${BPIBOARD}/linux/"
	CONFIG_BPI_UENVFILE="bananapi/${BPIBOARD}/linux/uEnv.txt"
	EOT

}

export BOARD=${BPIBOARD}
export ARCH=arm
BPICONF=Bananapi_defconfig
NEWBPICONF=$BPICONF

if [ -z "$MODE" ]; then
	MODE=mainline
else
	MODE=legacy
	export BOARD=${BPIBOARD}-legacy
	NEWBPICONF=BPI-$BPICONF
	cat configs/$BPICONF >configs/$NEWBPICONF
	generate_board_config "$1" >> configs/$NEWBPICONF
fi
echo MODE=$MODE

KBUILD_OUTPUT=out/$BOARD
export KBUILD_OUTPUT
mkdir -p $KBUILD_OUTPUT
export CROSS_COMPILE=arm-linux-gnueabihf-
make $NEWBPICONF
make -j8
./bpi-uimgz.sh $BOARD

