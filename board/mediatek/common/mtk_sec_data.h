/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 Mediatek Inc. All Rights Reserved.
 */
#ifndef _MTK_SEC_DATA_H_
#define _MTK_SEC_DATA_H_

#define MTK_SECURE_DATA_K_ROE			"k-roe"
#define MTK_SECURE_DATA_K_ROOTFS		"k-rootfs"
#define MTK_SECURE_DATA_FIT_SECRET		"fit-secret"
#define MTK_SECURE_DATA_SIG_HASH		"signature-hash"
#define FIT_SFDT_PROP				"sfdt"
#define FIT_SECRETS_PATH			"/fit-secrets"
#define SFDT_SD_PATH				"/secure-data"
#define FDT_DEV_NODE				"key_dev_specific"
#define MAX_BOOTARGS_LEN			4096
#define MTK_SECURE_DATA_GET_SHM_CONFIG		0xC2000520
#define MTK_SECURE_DATA_PROC_DATA		0xC2000521
#define MTK_SECURE_DATA_GET_KEY			0xC2000522
#define MTK_SECURE_DATA_MAX_DATA_LEN		160
#define MTK_SECURE_DATA_KEY_LEN			32
#define MTK_SECURE_DATA_ROOTFS_KEY_ID		0
#define MTK_SECURE_DATA_DERIVED_KEY_ID		1

int mtk_secure_data_proc_data(const void *fit, const char *conf_name,
			      int conf_noffset);

int mtk_secure_data_set_fdt(void *fdt);

#endif /* _MTK_SEC_DATA_H_ */
