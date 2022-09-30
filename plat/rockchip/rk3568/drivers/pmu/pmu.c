/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <bakery_lock.h>
#include <cortex_a55.h>
#include <mmio.h>
#include <platform.h>
#include <pmu.h>
#include <cpus_on_fixed_addr.h>
#include <plat_private.h>
#include <platform_def.h>
#include <soc.h>

/*
 * Use this macro to instantiate lock before it is used in below
 * rockchip_pd_lock_xxx() macros
 */
DECLARE_BAKERY_LOCK(rockchip_pd_lock);

static uint32_t grf_ddr_con3;
static struct psram_data_t *psram_sleep_cfg =
	(struct psram_data_t *)&sys_sleep_flag_sram;

/*
 * These are wrapper macros to the powe domain Bakery Lock API.
 */
#define rockchip_pd_lock_init() bakery_lock_init(&rockchip_pd_lock)
#define rockchip_pd_lock_get() bakery_lock_get(&rockchip_pd_lock)
#define rockchip_pd_lock_rls() bakery_lock_release(&rockchip_pd_lock)

static void pmu_pmic_sleep_mode_config(void)
{
	/* pmic sleep function selection
	 * 1'b0: From reset pulse generator, can reset external PMIC
	 * 1'b1: From pmu block, only support sleep function for external PMIC
	 */
	mmio_write_32(PMUGRF_BASE + PMU_GRF_SOC_CON(0),  WRITE_MASK_SET(BIT(7)));
	mmio_write_32(PMUGRF_BASE + PMU_GRF_GPIO0A_IOMUX_L, PMIC_SLEEP_FUN);
}

static void pmu_wakeup_source_config(void)
{
	/* config wakeup source */
	mmio_write_32(PMU_BASE + PMU_WAKEUP_INT_CON, WRITE_MASK_SET(BIT(WAKEUP_GPIO0_INT_EN)));

	INFO("WAKEUP: PMU_WAKEUP_INT_CON:0x%x, reg: 0x%x\n",
	     mmio_read_32(PMU_BASE + PMU_WAKEUP_INT_CON), PMU_WAKEUP_INT_CON);
}

static void pmu_pll_powerdown_config(void)
{
	uint32_t pll_id;

	/* PLL power down by PMU */
	pll_id = BIT(APLL_PD_ENA) |
		BIT(CPLL_PD_ENA) |
		BIT(GPLL_PD_ENA) |
		BIT(MPLL_PD_ENA) |
		BIT(NPLL_PD_ENA) |
		BIT(HPLL_PD_ENA) |
		BIT(PPLL_PD_ENA) |
		BIT(VPLL_PD_ENA);
	mmio_write_32(PMU_BASE + PMU_PLLPD_CON, WRITE_MASK_SET(pll_id));
	INFO("PLL: PMU_PLLPD_CON(0x%x):0x%x\n",
	     PMU_PLLPD_CON, mmio_read_32(PMU_BASE + PMU_PLLPD_CON));
}

static void pmu_stable_count_config(void)
{
	mmio_write_32(PMU_BASE + PMU_DSU_STABLE_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_PMIC_STABLE_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_OSC_STABLE_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_WAKEUP_RSTCLR_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_PLL_LOCK_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_DSU_PWRUP_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_DSU_PWRDN_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_GPU_VOLUP_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_GPU_VOLDN_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_WAKEUP_TIMEOUT_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_PWM_SWITCH_CNT, 0x180);
	mmio_write_32(PMU_BASE + PMU_DBG_RST_CNT, 0x180);
}

