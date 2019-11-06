/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7623 SoC
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef __MT7623_H
#define __MT7623_H

#include <linux/sizes.h>

/* Miscellaneous configurable options */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_TAG

#define CONFIG_SYS_MAXARGS		32
#define CONFIG_SYS_BOOTM_LEN		SZ_64M
#define CONFIG_SYS_CBSIZE		SZ_1K
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE +	\
					sizeof(CONFIG_SYS_PROMPT) + 16)

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		SZ_4M
#define CONFIG_SYS_NONCACHED_MEMORY	SZ_1M

/* Environment */
#ifdef BPI
#define CONFIG_ENV_SIZE			SZ_4K
#else
#define CONFIG_ENV_SIZE			0x10000
#endif
/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Preloader -> Uboot */
#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + SZ_2M - \
					 GENERATED_GBL_DATA_SIZE)
/* BPI */
#ifdef BPI
#define CONFIG_CMD_BOOTEFI
#define CONFIG_CMD_BOOTEFI_HELLO_COMPILE
#endif

/* UBoot -> Kernel */
#define SDRAM_OFFSET(x) 0x8##x
/* DRAM */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_LOAD_ADDR		0x82000000 /* default load address */

/* MMC */
#define MMC_SUPPORTS_TUNING

#define SCRIPT_BOOT \
	"fileload=${mmctype}load mmc ${devnum}:${mmcpart} " \
		"${loadaddr} ${mmcfile}\0" \
	"fileload2=${mmctype}load mmc ${devnum}:2 " \
		"${loadaddr} ${mmcfile}\0" \
	"kernload=setenv loadaddr ${kernel_addr_r};" \
		"setenv mmcfile ${mmckernfile};" \
		"run fileload\0" \
	"initrdload=setenv loadaddr ${rdaddr};" \
		"setenv mmcfile ${mmcinitrdfile};" \
		"run fileload\0" \
	"fdtload=setenv loadaddr ${fdtaddr};" \
		"setenv mmcfile ${mmcfdtfile};" \
		"run fileload\0" \
	"fdtload2=setenv loadaddr ${fdtaddr};" \
		"setenv mmcfile ${mmcfdtfile};" \
		"run fileload2\0" \
	"scriptload=setenv loadaddr ${scriptaddr};" \
		"setenv mmcfile ${mmcscriptfile};" \
		"run fileload\0" \
	"scriptboot=echo Running ${mmcscriptfile} from: mmc ${devnum}:${mmcpart} using ${mmcscriptfile};" \
		"source ${scriptaddr};" \

/* Extra environment variables */
#define MTK_ENV_SETTINGS	\
	"loadaddr=0x82000000\0" \
	"kernel_addr_r=0x82000000\0" \
	"scriptaddr=0x85F80000\0" \
	"fdtaddr=0x86000000\0" \
	"fdt_addr_r=0x86000000\0" \
	"rdaddr=0x86080000\0" \
	"ramdisk_addr_r=0x86080000\0" \
	"bootm_size=0x10000000\0" \
	"mmckernfile=boot/zImage\0" \
	"mmcinitrdfile= boot/uInitrd\0" \
	"mmcfdtfile=boot/dtb/mt7623n-bananapi-bpi-r2.dtb\0" \
	"mmcscriptfile=boot/boot.scr\0" \
	"mmctype=ext4\0" \
	"devnum=1\0" \
	"mmcpart=1\0" \
	"mmc_bootdev=1\0" \
	SCRIPT_BOOT

#ifdef BPI
#define CONFIG_BOOTCOMMAND \
	"mmc dev 1;" \
	"run scriptload;" \
	"run scriptboot;" \
	"setenv devnum 0;" \
	"mmc dev 0;" \
	"run scriptload;" \
	"run scriptboot"
#endif

/* Ethernet */
#define CONFIG_IPADDR			192.168.1.1
#define CONFIG_SERVERIP			192.168.1.2

#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_OFFSET		0x100000


#ifndef CONFIG_SPL_BUILD

