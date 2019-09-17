/*
    CSE-SHE - Copyright (C) 2015 STMicroelectronics

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    memory.h
 * @brief   Memory mapping
 *
 * @addtogroup MEMORY
 * @{
 */
#ifndef _MEMORY_H_
#define _MEMORY_H_

//#include "platform.h"
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
#define HSM_START_RAM        0xA0000000U
#define HSM_RAM_SIZE         0x0000A000U
#define HSM_END_RAM          (HSM_START_RAM + HSM_RAM_SIZE -1 )

#define HSM_START_AREA       0xA0000000U
#define HSM_AREA_SIZE        0x00010000U
#define HSM_AREA_END         (HSM_START_AREA + HSM_AREA_SIZE -1 )

#define HSM_START_CODEFLASH  0x00600000U
#define HSM_CODEFLASH_SIZE   0x00030000U        /* Populated flash area */
#define HSM_END_CODEFLASH    (HSM_START_CODEFLASH + HSM_CODEFLASH_SIZE -1 )

#define HSM_START_DATAFLASH  0x00680000U
#define HSM_DATAFLASH_SIZE   0x00008000U        /* Populated flash area */
#define HSM_END_DATAFLASH    (HSM_START_DATAFLASH + HSM_DATAFLASH_SIZE -1 )



#define HSM_START_FLASHAREA  0x00600000U
#define HSM_FLASHAREA_SIZE   0x00200000U     /*     Reserved section area, not all populated  */
#define HSM_END_FLASHAREA    (HSM_START_FLASHAREA + HSM_FLASHAREA_SIZE -1 )


/*
 * MEMORY range definition to check buffers are in existing memories
 */
/* 256KB data flash */
#define DATAFLASH_START         0x00800000U
#define DATAFLASH_END           0x0083FFFFU

#ifdef McK2_PLATFORM_CSE
/* 256 KB Low & Mid blocks, 3.5MB Large blocks */
#define FLASHLOWMIDLARGE_START     0x00FC0000U
#define FLASHLOWMIDLARGE_END       0x0137FFFFU

/* 256 KB Low & Mid blocks, 3.5MB Large blocks */
#define FLASHLOWMIDLARGE_OVERLAY_START 0x08FC0000U
#define FLASHLOWMIDLARGE_OVERLAY_END   0x0937FFFFU

/* System RAM 128KB */
#define SYSTEM_RAM_START        0x40000000U
#define SYSTEM_RAM_END           0x4001FFFFU
#endif


#ifdef _SPC58NExx_        /* Eiger */
/* 256 KB Low & Mid blocks, 5MB Large blocks */
#define FLASHLOWMIDLARGE_START     0x00FC0000U
#define FLASHLOWMIDLARGE_END       0x015BFFFFU

/* 256 KB Low & Mid blocks, 5MB Large blocks */
#define FLASHLOWMIDLARGE_OVERLAY_START 0x08FC0000U
#define FLASHLOWMIDLARGE_OVERLAY_END   0x095BFFFFU

/* System RAM */
#if defined(_SPC58NE84C3_CUT1_)
/* 640KB for cut 1 */
#define SYSTEM_RAM_START        0x40058000U
#else
/* 608KB for cut 2 */
#define SYSTEM_RAM_START        0x40060000U
#endif
#define SYSTEM_RAM_END          0x400F7FFFU
#endif

/* CORE0 and CORE1 ISRAM : 16KB, DSRAM : 64KB */
#define CORE0_ISRAM_START         0x50000000U
#define CORE0_ISRAM_END           0x50003FFFU
#define CORE0_DSRAM_START         0x50800000U
#define CORE0_DSRAM_END           0x5080FFFFU

#define CORE1_ISRAM_START         0x51000000U
#define CORE1_ISRAM_END           0x51003FFFU
#define CORE1_DSRAM_START         0x51800000U
#define CORE1_DSRAM_END           0x5180FFFFU


#ifdef McK2_PLATFORM_CSE
/* CORE2 ISRAM : 16KB DSRAM : 64KB */
#define CORE2_ISRAM_START         0x52000000U
#define CORE2_ISRAM_END           0x52003FFFU

/* 64 KBytes */
#define CORE2_DSRAM_START         0x52800000U
#define CORE2_DSRAM_END           0x5280FFFFU
#endif

#ifdef _SPC58NExx_        /* Eiger */
/* CORE2 ISRAM : 16KB DSRAM : 32KB */
#define CORE2_ISRAM_START         0x52000000U
#define CORE2_ISRAM_END           0x52003FFFU

#define CORE2_DSRAM_START         0x52800000U
#define CORE2_DSRAM_END           0x52807FFFU
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* _MEMORY_H_ */

/** @} */