static void pmu_pd_powerdown_config(void)
{
	uint32_t pwr_gate_con, pwr_dwn_st, pmu_bus_idle_con0 = 0;
	uint32_t pmu_bus_idle_con1;

	/* Pd power down by PMU */
	pwr_dwn_st = mmio_read_32(PMU_BASE + PMU_PWR_DWN_ST);
	pwr_gate_con = ~pwr_dwn_st & 0x3ff;

	if (pwr_gate_con & BIT(PD_GPU_DWN_ENA))
		pmu_bus_idle_con0 |= BIT(IDLE_REQ_GPU);

	if (pwr_gate_con & BIT(PD_NPU_DWN_ENA))
		pmu_bus_idle_con0 |= BIT(IDLE_REQ_NPU);
	if (pwr_gate_con & BIT(PD_RKVENC_DWN_ENA))
		pmu_bus_idle_con0 |= BIT(IDLE_REQ_RKVENC);
	if (pwr_gate_con & BIT(PD_RKVDEC_DWN_ENA))
		pmu_bus_idle_con0 |= BIT(IDLE_REQ_RKVDEC);
	if (pwr_gate_con & BIT(PD_RGA_DWN_ENA))
		pmu_bus_idle_con0 |= BIT(IDLE_REQ_RGA);
	if (pwr_gate_con & BIT(PD_VI_DWN_ENA))
		pmu_bus_idle_con0 |= BIT(IDLE_REQ_VI);
	if (pwr_gate_con & BIT(PD_VO_DWN_ENA))
		pmu_bus_idle_con0 |= BIT(IDLE_REQ_VO);

	if (pwr_gate_con & BIT(PD_PIPE_DWN_ENA))
		pmu_bus_idle_con0 |= BIT(IDLE_REQ_PIPE);

	pmu_bus_idle_con0 |= BIT(IDLE_REQ_GIC_AUDIO) |
		BIT(IDLE_REQ_MSCH) |
		BIT(IDLE_REQ_PHP) |
		BIT(IDLE_REQ_SECURE_FLASH) |
		BIT(IDLE_REQ_PERIMID) |
		BIT(IDLE_REQ_USB) |
		BIT(IDLE_REQ_BUS);

	/* Enable power down PD by PMU automatically */
	pwr_gate_con |= (BIT(PD_GPU_DWN_ENA) |
		BIT(PD_NPU_DWN_ENA) |
		BIT(PD_VPU_DWN_ENA) |
		BIT(PD_RKVENC_DWN_ENA) |
		BIT(PD_RKVDEC_DWN_ENA) |
		BIT(PD_RGA_DWN_ENA) |
		BIT(PD_VI_DWN_ENA) |
		BIT(PD_VO_DWN_ENA) |
		BIT(PD_PIPE_DWN_ENA)) << 16;

	pmu_bus_idle_con1 = 0;

	mmio_write_32(PMU_BASE + PMU_PWR_GATE_CON, pwr_gate_con);
	mmio_write_32(PMU_BASE + PMU_BUS_IDLE_CON0, WRITE_MASK_SET(pmu_bus_idle_con0));
	mmio_write_32(PMU_BASE + PMU_BUS_IDLE_CON1, WRITE_MASK_SET(pmu_bus_idle_con1));

	/* When perform idle operation,
	 * corresponding clock can be opened or gated automatically
	 */
	mmio_write_32(PMU_BASE + PMU_NOC_AUTO_CON0, 0xffffffff);
	mmio_write_32(PMU_BASE + PMU_NOC_AUTO_CON1, 0x00070007);

	mmio_write_32(PMU_BASE + PMU_VOL_GATE_SFTCON, WRITE_MASK_SET(BIT(VD_NPU_ENA)));

	mmio_write_32(PMU_BASE + PMU_PWR_CON, WRITE_MASK_CLR(BIT(PWRDN_BYPASS)));
	mmio_write_32(PMU_BASE + PMU_PWR_CON, WRITE_MASK_CLR(BIT(BUS_BYPASS)));

	INFO("PD & BUS:PMU_PWR_DWN_ST(0x%x):0x%x\n",
	     PMU_PWR_DWN_ST, mmio_read_32(PMU_BASE + PMU_PWR_DWN_ST));
	INFO("PD & BUS:PMU_PWR_GATE_CON(0x%x):0x%x\n",
	     PMU_PWR_GATE_CON, mmio_read_32(PMU_BASE + PMU_PWR_GATE_CON));
	INFO("PD & BUS:PMU_BUS_IDLE_CON0(0x%x):0x%x\n",
	     PMU_BUS_IDLE_CON0, mmio_read_32(PMU_BASE + PMU_BUS_IDLE_CON0));
	INFO("PD & BUS:PMU_BUS_IDLE_CON1(0x%x):0x%x\n",
	     PMU_BUS_IDLE_CON1, mmio_read_32(PMU_BASE + PMU_BUS_IDLE_CON1));
	INFO("PD & BUS:PMU_PWR_CON(0x%x):0x%x\n",
	     PMU_PWR_CON, mmio_read_32(PMU_BASE + PMU_PWR_CON));
}

