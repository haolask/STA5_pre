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
--  Abstract : Utility macros for debugging and tracing
--
------------------------------------------------------------------------------*/

#ifndef __H264DEBUG_H__
#define __H264DEBUG_H__

/* macro for assertion, used only when _ASSERT_USED is defined */
#ifdef _ASSERT_USED
#ifndef ASSERT
#include <assert.h>
#define ASSERT(expr) assert(expr)
#endif
#else
#define ASSERT(expr)
#endif

/* macros for range checking used only when _RANGE_CHECK is defined */
#ifdef _RANGE_CHECK

#include <stdio.h>

/* macro for range checking an single value */
#define RANGE_CHECK(value, minBound, maxBound) \
{ \
    if ((value) < (minBound) || (value) > (maxBound)) \
        fprintf(stderr, "Warning: Value exceeds given limit(s)!\n"); \
}

/* macro for range checking an array of values */
#define RANGE_CHECK_ARRAY(array, minBound, maxBound, length) \
{ \
    i32 i; \
    for (i = 0; i < (length); i++) \
        if ((array)[i] < (minBound) || (array)[i] > (maxBound)) \
            fprintf(stderr,"Warning: Value [%d] exceeds given limit(s)!\n",i); \
}

#else /* _RANGE_CHECK */

#define RANGE_CHECK_ARRAY(array, minBound, maxBound, length)
#define RANGE_CHECK(value, minBound, maxBound)

#endif /* _RANGE_CHECK */

/* macro for debug printing, used only when _DEBUG_PRINT is defined */
#ifdef _DEBUG_PRINT
#ifndef DEBUG_PRINT
#include <stdio.h>
#define DEBUG_PRINT(args) printf args
#endif
#else
#define DEBUG_PRINT(args)
#endif

/* macro for error printing, used only when _ERROR_PRINT is defined */
#ifdef _ERROR_PRINT
#ifndef ERROR_PRINT
#include <stdio.h>
#define ERROR_PRINT(msg) fprintf(stderr,"ERROR: %s\n",msg)
#endif
#else
#define ERROR_PRINT(msg)
#endif

#endif /* __H264DEBUG_H__ */
