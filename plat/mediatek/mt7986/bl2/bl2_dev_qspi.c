/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <boot_spi.h>
#include <mtk_spi.h>

#define MTK_QSPI_SRC_CLK		CLK_MPLL_D2

int mtk_plat_qspi_init(void)
{
	/* config GPIO pinmux to spi mode */
	mtk_spi_gpio_init();

	/* select 208M clk */
	mtk_spi_source_clock_select(MTK_QSPI_SRC_CLK);

	return mtk_qspi_init(MTK_QSPI_SRC_CLK);
}
