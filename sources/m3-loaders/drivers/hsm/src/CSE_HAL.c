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
 * @file    CSE_HAL.c
 * @brief   CSE abstraction layer module.
 * @details Allow to use a B3M CSE HW or an emulated CSE running on HSM,
 *          hiding the physical interface
 *
 *
 * @addtogroup CSE_driver
 * @{
 * @addtogroup HAL Hardware Abstraction Layer
 * @{
 */
#include "sta_common.h"

#include "config.h"
#include "CSE_Constants.h"
#include "cse_ext_tc3p_patch.h"
#include "CSE_Key.h"
#include "cse_typedefs.h"
#include "serialprintf.h"

#ifdef CSE_HSM
#include "cse_client.h"
#include "CSE_HAL.h"
#include "CSE_ext_ECC.h"

#ifdef PERF_MEASURMENT
#include "pit_perf_meas.h"

struct timestamp_t cmd_start_ts;
struct timestamp_t cmd_stop_ts;

uint32_t delta_timer_ticks;

uint32_t microseconds;
uint32_t nanoseconds;

uint32_t perf_verbose;
#endif /* PERF_MEASURMENT */

#if (defined(VERBOSE) || defined(VERBOSE_UNALIGNED_BUF) \
	|| defined(PERF_MEASURMENT)) || defined(MEM_ALIAS_BUF)
#include "CSE_cmd_param.h"
#endif
/* VERBOSE || VERBOSE_UNALIGNED_BUF || PERF_MEASURMENT || MEM_ALIAS_BUF */

#if defined ESRAM_M3_HSM_MAILBOXES_BASE
static uint32_t hsm_ready_mailbox_notification_received;
#endif

volatile struct context_stt ehsm_context = {0};
volatile struct CSE_HSM_tag* CSE;

#define TIMEOUT     1
#define NO_TIMEOUT  0
#define TIMEOUT_MS  60000	/* 1 min timeout */

/*============================================================================*/
/**
 * @brief	  Tells if HSM ready notification is set in the status register
 * @details	  Used by the host to know if HSM is ready to accept commands,
 *                 refreshed by HSM each time SR is updated
 *
 * @return         0 if not ready, 1 if ready
 */
static inline uint32_t hsm_ready_from_SR(void)
{
	return CSE->SR.B.READY;
}

/*============================================================================*/
/**
 * @brief	  Tells if HSM busy notification is set in the status register
 * @details	  Used by the host to know if HSM is busy
 *
 * @return         0 if not busy, 1 if busy
 */
static inline uint32_t hsm_busy_from_SR(void)
{
	return CSE->SR.B.BSY;
}

/*============================================================================*/
/**
 * @brief          This function waits for the notification via the mailbox that
 *                 HSM is ready
 *
 * @details        For further checks, better to rely on the READY bit in the
 *                 mailbox.
 *
 */
void wait_for_hsm_ready(void)
{
	while (!hsm_ready_from_mailbox())
		;
}

#ifdef ANSWER_NOTIFICATION_BY_ISR
volatile uint32_t answer_available;
#endif

/*============================================================================*/
/**
 * @brief          This function notifies a new request is available
 *
 * @details        Used by the Host to notify HSM that a new request is
 *		   available.
 */
static inline void CSE_HSM_notify_new_req(void)
{
#ifdef PERF_MEASURMENT
	PIT_perf_get_timestamp(&cmd_start_ts);
#endif

	notify_new_request_available();
}

#ifdef ANSWER_NOTIFICATION_BY_ISR
/*============================================================================*/
/**
 * @brief          HSM2HT bit 0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(vector527) {
	OSAL_IRQ_PROLOGUE();

	osalSysLockFromISR();
#ifdef PERF_MEASURMENT
	PIT_perf_get_timestamp(&cmd_stop_ts);
#endif

	/* Clear IRQ flag */
	HSM_HOST_IF.HSM2HTF.R |= NEW_ANS_MASK;

	/* wait until CSE is idle, means command execution is done */
	while (hsm_busy_from_SR())
		;

	/* Notify app there's an answer */
	answer_available = 1;

	osalSysUnlockFromISR();
	OSAL_IRQ_EPILOGUE();
}
#endif /* ANSWER_NOTIFICATION_BY_ISR */


/*============================================================================*/
/**
 * @brief          This function waits until CSE is idle, means command
 *		   execution is done.
 *
 * @details        Used by the Host to manage the answer of HSM
 */
