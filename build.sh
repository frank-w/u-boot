#!/bin/bash
export ARCH=arm;
export CROSS_COMPILE=arm-linux-gnueabihf-

uver=$(make ubootversion)
ubranch=$(git branch --contains $(git log -n 1 --pretty='%h') | grep -v '(HEAD' | head -1 | sed 's/^..//'| sed 's/^[0-9.-]*//')

echo "ver:$uver,ubranch:$ubranch"

#values in kB
UBOOT_START=320
ENV_START=1024

MAXSIZE=$(( ($ENV_START - $UBOOT_START)*1024 -1 ))

if [[ -e "build.conf" ]];
then
	. build.conf
fi

function upload {
	imagename="u-boot_${uver}-${ubranch}.bin"
	read -e -i $imagename -p "u-boot-filename: " input
	imagename="${input:-$imagename}"

	echo "Name: $imagename"
	echo "uploading to ${uploadserver}:${uploaddir}..."

	scp u-boot.bin ${uploaduser}@${uploadserver}:${uploaddir}/${imagename}
}

case $1 in
	"build")
		make LOCALVERSION="-$ubranch";
		FILESIZE=$(stat -c%s "u-boot.bin");
		if [[ $FILESIZE -gt $MAXSIZE ]]; then
			echo "=============== WARNING ==============="
			echo "u-boot will overwrite env-storage area!"
			echo "if you use this u-boot.bin don't use saveenv!"
		fi;
	;;
	"config")
		make menuconfig;
	;;
	"importconfig")
		make mt7623n_bpir2_defconfig;
	;;
	"defconfig")
		nano configs/mt7623n_bpir2_defconfig;
	;;
	"install")
		dev=/dev/sdb
		choice=y
		read -e -i "$dev" -p "Please enter target device: " dev
		if ! [[ "$(lsblk ${dev}1 -o label -n)" == "BPI-BOOT" ]]; then
			read -e -p "this device seems not to be a BPI-R2 SD-Card, do you really want to use this device? [yn]" choice
		fi
		if [[ "$choice" == "y" ]];then
			echo "writing to $dev"
			sudo dd of=$dev if=u-boot.bin bs=1k seek=320;
			sync
		fi
	;;
	"umount")
		umount /media/$USER/BPI-BOOT
		umount /media/$USER/BPI-ROOT
	;;
	"uenv")
		nano /media/$USER/BPI-BOOT/bananapi/bpi-r2/linux/uEnv.txt
	;;
	"board")
		nano board/mediatek/mt7623/mt7623_rfb.c
	;;
	"dts")
		nano arch/arm/dts/mt7623n-bananapi-bpi-r2.dts
	;;
	"dtsi")
		nano arch/arm/dts/mt7623.dtsi
	;;
	"soc")
		nano ./include/configs/mt7623.h
	;;
	"upload")
		upload
	;;
	*)
		$0 build;
	;;
esac
