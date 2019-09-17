/*
 * Copyright (c) 2014-2016, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Declare variables shared between M3 and AP processors */
#define DECLARE_SHARED_DATA

#include <arm32.h>
#include <console.h>
#include <drivers/gic.h>
#include <drivers/pl011.h>
#include <io.h>
#include <kernel/generic_boot.h>
#include <kernel/misc.h>
#include <kernel/panic.h>
#include <kernel/pm_stubs.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <tee/entry_std.h>
#include <tee/entry_fast.h>
#include <sta_helper.h>
#include <trace.h>
#include <util.h>
#include <mailboxes.h>

/*
 * Platform specific MMU mapping configuration
 */
/*** Secure/NonSecure iomem (non cachable) ***/
/* Platform peripheral */
#if defined(CFG_SM_PLATFORM_HANDLER) && !defined(CFG_STA_REMOTEPROC_CTRL)
register_phys_mem(MEM_AREA_IO_SEC, PERIPHS_IOMEM_BASE, PERIPHS_IOMEM_SIZE);
#else
register_phys_mem(MEM_AREA_IO_SEC_SPG, MBOX_M3_IOMEM_BASE, MBOX_M3_IOMEM_SIZE);
register_phys_mem(MEM_AREA_IO_SEC_SPG, MBOX_HSM_IOMEM_BASE,
		  MBOX_HSM_IOMEM_SIZE);
register_phys_mem(MEM_AREA_IO_SEC_SPG, GIC_IOMEM_BASE, GIC_IOMEM_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC_SPG, UART2_IOMEM_BASE, UART2_IOMEM_SIZE);
register_phys_mem(MEM_AREA_IO_NSEC_SPG, UART3_IOMEM_BASE, UART3_IOMEM_SIZE);
#endif
/* M3 eSRAM for M3_OS part1 loading at PSCI resume state */
register_phys_mem(MEM_AREA_IO_SEC, ESRAM_M3_IOMEM_BASE, ESRAM_M3_IOMEM_SIZE);
#if defined(SCP_FILE2_BL2_BASE)
/* A7 eSRAM for M3_OS part2 loading at PSCI resume state */
register_phys_mem(MEM_AREA_IO_SEC, ESRAM_A7_IOMEM_BASE, ESRAM_A7_IOMEM_SIZE);
#endif
/* A7 eSRAM for IPC M3 & HSM shared mem */
register_phys_mem(MEM_AREA_IO_SEC_SPG, IPC_M3_IOMEM_BASE, IPC_M3_IOMEM_SIZE);
register_phys_mem(MEM_AREA_IO_SEC_SPG, IPC_HSM_IOMEM_BASE, IPC_HSM_IOMEM_SIZE);
/* DDRAM NonSecure for shared_data */
register_phys_mem(MEM_AREA_IO_NSEC_SPG, SHARED_DATA_IOMEM_BASE,
		  SHARED_DATA_IOMEM_SIZE);

/*** Secure/NonSecure mem ***/
/* DDRAM Secure for PSCI STR saved area */
register_phys_mem(MEM_AREA_RAM_SEC, MEM_DRAM_SEC_BASE, MEM_DRAM_SEC_SIZE);

static struct gic_data gic_data;
static struct pl011_data console_data;
static uint32_t plat_cntfrq = TIMER_CLK_FREQ;

static void main_fiq(void);
static void prepare_cpu_sys_regs(void);

static const struct thread_handlers handlers = {
	.std_smc = tee_entry_std,
	.fast_smc = tee_entry_fast,
	.nintr = main_fiq,
	.cpu_on = pm_panic,
	.cpu_off = pm_panic,
	.cpu_suspend = pm_panic,
	.cpu_resume = pm_panic,
	.system_off = pm_panic,
	.system_reset = pm_panic,
};

const struct thread_handlers *generic_boot_get_handlers(void)
{
	return &handlers;
}

