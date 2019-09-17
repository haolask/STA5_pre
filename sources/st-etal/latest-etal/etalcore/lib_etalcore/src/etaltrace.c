//!
//!  \file 		etaltrace.c
//!  \brief 	<i><b> ETAL trace and log functionalities </b></i>
//!  \details   The ETAL trace and log functionality may be implemented
//!				through a dedicated thread that outputs the strings in the background,
//!				to avoid disrupting the ETAL processing with time consuming operations.
//!				This file contains that thread's code and also the utilities to convert
//!				various ETAL internal structures to a human-readable format, for
//!				logging purposes:
//!				- utility to print the ETAL events; this is routinely invoked by the
//!				  ETAL callback handler (#ETAL_callbackInvoke) to print EVENT
//!				  information, normally with class TR_CLASS_APP_ETAL_API and level TR_LEVEL_SYSTEM
//!				  (but there are some special cases, see #ETAL_traceEvent)
//!
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*!
 * \def		ETAL_TRACE_COMPACT_OUTPUT
 * 			If defined outputs spaces instead of tabs to obtain
 * 			a more compact print.
 *
 * 			Defaults to defined.
 */
#define ETAL_TRACE_COMPACT_OUTPUT

/*!
 * \def		ETAL_TRACE_FIFO_SIZE
 * 			When using CONFIG_TRACE_ASYNC mode, the strings are copied
 * 			to a FIFO (the #etalTraceBuffer) and later output by the print thread.
 * 			This macro defines the maximum number of messages we want to buffer.
 *
 * 			Messages en-queued after this max is reached will be discarded.
 *
 * 			Larger FIFO means messages have less chance to be discarded, but more
 * 			RAM occupation. The actual memory occupation depends on #ETAL_TRACE_ASYNC_USES_MALLOC.
 *
 * 			Defaults to 64, can be customized.
 */
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	/* There are a lot of messages in this level, increase the FIFO size to avoid loosing them */
	#define ETAL_TRACE_FIFO_SIZE           256
#else
	#define ETAL_TRACE_FIFO_SIZE            64
#endif

/*!
 * \def		ETAL_TRACE_SUBSTRING_SIZE
 * 			When using CONFIG_TRACE_ASYNC mode, the strings from the #etalTraceBuffer
 * 			are output in small pieces to reduce the time stolen to ETAL-critical
 * 			tasks. This macro defines the number of characters that will be
 * 			output at a time for each string.
 *
 * 			Larger size means the #etalTraceBuffer will be emptied faster, but there
 * 			will be more chances to disrupt the ETAL-critical tasks.
 *
 * 			Defaults to 31, can be customized.
 */
#define ETAL_TRACE_SUBSTRING_SIZE      31

/*!
 * \def		ETAL_TRACE_INIT_INDEX
 *			Internal macro used to identify the empty condition at FIFO startup.
 *
 *			Must not be changed.
 */
#define ETAL_TRACE_INIT_INDEX  (ETAL_TRACE_FIFO_SIZE + 1)

/*!
 * \def		ETAL_TRACE_ASYNC_USES_MALLOC
 * 			If defined the ETAL_tracePrint_ThreadEntry dynamically allocates
 * 			buffers for the strings passed to OSALUTIL_s32TracePrintf;
 * 			otherwise it creates a static array. In the latter case each
 * 			message requires OSAL_C_U32_TRACE_MAX_MESSAGESIZE (384) bytes
 * 			plus 12 bytes of meta-information.
 *
 * 			Defaults to undefined
 */
#undef ETAL_TRACE_ASYNC_USES_MALLOC

/*!
 * \def		ETAL_TRACE_STRING_LEN
 * 			Size in bytes of the intermediate buffer used to create and format
 * 			strings before outputting them.
 *
 * 			Defaults to 30, can be customized.
 */
#define ETAL_TRACE_STRING_LEN      30
/*!
 * \def		ETAL_TRACE_LONG_STRING_LEN
 * 			Size in bytes of the intermediate buffer used to create and format
 * 			**long** strings before outputting them.
 *
 * 			Defaults to 100, can be customized.
 */
#define ETAL_TRACE_LONG_STRING_LEN 100

/*!
 * \def		ETAL_TRACE_MAX_EVENT_STRING
 * 			Size in bytes of the intermediate buffer used to create and format
 * 			event logging strings.
 *
 * 			Defaults to 64, can be customized.
 */
#define ETAL_TRACE_MAX_EVENT_STRING 64

#ifdef ETAL_TRACE_COMPACT_OUTPUT
/*! \def	PREFIX0
 * 			String prefix, 0 units */
#define PREFIX0 ""
/*! \def	PREFIX1
 * 			String prefix, 1 unit */
#define PREFIX1 "  "
/*! \def	PREFIX2
 * 			String prefix, 2 units */
#define PREFIX2 "    "
/*! \def	PREFIX3
 * 			String prefix, 3 units */
#define PREFIX3 "      "
/*! \def	PREFIX4
 * 			String prefix, 4 units */
#define PREFIX4 "        "
/*! \def	PREFIX5
 * 			String prefix, 5 units */
#define PREFIX5 "          "
/*! \def	PREFIX_UNIT
 * 			String prefix */
#define PREFIX_UNIT "  "
#else
/*! \def	PREFIX0
 * 			String prefix, 0 units */
#define PREFIX0 ""
/*! \def	PREFIX1
 * 			String prefix, 1 unit */
#define PREFIX1 "\t"
/*! \def	PREFIX2
 * 			String prefix, 2 units */
#define PREFIX2 "\t\t"
/*! \def	PREFIX3
 * 			String prefix, 3 units */
#define PREFIX3 "\t\t\t"
/*! \def	PREFIX4
 * 			String prefix, 4 units */
#define PREFIX4 "\t\t\t\t"
/*! \def	PREFIX5
 * 			String prefix, 5 units */
#define PREFIX5 "\t\t\t\t\t"
/*! \def	PREFIX0
 * 			String prefix */
#define PREFIX_UNIT "\t"
#endif
/*! \def	PREFIX_LEN
 * 			Longest prefix size */
#define PREFIX_LEN (sizeof(PREFIX5))

/*!
 * \def		ETAL_tracePrintApi
 * 			Macro defined for code readabilty: invokes #OSALUTIL_s32TracePrintf
 * 			with some pre-defined parameters.
 */
#define ETAL_tracePrintApi(_lvl_, ...)		do{(void)OSALUTIL_s32TracePrintf(0, (_lvl_), TR_CLASS_APP_ETAL_API, ## __VA_ARGS__);}while(0)

/*!
 * \def		ETAL_DABMW_DATA_PROTOCOL_ASCII_MSG_SIZE
 * 			The max size supported for the ASCII message. This does not include
 * 			the message header and includes the prefix inserted by #ETAL_tracePrintAsciiLUN
 */
#define ETAL_DABMW_DATA_PROTOCOL_ASCII_MSG_SIZE   (OSAL_C_U32_TRACE_MAX_MESSAGESIZE)
/*!
 * \def		ETAL_DABMW_DATA_CHANNEL_ASCII_LEVEL
 * 			Extract from the message received on the ASCII LUN the message level
 */
#define ETAL_DABMW_DATA_CHANNEL_ASCII_LEVEL(_b_)  ((((tU32)((_b_)[12]) << 24) & 0xFF000000) | (((tU32)((_b_)[13]) << 16) & 0x00FF0000) | (((tU32)((_b_)[14]) << 8) & 0x0000FF00) | ((tU32)((_b_)[15]) & 0x000000FF))
/*!
 * \def		ETAL_DABMW_DATA_CHANNEL_ASCII_CLASS
 * 			Extract from the message received on the ASCII LUN the message class
 */
#define ETAL_DABMW_DATA_CHANNEL_ASCII_CLASS(_b_)  ((((tU32)((_b_)[16]) << 24) & 0xFF000000) | (((tU32)((_b_)[17]) << 16) & 0x00FF0000) | (((tU32)((_b_)[18]) << 8) & 0x0000FF00) | ((tU32)((_b_)[19]) & 0x000000FF))
/*!
 * \def		ETAL_DABMW_DATA_CHANNEL_ASCII_SIZE
 * 			Calculate from the message received on the ASCII LUN the printable message size
 */
#define ETAL_DABMW_DATA_CHANNEL_ASCII_SIZE(_b_)   ((((tU32)((_b_)[20]) << 24) & 0xFF000000) | (((tU32)((_b_)[21]) << 16) & 0x00FF0000) | (((tU32)((_b_)[22]) << 8) & 0x0000FF00) | ((tU32)((_b_)[23]) & 0x000000FF))
/*!
 * \def		ETAL_DABMW_DATA_CHANNEL_ASCII_PAYLOAD
 * 			Extract from the message received on the ASCII LUN the printable message
 */
#define ETAL_DABMW_DATA_CHANNEL_ASCII_PAYLOAD(_b_)((_b_) + 24)


/*****************************************************************
| local types
|----------------------------------------------------------------*/
#if defined (CONFIG_TRACE_ASYNC)
/*!
 * \struct		EtalTraceElemTy
 * 				Describes one element of the #etalTraceBuffer.
 */
typedef struct
{
	/*! Time when the message was inserted in the #etalTraceBuffer */
	OSAL_tMSecond timestamp;
	/*! The #OSALUTIL_s32TracePrintf level parameter */
	tU32 level;
	/*! The #OSALUTIL_s32TracePrintf class parameter */
	tU32 class;
#ifdef ETAL_TRACE_ASYNC_USES_MALLOC
	/*! The message pointer in case #ETAL_TRACE_ASYNC_USES_MALLOC is defined.
	 *
	 *  The message is a null-terminated string. */
	tChar *message;
#else
	/*! The message in case #ETAL_TRACE_ASYNC_USES_MALLOC is **not** defined.
	 *
	 *  The message is a null-terminated string. */
	tChar message[OSAL_C_U32_TRACE_MAX_MESSAGESIZE];
#endif
} EtalTraceElemTy;

/*!
 * \enum		EtalTraceStateTy
 * 				The internal states of the print thread.
 */
