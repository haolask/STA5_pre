//!
//!  \file 		hdradio_trace.h
//!  \brief 	<i><b> HDRADIO module trace and log macros </b></i>
//!  \details   This file contains macros to print messages to the output.
//!				The macros are defined so that if the TR_LEVEL is below
//!				a threshold the trace code is not even included in the build.
//!				The macros map to the OSAL OSALUTIL_s32TracePrintf function.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!
#ifndef HDRADIO_TRACE_H
#define HDRADIO_TRACE_H

/***********************************
 *
 * Macros
 *
 **********************************/

/*!
 * \def		HDRADIO_tracePrintFatal
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_FATAL or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_HDRADIO and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_FATAL)
	#define HDRADIO_tracePrintFatal(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_FATAL, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define HDRADIO_tracePrintFatal(mclass, ...) do { } while (0)
#endif

/*!
 * \def		HDRADIO_tracePrintError
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_ERRORS or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_HDRADIO and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_ERRORS)
	#define HDRADIO_tracePrintError(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_ERRORS, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define HDRADIO_tracePrintError(mclass, ...) do { } while (0)
#endif

/*!
 * \def		HDRADIO_tracePrintSysmin
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_SYSTEM_MIN or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_HDRADIO and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_SYSTEM_MIN)
	#define HDRADIO_tracePrintSysmin(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, (mclass), ## __VA_ARGS__); } while (0)
#else
	#define HDRADIO_tracePrintSysmin(mclass, ...) do { } while (0)
#endif

/*!
 * \def		HDRADIO_tracePrintSystem
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_SYSTEM or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_HDRADIO and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_SYSTEM)
	#define HDRADIO_tracePrintSystem(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM, mclass, __VA_ARGS__); } while (0)
#else
	#define HDRADIO_tracePrintSystem(mclass, ...) do { } while (0)
#endif

/*!
 * \def		HDRADIO_tracePrintComponent
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_COMPONENT or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_HDRADIO and its sub-classes 
 */
#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_COMPONENT)
	#define HDRADIO_tracePrintComponent(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_COMPONENT, mclass, __VA_ARGS__); } while (0)
#else
	#define HDRADIO_tracePrintComponent(mclass, ...) do { } while (0)
#endif

#endif // HDRADIO_TRACE_H
