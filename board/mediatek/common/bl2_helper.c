// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All Rights Reserved.
 *
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <asm/global_data.h>
#include <common.h>
#include <fdt_support.h>
#include <image.h>
#include "bl2_helper.h"
#include "fip_helper.h"
#include "board_info.h"

DECLARE_GLOBAL_DATA_PTR;

static const struct bl2_entry bl2_hdr_entries[] = {
	{ .compat = "spim-nor", .header = SPIM_NOR_HDR },
	{ .compat = "spim-nand", .header = SPIM_NAND_HDR },
	{ .compat = "snfi-nand", .header = SNFI_NAND_HDR },
	{ .compat = "emmc", .header = EMMC_HDR },
	{ .compat = "sd", .header = SD_HDR },
	{ NULL },
};

static int bl2_get_compat_list(const void *fdt_blob, struct compat_list *cl)
{
	int ret;

	ret = fdt_path_offset(fdt_blob, "/bl2_verify");
	if (ret < 0)
		return -ENOENT;

	return fdt_read_compat_list(fdt_blob, ret, "bl2_compatible", cl);
}

static const struct bl2_entry *get_bl2_entry(const char *compat)
{
	const struct bl2_entry *entry;

	for (entry = bl2_hdr_entries; entry->compat; entry++)
		if (!strcmp(compat, entry->compat))
			return entry;

	return NULL;
}

bool bl2_check_image_data(const void *bl2, ulong bl2_size)
{
	u8 bl2_hdr[BL2_HDR_SIZE];
	const struct bl2_entry *entry;
	struct compat_list cl;
	int i;

	/* current bl2 has no 'bl2_verify' node, we should skip verify image */
	if (bl2_get_compat_list(gd->fdt_blob, &cl))
		return true;

	memcpy(bl2_hdr, bl2, BL2_HDR_SIZE);

	for (i = 0; i < cl.count; i++) {
		entry = get_bl2_entry(cl.compats[i]);
		if (!entry)
			continue;
		if (!memcmp(&entry->header, bl2_hdr, 8))
			return true;
	}

	return false;
}
