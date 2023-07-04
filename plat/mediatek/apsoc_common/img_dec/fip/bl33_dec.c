// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 */

#include <common/debug.h>
#include <tools_share/firmware_encrypted.h>
#include <mbedtls_helper.h>
#include <explicit_zeroize.h>
#include <shm.h>
#include <mtk_roe.h>
#include <key_info.h>
#include <salt.h>

#define ENC_HEADER_CHECK_ERR		1

static int get_fw_enc_hdr(struct fw_enc_hdr *header)
{
	uint8_t *ptr = NULL;
	uintptr_t shm_vaddr = 0;
	int ret = 0;
	int stat;

	ret = set_shared_memory((uintptr_t)BL33_BASE, sizeof(struct fw_enc_hdr),
				&shm_vaddr, MT_MEMORY | MT_RO | MT_NS);
	if (ret)
		return ret;

	ptr = (uint8_t *)shm_vaddr;
	memcpy(header, ptr, sizeof(struct fw_enc_hdr));
	if(header->magic != ENC_HEADER_MAGIC)
		ret = -ENC_HEADER_CHECK_ERR;

	if ((header->iv_len > ENC_MAX_IV_SIZE) ||
	    (header->tag_len > ENC_MAX_TAG_SIZE))
		ret = -ENC_HEADER_CHECK_ERR;

	if (!header->image_len ||
	    header->image_len > MAX_SHM_SIZE - sizeof(struct fw_enc_hdr))
		ret = -ENC_HEADER_CHECK_ERR;

	stat = free_shared_memory(shm_vaddr, sizeof(struct fw_enc_hdr));
	if (stat)
		ret |= stat;

	return ret;
}

int bl33_decrypt(void)
{
	uint8_t *ptr = NULL;
	struct fw_enc_hdr header;
	uintptr_t shm_vaddr;
	int ret = 0;
	int stat;
	uint8_t key[FIP_KEY_SIZE] = { 0 };
	uint8_t salt[SALT_SIZE] = FIP_KEY_SALT;

	memset(&header, 0, sizeof(struct fw_enc_hdr));

	ret = get_fw_enc_hdr(&header);
	if (ret) {
		ERROR("Failed to get encryption header: %d\n", ret);
		return ret;
	}

	ret = set_shared_memory((uintptr_t)BL33_BASE,
				header.image_len + sizeof(struct fw_enc_hdr),
				&shm_vaddr, MT_MEMORY | MT_RW | MT_NS);
	if (ret)
		return ret;

	ptr = (uint8_t *)shm_vaddr;

	bl31_mbedtls_init();

	ret = derive_from_roe_key(salt, SALT_SIZE, key, FIP_KEY_SIZE);
	if (ret) {
		ERROR("Failed to derive key: %d\n", ret);
		goto deinit_out;
	}

	ret = aes_gcm_decrypt(ptr + sizeof(struct fw_enc_hdr), header.image_len,
			      key, FIP_KEY_SIZE, header.iv, header.iv_len, header.tag,
			      header.tag_len, NULL, 0, ptr);
	if (ret)
		ERROR("BL33 decryption failed: %d\n", ret);

deinit_out:
	clean_dcache_range((uintptr_t)shm_vaddr, header.image_len + sizeof(struct fw_enc_hdr));
	bl31_mbedtls_deinit();
	stat = free_shared_memory(shm_vaddr, header.image_len + sizeof(struct fw_enc_hdr));
	if (stat)
		ret |= stat;
	explicit_zeroize(key, FIP_KEY_SIZE);
	return ret;
}
