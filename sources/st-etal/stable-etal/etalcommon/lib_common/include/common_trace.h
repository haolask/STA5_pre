//!
//!  \file		common_trace.h
//!  \brief 	<i><b> COMMON module trace and log macros </b></i>
//!  \details   Utilities to print strings to the standard output.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!
#ifndef COMMON_TRACE_H
#define COMMON_TRACE_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef CONFIG_APP_TUNERDRIVER_LIBRARY
	#define TR_CLASS_TUNERDRIVER 0x02000000L
	#define TR_CLASS_CMOST       0x04000000L
	#define TR_CLASS_STECI       0x05000000L
	#define TR_CLASS_BOOT        0x06000000L
	#define TR_CLASS_BSP         0x08000000L
	#define TR_CLASS_EXTERNAL    0x09000000L

	#define TR_LEVEL_FATAL      0
	#define TR_LEVEL_ERRORS     1
    #define TR_LEVEL_SYSTEM_MIN 2
	#define TR_LEVEL_SYSTEM		3
	#define TR_LEVEL_COMPONENT  4

	tVoid COMMON_tracePrint(tS32 level, tU32 tr_class, const tChar *coszFormat, ...);
#else
/*!
 * \def		COMMON_tracePrint
 * 			For ETAL builds maps to the OSAL trace utility.
 * 			For TUNER_DRIVER builds maps to a similar but simplified function
 * 			provideded byt lib_common. This is to avoid dependency on OSAL from
 * 			TUNER_DRIVER.
 */
	#define COMMON_tracePrint(level, tr_class, ...) ((void)OSALUTIL_s32TracePrintf(0, (level), (tr_class), ## __VA_ARGS__))
#endif

#define	COMMON_tracePrintBufComponent(lclass, buffer, len, opt_string)	(COMMON_tracePrintBuf(TR_LEVEL_COMPONENT, (lclass),(buffer), (len), (opt_string)))
#define	COMMON_tracePrintBufSystem(lclass, buffer, len, opt_string)	(COMMON_tracePrintBuf(TR_LEVEL_SYSTEM, (lclass),(buffer), (len), (opt_string)))
#define	COMMON_tracePrintBufError(lclass, buffer, len, opt_string)	(COMMON_tracePrintBuf(TR_LEVEL_ERRORS, (lclass),(buffer), (len), (opt_string)))


tVoid COMMON_tracePrintBuf(tU32 level, tU32 lclass, tU8 *buffer, tU32 len, tChar *opt_string);

#ifdef __cplusplus
}
#endif

#endif // COMMON_TRACE_H