static void pmu_ddr_suspend_config(void)
{
	uint32_t pmu_ddr_pwr_con;

	pmu_ddr_pwr_con = BIT(DDR_SREF_ENA) |
		BIT(DDRIO_RET_ENTER_ENA) |
		BIT(DDRIO_RET_EXIT_ENA) |
		BIT(DDRPHY_AUTO_GATING_ENA);

	mmio_write_32(PMU_BASE + PMU_DDR_PWR_CON, WRITE_MASK_SET(pmu_ddr_pwr_con));
	/* DPLL power down by PMU */
	mmio_write_32(PMU_BASE + PMU_PLLPD_CON, WRITE_MASK_SET(BIT(DPLL_PD_ENA)));
	mmio_write_32(PMU_BASE + PMU_PWR_CON, WRITE_MASK_CLR(BIT(DDR_BYPASS)));

	grf_ddr_con3 = mmio_read_32(DDRGRF_BASE + GRF_DDR_CON3);

	mmio_write_32(DDRGRF_BASE + GRF_DDR_CON3, 0x00600020);

	pmu_ddr_pwr_con = mmio_read_32(PMU_BASE + PMU_DDR_PWR_CON);

	INFO("DDR: PMU_PLLPD_CON(0x%x):0x%x\n",
	     PMU_PLLPD_CON, mmio_read_32(PMU_BASE + PMU_PLLPD_CON));
	INFO("DDR: PMU_DDR_PWR_CON(0x%x):\t0x%x\n",
	     PMU_DDR_PWR_CON, pmu_ddr_pwr_con);
	if (pmu_ddr_pwr_con & BIT(DDR_SREF_ENA))
		INFO("\t DDR_SREF_ENA\n");
	if (pmu_ddr_pwr_con & BIT(DDRIO_RET_ENTER_ENA))
		INFO("\t DDRIO_RET_ENTER_ENA\n");
	if (pmu_ddr_pwr_con & BIT(DDRIO_RET_EXIT_ENA))
		INFO("\t DDRIO_RET_EXIT_ENA\n");
	if (pmu_ddr_pwr_con & BIT(DDRPHY_AUTO_GATING_ENA))
		INFO("\t DDRPHY_AUTO_GATING_ENA\n");
}

static void pmu_dsu_suspend_config(void)
{
	uint32_t pmu_dsu_pwr_con;

	pmu_dsu_pwr_con = BIT(DSU_PWRDN_ENA);

	mmio_write_32(PMU_BASE + PMU_CLUSTER_IDLE_CON, 0x000f000f);
	mmio_write_32(PMU_BASE + PMU_DSU_PWR_CON, WRITE_MASK_SET(pmu_dsu_pwr_con));
	mmio_write_32(PMU_BASE + PMU_PWR_CON, WRITE_MASK_CLR(BIT(DSU_BYPASS)));
	cortex_a55_dsu_pwr_dwn();

	INFO("DSU: PMU_DSU_PWR_CON(0x%x): 0x%x\n",
	     PMU_DSU_PWR_CON, mmio_read_32(PMU_BASE + PMU_DSU_PWR_CON));
	INFO("DSU: PMU_CLUSTER_IDLE_CON(0x%x),: 0x%x\n",
	     PMU_CLUSTER_IDLE_CON,  mmio_read_32(PMU_BASE + PMU_CLUSTER_IDLE_CON));
	INFO("DSU: PMU_PWR_CON(0x%x),: 0x%x\n",
	     PMU_PWR_CON, mmio_read_32(PMU_BASE + PMU_PWR_CON));
}

