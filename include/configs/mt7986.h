/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7986 SoC
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#ifndef __MT7986_H
#define __MT7986_H

#include <linux/sizes.h>

#define CONFIG_SYS_NONCACHED_MEMORY	SZ_1M
#define CONFIG_SYS_MMC_ENV_DEV		0

/* DRAM */
#define CFG_SYS_SDRAM_BASE		0x40000000

#endif
