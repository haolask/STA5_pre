/**
 * @file sta_nvic.c
 * @brief Nested Vectored Interrupt Controller functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "sta_map.h"
#include "sta_nvic.h"

#define CPUID_REV_MASK				0x0F
#define CPUID_PARTNO_MASK			0xFF
#define CPUID_VARIANT_MASK			0x0F
#define CPUID_IMPL_MASK				0xFF

#define ISR_VECTACT_MASK			0x1FF
#define ISR_RETTOBASE				BIT(11)
#define ISR_VECTPEND_MASK			(0x3FF << 12)
#define ISR_PENDING					BIT(22)
#define ISR_PREEMPT					BIT(23)
#define ISR_PENDSTCLR				BIT(25)
#define ISR_PENDSTSET				BIT(26)
#define ISR_PENDSVCLR				BIT(27)
#define ISR_PENDSVSET				BIT(28)
#define ISR_NMIPENDSET				BIT(31)

/* Register key.
 * Writing to SCB_AIRCR this register requires 0x5FA in the VECTKEY field.
 * Otherwise the write value is ignored.
 * Reads as 0xFA05.
 */
#define AIRC_VECTKEY_W				0x05fa
#define AIRC_VECTKEY_R				0xfa05
#define AIRCR_VECTKEY_SHIFT         16
#define AIRC_VECTKEY_RW_MASK        (0xFFFF << AIRCR_VECTKEY_SHIFT)

#define is_nvic_priority_grp_valid(g) ((g == NVIC_PRIORITY_GRP0) || \
                                       (g == NVIC_PRIORITY_GRP1) || \
                                       (g == NVIC_PRIORITY_GRP2) || \
                                       (g == NVIC_PRIORITY_GRP3) || \
                                       (g == NVIC_PRIORITY_GRP4))
#define AIRCR_PRIGROUP_SHIFT        8
#define AIRCR_PRIGROUP_MASK         (7 << AIRCR_PRIGROUP_SHIFT)

/* Depending on the IRQ_MAX_PRIO priority rank available, compute the number
 * of bits that are allocated to the priority group */
#define NVIC_MAX_PRIGROUP_BITS count_bits_sets(IRQ_MAX_PRIO_INDEX)

#define AIRCR_PRIGROUP(aircr)		(aircr & AIRCR_PRIGROUP_MASK) >> AIRCR_PRIGROUP_SHIFT
#define AIRCR_MAX_PRIGROUP			NVIC_MAX_PRIGROUP_BITS /* hardware implementation dependent */

#define AIRCR_VECTRESET				BIT(0)
#define AIRCR_SRB_VECTCLRACTIVE		BIT(1)
#define AIRCR_SYSRESETREQ			BIT(2)

#define IRQ_MAX_CHNL				48
#define IRQ_BY_REG					32

#define is_nvic_irq_chnl_valid(c)		(c < IRQ_MAX_CHNL)
#define is_nvic_prio_valid(p)			(p <= IRQ_MAX_PRIO_INDEX)
#define is_nvic_preempt_prio_valid(p)	is_nvic_prio_valid(p)
#define is_nvic_sub_prio_valid(p)		is_nvic_prio_valid(p)

/**
 * @brief  checks if a configuration is valid
 * @param  irq_chnl: configuration of the channel to be checked
 * @retval true if valid, false otherwise
 */
bool nvic_chnl_is_valid(struct nvic_chnl *irq_chnl)
{
	if (!irq_chnl)
		return false;

	if (!is_nvic_irq_chnl_valid(irq_chnl->id))
		return false;

	if (!is_nvic_preempt_prio_valid(irq_chnl->preempt_prio))
		return false;

	/* do not test sub prio since always set to 0 */

	return true;
}

/**
 * @brief  un-initializes nvic peripheral by resetting all interrupts bits
 * (interrupts and pending) as well as the corresponding priorities
 * @param  None
 * @retval None
 */
void nvic_deinit(void)
{
	int i;
	uint32_t reg = 0xFFFFFFFF;

	for (i = 0; i < (IRQ_MAX_CHNL / 6); i++) {
		nvic_regs->clear[i] = reg; /* disable 32 int in a row */
		nvic_regs->clear_pending[i] = reg; /* disable 32 pending int in a row */
	}

	for (i = 0; i < (IRQ_MAX_CHNL / 8); i++)
		nvic_regs->priority[i] = ~reg; /* clears 4 priorities in a row */
}

