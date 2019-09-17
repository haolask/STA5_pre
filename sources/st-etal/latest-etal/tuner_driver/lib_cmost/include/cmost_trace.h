//!  \file 		cmost_trace.h
//!  \brief 	<i><b> CMOST module trace and log macros </b></i>
//!  \details   This file contains macros to print messages to the output.
//!				The macros are defined so that if the TR_LEVEL is below
//!				a threshold the trace code is not even included in the build.
//!				The macros map to the OSAL OSALUTIL_s32TracePrintf function.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!
#ifndef CMOST_TRACE_H
#define CMOST_TRACE_H

/***********************************
 *
 * Macros
 *
 **********************************/

/*!
 * \def		CMOST_tracePrintFatal
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_FATAL or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_CMOST and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_FATAL)
	#define CMOST_tracePrintFatal(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_FATAL, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define CMOST_tracePrintFatal(mclass, ...) do { } while (0)
#endif

/*!
 * \def		CMOST_tracePrintError
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_ERRORS or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_CMOST and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_ERRORS)
	#define CMOST_tracePrintError(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_ERRORS, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define CMOST_tracePrintError(mclass, ...) do { } while (0)
#endif

/*!
 * \def		CMOST_tracePrintSysmin
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_SYSTEM_MIN or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_CMOST and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_SYSTEM_MIN)
	#define CMOST_tracePrintSysmin(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define CMOST_tracePrintSysmin(mclass, ...) do { } while (0)
#endif

/*!
 * \def		CMOST_tracePrintComponent
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_COMPONENT or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_CMOST and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_COMPONENT)
	#define CMOST_tracePrintComponent(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_COMPONENT, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define CMOST_tracePrintComponent(mclass, ...) do { } while (0)
#endif

#endif // CMOST_TRACE_H
