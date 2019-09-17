//!
//!  \file 		etal_trace.h
//!  \brief 	<i><b> ETAL module trace and log macros </b></i>
//!  \details   This file contains macros to print messages to the output.
//!				The macros are defined so that if the TR_LEVEL is below
//!				a threshold the trace code is not even included in the build.
//!				The macros map to the OSAL OSALUTIL_s32TracePrintf function.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!
#ifndef ETAL_TRACE_H
#define ETAL_TRACE_H

/***********************************
 *
 * Macros
 *
 **********************************/

/*!
 * \def		ETAL_tracePrintFatal
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_FATAL or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_APP_ETAL and its sub-classes (TR_CLASS_APP_ETAL_COMM,...).
 */
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_FATAL)
	#define ETAL_tracePrintFatal(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_FATAL, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define ETAL_tracePrintFatal(mclass, ...) do { } while (0)
#endif

/*!
 * \def		ETAL_tracePrintError
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_ERRORS or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_APP_ETAL and its sub-classes (TR_CLASS_APP_ETAL_COMM,...).
 */
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS)
	#define ETAL_tracePrintError(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_ERRORS, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define ETAL_tracePrintError(mclass, ...) do { } while (0)
#endif

/*!
 * \def		ETAL_tracePrintSysmin
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_SYSTEM_MIN or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_APP_ETAL and its sub-classes (TR_CLASS_APP_ETAL_COMM,...).
 */
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	#define ETAL_tracePrintSysmin(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define ETAL_tracePrintSysmin(mclass, ...) do { } while (0)
#endif

/*!
 * \def		ETAL_tracePrintSystem
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_SYSTEMor greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_APP_ETAL and its sub-classes (TR_CLASS_APP_ETAL_COMM,...).
 */
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM)
	#define ETAL_tracePrintSystem(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define ETAL_tracePrintSystem(mclass, ...) do { } while (0)
#endif


/*!
 * \def		ETAL_tracePrintComponent
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_COMPONENT or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_APP_ETAL and its sub-classes (TR_CLASS_APP_ETAL_COMM,...).
 */
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
	#define ETAL_tracePrintComponent(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_COMPONENT, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define ETAL_tracePrintComponent(mclass, ...) do { } while (0)
#endif

/*!
 * \def		ETAL_tracePrintUser1
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_USER_1 or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_APP_ETAL and its sub-classes (TR_CLASS_APP_ETAL_COMM,...).
 */
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_1)
	#define ETAL_tracePrintUser1(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_USER_1, mclass, __VA_ARGS__); } while (0)
#else
	#define ETAL_tracePrintUser1(mclass, ...) do { } while (0)
#endif

#endif // ETAL_TRACE_H
