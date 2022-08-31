// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/types.h>
#include <cpu.h>
#include <dm.h>
#include <fdt_support.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <linux/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct mtk_cpu_plat {
	void __iomem *hwver_base;
};

static int mtk_cpu_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct mtk_cpu_plat *plat = dev_get_plat(dev);

	snprintf(buf, size, "MediaTek MT%04X", readl(plat->hwver_base));

	return 0;
}

static int mtk_cpu_get_count(const struct udevice *dev)
{
	return 1;
}

static int mtk_cpu_get_vendor(const struct udevice *dev, char *buf, int size)
{
	snprintf(buf, size, "MediaTek");

	return 0;
}

static int mtk_cpu_probe(struct udevice *dev)
{
	struct mtk_cpu_plat *plat = dev_get_plat(dev);
	const void *fdt = gd->fdt_blob, *reg;
	int offset, parent, len, na, ns;
	u64 addr;

	if (!fdt)
		return -ENODEV;

	offset = fdt_path_offset(fdt, "/hwver");
	if (offset < 0)
		return -ENODEV;

	parent = fdt_parent_offset(fdt, offset);
	if (parent < 0)
		return -ENODEV;

	na = fdt_address_cells(fdt, parent);
	if (na < 1)
		return -ENODEV;

	ns = fdt_size_cells(gd->fdt_blob, parent);
	if (ns < 0)
		return -ENODEV;

	reg = fdt_getprop(gd->fdt_blob, offset, "reg", &len);
	if (!reg)
		return -ENODEV;

	if (ns)
		addr = fdt_translate_address(fdt, offset, reg);
	else
		addr = fdt_read_number(reg, na);

	plat->hwver_base = map_sysmem(addr, 0);
	if (!plat->hwver_base)
		return -EINVAL;

	return 0;
}

static const struct cpu_ops mtk_cpu_ops = {
	.get_desc	= mtk_cpu_get_desc,
	.get_count	= mtk_cpu_get_count,
	.get_vendor	= mtk_cpu_get_vendor,
};

static const struct udevice_id mtk_cpu_ids[] = {
	{ .compatible = "arm,cortex-a7" },
	{ .compatible = "arm,cortex-a53" },
	{ .compatible = "arm,cortex-a73" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(cpu_mtk) = {
	.name		= "mtk-cpu",
	.id		= UCLASS_CPU,
	.of_match	= mtk_cpu_ids,
	.ops		= &mtk_cpu_ops,
	.probe		= mtk_cpu_probe,
	.plat_auto	= sizeof(struct mtk_cpu_plat),
	.flags		= DM_FLAG_PRE_RELOC,
};
