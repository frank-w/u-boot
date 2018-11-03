# U-boot for BPI-R2

## Requirements

On x86/x64-host you need cross compile tools for the armhf architecture:
```sh
sudo apt-get install gcc-arm-linux-gnueabihf libc6-armhf-cross u-boot-tools make gcc
```

## Issues
- no ethernet-driver
- saveenv only saves to emmc (because i need to set offset for SD-Card see https://lists.denx.de/pipermail/u-boot/2018-October/345667.html)
- no emmc-command

## Usage

```sh
  ./build.sh importconfig
  ./build.sh config
  ./build.sh
  ./build.sh install
```
