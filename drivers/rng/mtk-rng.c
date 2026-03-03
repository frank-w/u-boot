// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Driver for Mediatek Hardware Random Number Generator
 *
 * Copyright (C) 2017 Sean Wang <sean.wang@mediatek.com>
 *
 * Converted from Linux to U-Boot driver model.
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <rng.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>

#define RNG_CTRL		0x00
#define RNG_EN			BIT(0)
#define RNG_READY		BIT(31)

#define RNG_DATA		0x08

#define RNG_POLL_US		2
#define RNG_TIMEOUT_US		500000

struct mtk_rng_priv {
	void __iomem *base;
	struct clk clk;
};

static int mtk_rng_read(struct udevice *dev, void *data, size_t len)
{
	struct mtk_rng_priv *priv = dev_get_priv(dev);
	u32 val;
	char *buf = data;
	int ret;

	while (len) {
		/* Poll until RNG has data ready */
		ret = readl_poll_sleep_timeout(priv->base + RNG_CTRL, val,
					       val & RNG_READY,
					       RNG_POLL_US, RNG_TIMEOUT_US);
		if (ret)
			return -EIO;

		val = readl(priv->base + RNG_DATA);
		if (len >= sizeof(u32)) {
			*(u32 *)buf = val;
			buf += sizeof(u32);
			len -= sizeof(u32);
		} else {
			memcpy(buf, &val, len);
			len = 0;
		}
	}

	return 0;
}

static int mtk_rng_probe(struct udevice *dev)
{
	struct mtk_rng_priv *priv = dev_get_priv(dev);
	u32 val;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_name(dev, "rng", &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;

	/* Enable the RNG */
	val = readl(priv->base + RNG_CTRL);
	val |= RNG_EN;
	writel(val, priv->base + RNG_CTRL);

	/* Verify RNG_EN took effect */
	val = readl(priv->base + RNG_CTRL);
	if (!(val & RNG_EN)) {
		dev_err(dev, "failed to enable RNG (CTRL=0x%08x)\n", val);
		clk_disable(&priv->clk);
		return -EIO;
	}

	return 0;
}

static int mtk_rng_remove(struct udevice *dev)
{
	struct mtk_rng_priv *priv = dev_get_priv(dev);
	u32 val;

	/* Disable the RNG */
	val = readl(priv->base + RNG_CTRL);
	val &= ~RNG_EN;
	writel(val, priv->base + RNG_CTRL);

	clk_disable(&priv->clk);

	return 0;
}

static const struct dm_rng_ops mtk_rng_ops = {
	.read = mtk_rng_read,
};

static const struct udevice_id mtk_rng_match[] = {
	{ .compatible = "mediatek,mt7623-rng" },
	{},
};

U_BOOT_DRIVER(mtk_rng) = {
	.name = "mtk-rng",
	.id = UCLASS_RNG,
	.of_match = mtk_rng_match,
	.ops = &mtk_rng_ops,
	.probe = mtk_rng_probe,
	.remove = mtk_rng_remove,
	.priv_auto = sizeof(struct mtk_rng_priv),
};
