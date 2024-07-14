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
#r2pro needs python elftools
#sudo apt-get install python3-pyelftools

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

FILE_UENV=/media/$USER/BPI-BOOT/uEnv.txt

function edit()
{
        file=$1
        if [[ -z "$EDITOR" ]];then EDITOR=/usr/bin/editor;fi #use alternatives setting
        if [[ -e "$file" ]];then
                if [[ -w "$file" ]];then
                        $EDITOR "$file"
                else
                        echo "file $file not writable by user using sudo..."
                        sudo $EDITOR "$file"
                fi
        else
                echo "file $file not found"
        fi
}

case $board in
	"bpi-r2")
		FILE_DTS=arch/arm/dts/mt7623n-bananapi-bpi-r2.dts
		FILE_DTSI=arch/arm/dts/mt7623.dtsi
		FILE_DEFCFG=mt7623n_bpir2_defconfig
		FILE_BOARD=board/mediatek/mt7623/mt7623_rfb.c
		FILE_SOC=include/configs/mt7623.h
		FILE_UENV=/media/$USER/BPI-BOOT/bananapi/$board/linux/uEnv.txt

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
		FILE_UENV=/media/$USER/BPI-BOOT/bananapi/$board/linux/uEnv.txt

		#~40kb bl31+~640kb uboot =~ 682kb fip @0x160000 <0x300000
		UBOOT_START=1064 #1024k + 40k
		ENV_START=3072 #ENV_OFFSET (bytes) = 0x300000 (0x1800 /2 kbytes)
		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-gnu-
		UBOOT_FILE=u-boot.bin
	;;
	"bpi-r2pro")
		FILE_DTS=arch/arm/dts/rk3568-bpi-r2-pro.dts
		FILE_DTSI=arch/arm/dts/rk3568.dtsi
		FILE_DEFCFG=bpi-r2-pro-rk3568_defconfig
		FILE_BOARD=board/rockchip/evb_rk3568/evb_rk3568.c
		FILE_SOC=include/configs/rk3568_bpi-r2-pro.h

		export BL31=files/bpi-r2pro/rk3568_bl31_v1.34.elf
		export ROCKCHIP_TPL=files/bpi-r2pro/rk3568_ddr_1560MHz_v1.13.bin

		#start-values in kB
		UBOOT_START=32
		#ENV_START=1024 #ENV_OFFSET = 0x100000
		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-gnu-
		UBOOT_FILE=u-boot-rockchip.bin
	;;
	"bpi-r3"|"bpi-r3mini")
		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-gnu-

		if [[ "$device" =~ (emmc|spi-nand|spi-nor) ]];then
			dev=emmc
		else
			dev=$device
		fi
		FILE_DEFCFG=mt7986a_bpir3_${dev}_defconfig

		DTS=mt7986a-bpi-r3-${dev}
		if [[ "$board" == "bpi-r3mini" ]];
		then
			FILE_DEFCFG=mt7986a_bpir3mini_emmc_defconfig
			DTS=mt7986a-bpi-r3-mini
		fi
		FILE_DTS=arch/arm/dts/${DTS}.dts
		FILE_DTSI=arch/arm/dts/mt7986.dtsi

		FILE_BOARD=board/mediatek/mt7986/mt7986_rfb.c
		FILE_SOC=include/configs/mt7986.h
		UBOOT_FILE=u-boot.bin
	;;
	"bpi-r4")
		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-gnu-

		if [[ "$device" =~ (emmc|spi-nand|spi-nor) ]];then
			dev=emmc
		else
			dev=$device
		fi
		FILE_DEFCFG=mt7988a_bpir4_${dev}_defconfig

		#DTS=mt7988a-${dev}-rfb
		DTS=mt7988-sd-rfb
		FILE_DTS=arch/arm/dts/${DTS}.dts
		FILE_DTSI=arch/arm/dts/mt7988.dtsi

		FILE_BOARD=board/mediatek/mt7988/mt7988_rfb.c
		FILE_SOC=include/configs/mt7988.h
		UBOOT_FILE=u-boot.bin
	;;
	*)
		echo "unsupported"
		exit 1;
	;;
esac

if [[ $ENV_START -ne 0 ]];then
	MAXSIZE=$(( ($ENV_START - $UBOOT_START) *1024 -1 ))
else
	MAXSIZE=0
fi

