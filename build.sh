#!/bin/bash
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

board="bpi-r3"
device="sdmmc"
#device="emmc"
#device="spim_nand"
#device="spim_nor"

#https://forum.banana-pi.org/t/tutorial-build-customize-and-use-mediatek-open-source-u-boot-and-atf/13785/
mkimg="USE_MKIMAGE=1 MKIMAGE=./tools/mkimage"

if [[ -e build.conf ]];then
       . build.conf
fi

case $device in
	"spi-nand") device="spim_nand";;
	"spi-nor") device="spim_nor";;
esac

DEFCONFIG="mt7986_rfb_${device}_defconfig"


case $1 in
	"importconfig")
		make $DEFCONFIG
	;;
	"defconfig")
		nano configs/$DEFCONFIG
	;;
	"config")
		make menuconfig
	;;
	"build")
		#make -f Makefile PLAT=mt7622 BOOT_DEVICE=sdmmc DDR3_FLYBY=1 all fip
		#make -f Makefile PLAT=mt7986 BOOT_DEVICE=sdmmc DRAM_USE_DDR4=1 all fip
		case "$board" in
			"bpi-r64") makeflags="PLAT=mt7622 DDR3_FLYBY=1";;
			"bpi-r3") makeflags="PLAT=mt7986 DRAM_USE_DDR4=1";;
		esac
		makeflags="$makeflags BL33=u-boot.bin"
		make $makeflags $mkimg BOOT_DEVICE=$device all fip
	;;
	"install")
		if [[ "$device" != "sdmmc" ]];then echo "$1 not supported for $device";exit 1;fi
		DEV=/dev/sdb
		read -e -i "$DEV" -p "Please enter target device: " DEV
		case $board in
			"bpi-r3")
				set -x
				sudo dd if=build/mt7986/release/bl2.img of=${DEV}1 conv=notrunc,fsync #1> /dev/null 2>&1
				sudo dd if=build/mt7986/release/fip.bin of=${DEV}4 conv=notrunc,fsync #1> /dev/null 2>&1
				set +x
			;;
		esac
		;;
	"createimg")
		if [[ "$device" != "sdmmc" ]];then echo "$1 not supported for $device";exit 1;fi
		IMGDIR=.
		IMGNAME=${board}_${device}
		REALSIZE=7000
		dd if=/dev/zero of=$IMGDIR/$IMGNAME.img bs=1M count=$REALSIZE 1> /dev/null 2>&1
		LDEV=`sudo losetup -f`
		DEV=`echo $LDEV | cut -d "/" -f 3`     #mount image to loop device
		echo "run losetup to assign image $IMGNAME.img to loopdev $LDEV ($DEV)"
		sudo losetup $LDEV $IMGDIR/$IMGNAME.img 1> /dev/null #2>&1
		case $board in
			"bpi-r3")
				sudo dd if=gpt_${device}_100m6g.img of=$LDEV conv=notrunc,fsync #1> /dev/null 2>&1

				#re-read part table
				sudo losetup -d $LDEV
				sudo losetup -P $LDEV $IMGDIR/$IMGNAME.img 1> /dev/null #2>&1
				#sudo gdisk -l /dev/sdb #try to repair MBR/GPT

				#sudo partprobe $LDEV #1> /dev/null 2>&1
				sudo dd if=build/mt7986/release/bl2.img of=${LDEV}p1 conv=notrunc,fsync #1> /dev/null 2>&1
				sudo dd if=build/mt7986/release/fip.bin of=${LDEV}p4 conv=notrunc,fsync #1> /dev/null 2>&1
				#sudo mkfs.vfat "${LDEV}p5" -n BPI-BOOT #1> /dev/null 2>&1
				#sudo mkfs.ext4 -O ^metadata_csum,^64bit "${LDEV}p6" -L BPI-ROOT #1> /dev/null 2>&1
			;;
		esac
		#sudo losetup -d $LDEV
	;;
	"rename")
		cp build/mt7986/release/bl2.img ${board}_${device}_bl2.img
		cp build/mt7986/release/fip.bin ${board}_${device}_fip.bin
	;;
	"")
		$0 build
	;;
esac
