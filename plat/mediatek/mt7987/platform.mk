#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MTK_PLAT		:=	plat/mediatek
MTK_PLAT_SOC		:=	$(MTK_PLAT)/$(PLAT)
APSOC_COMMON		:=	$(MTK_PLAT)/apsoc_common

ifeq ($(FPGA),1)
MTK_PLAT_SOC_BSP	:=	$(MTK_PLAT_SOC)/fpga
include $(MTK_PLAT_SOC_BSP)/fpga.mk
else
MTK_PLAT_SOC_BSP	:=	$(MTK_PLAT_SOC)
endif

# indicate the reset vector address can be programmed
PROGRAMMABLE_RESET_ADDRESS	:=	1

ENABLE_PIE		:=	1

# Do not enable SVE
ENABLE_SVE_FOR_NS	:=	0
MULTI_CONSOLE_API	:=	1

RESET_TO_BL2		:=	1

PLAT_INCLUDES		+=	-Iinclude/plat/arm/common			\
				-Iinclude/plat/arm/common/aarch64		\
				-I$(APSOC_COMMON)				\
				-I$(APSOC_COMMON)/drivers/uart			\
				-I$(APSOC_COMMON)/drivers/trng/v2		\
				-I$(APSOC_COMMON)/drivers/wdt			\
				-I$(MTK_PLAT_SOC)/drivers/dram			\
				-I$(MTK_PLAT_SOC)/drivers/pll			\
				-I$(MTK_PLAT_SOC)/drivers/spmc			\
				-I$(MTK_PLAT_SOC)/drivers/timer			\
				-I$(MTK_PLAT_SOC)/drivers/devapc		\
				-I$(MTK_PLAT_SOC)/include

include $(MTK_PLAT_SOC_BSP)/cpu.mk
include $(MTK_PLAT_SOC_BSP)/bl2pl/bl2pl.mk
include $(MTK_PLAT_SOC_BSP)/bl2/bl2.mk
include $(MTK_PLAT_SOC_BSP)/bl31/bl31.mk
include $(MTK_PLAT_SOC_BSP)/bl32.mk

include $(APSOC_COMMON)/bl2/tbbr_post.mk
include $(APSOC_COMMON)/bl2/bl2_image_post.mk
