/**
 * @file sta_pmu.c
 * @brief Provide all the sta PMU driver functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: Jean-Nicolas GRAUX <jean-nicolas.graux@st.com>
 */
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "utils.h"

#include "sta_map.h"
#include "sta_pmu.h"
#include "sta_type.h"

/* Registers offsets */
#define PMU_CR				0x00
#define PMU_WKUPSRC_SEL			0x04
#define PMU_SBYSRC_SEL			0x08
#define PMU_EDGE_SEL			0x0C
#define PMU_N2SBY_STATUS		0x10
#define PMU_SBY2N_STATUS		0x14
#define PMU_AO_DEDIPAD_PEN		0x18
#define PMU_AO_DEDIPAD_PUPD		0x1C
#define PMU_SPR(x)			0x20 + ((x & 3) * 4)
#define PMU_PERIPH_ID(x)		0x30 + ((x & 3) * 4)
#define PMU_PCELL_ID			0x40 + ((x & 3) * 4)
#define PMU_GEN_STATUS			0x64
#define PMU_N2SBY_PEND_STATUS		0x68
#define PMU_SBY2N_PEND_STATUS		0x6C
#define PMU_SWTST_CTRL			0x78
#define PMU_ONOFF_CTRL			0x7C
#define PMU_SPARE1			0x80

/* PMU_SBYSRC_SEL bits */
#define PMU_SWGOTOSBY			BIT(18)

/* PMU_CR bits */
#define PMU_N2SBYCTR_TO_BIT		26
#define PMU_XCOSC32K_BYPASS		BIT(22)
#define PMU_XCOSC32K_EN			BIT(23)
#define PMU_IOCTRL_GPIOITF		BIT(25)
#define PMU_VDDOK_FE_STATUS_BIT	3

/**
 * struct pmu_status - Power Manament Unit status
 * @wkup_status:	used to store PMU_SBY2N_STATUS register at boot time.
 * @sby_status:		used to store PMU_N2SBY_STATUS register on a hardware Go To
 *			STAND-BY event.
 */
struct pmu_status {
	int sby_status;
	int wkup_status;
};

static struct pmu_status pmu_stat;

static inline void __pmu_enable_wkup_src(uint32_t sel)
{
	pmu_regs->src_sel_reg.reg |= (sel & PMU_WKUP_SRC_MASK);
}

static inline void __pmu_disable_wkup_src(uint32_t sel)
{
	uint32_t val;

	val = ~(sel & PMU_WKUP_SRC_MASK);
	pmu_regs->src_sel_reg.reg = pmu_regs->src_sel_reg.reg & val;
}

static inline void __pmu_enable_wkup_edge(uint32_t sel)
{
	pmu_regs->edge_sel_reg.reg |= (sel & PMU_WKUP_SRC_MASK);
}

static inline void __pmu_disable_wkup_edge(uint32_t sel)
{
	uint32_t val;

	val = ~(sel & PMU_WKUP_SRC_MASK);
	pmu_regs->edge_sel_reg.reg = pmu_regs->edge_sel_reg.reg & val;
}

static inline void __pmu_enable_sby_src(uint32_t sel)
{
	pmu_regs->stby_src_sel_reg.reg |= (sel & PMU_SBY_SRC_MASK);
}

static inline void __pmu_disable_sby_src(uint32_t sel)
{
	uint32_t val;

	val = ~(sel & PMU_SBY_SRC_MASK);
	pmu_regs->stby_src_sel_reg.reg = pmu_regs->stby_src_sel_reg.reg & val;
}

static inline void __pmu_enable_sby_edge(uint32_t sel)
{
	pmu_regs->edge_sel_reg.reg |= (sel & PMU_SBY_SRC_MASK);
}

static inline void __pmu_disable_sby_edge(uint32_t sel)
{
	uint32_t val;

	val = ~(sel & PMU_SBY_SRC_MASK);
	pmu_regs->edge_sel_reg.reg = pmu_regs->edge_sel_reg.reg & val;
}

static inline uint32_t __pmu_get_sby_src(void)
{
	return pmu_regs->stby_src_sel_reg.reg & PMU_SBY_SRC_MASK;
}

static uint32_t __pmu_get_wkup_src(void)
{
	return pmu_regs->src_sel_reg.reg & PMU_WKUP_SRC_MASK;
}

static inline void __pmu_enable_sby_interrupts(uint32_t sel)
{
	pmu_regs->ctrl_reg.reg |= (sel & PMU_SBY_SRC_MASK);
}

static inline void __pmu_disable_sby_interrupts(uint32_t sel)
{
	uint32_t val;

	val = ~(sel & PMU_SBY_SRC_MASK);
	pmu_regs->ctrl_reg.reg = pmu_regs->ctrl_reg.reg & val;
}

void pmu_enable_wkup_src(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_enable_wkup_src(sel);
	taskEXIT_CRITICAL();
}

void pmu_disable_wkup_src(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_disable_wkup_src(sel);
	taskEXIT_CRITICAL();
}

void pmu_enable_wkup_edge(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_enable_wkup_edge(sel);
	taskEXIT_CRITICAL();
}