typedef enum
{
	/*! #etalTraceBuffer is empty, nothing to do */
	ETAL_TraceState_Idle,
	/*! Fetch the message on the top of the #etalTraceBuffer */
	ETAL_TraceState_StartRead,
	/*! Print the header containing timestamp, class and level of the message */
	ETAL_TraceState_PrintHeader,
	/*! Print one chunk of at most ETAL_TRACE_SUBSTRING_SIZE characters from the message */
	ETAL_TraceState_PrintMessage,
	/*! Message printed, go to idle */
	ETAL_TraceState_MessageComplete,
	/*! #etalTraceBuffer is not empty, advance the FIFO pointers to the first message */
	ETAL_TraceState_MessageAdvance
} EtalTraceStateTy;

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
/*!
 * \var		etalTraceBuffer
 * 			The FIFO buffer containing the messages collected from all
 *			OSALUTIL_s32TracePrintf calls.
 */
static EtalTraceElemTy etalTraceBuffer[ETAL_TRACE_FIFO_SIZE];
/*!
 * \var		etalTraceBufferLastRd
 * 			Next location to be read from #etalTraceBuffer.
 *
 * 			Initialized to an illegal value at startup to
 * 			differentiate from the empty condition.
 */
static tU32 etalTraceBufferLastRd;
/*!
 * \var		etalTraceBufferLastWr
 * 			Next location to be written to #etalTraceBuffer.
 *
 * 			Initialized to an illegal value at startup to
 * 			differentiate from the empty condition.
 */
static tU32 etalTraceBufferLastWr;
/*!
 * \var		etalTraceBufferOverflowCount
 * 			Number of times the #etalTraceBuffer overflowed and messages
 * 			were lost, for logging purposes.
 *
 * 			This value is printed when an overflow occurs.
 */
static tU32 etalTraceBufferOverflowCount;
/*!
 * \var		etalTraceBufferEmpty
 * 			TRUE if the #etalTraceBuffer is empty; not currently used.
 */
static tBool etalTraceBufferEmpty;
/*!
 * \var		ETAL_traceThreadId
 * 			The ETAL print thread ID, needed for deinitialization purposes.
 */
static OSAL_tThreadID ETAL_traceThreadId;
#endif // CONFIG_TRACE_ASYNC

#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_COMPONENT)
/***************************
 *
 * ETAL_tracePrintBuffer
 *
 **************************/
/*!
 * \brief		Print a buffer contents in hex format
 * \details		Prints a buffer, in multi-line format if so configured.
 *
 * 				The format of each line follows:
 * 				<prefix>_<counter> <data> [<TRUNC>]
 * 				<prefix> is the string passed in *prefix* parameter; if longer than
 * 				         #MAX_PREFIX_LEN the string "---" is printed instead.
 * 				         should be 3 characters to maintain alignment.
 * 				<counter> 1 digit from a modulo 10 counter incremented for
 * 				         every call to the function.
 *				<data>   the content of the *pBuffer* parameter: only 
 *				         #DATA_BYTES_PER_LINE are include in each printed line
 *				<TRUNC>  a string indicating that the buffer was not completely
 *				         printed (the final *pBuffer* data bytes are skipped)
 *				         This string is printed only after the last data
 *				         byte of the last output line.
 *
 *				The function outputs at most #MAX_LINES_PER_PRINT for
 *				each call
 * \param[in]	u32Level - the message level, see #OSALUTIL_s32TracePrintf
 * \param[in]	u32Class - the message class,  see #OSALUTIL_s32TracePrintf
 * \param[in]	prefix - a string that will be printed before all the lines;
 * 				         the string must be limited to MAX_PREFIX_LEN otherwise
 * 				         it will printed as '---'
 * \param[in]	pBuffer - the buffer to be printed
 * \param[in]	len - number of bytes in *pBuffer*
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
tVoid ETAL_tracePrintBuffer(tU32 u32Level, tU32 u32Class, tChar *prefix, tU8* pBuffer, tU32 len)
{
	/*! How many bytes to print on each line */
	const tU32 DATA_BYTES_PER_LINE = 12;
	/*! How many lines to print for each *pBuffer*;
	 *  set to >1 for multi-line print */
	const tU32 MAX_LINES_PER_PRINT = 2;

	/*! the max sixe of the *prefix* parameter.
	 *  Not configurable */
	const tU32 MAX_PREFIX_LEN = 3;
	/*! the number of digits from the counter to print */
	const tU32 STR_COUNTER_LEN = 1;
	/*! the size of the string appended to the last line to indicate
	 *  that the output is truncated.
	 *  Not configurable */
	const tU32 TRUNC_STRING_LEN = 5;

	const tU32 MAX_PRINTED_DATA_BYTES = DATA_BYTES_PER_LINE * MAX_LINES_PER_PRINT;
	const tU32 MAX_LINE = TRUNC_STRING_LEN  + 1 + MAX_PREFIX_LEN + 1 + STR_COUNTER_LEN + 1 + DATA_BYTES_PER_LINE * 3 + 1;
	tChar line[MAX_LINE];
	tU32 i, curr, overall;
	static tU32 str_count = 0;

	overall = 0;
	while ((overall < len) && (overall < MAX_PRINTED_DATA_BYTES))
	{
		curr = 0;

		/* since pBuffer is printed in pieces it is possible that due to
		 * a task switch it will not be printed all together.
		 * Add a one-digit counter in the print to ensure it will
		 * be possible to visually distinguish all the pieces
		 * making up the pBuffer */
		if ((tU32)OSAL_u32StringLength(prefix) > MAX_PREFIX_LEN)
		{
			OSAL_s32NPrintFormat(line + curr, MAX_LINE - curr, "---_%*d ", STR_COUNTER_LEN, str_count % 10);
			curr += MAX_PREFIX_LEN + 1 + STR_COUNTER_LEN + 1;
		}
		else
		{
			OSAL_s32NPrintFormat(line + curr, MAX_LINE - curr, "%s_%*d ", prefix, STR_COUNTER_LEN, str_count % 10);
			curr += OSAL_u32StringLength(prefix) + 1 + STR_COUNTER_LEN + 1;
		}

		for (i = 0; (overall < len) && (i < DATA_BYTES_PER_LINE); i++)
		{
			OSAL_s32NPrintFormat(line + curr, MAX_LINE - curr, "%.2x ", pBuffer[overall]);
			curr += 3;
			overall++;
		}

		/* print the TRUNC info, only on the last line of output */
		if ((len > MAX_PRINTED_DATA_BYTES) &&
			((overall >= len ) || (overall >= MAX_PRINTED_DATA_BYTES)))
		{
			OSAL_s32NPrintFormat(line + curr, MAX_LINE - curr, "TRUNC ");
			curr += TRUNC_STRING_LEN + 1;
		}

		line[curr + 1] = '\0';

		OSALUTIL_s32TracePrintf(0, u32Level, u32Class, line);
	}
	str_count++;
}
#endif // #if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_COMPONENT)

#if defined (CONFIG_TRACE_ASYNC)
/***************************
 *
 * ETAL_tracePrint
 *
 **************************/
/*!
 * \brief		Copies the string to be printed to the #etalTraceBuffer
 * \details		Function called by #OSALUTIL_s32TracePrintf instead of the plain printf
 * 				when CONFIG_TRACE_ASYNC is defined.
 *
 * 				To avoid disrupting the program flow with printf's this function
 * 				en-queues the message to be printed in an #etalTraceBuffer which is emptied
 * 				by a specific thread running at low priority.
 *
 * 				The #etalTraceBuffer is managed as a circular buffer through a 'last read'
 * 				and a 'last written' pointers:
 * 				- #etalTraceBufferLastWr points to the last FIFO location written by #ETAL_tracePrint
 * 				- #etalTraceBufferLastRd points to the last FIFO location read by #ETAL_tracePrint_ThreadEntry
 *
 * 				Both are initialized to an invalid value to distinguish the empty FIFO situation
 * 				from the FIFO startup situation since in the latter case the pointers must not
 * 				be advanced on the first call.
 *
 * 				The #etalTraceBufferLastWr should be protected by a semaphore because it may
 * 				be accessed by several threads at the same time: the function relies on the ostracesem_sem
 * 				taken by #OSALUTIL_s32TracePrintf before calling ETAL_tracePrint.
 * 				The #etalTraceBufferLastRd need not be protected because it is only
 * 				updated by the #ETAL_tracePrint_ThreadEntry.
 * \remark		In case of error (out of memory for dynamic memory allocation, or FIFO
 * 				buffer overflow) the function invokes directly the 'C' library printf
 * 				function to notify the user of the problem.
 * \remark		This function is available only if ETAL is built with CONFIG_TRACE_ASYNC.
 * \param[in]	u32Level - the message level (e.g. #TR_LEVEL_ERRORS)
 * \param[in]	u32Class - the message class (e.g. #TR_CLASS_APP_ETAL)
 * \param[in]	coszFormat - the message to be printed, a null-terminated string
 * \callgraph
 * \callergraph
 */
tVoid ETAL_tracePrint(tU32 u32Level, tU32 u32Class, tCString coszFormat)
{
	tU32 str_len;
	tU32 next_wridx;
#ifdef ETAL_TRACE_ASYNC_USES_MALLOC
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS)
	static tBool warning_issued = 0;
#endif
#endif

	if (etalTraceBufferLastWr == ETAL_TRACE_INIT_INDEX)
	{
		/* startup condition: instead of advancing the index, use the
		 * first FIFO location */
		next_wridx = 0;
	}
	else
	{
		next_wridx = (etalTraceBufferLastWr + 1) % ETAL_TRACE_FIFO_SIZE;
	}
	/* etalTraceBuffer full condition */
	if (next_wridx != etalTraceBufferLastRd)
	{
		str_len = (tU32)OSAL_u32StringLength(coszFormat) + 1; // consider terminating null char
		if (str_len > OSAL_C_U32_TRACE_MAX_MESSAGESIZE)
		{
			/* avoid overflowing EtalTraceElemTy.message for non-ETAL_TRACE_ASYNC_USES_MALLOC */
			str_len = OSAL_C_U32_TRACE_MAX_MESSAGESIZE;
		}
#ifdef ETAL_TRACE_ASYNC_USES_MALLOC
		etalTraceBuffer[next_wridx].message = (tChar *)OSAL_pvMemoryAllocate(str_len);

		if (etalTraceBuffer[next_wridx].message == NULL)
		{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS)
			if (!warning_issued)
			{
				warning_issued = 1;
				/* don't call OSALUTIL_s32TracePrintf or infinite recursion will happen */
				printf("------------------------------------------\n");
				printf("Memory allocation error in ETAL_tracePrint\n");
				printf("------------------------------------------\n");
			}
#endif
			ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, ETAL_INVALID_HANDLE,  sizeof(ETAL_HANDLE));
			/* behave as if the message was extracted */
			etalTraceBufferLastWr = next_wridx;
			return;
		}
		else
