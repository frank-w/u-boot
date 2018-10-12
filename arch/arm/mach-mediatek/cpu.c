// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <common.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <wdt.h>

int arch_misc_init(void)
{
	struct udevice *wdt;
	int ret;

	ret = uclass_get_device(UCLASS_WDT, 0, &wdt);
	if (!ret)
		wdt_stop(wdt);

	return 0;
}

int arch_cpu_init(void)
{
	icache_enable();

	return 0;
}

void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
