//!
//!  \file 		common_trace.c
//!  \brief 	<i><b> Trace functionalities </b></i>
//!  \details   Trace and debug functions for the ETAL, TUNER_DRIVER and DCOP_DRIVER
//!				modules, for OSAL and non-OSAL builds
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "target_config.h"

#ifdef CONFIG_TRACE_ENABLE

#if defined (CONFIG_APP_TUNERDRIVER_LIBRARY) && !defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
	#include "osal_replace.h"
#else
	#include "osal.h"
#endif

#include "common_trace.h"


/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*!
 * \def		COMMON_TRACE_MAX_MESSAGESIZE
 * 			Max length of the string supported by #COMMON_tracePrint.
 * 			If the requested string is longer than this limit
 * 			it will be truncated. This number impacts on a statically
 * 			allocated buffer.
 */
#define COMMON_TRACE_MAX_MESSAGESIZE    256
/*!
 * \def		COMMON_TRACE_BUFPRINT_SIZE
 * 			Max length of the string supported by #COMMON_tracePrintBufComponent.
 * 			If the requested string (i.e. the result of the conversion
 * 			of the data buffer to a printable string) is longer than this limit
 * 			it will be truncated. This number impacts on a statically
 * 			allocated buffer.
 */
#define COMMON_TRACE_BUFPRINT_SIZE      512

/*****************************************************************
| local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*****************************************************************
| functions
|----------------------------------------------------------------*/

#ifdef CONFIG_APP_TUNERDRIVER_LIBRARY
/**************************************
 *
 * COMMON_tracePrint
 *
 *************************************/
/*!
 * \brief		Formatted print functionality for non-OSAL builds
 * \details		This function is a replacement for #OSALUTIL_s32TracePrintf
 * 				to be used for builds which do not include OSAL (e.g. for
 * 				size constraints). All ETAL modules use the function
 * 				named COMMON_tracePrint to generate output. The name is mapped
 * 				to #OSALUTIL_s32TracePrintf through a 'C' preprocessor
 * 				macro if OSAL is available, or to this function if
 * 				OSAL is not available.
 *
 * 				The function mimics the #OSALUTIL_s32TracePrintf by printing
 * 				the main fields of the header, but provides no advanced
 * 				functionalities like message filtering based on class/level,
 * 				output to file, run-time disabling of the output.
 *
 * 				Since there is no specific configuration item to include
 * 				or not OSAL, the selection is made by looking at the
 *				CONFIG_APP_TUNERDRIVER_LIBRARY configuration item. When
 *				defined, ETAL is being built as TUNER_LIBRARY only,
 *				which apart from the printing has no OSAL dependency
 *				so it can be build without OSAL.
 * \param[in]	level - the level of the message (e.g. TR_LEVEL_ERROR),
 * 				        defined in tri_types.h (if OSAL is used) or in 
 * 				        common_trace.h (for OSAL-less builds).
 * \param[in]	tr_class - the class of the message (e.g. TR_CLASS_APP_ETAL),
 * 				        defined in #TR_tenTraceClass of tri_types.h (if OSAL is used)
 * 				        or in common_trace.h (for OSAL-less builds).
 * \param[in]	coszFormat - the string to be printed, in 'printf' format,
 * 				             followed by a variable number of arguments.
 * 				             The size of the string formed by the #coszFormat#
 * 				             parameter is limited to #COMMON_TRACE_MAX_MESSAGESIZE
 * 				             characters (the string is truncated in case of overflow).
 * \see			#OSALUTIL_s32TracePrintf
 * \callgraph
 * \callergraph
 */
tVoid COMMON_tracePrint(tS32 level, tU32 tr_class, const tChar *coszFormat, ...)
{
	static tChar szBuffer[COMMON_TRACE_MAX_MESSAGESIZE]; // static for size
	va_list argList;

	/* DeactivateLintMessage_ID0033 */ 
	/*lint -save -e530 MISRA 9.1 va_start allow to initialize the argument*/
	va_start(argList, coszFormat);
	/*lint -restore */
	vsnprintf(szBuffer, sizeof(szBuffer), coszFormat, argList);
	va_end(argList);

	switch (tr_class)
	{
		case TR_CLASS_CMOST:
			printf("CMOST ");
			break;

		case TR_CLASS_BOOT:
			printf("BOOT ");
			break;

		case TR_CLASS_BSP:
			printf("BSP ");
			break;

		case TR_CLASS_STECI:
			printf("STECI ");
			break;
	}

	switch (level)
	{
		case TR_LEVEL_FATAL:
			printf("FATAL ");
			break;
		case TR_LEVEL_ERRORS:
			printf("ERROR ");
			break;
		case TR_LEVEL_SYSTEM:
		case TR_LEVEL_SYSTEM_MIN:
			printf("INFO ");
			break;
		case TR_LEVEL_COMPONENT:
			break;
		default:
			printf("UNKNOWN LEVEL ");
	}
	printf(szBuffer);
	printf("\n");
}
#endif // CONFIG_APP_TUNERDRIVER_LIBRARY

#if (defined(CONFIG_TRACE_CLASS_CMOST)       && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_SYSTEM)) || \
	(defined(CONFIG_TRACE_CLASS_TUNERDRIVER) && (CONFIG_TRACE_CLASS_TUNERDRIVER >= TR_LEVEL_SYSTEM)) || \
	(defined(CONFIG_TRACE_CLASS_STECI)       && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_ERRORS)) || \
	(defined(CONFIG_TRACE_CLASS_ETAL)        && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)) || \
	(defined(CONFIG_TRACE_CLASS_HDRADIO)     && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_COMPONENT)) 
	
