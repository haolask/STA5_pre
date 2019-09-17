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
 * @file    CSE_cmd_param.h
 * @brief   Message parameters function.
 * @details Expose functions to check command parameters
 *
 *
 * @addtogroup UserAPI Single Call user level API
 * @{
 * @addtogroup API Functions
 * @{
 */

#ifndef _CSE_CMD_PARAM_H_
#define _CSE_CMD_PARAM_H_

#include "config.h"
#include "cse_typedefs.h"

#ifdef VERBOSE_UNALIGNED_BUF

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_1
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_1(uint32_t cmd, uint32_t p1);

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_2
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_2(uint32_t cmd, uint32_t p1, uint32_t p2);

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_3
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_3(uint32_t cmd, uint32_t p1,
				  uint32_t p2, uint32_t p3);

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_4
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 * @param  InputMessageLength: Parameter 4
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_4(uint32_t cmd, uint32_t p1, uint32_t p2,
				  uint32_t p3, uint32_t p4);

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_5
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 * @param  InputMessageLength: Parameter 4
 * @param  InputMessageLength: Parameter 5
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_5(uint32_t cmd, uint32_t p1, uint32_t p2,
				  uint32_t p3, uint32_t p4, uint32_t p5);

#endif /* VERBOSE_UNALIGNED_BUF */


#ifdef MEM_ALIAS_BUF

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_1
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_1(uint32_t cmd, uint32_t *p1);

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_2
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_2(uint32_t cmd, uint32_t *p1, uint32_t *p2);

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_3
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_3(uint32_t cmd, uint32_t *p1, uint32_t *p2,
			      uint32_t *p3);

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_4
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 * @param  InputMessageLength: Parameter 4
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_4(uint32_t cmd, uint32_t *p1, uint32_t *p2,
			      uint32_t *p3, uint32_t *p4);

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_5
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 * @param  InputMessageLength: Parameter 4
 * @param  InputMessageLength: Parameter 5
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_5(uint32_t cmd, uint32_t *p1, uint32_t *p2,
			      uint32_t *p3, uint32_t *p4, uint32_t *p5);

#endif /* MEM_ALIAS_BUF */

#endif //_CSE_CMD_PARAM_H_