static void pmu_cpu_powerdown_config(void)
{
	uint32_t pmu_cluster_pwr_st, cpus_state, cpus_bypass;

	pmu_cluster_pwr_st = mmio_read_32(PMU_BASE + PMU_CLUSTER_PWR_ST);
	cpus_state = pmu_cluster_pwr_st & 0x0f;

	cpus_bypass = cpus_state << CPU0_BYPASS;

	INFO("CPU: PMU_CLUSTER_PWR_ST(0x%x):0x%x\n",
	     PMU_CLUSTER_PWR_ST, mmio_read_32(PMU_BASE + PMU_CLUSTER_PWR_ST));
	cortex_a55_core_pwr_dwn();
	mmio_write_32(PMU_BASE + PMU_PWR_CON, (0xf << (16 + CPU0_BYPASS)) | cpus_bypass);

	INFO("CPU: PMU_PWR_CON(0x%x), 0x%x\n",
	     PMU_PWR_CON,  mmio_read_32(PMU_BASE + PMU_PWR_CON));
}

static void pvtm_32k_config(void)
{
	uint32_t pmu_cru_pwr_con;
	uint32_t pvtm_freq_khz, pvtm_div;

	mmio_write_32(PMUCRU_BASE + PMUCRU_PMUGATE_CON01, 0x38000000);
	mmio_write_32(PMUPVTM_BASE + PVTM_CON0, 0x00020002);
	dsb();

	mmio_write_32(PMUPVTM_BASE + PVTM_CON0, 0x001c0000);

	mmio_write_32(PMUPVTM_BASE + PVTM_CON1, PVTM_CALC_CNT);
	dsb();

	mmio_write_32(PMUPVTM_BASE + PVTM_CON0, 0x00010001);
	dsb();

	while (mmio_read_32(PMUPVTM_BASE + PVTM_STATUS1) < 30)
		;

	dsb();
	while (!(mmio_read_32(PMUPVTM_BASE + PVTM_STATUS0) & 0x1))
		;

	pvtm_freq_khz = (mmio_read_32(PMUPVTM_BASE + PVTM_STATUS1) * 24000 +
		PVTM_CALC_CNT / 2) / PVTM_CALC_CNT;
	pvtm_div = (pvtm_freq_khz + 16) / 32;

	mmio_write_32(PMUGRF_BASE + PMU_GRF_DLL_CON0, pvtm_div);

	mmio_write_32(PMUCRU_BASE + PMUCRU_PMUCLKSEL_CON00, 0x00c00000);

	pmu_cru_pwr_con = BIT(ALIVE_32K_ENA) | BIT(OSC_DIS_ENA);

	mmio_write_32(PMU_BASE + PMU_WAKEUP_TIMEOUT_CNT, 32000 * 10);

	mmio_write_32(PMU_BASE + PMU_CRU_PWR_CON, WRITE_MASK_SET(pmu_cru_pwr_con));
	INFO("PVTM: PMU_CRU_PWR_CON(0x0%x): 0x%x\n",
	     PMU_CRU_PWR_CON, mmio_read_32(PMU_BASE + PMU_CRU_PWR_CON));
}

static void pmu_cru_suspendmode_config(void)
{
	uint32_t pmu_cru_pwr_con;

	pmu_cru_pwr_con = BIT(ALIVE_OSC_ENA);

	mmio_write_32(PMU_BASE + PMU_CRU_PWR_CON, WRITE_MASK_SET(pmu_cru_pwr_con));
	INFO("CRU: PMU_CRU_PWR_CON(0x0%x): 0x%x\n",
	     PMU_CRU_PWR_CON, mmio_read_32(PMU_BASE + PMU_CRU_PWR_CON));
}

static void pmu_suspend_cru_fsm(void)
{
	pmu_pmic_sleep_mode_config();

	/* Global interrupt disable */
	mmio_write_32(PMU_BASE + PMU_INT_MASK_CON, CLB_INT_DISABLE);
	mmio_write_32(PMU_BASE + PMU_PWR_CON, CPUS_BYPASS);

	pmu_stable_count_config();
	pmu_wakeup_source_config();
	mmio_write_32(PMU_BASE + PMU_WAKEUP_TIMEOUT_CNT, 0x5dc0 * 20000);
	/* default cru config */
	mmio_write_32(PMU_BASE + PMU_CRU_PWR_CON, WRITE_MASK_SET(BIT(ALIVE_OSC_ENA)));

	pmu_cru_suspendmode_config();
	pmu_cpu_powerdown_config();
	pmu_pll_powerdown_config();
	pmu_pd_powerdown_config();
	pmu_ddr_suspend_config();
	pmu_dsu_suspend_config();
	pvtm_32k_config();
	cortex_a55_core_pwr_dwn();
	mmio_write_32(PMU_BASE + PMU_PWR_CON, 0x00010001);
}

