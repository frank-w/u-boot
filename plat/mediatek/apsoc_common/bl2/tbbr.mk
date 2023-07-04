#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Trusted board boot
#
ifeq ($(TRUSTED_BOARD_BOOT),1)

include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk

ifeq ($(MBEDTLS_DIR),)
$(error You must specify MBEDTLS_DIR when TRUSTED_BOARD_BOOT enabled)
endif

BL2_CPPFLAGS		+=	-DMTK_EFUSE_FIELD_NORMAL

AUTH_SOURCES		:=	drivers/auth/auth_mod.c				\
				drivers/auth/crypto_mod.c			\
				drivers/auth/img_parser_mod.c			\
				drivers/auth/tbbr/tbbr_cot_bl2.c		\
				drivers/auth/tbbr/tbbr_cot_common.c

BL2_SOURCES		+=	$(AUTH_SOURCES)					\
				$(MTK_PLAT_SOC)/mtk_tbbr.c			\
				$(MTK_PLAT_SOC)/mtk_rotpk.S

ROTPK_HASH		:=	$(BUILD_PLAT)/rotpk_sha256.bin
$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))

endif
