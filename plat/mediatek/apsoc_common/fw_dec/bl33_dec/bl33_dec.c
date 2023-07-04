// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 */

#include <common/debug.h>
#include <plat/common/platform.h>
#include <tools_share/firmware_encrypted.h>

#include <mbedtls_helper.h>
#include <shm.h>
#include <mtk_roe.h>
#include <salt.h>

static int get_fw_enc_hdr(struct fw_enc_hdr *header)
{
	uint8_t *ptr = NULL;
	uintptr_t shm_vaddr = 0;
	int ret = 0;

	ret = set_shared_memory((uintptr_t)BL33_BASE, sizeof(struct fw_enc_hdr), &shm_vaddr,
				MT_MEMORY | MT_RO | MT_NS);
	if (ret) {
		ERROR("get_fw_enc_hdr: set_shared_memory failed\n");
		return ret;
	}

	ptr = (uint8_t *)shm_vaddr;
	memcpy(header, ptr, sizeof(struct fw_enc_hdr));

	ret = free_shared_memory(shm_vaddr, sizeof(struct fw_enc_hdr));
	if (ret)
		ERROR("get_fw_enc_hdr: free_shared_memory failed\n");
	return ret;
}

int bl33_decrypt(void)
{
	uint8_t *ptr = NULL;
	struct fw_enc_hdr header;
	uintptr_t shm_vaddr;
	int ret = 0;
	uint8_t key[FIP_KEY_SIZE] = { 0 };
	uint8_t salt[SALT_SIZE] = MTK_FIP_ENC_SALT;
	size_t key_len;

	memset(&header, 0, sizeof(struct fw_enc_hdr));

	ret = get_fw_enc_hdr(&header);
	if (ret) {
		ERROR("bl33_decrypt: get_fw_enc_hdr: %d failed\n", ret);
		goto disable_out;
	}

	ret = set_shared_memory((uintptr_t)BL33_BASE, header.image_len,
				&shm_vaddr, MT_MEMORY | MT_RW | MT_NS);
	if(ret) {
		ERROR("bl33_decrypt: set_shared_memory:%d failed\n", ret);
		goto disable_out;
	}

	ptr = (uint8_t *) shm_vaddr;

	bl31_mbedtls_init();

	ret = derive_roe_key(salt, SALT_SIZE, key, &key_len);
	if (ret) {
		ERROR("bl33_decrypt: derive_roe_key:%d failed\n", ret);
		goto out;
	}

	ret = aes_gcm_decrypt(ptr + sizeof(struct fw_enc_hdr), header.image_len,
			      key, key_len, header.iv, header.iv_len, header.tag,
			      header.tag_len, NULL, 0, ptr);
	if (ret != 0) {
		ERROR("File decryption failed (%i)\n", ret);
		goto out;
	}

out:
	clean_dcache_range((uintptr_t) shm_vaddr, header.image_len);
	bl31_mbedtls_deinit();
	ret = free_shared_memory(shm_vaddr, header.image_len);
	if (ret)
		ERROR("get_fw_enc_hdr: free_shared_memory failed\n");
disable_out:
	disable_roe_key();
	memset(key, 0, FIP_KEY_SIZE);
	return ret;
}
