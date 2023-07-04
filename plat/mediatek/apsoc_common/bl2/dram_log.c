// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2026, MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <plat/common/platform.h>
#include <common/debug.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef DRAM_DEBUG_LOG
#define DRAM_DEBUG_LOG_VAL	1
#else
#define DRAM_DEBUG_LOG_VAL	0
#endif /* DRAM_DEBUG_LOG */

unsigned int mtk_dram_debug = DRAM_DEBUG_LOG_VAL;

static void _mtk_mem_print(unsigned int log_level, const char *fmt,
			   va_list args)
{
	const char *prefix_str;

	prefix_str = plat_log_get_prefix(log_level);

	while (*prefix_str != '\0') {
		(void)putchar(*prefix_str);
		prefix_str++;
	}

	(void)vprintf(fmt, args);
}

void mtk_mem_log_print(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	(void)_mtk_mem_print(LOG_LEVEL_NOTICE, fmt, args);
	va_end(args);
}

void mtk_mem_err_print(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	(void)_mtk_mem_print(LOG_LEVEL_ERROR, fmt, args);
	va_end(args);
}

void mtk_mem_dbg_print(const char *fmt, ...)
{
	va_list args;

	if (!mtk_dram_debug)
		return;

	va_start(args, fmt);
	(void)vprintf(fmt, args);
	va_end(args);
}
