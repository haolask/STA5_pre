/***********************************************************************************/
/*!
*
*  \file      sta_platform.h
*
*  \brief     <i><b> Public header included by STAudio.h for platform specific types </b></i>
*
*  \details
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

#ifndef STA_PLATFORM_H_
#define STA_PLATFORM_H_
/*
#ifdef __cplusplus
extern "C" {
#endif
*/

//other way to differentiate when compiling for PC/Win32 is check for WIN32
//#define STA_PC_MINGW32

//#define LINUX_OS
//#define FREE_RTOS
//#define TKERNEL

//#define DEBUG


#include "sta10xx_type.h"		//u32..., bool,  no float!


//=== RTOS ==============================================================
#ifdef FREE_RTOS

//#define STA_NO_FLOAT			0	//<- set in project build option
#define STA_WITH_DMABUS			1


//=== TKERNEL ===========================================================
#elif defined TKERNEL

#include <tk/tkernel.h>

#define STA_NO_FLOAT			0
#define STA_WITH_DMABUS			1
#define ACCORDO2 				1


//=== LINUX =============================================================
#elif defined LINUX_OS

#define STA_NO_FLOAT			1
#define STA_WITH_DMABUS			0


//=== ELSE ==============================================================
#else
#error "Which OS ?!"
#endif


//=== Common ============================================================
//Common so far...

#if STA_NO_FLOAT
typedef s32 					f32; //frac signed 0.24 bit
typedef s64						f64;
#else
typedef float					f32;
typedef double					f64;
#endif

/*
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif
*/

#define STA_API					extern

/*
#ifdef __cplusplus
}
#endif
*/

#endif // STA_PLATFORM_H_
