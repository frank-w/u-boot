/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 */

#ifndef FW_DEC_H
#define FW_DEC_H

int fw_dec_set_iv(uintptr_t iv_paddr, uint32_t iv_size);
int fw_dec_set_salt(uintptr_t salt_paddr, uint32_t salt_size);
int fw_dec_image(uintptr_t image_paddr, uint32_t image_size);

#define MTK_FW_DEC_ALLOC_SHARED_MEMORY_ERR	1
#define MTK_FW_DEC_FREE_SHARED_MEMORY_ERR	2
#define MTK_FW_DEC_HKDF_ERR			3
#define MTK_FW_DEC_IMAGE_DEC_ERR		4
#define MTK_FW_DEC_IV_OR_SALT_NOT_SET		5
#define MTK_FW_DEC_INPUT_ERROR			6

#define IV_SIZE 				16
#define SALT_SIZE				32
#define FW_KEY_SIZE				32
#endif
