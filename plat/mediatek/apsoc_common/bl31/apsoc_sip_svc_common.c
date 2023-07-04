// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 */

#include <errno.h>
#include <arch/aarch64/arch.h>
#include <common/runtime_svc.h>
#include <common/bl_common.h>
#include "mtk_boot_next.h"
#include "mtk_sip_svc.h"
#include "apsoc_sip_svc_common.h"
#ifdef MTK_EFUSE_PRESENT
#include <mtk_efuse.h>
#endif
#ifdef MTK_FSEK
#include <mtk_fsek.h>
#endif
#ifdef MTK_FW_ENC
#include <kernel_dec.h>
#endif
#include "memdump.h"
#include <mtk_wdt.h>
#include <platform_def.h>

#if MTK_SIP_KERNEL_BOOT_ENABLE
static uintptr_t apsoc_sip_boot_to_kernel(uint32_t smc_fid, u_register_t x1,
					  u_register_t x2, u_register_t x3,
					  u_register_t x4, void *cookie,
					  void *handle, u_register_t flags)
{
	boot_to_kernel(x1, x2, x3, x4);
	SMC_RET0(handle);
}
#endif

static uintptr_t apsoc_sip_get_bl31_region(uint32_t smc_fid, u_register_t x1,
					   u_register_t x2, u_register_t x3,
					   u_register_t x4, void *cookie,
					   void *handle, u_register_t flags)
{
	uintptr_t base, size;

#ifdef BL31_LOAD_OFFSET
	base = BL31_START - BL31_LOAD_OFFSET;
	size = TZRAM_SIZE;
#else
	base = TZRAM_BASE;
	size = TZRAM_SIZE;
#endif

	SMC_RET3(handle, 0, base, size);
}

static uintptr_t apsoc_sip_get_bl32_region(uint32_t smc_fid, u_register_t x1,
					   u_register_t x2, u_register_t x3,
					   u_register_t x4, void *cookie,
					   void *handle, u_register_t flags)
{
	uintptr_t base = 0, size = 0;

#ifdef NEED_BL32
#if defined(BL32_TZRAM_BASE) && defined(BL32_TZRAM_SIZE)
	base = BL32_TZRAM_BASE;
	size = BL32_TZRAM_SIZE;
#else
	base = TZRAM2_BASE;
	size = TZRAM2_SIZE;
#endif
#endif

	SMC_RET3(handle, 0, base, size);
}

#ifdef MTK_EFUSE_PRESENT
static uintptr_t apsoc_sip_efuse_get_len(uint32_t smc_fid, u_register_t x1,
					 u_register_t x2, u_register_t x3,
					 u_register_t x4, void *cookie,
					 void *handle, u_register_t flags)
{
	uint32_t efuse_len = 0;
	uintptr_t ret;

	ret = efuse_validate_field((uint32_t)x1, flags);
	if (ret)
		SMC_RET1(handle, ret);

	ret = mtk_efuse_get_len((uint32_t)x1, &efuse_len);
	SMC_RET2(handle, ret, efuse_len);
}

static uintptr_t apsoc_sip_efuse_read(uint32_t smc_fid, u_register_t x1,
				      u_register_t x2, u_register_t x3,
				      u_register_t x4, void *cookie,
				      void *handle, u_register_t flags)
{
	uint32_t val = 0;
	uintptr_t ret;

	ret = efuse_validate_field((uint32_t)x1, flags);
	if (ret)
		SMC_RET1(handle, ret);

	ret = sip_efuse_read((uint32_t)x1, (uint32_t)x2, &val);
	SMC_RET2(handle, ret, val);
}

static uintptr_t apsoc_sip_efuse_write(uint32_t smc_fid, u_register_t x1,
				       u_register_t x2, u_register_t x3,
				       u_register_t x4, void *cookie,
				       void *handle, u_register_t flags)
{
	uintptr_t ret;

	ret = efuse_validate_field((uint32_t)x1, flags);
	if (ret)
		SMC_RET1(handle, ret);

	ret = sip_efuse_write((uint32_t)x1, (uintptr_t)x2, (uint32_t)x3);
	SMC_RET1(handle, ret);
}

static uintptr_t apsoc_sip_efuse_disable(uint32_t smc_fid, u_register_t x1,
					 u_register_t x2, u_register_t x3,
					 u_register_t x4, void *cookie,
					 void *handle, u_register_t flags)
{
	uintptr_t ret;

	ret = efuse_validate_field((uint32_t)x1, flags);
	if (ret)
		SMC_RET1(handle, ret);

	ret = mtk_efuse_disable((uint32_t)x1);
	SMC_RET1(handle, ret);
}
#endif /* MTK_EFUSE_PRESENT */

