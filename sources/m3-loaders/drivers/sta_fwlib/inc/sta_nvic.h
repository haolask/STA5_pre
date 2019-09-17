/**
 * @file sta_nvic.h
 * @brief Nested Vectored Interrupt Controller header file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __STA_NVIC_H__
#define __STA_NVIC_H__

#include "utils.h"

#include "FreeRTOS.h"

#include "sta_map.h"

/* well hmm, this value (15) represents the lowest priority (0) */
#define IRQ_MAX_PRIO_INDEX			configLIBRARY_KERNEL_INTERRUPT_PRIORITY
#define IRQ_LOW_PRIO				configLIBRARY_KERNEL_INTERRUPT_PRIORITY

#define PMU_IRQChannel          ((uint8_t)0x00)  /* PMU Interrupt */
#define SRC_IRQChannel          ((uint8_t)0x01)  /* SRC Interrupt */
#define MTU_IRQChannel          ((uint8_t)0x02)  /* MTU Interrupt*/
#define MAILBOX_IRQChannel      ((uint8_t)0x03)  /* Mailbox Interrupt */
#define CAN0_IRQChannel         ((uint8_t)0x04)  /* CAN0 Interrupt */
#define GPIO_IRQChannel         ((uint8_t)0x05)  /* GPIO Interrupt */
#define WDG_A7_IRQChannel       ((uint8_t)0x06)  /* A7 Watchdog Interrupt */
#define WDG_IRQChannel          ((uint8_t)0x07)  /* M3 Watchdog Interrupt */
#define SSP0_IRQChannel         ((uint8_t)0x08)  /* SSP0 Interrupt */
#define RTC_IRQChannel          ((uint8_t)0x09)  /* RTC Interrupt */
#define CAN1_IRQChannel         ((uint8_t)0x0A)  /* CAN1 Interrupt */
#define UART0_IRQChannel        ((uint8_t)0x0B)  /* UART 0 Interrupt */
#define GPIO_S_IRQChannel       ((uint8_t)0x0C)  /* GPIO S Interrupt */
#define EFT3_IRQChannel         ((uint8_t)0x0D)  /* EFT3 Interrupt */
#define EFT4_IRQChannel         ((uint8_t)0x0E)  /* EFT4  Interrupt */
#define I2C0_IRQChannel         ((uint8_t)0x0F)  /* I2C0 Interrupt */
#define EXT0_IRQChannel         ((uint8_t)0x10)  /* EXT0 Interrupt */
#define EXT1_IRQChannel         ((uint8_t)0x11)  /* EXT1 Interrupt */
#define EXT2_IRQChannel         ((uint8_t)0x12)  /* EXT2 Interrupt */
#define EXT3_IRQChannel         ((uint8_t)0x13)  /* EXT3 Interrupt */
#define EXT4_IRQChannel         ((uint8_t)0x14)  /* EXT4 Interrupt */
#define EXT5_IRQChannel         ((uint8_t)0x15)  /* EXT5 Interrupt */
#define EXT6_IRQChannel         ((uint8_t)0x16)  /* EXT6 Interrupt */
#define EXT7_IRQChannel         ((uint8_t)0x17)  /* EXT7 Interrupt */
#define EXT8_IRQChannel         ((uint8_t)0x18)  /* EXT8 Interrupt */
#define EXT9_IRQChannel         ((uint8_t)0x19)  /* EXT9 Interrupt */
#define EXT10_IRQChannel        ((uint8_t)0x1A)  /* EXT10 Interrupt */
#define EXT11_IRQChannel        ((uint8_t)0x1B)  /* EXT11 Interrupt */
#define EXT12_IRQChannel        ((uint8_t)0x1C)  /* EXT12 Interrupt */
#define EXT13_IRQChannel        ((uint8_t)0x1D)  /* EXT13 Interrupt */
#define EXT14_IRQChannel        ((uint8_t)0x1E)  /* EXT14 Interrupt */
#define EXT15_IRQChannel        ((uint8_t)0x1F)  /* EXT15 Interrupt */
#define C3_IRQChannel           ((uint8_t)0x21)  /* C3 Interrupt */

/* Low power */
#define NVIC_LP_SEVONPEND            0x10
#define NVIC_LP_SLEEPDEEP            0x04
#define NVIC_LP_SLEEPONEXIT          0x02

/* Priority groups definition */
#define NVIC_PRIORITY_GRP0         0x700 /* 0 bit for preemp prio 4 for subprio */
#define NVIC_PRIORITY_GRP1         0x600 /* 1 bit for preemp prio 3 for subprio */
#define NVIC_PRIORITY_GRP2         0x500 /* 2 bits for preemp prio 2 for subprio */
#define NVIC_PRIORITY_GRP3         0x400 /* 3 bits for preemp prio 1 for subprio */
#define NVIC_PRIORITY_GRP4         0x300 /* 4 bits for preemp prio 0 for subprio */

/* Reset type */
#define RESET_TYPE_CORE				0
#define RESET_TYPE_SYSTEM			1

