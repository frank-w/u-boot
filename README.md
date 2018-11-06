# U-boot for BPI-R2

## Requirements

On x86/x64-host you need cross compile tools for the armhf architecture:
```sh
sudo apt-get install gcc-arm-linux-gnueabihf libc6-armhf-cross u-boot-tools make gcc
#v3+ patches need swig and Python.h
sudo apt-get install swig python-dev
```

## Issues
- no ethernet-driver
- saveenv only saves to emmc (because i need to set offset for SD-Card see https://lists.denx.de/pipermail/u-boot/2018-October/345667.html)
- no emmc-command (not needed "emmc pconf 0x48" = "mmc partconf 0 1 1 0")

## Usage

```sh
  ./build.sh importconfig
  ./build.sh config #optional (menuconfig)
  ./build.sh
  ./build.sh install #write to sd-card
```
