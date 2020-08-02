// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK SATA platform driver
 *
 * (C) Copyright 2020
 *     Mediatek
 *
 * Author: Frank Wunderlich <frank-w@public-files.de>
 * based on https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/ata/ahci_mtk.c
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <ahci.h>
#include <scsi.h>
#include <sata.h>
#include <reset.h>
#include <asm/io.h>
#include <generic-phy.h>
#include <dm/of_access.h>
#include <syscon.h>
#include <linux/err.h>
#include <regmap.h>

#define SYS_CFG			0x14
#define SYS_CFG_SATA_MSK	GENMASK(31, 30)
#define SYS_CFG_SATA_EN		BIT(31)

struct mtk_ahci_priv {
	void *base;

	struct regmap *mode;
	struct reset_ctl axi_rst;
	struct reset_ctl sw_rst;
	struct reset_ctl reg_rst;
};

static int mtk_ahci_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ahci_bind_scsi(dev, &scsi_dev);
}

static int mtk_ahci_ofdata_to_platdata(struct udevice *dev)
{
	struct mtk_ahci_priv *priv = dev_get_priv(dev);

	priv->base = map_physmem(devfdt_get_addr(dev), sizeof(void *),
				 MAP_NOCACHE);

	return 0;
}

static int mtk_ahci_platform_resets(struct udevice *dev)
{
	struct mtk_ahci_priv *priv = dev_get_priv(dev);
	int ret;

	ret=reset_get_by_name(dev, "axi",&priv->axi_rst);
	if (ret) {
		pr_err("unable to find reset controller 'axi'\n");
		return ret;
	}

	ret=reset_get_by_name(dev, "sw",&priv->sw_rst);
	if (ret) {
		pr_err("unable to find reset controller 'sw'\n");
		return ret;
	}

	ret=reset_get_by_name(dev, "reg",&priv->reg_rst);
	if (ret) {
		pr_err("unable to find reset controller 'reg'\n");
		return ret;
	}

	ret=reset_assert(&priv->axi_rst);
	if (ret) {
		pr_err("unable to assert reset controller 'axi'\n");
		return ret;
	}

	ret=reset_assert(&priv->sw_rst);
	if (ret) {
		pr_err("unable to assert reset controller 'sw'\n");
		return ret;
	}

	ret=reset_assert(&priv->reg_rst);
	if (ret) {
		pr_err("unable to assert reset controller 'reg'\n");
		return ret;
	}

	ret=reset_deassert(&priv->axi_rst);
	if (ret) {
		pr_err("unable to deassert reset controller 'axi'\n");
		return ret;
	}

	ret=reset_deassert(&priv->sw_rst);
	if (ret) {
		pr_err("unable to deassert reset controller 'sw'\n");
		return ret;
	}

	ret=reset_deassert(&priv->reg_rst);
	if (ret) {
		pr_err("unable to deassert reset controller 'reg'\n");
		return ret;
	}
	return 0;
}

static int mtk_ahci_parse_property(struct ahci_uc_priv *hpriv,struct udevice *dev)
{
	struct mtk_ahci_priv *plat = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;

	/* enable SATA function if needed */
	if (fdt_get_property(fdt, dev_of_offset(dev),"mediatek,phy-mode", NULL)) {
		plat->mode = syscon_regmap_lookup_by_phandle(
					dev, "mediatek,phy-mode");
		if (IS_ERR(plat->mode)) {
			dev_err(dev, "missing phy-mode phandle\n");
			return PTR_ERR(plat->mode);
		}
		regmap_update_bits(plat->mode, SYS_CFG, SYS_CFG_SATA_MSK,
				   SYS_CFG_SATA_EN);
	}

	ofnode_read_u32(dev->node, "ports-implemented", &hpriv->port_map);
	return 0;
}

static int mtk_ahci_probe(struct udevice *dev)
{
	struct mtk_ahci_priv *priv = dev_get_priv(dev);
	int ret;
	struct phy phy;
	struct ahci_uc_priv *hpriv;

	hpriv = malloc(sizeof(struct ahci_uc_priv));
	if (!hpriv)
		return -ENOMEM;

	memset(hpriv, 0, sizeof(struct ahci_uc_priv));

	ret = mtk_ahci_parse_property(hpriv,dev);
	if (ret)
		return ret;

	ret = mtk_ahci_platform_resets(dev);
	if (ret)
		return ret;

	ret = generic_phy_get_by_name(dev, "sata-phy", &phy);
	if (ret) {
		pr_err("can't get the phy from DT\n");
		return ret;
	}

	ret = generic_phy_init(&phy);
	if (ret) {
		pr_err("unable to initialize the sata phy\n");
		return ret;
	}

	ret = generic_phy_power_on(&phy);
	if (ret) {
		pr_err("unable to power on the sata phy\n");
		return ret;
	}

	return ahci_probe_scsi(dev, (ulong)priv->base);
}

static const struct udevice_id mtk_ahci_ids[] = {
	{ .compatible = "mediatek,mtk-ahci" },
	{ }
};

U_BOOT_DRIVER(mtk_ahci) = {
	.name	= "mtk_ahci",
	.id	= UCLASS_AHCI,
	.of_match = mtk_ahci_ids,
	.bind	= mtk_ahci_bind,
	.ofdata_to_platdata = mtk_ahci_ofdata_to_platdata,
	.ops	= &scsi_ops,
	.probe	= mtk_ahci_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_ahci_priv),
};
