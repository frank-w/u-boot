// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 */

#include <stddef.h>
#include <stdint.h>
#include <boot_spi.h>
#include <mtk_spi.h>

#define FIP_BASE			0x580000
#define FIP_SIZE			0x200000

#define MTK_QSPI_SRC_CLK		CB_MPLL_D2

#ifdef SPIM_NAND_PREFER_SPI2
#define DEFAULT_QSPI_IF		2
#else
#define DEFAULT_QSPI_IF		0
#endif

static uint32_t curr_qspi_if = DEFAULT_QSPI_IF;

int mtk_plat_qspi_init(void)
{
	/* config GPIO pinmux to spi mode */
	mtk_spi_gpio_init(curr_qspi_if == 2 ? SPIM2 : SPIM0);

	/* select 208M clk */
	mtk_spi_source_clock_select(MTK_QSPI_SRC_CLK);

	return mtk_qspi_init_by_path(
		curr_qspi_if == 2 ? "/soc/spi@11009800" : "/soc/spi@11007800",
		MTK_QSPI_SRC_CLK);
}

#ifndef SPIM_NAND_NO_RETRY
static bool qspi_if_switched;

int mtk_plat_switch_qspi_if(void)
{
	if (qspi_if_switched)
		return -ENODEV;

	qspi_if_switched = true;

	if (curr_qspi_if == 2)
		curr_qspi_if = 0;
	else
		curr_qspi_if = 2;

	NOTICE("Switched to SPI%u for probing\n", curr_qspi_if);

	return 0;
}
#endif

void mtk_plat_fip_location(size_t *fip_off, size_t *fip_size)
{
	*fip_off = FIP_BASE;
	*fip_size = FIP_SIZE;
}