/**
 * @brief  un-initializes nvi System Control Block (SCB) by resetting almost
 * every register available under SCB:
 *		- interrupts are cleared
 *		- vector table offset register is reset
 *		- Application interrupt and reset controler register is reset
 *		- System control register is reset
 *		- Configuration control register is reset
 * resetting all interrupts bits
 * (interrupts and pending) as well as the corresponding priorities
 * @retval None
 */
void nvic_scb_deinit(void)
{
	int i;

	scb_regs->irq_ctrl_state = (ISR_PENDSTCLR | ISR_PENDSVCLR);
	scb_regs->aircr = (AIRC_VECTKEY_W << AIRCR_VECTKEY_SHIFT);
	scb_regs->except_tbl_off = 0;
	scb_regs->sys_ctrl = 0;
	scb_regs->cfg_ctrl = 0;
	scb_regs->sys_hdl_ctrl = 0;

	scb_regs->cfg_flt_status = 0xFFFFFFFF;
	scb_regs->hrd_flt_status = 0xFFFFFFFF;
	scb_regs->dbg_flt_status = 0xFFFFFFFF;

	/* 3 System Handler Priority Registers to prioritize system handlers */
	for (i = 0; i < 3; i++)
		scb_regs->sys_priority[i] = 0;
}

/**
 * @brief  configures priority group
 * @param  group: priority group to be set to AIRCR. Values for pre-emption
 * priority and subpriority are then determined from the group as follow:
 * group0: 0 bit for preemp prio 4 for subprio
 * group1: 1 bit for preemp prio 3 for subprio
 * group2: 2 bit for preemp prio 2 for subprio
 * group3: 3 bit for preemp prio 1 for subprio
 * group4: 4 bit for preemp prio 0 for subprio
 * @retval 0 if no error, not 0 otherwise
 */
int nvic_set_priority_grp_config(uint32_t group)
{
	if (!is_nvic_priority_grp_valid(group))
		return -EINVAL;

	/* Set the PRIGROUP[10:8] bits */
	scb_regs->aircr = (AIRC_VECTKEY_W << AIRCR_VECTKEY_SHIFT) | group;

	return 0;
}

/**
 * @brief	enables the irq for a given channel (if not already), then sets
 * the enabled field to true.
 * @param	irq_chnl: configuration of the channel to be enabled
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_chnl_enable(struct nvic_chnl *irq_chnl)
{
	nvic_regs->enable[irq_chnl->id >> 5] = BIT(irq_chnl->id & 0x1F);
	irq_chnl->enabled = true;

	return 0;
}

/**
 * @brief	disables the irq for a given channel (if not already), then sets
 * the enabled field to false.
 * @param	irq_chnl: configuration of the channel to be enabled
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_chnl_disable(struct nvic_chnl *irq_chnl)
{
	nvic_regs->clear[irq_chnl->id >> 5] = BIT(irq_chnl->id & 0x1F);
	irq_chnl->enabled = false;

	return 0;
}

/**
 * @brief  set channel priority according to the couple pre-emption
 * priority/subpriority given in argument in irq_chnl structure.
 *
 * @param  irq_chnl: configuration of the channel to be initiliazed
 * @retval None
 */
static void nvic_chnl_set_priority(struct nvic_chnl *irq_chnl)
{
	uint32_t prigroup;
	uint32_t pre, sub;
	uint32_t prio, prio_msk, prio_reg;

	prigroup = AIRCR_PRIGROUP(scb_regs->aircr);
	/*
	 * depending on the priority group, compute the number of bits for pre-emption
	 * priority and subpriority according to the table below:
	 * prigroup	  preempt.subprio
	 *    0				7.1
	 *    1				6.2
	 *    2				5.3
	 *    3				4.4
	 *    4				3.5
	 *    5				2.6
	 *    6				1.7
	 *    7				0.8
	 */
	pre = (AIRCR_MAX_PRIGROUP - prigroup); /* nb bits for preempt prio */
	sub = (1 + prigroup); /* nb bits for sub prio */

	prio = (irq_chnl->preempt_prio << pre) | (irq_chnl->sub_prio & sub);

	/*
	 * If there are four bits of priority, the priority value is stored in bits [7:4] of the byte.
	 * If there are three bits of priority, the priority value is stored in bits [7:5] of the byte.
	 */
	prio <<= 4;

	/* compute priority position and mask */
	prio = prio << ((irq_chnl->id & 0x3) * 8);
	prio_msk = 0xff << ((irq_chnl->id & 0x3) * 8);

	/* apply the new priority */
	prio_reg = nvic_regs->priority[irq_chnl->id >> 2];
	prio_reg &= ~prio_msk;
	prio_reg |= (prio & prio_msk);

	nvic_regs->priority[irq_chnl->id >> 2] = prio_reg;
}

