/*
 *  Mediatek MT7623 SoC PCIE support
 *
 *  Copyright (C) 2015 Mediatek
 *  Copyright (C) 2015 John Crispin <blogic@openwrt.org>
 *  Copyright (C) 2015 Ziv Huang <ziv.huang@mediatek.com>
 *  Copyright (C) 2019 Oleksandr Rybalko <ray@ddteam.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <common.h>
#include <malloc.h>

#include <pci.h>
#include <asm/io.h>

#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <dm/lists.h>

#define iowrite32(v, a)	((*((volatile u32 *)(a))) = v)
#define iowrite16(v, a)	((*((volatile u16 *)(a))) = v)
#define iowrite8(v, a)	((*((volatile u8 *)(a))) = v)
#define ioread32(a)	((*((volatile u32 *)(a))))
#define ioread16(a)	((*((volatile u16 *)(a))))
#define ioread8(a)	((*((volatile u8 *)(a))))

#define RT_HIFSYS_BASE		0x1a000000
#define RT_PCIE_BASE		0x1a140000
#define RT_PCIE_IOWIN_BASE	0x1a160000
#define RT_PCIE_IOWIN_SIZE	0x00010000
#define RT_PCIE_MEMWIN_BASE	0x60000000
#define RT_PCIE_MEMWIN_SIZE	0x10000000

#define RD(x)		(*((volatile u32 *)(RT_PCIE_BASE | (x))))
#define WR(x, v) 	(*((volatile u32 *)(RT_PCIE_BASE | (x))) = (v))

#define SYSCFG1			0x14
#define RSTCTL			0x34
#define RSTSTAT			0x38
#define PCICFG			0x00
#define PCIINT			0x08
#define PCIENA			0x0c
#define CFGADDR			0x20
#define CFGDATA			0x24
#define MEMBASE			0x28
#define IOBASE			0x2c

#define BAR0SETUP		0x10
#define IMBASEBAR0		0x18
#define PCIE_CLASS		0x34
#define PCIE_SISTAT		0x50

#define MTK_PCIE_HIGH_PERF	BIT(14)
#define PCIEP0_BASE		0x2000
#define PCIEP1_BASE		0x3000
#define PCIEP2_BASE		0x4000

#define PHY_P0_CTL		0x9000
#define PHY_P1_CTL		0xa000
#define PHY_P2_CTL		0x4000 /* in USB space */

#define RSTCTL_PCIE0_RST	BIT(24)
#define RSTCTL_PCIE1_RST	BIT(25)
#define RSTCTL_PCIE2_RST	BIT(26)
#define MAX_PORT_NUM		3

struct resource {
	char *name;
	u32 start;
	u32 end;
};

struct mt_pcie {
	char name[16];
};

static struct mtk_pcie_port {
	int id;
	int enable;
	u32 base;
	u32 phy_base;
	u32 perst_n;
	u32 reset;
	u32 interrupt_en;
	int irq;
	u32 link;
} mtk_pcie_port[] = {
	{ 0, 1, PCIEP0_BASE, PHY_P0_CTL, BIT(1), RSTCTL_PCIE0_RST, BIT(20) },
	{ 1, 1, PCIEP1_BASE, PHY_P1_CTL, BIT(2), RSTCTL_PCIE1_RST, BIT(21) },
	{ 2, 0, PCIEP2_BASE, PHY_P2_CTL, BIT(3), RSTCTL_PCIE2_RST, BIT(22) },
};

struct mtk_pcie {
	struct device *dev;
	void __iomem *sys_base;		/* HIF SYSCTL registers */
	void __iomem *pcie_base;	/* PCIE registers */
	void __iomem *usb_base;		/* USB registers */

	struct resource io;
	struct resource pio;
	struct resource mem;
	struct resource prefetch;
	struct resource busn;

	u32 io_bus_addr;
	u32 mem_bus_addr;

	struct clk *clk;
	int pcie_card_link;
};

