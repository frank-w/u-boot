#
# Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT		:=	plat/mediatek
MTK_PLAT_SOC		:=	$(MTK_PLAT)/$(PLAT)
APSOC_COMMON		:=	$(MTK_PLAT)/apsoc_common

# Enable workarounds for selected Cortex-A53 erratas.
ERRATA_A53_826319	:=	1
ERRATA_A53_836870	:=	1
ERRATA_A53_855873	:=	1

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1

# Do not enable SVE
ENABLE_SVE_FOR_NS	:=	0
MULTI_CONSOLE_API	:=	1

RESET_TO_BL2		:=	1

PLAT_INCLUDES		:=	-I$(APSOC_COMMON)				\
				-I$(APSOC_COMMON)/drivers/uart			\
				-Iinclude/plat/arm/common			\
				-Iinclude/plat/arm/common/aarch64		\
				-I$(MTK_PLAT_SOC)/drivers/pmic			\
				-I$(MTK_PLAT_SOC)/drivers/pll			\
				-I$(MTK_PLAT_SOC)/drivers/rtc			\
				-I$(MTK_PLAT_SOC)/drivers/spm			\
				-I$(MTK_PLAT_SOC)/drivers/timer			\
				-I$(MTK_PLAT_SOC)/drivers/wdt			\
				-I$(MTK_PLAT_SOC)/include

include $(MTK_PLAT_SOC)/bl2pl/bl2pl.mk
include $(MTK_PLAT_SOC)/bl2/bl2.mk
include $(MTK_PLAT_SOC)/bl31/bl31.mk
include $(MTK_PLAT_SOC)/drivers/efuse/efuse.mk

include $(APSOC_COMMON)/bl2/tbbr_post.mk
include $(MTK_PLAT_SOC)/bl2/ar_post.mk
include $(APSOC_COMMON)/bl2/bl2_image_post.mk
