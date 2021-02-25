#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v2/gicv2_main.c			\
				drivers/arm/gic/v2/gicv2_helpers.c		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				$(MTK_PLAT)/common/drivers/uart/aarch64/hsuart.S\
				$(MTK_PLAT)/common/mtk_plat_common.c		\
				$(MTK_PLAT)/common/mtk_sip_svc_legacy.c		\
				$(MTK_PLAT)/common/mtk_bl31_boot_next_legacy.c	\
				$(MTK_PLAT)/common/drivers/efuse/mtk_efuse.c	\
				$(MTK_PLAT_SOC)/aarch64/plat_helpers.S		\
				$(MTK_PLAT_SOC)/aarch64/platform_common.c	\
				$(MTK_PLAT_SOC)/bl31_plat_setup.c		\
				$(MTK_PLAT_SOC)/drivers/pmic/pmic_wrap.c	\
				$(MTK_PLAT_SOC)/drivers/rtc/rtc.c		\
				$(MTK_PLAT_SOC)/drivers/spm/mtcmos.c		\
				$(MTK_PLAT_SOC)/drivers/spm/spm.c		\
				$(MTK_PLAT_SOC)/drivers/spm/spm_hotplug.c	\
				$(MTK_PLAT_SOC)/drivers/spm/spm_mcdi.c		\
				$(MTK_PLAT_SOC)/drivers/spm/spm_suspend.c	\
				$(MTK_PLAT_SOC)/drivers/timer/cpuxgpt.c		\
				$(MTK_PLAT_SOC)/drivers/wdt/mtk_wdt.c		\
				$(MTK_PLAT_SOC)/plat_pm.c			\
				$(MTK_PLAT_SOC)/plat_sip_calls.c		\
				$(MTK_PLAT_SOC)/plat_topology.c			\
				$(MTK_PLAT_SOC)/plat_mt_gic.c			\
				$(MTK_PLAT_SOC)/power_tracer.c			\
				$(MTK_PLAT_SOC)/scu.c				\
				$(MTK_PLAT_SOC)/mtk_ar_table.c

BL31_SOURCES		+=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				plat/arm/common/arm_gicv2.c			\
				plat/common/plat_gicv2.c


$(eval $(call add_define,MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE))

# BL33 is AArch64
BL31_CPPFLAGS		+=	-DKERNEL_IS_DEFAULT_64BIT