static const struct mtk_phy_init {
	uint32_t reg;
	uint32_t mask;
	uint32_t val;
} mtk_phy_init[] = {
	{ 0xc00, 0x33000, 0x22000 },
	{ 0xb04, 0xe0000000, 0x40000000 },
	{ 0xb00, 0xe, 0x4 },
	{ 0xc3C, 0xffff0000, 0x3c0000 },
	{ 0xc48, 0xffff, 0x36 },
	{ 0xc0c, 0x30000000, 0x10000000 },
	{ 0xc08, 0x3800c0, 0xc0 },
	{ 0xc10, 0xf0000, 0x20000 },
	{ 0xc0c, 0xf000, 0x1000 },
	{ 0xc14, 0xf0000, 0xa0000 },
	{ 0xa28, 0x3fe00, 0x2000 },
	{ 0xa2c, 0x1ff, 0x10 },
};

/*
 * Globals.
 */

struct mtk_pcie pcie;
struct mtk_pcie *pcie0;

int mtk_pcie_probe(void);
static int mt_pcie_read_config(struct udevice *, pci_dev_t, uint, ulong *,
		enum pci_size_t);
static int mt_pcie_write_config(struct udevice *, pci_dev_t, uint, ulong ,
		enum pci_size_t);

#define mtk_foreach_port(p)		\
		for ((p) = mtk_pcie_port;	\
		(p) != &mtk_pcie_port[ARRAY_SIZE(mtk_pcie_port)]; (p)++)
#define BITS(m,n)   (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))

static void
mt7623_pcie_pinmux_set(void)
{
	u32 regValue;
	
	/* Pin208: PCIE0_PERST_N (3) */
	regValue = le32_to_cpu(*(volatile u_long *)(0x100059f0));
	regValue &= ~(BITS(9,11));
	regValue |= 3 << 9;
	*(volatile u_long *)(0x100059f0) = regValue;

	/* Pin208: PCIE1_PERST_N (3) */
	regValue = le32_to_cpu(*(volatile u_long *)(0x100059f0));
	regValue &= ~(BITS(12,14));
	regValue |= 3 << 12;
	*(volatile u_long *)(0x100059f0) = regValue;
}

static void
sys_w32(struct mtk_pcie *pcie, u32 val, unsigned reg)
{
	
	iowrite32(val, pcie->sys_base + reg);
}

static u32
sys_r32(struct mtk_pcie *pcie, unsigned reg)
{
	
	return ioread32(pcie->sys_base + reg);
}

static void
pcie_w32(struct mtk_pcie *pcie, u32 val, unsigned reg)
{

	iowrite32(val, pcie->pcie_base + reg);
}

static void
pcie_w16(struct mtk_pcie *pcie, u16 val, unsigned reg)
{

	iowrite16(val, pcie->pcie_base + reg);
}

static void
pcie_w8(struct mtk_pcie *pcie, u8 val, unsigned reg)
{

	iowrite8(val, pcie->pcie_base + reg);
}

static u32
pcie_r32(struct mtk_pcie *pcie, unsigned reg)
{

	return ioread32(pcie->pcie_base + reg);
}

static u32
pcie_r16(struct mtk_pcie *pcie, unsigned reg)
{

	return ioread16(pcie->pcie_base + reg);
}

static u32
pcie_r8(struct mtk_pcie *pcie, unsigned reg)
{

	return ioread8(pcie->pcie_base + reg);
}

static void
pcie_m32(struct mtk_pcie *pcie, u32 mask, u32 val, unsigned reg)
{
	u32 v;

	v = pcie_r32(pcie, reg);
	v &= mask;
	v |= val;
	pcie_w32(pcie, v, reg);
}

static void
mtk_pcie_configure_phy(struct mtk_pcie_port *port)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mtk_phy_init); i++) {
		void __iomem *phy_addr = (void __iomem *)port->phy_base +
		    mtk_phy_init[i].reg;
		u32 val = ioread32(phy_addr);

		val &= ~mtk_phy_init[i].mask;
		val |= mtk_phy_init[i].val;
		iowrite32(val, phy_addr);
		udelay(100);
	}
	udelay(16000);
}

