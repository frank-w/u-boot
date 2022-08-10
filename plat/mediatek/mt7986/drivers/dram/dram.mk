#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include $(MTK_PLAT_SOC)/drivers/dram/dram-configs.mk

BL2_CPPFLAGS		+=	-I$(MTK_PLAT_SOC)/drivers/dram
BL2_SOURCES		+=	$(MTK_PLAT_SOC)/drivers/dram/emicfg.c

ifeq ($(DRAM_DEBUG_LOG), 1)
BL2_LIBS		+=	$(MTK_PLAT_SOC)/drivers/dram/release/dram-debug.a
else
BL2_LIBS		+=	$(MTK_PLAT_SOC)/drivers/dram/release/dram.a
endif