static void pmu_reinit(void)
{
	mmio_write_32(DDRGRF_BASE + GRF_DDR_CON3, grf_ddr_con3 | 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_INT_MASK_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_WAKEUP_INT_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_BUS_IDLE_CON0, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_DDR_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_BUS_IDLE_CON1, 0xffff0000);

	mmio_write_32(PMU_BASE + PMU_PWR_GATE_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_VOL_GATE_SFTCON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_CRU_PWR_CON, 0xffff0000);

	mmio_write_32(PMU_BASE + PMU_PLLPD_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_INFO_TX_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_DSU_PWR_CON, 0xffff0000);
	mmio_write_32(PMU_BASE + PMU_CLUSTER_IDLE_CON, 0xffff0000);
}

void rockchip_plat_mmu_el3(void)
{
}

int rockchip_soc_cores_pwr_dm_suspend(void)
{
	return 0;
}

int rockchip_soc_cores_pwr_dm_resume(void)
{
	return 0;
}

int rockchip_soc_sys_pwr_dm_suspend(void)
{
	psram_sleep_cfg->pm_flag = 0;
	flush_dcache_range((uintptr_t)&(psram_sleep_cfg->pm_flag),
			   sizeof(uint32_t));
	pmu_suspend_cru_fsm();

	return 0;
}

int rockchip_soc_sys_pwr_dm_resume(void)
{
	pmu_reinit();
	plat_rockchip_gic_cpuif_enable();
	psram_sleep_cfg->pm_flag = PM_WARM_BOOT_BIT;
	flush_dcache_range((uintptr_t)&(psram_sleep_cfg->pm_flag),
			   sizeof(uint32_t));

	return 0;
}

static int cpus_power_domain_off(uint32_t cpu_id, uint32_t pd_cfg)
{
	uint32_t apm_value, offset, idx;

	apm_value = BIT(core_pm_en) | BIT(core_pm_int_wakeup_glb_msk);

	if (pd_cfg == core_pwr_wfi_int)
		apm_value |= BIT(core_pm_int_wakeup_en);

	idx = cpu_id / 2;
	offset = (cpu_id % 2) << 3;

	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(idx),
		      BITS_WITH_WMASK(apm_value, 0xf, offset));
	dsb();

	return 0;
}

static int cpus_power_domain_on(uint32_t cpu_id)
{
	uint32_t offset, idx;

	idx = cpu_id / 2;
	offset = (cpu_id % 2) << 3;

	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(idx),
		      WMSK_BIT(core_pm_en + offset));
	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(idx),
		      BIT_WITH_WMSK(core_pm_sft_wakeup_en + offset));
	dsb();

	return 0;
}

int rockchip_soc_cores_pwr_dm_on(unsigned long mpidr, uint64_t entrypoint)
{
	uint32_t cpu_id = plat_core_pos_by_mpidr(mpidr);

	assert(cpu_id < PLATFORM_CORE_COUNT);

	cpuson_flags[cpu_id] = PMU_CPU_HOTPLUG;
	cpuson_entry_point[cpu_id] = entrypoint;
	flush_dcache_range((uintptr_t)cpuson_flags, sizeof(cpuson_flags));
	flush_dcache_range((uintptr_t)cpuson_entry_point,
			   sizeof(cpuson_entry_point));

	cpus_power_domain_on(cpu_id);
	return 0;
}

int rockchip_soc_cores_pwr_dm_off(void)
{
	uint32_t cpu_id = plat_my_core_pos();

	cpus_power_domain_off(cpu_id,
			      core_pwr_wfi);
	return 0;
}

