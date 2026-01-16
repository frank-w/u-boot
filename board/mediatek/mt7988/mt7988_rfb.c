// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <env.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/global_data.h>

#define	MT7988_BOOT_NOR		0
#define	MT7988_BOOT_SPIM_NAND	1
#define	MT7988_BOOT_EMMC	2
#define	MT7988_BOOT_SNFI_NAND	3

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

void getBootDevice(void)
{
	const char *media;

	printf("=> board_late_init...\n");
	switch ((readl(0x1001f6f0) & 0xc00) >> 10) {
	case MT7988_BOOT_NOR:
		media = "nor";
		break
		;;
	case MT7988_BOOT_SPIM_NAND:
		media = "spim-nand";
		break
		;;
	case MT7988_BOOT_EMMC:
		media = "emmc";
		break
		;;
	case MT7988_BOOT_SNFI_NAND:
		media = "sd";
		break
		;;
	}

	printf("bootmedia:%s\n",media);
	env_set("bootmedia",media);
}

void getRAMSize(void)
{
	if (gd->ram_size > (6ULL << 30))
		env_set("ram_gb", "8");
	else
		env_set("ram_gb", "4");
}

int board_late_init(void)
{
	getBootDevice();
	getRAMSize();
	return 0;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	const u32 *media_handle_p;
	int chosen, len, ret;
	char media[20];
	const char *bootdev;
	u32 media_handle;

#ifndef CONFIG_BOARD_LATE_INIT
	getBootDevice();
#endif
	bootdev=env_get("bootmedia");
	sprintf(media,"rootdisk-%s",bootdev);

	chosen = fdt_path_offset(blob, "/chosen");
	if (chosen <= 0)
		return 0;

	media_handle_p = fdt_getprop(blob, chosen, media, &len);
	if (media_handle_p <= 0 || len != 4)
		return 0;

	media_handle = *media_handle_p;
	ret = fdt_setprop(blob, chosen, "rootdisk", &media_handle, sizeof(media_handle));
	if (ret) {
		printf("cannot set media phandle %s as rootdisk /chosen node\n", media);
		return ret;
	}

	printf("set /chosen/rootdisk to bootrom media: %s (phandle 0x%08x)\n", media, fdt32_to_cpu(media_handle));

	return 0;
}
