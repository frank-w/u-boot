// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 MediaTek Incorporation. All Rights Reserved.
 *
 * Author: Tim-cy Yang <Tim-cy.Yang@mediatek.com>
 */

#include <common.h>
#include <image.h>
#include <linux/arm-smccc.h>
#include <linux/compiler.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/libfdt.h>
#include <hexdump.h>
#include <malloc.h>
#include "boot_helper.h"
#include "dm_parser.h"
#include "mtk_sec_data.h"
#include "mtk_sec_env.h"

static const char *sd_node_names[] = {
	MTK_SECURE_DATA_K_ROE,
	MTK_SECURE_DATA_FIT_SECRET,
	MTK_SECURE_DATA_K_ROOTFS,
	MTK_SECURE_DATA_SIG_HASH
};

static inline void memzero_explicit(void *s, size_t count)
{
	memset(s, 0, count);
	barrier_data(s);
}

/**
 * fit_get_sfdt_node_offset
 * @fit: pointer to the FIT format image header
 *
 * fit_get_sfdt_node_offset gets sfdt node offset in FIT.
 *
 * returns:
 *	>=0, sfdt offset
 *	<0, on failure
 */
static int fit_get_sfdt_node_offset(const void *fit, int conf_noffset)
{
	int img_noffset;
	int len;
	const char *sfdt_name;

	if (!fit || conf_noffset < 0)
		return -EINVAL;

	sfdt_name = fdt_getprop(fit, conf_noffset, FIT_SFDT_PROP, &len);
	if (!sfdt_name) {
		printf("Can't find property %s in config\n", FIT_SFDT_PROP);
		return -ENOENT;
	}

	printf("sFDT %s Found\n", sfdt_name);

	img_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (img_noffset < 0) {
		printf("Can't find images parent node %s\n", FIT_IMAGES_PATH);
		return img_noffset;
	}

	return fdt_subnode_offset(fit, img_noffset, sfdt_name);
}

/**
 * get_fit_secret_data_and_size
 * @fit: pointer to the FIT format image header
 * @conf_name: config name used for booting
 * @prop: property buffer
 * @size: size of @prop in bytes
 *
 * get_fit_secret_data_and_size gets fit-secret value
 * from fdt path fit-secrets/config-x, stores value,
 *  and size in @prop, @size.
 *
 * returns:
 *	0, on success
 *	<0, on failure
 */
static int get_fit_secret_data_and_size(const void *fit,
					const char *conf_name,
					const void **prop,
					int *size)
{
	int conf_noffset;
	int sec_noffset;

	if (!fit || !conf_name || !prop || !size)
		return -EINVAL;

	sec_noffset = fdt_path_offset(fit, FIT_SECRETS_PATH);
	if (sec_noffset < 0) {
		printf("Can't find fit-secrets parent node %s\n", FIT_SECRETS_PATH);
		return sec_noffset;
	}

	conf_noffset = fdt_subnode_offset(fit, sec_noffset, conf_name);
	if (conf_noffset < 0) {
		printf("Can't find %s node in fit-secret parent node\n", conf_name);
		return conf_noffset;
	}

	*prop = fdt_getprop(fit, conf_noffset, FIT_DATA_PROP, size);
	if (!(*prop)) {
		printf("Can't get property %s in sFDT/%s node\n", FIT_DATA_PROP,
		       conf_name);
		return -ENOENT;
	}

	return 0;
}

static int get_fit_secret_hash_algo(const void *fit, const char *conf_name,
				    const char **algo_name)
{
	int conf_noffset;
	int sec_noffset;
	int len;

	if (!fit || !conf_name || !algo_name)
		return -EINVAL;

	sec_noffset = fdt_path_offset(fit, FIT_SECRETS_PATH);
	if (sec_noffset < 0) {
		printf("Can't find fit-secrets parent node %s\n", FIT_SECRETS_PATH);
		return sec_noffset;
	}

	conf_noffset = fdt_subnode_offset(fit, sec_noffset, conf_name);
	if (conf_noffset < 0) {
		printf("Can't find %s node in fit-secret parent node\n", conf_name);
		return conf_noffset;
	}

	*algo_name = fdt_getprop(fit, conf_noffset, FIT_ALGO_PROP, &len);
	if (!(*algo_name)) {
		printf("Can't get property %s in sFDT/%s node\n", FIT_ALGO_PROP,
		       conf_name);
		return -ENOENT;
	}

	return 0;
}