/**************************************
 *
 * COMMON_tracePrintBufComponent
 *
 *************************************/
/*!
 * \brief		Prints the content of a binary buffer
 * \details		The function prints the #buffer# content, 8 bytes
 * 				per line, each byte in hexadecimal format separated
 * 				by a space by the preceding one and with no leading 0x
 * 				indication.
 *
 * 				If the #buffer#is empty (i.e. #len# is 0) the function
 * 				prints "(empty)".
 *
 * 				The buffer may optionally be preceded by a string
 * 				specified in #opt_string#.
 *
 * 				If the #buffer# length exceeds #COMMON_TRACE_BUFPRINT_SIZE
 * 				the function prints as many decoded bytes as possible
 * 				and then stops printing "(TRUNCATED)".
 *
 * 				Printing is done with the ETAL standard printing utility
 * 				#COMMON_tracePrint with fixed level TR_LEVEL_COMPONENT and
 * 				class #lclass#.
 *
 * \remark		The function is available only for TR_LEVEL_COMPONENT,
 * 				and only for some specific ETAL modules.
 * \param[in]	lclass - the class of the message being printed (e.g. TR_CLASS_APP_ETAL).
 * 				         This class is passed to the standard ETAL print
 * 				         utility so the output of this function will be
 * 				         subject to the usual ETAL filters.
 * \param[in]	buffer - pointer to a buffer containing the bytes to decode
 * 				         and print.
 * \param[in]	len - size in bytes of the #buffer#. Must be limited to
 * 				      #COMMON_TRACE_BUFPRINT_SIZE or less (including #opt_string
 * 				      size in the count) or truncation will occur.
 * \param[in]	opt_string - optional string to print before the first decoded
 * 				             #buffer# byte. If NULL it is ignored.
 * \callgraph
 * \callergraph
 */
#if defined (CONFIG_HOST_OS_FREERTOS)
tVoid COMMON_tracePrintBuf(tU32 level, tU32 lclass, tU8 *buffer, tU32 len, tChar *opt_string)
{
    static char line[COMMON_TRACE_BUFPRINT_SIZE] = "(empty)"; /* static for size */
    tU32 line_curr = 0;
    tU32 i;
	tS32 nl = -1;

    if (opt_string != NULL)
    {
		line_curr = (tU32)strlen((char *)opt_string) + 1;
        (void)OSAL_s32NPrintFormat(&line[0], COMMON_TRACE_BUFPRINT_SIZE, "%s\n", opt_string);
    }

    if (len != 0)
    {
	    for (i = 0; (i < len) && (line_curr < COMMON_TRACE_BUFPRINT_SIZE); i++)
	    {
	        if (nl++ == 7)
	        {
	            (void)OSAL_s32NPrintFormat(&line[line_curr], COMMON_TRACE_BUFPRINT_SIZE - line_curr, "\n");
	            nl = 0;
	            line_curr += 1;

		    COMMON_tracePrint(TR_LEVEL_COMPONENT, lclass, "%s", line);
		    line_curr = 0;
	        }
	        (void)OSAL_s32NPrintFormat(&line[line_curr], COMMON_TRACE_BUFPRINT_SIZE - line_curr, "%X ", *buffer++);
	        line_curr += 3;
	    }

	    if (i < len)
	    {
	        COMMON_tracePrint(TR_LEVEL_COMPONENT, lclass, "(TRUNCATED)");
		}
	    else
	    {
	        COMMON_tracePrint(TR_LEVEL_COMPONENT, lclass, "%s", line);
	    }

	}
}
#else
tVoid COMMON_tracePrintBuf(tU32 level, tU32 lclass, tU8 *buffer, tU32 len, tChar *opt_string)
{
    static char line[COMMON_TRACE_BUFPRINT_SIZE] = "(empty)"; /* static for size */
    tU32 line_curr = 0;
    tU32 i;
	tS32 nl = -1;

    if (opt_string != NULL)
    {
		line_curr = (tU32)strlen((char *)opt_string) + 1;
        (void)OSAL_s32NPrintFormat(&line[0], COMMON_TRACE_BUFPRINT_SIZE, "%s\n", opt_string);
    }

    if (len != 0)
    {
	    for (i = 0; (i < len) && (line_curr < COMMON_TRACE_BUFPRINT_SIZE); i++)
	    {
	        if (nl++ == 7)
	        {
	            (void)OSAL_s32NPrintFormat(&line[line_curr], COMMON_TRACE_BUFPRINT_SIZE - line_curr, "\n");
	            nl = 0;
	            line_curr += 1;
	        }
	        (void)OSAL_s32NPrintFormat(&line[line_curr], COMMON_TRACE_BUFPRINT_SIZE - line_curr, "%.2X ", *buffer++);
	        line_curr += 3;
	    }

	    if (i < len)
	    {
	        COMMON_tracePrint(level, lclass, "(TRUNCATED) %s", line);
		}
	    else
	    {
	        COMMON_tracePrint(level, lclass, "%s", line);
	    }
	}
}
#endif // CONFIG_HOST_OS_FREERTOS
#endif // complex condition
#endif // CONFIG_TRACE_ENABLE


