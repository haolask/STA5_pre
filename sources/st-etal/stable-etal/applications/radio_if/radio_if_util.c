//!
//!  \file    radio_if_util.c
//!  \brief   <i><b> Radio interface util </b></i>
//!  \details  Radio interface application utility functions
//!  \author  David Pastor
//!

#include "osal.h"
#include "radio_if_util.h"

#define UTIL_TRACE_BUFPRINT_SIZE    512

/**************************************
 *
 * rif_util_tracePrintBufComponent
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
tVoid rif_util_tracePrintBufComponent(tU32 lclass, tU8 *buffer, tU32 len, tChar *opt_string)
{
	static char line[UTIL_TRACE_BUFPRINT_SIZE] = "(empty)"; /* static for size */
	tU32 line_curr = 0;
	tU32 i;
	tS32 nl = -1;

	if (opt_string != NULL)
	{
		line_curr = (tU32)strlen((char *)opt_string);
		OSAL_s32NPrintFormat(&line[0], UTIL_TRACE_BUFPRINT_SIZE, "%s", opt_string);
	}
	for (i = 0; (i < len) && (line_curr < UTIL_TRACE_BUFPRINT_SIZE); i++)
	{
		if (nl++ == 7)
		{
			OSAL_s32NPrintFormat(&line[line_curr], UTIL_TRACE_BUFPRINT_SIZE - line_curr, "\n");
			nl = 0;
			line_curr += 1;
		}
		OSAL_s32NPrintFormat(&line[line_curr], UTIL_TRACE_BUFPRINT_SIZE - line_curr, "%.2X ", *buffer++);
		line_curr += 3;
	}

	if (i < len)
	{
		OSAL_TRACE_PRINTF(TR_LEVEL_COMPONENT, lclass, "(TRUNCATED) %s", line);
	}
	else
	{
		OSAL_TRACE_PRINTF(TR_LEVEL_COMPONENT, lclass, "%s", line);
	}

}

