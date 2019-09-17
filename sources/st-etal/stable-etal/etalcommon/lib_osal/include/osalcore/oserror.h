//!
//!  \file 		oserror.h
//!  \brief 	<i><b> OSAL Error-Functions Header File </b></i>
//!  \details	This is the headerfile for the OS - Abstraction Layer for
//!             Error-Functions. This Header has to be included to use
//!             functions for Error-Handling
//!  \author 	Luca Pesenti
//!  \author 	(original version) Giovanni Di Martino
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef OSERROR_H
#define OSERROR_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------
//defines and macros (scope: global)
//-----------------------------------------------------------------------
//!
//! \def OSAL error definition
//!
#define OSAL_C_COMPONENT	(((tU32)0x07)<<16)
#define OSAL_C_ERROR_TYPE   (((tU32)0x02)<<12)
 //all error code in the form:  0111 0010 0000 0000 00xx
// not error
#define OSAL_E_NOERROR		         ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|0)
// function call not allowed
#define OSAL_E_NOPERMISSION	         ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|1)
// wrong parameter
#define OSAL_E_INVALIDVALUE          ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|2)
// no more ressources(memory)
#define OSAL_E_NOSPACE               ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|3)
// ressource is in use
#define OSAL_E_BUSY                  ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|4)
// interrupt during wait
#define OSAL_E_INTERRUPT             ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|5)
// no acces rights
#define OSAL_E_NOACCESS              ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|6)
// ressource already exists
#define OSAL_E_ALREADYEXISTS         ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|7)
// ressource doesn't exist
#define OSAL_E_DOESNOTEXIST          ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|8)
// to much descriptors in use
#define OSAL_E_MAXFILES              ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|9)
// no more resources(filedescr.)
#define OSAL_E_NOFILEDESCRIPTOR      ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|10)
// name for file or descr. too long
#define OSAL_E_NAMETOOLONG           ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|11)
// not existing filedescriptor
#define OSAL_E_BADFILEDESCRIPTOR     ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|12)
// message is to long
//#define OSAL_E_MSGTOOLONG            ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|13)
// message queue is full
#define OSAL_E_QUEUEFULL             ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|14)
// thread doesn't exist
#define OSAL_E_WRONGTHREAD           ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|15)
// a task is already waiting for event
#define OSAL_E_EVENTINUSE            ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|16)
// function doesn't exist
#define OSAL_E_WRONGFUNC             ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|17)
// function call is not allowed
#define OSAL_E_NOTINTERRUPTCALLABLE  ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|18)
// function is not ready
#define OSAL_E_INPROGRESS            ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|19)
// function abort because of timeout
#define OSAL_E_TIMEOUT               ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|20)
// service not supported
#define OSAL_E_NOTSUPPORTED          ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|21)
// unknown error
#define OSAL_E_UNKNOWN               ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|22)
// thread creation failed
#define OSAL_E_THREAD_CREATE_FAILED  ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|23)

#define OSAL_E_WRONGPROCESS 	     ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|24)
// Operation was canceled  (e.g. AIO)
#define OSAL_E_CANCELED              ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|25)
// a thread is waiting for MQ
#define OSAL_E_MQINUSE               ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|26)

// --C++-Interface.
#define OSAL_E_ALREADYOPENED	     ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|27)

// Application in low priority level
#define OSAL_E_LOW_PRIORITY          ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|28)
// Resource in Exclusive access
#define OSAL_E_IN_EXCL_ACCESS        ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|29)
// Resource temporarily not available
#define OSAL_E_TEMP_NOT_AVAILABLE    ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|30)
// No media in the drive
#define OSAL_E_MEDIA_NOT_AVAILABLE   ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|31)
// Cannot eject the disc because the drive is locked
#define OSAL_E_EJECT_LOCKED          ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|32)
// If device is not initialised
#define OSAL_E_NOTINITIALIZED        ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|33)
// physical error of device
#define OSAL_E_IOERROR               ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|34)
// last Error
#define OSAL_E_LASTERROR             ((tU32)OSAL_C_COMPONENT|OSAL_C_ERROR_TYPE|35)

//-----------------------------------------------------------------------
//typedefs and struct defs (scope: global)
//-----------------------------------------------------------------------
//!
//! \typedef OSAL_tpfErrorHook
//!			This function pointer prototype is used to manage the error
//!
typedef tVoid (*OSAL_tpfErrorHook)(tU32);

//-----------------------------------------------------------------------
// function prototypes (scope: global)
//-----------------------------------------------------------------------
extern tU32 OSAL_u32ErrorCode( void );
extern tVoid OSAL_vSetErrorCode(tU32 u32ErrorCode);
extern tVoid OSAL_vSetErrorCodeEx(tU32 u32ErrorCode, tCString szFilename, tS32 s32LineNumber);
extern tCString OSAL_coszErrorText(tS32 s32Ecode);
#if 0
extern tVoid OSAL_vErrorHook( const OSAL_tpfErrorHook NewHook,OSAL_tpfErrorHook *    pOldHook );
#endif
extern tS32 /*@alt void@*/ OSAL_s32CallErrorHook(tU32 u32ErrorCode);

#if (OSAL_OS==OSAL_LINUX) || (OSAL_OS==OSAL_TKERNEL) || (OSAL_OS==OSAL_FREERTOS)//lint !e553 - MISRA 19.11 - not defined but not a problem
extern tU32 errno2osal(void);
#if 0
extern int osal2errno(tU32 osalerror);
#endif
#endif

#ifdef __cplusplus
}
#endif

#else
#error oserror.h included several times
#endif // !OSERROR_H


// EOF