/**
 * get_sig_hash_data_and_size
 * @fit: pointer to the FIT format image header
 * @conf_noffset: config node's offset used to boot in FIT
 * @conf_name: config name used for booting
 * @hash: signature's hash buffer
 * @size: size of @hash in bytes
 *
 * get_sig_hash_data_and_size extracts signature according to
 * current using config, algorithm, and calculates hash of signature, stores
 * hash value and size in @hash, @size.
 *
 * returns:
 *	0, on success
 *	<0, on failure
 */
static int get_sig_hash_data_and_size(const void *fit, int conf_noffset,
				      const char *conf_name, uint8_t *hash,
				      int *size)
{
	int len;
	int ret;
	int sig_noffset;
	const void *val;
	char *algo_name;

	if (!fit || conf_noffset < 0 || !conf_name || !hash || !size)
		return -EINVAL;

	sig_noffset = fdt_subnode_offset(fit, conf_noffset, FIT_SIG_NODENAME);
	if (sig_noffset < 0) {
		printf("Can't find %s node in config\n", FIT_SIG_NODENAME);
		return sig_noffset;
	}

	val = fdt_getprop(fit, sig_noffset, FIT_VALUE_PROP, &len);
	if (!val) {
		printf("Can't find property %s in %s\n", FIT_VALUE_PROP,
							 FIT_SIG_NODENAME);
		return -ENOENT;
	}

	ret = get_fit_secret_hash_algo(fit, conf_name, (const char **)&algo_name);
	if (ret) {
		printf("Can't get FIT-secret hash algorithm: %d\n", ret);
		return ret;
	}

	ret = calculate_hash(val, len, algo_name, hash, size);
	if (ret)
		printf("Can't calculate hash of signature\n");

	return ret;
}

static int get_k_roe_data_and_size(uint8_t *data, int *len)
{
	int ret;
	void *sec_env;

	if (!data || !len)
		return -EINVAL;

	ret = board_sec_env_load();
	if (ret) {
		puts("Can't load secure env\n");
		return ret;
	}

	ret = get_sec_env(MTK_SECURE_DATA_K_ROE, &sec_env);
	if (ret) {
		printf("Failed to get secure env: %s\n", MTK_SECURE_DATA_K_ROE);
		return ret;
	}

	if (!sec_env || strlen(sec_env) != MTK_SECURE_DATA_KEY_LEN * 2)
		return -EINVAL;

	ret = hex2bin(data, sec_env, MTK_SECURE_DATA_KEY_LEN);
	if (ret) {
		printf("Failed to convert hex to bin\n");
		return ret;
	}

	*len = MTK_SECURE_DATA_KEY_LEN;

	return 0;
}

/**
 * setup_secure_data_shm
 * @fit: pointer to the FIT format image header
 * @sfdt: pointer to the secure-data fdt
 * @conf_noffset: config node's offset used to boot in FIT
 * @conf_name: config node's name used to boot in FIT
 * @shm_paddr: shared memory physical address to store data
 *
 * setup_secure_data_shm extracts secure data from FIT, and
 * copys to shared memory region within ATF
 *
 * Buffer is following format:
 * Low address	--------------------------------
 * 		| k-roe			       |
 * 		--------------------------------
 * 		| fit-secret		       |
 * 		--------------------------------
 * 		| k-rootfs		       |
 * 		--------------------------------
 * 		| fit-signature's hash	       |
 * High address	--------------------------------
 * returns:
 *	0, on success
 *	<0, on failure
 */