static inline uint32_t CSE_HSM_wait_until_idle(void)
{
	uint32_t timeout = TIMEOUT_MS;

	/* wait until CSE is idle, means command execution is done */
	while (hsm_busy_from_SR() && timeout)
	{
		ehsm_timout_handling(2, &timeout);
	}
	if (!timeout)
		return TIMEOUT;

	return NO_TIMEOUT;
}

/*============================================================================*/
/**
 * @brief          This function waits the answer from HSM.
 *
 * @details        Used by the Host to manage the answer of HSM
 */
static inline uint32_t CSE_HSM_wait_for_answer(void)
{
#ifdef ANSWER_NOTIFICATION_BY_ISR
	/* wait until we're notified by the ISR */
	while (answer_available == 0)
		;

	/* Clear notification variable */
	answer_available = 0;

#else /* ANSWER_NOTIFICATION_BY_ISR */

	uint32_t timeout = TIMEOUT_MS;

	/* wait until CSE is idle, means command execution is done */
	if (CSE_HSM_wait_until_idle() == TIMEOUT)
		return TIMEOUT;

	/* wait for answer to be ready  */
	while (!new_answer_available() && timeout)
	{
		ehsm_timout_handling(2, &timeout);
	}
	if (!timeout)
		return TIMEOUT;

	/* Clear bit */
	clear_new_answer_flag();

#endif /* ANSWER_NOTIFICATION_BY_ISR */

#ifdef PERF_MEASURMENT

#ifndef ANSWER_NOTIFICATION_BY_ISR
	PIT_perf_get_timestamp(&cmd_stop_ts);
#endif

	if (perf_verbose) {
		/* Raw values */
		printf(" cmd_start(cnt:0x%08x,offset:0x%08x) ",
		       cmd_start_ts.H, cmd_start_ts.L);
		printf(" cmd_stop (cnt:0x%08x,offset:0x%08x)\n",
		       cmd_stop_ts.H, cmd_stop_ts.L);

		printf("resolution : 0x%x\n", PIT_perf_MSB_resolution());

		/*printf("Request notification -> answer delta time: ");*/
		delta_timer_ticks = PIT_perf_net_delta_ticks(&cmd_start_ts,
							     &cmd_stop_ts);
		printf("Elapsed time between Load request and answer :    ");
		printf(" %d (0x%x) ticks @%dMHz - %d ns (%d µs)\n",
		       delta_timer_ticks, delta_timer_ticks, PIT_freq,
		       PIT_perf_elapsed_nanosec(delta_timer_ticks),
		       PIT_perf_elapsed_microsec(delta_timer_ticks));
	}
#endif	/* PERF_MEASURMENT */

	return NO_TIMEOUT;
}
#endif /* CSE_HSM */

/*============================================================================*/
/**
 * @brief          Sends a command without parameter and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request
 *		   for CSE-HSM, then wait for completion
 *
 * @param[in]      CMD	Command identifier value
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE
 *		   returned ones
 *
 * @note           The function is blocking and waits for operation completion
 */
inline uint32_t CSE_HAL_send_cmd(uint32_t CMD)
{
	uint32_t ret = CSE_GENERAL_ERR;

	/* Wait until CSE is idle, to accept a new service request */
	if (CSE_HSM_wait_until_idle() == NO_TIMEOUT) {
		/* Common part, whatever the implementation */
		CSE->CMD.R = CMD;

#ifdef ANSWER_NOTIFICATION_BY_ISR
		answer_available = 0;
#endif

		/* notify HSM that a new request is available */
		CSE_HSM_notify_new_req();

		/* Now wait for command completion */
		if (CSE_HSM_wait_for_answer() == NO_TIMEOUT) {
			/* Test error code */
			ret = CSE->ECR.R;
		}
	}

	return ret;
}

/*============================================================================*/
/**
 * @brief          Sends a command with 1 parameter and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *		   CSE-HSM, then wait for completion
 *
 * @param[in]      CMD  Command identifier value
 * @param[in]      P1   Parameter 1
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_HAL_send_cmd1p(uint32_t CMD, uint32_t P1)
{
#ifdef VERBOSE_UNALIGNED_BUF
	if (unaligned_param_buffer_1(CMD, P1))
		printf(" !!! Unaligned parameter buffer\n");
#endif
#ifdef MEM_ALIAS_BUF
	mem_alias_param_buffer_1(CMD, &P1);
#endif /* MEM_ALIAS_BUF */
#ifdef VERBOSE
	printf("CMD : 0x%08x\n", CMD);
	printf("P1  : 0x%08x\n", P1);
#endif /* VERBOSE */

	CSE->P1.R = P1;
	return CSE_HAL_send_cmd(CMD);
}

