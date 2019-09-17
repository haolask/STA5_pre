/*
 * STA Hardware Semaphore - public header
 *
 * Copyright (C) 2014 STMicroelectronics
 *
 * Contact: Philippe Langlais <philippe.langlais@st.com>
 * Contact: Olivier Lebreton <olivier.lebreton@st.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_STA_HSEM_H
#define __LINUX_STA_HSEM_H

#include <linux/err.h>
#include <linux/sched.h>

/* hwspinlock mode argument */
#define HSEM_ID_UNDEF			0xFF
#define HSEM_INTR4			0x1	/* IRQ_A to R4 */
#define HSEM_INTM3			0x2	/* IRQ_B to M3 */

/* Predefined static HSEM ID definition */
enum {
	HSEM_FREE_MAX_ID = 11,
	HSEM_SQI_NOR_M3_ID = HSEM_FREE_MAX_ID, /* M3 SQI NOR Access */
	HSEM_SQI_NOR_AP_ID,      /* AP SQI NOR Access */
	HSEM_NAND_ID,            /* AP-M3 NAND shared resource */
	HSEM_MMC_ID = HSEM_NAND_ID,  /* AP-M3 MMC shared resource */
	HSEM_CONSOLE_ID,	     /* UART Console shared access */
};

struct hwspinlock;

struct hwspinlock *sta_hsem_lock_request(unsigned int id,
					 int irqmode, void *priv,
					 void (*hsemcb)(void *priv,
							void *data));
int sta_hsem_lock_free(struct hwspinlock *hwlock);
int sta_hsem_irq_clr(struct hwspinlock *hwlock, u32 msk);
u32 sta_hsem_irq_status(struct hwspinlock *hwlock, unsigned int irq);

/**
 * sta_hsem_trylock() - attempt to lock a specific hwspinlock
 * @hwlock: an hwspinlock which we want to trylock
 *
 * This function attempts to lock an hwspinlock, and will immediately fail
 * if the hwspinlock is already taken.
 *
 * Upon a successful return from this function, preemption is disabled,
 * so the caller must not sleep, and is advised to release the hwspinlock
 * as soon as possible. This is required in order to minimize remote cores
 * polling on the hardware interconnect.
 *
 * Returns 0 if we successfully locked the hwspinlock, -EBUSY if
 * the hwspinlock was already taken, and -EINVAL if @hwlock is invalid.
 */
static inline int sta_hsem_trylock(struct hwspinlock *hwlock)
{
	return hwspin_trylock(hwlock);
}

/**
 * sta_hsem_unlock() - unlock hwspinlock
 * @hwlock: a previously-acquired hwspinlock which we want to unlock
 *
 * This function will unlock a specific hwspinlock and enable preemption
 * back.
 *
 * @hwlock must be already locked (e.g. by hwspin_trylock()) before calling
 * this function: it is a bug to call unlock on a @hwlock that is already
 * unlocked.
 */
static inline void sta_hsem_unlock(struct hwspinlock *hwlock)
{
	hwspin_unlock(hwlock);
}

#endif /* __LINUX_STA_HSEM_H */