static int setup_secure_data_shm(const void *fit, void *sfdt,
				 int conf_noffset, const char *conf_name,
				 uint8_t *shm_paddr)
{
	int ret;
	int i = 0;
	int noffset;
	int sd_noffset;
	int len;
	uint8_t data[MTK_SECURE_DATA_MAX_DATA_LEN];

	if (!fit || !sfdt || conf_noffset < 0 || !conf_name || !shm_paddr)
		return -EINVAL;

	sd_noffset = fdt_path_offset(sfdt, SFDT_SD_PATH);
	if (sd_noffset < 0) {
		printf("Can't find %s node in sFDT\n", SFDT_SD_PATH);
		return sd_noffset;
	}

	for (i = 0; i < ARRAY_SIZE(sd_node_names); i++) {
		const void *prop;

		if (!strncmp(sd_node_names[i], MTK_SECURE_DATA_FIT_SECRET,
			     strlen(MTK_SECURE_DATA_FIT_SECRET))) {
			ret = get_fit_secret_data_and_size(fit, conf_name,
							   &prop, &len);
			if (ret)
				return ret;
		} else if (!strncmp(sd_node_names[i], MTK_SECURE_DATA_SIG_HASH,
				  strlen(MTK_SECURE_DATA_SIG_HASH))) {
			ret = get_sig_hash_data_and_size(fit, conf_noffset,
							 conf_name, data, &len);
			if (ret)
				return ret;

			prop = data;
		}  else if (!strncmp(sd_node_names[i], MTK_SECURE_DATA_K_ROE,
				     strlen(MTK_SECURE_DATA_K_ROE))) {
			ret = get_k_roe_data_and_size(data, &len);
			if (ret)
				return ret;

			prop = data;
		} else {
			noffset = fdt_subnode_offset(sfdt, sd_noffset,
						     sd_node_names[i]);
			if (noffset < 0) {
				printf("Can't find %s node in sFDT\n",
				      sd_node_names[i]);
				return noffset;
			}

			prop = fdt_getprop(sfdt, noffset, FIT_DATA_PROP, &len);
			if (!prop) {
				printf("Can't get property %s in %s node\n",
				      FIT_DATA_PROP,
				      sd_node_names[i]);
				return -ENOENT;
			}
		}

		if (len <= 0 || len > MTK_SECURE_DATA_MAX_DATA_LEN)
			return -EINVAL;

		memcpy(shm_paddr + i * MTK_SECURE_DATA_MAX_DATA_LEN, prop, len);
	}

	return 0;
}

static int fit_verify_sfdt(const void *fit, int noffset)
{
	if (!fit || noffset < 0)
		return -EINVAL;

	fit_image_print(fit, noffset, "   ");

	puts("   Verifying Hash Integrity ... ");
	if (!fit_image_verify(fit, noffset)) {
		puts("Bad Data Hash\n");
		return -EACCES;
	}
	puts("OK\n");

	return 0;
}

static int get_shm_config(uintptr_t *shm_paddr, size_t *size)
{
	struct arm_smccc_res res;

	if (!shm_paddr || !size)
		return -EINVAL;

	*shm_paddr = 0;
	*size = 0;

	arm_smccc_smc(MTK_SECURE_DATA_GET_SHM_CONFIG, 0, 0, 0, 0, 0, 0, 0, &res);

	if (res.a0 || !res.a1 || !res.a2)
		return -ENOMEM;

	*shm_paddr = res.a1;
	*size = res.a2;

	return 0;
}

/**
 * mtk_secure_data_proc_data
 * @fit: pointer to the FIT format image header
 * @conf_name: config node's name used to boot in FIT
 * @conf_noffset: config node's offset used to boot in FIT
 *
 * mtk_secure_data_proc_data extracts secure-data from FIT,
 * setups data in shared buffer, and invokes SMC call to ask
 * ATF to process data.
 *
 * returns:
 *	0, on success
 *	<0, on failure
 */
