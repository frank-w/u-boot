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
git clone git@github.com:brucerry/BPI-Router-Uboot.git -b 2025-10-bpi
cd BPI-Router-Uboot
```

## Steps

1. Run script

```
./run.sh bpi-r4
```

2. Find output

```
michael@michael-HP-ProBook-430-G5:~/BPI-Router-Uboot$ ll u-boot* bpi*
-rw-rw-r-- 1 michael michael  250190 Oct 13 11:18 bpi-r4_emmc_8GB_bl2.img
-rw-rw-r-- 1 michael michael  375265 Oct 13 11:18 bpi-r4_emmc_8GB_fip.bin
-rw-rw-r-- 1 michael michael  250190 Oct 13 10:54 bpi-r4_emmc_bl2.img
-rw-rw-r-- 1 michael michael  375265 Oct 13 10:54 bpi-r4_emmc_fip.bin
-rw-rw-r-- 1 michael michael 7952486 Oct 13 10:56 bpi-r4_emmc.img.gz
-rwxrwxr-x 1 michael michael 8317496 Oct 13 10:54 u-boot*
-rw-rw-r-- 1 michael michael  941936 Oct 13 10:54 u-boot.bin
-rw-rw-r-- 1 michael michael  329408 Oct 13 10:54 u-boot.bin.xz
-rw-rw-r-- 1 michael michael   13972 Oct 13 10:54 u-boot.cfg
-rw-rw-r-- 1 michael michael   22696 Oct 13 10:54 u-boot.dtb
-rw-rw-r-- 1 michael michael  941936 Oct 13 10:54 u-boot-dtb.bin
-rw-rw-r-- 1 michael michael    1315 Oct 13 10:54 u-boot.lds
-rw-rw-r-- 1 michael michael 1158959 Oct 13 10:54 u-boot.map
-rwxrwxr-x 1 michael michael  919240 Oct 13 10:54 u-boot-nodtb.bin*
-rw-rw-r-- 1 michael michael  941936 Oct 13 10:54 u-boot-r4_2025.10-bpi-arm64-emmc.bin
-rwxrwxr-x 1 michael michael 2752730 Oct 13 10:54 u-boot.srec*
-rw-rw-r-- 1 michael michael  324179 Oct 13 10:54 u-boot.sym
michael@michael-HP-ProBook-430-G5:~/BPI-Router-Uboot$ 
```
