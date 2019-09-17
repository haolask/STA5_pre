//!
//!  \file 		osdefine.h
//!  \brief 	<i><b> OSAL defines and constants Header File </b></i>
//!  \details	This is the headerfile for the OS - Abstraction Layer for
//!             defines and constants
//!  \author 	Luca Pesenti
//!  \author 	(original version) Giovanni Di Martino
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef OSDEFINE_H
#define OSDEFINE_H

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************/
/* defines and constants                                                  */
/**************************************************************************/

/* -- OSAL version, corresponding to "Referenzhandbuch": */
#define OSAL_VERSION         (tS32(0x100))

/* -- Error messages. */
#define OSAL_OK     ((tS32)  0)
#define OSAL_ERROR  ((tS32) -1)

#define OSAL_ERROR_UNEXPECTED  ((tS32) -2)
#define OSAL_ERROR_INVALID_PARAM ((tS32) -3)
#define OSAL_ERROR_DEVICE_INIT   ((tS32) -4)
#define OSAL_ERROR_DEVICE_NOT_OPEN ((tS32) -5)
#define OSAL_ERROR_ACTION_FORBIDDEN ((tS32) -6)
#define OSAL_ERROR_CANNOT_OPEN ((tS32) -7)
#define OSAL_ERROR_CHECKSUM ((tS32) -8)
#define OSAL_ERROR_FROM_SLAVE ((tS32) -9)
#define OSAL_ERROR_NOT_SUPPORTED ((tS32) -10)
#define OSAL_ERROR_TIMEOUT_EXPIRED ((tS32)-11)
#define OSAL_ERROR_CALLBACK ((tS32)-12)
#define OSAL_ERROR_NOT_IMPLEMENTED ((tS32)-13)


/* -- Maximum and minimum values of a data type: */
#define OSAL_C_S8_MIN        ((tS8)  -128)
#define OSAL_C_S8_MAX        ((tS8)   127)
#define OSAL_C_S16_MIN       ((tS16) -32768)
#define OSAL_C_S16_MAX       ((tS16)  32767)
#define OSAL_C_S32_MIN       ((tS32) -2147483648L)
#define OSAL_C_S32_MAX       ((tS32)  2147483647L)

#define OSAL_C_U8_MAX        ((tU8)  255U)
#define OSAL_C_U16_MAX       ((tU16) 65535U)
#define OSAL_C_U32_MAX       ((tU32) 4294967295UL)

/* -- STILL MISSING! */
#define OSAL_C_FLOAT_MIN
#define OSAL_C_FLOAT_MAX
#define OSAL_C_DOUBLE_MIN
#define OSAL_C_DOUBLE_MAX

/* Smaller value x with 1.0+x ! = x ( epsilon environment) */
#define OSAL_C_FLOAT_EPSILON
#define OSAL_C_DOUBLE_EPSILON

/* Mathematical constants */
#define OSAL_C_DOUBLE_PI      (3.14159265358979323846)

/* -- Minimal and maximal priorities of Threads */
#define OSAL_C_U32_THREAD_PRIORITIES   ((tU32) 64)

#ifdef HOST_OS_TKERNEL
#define OSAL_C_U32_THREAD_PRIORITY_HIGHEST ((tU32) 0)
#define OSAL_C_U32_THREAD_PRIORITY_NORMAL  ((tU32) 65)
#define OSAL_C_U32_THREAD_PRIORITY_LOWEST  ((tU32) 70)

#define OSAL_C_U32_MAX_NAMELENGTH		((tU32)8)


#else
#define OSAL_C_U32_THREAD_PRIORITY_HIGHEST ((tU32) 0)
#define OSAL_C_U32_THREAD_PRIORITY_NORMAL  ((tU32) 31)
#define OSAL_C_U32_THREAD_PRIORITY_LOWEST  ((tU32) 63)

#define OSAL_C_U32_MAX_NAMELENGTH		((tU32)32)

#endif

/* -- Minimal and maximal priorities of process */
#define OSAL_C_U32_PROCESS_PRIORITY_HIGHEST ((tU32) 0)
#define OSAL_C_U32_PROCESS_PRIORITY_NORMAL  ((tU32) 1)
#define OSAL_C_U32_PROCESS_PRIORITY_LOWEST  ((tU32) 2)

#define OSAL_NULL                (0)
#define OSAL_C_U32_INFINITE      (OSAL_C_U32_MAX)
#define OSAL_C_INVALID_HANDLE    ((tU32)-1)

/* Time out parameter for semaphors etc. */
#define OSAL_C_TIMEOUT_FOREVER		(OSAL_C_U32_INFINITE)
#define OSAL_C_TIMEOUT_NOBLOCKING   (OSAL_NULL)

/* own, not real , process- and thread id */
#define OSAL_C_PROCESS_ID_SELF			(OSAL_NULL)
#define OSAL_C_THREAD_ID_SELF			(OSAL_NULL)

/* Maximal length of a name (tString) */
#define OSAL_C_U32_MAX_NAMELENGTH		((tU32)32)
#define OSAL_C_U32_MAX_FILENAMELENGTH     ((tU32)14)


/* --Standardeingabe, -ausgabe und -fehler */
#define OSAL_C_FD_STANDARDIN		(0)
#define OSAL_C_FD_STANDARDOUT		(1)
#define OSAL_C_FD_STANDARDERROR		(2)


#ifdef __cplusplus
}
#endif

#else
#error osdefine.h included several times
#endif


/* EOF */

