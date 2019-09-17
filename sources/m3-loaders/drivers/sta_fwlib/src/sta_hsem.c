/**
 * @file sta_hsem.c
 * @brief Provide hardware semaphore definitions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "utils.h"

#include "sta_hsem.h"
#include "sta_nvic.h"
#include "trace.h"

#define HSEM_MAX_NUMBER     16
#define HSEM_ID_UNDEF       (HSEM_MAX_NUMBER + 1)
#define HSEM_M3_ID          (0x04)
#define HSEM_RESET          (0x0)	/* free */

/* Variables declaration */
static struct hsem_device v_hsem_dev;
static struct hsem_lock v_hsem_locks[HSEM_MAX_NUMBER];

/**
 * @brief Init HSEM driver
 *		- Allocate and init Hardware Semaphore bank structure
 *      - Enable HSEM interrupt IRQ_B if requested
 */
void hsem_init(void)
{
	struct nvic_chnl irq_chnl;

	/* Enable HSEM interrupt */
	/* WARNING: HSEM irq is connected to M3 external line */
	/* IRQ channel depends on the M3_IRQITF configuration */
	irq_chnl.id = EXT1_IRQChannel;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;

	nvic_chnl_init(&irq_chnl);

	/* Initialize HSEM device */
	v_hsem_dev.lock = &v_hsem_locks[0];

	/* Clean IRQ_B associated to HSEM M3 side  */
	hsem_regs->imscb = 0;
}

/**
 * @brief Allocate a HW Semaphore
 * The semaphore is exclusively allocated and can't be used by
 * another user before the owner calls hsem_lock_free
 * @param Pointer on HSEM request parameters :
 *      - lock_id: Optional Specific Semaphore requested by user.
 *              If set to HSEM_ID_UNDEF, 1st free semaphore index
 *              will be returned.
 *      - lock_cb: Optional atomic callback used by HSEM ISR.
 *              If not set, no IRQ will be triggered when HSEM is
 *              unlocked.
 * @return Pointer on HSEM allocated or NULL
 */
struct hsem_lock *hsem_lock_request(struct hsem_lock_req *lock_req)
{
	struct hsem_lock *lock;
	uint32_t i = 0;
	uint32_t nb = HSEM_FREE_MAX_ID;

	/* Specific HW Semaphore is requested */
	if (lock_req->id < HSEM_MAX_NUMBER) {
		i = lock_req->id;
		nb = lock_req->id + 1;
	}

	for (lock = v_hsem_dev.lock + i; i < nb; i++, lock++) {
		if (lock->assigned == false) {
			lock->assigned = true;
			lock->lock_id = i;

			/* IRQ management */
			if (lock_req->lock_cb != NULL) {
				/* Enable Rx IRQ */
				hsem_enable_irq(lock);

				/* Set callback */
				lock->lock_cb = lock_req->lock_cb;
				/* Set user cookie */
				lock->cookie = lock_req->cookie;

				/* IRQ will be generated to R4 when unlocked */
				lock->hsem_reset = HSEM_RESET | HSEM_IRQ_A;
			}

			TRACE_INFO
			    ("hsem_lock_request: HSEM [id:%d] allocated\n",
			     lock->lock_id);
			return lock;
		}
	}
	return NULL;
}

/**
 * @brief Free a HW Semaphore
 * @param Pointer on HSEM parameters to released
 */
void hsem_lock_free(struct hsem_lock *lock)
{
	if (!lock)
		return;

	/* Disable interrupt for the selected Semaphore */
	hsem_disable_irq(lock);

	TRACE_INFO("hsem_lock_free: HSEM [id:%d] released\n", lock->lock_id);

	/* Free HW semaphore */
	memset(lock, 0, sizeof(*lock));
}

/**
 * @brief lock requested HW Semaphore
 * @param Pointer on HW Semaphore structure
 * @return HSEM_BUSY or HSEM_LOCK
 */
int hsem_try_lock(struct hsem_lock *lock)
{
	uint32_t l;

	TRACE_ASSERT(lock);

	hsem_regs->rx[lock->lock_id] = HSEM_M3_ID;

	l = hsem_regs->rx[lock->lock_id];
	if (HSEM_M3_ID == (l & 0xF))
		return HSEM_LOCK;
	else
		return HSEM_BUSY;
}

/**
 * @brief infinite lock wait HW Semaphore
 * @param Pointer on HW Semaphore structure
 */
void hsem_lock(struct hsem_lock *lock)
{
	TRACE_ASSERT(lock);

	for (;;) {
		hsem_regs->rx[lock->lock_id] = HSEM_M3_ID;
		if ((hsem_regs->rx[lock->lock_id] & 0x0F) == HSEM_M3_ID)
			break;
	}
}

/**
 * @brief Unlock requested HW Semaphore
 * @param Pointer on HW Semaphore structure
 */
void hsem_unlock(struct hsem_lock *lock)
{
	TRACE_ASSERT(lock);

	hsem_regs->rx[lock->lock_id] = lock->hsem_reset;
}

/**
 * @brief Enable interrupt for the selected HW Semaphore
 * @param Pointer on HSEM parameters to released
 */
void hsem_enable_irq(struct hsem_lock *lock)
{
	if (lock)
		/* Enable interrupt for the selected Semaphore */
		hsem_regs->imscb |= BIT(lock->lock_id);
}

/**
 * @brief Disable interrupt for the selected HW Semaphore
 * @param Pointer on HSEM parameters to released
 */
void hsem_disable_irq(struct hsem_lock *lock)
{
	if (lock)
		/* Disable interrupt for the selected Semaphore */
		hsem_regs->imscb &= ~BIT(lock->lock_id);
}

/**
 * @brief Give the current status of the HSEM IRQ before masking
 *                  is applied
 * @param IRQ output requested: HSEM_IRQ_A or HSEM_IRQ_B
 */
uint32_t hsem_int_status(uint32_t irq_out)
{
	if (irq_out == HSEM_IRQ_A)
		return hsem_regs->risa;
	else
		return hsem_regs->risb;
}

/**
 * @brief HSEM ISR
 */
void hsem_irq_handler(void)
{
	uint32_t misb, imscb, msk, n;
	struct hsem_lock *lock;

	/* Get pending HSEM interrupts */
	misb = hsem_regs->misb;

	/* TRACE_INFO("hsem_irq_handler: Pending RISB:0x%x  MISB:0x%x  IMSCB:0x%x\n", HSEM->HSEM_RISB, misb, HSEM->HSEM_IMSCB); */

	/* Handle pending interrupt */
	for (n = 0; misb; n++) {
		msk = BIT(n);
		if (misb & msk) {
			misb -= msk;
			lock = v_hsem_dev.lock + n;
			imscb = hsem_regs->imscb;
			if (imscb & msk) {
				/* Call the Rx callback */
				if (lock->lock_cb != NULL)
					lock->lock_cb(lock->cookie);
			} else
				TRACE_ERR
				    ("hsem_irq_handler: Unexpected HSEM IRQ [id:%d]\n",
				     n);

			/* Clear IRQ */
			hsem_regs->icrb = msk;
		}
	}
	/* TRACE_INFO("hsem_irq_handler: Remaining RISB:0x%x  MISB:0x%x  IMSCB:0x%x\n", HSEM->HSEM_RISB, HSEM->HSEM_MISB, HSEM->HSEM_IMSCB); */

	return;
}

