#include <common.h>
#include <config.h>

//#include <asm/arch/typedefs.h>
//#include <asm/arch/timer.h>
//#include <asm/arch/wdt.h>

DECLARE_GLOBAL_DATA_PTR;

extern int rt2880_eth_initialize(bd_t *bis);


#if 0
/*
 *  Iverson 20140326 : DRAM have been initialized in preloader.
 */

int dram_init(void)
{
	/*
	 * UBoot support memory auto detection.
	 * So now support both static declaration and auto detection for DRAM size
	 */
#if CONFIG_CUSTOMIZE_DRAM_SIZE
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE - SZ_16M;
	printf("static declaration g_total_rank_size = 0x%8X\n", (int)gd->ram_size);
#else
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,0x40000000) - SZ_16M;
	printf("auto detection g_total_rank_size = 0x%8X\n", (int)gd->ram_size);
#endif

	return 0;
}
#endif

#define _getc()           *(volatile unsigned char *)(0x11002000)
#define _putc(c)           *(volatile unsigned char *)(0x11002000) = (c)

int board_init(void)
{

	/* Nelson: address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int board_late_init (void)
{
	gd->env_valid = 1; //to load environment variable from persistent store
	env_relocate();

	return 0;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_RT2880_ETH
	rt2880_eth_initialize(bis);
#endif
	return 0;
}

