// SPDX-License-Identifier: GPL-2.0

#define LOG_DEBUG

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <syscon.h>
#include <regmap.h>
#include <power/regulator.h>

#define MAX_SUPPLIES		16

/*
 * The max voltage for 1.8V and 3.3V come from the Rockchip datasheet under
 * "Recommended Operating Conditions" for "Digital GPIO".   When the typical
 * is 3.3V the max is 3.6V.  When the typical is 1.8V the max is 1.98V.
 *
 * They are used like this:
 * - If the voltage on a rail is above the "1.8" voltage (1.98V) we'll tell the
 *   SoC we're at 3.3.
 * - If the voltage on a rail is above the "3.3" voltage (3.6V) we'll consider
 *   that to be an error.
 */
#define MAX_VOLTAGE_1_8		1980000
#define MAX_VOLTAGE_3_3		3600000

#define RK3568_PMU_GRF_IO_VSEL0		(0x0140)
#define RK3568_PMU_GRF_IO_VSEL1		(0x0144)
#define RK3568_PMU_GRF_IO_VSEL2		(0x0148)

struct rockchip_iodomain_soc_data {
	int grf_offset;
	const char *supply_names[MAX_SUPPLIES];
	int (*write)(struct regmap *grf, const char *supply_name, int idx, int uV);
};

static int rk3568_iodomain_write(struct regmap *grf, const char *supply_name, int idx, int uV)
{
	u32 is_3v3 = uV > MAX_VOLTAGE_1_8;
	u32 val0, val1;
	int b;

	switch (idx) {
	case 0: /* pmuio1 */
		break;
	case 1: /* pmuio2 */
		b = idx;
		val0 = BIT(16 + b) | (is_3v3 ? 0 : BIT(b));
		b = idx + 4;
		val1 = BIT(16 + b) | (is_3v3 ? BIT(b) : 0);

		debug("%s(supply_name=%s, idx=%d, uV=%d): val0=%x val1=%x\n", __func__, supply_name, idx, uV, val0, val1);
		regmap_write(grf, RK3568_PMU_GRF_IO_VSEL2, val0);
		regmap_write(grf, RK3568_PMU_GRF_IO_VSEL2, val1);
		break;
	case 3: /* vccio2 */
		break;
	case 2: /* vccio1 */
	case 4: /* vccio3 */
	case 5: /* vccio4 */
	case 6: /* vccio5 */
	case 7: /* vccio6 */
	case 8: /* vccio7 */
		b = idx - 1;
		val0 = BIT(16 + b) | (is_3v3 ? 0 : BIT(b));
		val1 = BIT(16 + b) | (is_3v3 ? BIT(b) : 0);

		debug("%s(supply_name=%s, idx=%d, uV=%d): val0=%x val1=%x\n", __func__, supply_name, idx, uV, val0, val1);
		regmap_write(grf, RK3568_PMU_GRF_IO_VSEL0, val0);
		regmap_write(grf, RK3568_PMU_GRF_IO_VSEL1, val1);
		break;
	default:
		debug("%s(supply_name=%s, idx=%d, uV=%d): EINVAL\n", __func__, supply_name, idx, uV);
		return -EINVAL;
	}

	return 0;
}

static const struct rockchip_iodomain_soc_data soc_data_rk3568_pmu = {
	.grf_offset = 0x140,
	.supply_names = {
		"pmuio1-supply",
		"pmuio2-supply",
		"vccio1-supply",
		"vccio2-supply",
		"vccio3-supply",
		"vccio4-supply",
		"vccio5-supply",
		"vccio6-supply",
		"vccio7-supply",
	},
	.write = rk3568_iodomain_write,
};

static int rockchip_iodomain_bind(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);

	return 0;
}

static int rockchip_iodomain_probe(struct udevice *dev)
{
	struct rockchip_iodomain_soc_data *soc_data =
		(struct rockchip_iodomain_soc_data *)dev_get_driver_data(dev);
	struct regmap *grf;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	grf = syscon_get_regmap(dev_get_parent(dev));
	if (IS_ERR(grf)) {
		dev_err(dev, "couldn't find grf regmap\n");
		return PTR_ERR(grf);
	}

	for (int i = 0; i < MAX_SUPPLIES; i++) {
		const char *supply_name = soc_data->supply_names[i];
		struct udevice *reg;
		int uV;

		if (!supply_name)
			continue;

		ret = device_get_supply_regulator(dev, supply_name, &reg);
		if (ret) {
			dev_err(dev, "couldn't get regulator %s: %d\n", supply_name, ret);
			continue;
		}

		regulator_autoset(reg);

		/* set initial correct value */
		uV = regulator_get_value(reg);
		debug("%s: reg=%p supply_name=%s uV=%d\n", __func__, reg, supply_name, uV);

		/* must be a regulator we can get the voltage of */
		if (uV < 0) {
			dev_err(dev, "Can't determine voltage: %s: %d\n", supply_name, uV);
			continue;
		}

		if (uV == 0) {
			dev_info(dev, "Can't determine voltage: %s: %d\n", supply_name, uV);
			continue;
		}

		if (uV > MAX_VOLTAGE_3_3) {
			dev_crit(dev, "%d uV is too high. May damage SoC!\n", uV);
			continue;
		}

		soc_data->write(grf, supply_name, i, uV);
	}

	return 0;
}

static const struct udevice_id rockchip_iodomain_ids[] = {
	{
		.compatible = "rockchip,rk3568-pmu-io-voltage-domain",
		.data = (ulong)&soc_data_rk3568_pmu,
	},
	{ }
};

U_BOOT_DRIVER(rockchip_iodomain) = {
	.name = "rockchip_iodomain",
	.id = UCLASS_NOP,
	.of_match = rockchip_iodomain_ids,
	.bind = rockchip_iodomain_bind,
	.probe = rockchip_iodomain_probe,
};
