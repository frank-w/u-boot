/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <bl2_plat_setup.h>

extern void mtk_mem_init_real(void);
extern int mt7988_use_ddr4;
extern int mt7988_use_comb;
extern int mt7988_ddr4_4bg_mode;
extern int mt7988_ddr3_freq;
extern int mt7988_ddr4_freq;
extern int mt7988_ddr_refresh_interval;
extern unsigned int mt7988_dram_size;

void mtk_mem_init(void)
{
#ifdef DRAM_USE_COMB
	mt7988_use_comb = 1;
#endif /* DRAM_USE_COMB */

#ifdef DDR4_4BG_MODE
	NOTICE("EMI: DDR4 4BG mode\n");
	mt7988_ddr4_4bg_mode = 1;
#endif /* DDR4_4BG_MODE */

#ifdef DRAM_USE_DDR4
	mt7988_use_ddr4 = 1;
#endif /* DRAM_USE_DDR4 */

#ifdef DDR3_FREQ_2133
	mt7988_ddr3_freq = 2133;
#endif /* DDR3_FREQ_2133 */
#ifdef DDR3_FREQ_1866
	mt7988_ddr3_freq = 1866;
#endif /* DDR3_FREQ_1866 */

#ifdef DDR_REFRESH_INTERVAL_780
	mt7988_ddr_refresh_interval = 0xca;
#endif
#ifdef DDR_REFRESH_INTERVAL_390
	mt7988_ddr_refresh_interval = 0x65;
#endif
#ifdef DDR_REFRESH_INTERVAL_292
	mt7988_ddr_refresh_interval = 0x4c;
#endif
#ifdef DDR_REFRESH_INTERVAL_195
	mt7988_ddr_refresh_interval = 0x33;
#endif

#ifdef DDR4_FREQ_3200
	mt7988_ddr4_freq = 3200;
#endif /* DDR4_FREQ_3200 */
#ifdef DDR4_FREQ_2666
	mt7988_ddr4_freq = 2666;
#endif /* DDR4_FREQ_2666 */

	if (!mt7988_use_comb)
		NOTICE("EMI: Using DDR%u settings\n", mt7988_use_ddr4 ? 4 : 3);
	else
		NOTICE("EMI: Using DDR unknown settings\n");

	mtk_mem_init_real();

	NOTICE("DRAM: %uMB\n", mt7988_dram_size);

	mtk_bl2_set_dram_size((uint64_t)mt7988_dram_size << 20);
}
