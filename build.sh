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

if [[ -e "build.conf" ]];
then
	. build.conf
fi

case $board in
	"bpi-r2")
		FILE_DTS=arch/arm/dts/mt7623n-bananapi-bpi-r2.dts
		FILE_DTSI=arch/arm/dts/mt7623.dtsi
		FILE_DEFCFG=mt7623n_bpir2_defconfig
		FILE_BOARD=board/mediatek/mt7623/mt7623_rfb.c
		FILE_SOC=include/configs/mt7623.h
		FILE_UENV=/media/$USER/BPI-BOOT/bananapi/bpi-r2/linux/uEnv.txt

		#start-values in kB
		UBOOT_START=320
		UBOOT_FILE=u-boot.bin
		ENV_START=1024 #ENV_OFFSET = 0x100000
	;;
	"bpi-r64")
		FILE_DTS=arch/arm/dts/mt7622-bananapi-bpi-r64.dts
		FILE_DTSI=arch/arm/dts/mt7622.dtsi
		FILE_DEFCFG=mt7622_bpi-r64_32_defconfig
		FILE_BOARD=board/mediatek/mt7622/mt7622_rfb.c
		FILE_SOC=include/configs/mt7622.h

		#start-values in kB
		UBOOT_START=768
		UBOOT_FILE=u-boot-mtk.bin
		ENV_START=1280 #ENV_OFFSET = 0x140000

		if [[ "$arch" == "arm64" ]];then
			FILE_DTS=arch/arm/dts/mt7622-rfb.dts
			FILE_DEFCFG=mt7622_bpi-r64_defconfig
			export ARCH=arm64
			export CROSS_COMPILE=aarch64-linux-gnu-
			UBOOT_FILE=u-boot.bin
			#~40kb bl31+~640kb uboot =~ 682kb fip @0x160000 <0x300000
			UBOOT_START=1064 #1024k + 40k
			ENV_START=3072 #ENV_OFFSET (bytes) = 0x300000 (0x1800 /2 kbytes)
		fi
		FILE_UENV=/media/$USER/BPI-BOOT/bananapi/bpi-r64/linux/uEnv.txt
	;;
	*)
		echo "unsupported"
	;;
esac


MAXSIZE=$(( ($ENV_START - $UBOOT_START)*1024 -1 ))

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
			echo "bpi-r64 with new ATF needs uboot packed into fip!"
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
