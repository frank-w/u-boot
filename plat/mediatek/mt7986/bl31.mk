#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/xlat_tables_v2/xlat_tables.mk

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

BL31_SOURCES		+=	drivers/arm/cci/cci.c				\
				$(GICV3_SOURCES)				\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				plat/common/plat_gicv3.c			\
				$(MTK_PLAT)/common/drivers/uart/aarch64/hsuart.S\
				$(MTK_PLAT)/common/mtk_plat_common.c		\
				$(MTK_PLAT)/common/drivers/gpt/mt_gpt.c		\
				$(MTK_PLAT)/common/drivers/efuse/mtk_efuse.c	\
				$(MTK_PLAT)/common/mtk_sip_svc_legacy.c		\
				$(MTK_PLAT)/common/mtk_bl31_boot_next_legacy.c	\
				$(MTK_PLAT_SOC)/aarch64/plat_helpers.S		\
				$(MTK_PLAT_SOC)/aarch64/platform_common.c	\
				$(MTK_PLAT_SOC)/bl31_plat_setup.c		\
				$(MTK_PLAT_SOC)/plat_mt_gic.c			\
				$(MTK_PLAT_SOC)/plat_pm.c			\
				$(MTK_PLAT_SOC)/plat_sip_calls.c		\
				$(MTK_PLAT_SOC)/drivers/spmc/mtspmc.c		\
				$(MTK_PLAT_SOC)/drivers/timer/timer.c		\
				$(MTK_PLAT_SOC)/drivers/spmc/mtspmc.c		\
				$(MTK_PLAT_SOC)/drivers/dram/emi_mpu.c		\
				$(MTK_PLAT_SOC)/drivers/devapc/devapc.c		\
				$(MTK_PLAT_SOC)/drivers/wdt/mtk_wdt.c		\
				$(MTK_PLAT_SOC)/plat_topology.c

BL31_SOURCES		+=	$(XLAT_TABLES_LIB_SRCS)
BL31_CPPFLAGS		+=	-DPLAT_XLAT_TABLES_DYNAMIC

include $(MTK_PLAT)/apsoc_common/fsek.mk

$(eval $(call add_define,MTK_SIP_SET_AUTHORIZED_SECURE_REG_ENABLE))

MTK_SIP_KERNEL_BOOT_ENABLE := 1
$(eval $(call add_define,MTK_SIP_KERNEL_BOOT_ENABLE))

# BL33 is AArch64
BL31_CPPFLAGS		+=	-DKERNEL_IS_DEFAULT_64BIT
