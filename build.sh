#!/bin/bash
export ARCH=arm;
export CROSS_COMPILE=arm-linux-gnueabihf-

case $1 in
	"build")
		make;
	;;
	"config")
		make menuconfig;
	;;
	"importconfig")
		make mt7623n_bpir2_defconfig;
	;;
	"install")
		dev=/dev/sdb
		read -e -i "$dev" -p "Please enter target device: " dev
		sudo dd of=$dev if=u-boot.bin bs=1k seek=320;
		sync
	;;
	"umount")
		umount /media/$USER/BPI-BOOT
		umount /media/$USER/BPI-ROOT
	;;

	*)
		make;
	;;
esac
