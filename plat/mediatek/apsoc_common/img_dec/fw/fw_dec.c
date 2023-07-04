// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 */

#include <common/debug.h>
#include <lib/spinlock.h>
#include <mbedtls_helper.h>
#include <explicit_zeroize.h>
#include <shm.h>
#include <mtk_roe.h>
#include <key_info.h>
#include <salt.h>
#include "fw_dec.h"

static spinlock_t fw_dec_lock;

static uint8_t fw_key[FW_KEY_SIZE];
static uint8_t kernel_key[FW_KEY_SIZE];
static uint8_t rootfs_key[FW_KEY_SIZE];

static int derive_fw_key(uint32_t key_idx, uint8_t *key, size_t key_len)
{
	int ret = 0;
	uint8_t *salt = NULL;
	uint8_t kernel_salt[] = KERNEL_KEY_SALT;
	uint8_t rootfs_salt[] = ROOTFS_KEY_SALT;

	switch (key_idx) {
	case KERNEL_KEY_IDX:
		salt = kernel_salt;
		break;
	case ROOTFS_KEY_IDX:
		salt = rootfs_salt;
		break;
	default:
		return -FW_DEC_KEY_IDX_ERR;
	}

	bl31_mbedtls_init();

	ret = derive_from_roe_key(salt, SALT_SIZE, key, key_len);
	if (ret)
		ERROR("Failed to derive key: %d\n", ret);

	bl31_mbedtls_deinit();

	return ret;
}

void fw_dec_init(void)
{
	int ret;

	ret = derive_fw_key(KERNEL_KEY_IDX, kernel_key, sizeof(kernel_key));
	if (ret)
		panic();

	ret = derive_fw_key(ROOTFS_KEY_IDX, rootfs_key, sizeof(rootfs_key));
	if (ret)
		panic();
}

static int set_fw_key(uint32_t key_idx)
{
	uint8_t *key = NULL;

	switch (key_idx) {
	case KERNEL_KEY_IDX:
		key = kernel_key;
		break;
	case ROOTFS_KEY_IDX:
		key = rootfs_key;
		break;
	default:
		return -FW_DEC_KEY_IDX_ERR;
	}

	memcpy(fw_key, key, FW_KEY_SIZE);

	return 0;
}

int fw_dec_cleanup(void)
{
	spin_lock(&fw_dec_lock);

	explicit_zeroize(fw_key, FW_KEY_SIZE);
	explicit_zeroize(kernel_key, FW_KEY_SIZE);
	explicit_zeroize(rootfs_key, FW_KEY_SIZE);

	spin_unlock(&fw_dec_lock);

	return 0;
}

#ifdef MTK_FW_ENC_VIA_BL31
#define SET_IV_FLAG		0x0001
#define SET_KEY_FLAG		0x0010
#define MEBDTLS_INIT_FLAG	0x0100
static uint8_t iv[IV_SIZE];
static uint32_t init_flag;
mbedtls_cipher_context_t aes_ctx;

int fw_dec_set_key(uint32_t key_idx)
{
	int ret = 0;

	spin_lock(&fw_dec_lock);

	ret = set_fw_key(key_idx);
	if (ret)
		goto out;

	init_flag |= SET_KEY_FLAG;

out:
	spin_unlock(&fw_dec_lock);

	return ret;
}

int fw_dec_set_iv(uintptr_t iv_paddr, uint32_t iv_size)
{
	uintptr_t iv_vaddr;
	int ret = 0;
	int stat;

	if (!iv_paddr || iv_size != IV_SIZE)
		return -FW_DEC_INVALID_PARAM_ERR;

	spin_lock(&fw_dec_lock);

	ret = set_shared_memory(iv_paddr, iv_size, &iv_vaddr,
				MT_MEMORY | MT_RO | MT_NS);
	if (ret) {
		spin_unlock(&fw_dec_lock);
		return ret;
	}

	memcpy(iv, (uint32_t *)iv_vaddr, iv_size);
	init_flag |= SET_IV_FLAG;

	stat = free_shared_memory(iv_vaddr, iv_size);
	if (stat)
		ret |= stat;

	spin_unlock(&fw_dec_lock);
	return ret;
}

static void fw_dec_mbedtls_deinit(void)
{
	init_flag = 0;

	explicit_zeroize(iv, IV_SIZE);
	explicit_zeroize(fw_key, FW_KEY_SIZE);

	mbedtls_cipher_free(&aes_ctx);

	bl31_mbedtls_deinit();
}

