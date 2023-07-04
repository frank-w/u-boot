/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch/aarch64/arch.h>
#include <arch/aarch64/arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/utils_def.h>
#include <lib/mmio.h>
#include <mtk_sip_svc.h>
#include <plat_sip_calls.h>
#include <string.h>
#include <mtk_efuse.h>
#include <mtk_fsek.h>

/* Authorized secure register list */
enum {
	SREG_HDMI_COLOR_EN = 0x14000904
};

static const uint32_t authorized_sreg[] = {
	SREG_HDMI_COLOR_EN
};

#define authorized_sreg_cnt	ARRAY_SIZE(authorized_sreg)

uint64_t mt_sip_set_authorized_sreg(uint32_t sreg, uint32_t val)
{
	uint64_t i;

	for (i = 0; i < authorized_sreg_cnt; i++) {
		if (authorized_sreg[i] == sreg) {
			mmio_write_32(sreg, val);
			return MTK_SIP_E_SUCCESS;
		}
	}

	return MTK_SIP_E_INVALID_PARAM;
}

static uint64_t mt_sip_pwr_on_mtcmos(uint32_t val)
{
	return MTK_SIP_E_SUCCESS;
}

static uint64_t mt_sip_pwr_off_mtcmos(uint32_t val)
{
	return MTK_SIP_E_SUCCESS;
}

static uint64_t mt_sip_pwr_mtcmos_support(void)
{
	return MTK_SIP_E_SUCCESS;
}

uintptr_t mediatek_plat_sip_handler(uint32_t smc_fid,
				    u_register_t x1,
				    u_register_t x2,
				    u_register_t x3,
				    u_register_t x4,
				    void *cookie,
				    void *handle,
				    u_register_t flags)
{
	uint64_t ret;
	uint32_t efuse_len = 0;
	uint32_t efuse_data[2] = { (uint32_t)x3, (uint32_t)x4 };
	static uint32_t efuse_buffer[MTK_EFUSE_PUBK_HASH_INDEX_MAX];
	u_register_t buffer[4] = {0};

	switch (smc_fid) {
	case MTK_SIP_PWR_ON_MTCMOS:
		ret = mt_sip_pwr_on_mtcmos((uint32_t)x1);
		SMC_RET1(handle, ret);

	case MTK_SIP_PWR_OFF_MTCMOS:
		ret = mt_sip_pwr_off_mtcmos((uint32_t)x1);
		SMC_RET1(handle, ret);

	case MTK_SIP_PWR_MTCMOS_SUPPORT:
		ret = mt_sip_pwr_mtcmos_support();
		SMC_RET1(handle, ret);

	case MTK_SIP_EFUSE_GET_LEN:
		ret = mtk_efuse_get_len((uint32_t)x1, &efuse_len);
		SMC_RET4(handle, ret, efuse_len, 0x0, 0x0);

	case MTK_SIP_EFUSE_SEND_DATA:
		ret = mtk_efuse_send_data((uint8_t *)efuse_buffer,
					  (uint8_t *)efuse_data,
					  (uint32_t)x1,
					  (uint32_t)x2);
		SMC_RET4(handle, ret, x2, 0x0, 0x0);

	case MTK_SIP_EFUSE_GET_DATA:
		ret = mtk_efuse_get_data((uint8_t *)efuse_data,
					 (uint8_t *)efuse_buffer,
					 (uint32_t)x1,
					 (uint32_t)x2);
		SMC_RET4(handle, ret, x2, efuse_data[0], efuse_data[1]);

	case MTK_SIP_EFUSE_READ:
		ret = mtk_efuse_read((uint32_t)x1,
				     (uint8_t *)efuse_buffer,
				     sizeof(efuse_buffer));
		SMC_RET4(handle, ret, 0x0, 0x0, 0x0);

	case MTK_SIP_EFUSE_WRITE:
		ret = mtk_efuse_write((uint32_t)x1,
				      (uint8_t *)efuse_buffer,
				      sizeof(efuse_buffer));
		SMC_RET4(handle, ret, 0x0, 0x0, 0x0);

	case MTK_SIP_EFUSE_DISABLE:
		ret = mtk_efuse_disable((uint32_t)x1);
		SMC_RET4(handle, ret, 0x0, 0x0, 0x0);

	case MTK_SIP_FSEK_GET_SHM_CONFIG:
		if (GET_EL(read_spsr_el3()) == MODE_EL2) {
			ret = mtk_fsek_get_shm_config((uintptr_t *)&buffer[0],
						      (size_t *)&buffer[1]);
			SMC_RET3(handle, ret, buffer[0], buffer[1]);
		}

		SMC_RET3(handle, MTK_FSEK_ERR_WRONG_SRC, 0x0, 0x0);

	case MTK_SIP_FSEK_DECRYPT_RFSK:
		if (GET_EL(read_spsr_el3()) == MODE_EL2) {
			ret = mtk_fsek_decrypt_rfsk();
			SMC_RET1(handle, ret);
		}

		SMC_RET1(handle, MTK_FSEK_ERR_WRONG_SRC);

	case MTK_SIP_FSEK_GET_KEY:
		ret = mtk_fsek_get_key((uint32_t)x1,
				       (void *)buffer,
				       sizeof(buffer));
		if (!ret) {
			SMC_RET4(handle, buffer[0], buffer[1],
				 buffer[2], buffer[3]);
		} else {
			SMC_RET4(handle, ret, 0x0, 0x0, 0x0);
		}

		SMC_RET4(handle, MTK_FSEK_ERR_WRONG_SRC, 0x0, 0x0, 0x0);

	case MTK_SIP_FSEK_ENCRYPT_ROEK:
		ret = mtk_fsek_encrypt_roek(x1, x2, x3, x4,
					    (void *)buffer, sizeof(buffer));
		if (!ret) {
			SMC_RET4(handle, buffer[0], buffer[1],
				 buffer[2], buffer[3]);
		}
		else {
			SMC_RET4(handle, ret, 0x0, 0x0, 0x0);
		}

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}
