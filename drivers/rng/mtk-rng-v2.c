// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Driver for Mediatek Hardware Random Number Generator (v2/SMC)
 *
 * Copyright (C) 2023 Daniel Golle <daniel@makrotopia.org>
 *
 * On newer Mediatek SoCs the RNG hardware is only accessible from
 * secure world.  Random numbers are obtained via a vendor-defined
 * Secure Monitor Call handled by ARM Trusted Firmware-A.
 *
 * Converted from Linux to U-Boot driver model.
 */

#include <dm.h>
#include <rng.h>
#include <linux/arm-smccc.h>

/* MediaTek SIP SMC function ID for RNG — use SMC64 on AArch64, SMC32 on AArch32 */
#ifdef CONFIG_ARM64
#define MTK_SIP_SMC_CONVENTION	ARM_SMCCC_SMC_64
#else
#define MTK_SIP_SMC_CONVENTION	ARM_SMCCC_SMC_32
#endif

#define MTK_SIP_TRNG_GET_RND	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, \
						   MTK_SIP_SMC_CONVENTION, \
						   ARM_SMCCC_OWNER_SIP, 0x550)

static int mtk_rng_v2_read(struct udevice *dev, void *data, size_t len)
{
	struct arm_smccc_res res;
	char *buf = data;

	while (len) {
		arm_smccc_smc(MTK_SIP_TRNG_GET_RND, 0, 0, 0, 0, 0, 0, 0,
			      &res);
		if (res.a0)
			return -EIO;

		if (len >= sizeof(u32)) {
			*(u32 *)buf = res.a1;
			buf += sizeof(u32);
			len -= sizeof(u32);
		} else {
			memcpy(buf, &res.a1, len);
			len = 0;
		}
	}

	return 0;
}

static const struct dm_rng_ops mtk_rng_v2_ops = {
	.read = mtk_rng_v2_read,
};

static const struct udevice_id mtk_rng_v2_match[] = {
	{ .compatible = "mediatek,mt7981-rng" },
	{ .compatible = "mediatek,mt7986-rng" },
	{ .compatible = "mediatek,mt7987-rng" },
	{ .compatible = "mediatek,mt7988-rng" },
	{},
};

U_BOOT_DRIVER(mtk_rng_v2) = {
	.name = "mtk-rng-v2",
	.id = UCLASS_RNG,
	.of_match = mtk_rng_v2_match,
	.ops = &mtk_rng_v2_ops,
};
