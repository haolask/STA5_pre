//!
//!  \file 		steci_trace.h
//!  \brief 	<i><b> STECI module trace and log macros </b></i>
//!  \details   This file contains macros to print messages to the output.
//!				The macros are defined so that if the TR_LEVEL is below
//!				a threshold the trace code is not even included in the build.
//!				The macros map to the OSAL OSALUTIL_s32TracePrintf function.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!
#ifndef STECI_TRACE_H
#define STECI_TRACE_H

/***********************************
 *
 * Macros
 *
 **********************************/

/*!
 * \def		STECI_tracePrintFatal
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_FATAL or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_STECI and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_FATAL)
	#define STECI_tracePrintFatal(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_FATAL, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define STECI_tracePrintFatal(mclass, ...) do { } while (0)
#endif

/*!
 * \def		STECI_tracePrintError
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_ERRORS or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_STECI and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_ERRORS)
	#define STECI_tracePrintError(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_ERRORS, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define STECI_tracePrintError(mclass, ...) do { } while (0)
#endif

/*!
 * \def		STECI_tracePrintError
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_ERRORS or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_STECI and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_ERRORS)
	#define STECI_tracePrintBufError(mclass, ...) do { COMMON_tracePrintBufError((mclass), ## __VA_ARGS__); } while (0)
#else
	#define STECI_tracePrintBufError(mclass, ...) do { } while (0)
#endif


/*!
 * \def		STECI_tracePrintSysmin
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_SYSTEM_MIN or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_STECI and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_SYSTEM_MIN)
	#define STECI_tracePrintSysmin(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define STECI_tracePrintSysmin(mclass, ...) do { } while (0)
#endif

/*!
 * \def		STECI_tracePrintSytem
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_SYSTEM or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_STECI and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_SYSTEM)
	#define STECI_tracePrintSystem(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM, mclass, __VA_ARGS__); } while (0)
#else
	#define STECI_tracePrintSystem(mclass, ...) do { } while (0)
#endif

/*!
 * \def		STECI_tracePrintComponent
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_COMPONENT or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_STECI and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
	#define STECI_tracePrintComponent(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_COMPONENT, mclass, __VA_ARGS__); } while (0)
#else
	#define STECI_tracePrintComponent(mclass, ...) do { } while (0)
#endif

#endif // STECI_TRACE_H
