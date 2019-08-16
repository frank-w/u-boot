# U-boot for BPI-R2

## Requirements

On x86/x64-host you need cross compile tools for the armhf architecture:
```sh
sudo apt-get install gcc-arm-linux-gnueabihf libc6-armhf-cross u-boot-tools make gcc swig python-dev
```

## Issues
- loadenv failes because of resized environment (4096 => 8188)
  - backup your saved environment before update uboot or 
    change back CONFIG_ENV_SIZE to SZ_4K (./build.sh soc)
  - erase your saved environment

```
mmc erase 800 10
```

- no emmc-command (not needed "emmc pconf 0x48" = "mmc partconf 0 1 1 0")

## Usage

```sh
  ./build.sh importconfig
  ./build.sh config #optional (menuconfig)
  ./build.sh
  ./build.sh install #write to sd-card
  ./build.sh umount #umount automatic mounted partitions
```
