/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <linux/linkage.h>

.extern	preloader_param

ENTRY(save_boot_params)
	ldr	r6, =preloader_param
	str	r4, [r6]
	b	save_boot_params_ret
ENDPROC(save_boot_params)

ENTRY(lowlevel_init)
	/* enable SMP bit */
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #0x40
	mcr	p15, 0, r0, c1, c0, 1
	mov	pc, lr
ENDPROC(lowlevel_init)

ENTRY(jumparch64_smc)
	mov r3, r2
	mov r2, r1
	mov r1, r0
	ldr r0, =0x82000200  /*please check MTK_SIP_KERNEL_TMP_AARCH32 in  vendor/arm/atf/include/bl31/services/sip_svc.h*/ 
#	smc #0
	.word   0xe1600070  @ SMC #0 to enter monitor - hand assembled	
	b   .
ENDPROC(jumparch64_smc)
