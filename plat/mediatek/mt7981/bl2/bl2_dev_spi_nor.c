/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>
#include <boot_spi.h>
#include <mtk_spi.h>

#define FIP_BASE			0x100000
#define FIP_SIZE			0x80000

#define MTK_QSPI_SRC_CLK		CB_MPLL_D2

int mtk_plat_qspi_init(void)
{
	/* config GPIO pinmux to spi mode */
	mtk_spi_gpio_init(SPIM2);

	/* select 208M clk */
	mtk_spi_source_clock_select(MTK_QSPI_SRC_CLK);

	return mtk_qspi_init(MTK_QSPI_SRC_CLK);
}

void mtk_plat_fip_location(size_t *fip_off, size_t *fip_size)
{
	*fip_off = FIP_BASE;
	*fip_size = FIP_SIZE;
}