#endif // ETAL_TRACE_ASYNC_USES_MALLOC
		{
			etalTraceBuffer[next_wridx].timestamp = OSAL_ClockGetElapsedTime();
			etalTraceBuffer[next_wridx].level = u32Level;
			etalTraceBuffer[next_wridx].class = u32Class;
			OSAL_pvMemoryCopy((tVoid *)&etalTraceBuffer[next_wridx].message[0], (tPCVoid)coszFormat, str_len - 1);
			etalTraceBuffer[next_wridx].message[str_len - 1] = '\0';
			/* update the etalTraceBufferLastWr only after the message is written to
			 * avoid reading a partially updated location from ETAL_tracePrint_ThreadEntry */
			etalTraceBufferLastWr = next_wridx;
		}
	}
	else
	{
		etalTraceBufferOverflowCount++;
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS)
		/* don't call OSALUTIL_s32TracePrintf or infinite recursion will happen */
		printf("------------------------------------------------\n");
		printf("Overflow in ETAL trace (total %u strings lost)\n", etalTraceBufferOverflowCount);
		printf("------------------------------------------------\n");
#endif
	}
}

/***************************
 *
 * ETAL_tracePrint_ThreadEntry
 *
 **************************/
/*!
 * \brief		The ETAL print thread entry point
 * \details		Checks periodically the trace buffer for strings to print;
 * 				if there is one, prints it in small pieces to avoid locking
 * 				more important threads for too long.
 *
 * 				The function sleeps for #ETAL_TRACE_THREAD_SCHEDULING
 * 				between prints.
 *
 * 				Printing is performed through the 'C' library function printf.
 * \remark		This function is available only if ETAL is built with CONFIG_TRACE_ASYNC.
 * \param[in]	dummy - void pointer, unused
 * \see			#EtalTraceStateTy
 * \callgraph
 * \callergraph
 */
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETAL_tracePrint_ThreadEntry(tSInt stacd, tPVoid dummy)
#else
static tVoid ETAL_tracePrint_ThreadEntry(tVoid *dummy)
#endif
{
	EtalTraceStateTy state = ETAL_TraceState_Idle;
	EtalTraceElemTy *tb;
	tU32 message_index;
	tU32 message_len;
	tU32 message_substr_len;
	static tChar message_chunk[ETAL_TRACE_SUBSTRING_SIZE + 1]; // add the ending null char
	tBool do_sleep;

	while (TRUE)
	{
		do_sleep = TRUE;
		switch (state)
		{
			case ETAL_TraceState_Idle:
				/* etalTraceBuffer empty condition */
				if (etalTraceBufferLastRd == etalTraceBufferLastWr)
				{
					/* at startup both etalTraceBufferLastRd and etalTraceBufferLastWr
					 * are initialized to ETAL_TRACE_INIT_INDEX so we stay here
					 * also for the startup condition */
					etalTraceBufferEmpty = TRUE;
					break;
				}
				else
				{
					etalTraceBufferEmpty = FALSE;
					state = ETAL_TraceState_MessageAdvance;
					do_sleep = FALSE;
				}
				break;

			case ETAL_TraceState_MessageAdvance:
				if (etalTraceBufferLastRd == ETAL_TRACE_INIT_INDEX)
				{
					/* startup condition: instead of advancing the index, use the
					 * first FIFO location */
					etalTraceBufferLastRd = 0;
				}
				else
				{
					etalTraceBufferLastRd = (etalTraceBufferLastRd + 1) % ETAL_TRACE_FIFO_SIZE;
				}

				state = ETAL_TraceState_StartRead;
				do_sleep = FALSE;
				break;

			case ETAL_TraceState_StartRead:
				tb = &etalTraceBuffer[etalTraceBufferLastRd];
				message_index = 0; // count characters already printed from message
				message_len = 0;
#ifdef ETAL_TRACE_ASYNC_USES_MALLOC
				if (tb->message != NULL)
#else
				/* execute anyways because we have no way to distinguish an empty string 
				 * from an uninitialized entry */
#endif
				{
					message_len = (tU32)OSAL_u32StringLength(tb->message);

					state = ETAL_TraceState_PrintHeader;
					do_sleep = FALSE;
				}
#ifdef ETAL_TRACE_ASYNC_USES_MALLOC
				else
				{
					/* etalTraceBufferLastRd points to an empty FIFO location */
					/* this is abnormal, roll back the pointer to the empty buffer condition */
					ASSERT_ON_DEBUGGING(0);

					etalTraceBufferLastRd = etalTraceBufferLastWr;
					state = ETAL_TraceState_Idle;
				}
#endif
				break;

			case ETAL_TraceState_PrintHeader: /* printf done here */
				if (OSALUTIL_s32TracePrintHeader(tb->timestamp, tb->level, tb->class) == (tU8)0)
				{
					/* the header was not printed so no time was stolen to ETAL;
					 * can print the first message chunk already without sleeping
					 */
					do_sleep = FALSE;
				}

				state = ETAL_TraceState_PrintMessage;
				break;

			case ETAL_TraceState_PrintMessage: /* printf done here */
				if (message_index >= message_len)
				{
					printf("\n");
					(LINT_IGNORE_RET)fflush(NULL);

					do_sleep = FALSE;
					etalTraceBufferEmpty = TRUE;
					state = ETAL_TraceState_MessageComplete;
				}
				else
				{
					if ((message_index + ETAL_TRACE_SUBSTRING_SIZE) > message_len)
					{
						message_substr_len = message_len - message_index;
					}
					else
					{
						message_substr_len = ETAL_TRACE_SUBSTRING_SIZE;
					}
					OSAL_pvMemoryCopy((tVoid *)message_chunk, (tPCVoid)(tb->message + message_index), message_substr_len);
					message_chunk[message_substr_len] = '\0';
					message_index += message_substr_len;
					printf("%s", message_chunk);
				}
				break;

			case ETAL_TraceState_MessageComplete:
#ifdef ETAL_TRACE_ASYNC_USES_MALLOC
				OSAL_vMemoryFree(tb->message);
				tb->message = NULL;
#else
				tb->message[0] = '\0';
#endif

				state = ETAL_TraceState_Idle;
				do_sleep = FALSE;
				break;

		}

		if (do_sleep)
		{
			OSAL_s32ThreadWait(ETAL_TRACE_THREAD_SCHEDULING);
		}
	}
}
#endif // CONFIG_TRACE_ASYNC

/***************************
 *
 * ETAL_tracePrintInit
 *
 **************************/
/*!
 * \brief		Initializes the #etalTraceBuffer and creates the ETAL print thread
 * \details		Uses the global ETAL Status to initialize the trace level
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - error initializing the ETAL trace system (non-fatal)
 * \return		OSAL_ERROR_DEVICE_INIT - unable to create the Print thread
 * \callgraph
 * \callergraph
 */
tSInt ETAL_tracePrintInit(tVoid)
{
	tSInt retosal = OSAL_OK;
#if defined (CONFIG_TRACE_ENABLE)
#if defined (CONFIG_TRACE_ASYNC)
	OSAL_trThreadAttribute thread_attr;
#endif // CONFIG_TRACE_ASYNC

	if (ETAL_traceConfig(ETAL_statusHardwareAttrGetTraceConfig()) != ETAL_RET_SUCCESS)
	{
		retosal = OSAL_ERROR_INVALID_PARAM;
	}

#if defined (CONFIG_TRACE_ASYNC)
	/* these must be initialized before the ETAL_tracePrint_ThreadEntry is started! */
	OSAL_pvMemorySet((tVoid *)etalTraceBuffer, 0x00, sizeof(etalTraceBuffer));
	etalTraceBufferLastRd = ETAL_TRACE_INIT_INDEX;
	etalTraceBufferLastWr = ETAL_TRACE_INIT_INDEX;
	etalTraceBufferOverflowCount = 0;
	etalTraceBufferEmpty = TRUE;

	thread_attr.szName =       ETAL_TRACE_THREAD_NAME;
	thread_attr.u32Priority =  ETAL_TRACE_THREAD_PRIORITY;
	thread_attr.s32StackSize = ETAL_TRACE_STACK_SIZE;
	thread_attr.pfEntry = ETAL_tracePrint_ThreadEntry;
	thread_attr.pvArg = NULL;

	ETAL_traceThreadId = OSAL_ThreadSpawn(&thread_attr);
	if (ETAL_traceThreadId == OSAL_ERROR)
	{
		return OSAL_ERROR_DEVICE_INIT;
	}
#endif // CONFIG_TRACE_ASYNC
#endif // CONFIG_TRACE_ENABLE
	return retosal;
}

/***************************
 *
 * ETAL_tracePrintDeInit
 *
 **************************/
/*!
 * \brief		Destroys the Print thread
 * \details		The function allows some scheduling time to the 
 * 				#ETAL_tracePrint_ThreadEntry to flush the #etalTraceBuffer
 * 				to avoid loosing the last messages. To avoid locking the system
 * 				too long the time is set to 1s.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_tracePrintDeInit(tVoid)
{
#if defined (CONFIG_TRACE_ASYNC)
	OSAL_tMSecond end_time;

	/* Allow some scheduling time to the ETAL_tracePrint_ThreadEntry flush
	 * the etalTraceBuffer otherwise we loose the last messages
	 *
	 * But avoid locking the system for too long...
	 */
	end_time = OSAL_ClockGetElapsedTime() + 1000;
	while ((OSAL_ClockGetElapsedTime() < end_time) && !etalTraceBufferEmpty)
	{
		OSAL_s32ThreadWait(ETAL_TRACE_THREAD_SCHEDULING);
	}
	if (ETAL_traceThreadId != OSAL_ERROR)
	{
		(LINT_IGNORE_RET) OSAL_s32ThreadDelete(ETAL_traceThreadId);
	}
	ETAL_traceThreadId = OSAL_ERROR;
#endif // CONFIG_TRACE_ASYNC
}
#if !defined(CONFIG_HOST_OS_FREERTOS)
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM)
/***************************
 *
 * ETAL_Standard2Ascii
 *
 **************************/
/*!
 * \brief		Convert an #EtalBcastStandard to a string
 * \param[in]	std - the standard to convert
 * \return		A pointer to the string, or to "Illegal" if *std* does
 * 				not correspond to any known value.
 * \callgraph
 * \callergraph
 */
