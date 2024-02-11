#!/bin/bash
export LANG=C
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

board="bpi-r3"
#board="bpi-r64"
#board="bpi-r4"
device="sdmmc"
#device="emmc"
#device="spim-nand"
#device="nor"

FIP_COMPRESS=0

#https://forum.banana-pi.org/t/tutorial-build-customize-and-use-mediatek-open-source-u-boot-and-atf/13785/
mkimg="USE_MKIMAGE=1 MKIMAGE=./tools/mkimage"

if [[ -e build.conf ]];then
       . build.conf
fi

case $device in
	"spi-nand") device="spim-nand";;
	"spi-nor") device="nor";;
esac

#DEFCONFIG="mt7986_rfb_${device}_defconfig"

case $board in
	"bpi-r64") PLAT="mt7622";makeflags="DDR3_FLYBY=1";;
	"bpi-r3"|"bpi-r3mini") PLAT="mt7986";makeflags="DRAM_USE_DDR4=1";FIP_COMPRESS=1;;
	"bpi-r4") PLAT="mt7988";makeflags="DRAM_USE_COMB=1";FIP_COMPRESS=1;;
esac

if [[ $FIP_COMPRESS -eq 1 ]];then
	xz -f -e -k -9 -C crc32 u-boot.bin
	makeflags="$makeflags BL33=u-boot.bin.xz"
else
	makeflags="$makeflags BL33=u-boot.bin"
fi

makeflags="PLAT=${PLAT} BOOT_DEVICE=$device $makeflags"