static void
mtk_pcie_configure_rc(struct mtk_pcie *pcie, struct mtk_pcie_port *port)
{
	ulong val = 0;

	mt_pcie_write_config(NULL, (port->id)<<3, PCI_BASE_ADDRESS_0, 0x80000000,
			PCI_SIZE_32);
	mt_pcie_read_config(NULL, (port->id)<<3, PCI_BASE_ADDRESS_0, &val,
			PCI_SIZE_32);

	/* Configre RC Credit */
	val = 0;
	mt_pcie_read_config(NULL, (port->id)<<3, 0x73c, &val, PCI_SIZE_32);
	val &= ~(0x9fffUL)<<16;
	val |= 0x806c<<16;
	mt_pcie_write_config(NULL, (port->id)<<3, 0x73c, val, PCI_SIZE_32);
	mt_pcie_read_config(NULL, (port->id)<<3, 0x73c, &val, PCI_SIZE_32);

	/* Configre RC FTS number */
	mt_pcie_read_config(NULL, (port->id)<<3, 0x70c, &val, PCI_SIZE_32);
	val &= ~(0xff3) << 8;
	val |= 0x50 << 8;
	mt_pcie_write_config(NULL, (port->id)<<3, 0x70c, val, PCI_SIZE_32);
	mt_pcie_read_config(NULL, (port->id)<<3, 0x70c, &val, PCI_SIZE_32);
}

static void
mtk_pcie_preinit(struct mtk_pcie *pcie)
{
	struct mtk_pcie_port *port;
	u32 val = 0;
	int i;

	mt7623_pcie_pinmux_set();

	/* PCIe RC Reset */
	val = 0;
	mtk_foreach_port(port)
		if (port->enable)
			val |= port->reset;
	sys_w32(pcie, sys_r32(pcie, RSTCTL) | val, RSTCTL);
	udelay(12000);
	sys_w32(pcie, sys_r32(pcie, RSTCTL) & ~val, RSTCTL);
	udelay(12000);

	i = 100000;
	while (i--) {
		if (sys_r32(pcie, RSTSTAT) == 0)
			break;
		udelay(10);
	}

	/* Configure PCIe PHY */

	mtk_foreach_port(port)
		if (port->enable)
			mtk_pcie_configure_phy(port);

	/* PCIe EP reset */
	val = 0;
	mtk_foreach_port(port)
		if (port->enable)
			val |= port->perst_n;
	pcie_w32(pcie, pcie_r32(pcie, PCICFG) | val, PCICFG);
	udelay(12000);
	pcie_w32(pcie, pcie_r32(pcie, PCICFG) & ~val, PCICFG);
	udelay(200000);

	/* check the link status */
	val = 0;
	mtk_foreach_port(port) {
		if (port->enable) {
			if ((pcie_r32(pcie, port->base + PCIE_SISTAT) & 0x1))
				port->link = 1;
			else
				val |= port->reset;
		}
	}
	sys_w32(pcie, sys_r32(pcie, RSTCTL) | val, RSTCTL);
	udelay(200000);

	i = 100000;
	while (i--) {
		if (sys_r32(pcie, RSTSTAT) == 0)
			break;
		udelay(10);
	}

	mtk_foreach_port(port) {
		if (port->link) {
			pcie->pcie_card_link++;
		}
	}
	printf("%s: PCIe Link count=%d\n", __func__, pcie->pcie_card_link);
	if (!pcie->pcie_card_link)
		return;

	pcie_w32(pcie, pcie->mem_bus_addr, MEMBASE);
	pcie_w32(pcie, pcie->io_bus_addr, IOBASE);

	mtk_foreach_port(port) {
		if (port->link) {
			pcie_m32(pcie, 0xffffffff, port->interrupt_en, PCIENA);
			pcie_w32(pcie, 0x7fff0001, port->base + BAR0SETUP);
			pcie_w32(pcie, 0x80000000, port->base + IMBASEBAR0);
			pcie_w32(pcie, 0x06040001, port->base + PCIE_CLASS);
		}
	}
	udelay(100000);

	mtk_foreach_port(port)
		if (port->link)
			mtk_pcie_configure_rc(pcie, port);
}

static void
mtk_pcie_set_usb_pcie_co_phy(struct mtk_pcie *pcie)
{
#if 0
	if (mtk_pcie_port[2].enable == 1) {
		u32 val;
		/* USB-PCIe co-phy switch */
		val = sys_r32(pcie, SYSCFG1);

		printf("%s: PCIe/USB3 combo PHY mode SYSCFG1=%x\n",
			__func__, val);
		val &= ~(0x300000);
		sys_w32(pcie, val, SYSCFG1);
		printf("%s: PCIe/USB3 combo PHY mode SYSCFG1=%x\n",
			__func__, val);
	}
#endif
}

