// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <common.h>
#include <mmc.h>
#include <configs/mt7623.h> //CONFIG_ENV_OFFSET

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int mmc_get_boot_dev(void)
{
	int g_mmc_devid = -1;
	char *uflag = (char *)0x81DFFFF0;
	if (strncmp(uflag,"eMMC",4)==0)
	{
		g_mmc_devid = 0;
		printf("Boot From Emmc(id:%d)\n\n", g_mmc_devid);
	}
	else
	{
		g_mmc_devid = 1;
		printf("Boot From SD(id:%d)\n\n", g_mmc_devid);
	}
	return g_mmc_devid;
}

int mmc_get_env_dev(void)
{
	printf("%s:%d %s\n",__FILE__,__LINE__,__FUNCTION__);
	mmc_get_boot_dev();
	return CONFIG_SYS_MMC_ENV_DEV; //have to replaced by g_mmc_devid if sdcard-offset is implemented
}
/*
int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
	s64 offset = CONFIG_ENV_OFFSET;
	#ifdef CONFIG_ENV_OFFSET_REDUND
	if (copy)
		offset = CONFIG_ENV_OFFSET_REDUND;
	#endif
	if (offset < 0)
		offset += mmc->capacity;
	//If mmc is SD, then set offset 1Mb
	if(mmc_get_boot_dev() == 1)
		*env_addr = offset + 0x100000;
	else
		*env_addr = offset;
	return 0;
}
*/
static int do_mmc_get_boot_dev(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("%s:%d %s\n",__FILE__,__LINE__,__FUNCTION__);
	int g_mmc_devid = mmc_get_boot_dev();
	//try to register a env-var, but seems not working (maybe overidden by default env)
	char intstr[2];
	sprintf(intstr, "%d", g_mmc_devid);
	env_set("bootdev", intstr);
	return g_mmc_devid;
}

U_BOOT_CMD(
	bootdev, 1, 0, do_mmc_get_boot_dev,
	"display Boot-Device",
	"- display the current Boot-MMC device"
);
