// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common/debug.h>
#include <mtk_spi_nand.h>
#include <mtk_spi.h>
#include "bl2_plat_setup.h"

#pragma weak mtk_plat_switch_qspi_if
int mtk_plat_switch_qspi_if(void)
{
	return -ENOTSUP;
}

int mtk_plat_nand_setup(size_t *page_size, size_t *block_size, uint64_t *size)
{
	struct nand_device *nand_dev;
	unsigned long long chip_size;
	unsigned int erase_size;
	int ret;

	mtk_qspi_setup_buffer((void *)QSPI_BUF_OFFSET);

	while (true) {
		ret = mtk_plat_qspi_init();
		if (ret) {
			ERROR("Failed to initialize SPI controller, error %d\n",
			      ret);
			return ret;
		}

		ret = spi_nand_init(&chip_size, &erase_size);
		if (!ret)
			break;

		if (ret == -ENODEV) {
			ret = mtk_plat_switch_qspi_if();
			if (!ret)
				continue;

			ERROR("No SPI-NAND flash present\n");
			return ret;
		}

		if (ret != -ENOENT)
			ERROR("Failed to initialize SPI-NAND, error %d\n", ret);

		return ret;
	}

	nand_dev = get_nand_device();

	if (page_size)
		*page_size = nand_dev->page_size;

	if (block_size)
		*block_size = erase_size;

	if (size)
		*size = chip_size;

	return 0;
}
