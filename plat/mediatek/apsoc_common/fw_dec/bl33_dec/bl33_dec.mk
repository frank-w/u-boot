#
# Copyright (c) 2024, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq ($(FIP_ENC),1)
ifneq ($(TRUSTED_BOARD_BOOT),1)
$(error You must enable TRUSTED_BOARD_BOOT when FW_ENC enabled)
endif

include drivers/auth/mbedtls/mbedtls_crypto.mk

BL31_SOURCES		+=	$(APSOC_COMMON)/shm.c \
				$(APSOC_COMMON)/fw_dec/bl33_dec/bl33_dec.c

BL31_CPPFLAGS		+=	-I$(APSOC_COMMON)/fw_dec/bl33_dec \
				-DMTK_FIP_ENC

endif
