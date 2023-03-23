// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 * (C) Copyright 2023 Collabora Ltd
 */

#include <common.h>
#include <adc.h>

#define KEY_DOWN_MIN_VAL        0
#define KEY_DOWN_MAX_VAL        30

int rockchip_dnl_key_pressed(void)
{
	unsigned int val;

	if (adc_channel_single_shot("saradc@fe720000", 0, &val)) {
		printf("%s read adc key val failed\n", __func__);
		return false;
	}

	if (val >= KEY_DOWN_MIN_VAL && val <= KEY_DOWN_MAX_VAL)
		return true;
	else
		return false;
}
