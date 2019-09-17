/***********************************************************************************/
/*!
*
*  \file      mathFixed.h
*
*  \brief     <i><b> STAudioLib maths functions in fixed point </b></i>
*
*  \details
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2013/04/22
*
*  \bug       see Readme.txt
*
*  \warning
*
*  This file is part of STAudioLib and is dual licensed,
*  ST Proprietary License or GNU General Public License version 2.
*
********************************************************************************
*
* Copyright (c) 2014 STMicroelectronics - All Rights Reserved
* Reproduction and Communication of this document is strictly prohibited
* unless specifically authorized in writing by STMicroelectronics.
* FOR MORE INFORMATION PLEASE READ CAREFULLY THE LICENSE AGREEMENT LOCATED
* IN THE ROOT DIRECTORY OF THIS SOFTWARE PACKAGE.
*
********************************************************************************
*
* ALTERNATIVELY, this software may be distributed under the terms of the
* GNU General Public License ("GPL") version 2, in which case the following
* provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* STAudioLib is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* STAudioLib is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*
*/
/***********************************************************************************/

#ifndef _MATH_FIXED_H
#define _MATH_FIXED_H

#ifdef __cplusplus
extern "C" {
#endif


//NOTE: this FIXED representation is not matching the EMERALD fraction (where 1 = 0x7FFFFF)

#define FRACTION_BASE   24

#define FRAC_0_5		0x00800000
#define FRAC_1          ( 1 << FRACTION_BASE )
#define FRAC_2          ( 2 << FRACTION_BASE )
#define FRAC_4          ( 4 << FRACTION_BASE )
#define FRAC_PI         0x03243f6B
#define FRAC_SQRT2      0x016a09e6


//This macro has to be used only on const values to avoid using float at runtime.
//(hence this macro is used only by the compiler)
#define FRAC_(f)		((s32)((f) * FRAC_1 + 0.5))


typedef s32 fixpoint;


fixpoint F_DIV(fixpoint num, fixpoint den);
fixpoint F_MUL(fixpoint a, fixpoint b);

fixpoint F_SIN(fixpoint a);		//a must be in -PI to PI
fixpoint F_COS(fixpoint a);		//a must be in -PI to PI

fixpoint F_SQRT(fixpoint a);

fixpoint F_DBToLinear(s32 db);	//db in 1/128 (1<<7)


#ifdef __cplusplus
}
#endif

#endif //_MATH_FIXED_H