#ifdef MTK_FSEK
static uintptr_t apsoc_sip_fsek_get_shm_config(uint32_t smc_fid,
					       u_register_t x1,
					       u_register_t x2, u_register_t x3,
					       u_register_t x4, void *cookie,
					       void *handle, u_register_t flags)
{
	u_register_t buffer[4] = {0};
	uintptr_t ret;

	if (GET_EL(read_spsr_el3()) == MODE_EL2) {
		ret = mtk_fsek_get_shm_config((uintptr_t *)&buffer[0],
					      (size_t *)&buffer[1]);
		SMC_RET3(handle, ret, buffer[0], buffer[1]);
	}

	SMC_RET3(handle, MTK_FSEK_ERR_WRONG_SRC, 0x0, 0x0);
}

static uintptr_t apsoc_sip_fsek_decrypt_rfsk(uint32_t smc_fid,
					     u_register_t x1,
					     u_register_t x2, u_register_t x3,
					     u_register_t x4, void *cookie,
					     void *handle, u_register_t flags)
{
	uintptr_t ret;

	if (GET_EL(read_spsr_el3()) == MODE_EL2) {
		ret = mtk_fsek_decrypt_rfsk();
		SMC_RET1(handle, ret);
	}

	SMC_RET1(handle, MTK_FSEK_ERR_WRONG_SRC);
}

static uintptr_t apsoc_sip_fsek_get_key(uint32_t smc_fid,
					u_register_t x1,
					u_register_t x2, u_register_t x3,
					u_register_t x4, void *cookie,
					void *handle, u_register_t flags)
{
	u_register_t buffer[4] = {0};
	uintptr_t ret;

	ret = mtk_fsek_get_key((uint32_t)x1, (void *)buffer, sizeof(buffer));
	if (!ret) {
		SMC_RET4(handle, buffer[0], buffer[1], buffer[2], buffer[3]);
	}

	SMC_RET4(handle, MTK_FSEK_ERR_WRONG_SRC, 0x0, 0x0, 0x0);
}

static uintptr_t apsoc_sip_fsek_encrypt_roek(uint32_t smc_fid,
					     u_register_t x1,
					     u_register_t x2, u_register_t x3,
					     u_register_t x4, void *cookie,
					     void *handle, u_register_t flags)
{
	u_register_t buffer[4] = {0};
	uintptr_t ret;

	ret = mtk_fsek_encrypt_roek(x1, x2, x3, x4,
				    (void *)buffer, sizeof(buffer));
	if (!ret) {
		SMC_RET4(handle, buffer[0], buffer[1], buffer[2], buffer[3]);
	}

	SMC_RET4(handle, ret, 0x0, 0x0, 0x0);
}
#endif /* MTK_FSEK */

#ifdef EMERG_MEM_DUMP
static uintptr_t apsoc_sip_emerg_mem_dump(uint32_t smc_fid, u_register_t x1,
					  u_register_t x2, u_register_t x3,
					  u_register_t x4, void *cookie,
					  void *handle, u_register_t flags)
{
	do_mem_dump((int)x1, x2, handle);
	SMC_RET0(handle);
}

static uintptr_t apsoc_sip_emerg_mem_dump_cfg(uint32_t smc_fid, u_register_t x1,
					  u_register_t x2, u_register_t x3,
					  u_register_t x4, void *cookie,
					  void *handle, u_register_t flags)
{
	uintptr_t val;

	if ((uint32_t)x1 == 0) {
		switch ((uint32_t)x2) {
		case 0:
			val = mem_dump_get_local_ip();
			SMC_RET2(handle, 0, val);
			break;

		case 1:
			val = mem_dump_get_dest_ip();
			SMC_RET2(handle, 0, val);
			break;

		default:
			SMC_RET1(handle, (intptr_t)-EINVAL);
		}
	} else {
		switch ((uint32_t)x2) {
		case 0:
			mem_dump_set_local_ip((uint32_t)x3);
			SMC_RET1(handle, 0);
			break;

		case 1:
			mem_dump_set_dest_ip((uint32_t)x3);
			SMC_RET1(handle, 0);
			break;

		default:
			SMC_RET1(handle, (intptr_t)-EINVAL);
		}
	}
}
#endif

static uintptr_t apsoc_sip_read_nonrst_reg(uint32_t smc_fid, u_register_t x1,
					   u_register_t x2, u_register_t x3,
					   u_register_t x4, void *cookie,
					   void *handle, u_register_t flags)
{
	uintptr_t val = mtk_wdt_read_nonrst(x1);
	SMC_RET1(handle, val);
}

static uintptr_t apsoc_sip_write_nonrst_reg(uint32_t smc_fid, u_register_t x1,
					   u_register_t x2, u_register_t x3,
					   u_register_t x4, void *cookie,
					   void *handle, u_register_t flags)
{
	mtk_wdt_write_nonrst((uint32_t)x1, (uint32_t)x2);
	SMC_RET0(handle);
}

