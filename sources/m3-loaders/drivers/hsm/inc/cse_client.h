/*
 *  Copyright (C) 2014 STMicroelectronics
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @file    cse_client.h
 * @brief   CSE interface base address definition + notification functions,
 *	    client side
 * @details This file is dedicated to client side (ie. the application Z4 core),
 *	    and includes the common structure definition file shared between
 *	    client and server
 *	    Besides the interface register structure definition, it also provide
 *	    inline functions to ease the request and answer notification and
 *	    synchronization
 *
 */

/*
 * parasoft suppress item MISRA2012-RULE-19_2 "union are used to map hw register
 * bitfields, some are automatically generated from design database"
 */

#ifndef CSE_CLIENT_H
#define CSE_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Include Files                                                             */
/*===========================================================================*/

#include "config.h"
#include "cse_typedefs.h"

#include "cse_shared.h"
#include "she_registers.h"

#include "cse_shared_event.h"
#include "registers.h"

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/
struct context_stt {
	volatile uint32_t MBOX_M3;
	volatile uint32_t CSE_IF;
	void (*ms_delay) (uint32_t mtime);
};

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
extern volatile struct context_stt ehsm_context;
extern volatile struct CSE_HSM_tag* CSE;

#define MBOX		MBOX_M3_HSM
#define MS_DELAY	ehsm_context.ms_delay

/**
 * @brief          eHSM timout handling
 *
 * @return         No return value
 *
 */
static inline void ehsm_timout_handling(uint32_t mtime, uint32_t *timeout)
{
	if (MS_DELAY) {
		MS_DELAY(mtime);
		*timeout -= mtime;
	}
}

/**
 * @brief          Notifies a request is available
 * @details        Used by the Host to tell CSE a new request description is
 *		  available
 *
 * @return         No return value
 *
 */
static inline void notify_new_request_available(void)
{
	MBOX->IRSR = NEW_REQ_MASK;
}

/**
 * @brief          Tells if a request is available
 * @details        Used by the CSE to check is a new request description is
 *		   available
 *
 * @return         No return value
 *
 */
static inline uint32_t new_request_available(void)
{
	/* Not possible to read IRSR, always return 0 */
	return((MBOX->IRSR & NEW_REQ_MASK) == NEW_REQ_MASK);
}

/**
 * @brief          Tells if a answer is available
 * @details        Used by the Host to check is a new answer is available
 *
 * @return         No return value
 *
 */
static inline uint32_t new_answer_available(void)
{
	return((MBOX->IPSR & NEW_ANS_MASK) == NEW_ANS_MASK);
}

/**
 * @brief          Tells if HSM ready notification is set in the mailbox
 * @details        Used by the host to know if HSMis ready to accept commands
 *
 * @return         0 if not ready, 1 if ready
 *
 */
static inline uint32_t hsm_ready_from_mailbox(void)
{
	return((MBOX->IPSR & HSM_READY_MASK) == HSM_READY_MASK);
}

/**
 * @brief          Clears the answer available flag
 * @details        Used by the Host to clear the flag
 *                 (CSE does not wait for an acknowledge / clearing of this bit)
 *
 * @return         No return value
 *
 */
static inline void clear_new_answer_flag(void)
{
	/* Need to write a 0 to clear bit in the IPSR register */
	MBOX->IPSR = ~NEW_ANS_MASK;
}

#ifdef __cplusplus
}
#endif

#endif /* CSE_CLIENT_H */