void *get_shared_data_base(void)
{
	if (cpu_mmu_enabled()) {
		struct shared_data_t *base = (struct shared_data_t *)
					phys_to_virt_io(SHARED_DATA_BASE);

		return (void *)&base->bi;
	} else {
		return (void *)&shared_data.bi;
	}
}

void *get_shared_data_rw_base(void)
{
	if (cpu_mmu_enabled()) {
		struct shared_data_t *base = (struct shared_data_t *)
					phys_to_virt_io(SHARED_DATA_BASE);

		return (void *)&base->rw;
	} else {
		return (void *)&shared_data.rw;
	}
}

void plat_cpu_resume_reset_late(void)
{
	/* Set cntfrq according to the store value before suspend */
	write_cntfrq(plat_cntfrq);

	/* Prepare the CPU banked registers for first entry into normal world */
	prepare_cpu_sys_regs();
}

#define gicd_make_sgir(int_id, cpu_id) ((int_id) | 1 << (cpu_id) << 16)
void plat_cpu_reset_late(void)
{
	uint32_t cpu_id = get_core_pos();

	if (!cpu_id) {
		/* Primary CPU init specific */
		uint32_t request = gicd_make_sgir(WAKING_UP_SGI, cpu_id + 1);
		uint32_t addr = MAILBOXES_BASE +
				(MAILBOX_SIZE * (cpu_id + 1));

		/* Get cntfrq from primary core to apply it to other cores */
		plat_cntfrq = read_cntfrq();

		/* Set mailboxes control word and entrypoint */
		write32(WAKE_UP_CONTROL_WORD, addr + MAILBOX_CONTROL_WORD);
		write32(CFG_TEE_RAM_START, addr + MAILBOX_SEC_ENTRYPOINT);

		/*
		 * Program GIC so that primary core can distribute a
		 * SGI to wake up a secondary core.
		 */
		addr = GIC_DIST_BASE;
		write32(ENABLE_GRP0 | ENABLE_GRP1, addr + GICD_CTLR);
		dsb();
		/* Generate SGI to wake-up secondary CPU */
		write32(request, addr + GICD_SGIR);
	} else {
		/* Secondary CPU init specific */
		uint32_t addr = MAILBOXES_BASE +
				(MAILBOX_SIZE * cpu_id);

		/* The cntfrq is banked for each core */
		write_cntfrq(plat_cntfrq);

		/* Init cpu1 mailboxes control word for next reboot or wakeup */
		write32(0, addr + MAILBOX_CONTROL_WORD);
	}
	/* Prepare the CPU banked registers for first entry into normal world */
	prepare_cpu_sys_regs();
}

void console_init(void)
{
	paddr_t console_base;
	/*
	 * UART3 for A5
	 * UART2 for TC3P
	 */
	switch (get_soc_id()) {
	case SOCID_STA1295:
	case SOCID_STA1195:
	case SOCID_STA1275:
		console_base = UART3_BASE;
		break;
	case SOCID_STA1385:
	default:
		console_base = UART2_BASE;
		break;
	}

	pl011_init(&console_data, console_base, CONSOLE_UART_CLK_IN_HZ,
		   CONSOLE_BAUDRATE);
	register_serial_console(&console_data.chip);
}

void main_init_gic(void)
{
	vaddr_t gicc_base;
	vaddr_t gicd_base;

	gicc_base = (vaddr_t)phys_to_virt_io(GIC_CPU_BASE);
	gicd_base = (vaddr_t)phys_to_virt_io(GIC_DIST_BASE);

	if (!gicc_base || !gicd_base)
		panic();

	gic_init(&gic_data, gicc_base, gicd_base);
	itr_init(&gic_data.chip);
}

void main_secondary_init_gic(void)
{
	gic_cpu_init(&gic_data);
}

static void main_fiq(void)
{
	gic_it_handle(&gic_data);
}

