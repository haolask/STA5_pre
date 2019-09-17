//!
//!  \file 		ossmphr.h
//!  \brief 	<i><b> OSAL Semaphores-Functions Header File </b></i>
//!  \details	This is the headerfile for the OS - Abstraction Layer for
//!             Semaphores. This Header has to be included to
//!             use the functions for semaphores
//!  \author 	Luca Pesenti
//!  \author 	(original version) Giovanni Di Martino
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef OSSMPHR_H
#define OSSMPHR_H

 #ifdef __cplusplus
 extern "C"
 {
 #endif

//----------------------------------------------------------------------
// includes of component-internal interfaces
//----------------------------------------------------------------------
#if (OSAL_OS==OSAL_LINUX) || (OSAL_OS==OSAL_TKERNEL) //lint !e553 - MISRA 19.11 - not defined but not a problem
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#elif (OSAL_OS==OSAL_FREERTOS)
#include <fcntl.h>
#include <sys/stat.h>
#include "semphr.h"
#endif

//-----------------------------------------------------------------------
//typedefs and struct defs (scope: global)
//-----------------------------------------------------------------------
//!
//! \typedef The OSAL Semaphore Handle Type
//!
typedef int OSAL_tSemHandle;

//-----------------------------------------------------------------------
// function prototypes (scope: global)
//-----------------------------------------------------------------------
extern tSInt Linux_initSemaphores(void);
extern tSInt Linux_deinitSemaphores(void);
extern tS32 OSAL_s32SemaphoreCreate (tCString coszName, OSAL_tSemHandle * phSemaphore, tU32 uCount);
#ifndef OSAL_SEMAPHORE_SKIP_NAMES
    extern tS32 OSAL_s32SemaphoreOpen (tCString coszName, OSAL_tSemHandle * phSemaphore);
    extern tS32 OSAL_s32SemaphoreClose (OSAL_tSemHandle hSemaphore);
    extern tS32 OSAL_s32SemaphoreDelete(tCString coszName);
#else
    extern tS32 OSAL_s32SemaphoreFree(OSAL_tSemHandle hSid);
#endif
extern tS32 OSAL_s32SemaphoreGetValue (OSAL_tSemHandle hSemaphore, tPS32 ps32Value);
extern tS32 /*@alt void@*/ OSAL_s32SemaphorePost (OSAL_tSemHandle hSemaphore);
extern tS32 /*@alt void@*/ OSAL_s32SemaphoreWait(OSAL_tSemHandle hSemaphore,OSAL_tMSecond msec);

#ifdef __cplusplus
}
#endif

#else
#error osshmpr.h included several times
#endif // OSSMPHR_H


// EOF