static int fw_dec_mbedtls_init(void)
{
	int ret = 0;
	const mbedtls_cipher_info_t *info = NULL;

	bl31_mbedtls_init();

	mbedtls_cipher_init(&aes_ctx);

	info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_CBC);
	if (!info) {
		VERBOSE("MBEDTLS: Failed to get cipher info\n");
		ret = -MTK_MBEDTLS_ERR_CRYPT;
		goto err;
	}

	ret = mbedtls_cipher_setup(&aes_ctx, info);
	if (ret) {
		VERBOSE("MBEDTLS: Failed to setup cipher: %d\n", ret);
		goto err;
	}

	ret = mbedtls_cipher_setkey(&aes_ctx, fw_key,
				    FW_KEY_SIZE << 3, MBEDTLS_DECRYPT);
	if (ret) {
		VERBOSE("MBEDTLS: Failed to set cipher key: %d\n", ret);
		goto err;
	}

	ret = mbedtls_cipher_set_iv(&aes_ctx, iv, IV_SIZE);
	if (ret) {
		VERBOSE("MBEDTLS: Failed to set cipher iv: %d\n", ret);
		goto err;
	}

	ret = mbedtls_cipher_set_padding_mode(&aes_ctx, MBEDTLS_PADDING_NONE);
	if (ret) {
		VERBOSE("MBEDTLS: Failed to set padding mode: %d\n", ret);
		goto err;
	}

	return ret;
err:
	fw_dec_mbedtls_deinit();
	return ret;
}


static int do_decrypt(uintptr_t image_paddr, uint32_t image_size, int last_block)
{
	uintptr_t cipher_vaddr, plain_vaddr;
	int ret, stat;
	size_t len = 0;

	ret = set_shared_memory(image_paddr, image_size, &cipher_vaddr,
				MT_MEMORY | MT_RW | MT_NS);
	if (ret)
		return ret;

	plain_vaddr = cipher_vaddr;

	ret = mbedtls_cipher_update(&aes_ctx, (const unsigned char *)cipher_vaddr, image_size,
				    (unsigned char *)plain_vaddr, &len);
	if (ret) {
		VERBOSE("MBEDTLS: Failed to update cipher: %d\n", ret);
		ret = -FW_DEC_IMAGE_DEC_ERR;
		fw_dec_mbedtls_deinit();
		goto out;
	}

	if (last_block == 1) {
		ret = mbedtls_cipher_finish(&aes_ctx, (uint8_t *)plain_vaddr, &len);
		if (ret) {
			VERBOSE("MBEDTLS: Failed to finish cipher: %d\n", ret);
			ret = -FW_DEC_IMAGE_DEC_ERR;
		}
		fw_dec_mbedtls_deinit();
	}

out:
	stat = free_shared_memory(cipher_vaddr, image_size);
	if (stat)
		ret |= stat;

	return ret;
}

int fw_dec_image(uintptr_t image_paddr, uint32_t image_size, uint32_t last_block)
{
	int ret = 0;

	if (!image_paddr || !image_size || image_size > MAX_SHM_SIZE)
		return -FW_DEC_INVALID_PARAM_ERR;

	spin_lock(&fw_dec_lock);

	if (!(init_flag & (SET_IV_FLAG | SET_KEY_FLAG))) {
		ret = -FW_DEC_PARAM_NOT_SET_ERR;
		goto out;
	}

	if (!(init_flag & MEBDTLS_INIT_FLAG)) {
		if (fw_dec_mbedtls_init()) {
			ret = -FW_DEC_INIT_ERR;
			goto out;
		}
		init_flag |= MEBDTLS_INIT_FLAG;
	}

	ret = do_decrypt(image_paddr, image_size, last_block);

out:
	spin_unlock(&fw_dec_lock);
	return ret;
}
#endif /* MTK_FW_ENC_VIA_BL31 */

#ifdef MTK_FW_ENC_VIA_OPTEE
int fw_dec_get_key(uint32_t key_idx, uintptr_t paddr, uint32_t len)
{
	int ret = 0;
	int stat;
	uintptr_t vaddr;

	if (!paddr || (paddr & PAGE_SIZE_MASK) || len != FW_KEY_SIZE)
		return -FW_DEC_INVALID_PARAM_ERR;

#if defined(BL32_TZRAM_BASE) && defined(BL32_TZRAM_SIZE)
	if (paddr < BL32_TZRAM_BASE ||
	    paddr > (BL32_TZRAM_BASE + BL32_TZRAM_SIZE - PAGE_SIZE))
		return -FW_DEC_INVALID_PARAM_ERR;
#else
	if (paddr < TZRAM2_BASE ||
	    paddr > (TZRAM2_BASE + TZRAM2_SIZE - PAGE_SIZE))
		return -FW_DEC_INVALID_PARAM_ERR;
#endif

	spin_lock(&fw_dec_lock);

	ret = set_shared_memory(paddr, len, &vaddr,
				MT_MEMORY | MT_RW | MT_SECURE);
	if (ret) {
		spin_unlock(&fw_dec_lock);
		return ret;
	}

	ret = set_fw_key(key_idx);
	if (ret)
		goto out;

	memcpy((uint8_t *)vaddr, fw_key, len);

	explicit_zeroize(fw_key, FW_KEY_SIZE);

out:
	stat = free_shared_memory(vaddr, len);
	if (stat)
		ret |= stat;

	spin_unlock(&fw_dec_lock);
	return ret;
}
#endif /* MTK_FW_ENC_VIA_OPTEE */
