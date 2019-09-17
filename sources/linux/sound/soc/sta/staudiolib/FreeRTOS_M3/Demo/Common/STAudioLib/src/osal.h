/***********************************************************************************/
/*!
*
*  \file      osal.h
*
*  \brief     <i><b> STAudioLib OS abstraction layer </b></i>
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

#ifndef OSAL_H_
#define OSAL_H_


//note:
//osal.h defines OS specific stuff, internally, when building the STAudioLib, while
//sta_platforms.h defines OS specific stuff exposed to the user with the STA API

//sta_platforms.h: defines the target OS and basic types.
//note: we keep this header separate so to include it also in the user side header: staudio.h
#include "sta_platforms.h"


//lint -estring(*, dbg_printf, dbg_sprintf)   //inhibits misra2004 16.1(960):"variable number of arguments"
//lint -estring(9022, STA_PRINTF, STA_SPRINTF, STA_ERR) //Can't put brackets arround 'fmt' because it triggers compilation error
//lint -esym(960, e9022, STA_PRINTF, STA_SPRINTF, STA_ERR) inhibits misra2004 19.10 "unparenthesized macro parameter" because of __VA_ARGS__
//lint -esym(960, e9022, INLINE)		//misra2004 19.4 can't put parenthesis around 'inline'


//--- FreeRTOS -------------------------------
#ifdef FREE_RTOS

//#include <stdio.h>			//printf()
#include <stdlib.h>				//exit()
#include <string.h> 			//memset()
//#include <assert.h>			//assert()

#include <FreeRTOS.h>
#include <task.h>       		//vTaskDelay()
#include <timers.h>
#include <macros.h>				//first def of assert()

extern int dbg_printf(const char *, ...);
extern int dbg_sprintf(char *out, const char *format, ...);

#define INLINE                  inline

#define STA_MALLOC				(pvPortMalloc)
#define STA_FREE				(vPortFree)

#define STA_SLEEP(ms)			(vTaskDelay(ms))
#define STA_GETTIME()			(xTaskGetTickCount() * portTICK_RATE_MS)

#define STA_PRINTF(fmt,...)			do {(void)dbg_printf((fmt), __VA_ARGS__);} while(0)
#define STA_SPRINTF(out, fmt,...)	do {(void)dbg_sprintf((out), (fmt), __VA_ARGS__);} while(0)
#define STA_ERR(fmt,...)			do {(void)dbg_printf((fmt), __VA_ARGS__);} while(0)

#ifdef DEBUG
#define sta_assert(x)			do {if (!(x)) {STA_PRINTF("assert failed at %s:%d\n", __FILE__, __LINE__); exit(0);}} while(0)
#else
#define sta_assert(x)			do {} while(0)
#endif

//---  TKERNEL ----------------------------------
#elif defined TKERNEL

#include <strings.h>
#include <stdio.h>				//printf()
#include <stdlib.h>				//exit()
#include <string.h> 			//memset()
#include <assert.h>				//assert()

#define INLINE                  __inline

#define STA_MALLOC				(malloc)
#define STA_FREE				(free)

#define STA_SLEEP(ms)			((void)tk_dly_tsk(ms))

#define STA_PRINTF(fmt,...)			do {printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__);} while(0)
#define STA_SPRINTF(out, fmt,...)	do {sprintf((out), (fmt), __VA_ARGS__);} while(0)
#define STA_ERR(fmt,...)			do {printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__);} while(0)


#ifdef DEBUG
#define sta_assert(x)			do {if (!(x)) {STA_PRINTF("assert failed at %s:%d\n", __FILE__, __LINE__); exit(0);}} while(0)
#else
#define sta_assert(x)			do {} while(0)
#endif

static INLINE s64 STA_GETTIME(void);
static INLINE s64 STA_GETTIME(void)
{
	SYSTIM_U    time;
	UINT    	ofs;
	(void)tk_get_otm_u(&time,&ofs);
	return (s64)time;
}

//---  LINUX ----------------------------------
#elif defined LINUX_OS
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/delay.h>

#define INLINE                  inline

#define STA_MALLOC(...)			kmalloc( __VA_ARGS__, GFP_KERNEL)
#define STA_FREE				kfree

#define STA_SLEEP(ms)			usleep_range((ms)*1000, (ms)*1000)

//in ms
#define STA_GETTIME()			ktime_to_ms(ktime_get())

#define __SHORTFILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#define STA_PRINTF(fmt,...)		\
	do {dev_info((struct device *)g_drv.m_dev, "%s:%d: " fmt, __SHORTFILE__, __LINE__, __VA_ARGS__);} while(0)
#define STA_SPRINTF(out, fmt,...)	do {sprintf((out), (fmt), __VA_ARGS__);} while(0)
#define STA_ERR(fmt,...)		\
	dev_err((struct device *)g_drv.m_dev, "%s:%d: " fmt, __SHORTFILE__, __LINE__, __VA_ARGS__)

#define sta_assert(x) 			do { if (!(x)) BUG(); } while (0)


//---  Other OS -------------------------------
#else //other OS

#include <stdio.h>				//printf()
#include <stdlib.h>				//exit()
#include <string.h> 			//memset()
#include <assert.h>				//assert()

#define STA_MALLOC				malloc
#define STA_FREE				free

#define STA_SLEEP(ms)			usleep((ms)*1000)

#define STA_PRINTF(fmt,...)		printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)
#define STA_ERR(fmt,...)		printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__)


#ifndef assert
#ifdef DEBUG
#define assert(x)				if (!(x)) {STA_PRINTF("assert failed at %s:%d\n", __FILE__, __LINE__); exit(0);}
#else
#define assert(x)
#endif
#endif


//---------------------------------------------
#endif //other OS



#endif // OSAL_H_
