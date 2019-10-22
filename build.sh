#!/bin/bash
#export ARCH=arm64
#export CROSS_COMPILE=aarch64-linux-gnu-
export CROSS_COMPILE=arm-linux-gnueabihf-
LANG=C
CFLAGS=-j$(grep ^processor /proc/cpuinfo  | wc -l)

case $1 in

	"importconfig")
		make mt7622_bpir64_efi_defconfig
	;;
	"config")
		make menuconfig
	;;
	"build")
		make
	;;
	"install")
		echo "not yet supported!";exit;
		UBOOT=u-boot-mtk.bin
		read -e -i "$UBOOT" -p "Please enter source file: " UBOOT
		if [[ ! -e $UBOOT ]];then
			echo "please build uboot first or set correct file..."
			exit;
		fi
		dev=/dev/sdb
		read -e -i $dev -p "device to write: " input
		dev="${input:-$dev}"
		choice=y
		if ! [[ "$(lsblk ${dev}1 -o label -n)" == "BPI-BOOT" ]]; then
			read -e -p "this device seems not to be a BPI-R64 SD-Card, do you really want to use this device? [yn]" choice
		fi
		if [[ "$choice" == "y" ]];then
			echo "writing to $dev"
			sudo dd if=$UBOOT of=$dev bs=1k seek=768 #768k = 0xC0000
			sync
		fi
		;;
	"soc")
		include/configs/mt7622.h
		;;
	"umount")
		umount /media/$USER/BPI-BOOT
		umount /media/$USER/BPI-ROOT
	;;
esac
