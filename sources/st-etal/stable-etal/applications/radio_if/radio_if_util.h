//!
//!  \file		radio_if_util.c
//!  \brief		<i><b> Radio interface util for Accordo radio tuners</b></i>
//!  \details  Radio interface application utility functions
//!  \author	David Pastor
//!

#ifndef RADIO_IF_UTIL_H
#define RADIO_IF_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common_trace.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

#define UTIL_TRACE_BUF_COMPONENT(_lclass, _buffer, _len, _opt_string)  rif_util_tracePrintBufComponent(_lclass, _buffer, _len, _opt_string)

/*!
 * \def		rif_tracePrintFatal
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_FATAL or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 */
#if defined(CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER) && (CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER >= TR_LEVEL_FATAL)
	#define rif_pr_tracePrintFatal(mclass, ...) do { OSAL_TRACE_PRINTF(TR_LEVEL_FATAL, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_pr_tracePrintFatal(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_RIF_RIMW) && (CONFIG_TRACE_CLASS_RIF_RIMW >= TR_LEVEL_FATAL)
	#define rif_rimw_tracePrintFatal(mclass, ...) do { OSAL_TRACE_PRINTF(TR_LEVEL_FATAL, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_rimw_tracePrintFatal(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_STECI_UART) && (CONFIG_TRACE_CLASS_STECI_UART >= TR_LEVEL_FATAL)
	#define steci_uart_tracePrintFatal(mclass, ...) do { OSAL_TRACE_PRINTF(TR_LEVEL_FATAL, mclass, __VA_ARGS__); } while (0)
#else
	#define steci_uart_tracePrintFatal(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_MCP) && (CONFIG_TRACE_CLASS_MCP >= TR_LEVEL_FATAL)
	#define mcp_tracePrintFatal(mclass, ...) do { OSAL_TRACE_PRINTF(TR_LEVEL_FATAL, mclass, __VA_ARGS__); } while (0)
#else
	#define mcp_tracePrintFatal(mclass, ...) do { } while (0)
#endif

/*!
 * \def		rif_tracePrintError
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_ERRORS or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 */
#if defined(CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER) && (CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER >= TR_LEVEL_ERRORS)
	#define rif_pr_tracePrintError(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_ERRORS, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_pr_tracePrintError(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_RIF_RIMW) && (CONFIG_TRACE_CLASS_RIF_RIMW >= TR_LEVEL_ERRORS)
	#define rif_rimw_tracePrintError(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_ERRORS, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_rimw_tracePrintError(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_STECI_UART) && (CONFIG_TRACE_CLASS_STECI_UART >= TR_LEVEL_ERRORS)
	#define steci_uart_tracePrintError(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_ERRORS, mclass, __VA_ARGS__); } while (0)
#else
	#define steci_uart_tracePrintError(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_MCP) && (CONFIG_TRACE_CLASS_MCP >= TR_LEVEL_ERRORS)
	#define mcp_tracePrintError(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_ERRORS, mclass, __VA_ARGS__); } while (0)
#else
	#define mcp_tracePrintError(mclass, ...) do { } while (0)
#endif

/*!
 * \def		rif_tracePrintSysmin
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_SYSTEM_MIN or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 */
#if defined(CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER) && (CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER >= TR_LEVEL_SYSTEM_MIN)
	#define rif_pr_tracePrintSysmin(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_pr_tracePrintSysmin(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_RIF_RIMW) && (CONFIG_TRACE_CLASS_RIF_RIMW >= TR_LEVEL_SYSTEM_MIN)
	#define rif_rimw_tracePrintSysmin(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_rimw_tracePrintSysmin(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_STECI_UART) && (CONFIG_TRACE_CLASS_STECI_UART >= TR_LEVEL_SYSTEM_MIN)
	#define steci_uart_tracePrintSysmin(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, mclass, __VA_ARGS__); } while (0)
#else
	#define steci_uart_tracePrintSysmin(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_MCP) && (CONFIG_TRACE_CLASS_MCP >= TR_LEVEL_SYSTEM_MIN)
	#define mcp_tracePrintSysmin(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, mclass, __VA_ARGS__); } while (0)
#else
	#define mcp_tracePrintSysmin(mclass, ...) do { } while (0)
#endif

/*!
 * \def		rif_tracePrintSystem
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_SYSTEMor greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 */
#if defined(CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER) && (CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER >= TR_LEVEL_SYSTEM)
	#define rif_pr_tracePrintSystem(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_pr_tracePrintSystem(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_RIF_RIMW) && (CONFIG_TRACE_CLASS_RIF_RIMW >= TR_LEVEL_SYSTEM)
	#define rif_rimw_tracePrintSystem(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_rimw_tracePrintSystem(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_STECI_UART) && (CONFIG_TRACE_CLASS_STECI_UART >= TR_LEVEL_SYSTEM)
	#define steci_uart_tracePrintSystem(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM, mclass, __VA_ARGS__); } while (0)
#else
	#define steci_uart_tracePrintSystem(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_MCP) && (CONFIG_TRACE_CLASS_MCP >= TR_LEVEL_SYSTEM)
	#define mcp_tracePrintSystem(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_SYSTEM, mclass, __VA_ARGS__); } while (0)
#else
	#define mcp_tracePrintSystem(mclass, ...) do { } while (0)
#endif

/*!
 * \def		rif_tracePrintComponent
 * 			Wrapper for the print function. It expands to the print function if
 * 			the trace level is TR_LEVEL_COMPONENT or greater, to null operation otherwise.
 * 			The print function is #OSALUTIL_s32TracePrintf or #COMMON_tracePrint, depending
 * 			on the build options.
 * \remark	To be used only for TR_CLASS_APP_ETAL and its sub-classes (TR_CLASS_APP_ETAL_COMM,...).
 */
#if defined(CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER) && (CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER >= TR_LEVEL_COMPONENT)
	#define rif_pr_tracePrintComponent(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_COMPONENT, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_pr_tracePrintComponent(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_RIF_RIMW) && (CONFIG_TRACE_CLASS_RIF_RIMW >= TR_LEVEL_COMPONENT)
	#define rif_rimw_tracePrintComponent(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_COMPONENT, mclass, __VA_ARGS__); } while (0)
#else
	#define rif_rimw_tracePrintComponent(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_STECI_UART) && (CONFIG_TRACE_CLASS_STECI_UART >= TR_LEVEL_COMPONENT)
	#define steci_uart_tracePrintComponent(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_COMPONENT, mclass, __VA_ARGS__); } while (0)
#else
	#define steci_uart_tracePrintComponent(mclass, ...) do { } while (0)
#endif
#if defined(CONFIG_TRACE_CLASS_MCP) && (CONFIG_TRACE_CLASS_MCP >= TR_LEVEL_COMPONENT)
	#define mcp_tracePrintComponent(mclass, ...) do { COMMON_tracePrint(TR_LEVEL_COMPONENT, mclass, __VA_ARGS__); } while (0)
#else
	#define mcp_tracePrintComponent(mclass, ...) do { } while (0)
#endif


/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/

extern tVoid rif_util_tracePrintBufComponent(tU32 lclass, tU8 *buffer, tU32 len, tChar *opt_string);

#ifdef __cplusplus
}
#endif

#endif // RADIO_IF_UTIL_H

