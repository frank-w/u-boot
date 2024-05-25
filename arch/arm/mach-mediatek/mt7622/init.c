// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include <fdtdec.h>
#include <init.h>
#include <asm/armv8/mmu.h>
#include <asm/system.h>
#include <asm/global_data.h>
#include <asm/u-boot.h>
#include <linux/sizes.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int print_cpuinfo(void)
{
	printf("CPU:   MediaTek MT7622\n");
	return 0;
}

int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_mem_size_base();
	printf("dram_init ret: %d\n",ret);
	if (ret)
		return ret;

	printf("dram_init...ram-base: %08lx, ram-size: %08llx\n",gd->ram_base,gd->ram_size);
	gd->ram_size = get_ram_size((void *)gd->ram_base, SZ_1G);
	printf("dram_init gd->ram_size %08llx\n",gd->ram_size);

	return 0;
}

void reset_cpu(void)
{
	psci_system_reset();
}

#define UART_BASE						0x11002000
#define UART_BAUD						115200

// Real XTAL (MT7622/MT7986/MT7988)
#define UART_CLK						40000000

#define UART_RBR						(0x0)
#define UART_THR						(0x0)
#define UART_DLL						(0x0)
#define UART_DLH						(0x4)
#define UART_IER						(0x4)
#define UART_LCR						(0xc)
#define UART_LSR						(0x14)
#define UART_HIGHSPEED					(0x24)
#define UART_SAMPLE_COUNT				(0x28)
#define UART_SAMPLE_POINT				(0x2c)

#define UART_LSR_DR						(0x01)	/* Data ready */
#define UART_LSR_THRE					(0x20)	/* Xmit holding register empty */

#define UART_WRITE_REG(offset, val)		setbits_32(UART_BASE + (offset), val)
#define UART_READ_REG(offset)			readl(UART_BASE + (offset))

#define QUOT_VAL						((UART_CLK / (0xff * UART_BAUD)) + 1)
#define SAMPLE_COUNT_VAL				((UART_CLK / (UART_BAUD * QUOT_VAL)) - 1)
#define SAMPLE_POINT_VAL				((SAMPLE_COUNT_VAL - 1) / 2)


void board_debug_uart_init(void)
{
	int tmp;

	UART_WRITE_REG(UART_HIGHSPEED, 0x3);

	tmp = UART_READ_REG(UART_LCR);
	tmp |= (1<<7);
	UART_WRITE_REG(UART_LCR, tmp);

	UART_WRITE_REG(UART_THR, QUOT_VAL);
	UART_WRITE_REG(UART_IER, 0x0);
	UART_WRITE_REG(UART_SAMPLE_COUNT, SAMPLE_COUNT_VAL);
	UART_WRITE_REG(UART_SAMPLE_POINT, SAMPLE_POINT_VAL);

	tmp = UART_READ_REG(UART_LCR);
	tmp &= ~(1<<7);
	UART_WRITE_REG(UART_LCR, tmp);
}

static struct mm_region mt7622_mem_map[] = {
	{
		/* DDR */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE,
	}, {
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		0,
	}
};
struct mm_region *mem_map = mt7622_mem_map;
