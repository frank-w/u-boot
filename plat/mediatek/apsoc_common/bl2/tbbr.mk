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

AUTH_SOURCES		:=	drivers/auth/auth_mod.c				\
				drivers/auth/crypto_mod.c			\
				drivers/auth/img_parser_mod.c			\
				drivers/auth/tbbr/tbbr_cot_bl2.c		\
				drivers/auth/tbbr/tbbr_cot_common.c

BL2_SOURCES		+=	$(AUTH_SOURCES)					\
				$(APSOC_COMMON)/bl2/mtk_tbbr.c			\
				$(APSOC_COMMON)/bl2/mtk_rotpk.S

PLAT_BL_COMMON_SOURCES	+=	$(APSOC_COMMON)/mbedtls_helper.c

ROTPK_HASH		:=	$(BUILD_PLAT)/rotpk_$(HASH_ALG).bin
$(eval $(call add_define_val,ROTPK_HASH,'"$(ROTPK_HASH)"'))

ifeq ($(FIP_ENC),1)
SALTDEP 		:=	SALT
SALT_PATH		:=	salt
AUTOGEN_SALT_HEADER	:=	$(APSOC_COMMON)/salt.h
SALT_TOOL		:=	./tools/mediatek/gen_salt/gen_salt.sh
ENC_KEY 		:=	$(shell cat $(ENC_KEY_PATH) | od -A n -t x2 --endian=big | sed 's/ *//g' | tr -d '\n')
ENC_NONCE		:=	$(shell cat $(ENC_NONCE_PATH) | od -A n -t x2 --endian=big | sed 's/ *//g' | tr -d '\n')
ENC_ARGS		+=	-s $(shell cat $(SALT_PATH) | od -A n -t x2 --endian=big | sed 's/ *//g' | tr -d '\n')
BL2_CPPFLAGS		+=	-I$(APSOC_COMMON)/fw_dec/ \
				-DMTK_FIP_ENC
BL2_SOURCES		+=	drivers/io/io_encrypted.c
$(SALTDEP):
	$(SALT_TOOL) $(SALT_PATH) $(AUTOGEN_SALT_HEADER)

endif
endif
