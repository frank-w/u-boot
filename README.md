# ATF for BPI-R64/BPI-R3

based on https://github.com/mtk-openwrt/arm-trusted-firmware

https://forum.banana-pi.org/t/tutorial-build-customize-and-use-mediatek-open-source-u-boot-and-atf/13785

build u-boot first

create a build.conf containing board and maybe bootdevice (if not sd)

board=[bpi-r64|bpi-r3]
device=[sdmmc|emmc|spim-nand|nor]

then run

```sh
./build.sh
./build.sh rename
```

and for sdcard-creation maybe

```sh
./build.sh rename
```

sdcard-image can be flashed with this command:

```sh
gunzip -c bpi-r64_sdmmc.img.gz | sudo dd of=/dev/sdb bs=1M status=progress
```