static tCString ETAL_Standard2Ascii(EtalBcastStandard std)
{
	switch (std)
	{
		case ETAL_BCAST_STD_UNDEF:
			return "Undefined";
		case ETAL_BCAST_STD_DRM:
			return "DRM";
		case ETAL_BCAST_STD_DAB:
			return "DAB";
		case ETAL_BCAST_STD_FM:
			return "FM";
		case ETAL_BCAST_STD_HD_FM:
			return "HDFM";
		case ETAL_BCAST_STD_HD_AM:
			return "HDAM";
		case ETAL_BCAST_STD_AM:
			return "AM";
		default:
			return "Illegal";
	}
}
#endif // CONFIG_HOST_OS_FREERTOS
#endif // CONFIG_TRACE_CLASS_ETAL && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM)

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
/***************************
 *
 * ETAL_cmdStatusToASCII_MDR
 *
 **************************/
/*!
 * \brief		Converts the MDR communication thread status to a string
 * \param[in]	cmd - the status to convert
 * \return		A pointer to the string, or to "Illegal" if *cmd* does
 * 				not correspond to any known state.
 * \see			ETAL_sendCommandWithRetryTo_MDR
 * \callgraph
 * \callergraph
 */
tCString ETAL_cmdStatusToASCII_MDR(etalToMDRCmdStatusTy cmd)
{
	switch (cmd)
	{
		case cmdStatusInitMDR:
			return "cmdStatusInit";
		case cmdStatusWaitNotificationMDR:
			return "cmdStatusWaitNotification";
		case cmdStatusWaitResponseMDR:
			return "cmdStatusWaitResponse";
		case cmdStatusCompleteMDR:
			return "cmdStatusComplete";
		case cmdStatusErrorMDR:
			return "cmdStatusError";
		default:
			return "Illegal";
	}
}
#endif // (CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
/***************************
 *
 * ETAL_MDRStatusToString
 *
 **************************/
/*!
 * \brief		Converts an MDR status to a string
 * \details		The MDR status is defined in the MDR specification and delivered
 * 				in Header Byte 1 of the MDR communication protocol.
 * \param[in]	status - the command status to be converted
 * \return		A pointer to the string, or to "Unknown status code" if *status* does
 * 				not correspond to any known value.
 * \callgraph
 * \callergraph
 */
tCString ETAL_MDRStatusToString(tU32 status)
{
	switch (status)
	{
		case DABMW_CMD_STATUS_OK:
			return "No Error";

		case DABMW_CMD_STATUS_ERR_TIMEOUT:
			return "Error Timeout";

		case DABMW_CMD_STATUS_ERR_HEADER0_FORMAT:
			return "Error Header 0 format";

		case DABMW_CMD_STATUS_ERR_QUEUE_FULL:
			return "Error Queue full";

		case DABMW_CMD_STATUS_ERR_PARAM_WRONG:
			return "Error Parameter wrong";

		case DABMW_CMD_STATUS_ERR_ILLEGAL_CMDNUM:
			return "Error Illegal command number";

		case DABMW_CMD_STATUS_ERR_RESERVED_CMDNUM:
			return "Error Reserved command number";

		case DABMW_CMD_STATUS_ERR_DUPLICATED_AUTO:
			return "Error Duplicated auto command";

		case DABMW_CMD_STATUS_ERR_PAYLOAD_TOO_LONG:
			return "Error Payload too long";

		case DABMW_CMD_STATUS_ERR_UNAVAILABLE_FNCT:
			return "Error Function not available";

		case DABMW_CMD_STATUS_ERR_WRONG_CMD_LENGHT:
			return "Error Wrong command length";

		case DABMW_CMD_STATUS_ERR_GENERIC_FAILURE:
			return "Error Generic failure";

		case DABMW_CMD_STATUS_ERR_FNCT_DISABLED:
			return "Error Function disabled";

		case DABMW_CMD_STATUS_ERR_APP_NOT_SUPPORTED:
			return "Error Application not supported";

		case DABMW_CMD_STATUS_ERR_WRONG_STOP_BIT:
			return "Error Wrong stop bit";

		case DABMW_CMD_STATUS_ERR_CMD_IS_NOT_ONGOING:
			return "Error Command is not ongoing";

		case DABMW_CMD_STATUS_ERR_RADIO_IS_NOT_ON:
			return "Error Radio is not on";

		case DABMW_CMD_STATUS_ERR_NO_MEMORY_AVAIL:
			return "Error No memory";

		case DABMW_CMD_STATUS_ERR_NVM_FAILED:
			return "Error NVM failed";

		case DABMW_CMD_STATUS_RSP_WITHOUT_PAYLOAD:
			return "Response without payload";

		case DABMW_CMD_STATUS_RSP_NO_DATA_AVAILABLE:
			return "Response no data available";

		case DABMW_CMD_STATUS_NO_RSP_TO_SEND:
			return "Response nothing to send";

		case DABMW_CMD_STATUS_OPERATION_ONGOING:
			return "Operation ongoing";

		case DABMW_CMD_STATUS_OPERATION_FAILED:
			return "Operation failed";

		case DABMW_CMD_STATUS_ONGOING_NO_RSP_TO_SEND:
			return "Ongoing no response to send";

		case DABMW_CMD_STATUS_OTHER_CMD_WAITING:
			return "Other command waiting";

		case DABMW_CMD_STATUS_LASTOPERATION_STOPPED:
			return "Last operation stopped";

		default:
			return "Unknown status code";
	}
}
#endif // CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
/***************************
 *
 * ETAL_MsgTypeToString
 *
 **************************/
/*!
 * \brief		Converts an #etalCallbackTy to a string
 * \param[in]	type - the type to convert
 * \return		A pointer to the string, or to "Illegal" if *type* does
 * 				not correspond to any known value.
 * \callgraph
 * \callergraph
 */
tCString ETAL_MsgTypeToString(etalCallbackTy type)
{
	switch (type)
	{
		case cbTypeEvent:
			return "Event";
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		case cbTypeQuality_MDR:
			return "Quality MDR";
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO) || defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
		case cbTypeQuality:
			return "Quality CMOST or DAB or HD";
#endif
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API
		default:
			ASSERT_ON_DEBUGGING(0);
			return "Unknown";
	}
}
#endif // CONFIG_TRACE_CLASS_ETAL && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

/***************************
 *
 * ETAL_EventToString
 *
 **************************/
/*!
 * \brief		Converts an #ETAL_EVENT to a string
 * \remark		Only used by the ETAL test environment.
 * \param[in]	ev - the event code to be converted.
 * \return		A pointer to the string, or to "Undefined event" if *ev* does
 * 				not correspond to any known value.
 * \callgraph
 * \callergraph
 */
tCString ETAL_EventToString(ETAL_EVENTS ev)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	switch (ev)
	{
		case ETAL_ERROR_COMM_FAILED:
			return "Error communication failed";
		case ETAL_INFO_TUNE:
			return "Info tune";
        case ETAL_INFO_SEEK:
            return "Info seek";
		case ETAL_INFO_RDS_SEEK:
			return "Info Rds seek";
        case ETAL_INFO_SCAN:
			return "Info scan";
		case ETAL_INFO_LEARN:
			return "Info learn";
		case ETAL_INFO_SEAMLESS_ESTIMATION_END:
			return "Info seamless estimation end";
		case ETAL_INFO_SEAMLESS_SWITCHING_END:
			return "Info seamless switching end";
		case ETAL_INFO_DAB_AUTONOTIFICATION:
			return "Info DAB auto-notification";
        case ETAL_RECEIVER_ALIVE_ERROR:
            return "Error Tuner Alive";
		case ETAL_INFO_TUNE_SERVICE_ID:
			return "Info Tune service ID";
		case ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO:
			return "Info service following";
		case ETAL_INFO_RDS_STRATEGY:
			return "Info RDS strategy";
		case ETAL_INFO_FM_STEREO:
			return "Info FM stereo";
		case ETAL_WARNING_OUT_OF_MEMORY:
			return "Out of memory";
		default:
			return "Undefined event";
	}
#else
	return "";
#endif
}

/***************************
 *
 * ETAL_STATUS_toString
 *
 **************************/
/*!
 * \brief		Convert an #ETAL_STATUS to a string
 * \param[in]	s - the status to be converted
 * \return		A pointer to the string, or to "Illegal" if *s* does
 * 				not correspond to any known value.
 * \callgraph
 * \callergraph
 */
tCString ETAL_STATUS_toString(ETAL_STATUS s)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	switch (s)
	{
		case ETAL_RET_SUCCESS:
			return "Success";
		case ETAL_RET_ERROR:
			return "Error";
		case ETAL_RET_ALREADY_INITIALIZED:
			return "Already Initialized";
		case ETAL_RET_DATAPATH_SINK_ERR:
			return "Datapath Sink error";
		case ETAL_RET_FRONTEND_LIST_ERR:
			return "Invalid Frontend List";
		case ETAL_RET_FRONTEND_NOT_AVAILABLE:
			return "Frontend Not Available";
		case ETAL_RET_INVALID_BCAST_STANDARD:
			return "Invalid Broadcast Standard";
		case ETAL_RET_INVALID_DATA_TYPE:
			return "Invalid Data Type";
		case ETAL_RET_INVALID_HANDLE:
			return "Invalid Handle";
		case ETAL_RET_INVALID_RECEIVER:
			return "Invalid Receiver";
		case ETAL_RET_NO_DATA:
			return "No Data";
		case ETAL_RET_NO_HW_MODULE:
			return "No HW Module";
		case ETAL_RET_NOT_INITIALIZED:
			return "Not Initialized";
		case ETAL_RET_NOT_IMPLEMENTED:
			return "Not Implemented";
		case ETAL_RET_PARAMETER_ERR:
			return "Parameter Error";
		case ETAL_RET_QUAL_CONTAINER_ERR:
			return "Quality Container Error";
		case ETAL_RET_QUAL_FE_ERR:
			return "Quality Frontend Error";
		case ETAL_RET_ALREADY_USED:
			return "Resource Already Configured";
		case ETAL_RET_IN_PROGRESS:
			return "Command in progress";
		default:
			return "Illegal";
	}
#else
	return "";
#endif
}


#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS)
/***************************
 *
 * ETAL_commErrToString
 *
 **************************/
