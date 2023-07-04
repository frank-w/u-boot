// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common/debug.h>
#include <drivers/spi_nor.h>
#include <mtk_spi.h>
#include "bl2_plat_setup.h"

int mtk_plat_nor_setup(void)
{
	unsigned long long size;
	int ret;

	mtk_qspi_setup_buffer((void *)QSPI_BUF_OFFSET);

	ret = mtk_plat_qspi_init();
	if (ret) {
		ERROR("Failed to initialize SPI controller, error %d\n", ret);
		return ret;
	}

	ret = spi_nor_init(&size, NULL);
	if (ret) {
		ERROR("spi_nor_init() failed with %d\n", ret);
		return ret;
	}

	return 0;
}
