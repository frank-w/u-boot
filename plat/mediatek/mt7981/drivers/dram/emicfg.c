/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <bl2_plat_setup.h>

extern void mtk_mem_init_real(void);
extern int mt7981_use_ddr4;
extern int mt7981_ddr_size_limit;
extern int mt7981_bga_pkg;
extern int mt7981_ddr3_freq;
extern unsigned int mt7981_dram_size;
extern unsigned int mt7981_dram_size_usable;

void mtk_mem_init(void)
{
#ifdef DRAM_USE_DDR4
	mt7981_use_ddr4 = 1;
#endif /* DRAM_USE_DDR4 */

#ifdef DRAM_SIZE_LIMIT
	mt7981_ddr_size_limit = DRAM_SIZE_LIMIT;

	if (!mt7981_use_ddr4 && mt7981_ddr_size_limit > 512)
		mt7981_ddr_size_limit = 512;
#endif /* DRAM_SIZE_LIMIT */

#if defined(BOARD_BGA)
	mt7981_bga_pkg = 1;
#elif defined(BOARD_QFN)
	mt7981_bga_pkg = 0;
#endif /* BOARD_BGA */

#ifdef DDR3_FREQ_2133
	mt7981_ddr3_freq = 2133;
#endif /* DDR3_FREQ_2133 */
#ifdef DDR3_FREQ_1866
	mt7981_ddr3_freq = 1866;
#endif /* DDR3_FREQ_1866 */

	NOTICE("EMI: Using DDR%u settings\n", mt7981_use_ddr4 ? 4 : 3);

	mtk_mem_init_real();

	if (mt7981_dram_size_usable == mt7981_dram_size) {
		NOTICE("DRAM: %uMB\n", mt7981_dram_size);
	} else {
		NOTICE("DRAM: %uMB (%uMB usable)\n", mt7981_dram_size,
		       mt7981_dram_size_usable);
	}

	mtk_bl2_set_dram_size((uint64_t)mt7981_dram_size << 20);
}
