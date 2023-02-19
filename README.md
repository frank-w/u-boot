# U-boot for BPI-R2/R64/R2Pro/R3

![CI](https://github.com/frank-w/u-boot/workflows/CI/badge.svg?branch=2023-04-bpi)

## Requirements

On x86/x64-host you need cross compile tools for the armhf architecture:
```sh
sudo apt-get install gcc-arm-linux-gnueabihf gcc-aarch64-linux-gnu libc6-armhf-cross u-boot-tools make gcc swig python-dev python3-pyelftools
```

## Issues
- loadenv fails because of resized environment (4096 => 8188)
  - backup your saved environment before update uboot or 
    change back CONFIG_ENV_SIZE to SZ_4K (./build.sh soc)
  - erase your saved environment

```
env erase
```

- no emmc-command (not needed "emmc pconf 0x48" = "mmc partconf 0 1 1 0")

## Usage

```sh
  #edit build.conf to select bpi-r64/bpi-r2pro/bpi-r3 if needed
  ./build.sh importconfig
  ./build.sh config #optional (menuconfig)
  ./build.sh
  ./build.sh install #write to sd-card
  ./build.sh umount #umount automatic mounted partitions
```

## building and flash image

for r64, r2pro and r3 there are basic image templates created containing
the full bootchain till uboot.

- bpi_r64-sdmmc.img.gz
- bpi_r2pro.img.gz
- bpi_r3-sdmmc.img.gz

These do not contain linux kernel or rootfs.

```sh
gunzip -c bpi-r3_sdmmc.img.gz | sudo dd bs=1M status=progress conv=notrunc,fsync of=/dev/sdX
```

After this you can extract the rootfs and kernel to the card.

refresh partitiontable and mounting

```sh
sudo partprobe /dev/sdX
udisksctl mount -b /dev/disk/by-label/BPI-BOOT
udisksctl mount -b /dev/disk/by-label/BPI-ROOT
```

debian bullseye rootfs (created by buildchroot.sh in same folder):
https://drive.google.com/drive/folders/1mEcz1NLX8kv_AOKCPGGBcebRtLVNrQqF?usp=share_link

kernel:
https://github.com/frank-w/BPI-Router-Linux/releases/

```sh
# unpack debian rootfs
sudo tar -xzf bullseye_arm64.tar.gz -C /media/$USER/BPI-ROOT
# unpack kernel binary files
sudo tar -xzf bpi-r3_6.1.0-main.tar.gz --strip-components=1 -C /media/$USER/BPI-BOOT BPI-BOOT
# for r3 move kernel binary to root of boot-partition and rename it
mv /media/$USER/BPI-BOOT/bananapi/bpi-r3/linux/bpi-r3.itb /media/$USER/BPI-BOOT/bpi-r3-6.1.0.itb
echo "fit=bpi-r3-6.1.0.itb" >> /media/$USER/BPI-BOOT/uEnv.txt
# unpack kernel modules to rootfs
# debian uses /lib as symlink to usr/lib, extracting the dir from tar overwrites symlink with directory
# which contains then only the kernel-modules, but not other libs so extract the subfolder to /lib
sudo tar -xzf bpi-r3_6.1.0-main.tar.gz --strip-components=2 -C /media/$USER/BPI-ROOT/lib/ BPI-ROOT/lib/
```
R2 uses uImage and kernel=xxx in uEnv.txt (in folder bananapi/bpi-r2/linux)
R64 uses fit like r3
R2Pro uses Image.gz+dtb in extlinux folder + config for each entry,no uEnv.txt here.

set root-password and maybe make additional changes:

```sh
sudo chroot /media/$USER/BPI-ROOT
passwd
echo "bpi-r3" > /etc/hostname
```
/etc/fstab
```sh
# <file system>	<dir>	<type>	<options>		<dump>	<pass>
/dev/mmcblk0p5	/boot	vfat    errors=remount-ro	0	1
/dev/mmcblk0p6	/	ext4	defaults		0	0
```

maybe add network-config (systemd) and systemd services i uploaded here:
https://drive.google.com/drive/folders/1kST9ZOv8xQWFfo9GUNIIpKjD8QuYMj8i?usp=share_link
