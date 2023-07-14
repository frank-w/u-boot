/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7988 SoC
 *
 * Copyright (C) 2021 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#ifndef __MT7988_H
#define __MT7988_H

#include <linux/sizes.h>

/* Size of malloc() pool */
#define CONFIG_SYS_NONCACHED_MEMORY	SZ_1M

#define CONFIG_SYS_MMC_ENV_DEV		0

/* DRAM */
#define CFG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_VERY_BIG_RAM
#define CONFIG_MAX_MEM_MAPPED		0xC0000000

#endif