/**
 * @brief  initializes nvic peripheral
 * @param  irq_chnl: configuration of the channel to be initiliazed
 * @retval 0 if no error, not 0 otherwise
 */
int nvic_chnl_init(struct nvic_chnl *irq_chnl)
{
	int ret = 0;

	if (!nvic_chnl_is_valid(irq_chnl))
		return -EINVAL;

	/*
	 * According to http://www.freertos.org/RTOS-Cortex-M3-M4.html:
	 * It is recommended to assign all the priority bits to be preempt priority bits,
	 * leaving no priority bits as subpriority bits.
	 *
	 * Consequence: on STA, sub_prio is always set to 0
	 */
	irq_chnl->sub_prio = 0;

	if (irq_chnl->enabled) {
		nvic_chnl_set_priority(irq_chnl);
		ret = nvic_chnl_enable(irq_chnl);
	} else {
		ret = nvic_chnl_disable(irq_chnl);
	}

	return ret;
}

/**
  * @brief  read the priority group
  * @retval priority group from AIRCR
  */
uint32_t nvic_get_priority_grp_config(void)
{
	return ((scb_regs->aircr & AIRCR_PRIGROUP_MASK) >> AIRCR_PRIGROUP_SHIFT);
}

/**
 * @brief  provides cpu informations such as revision, part number, variant
 * and implementer
 * @param  cpu: structure representation of a cpu
 * @retval 0 if no error, not 0 otherwise
 */
int nvic_cpuid(struct cpu_info *cpu)
{
	uint32_t cpuid;

	if (!cpu)
		return -EINVAL;

	cpuid = scb_regs->cpuid;

	cpu->revision = cpuid & CPUID_REV_MASK;
	cpu->part_number = (cpuid >> 4) & CPUID_PARTNO_MASK;
	cpu->variant = (cpuid  >> 20) & CPUID_VARIANT_MASK;
	cpu->implementer = (cpuid  >> 24) & CPUID_IMPL_MASK;

	return 0;
}

/**
 * @brief	set the system control register low power mode
 * @param	mode: the mode to be set (sleep on exit, sleep deep or sev on
 * pending)
 * @param	enable: set to true when the Cortex-M3 clock can be stopped, false
 * otherwise
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_lp_mode_enable(uint8_t mode, bool enable)
{
	uint32_t reg = scb_regs->sys_ctrl;

	if (mode != NVIC_LP_SEVONPEND
			&& mode !=  NVIC_LP_SLEEPDEEP
			&& mode != NVIC_LP_SLEEPONEXIT)
		return -EINVAL;

	if (enable)
		reg |= mode;
	else
		reg &= ~mode;

	scb_regs->sys_ctrl = reg;

	return 0;
}

/**
 * @brief	enable/disable irq by clearing/setting PRIMASK
 * @param	None
 * @return	None
 */
static void __nvic_irq_change_state(bool enable)
{
	if (enable)
		__RESETPRIMASK();
	else
		__SETPRIMASK();
}

/**
 * @brief	disable irq
 * @param	None
 * @return	None
 */
void nvic_disable_irq(void)
{
	__nvic_irq_change_state(false);
}

/**
 * @brief	enable irq
 * @param	None
 * @return	None
 */
void nvic_enable_irq(void)
{
	__nvic_irq_change_state(true);
}

/**
 * @brief	enable/disable irq by clearing/setting PRIMASK
 * @param	None
 * @return	None
 */