#ifdef MTK_FW_ENC
static uintptr_t apsoc_sip_fw_dec_set_iv(uint32_t smc_fid, u_register_t x1,
					   u_register_t x2, u_register_t x3,
					   u_register_t x4, void *cookie,
					   void *handle, u_register_t flags)
{
	int ret = 0;

	ret = fw_dec_set_iv(x1, x2);
	SMC_RET1(handle, ret);
}

static uintptr_t apsoc_sip_fw_dec_set_salt(uint32_t smc_fid, u_register_t x1,
					    u_register_t x2, u_register_t x3,
					    u_register_t x4, void *cookie,
					    void *handle, u_register_t flags)
{
	int ret = 0;

	ret = fw_dec_set_salt(x1, x2);
	SMC_RET1(handle, ret);
}

static uintptr_t apsoc_sip_fw_dec_image(uint32_t smc_fid, u_register_t x1,
					   u_register_t x2, u_register_t x3,
					   u_register_t x4, void *cookie,
					   void *handle, u_register_t flags)
{
	int ret = 0;

	ret = fw_dec_image(x1, x2);
	SMC_RET1(handle, ret);
}
#endif

struct mtk_sip_call_record apsoc_common_sip_calls[] = {
#ifdef MTK_SIP_KERNEL_BOOT_ENABLE
	MTK_SIP_CALL_RECORD(MTK_SIP_KERNEL_BOOT_AARCH32, apsoc_sip_boot_to_kernel),
#endif
	MTK_SIP_CALL_RECORD(MTK_SIP_GET_BL31_REGION, apsoc_sip_get_bl31_region),
	MTK_SIP_CALL_RECORD(MTK_SIP_GET_BL32_REGION, apsoc_sip_get_bl32_region),
#ifdef MTK_EFUSE_PRESENT
	MTK_SIP_CALL_RECORD(MTK_SIP_EFUSE_GET_LEN, apsoc_sip_efuse_get_len),
	MTK_SIP_CALL_RECORD(MTK_SIP_EFUSE_READ, apsoc_sip_efuse_read),
	MTK_SIP_CALL_RECORD(MTK_SIP_EFUSE_WRITE, apsoc_sip_efuse_write),
	MTK_SIP_CALL_RECORD(MTK_SIP_EFUSE_DISABLE, apsoc_sip_efuse_disable),
#endif
#ifdef MTK_FSEK
	MTK_SIP_CALL_RECORD(MTK_SIP_FSEK_GET_SHM_CONFIG, apsoc_sip_fsek_get_shm_config),
	MTK_SIP_CALL_RECORD(MTK_SIP_FSEK_DECRYPT_RFSK, apsoc_sip_fsek_decrypt_rfsk),
	MTK_SIP_CALL_RECORD(MTK_SIP_FSEK_GET_KEY, apsoc_sip_fsek_get_key),
	MTK_SIP_CALL_RECORD(MTK_SIP_FSEK_ENCRYPT_ROEK, apsoc_sip_fsek_encrypt_roek),
#endif /* MTK_FSEK */
#ifdef EMERG_MEM_DUMP
	MTK_SIP_CALL_RECORD(MTK_SIP_EMERG_MEM_DUMP, apsoc_sip_emerg_mem_dump),
	MTK_SIP_CALL_RECORD(MTK_SIP_EMERG_MEM_DUMP_CFG, apsoc_sip_emerg_mem_dump_cfg),
#endif
	MTK_SIP_CALL_RECORD(MTK_SIP_READ_NONRST_REG, apsoc_sip_read_nonrst_reg),
	MTK_SIP_CALL_RECORD(MTK_SIP_WRITE_NONRST_REG, apsoc_sip_write_nonrst_reg),
#ifdef MTK_FW_ENC
	MTK_SIP_CALL_RECORD(MTK_SIP_FW_DEC_SET_IV, apsoc_sip_fw_dec_set_iv),
	MTK_SIP_CALL_RECORD(MTK_SIP_FW_DEC_SET_SALT, apsoc_sip_fw_dec_set_salt),
	MTK_SIP_CALL_RECORD(MTK_SIP_FW_DEC_IMAGE, apsoc_sip_fw_dec_image),
#endif
};

struct mtk_sip_call_record apsoc_common_sip_calls_from_sec[] = {
#ifdef MTK_EFUSE_PRESENT
	MTK_SIP_CALL_RECORD(MTK_SIP_EFUSE_READ, apsoc_sip_efuse_read),
	MTK_SIP_CALL_RECORD(MTK_SIP_EFUSE_GET_LEN, apsoc_sip_efuse_get_len),
#endif
};

const uint32_t apsoc_common_sip_call_num = ARRAY_SIZE(apsoc_common_sip_calls);
const uint32_t apsoc_common_sip_call_num_from_sec = ARRAY_SIZE(apsoc_common_sip_calls_from_sec);
