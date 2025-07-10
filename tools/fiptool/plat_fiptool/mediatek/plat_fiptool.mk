#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Name of the platform defined source file name,
# which contains platform defined UUID entries populated
# in the plat_def_toc_entries[].
PLAT_DEF_UUID_FILE_NAME	:= plat_def_uuid_config

FIPTOOL_INCLUDE_DIRS	+= ../../plat/mediatek/apsoc_common/bl2/include ./

PLAT_DEF_UUID		:= yes

ifeq (${PLAT_DEF_UUID},yes)
FIPTOOL_DEFINES += PLAT_DEF_FIP_UUID MTK_FIP_CHKSUM
FIPTOOL_SOURCES += plat_fiptool/mediatek/${PLAT_DEF_UUID_FILE_NAME}.c	\
	plat_fiptool/mediatek/sha256.c	\
	plat_fiptool/mediatek/crc32.c
endif
