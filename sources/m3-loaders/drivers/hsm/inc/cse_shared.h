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
 * @file    cse_shared.h
 * @brief   CSE interface module with structure definition and masks
 *          - shared between client and server
 * @details This file is common to host and CSE
 *          Besides the interface register structure definition,
 *          it also provide masks used in the request and answer notification
 *          and synchronization mechanisms
 *
 */

/*
 * parasoft suppress item MISRA2012-RULE-19_2 "union are used to map hw register
 * bitfields, some are automatically generated from design database"
 */

#ifndef CSE_SHARED_H
#define CSE_SHARED_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Include Files                                                             */
/*===========================================================================*/

#include "cse_typedefs.h"

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

/****************************************************************************/
/*          MODULE : CSE (Cryptographic Security Engine)                    */
/****************************************************************************/
/**
 * @brief Status Register bitfield structure, used for internal status and SR
 *	  reported to host
 */
union status_flags_ut {
	uint32_t R;
	struct {
		uint32_t BSY:1;
		uint32_t SB:1;
		uint32_t BIN:1;
		uint32_t BFN:1;
		uint32_t BOK:1;
		uint32_t RIN:1;
		uint32_t EDB:1;
		uint32_t IDB:1;
		uint32_t EX:1;
		uint32_t:22;
		uint32_t READY:1;
	} B;
};

/**
 * @brief Status Register bitfield structure, used for internal status and
 *	  SR reported to host
 */
struct CSE_HSM_tag {
	union { /* CSE Control (Base+0x0000) */
		vuint32_t R;
		struct {
			vuint32_t:16;
			vuint32_t DIV:8;	/* unused */
			vuint32_t:4;
			vuint32_t MDIS:1;	/* unused ? */
			vuint32_t SUS:1;	/* unused */
			vuint32_t:1;		/* unused */
			vuint32_t CIE:1;	/* unused */
		} B;
	} CR;

	union status_flags_ut SR; /* CSE Status (Base+0x0004) */

	union { /* CSE Error Code (Base+0x0008) */
		vuint32_t R;
		struct {
			vuint32_t:27;
			vuint32_t EC:5;
		} B;
	} ECR;

	union { /* CSE Command (Base+0x000C) */
		vuint32_t R;
		struct {
			vuint32_t:24;
			vuint32_t CMD:8;
		} B;
	} CMD;

	union { /* CSE Parameter 1 (Base+0x0010) */
		vuint32_t R;
		struct {
			vuint32_t PARM:32;
		} B;
	} P1;

	union { /* CSE Parameter 2 (Base+0x0014) */
		vuint32_t R;
		struct {
			vuint32_t PARM:32;
		} B;
	} P2;

	union { /* CSE Parameter 3 (Base+0x0018) */
		vuint32_t R;
		struct {
			vuint32_t PARM:32;
		} B;
	} P3;

	union { /* CSE Parameter 4 (Base+0x001C) */
		vuint32_t R;
		struct {
			vuint32_t PARM:32;
		} B;
	} P4;

	union { /* CSE Parameter 5 (Base+0x0020) */
		vuint32_t R;
		struct {
			vuint32_t PARM:32;
		} B;
	} P5;
}; /* end of CSE_tag */

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* CSE_SHARED_H */