int rockchip_soc_cores_pwr_dm_on_finish(void)
{
	uint32_t cpu_id = plat_my_core_pos();
	uint32_t offset, idx;

	/* Disable core_pm */
	idx = cpu_id / 2;
	offset = (cpu_id % 2) << 3;
	mmio_write_32(PMU_BASE + PMU_CPUAPM_CON(idx),
		      BITS_WITH_WMASK(0, 0xf, offset));

	return 0;
}

/*
 * Cortex-a55
 * Debug Power/Reset Control Register(DBGPRCR_EL1)
 *
 * CORENPDRQ, bit[0]
 * Core no powerdown request. Requests emulation of powerdown. Possible values
 * of this bit are:
 *   0:  On a powerdown request, the system powers down the Core power domain
 *   1:  On a powerdown request, the system emulates powerdown of the Core power domain.
 *
 * In this emulation mode the Core power domain is not actually powered down.
 * On Cold reset, the field resets to the value of EDPRCR.COREPURQ.
 *
 * S3_0_C15_C2_7: CORTEX_A55_CPUPWRCTLR_EL1.
 */
static inline void core_dbgprcr_pwrdown_wfi(void)
{
	__asm__ volatile (
		"msr DBGPRCR_EL1, xzr\n"
		"mrs x0, S3_0_C15_C2_7\n"
		"orr x0, x0, #0x1\n"
		"msr S3_0_C15_C2_7, x0\n"
		"wfi_loop:\n"
		"isb\n"
		"wfi\n"
		"b wfi_loop\n");
}

static void cpus_pd_req_enter_wfi(void)
{
	core_dbgprcr_pwrdown_wfi();
}

static void nonboot_cpus_off(void)
{
	uint32_t tmp;

	cpus_power_domain_off(1, 0);
	cpus_power_domain_off(2, 0);
	cpus_power_domain_off(3, 0);

	mmio_write_32(SYSSRAM_BASE + 0x04, 0xdeadbeaf);
	mmio_write_32(SYSSRAM_BASE + 0x08, (uintptr_t)&cpus_pd_req_enter_wfi);
	sev();

	do {
		tmp = mmio_read_32(PMU_BASE + PMU_CLUSTER_PWR_ST);
	} while ((tmp & 0xe) != 0xe);
}

void plat_rockchip_pmu_init(void)
{
	uint32_t cpu;

	rockchip_pd_lock_init();
	nonboot_cpus_off();
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++)
		cpuson_flags[cpu] = PMU_CPU_HOTPLUG;

	psram_sleep_cfg->ddr_data = (uint64_t)0;
	psram_sleep_cfg->sp = PSRAM_SP_TOP;
	psram_sleep_cfg->ddr_flag = 0x00;
	psram_sleep_cfg->boot_mpidr = read_mpidr_el1() & 0xffff;
	psram_sleep_cfg->pm_flag = PM_WARM_BOOT_BIT;

	/*
	 * When perform idle operation, corresponding clock can be
	 * opened or gated automatically.
	 */
	mmio_write_32(PMU_BASE + PMU_NOC_AUTO_CON0, 0xffffffff);
	mmio_write_32(PMU_BASE + PMU_NOC_AUTO_CON1, 0x00070007);

	/* grf_con_pmic_sleep_sel
	 * pmic sleep function selection
	 * 1'b0: From reset pulse generator, can reset external PMIC
	 * 1'b1: From pmu block, only support sleep function for external PMIC
	 */
	mmio_write_32(PMUGRF_BASE + PMU_GRF_SOC_CON(0), 0x00800080);

	/*
	 * force jtag control
	 * 1'b0: CPU debug port IO mux is controlled by sdmmc_detect_en status
	 * 1'b0: CPU debug port IO mux IS controlled by GRF
	 */
	mmio_write_32(SGRF_BASE + 0x008, 0x00100000);

	/*
	 * remap
	 * 2'b00: Boot from boot-rom.
	 * 2'b01: Boot from pmu mem.
	 * 2'b10: Boot from sys mem.
	 */
	mmio_write_32(PMUSGRF_BASE + PMU_SGRF_SOC_CON1, 0x18000800);
}
