// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 */

#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>
#include <common/debug.h>
#include <mbedtls/memory_buffer_alloc.h>
#include <mbedtls/platform_util.h>
#include <mbedtls/hkdf.h>
#include "mbedtls_helper.h"
#include <plat/common/platform.h>

#include <mbedtls_helper.h>
#include <kernel_dec.h>
#include <mtk_roe.h>
#include <shm.h>

static uint8_t iv[IV_SIZE];
static uint8_t salt[SALT_SIZE];
static uint8_t fw_key[FW_KEY_SIZE];
static uint8_t iv_flag;
static uint8_t salt_flag;

static int fw_get_key(void)
{
	int ret = 0;
	size_t key_len;

	ret = derive_roe_key(salt, SALT_SIZE, fw_key, &key_len);
	if (ret) {
		ERROR("derive_roe_key failed %d\n", ret);
		memset(fw_key, 0, FW_KEY_SIZE);
	}
	return ret;
}

int fw_dec_set_iv(uintptr_t iv_paddr, uint32_t iv_size)
{
	uintptr_t iv_vaddr;
	int ret = 0;

	if (!iv_paddr || !iv_size || iv_size != IV_SIZE) {
		ERROR("FW_DEC: iv is 0 or iv_size is 0.\n");
		return -1;
	}

	ret = set_shared_memory(iv_paddr, iv_size, &iv_vaddr,
				MT_MEMORY | MT_RO | MT_NS);
	if (ret) {
		ERROR("FW_DEC: iv set shared memory failed: %d\n", ret);
		return ret;
	}

	memcpy(iv, (uint32_t *) iv_vaddr, iv_size);
	iv_flag = 1;

	ret = free_shared_memory(iv_vaddr, iv_size);
	if (ret)
		ERROR("FW_DEC: iv free shared memory failed: %d\n", ret);

	return ret;
}

int fw_dec_set_salt(uintptr_t salt_paddr, uint32_t salt_size)
{
	uintptr_t salt_vaddr;
	int ret = 0;

	if (!salt_paddr || !salt_size || salt_size != SALT_SIZE) {
		ERROR("FW_DEC: salt is 0 or salt_size is 0.\n");
		return -1;
	}

	bl31_mbedtls_init();

	ret = set_shared_memory(salt_paddr, salt_size, &salt_vaddr,
				MT_MEMORY | MT_RO | MT_NS);
	if (ret) {
		ERROR("FW_DEC: salt set shared memory failed: %d\n", ret);
		goto out;
	}

	memcpy(salt, (uint32_t *) salt_vaddr, salt_size);

	ret = fw_get_key();
	if (ret)
		ERROR("FW_DEC: fw_get_key failed: %d\n", ret);

	ret = free_shared_memory(salt_vaddr, salt_size);
	if (ret)
		ERROR("FW_DEC: salt free shared memory failed: %d\n", ret);

out:
	if (!ret)
		salt_flag = 1;
	bl31_mbedtls_deinit();

	return ret;
}

static int do_decrypt(uint8_t *cipher, uint32_t cipher_size,
		      uint8_t *plain, uint32_t plain_size)
{
	int ret = 0;

	bl31_mbedtls_init();

	ret = aes_cbc_crypt(cipher, cipher_size,
			    fw_key, FW_KEY_SIZE,
			    iv, IV_SIZE,
			    plain, MBEDTLS_DECRYPT,
			    MBEDTLS_PADDING_NONE);
	if (ret) {
		ERROR("FW_DEC: image decrypt failed %d\n", ret);
		ret = -MTK_FW_DEC_IMAGE_DEC_ERR;
	}

	bl31_mbedtls_deinit();
	return ret;
}

int fw_dec_image(uintptr_t image_paddr, uint32_t image_size)
{
	uintptr_t cipher_vaddr, plain_vaddr;
	int ret = 0;

	if (!image_paddr || !image_size) {
		ERROR("FW_DEC: image is 0 or image_size is 0.\n");
		return -MTK_FW_DEC_INPUT_ERROR;
	}

	ret = set_shared_memory(image_paddr, image_size, &cipher_vaddr,
				MT_MEMORY | MT_RW | MT_NS);
	if (ret) {
		ERROR("FW_DEC: image set shared memory failed: %d\n", ret);
		return ret;
	}

	if (!iv_flag || !salt_flag) {
		ERROR("FW_DEC: iv or salt not set\n");
		ret = -MTK_FW_DEC_IV_OR_SALT_NOT_SET;
		goto out;
	}

	/* plain use same buffer with cipher */
	plain_vaddr = cipher_vaddr;

	ret = do_decrypt((uint8_t *) cipher_vaddr, image_size,
			 (uint8_t *) plain_vaddr, image_size);
	if(ret) {
		ERROR("FW_DEC: do_decrypt failed: %d\n", ret);
		goto out;
	}

out:
	ret = free_shared_memory(cipher_vaddr, image_size);
	if (ret)
		ERROR("FW_DEC: image free shared memory failed: %d\n", ret);

	memset(iv, 0, IV_SIZE);
	memset(salt, 0, SALT_SIZE);
	memset(fw_key, 0, FW_KEY_SIZE);
	iv_flag = 0;
	salt_flag = 0;
	return ret;
}