static void
mtk_pcie_fill_port(struct mtk_pcie *pcie)
{
	int i;

	for (i = 0; i < 2; i++)
		mtk_pcie_port[i].phy_base += (u32)pcie->pcie_base;

	mtk_pcie_port[2].phy_base += (u32)pcie->usb_base;
}

void
mt_pcie_init(void)
{
	/* Static instance of the controller. */
	static struct pci_controller	pcc;
	struct pci_controller		*hose = &pcc;

	memset(&pcc, 0, sizeof(pcc));

	/* PCI I/O space */
	pci_set_region(&hose->regions[0], RT_PCIE_IOWIN_BASE, RT_PCIE_IOWIN_BASE,
		       RT_PCIE_IOWIN_SIZE, PCI_REGION_IO);

	/* PCI memory space */
	pci_set_region(&hose->regions[1], RT_PCIE_MEMWIN_BASE, RT_PCIE_MEMWIN_BASE,
		       RT_PCIE_MEMWIN_SIZE, PCI_REGION_MEM);

	hose->region_count = 2;//3;
}


int
mtk_pcie_probe()
{
	pcie0 = &pcie;

	pcie0->io_bus_addr = RT_PCIE_IOWIN_BASE;
	pcie0->mem_bus_addr = RT_PCIE_MEMWIN_BASE;
	pcie0->sys_base = (u32 *)RT_HIFSYS_BASE;
	pcie0->pcie_base = (u32 *)RT_PCIE_BASE;

	mtk_pcie_set_usb_pcie_co_phy(pcie0);
	mtk_pcie_fill_port(pcie0);

	mtk_pcie_preinit(pcie0);

	return 0;
}

/* Probe function. */
void
pci_init_board(void)
{
	mtk_pcie_probe();
	mt_pcie_init();
}

static int
mt_pcie_read_config(struct udevice *bus, pci_dev_t bdf,
			       uint offset, ulong *valuep,
			       enum pci_size_t size)
{
	u32 address =  bdf | ((offset & 0xf00) << 16) | (offset & 0xfc);
	pcie_m32(pcie0, 0xf0000000, address, CFGADDR);

	switch (size) {
	case PCI_SIZE_8:
		*valuep = pcie_r8(pcie0, CFGDATA + (offset & 3));
		break;
	case PCI_SIZE_16:
		*valuep = pcie_r16(pcie0, CFGDATA + (offset & 2));
		break;
	case PCI_SIZE_32:
		*valuep = pcie_r32(pcie0, CFGDATA);
		break;
	}

	return 0;
}

static int
mt_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong value,
				enum pci_size_t size)
{
	u32 address =  bdf | ((offset & 0xf00) << 16) | (offset & 0xfc);
	pcie_m32(pcie0, 0xf0000000, address, CFGADDR);

	switch (size) {
	case PCI_SIZE_8:
		pcie_w8(pcie0, value, CFGDATA + (offset & 3));
		break;
	case PCI_SIZE_16:
		pcie_w16(pcie0, value, CFGDATA + (offset & 2));
		break;
	case PCI_SIZE_32:
	default:
		pcie_w32(pcie0, value, CFGDATA);
		break;
	}
	return 0;
}

static int
mt_pcie_probe(struct udevice *dev)
{
	mtk_pcie_probe();
	mt_pcie_init();
	return 0;
}


static const struct dm_pci_ops mt_pcie_ops = {
	.read_config	= mt_pcie_read_config,
	.write_config	= mt_pcie_write_config,
};

static const struct udevice_id mt_pcie_ids[] = {
	{ .compatible = "mediatek,mt7623-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_mt2701) = {
	.name = "pci_mt2701",
	.id = UCLASS_PCI,
	.of_match = mt_pcie_ids,
	.ops = &mt_pcie_ops,
	.probe	= mt_pcie_probe,
	.priv_auto_alloc_size = sizeof(struct mt_pcie),
};
