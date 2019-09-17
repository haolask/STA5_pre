//!
//!  \file    osal_replace.h
//!  \brief   <i><b> Include file for builds without OSAL </b></i>
//!  \details Usable only for TUNER_DRIVER builds, to remove dependency from OSAL
//!  \author  Raffaele Belardi
//!

/*
 * include in alternative to osal.h for CONFIG_APP_TUNERDRIVER_LIBRARY
 */

#ifndef OSAL_REPLACE_H
#define OSAL_REPLACE_H

#if defined (CONFIG_APP_TUNERDRIVER_LIBRARY) && !defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>  /* for usleep() */
#include <stdarg.h>  /* for va_start() */
#include <time.h>    /* for clock_gettime */
#include "types.h"
#ifdef CONFIG_DEBUG_OSAL
	#include <assert.h>
	#define ASSERT_ON_DEBUGGING(a) (assert(a))
#else
	#define ASSERT_ON_DEBUGGING(a) ((void)(a))
#endif

/*
 * Definitions from OSAL's osdefine.h
 */
#define OSAL_ERROR_INVALID_PARAM ((tS32) -3)
#define OSAL_ERROR_DEVICE_NOT_OPEN ((tS32) -5)

/*
 * Utilities for build without OSAL on Posix system; customize for other Operating Systems
 */
#define OSAL_pvMemorySet(_d_, _s_, _size_)  memset((_d_), (_s_), (_size_))
#define OSAL_pvMemoryCopy(_d_, _s_, _size_) memcpy((_d_), (_s_), (_size_))
#define OSAL_pvMemoryAllocate(_size_)       malloc((_size_))
#define OSAL_vMemoryFree(_b_)               free((_b_))
#define OSAL_s32ThreadWait(_s_)             usleep((_s_) * 1000)
#define OSAL_s32ThreadWait_us(_s_)          usleep((_s_))
#define OSAL_s32NPrintFormat(...)           snprintf(__VA_ARGS__)

#endif // CONFIG_APP_TUNERDRIVER_LIBRARY
#endif // OSAL_REPLACE_H
