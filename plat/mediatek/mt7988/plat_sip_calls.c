/*
 * copyright (c) 2021, mediatek inc. all rights reserved.
 *
 * spdx-license-identifier: bsd-3-clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/utils_def.h>
#include <lib/mmio.h>
#include <mtk_sip_svc.h>
#include <plat_sip_calls.h>
#include <string.h>
#include <rng.h>

/* Authorized secure register list */
enum { SREG_HDMI_COLOR_EN = 0x14000904 };

static const uint32_t authorized_sreg[] = { SREG_HDMI_COLOR_EN };

#define authorized_sreg_cnt ARRAY_SIZE(authorized_sreg)

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
	uint32_t value = 0;

	switch (smc_fid) {
	case MTK_SIP_TRNG_GET_RND:
		ret = plat_get_rnd(&value);
		SMC_RET2(handle, ret, value);

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}