/*!
 * \brief		Convert an error code to a printable string
 * \details		If the user does not register a notification handler, in case
 * 				of communication error ETAL prints a string containing the error
 * 				code and other information.
 * \param[in]	err - the error code
 * \return		The string or "Undefined error code" for unknown codes
 * \callgraph
 * \callergraph
 */
tCString ETAL_commErrToString(EtalCommErr err)
{
	switch (err)
	{
		case EtalCommStatus_NoError:
			return "No error";
		case EtalCommStatus_ChecksumError:
			return "Checksum error";
		case EtalCommStatus_TimeoutError:
			return "Timeout error";
		case EtalCommStatus_ProtocolHeaderError:
			return "Protocol header error";
		case EtalCommStatus_MessageFormatError:
			return "Message format error";
		case EtalCommStatus_BusError:
			return "Communication bus error";
		case EtalCommStatus_ProtocolContinuityError:
			return "Protocol continuity error";
		case EtalCommStatus_GenericError:
			return "Generic error";
		default:
			return "Undefined error code";
	}
}
#endif

#if !defined(CONFIG_HOST_OS_FREERTOS)
/***************************
 *
 * ETAL_tracePrintQuality
 *
 **************************/
/*!
 * \brief		Prints the content of a quality container
 * \details		A quality container is a general-purpose container
 * 				for quality measurements, valid for all Broadcast Standards.
 * 				The function uses the m_standard field to decide
 * 				which fields to print.
 *
 * 				The function is intended to be called only from within
 * 				ETAL CORE, in particular to print quality information
 * 				from the EVENT Trace function (see #ETAL_traceEvent).
 * \param[in]   level - the level of the message (see #OSALUTIL_s32TracePrintf)
 * \param[in]	q - pointer to a quality container
 * \callgraph
 * \callergraph
 */
tVoid ETAL_tracePrintQuality(tU32 level, EtalBcastQualityContainer *q)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM)
	tChar prefix[PREFIX_LEN];

	OSAL_szStringCopy(prefix, PREFIX0);

	ETAL_tracePrintApi(level, "%sQuality Container for %s", prefix, ETAL_Standard2Ascii(q->m_standard));
	ETAL_tracePrintApi(level, "%s", prefix);

	OSAL_szStringCopy(prefix, PREFIX1);
	switch (q->m_standard)
	{
		case ETAL_BCAST_STD_DAB:
			{
				EtalDabQualityEntries *qq = &(q->EtalQualityEntries.dab);
			    ETAL_tracePrintApi(level, "%sTimestamp:                      %ld ms", prefix, q->m_TimeStamp);
				ETAL_tracePrintApi(level, "%sRFFieldStrength:                %ld dBm", prefix, qq->m_RFFieldStrength);
				ETAL_tracePrintApi(level, "%sBBFieldStrength:                %ld", prefix, qq->m_BBFieldStrength);
				ETAL_tracePrintApi(level, "%sFicBitErrorRatio:               %d (*10-6)", prefix, qq->m_FicBitErrorRatio);
				ETAL_tracePrintApi(level, "%sisValidFicBitErrorRatio:        %d", prefix, qq->m_isValidFicBitErrorRatio);
				ETAL_tracePrintApi(level, "%sMscBitErrorRatio:               %d (*10-6)", prefix, qq->m_MscBitErrorRatio);
				ETAL_tracePrintApi(level, "%sisValidMscBitErrorRatio:        %d", prefix, qq->m_isValidMscBitErrorRatio);
				ETAL_tracePrintApi(level, "%sDataSubChBitErrorRatio:         %d (*10-6)", prefix, qq->m_dataSubChBitErrorRatio);
				ETAL_tracePrintApi(level, "%sisValidDataSubChBitErrorRatio:  %d", prefix, qq->m_isValidDataSubChBitErrorRatio);
				ETAL_tracePrintApi(level, "%sAudioSubChBitErrorRatio:        %d (*10-6)", prefix, qq->m_audioSubChBitErrorRatio);
				ETAL_tracePrintApi(level, "%sisValidAudioSubChBitErrorRatio: %d", prefix, qq->m_isValidAudioSubChBitErrorRatio);
				ETAL_tracePrintApi(level, "%sAudioBitErrorRatioLevel:        %d (0=bad-9=good)", prefix, qq->m_audioBitErrorRatioLevel);
				ETAL_tracePrintApi(level, "%sReedSolomonInformation:         %d", prefix, qq->m_reedSolomonInformation);
				ETAL_tracePrintApi(level, "%sSyncStatus:                     %d", prefix, qq->m_syncStatus);
				ETAL_tracePrintApi(level, "%sMuteFlag:                       %d", prefix, qq->m_muteFlag);
			}
			break;

		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
			{
				EtalFmQualityEntries *qq = &(q->EtalQualityEntries.amfm);
			    ETAL_tracePrintApi(level, "%sTimestamp:           %ld ms", prefix, q->m_TimeStamp);
				ETAL_tracePrintApi(level, "%sRFFieldStrength:     %ld dBuV", prefix, qq->m_RFFieldStrength);
				ETAL_tracePrintApi(level, "%sBBFieldStrength:     %ld dBuV", prefix, qq->m_BBFieldStrength);
				ETAL_tracePrintApi(level, "%sFrequencyOffset:     %lu Hz", prefix, qq->m_FrequencyOffset);
				ETAL_tracePrintApi(level, "%sModulationDetector:  %lu Hz", prefix, qq->m_ModulationDetector);
				ETAL_tracePrintApi(level, "%sMultipath:           %lu (in percentage)", prefix, qq->m_Multipath);
				ETAL_tracePrintApi(level, "%sUltrasonicNoise:     %lu (in percentage)", prefix, qq->m_UltrasonicNoise);
				ETAL_tracePrintApi(level, "%sAdjacentChannel:     %ld (log ratio)", prefix, qq->m_AdjacentChannel);
				ETAL_tracePrintApi(level, "%sSNR:                 %lu (0=bad-255=good)", prefix, qq->m_SNR);
				ETAL_tracePrintApi(level, "%scoChannel:           %lu (in percentage)", prefix, qq->m_coChannel);
				ETAL_tracePrintApi(level, "%sStereoMonoReception: %ld", prefix, qq->m_StereoMonoReception);
			}
			break;

		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
			{
				EtalHdQualityEntries *qq = &(q->EtalQualityEntries.hd);
				EtalFmQualityEntries *qqfm = &(q->EtalQualityEntries.hd.m_analogQualityEntries);

                ETAL_tracePrintApi(level, "%sTimestamp:        %ld ms", prefix, q->m_TimeStamp);
				ETAL_tracePrintApi(level, "%sisValidDigital:   %d", prefix, qq->m_isValidDigital);
				ETAL_tracePrintApi(level, "%sQI:               %d", prefix, qq->m_QI);
				ETAL_tracePrintApi(level, "%sCdToNo:           %d", prefix, qq->m_CdToNo);
				ETAL_tracePrintApi(level, "%sDSQM:             %.2f", prefix, qq->m_DSQM/ETAL_HDRADIO_DSQM_DIVISOR);
				ETAL_tracePrintApi(level, "%sAudioAlignment:   %d", prefix, qq->m_AudioAlignment);

				ETAL_tracePrintApi(level, "%sRFFieldStrength:     %ld dBuV", prefix, qqfm->m_RFFieldStrength);
				ETAL_tracePrintApi(level, "%sBBFieldStrength:     %ld dBuV", prefix, qqfm->m_BBFieldStrength);
				ETAL_tracePrintApi(level, "%sFrequencyOffset:     %lu Hz", prefix, qqfm->m_FrequencyOffset);
				ETAL_tracePrintApi(level, "%sModulationDetector:  %lu Hz", prefix, qqfm->m_ModulationDetector);
				ETAL_tracePrintApi(level, "%sMultipath:           %lu (in percentage)", prefix, qqfm->m_Multipath);
				ETAL_tracePrintApi(level, "%sUltrasonicNoise:     %lu (in percentage)", prefix, qqfm->m_UltrasonicNoise);
				ETAL_tracePrintApi(level, "%sAdjacentChannel:     %ld (log ratio)", prefix, qqfm->m_AdjacentChannel);
				ETAL_tracePrintApi(level, "%sSNR:                 %lu (0=bad-255=good)", prefix, qqfm->m_SNR);
				ETAL_tracePrintApi(level, "%scoChannel:           %lu (in percentage)", prefix, qqfm->m_coChannel);
				ETAL_tracePrintApi(level, "%sStereoMonoReception: %ld", prefix, qqfm->m_StereoMonoReception);
			}
			break;

		default:
			ETAL_tracePrintApi(level, "%sIllegal standard (%d)", prefix, q->m_standard);
			break;
	}
	ETAL_tracePrintApi(level, "%s", prefix);
#endif // CONFIG_TRACE_CLASS_ETAL && CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM
}

#else // CONFIG_HOST_OS_FREERTOS

tVoid ETAL_tracePrintQuality(tU32 level, EtalBcastQualityContainer *q)
{
}

#endif //CONFIG_HOST_OS_FREERTOS
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM)
/***************************
 *
 * ETAL_traceSeekStatusToString
 *
 **************************/
static tCString ETAL_traceSeekStatusToString(EtalSeekStatus *status)
{
	switch (status->m_status)
	{
		case ETAL_SEEK_STARTED:
			return "ETAL_SEEK_STARTED";
		case ETAL_SEEK_RESULT:
			return "ETAL_SEEK_RESULT";
		case ETAL_SEEK_FINISHED:
			return "ETAL_SEEK_FINISHED";
		default:
			return "ETAL_SEEK_ERROR";
	}
}

#if defined (CONFIG_ETAL_HAVE_ETALTML)

/***************************
 *
 * ETAL_traceScanStatusToString
 *
 **************************/
static tCString ETAL_traceScanStatusToString(EtalScanStatusTy *status)
{
	switch (status->m_status)
	{
		case ETAL_SCAN_STARTED:
			return "ETAL_SCAN_STARTED";
		case ETAL_SCAN_RESULT:
			return "ETAL_SCAN_RESULT";
		case ETAL_SCAN_FINISHED:
			return "ETAL_SCAN_FINISHED";
		default:
			return "ETAL_SCAN_ERROR";
	}
}

/***************************
 *
 * ETAL_traceLearnStatusToString
 *
 **************************/
