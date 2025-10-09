## Setup the Workspace in Native Linux Distro

It can be shown that Docker would produce the `/dev/loop*` related errors for "`./build.sh createimg`" command in branch `mtk-atf-2025`.

The build is tested in Debian/Ubuntu/LinuxMint system:

```
cat /etc/os-release

NAME="Linux Mint"
VERSION="21.1 (Vera)"
ID=linuxmint
ID_LIKE="ubuntu debian"
PRETTY_NAME="Linux Mint 21.1"
VERSION_ID="21.1"
HOME_URL="https://www.linuxmint.com/"
SUPPORT_URL="https://forums.linuxmint.com/"
BUG_REPORT_URL="http://linuxmint-troubleshooting-guide.readthedocs.io/en/latest/"
PRIVACY_POLICY_URL="https://www.linuxmint.com/"
VERSION_CODENAME=vera
UBUNTU_CODENAME=jammy
```

Install packages:

```
sudo apt install -y \
xxd gdisk dosfstools \
gcc-arm-linux-gnueabihf gcc-aarch64-linux-gnu libc6-armhf-cross u-boot-tools make gcc swig python-dev-is-python3 python3-pyelftools \
gcc-aarch64-linux-gnu u-boot-tools bc make gcc ccache libc6-dev libncurses5-dev libssl-dev bison flex
```

Clone:

```
cd ~
git clone git@github.com:brucerry/BPI-Router-Uboot.git -b 2025-07-bpi
cd BPI-Router-Uboot
```

## Steps

1. Run script

```
./run.sh bpi-r3
```

2. Find output

```
ubuntu@afe0fd51a26a:~/BPI-Router-Uboot$ ll u-boot* bpi*
-rw-r--r-- 1 ubuntu ubuntu 7588105 Sep 26 17:33 bpi-r3_emmc.img.gz
-rw-r--r-- 1 ubuntu ubuntu  200793 Sep 26 17:33 bpi-r3_emmc_bl2.img
-rw-r--r-- 1 ubuntu ubuntu  280761 Sep 26 17:33 bpi-r3_emmc_fip.bin
-rwxr-xr-x 1 ubuntu ubuntu 6064000 Sep 26 17:28 u-boot*
-rw-r--r-- 1 ubuntu ubuntu  681072 Sep 26 17:28 u-boot-dtb.bin
-rwxr-xr-x 1 ubuntu ubuntu  667432 Sep 26 17:28 u-boot-nodtb.bin*
-rw-r--r-- 1 ubuntu ubuntu  681072 Sep 26 17:28 u-boot-r3_2025.07-bpi-arm64-emmc.bin
-rw-r--r-- 1 ubuntu ubuntu  681072 Sep 26 17:28 u-boot.bin
-rw-r--r-- 1 ubuntu ubuntu  243072 Sep 26 17:28 u-boot.bin.xz
-rw-r--r-- 1 ubuntu ubuntu   12546 Sep 26 17:28 u-boot.cfg
-rw-r--r-- 1 ubuntu ubuntu   13640 Sep 26 17:28 u-boot.dtb
-rw-r--r-- 1 ubuntu ubuntu    1315 Sep 26 17:28 u-boot.lds
-rw-r--r-- 1 ubuntu ubuntu  938726 Sep 26 17:28 u-boot.map
-rwxr-xr-x 1 ubuntu ubuntu 1997290 Sep 26 17:28 u-boot.srec*
-rw-r--r-- 1 ubuntu ubuntu  251269 Sep 26 17:28 u-boot.sym
-rw-r--r-- 1 ubuntu ubuntu  658384 Sep 26 17:29 u-boot_mt7988.bin
ubuntu@afe0fd51a26a:~/BPI-Router-Uboot$ 
```