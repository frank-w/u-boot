#!/bin/bash

device=sd
#device=emmc
#device=spi-nand
#device=spi-nor

if [[ -e build.conf ]];then
	. build.conf
fi

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

if [[ "$device" =~ (emmc|spi-nand|spi-nor) ]];then
	dev=emmc
else
	dev=$device
fi
DEFCONFIG=mt7986a_bpi-r3-${dev}_defconfig
DTS=mt7986a-bpi-r3-${dev}
DTSFILE=arch/arm/dts/${DTS}.dts
DTSIFILE=arch/arm/dts/mt7986.dtsi
DEFCFGFILE=configs/$DEFCONFIG

echo "device: $device"
case $1 in
	"dts")
		nano $DTSFILE
	;;
	"dtsi")
		nano $DTSIFILE
	;;
	"defconfig")
		nano $DEFCFGFILE
	;;
	"importconfig")
		echo "import $DEFCONFIG..."
		rm ${DEFCFGFILE}.bak 2>/dev/null
		if [[ "$device" == "emmc" ]];then
			sed -i.bak 's/^#\(CONFIG_ENV\)/\1/' $DEFCFGFILE
		fi
		sed -i.bak 's/\(bootdevice=\).*/\1'${device}'/' uEnv_r3.txt
		make $DEFCONFIG
		if [[ -e ${DEFCFGFILE}.bak ]];then
			mv ${DEFCFGFILE}{.bak,}
		fi
	;;
	"config")
		make menuconfig
	;;
	"build")
		make
		mv uEnv_r3.txt{.bak,}
	;;
	"")
		$0 build
	;;
esac
