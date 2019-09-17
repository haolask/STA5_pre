/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file lib_string.h																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains platform related API's declarations and Macro definitions		*
*																											*
*																											*
*************************************************************************************************************/

#ifndef LIB_BITMANIP_H_
#define LIB_BITMANIP_H_

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_SETBIT
/// This macro sets a bit at the given position i in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_SETBIT(x, i)   ((x) | (1u << (i)))

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_CLEARBIT
/// This macro clears a bit at the given position i in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_CLEARBIT(x, i)   ((x) & ~(1u << (i)))

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_TOGGLEBIT
/// This macro inverts a bit at the given position i in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_TOGGLEBIT(x, i)   ((x) ^ (1u << (i)))

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_ISBITSET
/// This macro return true if a bit is set at the given position i in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_ISBITSET(x, i)   (((x) & (1u << (i))) != 0)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_SET_MASKBIT
/// This macro sets a bit at the given mask value in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_SET_MASKBIT(x, mask)   (x | mask)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_CLEAR_MASKBIT
/// This macro clears a bit at the given mask in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_CLEAR_MASKBIT(x, mask)   (x & ~( mask))

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_TOGGLE_MASKBIT
/// This macro inverts a bit at the given mask in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_TOGGLE_MASKBIT(x, mask)   (x ^= mask)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_ISANY_MASKBITSET
/// This macro return true if a any bit is set at the given mask in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_ISANY_MASKBITSET(x, mask)   ((x & mask) != 0)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_ISALL_MASKBITSET
/// This macro return true if all bits are set at the given mask in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_ISALL_MASKBITSET(x, mask)   ((x & mask) == mask)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_CONCATENATE
/// This macro will concatenate x and y, after shifting x by n data bits
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_CONCATENATE(x, n, y)  (( x << n) | y)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_MASK_AND_SHIFT
/// This macro will mask x with mask bits and shift the result by n data bits
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_MASK_AND_SHIFT(x, mask, n)  ((x & mask) >> n)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_SHIFTLEFT
/// This macro does the shift left operation in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_SHIFTLEFT(x, val)   (x << val)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_SHIFTRIGHT
/// This macro does the shift right operation in the given argument x.
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_SHIFTRIGHT(x, val)   (x  >> val)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_AND
/// This macro does the bitwise AND operation
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_AND(x, y)   (x & y)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_OR
/// This macro does the bitwise OR operation
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_OR(x, y)   (x | y)

////////////////////////////////////////////////////////////////////////////////////////////////
/// LIB_XOR
/// This macro does the bitwise XOR operation
///////////////////////////////////////////////////////////////////////////////////////////////////
#define LIB_XOR(x, y)   (x ^ y)


#endif /* LIB_BITMANIP_H_ */
/*=====================================================================================================
    end of file
======================================================================================================*/