static tCString ETAL_traceLearnStatusToString(EtalLearnStatusTy *status)
{
	switch (status->m_status)
	{
		case ETAL_LEARN_STARTED:
			return "ETAL_LEARN_STARTED";
		case ETAL_LEARN_RESULT:
			return "ETAL_LEARN_RESULT";
		case ETAL_LEARN_FINISHED:
			return "ETAL_LEARN_FINISHED";
		default:
			return "ETAL_LEARN_ERROR";
	}
}
#endif // CONFIG_ETAL_HAVE_ETALTML


/***************************
 *
 * ETAL_traceEvent
 *
 **************************/
/*!
 * \brief		Prints the description of an ETAL event
 * \details		Prints the decoded event name and parameters with
 * 				trace level #TR_LEVEL_SYSTEM. Quality events are ignored.
 * \param[in]	msg_type - the message type (quality or event)
 * \param[in]	event - the event
 * \param[in]	param - the event parameter
 * \callgraph
 * \callergraph
 */
tVoid ETAL_traceEvent(etalCallbackTy msg_type, ETAL_EVENTS event, tVoid *param)
{
	tChar str_evt[ETAL_TRACE_MAX_EVENT_STRING];
	tU32 i;	

	if (cbTypeEvent == msg_type)
	{

		/* prepare the event name (don't print yet) */
		(void)OSAL_s32NPrintFormat(str_evt, ETAL_TRACE_MAX_EVENT_STRING, "ETAL->APP EVENT %s ", ETAL_EventToString(event));

		/* special case: event with no parameter */
		if (param == NULL)
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s with NULL parameter", str_evt);
			goto exit;
		}


		/* event with parameter requires event-specific formatting */
		switch (event)
		{
			case ETAL_ERROR_COMM_FAILED:
			{
				EtalCommErrStatus *status = (EtalCommErrStatus *)param;

				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (comErr: %s, comErrRaw: %d, comErrRec: %d, comErrBufSiz: %d",
					str_evt,
					ETAL_commErrToString(status->m_commErr),
					status->m_commErrRaw,
					status->m_commErrReceiver,
					status->m_commErrBufferSize);
				// TODO use a more compact output format
				for (i = 0; i < (tU32)abs(status->m_commErrBufferSize); i++)
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "comErrBuf[%d]: %d", i, status->m_commErrBuffer[i]);
				}
			}
			break;

			case ETAL_INFO_TUNE:
			{
				EtalTuneStatus *status = (EtalTuneStatus *)param;

				if (ETAL_receiverGetStandard(status->m_receiverHandle) == ETAL_BCAST_STD_DAB)
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, stoFre: %d,  ServId = %d, syncFound : %d, synAcquired: %d, syncComplete: %d, muteStatus: %d)",
						str_evt,
						status->m_receiverHandle,
						status->m_stopFrequency,
						status->m_serviceId,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_DAB_FOUND) == ETAL_TUNESTATUS_SYNCMASK_DAB_FOUND,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_DAB_SYNC) == ETAL_TUNESTATUS_SYNCMASK_DAB_SYNC,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_DAB_MCI) == ETAL_TUNESTATUS_SYNCMASK_DAB_MCI,
						status->m_muteStatus);
				}
				else
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, stoFre: %d,  ServId = %d, syncFound : %d, syncNotFound : %d, synAcquired: %d, syncFailed = %d, SisAcquired: %d, SisFailed = %d, synHDAudioAcquired: %d, synHDAudioFailed: %d)",
						str_evt,
						status->m_receiverHandle,
						status->m_stopFrequency,
						status->m_serviceId,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND) == ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_SYNC_ACQUIRED) == ETAL_TUNESTATUS_SYNCMASK_SYNC_ACQUIRED,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_SYNC_FAILURE) == ETAL_TUNESTATUS_SYNCMASK_SYNC_FAILURE,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_SIS_ACQUIRED) == ETAL_TUNESTATUS_SYNCMASK_SIS_ACQUIRED,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_SIS_FAILED) == ETAL_TUNESTATUS_SYNCMASK_SIS_FAILED,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_COMPLETE) == ETAL_TUNESTATUS_SYNCMASK_COMPLETE,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED) == ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED);
				}
			}
			break;

			case ETAL_INFO_SEEK:
			case ETAL_INFO_RDS_SEEK:
			{
				EtalSeekStatus *status = (EtalSeekStatus *)param;

				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s - %s (recHan: %d, fre: %d, freFou: %d, HDProFou: %d, fulCycRea: %d, ServId: %d)",
					str_evt,
					ETAL_traceSeekStatusToString(status),
					status->m_receiverHandle,
					status->m_frequency,
					status->m_frequencyFound,
					status->m_HDProgramFound,
					status->m_fullCycleReached,
					status->m_serviceId);

				if ((status->m_status == ETAL_SEEK_FINISHED) || (status->m_status == ETAL_SEEK_RESULT))
				{
					ETAL_tracePrintQuality(TR_LEVEL_COMPONENT, &(status->m_quality));
				}
				else
				{
					// don't print quality
				}
			}
			break;

#if defined (CONFIG_ETAL_HAVE_ETALTML)
			case ETAL_INFO_SCAN:
			{
				EtalScanStatusTy *status = (EtalScanStatusTy *)param;

				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s - %s (recHan: %d, fre: %d, freFou: %d)",
					str_evt,
					ETAL_traceScanStatusToString(status),
					status->m_receiverHandle,
					status->m_frequency,
					status->m_frequencyFound);
			}
			break;

			case ETAL_INFO_LEARN:
			{
				EtalLearnStatusTy *status = (EtalLearnStatusTy *)param;

				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s - %s (recHan: %d, fre: %d, nbOfFre: %d)",
					str_evt,
					ETAL_traceLearnStatusToString(status),
					status->m_receiverHandle,
					status->m_frequency,
					status->m_nbOfFrequency);
				for (i = 0; i < status->m_nbOfFrequency; i++)
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Element[%d] : Freq: %d, Field Strength: %d, HDServiceFound: %d, ChannelID: %d",
							i, status->m_frequencyList[i].m_frequency, status->m_frequencyList[i].m_fieldStrength,
							status->m_frequencyList[i].m_HDServiceFound, status->m_frequencyList[i].m_ChannelID);
				}
			}
			break;

#endif // CONFIG_ETAL_HAVE_ETALTML

			case ETAL_INFO_SEAMLESS_ESTIMATION_END:
			{
				EtalSeamlessEstimationStatus *seamless_estimation_status = (EtalSeamlessEstimationStatus *)param;

				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (Status=%d, Provider type=%d, Absolute delay estimate=%d, Delay estimate=%d, TS FAS=%u, TS SAS=%u, Avg RMS FAS=%u, Avg RMS SAS=%u, Confidence level=%u)",
					str_evt,
					seamless_estimation_status->m_status,
					seamless_estimation_status->m_providerType,
					seamless_estimation_status->m_absoluteDelayEstimate,
					seamless_estimation_status->m_delayEstimate,
					seamless_estimation_status->m_timestamp_FAS,
					seamless_estimation_status->m_timestamp_SAS,
					seamless_estimation_status->m_RMS2_FAS,
					seamless_estimation_status->m_RMS2_SAS,
					seamless_estimation_status->m_confidenceLevel);
			}
			break;

			case ETAL_INFO_SEAMLESS_SWITCHING_END:
			{
				EtalSeamlessSwitchingStatus *seamless_switching_status = (EtalSeamlessSwitchingStatus *)param;

				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, sta: %d, absDelEst: %d)",
					str_evt,
					seamless_switching_status->m_receiverHandle,
					seamless_switching_status->m_status,
					seamless_switching_status->m_absoluteDelayEstimate);
			}
			break;

			case ETAL_INFO_DAB_AUTONOTIFICATION:
			{
				etalAutoNotificationStatusTy *autonotification_status = (etalAutoNotificationStatusTy *)param;

				if(autonotification_status->type == autoDABAnnouncementSwitching)
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, eveTyp=%d, annTyp=0x%x, sid= 0x%x, subChId=0x%x, regionId=0x%x, newFlag=%d)",
						str_evt,
						autonotification_status->receiverHandle,
						autonotification_status->type,
						autonotification_status->status.DABAnnouncement.announcementType,
						autonotification_status->status.DABAnnouncement.sid,
						autonotification_status->status.DABAnnouncement.subChId,
						autonotification_status->status.DABAnnouncement.regionId,
						autonotification_status->status.DABAnnouncement.newFlag);
				}
				else if(autonotification_status->type == autoDABAnnouncementSwitchingRaw)
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, eveTyp=%d, nbofItem=%d)",
						str_evt,
						autonotification_status->receiverHandle,
						autonotification_status->type,
						autonotification_status->status.DABAnnouncementRaw.m_nbOfItems);

                    for(i = 0; i < autonotification_status->status.DABAnnouncementRaw.m_nbOfItems; i++)
                    {
                        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Item[%d]: annTyp= 0x%x, clusterId=0x%x, subChId=0x%x, regionId=0x%x, alarmFlag=%d, newFlag=%d",
                                i,
                                autonotification_status->status.DABAnnouncementRaw.m_item[i].announcementType,
                                autonotification_status->status.DABAnnouncementRaw.m_item[i].clusterId,
                                autonotification_status->status.DABAnnouncementRaw.m_item[i].subChId,
                                autonotification_status->status.DABAnnouncementRaw.m_item[i].regionId,
                                autonotification_status->status.DABAnnouncementRaw.m_item[i].alarmFlag,
                                autonotification_status->status.DABAnnouncementRaw.m_item[i].newFlag);
                    }
				}
				else if(autonotification_status->type == autoDABReconfiguration)
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, eveTyp=%d, recTyp=0x%x, occTim=%d, cifCou=%d)",
						str_evt,
						autonotification_status->receiverHandle,
						autonotification_status->type,
						autonotification_status->status.DABReconfiguration.reconfigurationType,
						autonotification_status->status.DABReconfiguration.occurrenceTime,
						autonotification_status->status.DABReconfiguration.cifCounter);
				}
				else if(autonotification_status->type == autoDABDataStatus)
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, epgCom=%d)",
						str_evt,
						autonotification_status->receiverHandle,
						autonotification_status->type,
						autonotification_status->status.DABDataStatus.epgComplete);
				}
				else if(autonotification_status->type == autoDABStatus)
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, tunFre=%d, ber=%d, mut=0x%x, rea=Ox%x, rec=0x%x, sea=0x%X, syn=0x%x, traMod=0x%x)",
						str_evt,
						autonotification_status->receiverHandle,
						autonotification_status->status.DABStatus.tunedFrequency,
						autonotification_status->status.DABStatus.ber,
						autonotification_status->status.DABStatus.mute,
						autonotification_status->status.DABStatus.reason,
						autonotification_status->status.DABStatus.reconfiguration,
						autonotification_status->status.DABStatus.search,
						autonotification_status->status.DABStatus.sync,
						autonotification_status->status.DABStatus.transmissionMode);
				}
				else
				{
					/* unhandled announcement event type */
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, typEve=%d)", str_evt, autonotification_status->type);
				}
			}
			break;

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
			case ETAL_INFO_TUNE_SERVICE_ID:
			{
				EtalTuneServiceIdStatus *status = (EtalTuneServiceIdStatus *)param;

				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, isFouSta: %d, freIsDab: %d, fre: %d, SidPi: 0x%x, Ueid: 0x%x, AFIsAva: %d, ssAppOnAF: %d, AFIsSyn: %d)",
					str_evt,
					status->m_receiverHandle,
					status->m_IsFoundstatus,
					status->m_freqIsDab,
					status->m_freq,
					status->m_SidPi,
					status->m_Ueid,
					status->m_AFisAvailable,
					status->m_ssApplicableOnAF,
					status->m_AFisSync);
			}
			break;

			case ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO :
			{
				EtalTuneServiceIdStatus *status = (EtalTuneServiceIdStatus *)param;

				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (recHan: %d, isFouSta: %d, freIsDab: %d, fre: %d, SidPi: 0x%x, Ueid: 0x%x, AFIsAva: %d, ssAppOnAF: %d, AFIsSyn: %d)",
					str_evt,
					status->m_receiverHandle,
					status->m_IsFoundstatus,
					status->m_freqIsDab,
					status->m_freq,
					status->m_SidPi,
					status->m_Ueid,
					status->m_AFisAvailable,
					status->m_ssApplicableOnAF,
					status->m_AFisSync);
			}
			break;
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && define (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)

