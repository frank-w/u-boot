/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 */

#ifndef MTK_ROE_H
#define MTK_ROE_H

#include <stdint.h>

#define ROE_KEY_SIZE				16
#define FIP_KEY_SIZE				32
#define SALT_SIZE				32

void disable_roe_key(void);

int derive_roe_key(uint8_t *salt, size_t salt_len,
		   uint8_t *key, size_t *key_len);
#endif
