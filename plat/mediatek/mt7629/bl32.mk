#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_SOURCES		+=	drivers/arm/cci/cci.c				\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				plat/common/plat_gicv2.c			\
				plat/common/plat_psci_common.c			\
				$(MTK_PLAT)/common/mtk_sip_svc_legacy.c		\
				$(MTK_PLAT)/common/mtk_bl31_boot_next_legacy.c	\
				$(MTK_PLAT)/common/drivers/efuse/mtk_efuse.c	\
				$(MTK_PLAT_SOC)/drivers/spm/spmc.c		\
				$(MTK_PLAT_SOC)/drivers/wdt/mtk_wdt.c		\
				$(MTK_PLAT_SOC)/plat_sip_calls.c		\
				$(MTK_PLAT_SOC)/plat_topology.c			\
				$(MTK_PLAT_SOC)/plat_pm.c			\
				$(MTK_PLAT_SOC)/plat_mt_gic.c

BL32_SOURCES		+=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch32/xlat_tables.c

$(eval $(call add_define,MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE))
