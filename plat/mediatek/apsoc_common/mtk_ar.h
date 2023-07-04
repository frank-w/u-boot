/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 */

#ifndef _MTK_AR_H_
#define _MTK_AR_H_

enum AR_VER_ID {
	BL_AR_VER_ID = 0,
	FW_AR_VER_ID,
};

int mtk_ar_get_bl_ar_ver(uint32_t *ar_ver);

int mtk_ar_get_ar_ver(uint32_t id, uint32_t *ar_ver);

int mtk_ar_update_bl_ar_ver(void);

int mtk_ar_update_ar_ver(uint32_t id, uint32_t ar_ver);

int mtk_ar_lock_ar_ver(void);

#endif /* _MTK_AR_H_ */