/*============================================================================*/
/**
 * @brief          Sends a command with 2 parameters and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *		   CSE-HSM, then wait for completion
 *
 * @param[in]      CMD  Command identifier value
 * @param[in]      P1   Parameter 1
 * @param[in]      P2	Parameter 2
 *
 * @return         Error code
 * @retval 0       When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE
 *		   returned ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_HAL_send_cmd2p(uint32_t CMD, uint32_t P1, uint32_t P2)
{
#ifdef VERBOSE_UNALIGNED_BUF
	if (unaligned_param_buffer_2(CMD, P1, P2))
		printf(" !!! Unaligned parameter buffer\n");
#endif
#ifdef MEM_ALIAS_BUF
	mem_alias_param_buffer_2(CMD, &P1, &P2);
#endif /* MEM_ALIAS_BUF */
#ifdef VERBOSE
	printf("CMD : 0x%08x\n", CMD);
	printf("P1  : 0x%08x\n", P1);
	printf("P2  : 0x%08x\n", P2);
#endif /* VERBOSE */

	/* Common part, whatever the implementation */
	CSE->P1.R = P1;
	CSE->P2.R = P2;
	return CSE_HAL_send_cmd(CMD);
}

/*============================================================================*/
/**
 * @brief          Sends a command with 3 parameters and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *		   CSE-HSM, then wait for completion
 *
 * @param[in]      CMD	Command identifier value
 * @param[in]      P1	Parameter 1
 * @param[in]      P2	Parameter 2
 * @param[in]      P3	Parameter 3
 *
 * @return         Error code
 * @retval 0       When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_HAL_send_cmd3p(uint32_t CMD, uint32_t P1, uint32_t P2,
			    uint32_t P3)
{
#ifdef VERBOSE_UNALIGNED_BUF
	if (unaligned_param_buffer_3(CMD, P1, P2, P3))
		printf(" !!! Unaligned parameter buffer\n");
#endif
#ifdef MEM_ALIAS_BUF
	mem_alias_param_buffer_3(CMD, &P1, &P2, &P3);
#endif /* MEM_ALIAS_BUF */
#ifdef VERBOSE
	printf("CMD : 0x%08x\n", CMD);
	printf("P1  : 0x%08x\n", P1);
	printf("P2  : 0x%08x\n", P2);
	printf("P3  : 0x%08x\n", P3);
#endif /* VERBOSE */

	/* Common part, whatever the implementation */
	CSE->P1.R = P1;
	CSE->P2.R = P2;
	CSE->P3.R = P3;
	return CSE_HAL_send_cmd(CMD);
}

/*============================================================================*/
/**
 * @brief          Sends a command with 4 parameters and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *		   CSE-HSM, then wait for completion
 *
 * @param[in]      CMD	Command identifier value
 * @param[in]      P1	Parameter 1
 * @param[in]      P2	Parameter 2
 * @param[in]      P3	Parameter 3
 * @param[in]      P4	Parameter 4
 *
 * @return         Error code
 * @retval 0       When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_HAL_send_cmd4p(uint32_t CMD, uint32_t P1, uint32_t P2,
			    uint32_t P3, uint32_t P4)
{
#ifdef VERBOSE_UNALIGNED_BUF
	if (unaligned_param_buffer_4(CMD, P1, P2, P3, P4))
		printf(" !!! Unaligned parameter buffer\n");
#endif
#ifdef MEM_ALIAS_BUF
	mem_alias_param_buffer_4(CMD, &P1, &P2, &P3, &P4);
#endif /* MEM_ALIAS_BUF */
#ifdef VERBOSE
	printf("CMD : 0x%08x\n", CMD);
	printf("P1  : 0x%08x\n", P1);
	printf("P2  : 0x%08x\n", P2);
	printf("P3  : 0x%08x\n", P3);
	printf("P4  : 0x%08x\n", P4);
#endif /* VERBOSE */

	/* Common part, whatever the implementation */
	CSE->P1.R = P1;
	CSE->P2.R = P2;
	CSE->P3.R = P3;
	CSE->P4.R = P4;
	return CSE_HAL_send_cmd(CMD);
}