/*
 * 160M RAM (256M minimum minus 64MB heap + 32MB for u-boot, stack, fb, etc.
 * 32M uncompressed kernel, 16M compressed kernel, 1M fdt,
 * 1M script, 1M pxe and the ramdisk at the end.
	"bootm_size=0x10000000\0"
 */
#define BOOTM_SIZE     __stringify(0xa000000)
#define KERNEL_ADDR_R  __stringify(SDRAM_OFFSET(2000000))
#define FDT_ADDR_R     __stringify(SDRAM_OFFSET(6000000))
#define SCRIPT_ADDR_R  __stringify(SDRAM_OFFSET(5F80000))
#define PXEFILE_ADDR_R __stringify(SDRAM_OFFSET(5200000))
#define RAMDISK_ADDR_R __stringify(SDRAM_OFFSET(6080000))

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=" BOOTM_SIZE "\0" \
	"kernel_addr_r=" KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" FDT_ADDR_R "\0" \
	"scriptaddr=" SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" PXEFILE_ADDR_R "\0" \
	"ramdisk_addr_r=" RAMDISK_ADDR_R "\0"

#define DFU_ALT_INFO_RAM \
	"dfu_alt_info_ram=" \
	"kernel ram " KERNEL_ADDR_R " 0x1000000;" \
	"fdt ram " FDT_ADDR_R " 0x100000;" \
	"ramdisk ram " RAMDISK_ADDR_R " 0x4000000\0"

#ifdef CONFIG_MMC
#if CONFIG_MMC_SUNXI_SLOT_EXTRA != -1
#define BOOTENV_DEV_MMC_AUTO(devtypeu, devtypel, instance)		\
	BOOTENV_DEV_MMC(MMC, mmc, 0)					\
	BOOTENV_DEV_MMC(MMC, mmc, 1)					\
	"bootcmd_mmc_bpi=setenv devnum ${mmc_bootdev}; run mmc_boot;\0"	\
	"bootcmd_mmc0=setenv devnum 0; run fdtload2; run fdtload; run mmc_boot;\0"			\
	"bootcmd_mmc1=setenv devnum 1; run fdtload2; run fdtload; run mmc_boot;\0"			\
	"bootcmd_mmc_auto="						\
		"if test ${mmc_bootdev} -eq 1; then "			\
			"run bootcmd_mmc1; "				\
			"run bootcmd_mmc0; "				\
		"elif test ${mmc_bootdev} -eq 0; then "			\
			"run bootcmd_mmc0; "				\
			"run bootcmd_mmc1; "				\
		"fi\0"

#define BOOTENV_DEV_NAME_MMC_AUTO(devtypeu, devtypel, instance) \
	"mmc_auto "

#define BOOT_TARGET_DEVICES_MMC(func) func(MMC_AUTO, mmc_auto, na)
#else
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0)
#endif
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_AHCI
#define BOOT_TARGET_DEVICES_SCSI(func) func(SCSI, scsi, 0)
#else
#define BOOT_TARGET_DEVICES_SCSI(func)
#endif

#ifdef CONFIG_USB_STORAGE
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#ifdef CONFIG_CMD_PXE
#define BOOT_TARGET_DEVICES_PXE(func) func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#ifdef CONFIG_CMD_DHCP
#define BOOT_TARGET_DEVICES_DHCP(func) func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

/* FEL boot support, auto-execute boot.scr if a script address was provided */
#define BOOTENV_DEV_FEL(devtypeu, devtypel, instance) \
	"bootcmd_fel=" \
		"if test -n ${fel_booted} && test -n ${fel_scriptaddr}; then " \
			"echo '(FEL boot)'; " \
			"source ${fel_scriptaddr}; " \
		"fi\0"
#define BOOTENV_DEV_NAME_FEL(devtypeu, devtypel, instance) \
	"fel "

#define BOOT_TARGET_DEVICES(func) \
	func(FEL, fel, na) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_SCSI(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)

