/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <bl2_plat_setup.h>

extern void mtk_mem_init_real(void);
extern unsigned long mt7629_dram_size;

void mtk_mem_init(void)
{
	mtk_mem_init_real();

	NOTICE("DRAM: %luMB\n", mt7629_dram_size >> 20);

	mtk_bl2_set_dram_size(mt7629_dram_size);
}
