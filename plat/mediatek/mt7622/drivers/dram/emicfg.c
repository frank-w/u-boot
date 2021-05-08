/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <bl2_plat_setup.h>

extern void mtk_mem_init_real(void);
extern int mt7622_ddr3_flyby;
extern unsigned long mt7622_dram_size;

void mtk_mem_init(void)
{
#ifdef DDR3_FLYBY
	mt7622_ddr3_flyby = 1;
#endif

	if (mt7622_ddr3_flyby)
		NOTICE("EMI: Using DDR3 fly-by settings\n");
	else
		NOTICE("EMI: Using DDR3 settings\n");

	mtk_mem_init_real();

	NOTICE("DRAM: %luMB\n", mt7622_dram_size >> 20);

	mtk_bl2_set_dram_size(mt7622_dram_size);
}
