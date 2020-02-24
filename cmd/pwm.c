// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 MediaTek Inc.
 * 
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <pwm.h>
#include <dm/uclass-internal.h>

int do_pwm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	unsigned long pwm_id, period, duty;

	if (uclass_get_device(UCLASS_PWM, 0, &dev)) {
		printf("unable to find pwm driver\n");
		return CMD_RET_FAILURE;
	}
	if (argc < 2)
		return CMD_RET_USAGE;
	pwm_id = simple_strtoul(argv[1], NULL, 0);

	if (strncmp(argv[2], "config", 10) == 0) {
		if (argc < 4)
			return CMD_RET_USAGE;
		period = simple_strtoul(argv[3], NULL, 0);
		duty = simple_strtoul(argv[4], NULL, 0);
		if (pwm_set_config(dev, pwm_id, period, duty)) {
			printf("unable to config pwm driver\n");
			return CMD_RET_FAILURE;
		}
	}
	else if (strncmp(argv[2], "enable", 10) == 0) {
		pwm_set_enable(dev, pwm_id, true);
	}
	else if (strncmp(argv[2], "disable", 10) == 0) {
		pwm_set_enable(dev, pwm_id, false);
	}

	return 0;
}

U_BOOT_CMD(
	pwm, 5, 1, do_pwm,
	"manage PWMs\n",
	"<pwm_id> [config|enable|disable]\n"
	"config: <pwm_id> config <period_in_ns> <duty_in_ns>\n"
	"enable: <pwm_id> enable\n"
);
