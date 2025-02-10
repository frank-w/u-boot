/*
 * Copyright (c) 2019, Linaro Limited. All rights reserved.
 * Author: Sumit Garg <sumit.garg@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <mtk_roe.h>

#define SALT_SIZE		32

/* Supported key algorithms */
enum {
	KEY_ALG_GCM		/* AES-GCM (default) */
};

int encrypt_file(unsigned short fw_enc_status, int enc_alg, char *key_string,
		 char *nonce_string, const char *ip_name, const char *op_name);
int do_hkdf(char *key, uint32_t key_len, char *salt, uint32_t salt_len,
	    char *out, size_t out_len);
#endif /* ENCRYPT_H */
