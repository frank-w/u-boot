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

#values in kB
if [[ "$board" == "bpi-r64" ]];then
	UBOOT_START=768
	UBOOT_FILE=u-boot-mtk.bin
	ENV_START=1280 #ENV_OFFSET = 0x140000
	#official r64-patches are arm64
	if [[ "$arch" == "arm64" ]];then
		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-gnu-
		UBOOT_FILE=u-boot.bin
		#~40kb bl31+~640kb uboot =~ 682kb fip @0x160000 <0x300000
		UBOOT_START=1064 #1024k + 40k
		ENV_START=3072 #ENV_OFFSET (bytes) = 0x300000 (0x1800 /2 kbytes)
	fi
else
	UBOOT_START=320
	UBOOT_FILE=u-boot.bin
	ENV_START=1024 #ENV_OFFSET = 0x100000
fi
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
			if [[ $FILESIZE -gt $MAXSIZE ]]; then
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
		if [[ "$board" == "bpi-r64" ]];then
			if [[ "$arch" == "arm64" ]];then
				DEFCONFIG=mt7622_bpi-r64_defconfig
			else
				DEFCONFIG=mt7622_bpi-r64_32_defconfig
			fi
		else
			DEFCONFIG=mt7623n_bpir2_defconfig;
		fi
		if [[ -n "$DEFCONFIG" ]];then
			echo "importing $DEFCONFIG"
			make $DEFCONFIG
		fi
	;;
	"defconfig")
		if [[ "$board" == "bpi-r64" ]];then
			if [[ "$arch" == "arm64" ]];then
				nano configs/mt7622_bpi-r64_defconfig;
			else
				nano configs/mt7622_bpi-r64_32_defconfig;
			fi
		else
			nano configs/mt7623n_bpir2_defconfig;
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
		if [[ "$board" == "bpi-r64" ]];then
			nano /media/$USER/BPI-BOOT/bananapi/bpi-r64/linux/uEnv.txt
		else
			nano /media/$USER/BPI-BOOT/bananapi/bpi-r2/linux/uEnv.txt
		fi
	;;
	"board")
		if [[ "$board" == "bpi-r64" ]];then
			nano board/mediatek/mt7622/mt7622_rfb.c
		else
			nano board/mediatek/mt7623/mt7623_rfb.c
		fi
	;;
	"dts")
		if [[ "$board" == "bpi-r64" ]];then
			if [[ "$arch" == "arm64" ]];then
				nano arch/arm/dts/mt7622-rfb.dts
			else
				nano arch/arm/dts/mt7622-bananapi-bpi-r64.dts
			fi
		else
			nano arch/arm/dts/mt7623n-bananapi-bpi-r2.dts
		fi
	;;
	"dtsi")
		if [[ "$board" == "bpi-r64" ]];then
			nano arch/arm/dts/mt7622.dtsi
		else
			nano arch/arm/dts/mt7623.dtsi
		fi
	;;
	"soc")
		if [[ "$board" == "bpi-r64" ]];then
			nano include/configs/mt7622.h
		else
			nano include/configs/mt7623.h
		fi
	;;
	"upload")
		upload
	;;
	"rename")
		filename=$(generate_filename)
		echo "rename $UBOOT_FILE to $filename"
		mv $UBOOT_FILE $filename
	;;
	"createimg")
		IMGDIR=.
		IMGNAME=$board
		REALSIZE=6000
		dd if=/dev/zero of=$IMGDIR/$IMGNAME.img bs=1M count=$REALSIZE 1> /dev/null #2>&1
		LDEV=`losetup -f`
		DEV=`echo $LDEV | cut -d "/" -f 3`     #mount image to loop device
		echo "run losetup to assign image $IMGNAME to loopdev $LDEV ($DEV)"
		sudo losetup $LDEV $IMGDIR/$IMGNAME.img 1> /dev/null #2>&1
		case $board in
			"bpi-r2pro")
				#https://gitlab.manjaro.org/manjaro-arm/applications/manjaro-arm-tools/-/blob/master/lib/functions.sh
				sudo parted -s $LDEV mklabel gpt 1> /dev/null 2>&1
				sudo parted -s $LDEV mkpart uboot fat32 8M 16M 1> /dev/null 2>&1
				sudo parted -s $LDEV mkpart primary fat32 32M 256M 1> /dev/null 2>&1
				START=`cat /sys/block/$DEV/${DEV}p2/start`
				SIZE=`cat /sys/block/$DEV/${DEV}p2/size`
				END_SECTOR=$(expr $START + $SIZE)
				sudo parted -s $LDEV mkpart primary ext4 "${END_SECTOR}s" 100% 1> /dev/null 2>&1
				sudo parted -s $LDEV set 2 esp on
				sudo partprobe $LDEV 1> /dev/null 2>&1
				sudo mkfs.vfat "${LDEV}p2" -n BPI-BOOT 1> /dev/null 2>&1
				sudo mkfs.ext4 -O ^metadata_csum,^64bit "${LDEV}p3" -L BPI-ROOT 1> /dev/null 2>&1
				sudo dd if=files/$board/idblock.bin of=${LDEV} seek=64 conv=notrunc,fsync 1> /dev/null 2>&1
				sudo dd if=files/$board/uboot.img of=${LDEV}p1 conv=notrunc,fsync 1> /dev/null 2>&1
			;;
		esac
		sudo losetup -d $LDEV
	;;
	*)
		$0 build;
	;;
esac
