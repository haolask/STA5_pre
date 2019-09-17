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
 * @file    she_registers.h
 * @brief   SHE registers header file.
 *
 * @addtogroup SHE_REGISTERS
 * @{
 */

#ifndef _SHE_REGISTERS_H_
#define _SHE_REGISTERS_H_

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*
 * Addresses should be aligned with emulated register set exposed to M3 core
 * in HSM side
 */
#define SHE_REGS0_REMOTE_BASE (ehsm_context.CSE_IF)

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Mailbox 0 enable switch.
 */
#if !defined(SHE_REGS_USE_REGISTERS0) || defined(__DOXYGEN__)
#define SHE_REGS_USE_REGISTERS0              1
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if SHE_REGS_USE_REGISTERS0 || defined(__DOXYGEN__)
#define SHE_REGS0_REMOTE (*((struct she_registers_t *)SHE_REGS0_REMOTE_BASE))
#endif

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a SHE status register definition.
 */
union she_status_register_t {
	uint32_t R;
	struct {
		uint32_t:23;
		uint32_t EX:1;
		uint32_t IDB:1;
		uint32_t EDB:1;
		uint32_t RIN:1;
		uint32_t BOK:1;
		uint32_t BFN:1;
		uint32_t BIN:1;
		uint32_t SB:1;
		uint32_t BSY:1;
	} B;
};

/**
 * @brief   Type of a SHE control register definition.
 */
union she_control_register_t {
	uint32_t R;
	struct {
		uint32_t:16;
		uint32_t DIV:8;
		uint32_t:4;
		uint32_t MDIS:1;
		uint32_t SUS:1;
		uint32_t:1;
		uint32_t CIE:1;
	} B;
};

/**
 * @brief   Type of a SHE error code register definition.
 */
union she_error_code_register_t {
	uint32_t R;
	struct {
		uint32_t:27;
		uint32_t EC:5;
	} B;
};

/**
 * @brief   Type of a SHE command register definition.
 */
union she_command_register_t {
	uint32_t R;
	struct {
		uint32_t:27;
		uint32_t CMD:5;
	} B;
};

/**
 * @brief   Type of a SHE mailbox.
 */
struct she_registers_t {
	union she_control_register_t        CR;
	union she_status_register_t         SR;
	union she_error_code_register_t     ECR;
	union she_command_register_t        CMD;
	uint32_t                      P[5];
};

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the boolean value of the SR.BSY flag.
 * @note    This operation is only meaningful on the remote mailbox.
 */
#define sheGetBusy(regsp) ((regsp)->SR.B.BSY != 0U)

/**
 * @brief   Sets the value of the SR.BSY flag.
 * @note    This operation is only meaningful on the remote mailbox.
 */
#define sheSetBusy(regsp, flag) ((regsp)->SR.B.BSY = (flag))

/**
 * @brief   Sets the value of the BSY flag.
 * @note    This operation is only meaningful on the remote mailbox.
 */
#define sheSetError(regsp, err) ((regsp)->ECR.R = (err))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
#if SHE_REGS_USE_REGISTERS0
extern struct she_registers_t SHE_REGS0;
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#endif /* _SHE_REGISTERS_H_ */

/** @} */
