/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <bl2_plat_setup.h>
#include <mt7986_def.h>

extern void mtk_mem_init_real(void);
extern int mt7986_use_ddr4;
extern int mt7986_ddr4_freq;
extern int mt7986_ddr_size_limit;
extern unsigned int mt7986_dram_size;
extern unsigned int mt7986_dram_size_usable;

void mtk_mem_init(void)
{
#ifdef DRAM_USE_DDR4
#if 0	/* Enable this after eFuse bit is ready */
	/* Only IAP supports DDR4 */
	if (mmio_read_32(IAP_REBB_SWITCH) & IAP_IND)
#endif
		mt7986_use_ddr4 = 1;
#endif /* DRAM_USE_DDR4 */

#ifdef DRAM_SIZE_LIMIT
	mt7986_ddr_size_limit = DRAM_SIZE_LIMIT;

	if (!mt7986_use_ddr4 && mt7986_ddr_size_limit > 512)
		mt7986_ddr_size_limit = 512;
#endif /* DRAM_SIZE_LIMIT */

#ifdef DDR4_FREQ_3200
	mt7986_ddr4_freq = 3200;
#endif /* DDR4_FREQ_3200 */
#ifdef DDR4_FREQ_2666
	mt7986_ddr4_freq = 2666;
#endif /* DDR4_FREQ_2400 */

	NOTICE("EMI: Using DDR%u settings\n", mt7986_use_ddr4 ? 4 : 3);

	mtk_mem_init_real();

	if (mt7986_dram_size_usable == mt7986_dram_size) {
		NOTICE("DRAM: %uMB\n", mt7986_dram_size);
	} else {
		NOTICE("DRAM: %uMB (%uMB usable)\n", mt7986_dram_size,
		       mt7986_dram_size_usable);
	}

	mtk_bl2_set_dram_size((uint64_t)mt7986_dram_size_usable << 20);
}
