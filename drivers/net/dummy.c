// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <net.h>
#include <dm/device_compat.h>

static int dummy_start(struct udevice *dev)
{
	dev_dbg(dev, "%s()\n", __func__);

	return -ENODEV;
}

static int dummy_send(struct udevice *dev, void *packet, int length)
{
	dev_dbg(dev, "%s()\n", __func__);

	return -ENODEV;
}

static int dummy_recv(struct udevice *dev, int flags, uchar **packetp)
{
	dev_dbg(dev, "%s()\n", __func__);

	return -ENODEV;
}

static void dummy_stop(struct udevice *dev)
{
	dev_dbg(dev, "%s()\n", __func__);
}

static int dummy_probe(struct udevice *dev)
{
	dev_dbg(dev, "%s()\n", __func__);

	/* Force probe of PHY to issue reset-gpios */
	uclass_get_device_by_phandle(UCLASS_ETH_PHY, dev, "phy-handle", NULL);

	return -ENODEV;
}

static const struct eth_ops dummy_ops = {
	.start = dummy_start,
	.send = dummy_send,
	.recv = dummy_recv,
	.stop = dummy_stop,
};

static const struct udevice_id dummy_ids[] = {
	{ .compatible = "snps,dwmac-4.20a" },
	{ }
};

U_BOOT_DRIVER(eth_dummy) = {
	.name = "eth_dummy",
	.id = UCLASS_ETH,
	.of_match = dummy_ids,
	.probe = dummy_probe,
	.ops = &dummy_ops,
	.plat_auto = sizeof(struct eth_pdata),
};
