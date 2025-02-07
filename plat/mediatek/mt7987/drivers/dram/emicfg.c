/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <bl2_plat_setup.h>

extern void mtk_mem_init_real(void);
extern unsigned int mt7987_use_ddr4;
extern unsigned int mt7987_ddr4_4bg_mode;
extern unsigned int mt7987_ddr3_freq;
extern unsigned int mt7987_ddr4_freq;
extern unsigned int mt7987_ddr_refresh_interval;
extern unsigned int mt7987_ddr_power_down;
extern unsigned int mt7987_dram_size;
extern unsigned int mt7987_socket_board;
extern signed int mt7987_ddr_freq_adj;

void mtk_mem_init(void)
{
	mt7987_use_ddr4 = (mmio_read_32(0x1001f6f0) & 0x100) ? 1 : 0;

#ifdef DDR4_4BG_MODE
	if (mt7987_use_ddr4) {
		NOTICE("EMI: DDR4 4BG mode\n");
		mt7987_ddr4_4bg_mode = 1;/* 4 BG mode */
	} else {
		mt7987_ddr4_4bg_mode = 0;/* NOT 4 BG mode */
	}
#endif

#ifdef DDR3_FREQ_2133
	mt7987_ddr3_freq = 2133;
#endif /* DDR3_FREQ_2133 */
#ifdef DDR3_FREQ_1866
	mt7987_ddr3_freq = 1866;
#endif /* DDR3_FREQ_1866 */

#ifdef DDR_REFRESH_INTERVAL_780
	mt7987_ddr_refresh_interval = 0xca;
#endif
#ifdef DDR_REFRESH_INTERVAL_390
	mt7987_ddr_refresh_interval = 0x65;
#endif
#ifdef DDR_REFRESH_INTERVAL_195
	mt7987_ddr_refresh_interval = 0x33;
#endif

#ifdef DDR4_FREQ_3200
	mt7987_ddr4_freq = 3200;
#endif /* DDR4_FREQ_3200 */
#ifdef DDR4_FREQ_2666
	mt7987_ddr4_freq = 2666;
#endif /* DDR4_FREQ_2666 */

#ifdef SOCKET_BOARD
	mt7987_socket_board = 1;
#endif

#ifdef DDR_POWER_DOWN_ALWAYS_ON
	mt7987_ddr_power_down = 1;
#endif
#ifdef DDR_POWER_DOWN_DYNAMIC
	mt7987_ddr_power_down = 2;
#endif

#ifdef DDR_FREQ_ADJUST
	mt7987_ddr_freq_adj = DDR_FREQ_ADJUST;
#endif

	NOTICE("EMI: Using DDR%u settings\n", mt7987_use_ddr4 ? 4 : 3);

	mtk_mem_init_real();

	NOTICE("DRAM: %uMB\n", mt7987_dram_size);

	mtk_bl2_set_dram_size((uint64_t)mt7987_dram_size << 20);
}