#if defined (CONFIG_ETAL_HAVE_ETALTML)
			case ETAL_INFO_RDS_STRATEGY:
			{
				EtalRDSStrategyStatus *status = (EtalRDSStrategyStatus *)param;
				
				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s (SwitchedFreq: %d, Bitmap : 0x%x)",
					str_evt,
					status->m_AFSwitchedFreq,
					status->m_RDSStrategyBitmap);			
			}
			break;
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML)
			
			default:
				/* unhandled event, or evnt with no parameters: just print the event name */
				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%s",
					str_evt);
				break;
		}
	}
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	else if (cbTypeQuality == msg_type)
	{
		etalQualityCbTy *pl_quality = (etalQualityCbTy *)param;

		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "QUALITY MONITOR (MonHan: %d)",  pl_quality->hMonitor);
		ETAL_tracePrintQuality(TR_LEVEL_SYSTEM, &(pl_quality->qual));
	}
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	else if (cbTypeQuality_MDR == msg_type)
	{
		etalQualityCbTy *pl_quality = (etalQualityCbTy *)param;

		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "QUALITY MONITOR (MonHan: %d)",  pl_quality->hMonitor);
		ETAL_tracePrintQuality(TR_LEVEL_SYSTEM, &(pl_quality->qual));
	}
#endif
	else
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Type of message unknown (decoding TBD)");
	}

exit:
	return;
}

/***************************
 *
 * ETAL_traceDLS
 *
 **************************/
/*!
 * \brief		Prints a DLS data container
 * \remark		The function produces output only if CONFIG_TRACE_CLASS_ETAL is TR_LEVEL_COMPONENT or greater
 * \param[in]	data - pointer to a DLS data container
 * \param[in]	len - lenght of DLS data
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_traceDLS(etalPADDLSTy *data,tU32 len)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "DLS data, Len: %d, Charset %d", len, data->m_charset);
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Radio text: %s\n", data->m_PAD_DLS);
#endif // defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
}

/***************************
 *
 * ETAL_traceDLPLUS
 *
 **************************/
/*!
 * \brief		Prints a DL PLUS data container
 * \remark		The function produces output only if CONFIG_TRACE_CLASS_ETAL is TR_LEVEL_COMPONENT or greater
 * \param[in]	data - pointer to a DL PLUS data container
 * \param[in]	len - lenght of DL PLUS data
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_traceDLPLUS(etalPADDLPLUSTy *data,tU32 len)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
	tU8 i;

	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "PAD DLPLUS data, Len: %d, NbOfItems: %d", len, data->m_nbOfItems);
	for(i = 0; i < data->m_nbOfItems; i++)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "PAD DLPLUS data: Item Nb %d :\n", i);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "PAD DLPLUS data: Content type: 0x%x\n", data->m_item[i].m_contentType);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "PAD DLPLUS data: running status: 0x%x\n", data->m_item[i].m_runningStatus);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "PAD DLPLUS data: Charset: %d\n", data->m_item[i].m_charset);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "PAD DLPLUS data: Label length: %d\n", data->m_item[i].m_labelLength);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "PAD DLPLUS data: Label: %s\n", data->m_item[i].m_label);
	}
#endif // defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
}

/***************************
 *
 * ETAL_traceRDSRaw
 *
 **************************/
/*!
 * \brief		Prints a RAW RDS data container
 * \remark		The function produces output only if CONFIG_TRACE_CLASS_ETAL is TR_LEVEL_USER_4 or greater
 * \param[in]	rds - pointer to a RAW RDS data container
 * \param[in]	len - lenght of rds parameter
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_traceRDSRaw(EtalRDSRawData *rds,tU32 len)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_1)
	tU32 RNR, i, raw_rds_data;

	if (len >= 6)
	{
		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "RDS Raw data, Len: %d", len);

		ETAL_getParameter_CMOST(rds->m_RNR, &RNR);

		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "RNR: %s %s %s %s %s %s", ((RNR & STAR_RNR_DATARDY)?"DATARDY":"       "), ((RNR & STAR_RNR_SYNC)?"SYNC":"    "), 
			((RNR & STAR_RNR_BOFL)?"BOFL":"    "), ((RNR & STAR_RNR_BNE)?"BNE":"   "), ((RNR & STAR_RNR_TUNCH2)?"TUNCH2":"      "), 
			((RNR & STAR_RNR_TUNCH1)?"TUNCH1":"      "));

		for(i = 0; i < (len - 3); i += 3)
		{
			ETAL_getParameter_CMOST(&(rds->m_RDS_Data[i]), &raw_rds_data);

			ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ERRCOUNT: %d, RDS block offset %c%c, RDS raw data: 0x%04X", ((raw_rds_data & START_RDS_DATA_ERRCOUNT_MASK) >> START_RDS_DATA_ERRCOUNT_SHIFT),
				((raw_rds_data & START_RDS_DATA_BLOCKID_MASK) >> START_RDS_DATA_BLOCKID_SHIFT) + 65,
				((raw_rds_data & START_RDS_DATA_CTYPE_MASK) >> START_RDS_DATA_CTYPE_SHIFT)?'\'':' ',
				(raw_rds_data & START_RDS_DATA_MASK));
		}
	}
#endif // defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_1)
}

#if defined (CONFIG_ETAL_HAVE_ETALTML)
/***************************
 *
 * ETAL_traceRDSData
 *
 **************************/
/*!
 * \brief		Prints a decoded RDS data container
 * \param[in]	rds - pointer to a decoded RDS data container
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_traceRDSData(EtalRDSData *rds, tPChar prefix)
{
	tU32 i;

	if (rds->m_validityBitmap == 0)
	{
		return;
	}
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sRDS decoded data 0x%03x", prefix, rds->m_validityBitmap);
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sRDS stored  data 0x%03x", prefix, rds->m_storedInfoBitmap);

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sPI          : 0x%x", prefix, rds->m_PI);
	}
	else if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sPI Stored   : 0x%x", prefix, rds->m_PI);
	}
		
	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sPS          : %.*s", prefix, ETAL_DEF_MAX_PS_LEN, rds->m_PS);
	}
	else if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sPS Stored    : %.*s", prefix, ETAL_DEF_MAX_PS_LEN, rds->m_PS);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sRT          : %.*s", prefix, ETAL_DEF_MAX_RT_LEN, rds->m_RT);
	}	
	else if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sRT Stored  : %.*s", prefix, ETAL_DEF_MAX_RT_LEN, rds->m_RT);
	}	
	
	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_DI) == ETAL_DECODED_RDS_VALID_DI)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sDI          : 0x%x", prefix, rds->m_DI);
	}
	else if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_DI) == ETAL_DECODED_RDS_VALID_DI)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sDI Stored   : 0x%x", prefix, rds->m_DI);
	}


	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TOM) == ETAL_DECODED_RDS_VALID_TOM)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTime(hour)  : %d", prefix, rds->m_timeHour);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTime(min)   : %d", prefix, rds->m_timeMinutes);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTime(offs)  : %d", prefix, rds->m_offset);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTime(MJD)   : %d", prefix, rds->m_MJD);
	}
	else if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_TOM) == ETAL_DECODED_RDS_VALID_TOM)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTime(hour) Stored  : %d", prefix, rds->m_timeHour);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTime(min)  Stored  : %d", prefix, rds->m_timeMinutes);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTime(offs) Stored  : %d", prefix, rds->m_offset);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTime(MJD)  Stored  : %d", prefix, rds->m_MJD);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_AF) == ETAL_DECODED_RDS_VALID_AF)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sAFListPI    : 0x%x", prefix, rds->m_AFListPI);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sAFListLen   : %d", prefix, rds->m_AFListLen);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sAFList      : ", prefix);
		for (i = 0; i < rds->m_AFListLen; i++)
		{
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%s\t%6d MHz", prefix, 87500 + 100 * (tU32)rds->m_AFList[i]);
		}
	}
	else if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_AF) == ETAL_DECODED_RDS_VALID_AF)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sAFListPI Stored    : 0x%x", prefix, rds->m_AFListPI);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sAFListLen Stored   : %d", prefix, rds->m_AFListLen);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sAFList Stored      : ", prefix);
		for (i = 0; i < rds->m_AFListLen; i++)
		{
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sStored\t%6d MHz", prefix, 87500 + 100 * (tU32)rds->m_AFList[i]);
		}
	}
	
	
	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PTY) == ETAL_DECODED_RDS_VALID_PTY)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sPTY         : %d", prefix, rds->m_PTY);
	}
	else if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_PTY) == ETAL_DECODED_RDS_VALID_PTY)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sPTY Stored  : %d", prefix, rds->m_PTY);
	}
	
	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TP) == ETAL_DECODED_RDS_VALID_TP)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTP          : %d", prefix, rds->m_TP);
	}
	else 	if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_TP) == ETAL_DECODED_RDS_VALID_TP)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTP Stored   : %d", prefix, rds->m_TP);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TA) == ETAL_DECODED_RDS_VALID_TA)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTA          : %d", prefix, rds->m_TA);
	}
	else	if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_TA) == ETAL_DECODED_RDS_VALID_TA)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sTA Stored   : %d", prefix, rds->m_TA);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_MS) == ETAL_DECODED_RDS_VALID_MS)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sMS          : %d", prefix, rds->m_MS);
	}
	else if ((rds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_MS) == ETAL_DECODED_RDS_VALID_MS)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sMS Stored   : %d", prefix, rds->m_MS);
	}
	
}

/***************************
 *
 * ETAL_traceTextInfo
 *
 **************************/