function generate_filename
{
	#grep '^CONFIG_MT7531=y' .config >/dev/null;if [[ $? -eq 0 ]];then ETH="MT7531";fi
	filename=u-boot-${board//bpi-/}_${uver}-${ubranch}-${ARCH}-${device}.bin
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
			if [[ -e uEnv_r3.txt.bak ]];then
				mv uEnv_r3.txt{.bak,}
			fi
		else
			echo "build failed!"
			exit 1;
		fi
	;;
	"config")
		make menuconfig;
	;;
	"importconfig")
		if [[ -n "$FILE_DEFCFG" ]];then
			echo "importing $FILE_DEFCFG"
			if [[ "$board" =~ bpi-r3(mini)? ]]; then
				rm configs/${FILE_DEFCFG}.bak 2>/dev/null
				if [[ "$device" =~ "spi" ]];then
					sed -i.bak '/^CONFIG_ENV_IS_IN_MMC/d' configs/$FILE_DEFCFG
				fi
				cp uEnv_r3.txt{,.bak}
				sed -i 's/\(bootdevice=\).*/\1'${device}'/' uEnv_r3.txt
				if [[ "$board" == "bpi-r3mini" ]];then
					sed -i 's/\(setbootconf=\).*/\1setenv bootconf "#conf-emmc-mini"/' uEnv_r3.txt
				fi
			fi
			make $FILE_DEFCFG
			if [[ -e configs/${FILE_DEFCFG}.bak ]];then
				mv configs/${FILE_DEFCFG}{.bak,}
			fi
		fi
	;;
	"defconfig")
		if [[ -n "$FILE_DEFCFG" ]];then
			edit configs/$FILE_DEFCFG;
		fi
	;;
	"install")
		if [[ $ARCH == "arm" ]] || [[ "$board" == "bpi-r2pro" ]];then
			dev=/dev/sdb
			choice=y
			read -e -i "$dev" -p "Please enter target device: " dev
			if [[ "$board" == "bpi-r2pro" ]];then
				bootpart=$(lsblk ${dev}2 -o label -n)
			else
				bootpart=$(lsblk ${dev}1 -o label -n)
			fi
			if ! [[ "$bootpart" == "BPI-BOOT" ]]; then
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
	"backup")
		DEV=/dev/sdb
		read -e -i "$DEV" -p "Please enter target device: " DEV
		case $board in
			"bpi-r2"|"bpi-r2pro")
				sudo dd if=${DEV} of=${board}_boot.img bs=1M count=2
			;;
			*)
				echo "unsupported board ${board} (bpi-r64/r3/r4 in mtk-atf branch)";
			;;
		esac
	;;
	"mount")
		mount | grep "BPI-BOOT" > /dev/null
		if [[ $? -ne 0 ]];then
			udisksctl mount -b /dev/disk/by-label/BPI-BOOT
		fi
		mount | grep "BPI-ROOT" > /dev/null
		if [[ $? -ne 0 ]];then
			udisksctl mount -b /dev/disk/by-label/BPI-ROOT
		fi
	;;
	"umount")
		umount /media/$USER/BPI-BOOT
		umount /media/$USER/BPI-ROOT
	;;
	"uenv")
		if [[ -n "$FILE_UENV" ]]; then
			edit $FILE_UENV
		fi
	;;
	"board")
		edit $FILE_BOARD
	;;
	"dts")
		edit $FILE_DTS
	;;
	"dtsi")
		edit $FILE_DTSI
	;;
	"soc")
		edit $FILE_SOC
	;;
	"upload")
		upload
	;;
	"rename")
		filename=$(generate_filename)
		echo "rename $UBOOT_FILE to $filename"
		cp $UBOOT_FILE $filename
	;;

	"createimg")
		IMGDIR=.
		if [[ "$device" != "emmc" ]];then device="sdmmc";
		else echo "emmc not supported";exit 1;fi
		IMGNAME=${board}_${device}
		REALSIZE=7000
		dd if=/dev/zero of=$IMGDIR/$IMGNAME.img bs=1M count=$REALSIZE 1> /dev/null #2>&1
		LDEV=`losetup -f`
		DEV=`echo $LDEV | cut -d "/" -f 3`     #mount image to loop device
		echo "run losetup to assign image $IMGNAME.img to loopdev $LDEV ($DEV)"
		sudo losetup $LDEV $IMGDIR/$IMGNAME.img 1> /dev/null #2>&1
		case $board in
			"bpi-r2")
				sudo parted -s $LDEV mklabel msdos
				sudo parted -s $LDEV unit MiB mkpart primary fat32 -- 100MiB 356MiB
				sudo parted -s $LDEV unit MiB mkpart primary ext4 -- 356MiB 6600MiB
				sudo partprobe $LDEV
				sudo mkfs.vfat ${LDEV}p1 -I -n BPI-BOOT
				sudo mkfs.ext4 -O ^has_journal -E stride=2,stripe-width=1024 -b 4096 ${LDEV}p2 -L BPI-ROOT

				gunzip -c files/bpi-r2/BPI-R2-HEAD440-0k.img.gz | sudo dd of=$LDEV bs=1024 seek=0
				gunzip -c files/bpi-r2/BPI-R2-HEAD1-512b.img.gz | sudo dd of=$LDEV bs=512 seek=1
				gunzip -c files/bpi-r2/BPI-R2-preloader-DDR1600-20191024-2k.img.gz | sudo dd of=$LDEV bs=1024 seek=2
				sudo dd if=u-boot.bin of=$LDEV bs=1024 seek=320
				sync
			;;
			"bpi-r2pro")
				#https://gitlab.manjaro.org/manjaro-arm/applications/manjaro-arm-tools/-/blob/master/lib/functions.sh
				sudo parted -s $LDEV mklabel gpt 1> /dev/null 2>&1
				sudo parted -s $LDEV mkpart uboot fat32 8M 16M 1> /dev/null 2>&1
				sudo parted -s $LDEV mkpart boot fat32 32M 256M 1> /dev/null 2>&1
				START=`cat /sys/block/$DEV/${DEV}p2/start`
				SIZE=`cat /sys/block/$DEV/${DEV}p2/size`
				END_SECTOR=$(expr $START + $SIZE)
				sudo parted -s $LDEV mkpart root ext4 "${END_SECTOR}s" 100% 1> /dev/null 2>&1
				sudo parted -s $LDEV set 2 esp on
				sudo partprobe $LDEV 1> /dev/null 2>&1
				sudo mkfs.vfat "${LDEV}p2" -n BPI-BOOT 1> /dev/null 2>&1
				sudo mkfs.ext4 -O ^metadata_csum,^64bit "${LDEV}p3" -L BPI-ROOT 1> /dev/null 2>&1
				sudo dd if=u-boot-rockchip.bin of=${LDEV} seek=64 conv=notrunc,fsync 1> /dev/null 2>&1
			;;
		esac
		sudo losetup -d $LDEV
		echo "packing image..."
		gzip $IMGDIR/$IMGNAME.img
	;;
	*)
		$0 build;
	;;
esac