int mtk_secure_data_proc_data(const void *fit, const char *conf_name,
			      int conf_noffset)
{
	void *sfdt;
	size_t size;
	int sfdt_noffset;
	int ret;
	struct arm_smccc_res res;
	uintptr_t shm_paddr;
	size_t shm_size;

	puts("mtk-secure-data: Finding sFDT in FIT ... ");

	if (!fit || !conf_name || conf_noffset < 0)
		return -EINVAL;

	sfdt_noffset = fit_get_sfdt_node_offset(fit, conf_noffset);
	if (sfdt_noffset < 0) {
		printf("Can't find sFDT node in FIT: %d\n", sfdt_noffset);
		return sfdt_noffset;
	}

	ret = fit_verify_sfdt(fit, sfdt_noffset);
	if (ret)
		return ret;

	ret = fit_image_get_data_and_size(fit, sfdt_noffset,
					  (const void **)&sfdt, &size);
	if (ret) {
		printf("Can't get sFDT data and size: %d\n", ret);
		return ret;
	}

	ret = get_shm_config(&shm_paddr, &shm_size);
	if (ret) {
		puts("Can't get shm config\n");
		return -ENOMEM;
	}
	if (ARRAY_SIZE(sd_node_names) * MTK_SECURE_DATA_MAX_DATA_LEN > shm_size)
		return -ENOMEM;

	ret = setup_secure_data_shm(fit, sfdt, conf_noffset,
				    conf_name, (uint8_t *)shm_paddr);
	if (ret) {
		puts("Can't setup secure data nodes\n");
		goto proc_data_err;
	}

	puts("mtk-secure-data: Processing Secure Data ... ");
	arm_smccc_smc(MTK_SECURE_DATA_PROC_DATA, 0, 0, 0, 0, 0, 0, 0, &res);
	ret = res.a0;

proc_data_err:
	memzero_explicit((void *)shm_paddr, ARRAY_SIZE(sd_node_names) *
	       MTK_SECURE_DATA_MAX_DATA_LEN);

	if (ret)
		printf("Failed: %d\n", ret);
	else
		puts("OK\n");

	return ret;
}

static int mtk_secure_data_get_key(uint32_t key_id, struct arm_smccc_res *res)
{
	if (!res)
		return -EINVAL;

	arm_smccc_smc(MTK_SECURE_DATA_GET_KEY, key_id, 0, 0, 0, 0, 0, 0, res);
	if (res->a0 && !res->a1 && !res->a2 && !res->a3)
		return res->a0;

	return 0;
}

static bool compare_key(const char *key, size_t keylen, const char *str)
{
	size_t ckeylen;

	ckeylen = strlen(str);
	if (ckeylen == keylen) {
		if (!strncmp(str, key, keylen))
			return true;
	}

	return false;
}

/**
 * construct_dm_mod_create_args
 * @dm_mod: string of dm-mod.create arguments
 * @b: current pointer in new bootargs buffer
 *
 * construct_dm_mod_create_args parses dm-mod.create, finds
 * dm-crypt key argument position, and calls SMC to get key
 * from ATF, inserts key value into new bootargs.
 *
 * returns:
 *	0, on success
 * 	<0, on failure
 */
static int construct_dm_mod_create_args(char *dm_mod, char *b)
{
	int ret;
	struct dm_crypt_info dci = {0};
	struct arm_smccc_res res;

	if (!dm_mod || !b)
		return -EINVAL;

	ret = dm_mod_create_arg_parse(dm_mod, "crypt", (struct dm_info *)&dci);
	if (ret) {
		puts("Can't parse dm-crypt args\n");
		goto construct_dm_mod_create_err;
	}

	if (!dci.key) {
		ret = -EINVAL;
		goto construct_dm_mod_create_err;
	}

	memcpy(b, dm_mod, dci.key_pos);
	b += dci.key_pos;

	ret = mtk_secure_data_get_key(MTK_SECURE_DATA_ROOTFS_KEY_ID, &res);
	if (ret) {
		printf("ATF SMC call failed: %d\n", ret);
		goto construct_dm_mod_create_err;
	}

	b = bin2hex(b, (uint8_t *)&res, sizeof(struct arm_smccc_res));

	memcpy(b, dm_mod + dci.key_pos + dci.key_len,
	       strlen(dm_mod) - dci.key_pos - dci.key_len);
	b += strlen(dm_mod) - dci.key_pos - dci.key_len;

	memcpy(b, "\" ", 2);
	b += 2;
	*b = '\0';

construct_dm_mod_create_err:
	memzero_explicit(&res, sizeof(struct arm_smccc_res));
	if (dci.tmpbuf)
		free(dci.tmpbuf);

	return ret;
}