void pmu_disable_wkup_edge(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_disable_wkup_edge(sel);
	taskEXIT_CRITICAL();
}

void pmu_enable_sby_src(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_enable_sby_src(sel);
	taskEXIT_CRITICAL();
}

void pmu_disable_sby_src(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_disable_sby_src(sel);
	taskEXIT_CRITICAL();
}

void pmu_enable_sby_edge(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_enable_sby_edge(sel);
	taskEXIT_CRITICAL();
}

void pmu_disable_sby_edge(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_disable_sby_edge(sel);
	taskEXIT_CRITICAL();
}

void pmu_enable_sby_irq(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_enable_sby_interrupts(sel);
	taskEXIT_CRITICAL();
}

void pmu_disable_sby_irq(uint32_t sel)
{
	taskENTER_CRITICAL();
	__pmu_disable_sby_interrupts(sel);
	taskEXIT_CRITICAL();
}

uint32_t pmu_get_wkup_stat(void)
{
	return pmu_stat.wkup_status;
}

uint32_t pmu_get_wkup_src(void)
{
	return __pmu_get_wkup_src();
}

uint32_t pmu_get_sby_stat(void)
{
	return pmu_stat.sby_status;
}

uint32_t pmu_get_sby_src(void)
{
	return __pmu_get_sby_src();
}

void pmu_goto_sby(void)
{
	if (portNVIC_INT_ACTIVE())
		taskENTER_CRITICAL_FROM_ISR();
	else
		taskENTER_CRITICAL();

	/* clear wake-up status */
	pmu_regs->stby_nrm_stat_reg.reg |=
		(PMU_WKUP_STAT_MASK|PMU_SBY2NCTR_DONE);
	/* disable interrupts */
	__pmu_disable_sby_interrupts(PMU_SBY_SRC_MASK);
	/* move to STAND-BY state */
	pmu_regs->stby_src_sel_reg.bit.swgotosby = 1;

	/** Loop forever, waiting for SoC to effectively
	 * enter the STAND-BY state.
	 */
	while(1);
}

void pmu_clear_sby(void)
{
	UBaseType_t saved_interrupt_status = 0;

	if (portNVIC_INT_ACTIVE())
		saved_interrupt_status = taskENTER_CRITICAL_FROM_ISR();
	else
		taskENTER_CRITICAL();

	/* store STAND-BY status */
	pmu_stat.sby_status = pmu_regs->nrm_stby_stat_reg.reg;

	/* clear STAND-BY status */
	pmu_regs->nrm_stby_stat_reg.reg = pmu_stat.sby_status &
		PMU_SBY_SRC_MASK;

	if (portNVIC_INT_ACTIVE())
		taskEXIT_CRITICAL_FROM_ISR(saved_interrupt_status);
	else
		taskEXIT_CRITICAL();
}

void pmu_init(void) {
	/* store wake-up status */
	pmu_stat.wkup_status = pmu_regs->stby_nrm_stat_reg.reg;

	/* clear wake-up status */
	pmu_regs->stby_nrm_stat_reg.reg |= (PMU_WKUP_STAT_MASK|PMU_SBY2NCTR_DONE);

	if (pmu_stat.wkup_status & PMU_PORLVD) {
		/* Power On Reset */

		/**
		 * Optionally enable/disable other wake-up sources
		 * than default ones already enabled on a POR.
		 */
		/*__pmu_enable_wkup_src(PMU_WKUP_SRC_MASK);*/

		/**
		 * Optionally enable/disable other stand-by sources
		 * than default ones already enabled on a POR
		 */
		/*__pmu_enable_sby_src(PMU_SBY_SRC_MASK);*/

	} else {
		/* STAND-BY to ON */
		if (pmu_regs->nrm_stby_stat_reg.bit.hwsby_vddok_fe_status &&
				pmu_regs->nrm_stby_pending_stat_reg.bit.hwsby_vddok_fe_status) {

			/* Go To STAND-BY event is pending! Clear it */
			pmu_regs->nrm_stby_stat_reg.bit.hwsby_vddok_fe_status = 1;

			pmu_goto_sby();
		}
		/* clear stand-by status */
		pmu_regs->nrm_stby_stat_reg.reg =
			pmu_regs->nrm_stby_stat_reg.reg & PMU_SBY_SRC_MASK;
	}

	/* enable PMU interrupts sources */
	__pmu_enable_sby_interrupts(__pmu_get_sby_src());

	/* HwGoToSby timeout counter to max value */
	pmu_regs->ctrl_reg.bit.n2sbyctr_to = PMU_N2SBYCTR_TO_MAX;

	/* enable RTC */
	pmu_regs->ctrl_reg.bit.xcosc32k_bypass   = 0;
	pmu_regs->ctrl_reg.bit.xcosc32k_en       = 1;

	/* set GPIO_ITF bit to make M3 GPIOs controlable from AP */
	pmu_regs->ctrl_reg.bit.ioctrl_gpioitf    = 1;
}

void pmu_free_ao_gpio(void)
{
	/* set GPIO_ITF bit to make M3 GPIOs controlable from AP */
	pmu_regs->ctrl_reg.bit.ioctrl_gpioitf    = 1;
}