/*============================================================================*/
/**
 * @brief          Sends a command with 5 parameters and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *		   CSE-HSM, then wait for completion
 *
 * @param[in]      CMD	Command identifier value
 * @param[in]      P1	Parameter 1
 * @param[in]      P2	Parameter 2
 * @param[in]      P3	Parameter 3
 * @param[in]      P4	Parameter 4
 * @param[in]      P5	Parameter 5
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE
 *		   returned ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_HAL_send_cmd5p(uint32_t CMD, uint32_t P1, uint32_t P2,
			    uint32_t P3, uint32_t P4, uint32_t P5)
{
#ifdef VERBOSE_UNALIGNED_BUF
	if (unaligned_param_buffer_5(CMD, P1, P2, P3, P4, P5))
		printf(" !!! Unaligned parameter buffer\n");
#endif
#ifdef MEM_ALIAS_BUF
	mem_alias_param_buffer_5(CMD, &P1, &P2, &P3, &P4, &P5);
#endif /* MEM_ALIAS_BUF */
#ifdef VERBOSE
	printf("CMD : 0x%08x\n", CMD);
	printf("P1  : 0x%08x\n", P1);
	printf("P2  : 0x%08x\n", P2);
	printf("P3  : 0x%08x\n", P3);
	printf("P4  : 0x%08x\n", P4);
	printf("P5  : 0x%08x\n", P5);
#endif /* VERBOSE */

	/* Common part, whatever the implementation */
	CSE->P1.R = P1;
	CSE->P2.R = P2;
	CSE->P3.R = P3;
	CSE->P4.R = P4;
	CSE->P5.R = P5;
	return CSE_HAL_send_cmd(CMD);
}

/*============================================================================*/
/**
 * @brief          eHSM update
 * @details        Apply some specific updates for eHSM FW
 */
uint32_t hsm_update(void)
{
	uint32_t ret = CSE_NO_ERR;

	/* Apply patches for TC3P CUT2-BF version */
	if (get_cut_rev() == CUT_22) {
		ret = CSE_HAL_send_cmd2p(CSE_EXT_UPDATE_SERVICE_SET,
					 (uint32_t)t3p_hsm_patch_ecdsa_2bf,
					 sizeof(t3p_hsm_patch_ecdsa_2bf));

		if (ret == CSE_NO_ERR) {
			ret = CSE_HAL_send_cmd(0);

			if (ret == CSE_NO_ERR) {
				ret = CSE_HAL_send_cmd2p(
					CSE_EXT_UPDATE_SERVICE_SET,
					(uint32_t)t3p_hsm_patch_ecc_regen_2bf,
					sizeof(t3p_hsm_patch_ecc_regen_2bf));
			}
		}

		if (ret == CSE_NO_ERR) {
			union EXTENDED_KEY_flags ec_flags;

			/* By default fill the ECC RAM key (slot 0) */
			ec_flags.R = 0;
			ec_flags.B.sign = 1;
			ec_flags.B.verify = 1;
			ret = CSE_ECC_generateLoadKeyPair(0, ec_flags.R);
		}

		if (ret == CSE_NO_ERR) {
			ret = CSE_HAL_send_cmd2p(CSE_EXT_UPDATE_SERVICE_SET,
				(uint32_t)t3p_hsm_patch_m2_size_fix_2bf,
				sizeof(t3p_hsm_patch_m2_size_fix_2bf));
		}

		if (ret == CSE_NO_ERR) {
			ret = CSE_HAL_send_cmd2p(CSE_EXT_UPDATE_SERVICE_SET,
				(uint32_t)t3p_hsm_patch_ecdsa_digest_2bf,
				sizeof(t3p_hsm_patch_ecdsa_digest_2bf));

			if (ret == CSE_NO_ERR)
				(void)CSE_HAL_send_cmd(0x70);
		}
	}

	return ret;
}

/*============================================================================*/
/**
 * @brief          eHSM initialization
 * @details        Find the virtual address for mailbox resources and check
 *		   READY bit
 */
uint32_t hsm_init(void)
{
#if defined ESRAM_M3_HSM_MAILBOXES_BASE
	uint32_t timeout_expired = 0U;

	/* Get M3 base addresses */
	ehsm_context.CSE_IF = ESRAM_M3_HSM_MAILBOXES_BASE;
	ehsm_context.MBOX_M3 = 0x48230000U;

	/* Init timer for timeout handling */
	ehsm_context.ms_delay = NULL;

	CSE = (volatile struct CSE_HSM_tag*)&SHE_REGS0_REMOTE;

	/*
	 * If first command to be sent, we need to check if HSM is ready
	 * in the MAILBOX to avoid race conditions when accessing the emulated
	 * status register
	 */
	if (hsm_ready_mailbox_notification_received == 0U) {
		while (!timeout_expired) {
			if (hsm_ready_from_mailbox()) {
				hsm_ready_mailbox_notification_received = 1U;

				/* exit the while loop */
				break;
			}
		}
	}
#endif /* ESRAM_M3_HSM_MAILBOXES_BASE */

	return CSE_NO_ERR;
}

/**
 * @}
 * @}
 */