/**
 * struct nvic_chnl - nvic channel structure
 * @id: channel id
 * @preempt_prio: channel preemption priority, value must be less than 16
 * @sub_prio: channel sub priority, value must be less than 16 (always set to 0)
 * @enabled: channel status
 */
struct nvic_chnl {
	uint8_t id;
	uint8_t preempt_prio;
	uint8_t sub_prio;
	bool enabled;
};

/**
 * struct cpu_info - CPU information structure
 * @revision: revision id
 * @part_number: cortex family, version, M and family member
 * @variant: channel sub priority, value must be less than 16 (always set to 0)
 * @implementer: implementer ID
 */
struct cpu_info {
	uint8_t revision;
	uint8_t part_number;
	uint8_t variant;
	uint8_t implementer;
};

/**
 * @brief  checks if a configuration is valid
 * @param  irq_chnl: configuration of the channel to be checked
 * @retval true if valid, false otherwise
 */
bool nvic_chnl_is_valid(struct nvic_chnl *irq_chnl);

/**
 * @brief  un-initializes nvic peripheral by resetting all interrupts bits
 * (interrupts and pending) as well as the corresponding priorities
 * @param  None
 * @retval None
 */
void nvic_deinit(void);

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
void nvic_scb_deinit(void);

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
int nvic_set_priority_grp_config(uint32_t group);

/**
 * @brief	enables the irq for a given channel (if not already), then sets
 * the enabled field to true.
 * @param	irq_chnl: configuration of the channel to be enabled
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_chnl_enable(struct nvic_chnl *irq_chnl);

/**
 * @brief	disables the irq for a given channel (if not already), then sets
 * the enabled field to false.
 * @param	irq_chnl: configuration of the channel to be enabled
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_chnl_disable(struct nvic_chnl *irq_chnl);

/**
 * @brief  initializes nvic peripheral
 * @param  irq_chnl: configuration of the channel to be initiliazed
 * @retval 0 if no error, not 0 otherwise
 */
int nvic_chnl_init(struct nvic_chnl *irq_chnl);

/**
  * @brief  read the priority group
  * @retval priority group from AIRCR
  */
uint32_t nvic_get_priority_grp_config(void);

/**
 * @brief  provides cpu informations such as revision, part number, variant
 * and implementer
 * @param  cpu: structure representation of a cpu
 * @retval 0 if no error, not 0 otherwise
 */
int nvic_cpuid(struct cpu_info *cpu);

/**
 * @brief	set the system control register low power mode
 * @param	mode: the mode to be set (sleep on exit, sleep deep or sev on
 * pending)
 * @param	enable: set to true when the Cortex-M3 clock can be stopped, false
 * otherwise
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_lp_mode_enable(uint8_t mode, bool enable);

/**
 * @brief	disable irq
 * @param	None
 * @return	None
 */
void nvic_disable_irq(void);

/**
 * @brief	enable irq
 * @param	None
 * @return	None
 */
void nvic_enable_irq(void);

/**
 * @brief	disable fault-irq
 * @param	None
 * @return	None
 */
void nvic_disable_fiq(void);

/**
 * @brief	enable fault-irq
 * @param	None
 * @return	None
 */
void nvic_enable_fiq(void);

/**
 * @brief	sets the base priority mask register
 * @param	p the new priority to be applied
 * @return	None
 */
void nvic_set_basepri(uint32_t p);

/**
 * @brief	gets the base priority mask register
 * @param	None
 * @return	the BASEPRI register
 */
uint32_t nvic_get_basepri(void);

/**
 * @brief	generates a reset
 * @param	type can be:
 *	- RESET_TYPE_CORE: to reset the core except debug components
 *	- RESET_TYPE_SYSTEM: to reset the system
 * @return	0 if no error, not 0 otherwise
 */
int nvic_trigger_reset(uint8_t type);

/**
 * @brief	gets the current pending irq
 * @return	pending irq channel id
 */
uint32_t nvic_get_current_pending_irq(void);

/**
 * @brief	checks if the current irq is pending
 * @param	irq_chnl: to be checked
 * @return	true if irq is marked as pending, false otherwise
 */
bool nvic_is_irq_pending(struct nvic_chnl *irq_chnl);

/**
 * @brief	sets an irq as pending
 * @param	irq_chnl: irq to be set as pending
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_irq_set_pending(struct nvic_chnl *irq_chnl);

/**
 * @brief	clears an irq as pending
 * @param	irq_chnl: irq to be cleared as pending
 * @retval	0 if no error, not 0 otherwise
 */
int nvic_irq_clear_pending(struct nvic_chnl *irq_chnl);

/**
 * @brief	gets the current irq
 * @return	current irq channel id
 */
uint32_t nvic_get_current_irq(void);

/**
 * @brief	gets the current irq
 * @param	irq_chnl: irq channel to be checked
 * @return	true if interrupt is active or pre-empter and stacked, false otherwise
 */
bool nvic_get_active_bit(struct nvic_chnl *irq_chnl);
#endif /* __STA_NVIC_H__ */

