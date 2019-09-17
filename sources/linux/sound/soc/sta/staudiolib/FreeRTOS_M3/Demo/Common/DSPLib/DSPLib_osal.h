/***********************************************************************************/
/*!
*  \file      DSPLib_osal.h
*
*  \brief     <i><b> dummy dsp lib </b></i>
*
*  \details   dummy dsp lib
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2015/07/21
*
*
*  This file is part of <component name> and is dual licensed,
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
* <component_name> is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* <component_name> is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*
*/
/***********************************************************************************/

#include <stdlib.h>			//exit()
#include <string.h> 		//memset()
//#include <assert.h>		//NOTE: assert() seems not implemented in emIDE/RTOS/ARM


//--- FreeRTOS ----
#ifdef FREE_RTOS
#include "FreeRTOS.h"   	//required before including "task.h"
#include "task.h"       	//vTaskDelay()
#include "timers.h"

#include "sta10xx_lib.h"	//lld, maps...
#include "sta10xx_type.h"

#define PRINTF				(void)dbg_printf
#define SLEEP(ms)			vTaskDelay(ms)
//---  Others -----
#else //not FREE_RTOS
#define PRINTF				printf
#define SLEEP(ms)			{int ret = usleep((ms)*1000); if (ret != 0) exit(0);}
#endif
//----------------

#ifdef DEBUG
#define ASSERT(x)			if (!(x)) {int ret = PRINTF("assert failed at %s:%d\n", __FILE__, __LINE__); if (ret == 0) exit(EXIT_SUCCESS); else exit(EXIT_FAILURE);}
#else
#define ASSERT(x)
#endif

#ifndef assert
#ifdef DEBUG
#define assert(x)			if (!(x)) {int ret = PRINTF("assert failed at %s:%d\n", __FILE__, __LINE__); if (ret == 0) exit(EXIT_SUCCESS); else exit(EXIT_FAILURE);}
#else
#define assert(x)
#endif
#endif

