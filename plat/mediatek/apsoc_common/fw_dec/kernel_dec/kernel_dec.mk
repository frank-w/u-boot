#
# Copyright (c) 2024, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq ($(FW_ENC),1)
ifneq ($(TRUSTED_BOARD_BOOT),1)
$(error You must enable TRUSTED_BOARD_BOOT when FW_ENC enabled)
endif
BL31_SOURCES		+=	drivers/auth/mbedtls/mbedtls_common.c \
				$(APSOC_COMMON)/fw_dec/kernel_dec/kernel_dec.c \
				$(APSOC_COMMON)/shm.c

BL31_CPPFLAGS		+=	-I$(APSOC_COMMON)/fw_dec/kernel_dec \
				-DMTK_FW_ENC

endif