static void __nvic_fiq_change_state(bool enable)
{
	if (enable)
		__RESETFAULTMASK();
	else
		__SETFAULTMASK();
}

/**
 * @brief	disable fault-irq
 * @param	None
 * @return	None
 */
void nvic_disable_fiq(void)
{
	__nvic_fiq_change_state(false);
}

/**
 * @brief	enable fault-irq
 * @param	None
 * @return	None
 */
void nvic_enable_fiq(void)
{
	__nvic_fiq_change_state(true);
}

/**
 * @brief	sets the base priority mask register
 * @param	p the new priority to be applied
 * @return	None
 */
void nvic_set_basepri(uint32_t p)
{
	if (is_nvic_prio_valid(p))
		__BASEPRICONFIG(p << 0x04); /* prio is stored in MSB */
}

/**
 * @brief	gets the base priority mask register
 * @param	None
 * @return	the BASEPRI register
 */
uint32_t nvic_get_basepri(void)
{
	return (__GetBASEPRI());
}

/**
 * @brief	generates a reset
 * @param	type can be:
 *	- RESET_TYPE_CORE: to reset the core except debug components
 *	- RESET_TYPE_SYSTEM: to reset the system
 * @return	0 if no error, not 0 otherwise
 */
int nvic_trigger_reset(uint8_t type)
{
	uint32_t reg = scb_regs->aircr;

	reg |= (AIRC_VECTKEY_W << AIRCR_VECTKEY_SHIFT);

	if (type == RESET_TYPE_CORE)
		reg |= AIRCR_VECTRESET;
	else if (type == RESET_TYPE_SYSTEM)
		reg |= AIRCR_SYSRESETREQ;
	else
		return -EINVAL;

	scb_regs->aircr = reg;

	return 0;
}

/**
 * @brief	gets the current pending irq
 * @return	pending irq channel id
 */
uint32_t nvic_get_current_pending_irq(void)
{
	return (scb_regs->irq_ctrl_state & ISR_VECTPEND_MASK) >> 12;
}

/**
 * @brief	checks if the current irq is pending
 * @param	irq_chnl: to be checked
 * @return	true if irq is marked as pending, false otherwise
 */
bool nvic_is_irq_pending(struct nvic_chnl *irq_chnl)
{
	uint8_t id = irq_chnl->id;

	if (!is_nvic_irq_chnl_valid(id))
		return false;

	return (nvic_regs->set_pending[id >> 5] & BIT(id));
}


/**
 * @brief	changes the pending state of a given irq
 * @param	irq_chnl: irq to be set/cleared as pending
 * @param	set: to true if to be set as pending, to false to be cleared
 * @retval	0 if no error, not 0 otherwise
 */
static int __nvic_irq_change_pending_state(struct nvic_chnl *irq_chnl, bool set)
{
	uint8_t id = irq_chnl->id;

	if (!is_nvic_irq_chnl_valid(id))
		return -EINVAL;

	if (set)
		nvic_regs->set_pending[id >> 5] |= BIT(id);
	else
		nvic_regs->clear_pending[id >> 5] |= BIT(id);

	return 0;
}

/**
 * @brief	sets an irq as pending
 * @param	irq_chnl: irq to be set as pending
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_irq_set_pending(struct nvic_chnl *irq_chnl)
{
	return __nvic_irq_change_pending_state(irq_chnl, true);
}

/**
 * @brief	clears an irq as pending
 * @param	irq_chnl: irq to be cleared as pending
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_irq_clear_pending(struct nvic_chnl *irq_chnl)
{
	return __nvic_irq_change_pending_state(irq_chnl, false);
}

/**
 * @brief	gets the current irq
 * @return	current irq channel id
 */
uint32_t nvic_get_current_irq(void)
{
	return (scb_regs->irq_ctrl_state & ISR_VECTACT_MASK);
}

/**
 * @brief	gets the current irq
 * @param	irq_chnl: irq channel to be checked
 * @return	true if interrupt is active or pre-empter and stacked, false otherwise
 */
bool nvic_get_active_bit(struct nvic_chnl *irq_chnl)
{
	uint8_t id = irq_chnl->id;

	if (!is_nvic_irq_chnl_valid(id))
		return false;

	return (nvic_regs->active_bit[id >> 5] & BIT(id));
}
