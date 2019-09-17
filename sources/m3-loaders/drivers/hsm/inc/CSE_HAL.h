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
 * @file    CSE_HAL.h
 * @brief   CSE abstraction layer module.
 * @details Allow to use a B3M CSE HW or an emulated CSE running on McK HSM,
 *          hiding the physical interface
 *
 *
 * @addtogroup CSE_driver
 * @{
 * @addtogroup HAL Hardware Abstraction Layer
 * @{
 */
#ifndef _CSE_HAL_H_
#define _CSE_HAL_H_

#include "cse_typedefs.h"

#include "cse_client.h"

#ifdef PERF_MEASURMENT
#include "os_hal.h"
extern uint32_t cmd_start;
extern uint32_t cmd_stop;
#endif

/*============================================================================*/
/**
 * @brief          Sends a command without parameter and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *                 CSE-HSM, then wait for completion
 *
 * @param[in]      CMD	Command identifier value
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *                 ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_HAL_send_cmd(uint32_t CMD);

/*============================================================================*/
/**
 * @brief          Sends a command with 1 parameter and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *                 CSE-HSM, then wait for completion
 *
 * @param[in]      CMD	Command identifier value
 * @param[in]      P1	Parameter 1
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *                 ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_HAL_send_cmd1p(uint32_t CMD, uint32_t P1);

/*============================================================================*/
/**
 * @brief          Sends a command with 2 parameters and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *                 CSE-HSM, then wait for completion
 *
 * @param[in]      CMD	Command identifier value
 * @param[in]      P1	Parameter 1
 * @param[in]      P2	Parameter 2
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *                 ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_HAL_send_cmd2p(uint32_t CMD, uint32_t P1, uint32_t P2);

/*============================================================================*/
/**
 * @brief          Sends a command with 3 parameters and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *                 CSE-HSM, then wait for completion
 *
 * @param[in]      CMD	Command identifier value
 * @param[in]      P1	Parameter 1
 * @param[in]      P2	Parameter 2
 * @param[in]      P3	Parameter 3
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *                 ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_HAL_send_cmd3p(uint32_t CMD, uint32_t P1, uint32_t P2,
				   uint32_t P3);

/*============================================================================*/
/**
 * @brief          Sends a command with 4 parameters and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *                 CSE-HSM,
 *				   then wait for completion
 *
 * @param[in]      CMD	Command identifier value
 * @param[in]      P1	Parameter 1
 * @param[in]      P2	Parameter 2
 * @param[in]      P3	Parameter 3
 * @param[in]      P4	Parameter 4
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *                 ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_HAL_send_cmd4p(uint32_t CMD, uint32_t P1, uint32_t P2,
				   uint32_t P3, uint32_t P4);

/*============================================================================*/
/**
 * @brief          Sends a command with 5 parameters and wait for answer
 * @details        Copy the params and CMD id, and notify there's a request for
 *                 CSE-HSM, then wait for completion
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
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *                 ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_HAL_send_cmd5p(uint32_t CMD, uint32_t P1, uint32_t P2,
				   uint32_t P3, uint32_t P4, uint32_t P5);

/*============================================================================*/
/**
 * @brief          eHSM update
 * @details        Apply some specific updates for eHSM FW
 */
uint32_t hsm_update(void);

/*============================================================================*/
/**
 * @brief          eHSM initialization
 * @details        Find the virtual address for mailbox resources and check
 *		   READY bit
 */
uint32_t hsm_init(void);

#endif //_CSE_HAL_H_

/**
 * @}
 * @}
 */

