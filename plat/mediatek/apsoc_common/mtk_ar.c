// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 */

#include <common/debug.h>
#include <errno.h>
#include <lib/spinlock.h>

#include <mtk_efuse.h>
#include "mtk_ar.h"

#define MTK_AR_MAX_VER			64
#define MTK_AR_NUM_AR_VER		2

#define AR_VER(mask) \
	((mask) == 0 ? 0 : (64 - __builtin_clzll((mask))))
#define AR_VER_MASK(ver) \
	((ver) == 0 ? 0 : GENMASK((ver) - 1, 0))

extern const uint32_t bl_ar_ver;

static bool write_lock;
static spinlock_t ar_lock;

static int get_efuse_ar_ver(uint32_t field, uint32_t *ar_ver)
{
	uint32_t buf[MTK_AR_NUM_AR_VER] = { 0 };
	uint64_t val;
	uint32_t ret;
	uint32_t i;

	*ar_ver = 0;

	for (i = 0; i < MTK_AR_NUM_AR_VER; i++) {
		ret = mtk_efuse_read(field + i, (uint8_t *)&buf[i],
				     sizeof(uint32_t));
		if (ret) {
			ERROR("Anti-Rollback: Failed to read version: %d\n", ret);
			return -EFAULT;
		}
	}
	val = ((uint64_t)buf[1] << 32) | buf[0];

	*ar_ver = AR_VER(val);

	return 0;
}

static int set_efuse_ar_ver(uint32_t field, uint32_t ar_ver)
{
	uint32_t buf[MTK_AR_NUM_AR_VER] = { 0 };
	uint64_t val = AR_VER_MASK(ar_ver);
	uint32_t efuse_ar_ver = 0;
	uint32_t exp_val;
	uint32_t ret;
	uint32_t i;

	ret = get_efuse_ar_ver(field, &efuse_ar_ver);
	if (ret)
		return ret;

	if (ar_ver <= efuse_ar_ver)
		return 0;

	for (i = 0; i < MTK_AR_NUM_AR_VER; i++) {
		exp_val = (uint32_t)(val >> (32 * i));

		ret = mtk_efuse_read(field + i, (uint8_t *)&buf[i],
				     sizeof(uint32_t));
		if (ret) {
			ERROR("Anti-Rollback: Failed to read version: %d\n", ret);
			return -EFAULT;
		}

		if (buf[i] != exp_val) {
			buf[i] = exp_val;

			ret = mtk_efuse_write(field + i, (uint8_t *)&buf[i],
					      sizeof(uint32_t));
			if (ret) {
				ERROR("Anti-Rollback: Failed to write version: %d\n", ret);
				return -EFAULT;
			}
		}
	}

	return 0;
}

static int get_efuse_bl_ar_ver(uint32_t *ar_ver)
{
	uint32_t ar_ver0 = 0;
	uint32_t ar_ver1 = 0;
	int ret;

	*ar_ver = 0;

	ret = get_efuse_ar_ver(EFUSE_BL_AR_VER0, &ar_ver0);
	if (ret)
		ERROR("Anti-Rollback: Failed to get version: %d\n", ret);

#ifdef BL_AR_VER_DUAL
	ret = get_efuse_ar_ver(EFUSE_BL_AR_VER2, &ar_ver1);
	if (ret)
		ERROR("Anti-Rollback: Failed to get version: %d\n", ret);
#endif

	*ar_ver = ar_ver0 | ar_ver1;

	return ret;
}

static int get_efuse_fw_ar_ver(uint32_t *ar_ver)
{
#ifdef FW_AR_VER_SUPPORT
	return get_efuse_ar_ver(EFUSE_FW_AR_VER0, ar_ver);
#else
	return -ENODEV;
#endif
}

int mtk_ar_get_bl_ar_ver(uint32_t *ar_ver)
{
	if (!ar_ver)
		return -EINVAL;

	return get_efuse_bl_ar_ver(ar_ver);
}

int mtk_ar_get_ar_ver(uint32_t id, uint32_t *ar_ver)
{
	if (!ar_ver)
		return -EINVAL;

	if (id == BL_AR_VER_ID)
		return get_efuse_bl_ar_ver(ar_ver);
	else if (id == FW_AR_VER_ID)
		return get_efuse_fw_ar_ver(ar_ver);
	else
		return -EINVAL;

	return 0;
}

static int get_efuse_ar_en(uint32_t *ar_en)
{
	uint32_t ret;

	ret = mtk_efuse_read(EFUSE_AR_EN, (uint8_t *)ar_en, sizeof(*ar_en));
	if (ret) {
		ERROR("Anti-Rollback: Failed to read status: %d\n", ret);
		return -EFAULT;
	}

	return 0;
}

int mtk_ar_check_consis(uint32_t ar_ver)
{
	/* make sure the version is equal to current running BL's version */
	if (ar_ver != bl_ar_ver) {
		ERROR("Anti-Rollback: BL Version is not consistent\n");
		return -EINVAL;
	}

	return 0;
}

static int update_efuse_bl_ar_ver(void)
{
	int ret;

	/* No need to carry the BL version from normal world,
	   since BL31 already knows the version. */

	ret = set_efuse_ar_ver(EFUSE_BL_AR_VER0, bl_ar_ver);
	if (ret)
		ERROR("Anti-Rollback: Failed to update version: %d\n", ret);

#ifdef BL_AR_VER_DUAL
	ret = set_efuse_ar_ver(EFUSE_BL_AR_VER2, bl_ar_ver);
	if (ret)
		ERROR("Anti-Rollback: Failed to update version: %d\n", ret);
#endif

	return ret;
}

static int update_efuse_fw_ar_ver(uint32_t ar_ver)
{
#ifdef FW_AR_VER_SUPPORT
	return set_efuse_ar_ver(EFUSE_FW_AR_VER0, ar_ver);
#else
	return -ENODEV;
#endif
}

int mtk_ar_update_bl_ar_ver(void)
{
	uint32_t ar_en = 0;
	int ret;

	ret = get_efuse_ar_en(&ar_en);
	if (ret)
		return ret;

	if (ar_en) {
		ret = update_efuse_bl_ar_ver();
		NOTICE("Updating BL Anti-Rollback Version ... bl_ar_ver:%u %s\n",
		       bl_ar_ver, (!ret) ? "OK" : "FAIL");
	}

	return ret;
}

int mtk_ar_update_ar_ver(uint32_t id, uint32_t ar_ver)
{
	uint32_t ar_en = 0;
	int ret;

	if (ar_ver > MTK_AR_MAX_VER) {
		ERROR("Anti-Rollback: Invalid version: %u\n", ar_ver);
		return -EINVAL;
	}

	spin_lock(&ar_lock);
	if (write_lock) {
		spin_unlock(&ar_lock);
		return -EPERM;
	}
	spin_unlock(&ar_lock);

	ret = get_efuse_ar_en(&ar_en);
	if (ret)
		return ret;

	if (ar_en) {
		if (id == BL_AR_VER_ID)
			return update_efuse_bl_ar_ver();
		else if (id == FW_AR_VER_ID)
			return update_efuse_fw_ar_ver(ar_ver);
		else
			return -EINVAL;
	}

	return 0;
}

int mtk_ar_lock_ar_ver(void)
{
	spin_lock(&ar_lock);
	write_lock = 1;
	spin_unlock(&ar_lock);

	return 0;
}
