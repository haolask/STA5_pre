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

#ifndef CSE_SHARED_EVENT_H
#define CSE_SHARED_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Include Files                                                             */
/*===========================================================================*/

#include "cse_typedefs.h"
#include "config.h"

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

#define NEW_REQ_MASK    ((uint32_t)0x00000001UL)
#define NEW_ANS_MASK    ((uint32_t)0x00000001UL)
#define HSM_READY_MASK  ((uint32_t)0x00004000UL)

#define TRAP_ERROR_MASK	((uint32_t)0x00000002UL)
#define KS_WARNING_MASK ((uint32_t)0x00000004UL)

#define UNRECOVERABLE_FLASH_ERR ((uint32_t)20U)
#define WRONG_SILICON_VERSION   ((uint32_t)0x0015U)		/* 21 */

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
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* CSE_SHARED_EVENT_H */
