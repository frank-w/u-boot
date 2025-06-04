/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 SolidRun
 */

#ifndef __BOARD_SR_COMMON_H_
#define __BOARD_SR_COMMON_H_

struct tlv_data {
	u8 macbase[6];
	uint16_t maclen;
};

void read_tlv_data(struct tlv_data *td);

#endif /* __BOARD_SR_COMMON_H_ */
