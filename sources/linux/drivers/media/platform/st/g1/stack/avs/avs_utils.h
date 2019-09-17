/*
 * (C) COPYRIGHT 2011 HANTRO PRODUCTS
 *
 * Please contact: hantro-support@verisilicon.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

/*------------------------------------------------------------------------------
--
--  Abstract : Header file for stream decoding utilities
--
------------------------------------------------------------------------------*/

#ifndef STRMDEC_UTILS_H_DEFINED
#define STRMDEC_UTILS_H_DEFINED

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "avs_container.h"
/*
#ifdef _ASSERT_USED
#include <assert.h>
#endif
*/
#ifdef _UTEST
#include <stdio.h>
#endif

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

/* constant definitions */
#ifndef OK
#define OK 0
#endif

#ifndef NOK
#define NOK 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

/* decoder states */
enum
{
    STATE_OK,
    STATE_NOT_READY,
    STATE_SYNC_LOST
};

#define HANTRO_OK 0
#define HANTRO_NOK 1

#ifndef NULL
#define NULL 0
#endif

/* picture structure */
#define FIELDPICTURE 0
#define FRAMEPICTURE 1

/* Error concealment */
#define FREEZED_PIC_RDY 1

/* start codes */
enum
{
    SC_SLICE = 0x00,    /* throug AF */
    SC_SEQUENCE = 0xB0,
    SC_SEQ_END = 0xB1,
    SC_USER_DATA = 0xB2,
    SC_I_PICTURE = 0xB3,
    SC_EXTENSION = 0xB5,
    SC_PB_PICTURE = 0xB6,
    SC_NOT_FOUND = 0xFFFE,
    SC_ERROR = 0xFFFF
};

/* start codes */
enum
{
    SC_SEQ_DISPLAY_EXT = 0x02
};

enum
{
    IFRAME = 1,
    PFRAME = 2,
    BFRAME = 3
};

enum
{
    OUT_OF_BUFFER = 0xFF
};

/* value to be returned by GetBits if stream buffer is empty */
#define END_OF_STREAM 0xFFFFFFFFU

#ifdef _DEBUG_PRINT
#define AVSDEC_DEBUG(args) DEBUG_PRINT(args)
#else
#define AVSDEC_DEBUG(args)
#endif

/* macro to check if stream ends */
#define IS_END_OF_STREAM(pContainer) \
    ( (pContainer)->StrmDesc.strmBuffReadBits == \
      (8*(pContainer)->StrmDesc.strmBuffSize) )

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

u32 AvsStrmDec_GetBits(DecContainer *, u32 numBits);
u32 AvsStrmDec_ShowBits(DecContainer *, u32 numBits);
u32 AvsStrmDec_ShowBits32(DecContainer *);
u32 AvsStrmDec_ShowBitsAligned(DecContainer *, u32 numBits, u32 numBytes);
u32 AvsStrmDec_FlushBits(DecContainer *, u32 numBits);
u32 AvsStrmDec_UnFlushBits(DecContainer *, u32 numBits);

u32 AvsStrmDec_NextStartCode(DecContainer *);

u32 AvsStrmDec_NumBits(u32 value);
u32 AvsStrmDec_CountLeadingZeros(u32 value, u32 len);

#endif
