// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 Mediatek
 */

#include <dm.h>
#include <log.h>
#include <syscon.h>
#include <asm/arch-mediatek/clock.h>

static const struct udevice_id mt7988_syscon_ids[] = {
	{ .compatible = "mediatek,mt7988-wdt", .data = MEDIATEK_SYSCON_WDT },
	{ .compatible = "mediatek,mt7988-ethwarp", .data = MEDIATEK_SYSCON_RESET },
	{ .compatible = "mediatek,mt7988-xfi-pll", .data = MEDIATEK_SYSCON_XFIPLL },
	{ }
};

U_BOOT_DRIVER(syscon_mt7988) = {
	.name = "mt7988_syscon",
	.id = UCLASS_SYSCON,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind = dm_scan_fdt_dev,
#endif
	.of_match = mt7988_syscon_ids,
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
static int mt7988_syscon_bind_of_plat(struct udevice *dev)
{
	dev->driver_data = dev->driver->of_match->data;
	debug("syscon: %s %d\n", dev->name, (uint)dev->driver_data);

	return 0;
}

U_BOOT_DRIVER(mediatek_mt7988_wdt) = {
	.name = "mediatek_mt7988_wdt",
	.id = UCLASS_SYSCON,
	.of_match = mt7988_syscon_ids,
	.bind = mt7988_syscon_bind_of_plat,
};

U_BOOT_DRIVER(mediatek_mt7988_ethwarp) = {
	.name = "mediatek_mt7988_ethwarp",
	.id = UCLASS_SYSCON,
	.of_match = mt7988_syscon_ids + 1,
	.bind = mt7988_syscon_bind_of_plat,
};
U_BOOT_DRIVER(mediatek_mt7988_xfi_pll) = {
	.name = "mediatek_mt7988_xfi_pll",
	.id = UCLASS_SYSCON,
	.of_match = mt7988_syscon_ids + 2,
	.bind = mt7988_syscon_bind_of_plat,
};

#endif