case $1 in
	#"importconfig")
	#	make $DEFCONFIG
	#;;
	#"defconfig")
	#	nano configs/$DEFCONFIG
	#;;
	"config")
		make menuconfig
	;;
	"build")
		#make -f Makefile PLAT=mt7622 BOOT_DEVICE=sdmmc DDR3_FLYBY=1 all fip
		#make -f Makefile PLAT=mt7986 BOOT_DEVICE=sdmmc DRAM_USE_DDR4=1 all fip
                set -x
		make $makeflags $mkimg all fip
                set +x
	;;
	"install")
		if [[ "$device" != "sdmmc" ]];then echo "$1 not supported for $device";exit 1;fi
		DEV=/dev/sdb
		read -e -i "$DEV" -p "Please enter target device: " DEV
		case $board in
			"bpi-r64")
				set -x
				sudo dd if=build/${PLAT}/release/bl2.img of=${DEV} bs=512 seek=1024 conv=notrunc,fsync #1> /dev/null 2>&1
				sudo dd if=build/${PLAT}/release/fip.bin of=${DEV} bs=512 seek=2048 conv=notrunc,fsync #1> /dev/null 2>&1
				set +x
			;;
			"bpi-r3"|"bpi-r4")
				set -x
				sudo dd if=build/${PLAT}/release/bl2.img of=${DEV}1 conv=notrunc,fsync #1> /dev/null 2>&1
				sudo dd if=build/${PLAT}/release/fip.bin of=${DEV}4 conv=notrunc,fsync #1> /dev/null 2>&1
				set +x
			;;
		esac
		;;
	"backup")
		DEV=/dev/sdb
		read -e -i "$DEV" -p "Please enter target device: " DEV
		case $board in
			"bpi-r64")
				sudo dd if=${DEV} of=${board}_bl2fip.img bs=512 count=6144
			;;
			"bpi-r3"|"bpi-r4")
				sudo dd of=${board}_bl2.img if=${DEV}1
				sudo dd of=${board}_fip.bin if=${DEV}4
			;;
		esac
		;;
	"createimg")
		if ! [[ "$device" =~ (sd|e)mmc ]];then echo "$1 not supported for $device";exit 1;fi

		IMGDIR=.
		IMGNAME=${board}_${device}
		REALSIZE=7456
		echo "create $IMGNAME.img"
		dd if=/dev/zero of=$IMGDIR/$IMGNAME.img bs=1M count=$REALSIZE 1> /dev/null 2>&1
		LDEV=`sudo losetup -f`
		DEV=`echo $LDEV | cut -d "/" -f 3`     #mount image to loop device
		echo "run losetup to assign image $IMGNAME.img to loopdev $LDEV ($DEV)"
		sudo losetup $LDEV $IMGDIR/$IMGNAME.img 1> /dev/null #2>&1
		bootsize=100
		rootsize=6600
		if [[ "$2" != "non-interactive" ]];then
			read -p "size of boot? (MiB): " -ei $bootsize bootsize
			read -p "size of root? (MiB): " -ei $rootsize rootsize
		fi
		case $board in
			"bpi-r64")
				bootstart=8192
				bootend=$(( ${bootstart}+(${bootsize}*1024*2)-1 ))
				rootstart=$(( ${bootend}+1 ))
				rootend=$(( ${rootstart} + (${rootsize}*1024*2) ))
				sudo sgdisk -o ${LDEV}
				sudo sgdisk -a 1 -n 1:2048:6143 -t 1:8300 -c 1:"fip"		${LDEV}
				sudo sgdisk -a 1 -n 2:6144:7167 -t 2:8300 -c 2:"config"		${LDEV}
				sudo sgdisk -a 1 -n 3:7168:8191 -t 3:8300 -c 3:"rf"		${LDEV}
				sudo sgdisk -a 1024 -n 4:${bootstart}:${bootend} -t 4:0700 -c 4:"kernel" ${LDEV}
				sudo sgdisk -a 1024 -n 5:${rootstart}:${rootend} -t 5:8300 -c 5:"root" ${LDEV}
				#r64 needs special MBR
				if [[ "$device" == "sdmmc" ]];then
					sudo dd of=${LDEV} if=r64_header_sdmmc.bin || exit 1
					sudo dd of=${LDEV} if=build/${PLAT}/release/bl2.img bs=512 seek=1024
				else #emmc
					sudo dd of=${LDEV} if=r64_header_emmc.bin || exit 1
				fi
				sudo dd of=${LDEV} if=build/${PLAT}/release/fip.bin bs=512 seek=2048
				#re-read part table
				sudo losetup -d $LDEV
				sudo losetup -P $LDEV $IMGDIR/$IMGNAME.img 1> /dev/null #2>&1
				sudo mkfs.vfat "${LDEV}p4" -n BPI-BOOT #1> /dev/null 2>&1
				sudo mkfs.ext4 -O ^metadata_csum,^64bit "${LDEV}p5" -L BPI-ROOT #1> /dev/null 2>&1
			;;
			"bpi-r3"|"bpi-r3mini"|"bpi-r4")
				bootstart=17408
				bootend=$(( ${bootstart}+(${bootsize}*1024*2)-1 ))
				rootstart=$(( ${bootend}+1 ))
				rootend=$(( ${rootstart} + (${rootsize}*1024*2) ))
				sudo sgdisk -o ${LDEV}
				sudo sgdisk -a 1 -n 1:34:8191 -A 1:set:2 -t 1:8300 -c 1:"bl2"		${LDEV}
				#sudo sgdisk --attributes=1:set:2 ${LDEV}
				sudo sgdisk -a 1 -n 2:8192:9215 -A 2:set:63	-t 2:8300 -c 2:"u-boot-env"	${LDEV}
				sudo sgdisk -a 1 -n 3:9216:13311 -A 3:set:63	-t 3:8300 -c 3:"factory"	${LDEV}
				sudo sgdisk -a 1 -n 4:13312:17407 -A 4:set:63	-t 4:8300 -c 4:"fip"		${LDEV}
				sudo sgdisk -a 1024 -n 5:17408:${bootend}	-t 5:8300 -c 5:"boot"		${LDEV}
				sudo sgdisk -a 1024 -n 6:${rootstart}:${rootend} -t 6:8300 -c 6:"rootfs"	${LDEV}

				#sudo dd if=gpt_${device}_100m6g.img of=$LDEV conv=notrunc,fsync #1> /dev/null 2>&1
				#try to repair MBR/GPT
				#sudo sgdisk --backup=bpi-r3_sgdisk.gpt /dev/sdb
				#sudo sgdisk --load-backup=bpi-r3_sgdisk.gpt /dev/sdb
				#re-read part table
				sudo losetup -d $LDEV
				sudo losetup -P $LDEV $IMGDIR/$IMGNAME.img 1> /dev/null #2>&1

				#sudo partprobe $LDEV #1> /dev/null 2>&1
				if [[ "$device" == "sdmmc" ]];then
					sudo dd if=build/${PLAT}/release/bl2.img of=${LDEV}p1 conv=notrunc,fsync #1> /dev/null 2>&1
				fi
				sudo dd if=build/${PLAT}/release/fip.bin of=${LDEV}p4 conv=notrunc,fsync #1> /dev/null 2>&1
				sudo mkfs.vfat "${LDEV}p5" -n BPI-BOOT #1> /dev/null 2>&1
				sudo mkfs.ext4 -O ^metadata_csum,^64bit "${LDEV}p6" -L BPI-ROOT #1> /dev/null 2>&1
			;;
		esac
		sudo losetup -d $LDEV
		echo "packing image..."
		gzip $IMGDIR/$IMGNAME.img
	;;
	"rename")
		set -x
		cp build/${PLAT}/release/bl2.img ${board}_${device}_bl2.img
		cp build/${PLAT}/release/fip.bin ${board}_${device}_fip.bin
		set +x
	;;
	"clean")
		make $makeflags clean
	;;
	"")
		$0 build
	;;
	*)
		echo "unsupported option $1"
	;;
esac