/*******************************************************************************
 * Prepare the CPU system registers for first entry into secure or normal world
 *
 * If execution is requested to non-secure PL1, and the CPU supports
 * HYP mode then HYP mode is disabled by configuring all necessary HYP mode
 * registers.
 ******************************************************************************/
static void prepare_cpu_sys_regs(void)
{
	/* Set CPU mode to access NS copies of certain banked registers */
	set_cpsr_mode_mon();
	isb();

	if (read_id_pfr1() &
	    (ID_PFR1_VIRTEXT_MASK << ID_PFR1_VIRTEXT_SHIFT)) {
		/*
		 * Set the NS bit to access NS copies of certain banked
		 * registers
		 */
		write_scr(read_scr() | SCR_NS);
		isb();

		/*
		 * Hyp / PL2 present but unused, need to disable safely.
		 * HSCTLR can be ignored in this case.
		 *
		 * Set HCR to its architectural reset value so that
		 * Non-secure operations do not trap to Hyp mode.
		 */
		write_hcr(HCR_RESET_VAL);

		/*
		 * Set HCPTR to its architectural reset value so that
		 * Non-secure access from EL1 or EL0 to trace and to
		 * Advanced SIMD and floating point functionality does
		 * not trap to Hyp mode.
		 */
		write_hcptr(HCPTR_RESET_VAL);

		/*
		 * Initialise CNTHCTL. All fields are architecturally
		 * UNKNOWN on reset and are set to zero except for
		 * field(s) listed below.
		 *
		 * CNTHCTL.PL1PCEN: Disable traps to Hyp mode of
		 *  Non-secure EL0 and EL1 accessed to the physical
		 *  timer registers.
		 *
		 * CNTHCTL.PL1PCTEN: Disable traps to Hyp mode of
		 *  Non-secure EL0 and EL1 accessed to the physical
		 *  counter registers.
		 */
		write_cnthctl(CNTHCTL_RESET_VAL | PL1PCEN_BIT | PL1PCTEN_BIT);

		/*
		 * Initialise CNTVOFF to zero as it resets to an
		 * IMPLEMENTATION DEFINED value.
		 */
		write64_cntvoff(0);

		/*
		 * Set VPIDR and VMPIDR to match MIDR_EL1 and MPIDR
		 * respectively.
		 */
		write_vpidr(read_midr());
		write_vmpidr(read_mpidr());

		/*
		 * Initialise VTTBR, setting all fields rather than
		 * relying on the hw. Some fields are architecturally
		 * UNKNOWN at reset.
		 *
		 * VTTBR.VMID: Set to zero which is the architecturally
		 *  defined reset value. Even though EL1&0 stage 2
		 *  address translation is disabled, cache maintenance
		 *  operations depend on the VMID.
		 *
		 * VTTBR.BADDR: Set to zero as EL1&0 stage 2 address
		 *  translation is disabled.
		 */
		write64_vttbr(VTTBR_RESET_VAL);

		/*
		 * Initialise HDCR, setting all the fields rather than
		 * relying on hw.
		 *
		 * HDCR.HPMN: Set to value of PMCR.N which is the
		 *  architecturally-defined reset value.
		 */
		write_hdcr(HDCR_RESET_VAL |
			   ((read_pmcr() & PMCR_N_BITS) >> PMCR_N_SHIFT));

		/*
		 * Set HSTR to its architectural reset value so that
		 * access to system registers in the cproc=1111
		 * encoding space do not trap to Hyp mode.
		 */
		write_hstr(HSTR_RESET_VAL);

		/*
		 * Set CNTHP_CTL to its architectural reset value to
		 * disable the EL2 physical timer and prevent timer
		 * interrupts. Some fields are architecturally UNKNOWN
		 * on reset and are set to zero.
		 */
		write_cnthp_ctl(CNTHP_CTL_RESET_VAL);
		isb();

		write_scr(read_scr() & ~SCR_NS);
		isb();
	}
	/* OPTEE expects the processor mode set to SVC */
	set_cpsr_mode_svc();
	isb();
}