/*!
 * \brief		Prints a RadioText (i.e. TextInfo) container
 * \remark		The function prints the fields only if they are flagged
 * 				as new. If there is nothing new, it prints "Nothing new".
 * \param[in]	radio_text - pointer to a TextInfo container
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_traceTextInfo(EtalTextInfo *radio_text)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sRadio text:\n", PREFIX0);
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sBroadcast Standard: %s\n", PREFIX1, ETAL_Standard2Ascii(radio_text->m_broadcastStandard));
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sService name %s: %.*s\n", PREFIX1, 
			((radio_text->m_serviceNameIsNew == TRUE)?"is new   ":((radio_text->m_serviceNameIsStored == TRUE)?"is stored":"          ")), 
						ETAL_DEF_MAX_SERVICENAME, radio_text->m_serviceName);
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "%sCurrent Info %s: %s\n", PREFIX1, 
			((radio_text->m_currentInfoIsNew == TRUE)?"is new   ":((radio_text->m_currentInfoIsStored == TRUE)?"is stored":"          ")),
			radio_text->m_currentInfo);
#endif // defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
}
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML)

/***************************
 *
 * ETAL_traceDataPath
 *
 **************************/
/*!
 * \brief		Prints the description of an ETAL datapath
 * \details		Prints the decoded datapath parameters with
 * 				trace level #TR_LEVEL_SYSTEM. Some datapath are ignored.
 * \param[in]	msg_type - the message type (quality or event)
 * \param[in]	event - the event
 * \param[in]	param - the event parameter
 * \callgraph
 * \callergraph
 */
tVoid ETAL_traceDataPath(ETAL_HANDLE hDatapath, tVoid *param, tU32 param_size, EtalDataBlockStatusTy *status)
{
	EtalBcastDataType datapathDataType;

	if (ETAL_handleIsDatapath(hDatapath))
	{
		datapathDataType = ETAL_receiverGetDataTypeForDatapath(hDatapath);
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP DataPath hDatapath 0x%x type 0x%x param_size %d", hDatapath, datapathDataType, param_size);
		switch (datapathDataType)
		{
			case ETAL_DATA_TYPE_AUDIO:
			case ETAL_DATA_TYPE_DCOP_AUDIO:
			case ETAL_DATA_TYPE_DATA_SERVICE:
			case ETAL_DATA_TYPE_DAB_DATA_RAW:
			case ETAL_DATA_TYPE_DAB_AUDIO_RAW:
			case ETAL_DATA_TYPE_DAB_FIC:
				break;

#if defined (CONFIG_ETAL_HAVE_ETALTML)
			case ETAL_DATA_TYPE_TEXTINFO:
				if (param != NULL)
				{
					ETAL_traceTextInfo((EtalTextInfo *)param);
				}
				else
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "param = NULL");
				}
				break;

			case ETAL_DATA_TYPE_FM_RDS:
				if (param != NULL)
				{
					ETAL_traceRDSData((EtalRDSData *)param, "FM_RDS_DATAPATH/");
				}
				else
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "param = NULL");
				}
				break;
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML)

			case ETAL_DATA_TYPE_DAB_DLS:
				if (param != NULL)
				{
					ETAL_traceDLS((etalPADDLSTy *)param, param_size);
				}
				else
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "param = NULL");
				}
				break;

			case ETAL_DATA_TYPE_DAB_DLPLUS:
				if (param != NULL)
				{
					ETAL_traceDLPLUS((etalPADDLPLUSTy *)param, param_size);
				}
				else
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "param = NULL");
				}
				break;

			case ETAL_DATA_TYPE_FM_RDS_RAW:
				if (param != NULL)
				{
					ETAL_traceRDSRaw((EtalRDSRawData *)param, param_size);
				}
				else
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "param = NULL");
				}
				break;

			case ETAL_DATA_TYPE_UNDEF:
			default:
				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "invalid datapath type 0x%x", datapathDataType);
				break;
		}
	}
	else
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP DataPath invalid hDatapath 0x%x", hDatapath);
	}
}

#endif // CONFIG_TRACE_CLASS_ETAL && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM)


/***************************
 *
 * ETAL_tracePrintAsciiLUN
 *
 **************************/
/*!
 * \brief		Extracts the contents receives on the MDR ASCII LUN and prints it
 * \details		The DAB DCOP (MDR) uses the LUN concept to distinguish between classes
 * 				of messages. This function processes messages received on the ASCII LUN
 * 				(typically diagnostic messages).
 *
 * 				The function compares the level of the message, embedded in the message
 * 				header, with the minimum level set by build configuration (CONFIG_TRACE_CLASS_EXTERNAL)
 * 				and prints the message only if it is greater or equal than that level.
 *
 * 				If the message is longer than the size of a local buffer (#ETAL_DABMW_DATA_PROTOCOL_ASCII_MSG_SIZE)
 * 				the message is truncated to that size.
 *
 * 				The message is prefixed with a fixed string: "DIRECT MESSAGE FROM MDR3 => ".
 * \param[in]	buf - a buffer containing the complete message received from the DAB DCOP
 * \return		OSAL_OK - the message was printed (possibly truncated) or the message level
 * 				          was lower than the configured minimum CONFIG_TRACE_CLASS_EXTERNAL
 * 				          thus it was ignored.
 * \return		OSAL_ERROR - the message header does not indicate an ASCII LUN message,
 *				             the message is ignored
 * \see			ETAL_CommunicationLayer_ReceiveData_MDR
 * \callgraph
 * \callergraph
 */
tSInt ETAL_tracePrintAsciiLUN(tU8 *buf)
{
#if defined(CONFIG_TRACE_CLASS_EXTERNAL)
	tU32 level;
	tU32 size;
	static tChar szBuffer[ETAL_DABMW_DATA_PROTOCOL_ASCII_MSG_SIZE];
	tChar prefix[] = "DIRECT MESSAGE FROM MDR3 => ";
	tU32 prefix_len;
#endif
	tSInt retval = OSAL_OK;

	if ((buf[0] != (tU8)0xD6) || (buf[1] != (tU8)0x84))
	{
		retval = OSAL_ERROR;
		goto exit;
	}
#if defined(CONFIG_TRACE_CLASS_EXTERNAL)
	prefix_len = (tU32)OSAL_u32StringLength(prefix);
	level = ETAL_DABMW_DATA_CHANNEL_ASCII_LEVEL(buf);
	size = ETAL_DABMW_DATA_CHANNEL_ASCII_SIZE(buf);
	
	if (level > CONFIG_TRACE_CLASS_EXTERNAL)
	{
		retval = OSAL_OK;
		goto exit;
	}
	if (size > (tU32)ETAL_DABMW_DATA_PROTOCOL_ASCII_MSG_SIZE - prefix_len - 1)
	{
		size = ETAL_DABMW_DATA_PROTOCOL_ASCII_MSG_SIZE - prefix_len - 1;
	}
	(void)OSAL_pvMemoryCopy((tVoid *)szBuffer, (tPCVoid)prefix, prefix_len);
	(void)OSAL_pvMemoryCopy((tVoid *)(szBuffer + prefix_len), (tPCVoid)ETAL_DABMW_DATA_CHANNEL_ASCII_PAYLOAD(buf), size);
	szBuffer[prefix_len + size] = '\0';
	(void)OSALUTIL_s32TracePrintf(0, level, TR_CLASS_EXTERNAL, (tCString)szBuffer);	
#endif

exit:	
	return retval;
}



#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_tracePrintVersion
 *
 **************************/
tVoid ETAL_tracePrintVersion(EtalVersion *version)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	tU32 vl_index;

	ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "---------- ETAL versions -------------"	);
	ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "\t ETAL     : %s ", version->m_ETAL.m_name);
	
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	for (vl_index = 0; vl_index < ETAL_CAPA_MAX_TUNER; vl_index++)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "\t CMOST(%d) : %s ", vl_index+1, version->m_CMOST[vl_index].m_name);
	}
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "\t DCOP DAB : %s ", version->m_MDR.m_name);
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "\t DCOP HD :  %s ", version->m_HDRadio.m_name);
#endif

	ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "--------------------------------------");
#endif // CONFIG_TRACE_CLASS_ETAL && CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS
}

/***************************
 *
 * ETAL_tracePrintHwVersion
 *
 **************************/
tVoid ETAL_tracePrintHwVersion(tVoid)
{

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	tU32 vl_index;
	EtalInitStatus vl_InitStatus;

	ETAL_initStatusGet(&vl_InitStatus);

	ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "---------- Hw versions -------------"	);

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	for (vl_index = 0; vl_index < ETAL_CAPA_MAX_TUNER; vl_index++)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "\t CMOST(%d) : %s ", vl_index+1, vl_InitStatus.m_tunerStatus[vl_index].m_detectedSilicon);
	}
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "\t DCOP DAB : %s ", vl_InitStatus.m_DCOPStatus.m_detectedSilicon);
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "\t DCOP HD :  %s ", vl_InitStatus.m_DCOPStatus.m_detectedSilicon);
#endif

	ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "--------------------------------------");
#endif // CONFIG_TRACE_CLASS_ETAL && CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS
}

#endif // CONFIG_ETAL_HAVE_GET_VERSION || CONFIG_ETAL_HAVE_ALL_API

