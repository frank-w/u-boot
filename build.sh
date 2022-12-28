#!/bin/bash
export ARCH=arm;
export CROSS_COMPILE=arm-linux-gnueabihf-

uver=$(make ubootversion)
commit=$(git log -n 1 --pretty='%h')
ubranch=$(git branch --contains $commit | grep -v '(HEAD' | head -1 | sed 's/^..//'| sed 's/^[0-9.-]*//')

if [[ "${ubranch}" =~ ^\(.* ]]; then
  ubranch="$commit";
fi
#uboot 2020-01 needs python.h...just note for a checkdep-function
#sudo apt-get install python3-dev

echo "ver:$uver,ubranch:$ubranch"

device=sd
#device=emmc
#bpi-r3-only:
#device=spi-nand
#device=spi-nor

if [[ -e "build.conf" ]];
then
	. build.conf
fi

ENV_START=0
case $board in
	"bpi-r2")
		FILE_DTS=arch/arm/dts/mt7623n-bananapi-bpi-r2.dts
		FILE_DTSI=arch/arm/dts/mt7623.dtsi
		FILE_DEFCFG=mt7623n_bpir2_defconfig
		FILE_BOARD=board/mediatek/mt7623/mt7623_rfb.c
		FILE_SOC=include/configs/mt7623.h

		#start-values in kB
		UBOOT_START=320
		UBOOT_FILE=u-boot.bin
		ENV_START=1024 #ENV_OFFSET = 0x100000
	;;
	"bpi-r64")
		FILE_DTS=arch/arm/dts/mt7622-bananapi-bpi-r64.dts
		FILE_DEFCFG=mt7622_bpi-r64_defconfig
		FILE_DTSI=arch/arm/dts/mt7622.dtsi
		FILE_BOARD=board/mediatek/mt7622/mt7622_rfb.c
		FILE_SOC=include/configs/mt7622.h

		#~40kb bl31+~640kb uboot =~ 682kb fip @0x160000 <0x300000
		UBOOT_START=1064 #1024k + 40k
		ENV_START=3072 #ENV_OFFSET (bytes) = 0x300000 (0x1800 /2 kbytes)
		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-gnu-
		UBOOT_FILE=u-boot.bin
	;;
	"bpi-r3")
		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-gnu-

		if [[ "$device" =~ (emmc|spi-nand|spi-nor) ]];then
			dev=emmc
		else
			dev=$device
		fi
		FILE_DEFCFG=mt7986a_bpir3_${dev}_defconfig

		DTS=mt7986a-${dev}-rfb
		FILE_DTS=arch/arm/dts/${DTS}.dts
		FILE_DTSI=arch/arm/dts/mt7986.dtsi

		FILE_BOARD=board/mediatek/mt7986/mt7986_rfb.c
		FILE_SOC=include/configs/mt7986.h
		UBOOT_FILE=u-boot.bin
	;;
	*)
		echo "unsupported"
		exit 1;
	;;
esac
FILE_UENV=/media/$USER/BPI-BOOT/bananapi/$board/linux/uEnv.txt

if [[ $ENV_START -ne 0 ]];then
	MAXSIZE=$(( ($ENV_START - $UBOOT_START) *1024 -1 ))
else
	MAXSIZE=0
fi

function generate_filename
{
	#grep '^CONFIG_MT7531=y' .config >/dev/null;if [[ $? -eq 0 ]];then ETH="MT7531";fi
	filename=u-boot-${board//bpi-/}_${uver}-${ubranch}-${ARCH}.bin
	echo $filename
}

function upload {
	#imagename="u-boot_${uver}-${ubranch}.bin"
	imagename=$(generate_filename)
	read -e -i $imagename -p "u-boot-filename: " input
	imagename="${input:-$imagename}"

	echo "Name: $imagename"
	echo "uploading ${UBOOT_FILE} to ${uploadserver}:${uploaddir}..."

	scp ${UBOOT_FILE} ${uploaduser}@${uploadserver}:${uploaddir}/${imagename}
}

case $1 in
	"build")
		LANG=C
		CFLAGS=-j$(grep ^processor /proc/cpuinfo  | wc -l)
		echo "LV: -$ubranch, crosscompile: $CROSS_COMPILE, CFLAGS: $CFLAGS"
		make LOCALVERSION="-$ubranch" ${CFLAGS} 2> >(tee "build.log")
		if [[ $? -eq 0 ]];then
			FILESIZE=$(stat -c%s "u-boot.bin");
			if [[ $MAXSIZE -gt 0 && $FILESIZE -gt $MAXSIZE ]]; then
				echo "=============== WARNING ==============="
				echo "u-boot will overwrite env-storage area!"
				echo "if you use this u-boot.bin don't use saveenv!"
			fi;
		else
			echo "build failed!"
		fi
	;;
	"config")
		make menuconfig;
	;;
	"importconfig")
		if [[ -n "$FILE_DEFCFG" ]];then
			echo "importing $FILE_DEFCFG"
			make $FILE_DEFCFG
		fi
	;;
	"defconfig")
		if [[ -n "$FILE_DEFCFG" ]];then
			nano configs/$FILE_DEFCFG;
		fi
	;;
	"install")
		if [[ $ARCH == "arm" ]];then
			dev=/dev/sdb
			choice=y
			read -e -i "$dev" -p "Please enter target device: " dev
			if ! [[ "$(lsblk ${dev}1 -o label -n)" == "BPI-BOOT" ]]; then
				read -e -p "this device seems not to be a BPI-R2 SD-Card, do you really want to use this device? [yn]" choice
			fi
			if [[ "$choice" == "y" ]];then
				echo "writing to $dev ($UBOOT_FILE to ${UBOOT_START}k)"
				sudo dd of=$dev if=$UBOOT_FILE bs=1k seek=$UBOOT_START;
				sync
			fi
		else
			echo "bpi-r64/bpi-r3 with new ATF needs uboot packed into fip!"
		fi
	;;
	"umount")
		umount /media/$USER/BPI-BOOT
		umount /media/$USER/BPI-ROOT
	;;
	"uenv")
		if [[ -n "$FILE_UENV" ]]; then
			nano $FILE_UENV
		fi
	;;
	"board")
		nano $FILE_BOARD
	;;
	"dts")
		nano $FILE_DTS
	;;
	"dtsi")
		nano $FILE_DTSI
	;;
	"soc")
		nano $FILE_SOC
	;;
	"upload")
		upload
	;;
	"rename")
		filename=$(generate_filename)
		echo "rename $UBOOT_FILE to $filename"
		mv $UBOOT_FILE $filename
	;;
	*)
		$0 build;
	;;
esac