#ifdef CONFIG_OLD_SUNXI_KERNEL_COMPAT
/* BPI */
#define BOOTCMD_SUNXI_COMPAT \
	"bootcmd_sunxi_compat=" \
		"setenv root /dev/mmcblk0p2 rootwait; " \
		"if fatload mmc 0 0x44000000 ${bpi}/${board}/${service}/uEnv.txt; then " \
			"echo Loaded environment from uEnv.txt; " \
			"env import -t 0x44000000 ${filesize}; " \
		"fi; " \
		"setenv root /dev/mmcblk0p3 rootwait; " \
		"if ext2load mmc 0 0x44000000 uEnv.txt; then " \
			"echo Loaded environment from uEnv.txt; " \
			"env import -t 0x44000000 ${filesize}; " \
		"fi; " \
		"setenv bootargs console=${console} root=${root} ${extraargs}; " \
		"ext2load mmc 0 0x43000000 script.bin && " \
		"ext2load mmc 0 0x48000000 uImage && " \
		"bootm 0x48000000\0"
#else
#define BOOTCMD_SUNXI_COMPAT
#endif

#include <config_distro_bootcmd.h>

#ifdef CONFIG_USB_KEYBOARD
#define CONSOLE_STDIN_SETTINGS \
	"preboot=usb start\0" \
	"stdin=serial,usbkbd\0"
#else
#define CONSOLE_STDIN_SETTINGS \
	"stdin=serial\0"
#endif

#ifdef CONFIG_VIDEO
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial,vga\0" \
	"stderr=serial,vga\0"
#elif CONFIG_DM_VIDEO
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0"
#else
#define CONSOLE_STDOUT_SETTINGS \
	"stdout=serial\0" \
	"stderr=serial\0"
#endif

#ifdef CONFIG_MTDIDS_DEFAULT
#define SUNXI_MTDIDS_DEFAULT \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0"
#else
#define SUNXI_MTDIDS_DEFAULT
#endif

#ifdef CONFIG_MTDPARTS_DEFAULT
#define SUNXI_MTDPARTS_DEFAULT \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"
#else
#define SUNXI_MTDPARTS_DEFAULT
#endif

#define PARTS_DEFAULT \
	"name=loader1,start=8k,size=32k,uuid=${uuid_gpt_loader1};" \
	"name=loader2,size=984k,uuid=${uuid_gpt_loader2};" \
	"name=esp,size=128M,bootable,uuid=${uuid_gpt_esp};" \
	"name=system,size=-,uuid=${uuid_gpt_system};"

#define UUID_GPT_ESP "c12a7328-f81f-11d2-ba4b-00a0c93ec93b"

#ifdef CONFIG_ARM64
#define UUID_GPT_SYSTEM "b921b045-1df0-41c3-af44-4c6f280d3fae"
#else
#define UUID_GPT_SYSTEM "69dad710-2ce4-4e3c-b16c-21a1d49abed3"
#endif

#define CONSOLE_ENV_SETTINGS \
	CONSOLE_STDIN_SETTINGS \
	CONSOLE_STDOUT_SETTINGS

#ifdef CONFIG_ARM64
#define FDTFILE "allwinner/" CONFIG_DEFAULT_DEVICE_TREE ".dtb"
#else
#define FDTFILE CONFIG_DEFAULT_DEVICE_TREE ".dtb"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	MTK_ENV_SETTINGS \
	CONSOLE_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	DFU_ALT_INFO_RAM \
	"fdtfile=" FDTFILE "\0" \
	"console=ttyS0,115200\0" \
	SUNXI_MTDIDS_DEFAULT \
	SUNXI_MTDPARTS_DEFAULT \
	"uuid_gpt_esp=" UUID_GPT_ESP "\0" \
	"uuid_gpt_system=" UUID_GPT_SYSTEM "\0" \
	"partitions=" PARTS_DEFAULT "\0" \
	BOOTCMD_SUNXI_COMPAT \
	BOOTENV

#else /* ifndef CONFIG_SPL_BUILD */
#define CONFIG_EXTRA_ENV_SETTINGS
#endif

#endif
