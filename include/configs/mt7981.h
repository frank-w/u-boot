/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT7981 SoC
 *
 * Copyright (C) 2022 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#ifndef __MT7981_H
#define __MT7981_H

#include <linux/sizes.h>

#define CONFIG_SYS_NONCACHED_MEMORY	SZ_1M
#define CONFIG_SYS_MMC_ENV_DEV		0

/* Uboot definition */
#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* SPL -> Uboot */
#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE

/* DRAM */
#define CONFIG_SYS_SDRAM_BASE		0x40000000

/* Ethernet */
#define CONFIG_IPADDR			192.168.1.1
#define CONFIG_SERVERIP			192.168.1.2
#define CONFIG_NETMASK			255.255.255.0

#endif
