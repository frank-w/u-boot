#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# FSEK related build macros
#
ifeq ($(FSEK),1)
ifneq ($(TRUSTED_BOARD_BOOT),1)
$(error You must enable TRUSTED_BOARD_BOOT when FSEK enabled)
endif
BL31_SOURCES		+=	$(MTK_PLAT_SOC)/mtk_tbbr.c			\
				drivers/auth/mbedtls/mbedtls_common.c		\
				$(MTK_PLAT)/common/mbedtls_helper.c		\
				$(MTK_PLAT)/common/mtk_fsek.c
BL31_CPPFLAGS		+=	-DMTK_FSEK

#
# SW HUK related build macros
#
BL31_SOURCES		+=	$(MTK_PLAT)/common/mtk_huk.c			\
				$(MTK_PLAT)/common/drivers/crypto/mtk_crypto.c
BL31_INCLUDES		+=	-I$(MTK_PLAT)/common/drivers/crypto		\
				-I$(MTK_PLAT)/common/drivers/crypto/include
LDLIBS			+=	$(MTK_PLAT_SOC)/drivers/crypto/release/libcrypto.a

#
# SW ROEK related build macros
#
BL31_SOURCES		+=	$(MTK_PLAT)/common/mtk_roek.c
ifeq ($(ENC_ROEK),1)
BL31_CPPFLAGS		+=	-DMTK_ENC_ROEK
endif
endif
