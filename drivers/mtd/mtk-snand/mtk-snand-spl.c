// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/uclass.h>
#include <malloc.h>
#include <mapmem.h>
#include <mtd.h>
#include <watchdog.h>

#include "mtk-snand.h"

static struct mtk_snand *snf;
static struct mtk_snand_chip_info cinfo;
static u32 oobavail;

static u8 *page_cache;

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst)
{
	u32 sizeremain = size, chunksize, leading;
	uint32_t off = offs, writesize_mask = cinfo.pagesize - 1;
	uint8_t *ptr = dst;
	int ret;

	if (!snf)
		return -ENODEV;

	while (sizeremain) {
		WATCHDOG_RESET();

		leading = off & writesize_mask;
		chunksize = cinfo.pagesize - leading;
		if (chunksize > sizeremain)
			chunksize = sizeremain;

		if (chunksize == cinfo.pagesize) {
			ret = mtk_snand_read_page(snf, off - leading, ptr,
						  NULL, false);
			if (ret)
				break;
		} else {
			ret = mtk_snand_read_page(snf, off - leading,
						  page_cache, NULL, false);
			if (ret)
				break;

			memcpy(ptr, page_cache + leading, chunksize);
		}

		off += chunksize;
		ptr += chunksize;
		sizeremain -= chunksize;
	}

	return ret;
}

void nand_init(void)
{
	struct mtk_snand_platdata mtk_snand_pdata = {};
	struct udevice *dev;
	fdt_addr_t base;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD, DM_DRIVER_GET(mtk_snand),
					  &dev);
	if (ret) {
		printf("mtk-snand-spl: Device instance not found!\n");
		return;
	}

	base = dev_read_addr_name(dev, "nfi");
	if (base == FDT_ADDR_T_NONE) {
		printf("mtk-snand-spl: NFI base not set\n");
		return;
	}
	mtk_snand_pdata.nfi_base = map_sysmem(base, 0);

	base = dev_read_addr_name(dev, "ecc");
	if (base == FDT_ADDR_T_NONE) {
		printf("mtk-snand-spl: ECC base not set\n");
		return;
	}
	mtk_snand_pdata.ecc_base = map_sysmem(base, 0);

	mtk_snand_pdata.soc = dev_get_driver_data(dev);
	mtk_snand_pdata.quad_spi = dev_read_bool(dev, "quad-spi");

	ret = mtk_snand_init(NULL, &mtk_snand_pdata, &snf);
	if (ret) {
		printf("mtk-snand-spl: failed to initialize SPI-NAND\n");
		return;
	}

	mtk_snand_get_chip_info(snf, &cinfo);

	oobavail = cinfo.num_sectors * (cinfo.fdm_size - 1);

	printf("SPI-NAND: %s (%uMB)\n", cinfo.model,
	       (u32)(cinfo.chipsize >> 20));

	page_cache = malloc(cinfo.pagesize + cinfo.sparesize);
	if (!page_cache) {
		mtk_snand_cleanup(snf);
		printf("mtk-snand-spl: failed to allocate page cache\n");
	}
}

void nand_deselect(void)
{

}

static const struct udevice_id mtk_snand_ids[] = {
	{ .compatible = "mediatek,mt7622-snand", .data = SNAND_SOC_MT7622 },
	{ .compatible = "mediatek,mt7629-snand", .data = SNAND_SOC_MT7629 },
	{ .compatible = "mediatek,mt7981-snand", .data = SNAND_SOC_MT7981 },
	{ .compatible = "mediatek,mt7986-snand", .data = SNAND_SOC_MT7986 },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(mtk_snand) = {
	.name = "mtk-snand",
	.id = UCLASS_MTD,
	.of_match = mtk_snand_ids,
	.flags = DM_FLAG_PRE_RELOC,
};