static int create_new_bootargs(const char *orig_bootargs, char *new_bootargs)
{
	int ret;
	size_t keylen;
	size_t plen;
	char *dm_mod = NULL;
	const char *n;
	const char *p;
	char *b;

	if (!orig_bootargs || !strlen(orig_bootargs) || !new_bootargs)
		return -EINVAL;

	n = orig_bootargs;
	b = new_bootargs;

	/* find dm-mod.create entry in original bootargs */
	while (1) {
		n = get_arg_next(n, &p, &keylen);
		if (!n)
			break;

		plen = n - p;

		if (compare_key(p, keylen, "dm-mod.create")) {
			if (dm_mod) {
				free(dm_mod);
				return -EINVAL;
			}
			/* assume "" always exists */
			if (keylen + 3 >= plen)
				return -ENOMEM;

			dm_mod = malloc(plen - keylen - 2);
			if (!dm_mod)
				return -ENOMEM;

			memcpy(dm_mod, p + keylen + 2, plen - keylen - 3);
			dm_mod[plen - keylen - 3] = '\0';
		} else {
			memcpy(b, p, plen);
			b += plen;
			*b++ = ' ';
		}
	}

	if (!dm_mod)
		return -EINVAL;

	p = "dm-mod.create=\"";
	plen = strlen(p);
	memcpy(b, p, plen);
	b += plen;

	ret = construct_dm_mod_create_args(dm_mod, b);
	if (ret)
		puts("Can't parse dm-mod args\n");

	free(dm_mod);

	return ret;
}

static int set_bootargs(void *fdt)
{
	int ret;
	int orig_len;
	int len;
	int chosen_noffset;
	const char *orig_bootargs;
	char *bootargs;

	if (!fdt)
		return -EINVAL;

	chosen_noffset = fdt_find_or_add_subnode(fdt, 0, "chosen");
	if (chosen_noffset < 0) {
		puts("Can't find chosen node in FDT\n");
		return -ENOENT;
	}

	orig_bootargs = fdt_getprop(fdt, chosen_noffset, "bootargs", &orig_len);
	if (!orig_bootargs) {
		puts("Can't find property bootargs in chosen node\n");
		return -ENOENT;
	}

	if (orig_len <= 0 ||
	    INT_MAX - orig_len < MTK_SECURE_DATA_KEY_LEN * 2 + 1)
		return -EINVAL;

	len = orig_len + MTK_SECURE_DATA_KEY_LEN * 2 + 1;
	if (len > MAX_BOOTARGS_LEN)
		return -EINVAL;

	bootargs = malloc(len);
	if (!bootargs)
		return -ENOMEM;

	ret = create_new_bootargs(orig_bootargs, bootargs);
	if (ret) {
		puts("Can't create new bootargs\n");
		goto set_bootargs_err;
	}

	ret = fdt_setprop(fdt, chosen_noffset,
			  "bootargs", bootargs, strlen(bootargs));
	if (ret)
		printf("Can't set property bootargs in chosen node: %d\n", ret);

set_bootargs_err:
	memzero_explicit(bootargs, len);
	free(bootargs);

	return ret;
}

static int set_dev_specific_key(void *fdt)
{
	int ret;
	int dev_noffset;
	struct arm_smccc_res res;

	if (!fdt)
		return -EINVAL;

	dev_noffset = fdt_find_or_add_subnode(fdt, 0, FDT_DEV_NODE);
	if (dev_noffset < 0) {
		printf("Can't find or add %s node\n", FDT_DEV_NODE);
		return dev_noffset;
	}

	ret = mtk_secure_data_get_key(MTK_SECURE_DATA_DERIVED_KEY_ID, &res);
	if (ret) {
		printf("ATF SMC call failed: %d\n", ret);
		goto set_dev_specific_key_err;
	}

	ret = fdt_setprop(fdt, dev_noffset, FDT_DEV_NODE, (uint8_t *)&res,
			  sizeof(struct arm_smccc_res));
	if (ret)
		printf("Can't set property in %s node\n", FDT_DEV_NODE);

set_dev_specific_key_err:
	memzero_explicit(&res, sizeof(struct arm_smccc_res));

	return ret;
}

int mtk_secure_data_set_fdt(void *fdt)
{
	int ret;

	if (!fdt)
		return -EINVAL;

	ret = set_bootargs(fdt);
	if (ret) {
		puts("Can't set bootargs in FDT\n");
		return ret;
	}

	ret = set_dev_specific_key(fdt);
	if (ret)
		puts("Can't set dev-specific key\n");

	return ret;
}
