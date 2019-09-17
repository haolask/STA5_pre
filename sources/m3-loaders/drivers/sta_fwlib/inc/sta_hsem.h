/**
 * @file sta_hsem.h
 * @brief Provide hardware semaphore definitions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#ifndef _STA_HSEM_H_
#define _STA_HSEM_H_

#include "sta_type.h"

#define HSEM_BUSY           0
#define HSEM_LOCK           1
#define HSEM_ERROR          -1

#define HSEM_IRQ_A		(0x10)	/* IRQ to R4 */
#define HSEM_IRQ_B		(0x20)	/* IRQ to M3 */

/* Predefined static HSEM ID definition shared with AP */
enum {
	HSEM_FREE_MAX_ID = 11,
	HSEM_SQI_NOR_M3_ID = HSEM_FREE_MAX_ID, /* M3 SQI NOR Access */
	HSEM_SQI_NOR_AP_ID,      /* AP SQI NOR Access */
	HSEM_NAND_ID,            /* AP-M3 NAND shared resource */
	HSEM_MMC_ID = HSEM_NAND_ID,  /* AP-M3 MMC shared resource */
	HSEM_CONSOLE_ID,	     /* UART Console shared access */
};

/**
 * struct hsem_lock - HW Semaphore allocated by user
 * @lock_cb: atomic callback used by HSEM ISR if enabled
 * @cookie: optional pointer to user data put in argument of lock_cb()
 * @hsem_reset: Semaphore reset code to use
 * @lock_id: HSEM instance
 * @assigned: Flag set if HW Semaphore is assigned
 */
struct hsem_lock {
	void (*lock_cb) (void *cookie);
	void *cookie;
	uint32_t hsem_reset;
	uint32_t lock_id;
	bool assigned;
};

/**
 * struct hsem_device - HSEM device data
 * @NumHsem:	    number of HW Semaphores for the platform
 * @lock:		    array of HW Semaphore device attributes
 */
struct hsem_device {
	uint32_t NumHsem;
	struct hsem_lock *lock;
};

/**
 * struct hsem_lock_req - HW Semaphore request by user
 * @id:     Optional Specific Semaphore requested by user
 *          if set to HSEM_ID_UNDEF, 1st free semaphore index will be returned
 * @lock_cb: Optional atomic callback used by HSEM ISR
            if not set, no IRQ will be triggered when HSEM is released
 * @cookie: optional pointer to user data put in argument of lock_cb()
 */
struct hsem_lock_req {
	uint32_t id;
	void (*lock_cb) (void *cookie);
	void *cookie;
};

/**
 * @brief Init HSEM driver
 *		- Allocate and init Hardware Semaphore bank structure
 *      - Enable HSEM interrupt IRQ_B if requested
 */
void hsem_init(void);

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
struct hsem_lock *hsem_lock_request(struct hsem_lock_req *lock_req);

/**
 * @brief Free a HW Semaphore
 * @param Pointer on HSEM parameters to released
 */
void hsem_lock_free(struct hsem_lock *lock);

/**
 * @brief lock requested HW Semaphore
 * @param Pointer on HW Semaphore structure
 * @return HSEM_BUSY or HSEM_LOCK
 */
int hsem_try_lock(struct hsem_lock *lock);

/**
 * @brief infinite lock wait HW Semaphore
 * @param Pointer on HW Semaphore structure
 */
void hsem_lock(struct hsem_lock *lock);

/**
 * @brief Unlock requested HW Semaphore
 * @param Pointer on HW Semaphore structure
 */
void hsem_unlock(struct hsem_lock *lock);

/**
 * @brief Unlock requested HW Semaphore
 * @param Pointer on HW Semaphore structure
 */
void hsem_unlock(struct hsem_lock *lock);

/**
 * @brief Enable interrupt for the selected HW Semaphore
 * @param Pointer on HSEM parameters to released
 */
void hsem_enable_irq(struct hsem_lock *lock);

/**
 * @brief Disable interrupt for the selected HW Semaphore
 * @param Pointer on HSEM parameters to released
 */
void hsem_disable_irq(struct hsem_lock *lock);

/**
 * @brief Give the current status of the HSEM IRQ before masking
 *                  is applied
 * @param IRQ output requested: HSEM_IRQ_A or HSEM_IRQ_B
 */
uint32_t hsem_int_status(uint32_t irq_out);

/**
 * @brief HSEM ISR
 */
void hsem_irq_handler(void);

#endif /* _STA_HSEM_H_ */
