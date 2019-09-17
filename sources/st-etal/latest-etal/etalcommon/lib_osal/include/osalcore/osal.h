//!
//!  \file 		osal.h
//!  \brief 	<i><b>OSAL top header file</b></i>
//!  \details	This file is the top header one and its main purpose
//!             is to include the other os*.h files and contains some
//!             significant define. Include this one and you'll be happy.
//!
#ifndef OSAL_H
#define OSAL_H

//#define OSAL_LINUX		1
//#define OSAL_TKERNEL 	2
//#define OSAL_OS21		3
#define OSAL_FREERTOS 	4

//----------------------------------------------------------------------
// includes
//----------------------------------------------------------------------
#include "target_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>

#ifdef CONFIG_HOST_OS_LINUX
	#include <sys/socket.h>
	#include <arpa/inet.h>

//	#define OSAL_OS	OSAL_LINUX
#elif defined(CONFIG_HOST_OS_TKERNEL)
	// nothing specific

//	#define OSAL_OS OSAL_TKERNEL

#elif defined (CONFIG_HOST_OS_FREERTOS)
	#define OSAL_OS OSAL_FREERTOS
	#include "FreeRTOS.h"

#else
	#include <winsock2.h>
#endif

#include "linux_osal.h"

#include "types.h"
#include "tri_types.h"
#include "osdefine.h"
#include "ostypes.h"
#include "ostime.h"
#include "osutilio.h"
#include "osansi.h"
#include "oserror.h"
#include "ossmphr.h"
#include "osproc.h"
#include "osmemory.h"
#include "osevent.h"


#ifdef __cplusplus
extern "C"
{
#endif

extern tSInt OSAL_INIT (void);
extern tSInt OSAL_DEINIT (void);

#ifdef __cplusplus
}
#endif

#define OSAL_C_U32_TRACE_MAX_MESSAGESIZE        (384)

#else
#error osal.h included several times
#endif

// END OF FILE

