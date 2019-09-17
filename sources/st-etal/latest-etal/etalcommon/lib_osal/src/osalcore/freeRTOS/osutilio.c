//!
//!  \file      osutilio.c
//!  \brief     <i><b>OSAL IO utilities Functions</b></i>
//!  \details   This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) IO utilities Functions.
//!

/* DeactivateLintMessage_ID0026 */
#ifndef OSUTILIO_C
#define OSUTILIO_C

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************/
/* includes                                                               */
/**************************************************************************/
#include "target_config.h"
#include "printf-stdarg.h"
#if defined (CONFIG_TRACE_ENABLE)
#include "osal.h"

#ifndef CONFIG_TRACE_ASYNC
	#define headerBufferLen 100
#endif
	
// Local to manage the syslog logging

#ifdef CONFIG_TRACE_ETAL_SYSLOG 
#include "syslog.h"

// add syslog try
#define OS_PRINTF(level, ...) \
	{\
	syslog(((level==TR_LEVEL_FATAL)?LOG_ERR:((level==TR_LEVEL_ERRORS)?LOG_WARNING:((level<TR_LEVEL_USER_1)?LOG_INFO:LOG_DEBUG))),__VA_ARGS__);\
	}
#else
#define OS_PRINTF(...) (trace_printf(__VA_ARGS__))
#endif 

// ***************
// local functions
// **************
#ifndef CONFIG_TRACE_ASYNC
//static tU8 OSALUTIL_s32TracePrintHeader(tU32 u32Level, tU32 u32Class, tPChar pO_Header, tU8 vI_HeaderBufferLen);
#endif


/**
 * @brief     OSALUTIL_s32TraceSetOutputDriverCallback
 *
 * @details   set the output driver callback
 *
 * @param     pointer to callback function
 *
 * @return
 *            void
 *
 */
static OSALUTIL_vOutputDriverCallbackType osutil_trace_output_driver = OSAL_NULL;

tVoid OSALUTIL_vTraceSetOutputDriverCallback( OSALUTIL_vOutputDriverCallbackType cb )
{
    if( osutil_trace_output_driver == OSAL_NULL )
    {
        osutil_trace_output_driver = cb;
    }
}

/**
 * @brief     OSALUTIL_s32TraceDisablePrintfOutput
 *
 * @details   disable the "printf" output
 *
 */
static tBool OSALUTIL_PrintfEnabled = TRUE;
tVoid OSALUTIL_s32TraceDisablePrintfOutput( void )
{
    OSALUTIL_PrintfEnabled = FALSE;
}

static OSAL_tSemHandle ostracesem_sem = (OSAL_tSemHandle)NULL;

#ifndef CONFIG_OSUTIL_TRACE_NUM_FILTERS
#define CONFIG_OSUTIL_TRACE_NUM_FILTERS 8
#endif /* ! CONFIG_OSUTIL_TRACE_NUM_FILTERS */

static struct {
    /* default trace level for any trace class:
     * any trace message with a level <= default_level
     * will be logged.
     */
    tU32  u32DefaultLevel;
	/* if TRUE the TracePrintf will not issue the "time:level:class" header */
	tBool boolOmitHeader;
#if (CONFIG_OSUTIL_TRACE_NUM_FILTERS != 0)
    tU32  t32NumUsedFilters;
    struct {
        tU32 u32Class;
        tU32 u32Mask;
        tU32 u32Level;
    } tTraceFilter[CONFIG_OSUTIL_TRACE_NUM_FILTERS];
#endif
} tOSTraceConfig;

tBool
OSALUTIL_s32TraceCheckFilter(tU32 u32Level, tU32 u32Class)
{
	tBool ret;
	
  if (u32Level <= tOSTraceConfig.u32DefaultLevel)
    {
      ret = TRUE;
      goto exit;
    }
#ifdef CONFIG_OSUTIL_TRACE_NUM_FILTERS
#if (CONFIG_OSUTIL_TRACE_NUM_FILTERS != 0)
  else
    {
      int i;

      if(tOSTraceConfig.t32NumUsedFilters == 0)
        {
          ret = FALSE;
          goto exit;
        }

      for (i = 0; i < CONFIG_OSUTIL_TRACE_NUM_FILTERS; i++)
        {
          if (tOSTraceConfig.tTraceFilter[i].u32Class != 0)
            {
              if ((u32Class & tOSTraceConfig.tTraceFilter[i].u32Mask) ==
                      (tOSTraceConfig.tTraceFilter[i].u32Class & tOSTraceConfig.tTraceFilter[i].u32Mask))
                {
                  if (u32Level <= tOSTraceConfig.tTraceFilter[i].u32Level)
                    {
                      ret = TRUE;
                      goto exit;
                    }
                }
            }
        }
    }
#endif

#else
  (void) u32Class; /* make Lint happy */
#endif
  ret = FALSE;

exit:
	return ret;
}

tVoid
OSALUTIL_vTraceSetDefaultLevel(tU32 u32Level)
{
  tOSTraceConfig.u32DefaultLevel = u32Level;
}

tVoid
OSALUTIL_vTraceDisableHeader(tVoid)
{
  tOSTraceConfig.boolOmitHeader = TRUE;
}

tVoid
OSALUTIL_vTraceInit(tVoid)
{
	(void)OSAL_pvMemorySet((tVoid *)&tOSTraceConfig, 0x00, sizeof(tOSTraceConfig));
#ifdef CONFIG_TRACE_DEFAULT_LEVEL
	tOSTraceConfig.u32DefaultLevel = CONFIG_TRACE_DEFAULT_LEVEL;
#else
	tOSTraceConfig.u32DefaultLevel = TR_LEVEL_SYSTEM_MIN;
#endif
}


tVoid
OSALUTIL_vTraceDeInit(tVoid)
{
	// nothing specific.
	
}

tVoid OSALUTIL_s32TraceSetFilterWithMask( tU32 u32Class, tU32 u32Mask, tU32 u32Level )
{
#ifdef CONFIG_OSUTIL_TRACE_NUM_FILTERS
  int i;
  if(u32Class == 0)
    {
      goto exit;
    }

  /* search for class */
  for (i = 0; i < CONFIG_OSUTIL_TRACE_NUM_FILTERS; i++)
    {
      if (u32Class == tOSTraceConfig.tTraceFilter[i].u32Class)
        {
          if (u32Level != tOSTraceConfig.tTraceFilter[i].u32Level)
            {
              /* just change the level for the class */
              tOSTraceConfig.tTraceFilter[i].u32Mask = u32Mask;
              tOSTraceConfig.tTraceFilter[i].u32Level = u32Level;
              if(u32Level != 0)
                {
                  tOSTraceConfig.t32NumUsedFilters++;
                }
              else
                {
                  tOSTraceConfig.t32NumUsedFilters--;
                  tOSTraceConfig.tTraceFilter[i].u32Class = 0;
                  tOSTraceConfig.tTraceFilter[i].u32Level = 0;
                  tOSTraceConfig.tTraceFilter[i].u32Mask = 0;
                }
              goto exit;
            }
        }
    }
  /* add class in table */
  for (i = 0; i < CONFIG_OSUTIL_TRACE_NUM_FILTERS; i++)
    {
      if (tOSTraceConfig.tTraceFilter[i].u32Level == 0)
        {
          tOSTraceConfig.tTraceFilter[i].u32Class = u32Class;
          tOSTraceConfig.tTraceFilter[i].u32Mask = u32Mask;
          tOSTraceConfig.tTraceFilter[i].u32Level = u32Level;
          tOSTraceConfig.t32NumUsedFilters++;
          goto exit;
        }
    }

exit:
	return;
   
  /* no space left in table */
#else
  (void) u32Class; /* make Lint happy */
  (void) u32Mask;  /* make Lint happy */
  (void) u32Level; /* make Lint happy */
#endif
}

tU32 OSALUTIL_s32GetFilterLevel(tU32 u32Class)
{
    int i;
    tU32 ret = TR_LEVEL_FATAL;
    for (i = 0; i < CONFIG_OSUTIL_TRACE_NUM_FILTERS; i++)
    {
        if (tOSTraceConfig.tTraceFilter[i].u32Class == u32Class)
        {
            ret = tOSTraceConfig.tTraceFilter[i].u32Level;
            goto exit;
        }
    }

exit:
    return ret;
}

tVoid OSALUTIL_s32TraceClearFilter( void )
{
#ifdef CONFIG_OSUTIL_TRACE_NUM_FILTERS
  int i;
  tOSTraceConfig.t32NumUsedFilters=0;
  for (i = 0; i < CONFIG_OSUTIL_TRACE_NUM_FILTERS; i++)
  {
      tOSTraceConfig.tTraceFilter[i].u32Class = 0;
      tOSTraceConfig.tTraceFilter[i].u32Level = 0;
      tOSTraceConfig.tTraceFilter[i].u32Mask = 0;
  }
#endif /* CONFIG_OSUTIL_TRACE_NUM_FILTERS */
}

tVoid
OSALUTIL_s32TraceSetFilter(tU32 u32Class, tU32 u32Level)
{
    OSALUTIL_s32TraceSetFilterWithMask(u32Class,0xffffffffL,u32Level);
}
#if 0
#ifdef CONFIG_TRACE_ASYNC
tU8 OSALUTIL_s32TracePrintHeader(OSAL_tMSecond t, tU32 u32Level, tU32 u32Class)
#else
static tU8 OSALUTIL_s32TracePrintHeader(tU32 u32Level, tU32 u32Class, tPChar pO_Header, tU8 vI_HeaderBufferLen)
#endif
{
  int m, s, ms;
  tU32 u32temp;
	tU8 u8Temp = 0;
	  
#ifdef CONFIG_TRACE_ASYNC
	tChar pO_Header[100];
	tU8 vI_HeaderBufferLen = 100;
#endif 

	tU8 vl_len = 0;

	if (tOSTraceConfig.boolOmitHeader || (pO_Header == OSAL_NULL))
	{
		vl_len = 0;
		goto exit;
	}

    /* print time */
    {
#ifndef CONFIG_TRACE_ASYNC
        OSAL_tMSecond t = OSAL_ClockGetElapsedTime();
#endif
 
         if (t < 1000)
        {
            m = 0;
            s = 0;
            ms = (int)t;
        }
        else if (t < 60000)
        {
            m = 0;
            s  = (int)t / 1000;
            ms = (int)t - (s * 1000);
        }
        else
        {
            m  = (int)t / 60000;
			u32temp = t - (m * 60000);
            s  = (int)(u32temp) / 1000;
            ms = (int)t - (s * 1000) - (m * 60000);
        }
		u8Temp = vI_HeaderBufferLen-vl_len;
        vl_len += snprintf((pO_Header+vl_len), (size_t)(u8Temp), "%02dm,%02d.%03ds:", m, s, ms);
 
    }
    /* print level */
    switch (u32Level)
    {
    case TR_LEVEL_FATAL:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"FATAL");
        break;
    case TR_LEVEL_ERRORS:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"ERROR");
        break;
    case TR_LEVEL_SYSTEM_MIN:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"SYSMIN");
        break;
    case TR_LEVEL_SYSTEM:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"SYS");
        break;
    case TR_LEVEL_COMPONENT:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"COMP");
        break;
    case TR_LEVEL_USER_1:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"USER1");
        break;
    case TR_LEVEL_USER_2:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"USER2");
        break;
    case TR_LEVEL_USER_3:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"USER3");
        break;
    case TR_LEVEL_USER_4:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"USER4");
        break;
    default:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"%08x", u32Level);
    }
    vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),":");
    /* print class */
    switch (u32Class)
    {
#ifdef CONFIG_TRACE_CLASS_OSALCORE
    case TR_CLASS_OSALCORE:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"OSALCORE");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_IPFORWARD
    case TR_CLASS_APP_IPFORWARD:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"IPF");
        break;
#endif
#ifdef CONFIG_ENABLE_CLASS_APP_DABMW
    case TR_CLASS_APP_DABMW:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"DABMW");
        break;
#endif
#ifdef CONFIG_ENABLE_CLASS_APP_DABMW_SF
    case TR_CLASS_APP_DABMW_SF:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"SERVFOLL");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_ETAL
	case TR_CLASS_APP_ETAL:
		vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"ETAL");
		break;
	case TR_CLASS_APP_ETAL_COMM:
		vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"ETALCOMM");
		break;
	case TR_CLASS_APP_ETAL_CMD:
		vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"ETALCMD");
		break;
    case TR_CLASS_APP_ETAL_API:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"ETALAPI");
        break;
    case TR_CLASS_APP_ETAL_TEST:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"ETALTEST");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_CMOST
	case TR_CLASS_CMOST:
		vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"CMOST");
		break;
#endif
#ifdef CONFIG_TRACE_CLASS_HDRADIO
	case TR_CLASS_HDRADIO:
		vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"HDRADIO");
		break;
#endif
#ifdef CONFIG_TRACE_CLASS_BOOT
	case TR_CLASS_BOOT:
		vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"BOOT");
		break;
#endif
#ifdef CONFIG_TRACE_CLASS_BSP
	case TR_CLASS_BSP:
		vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"BSP");
		break;
#endif
#ifdef CONFIG_TRACE_CLASS_STECI
	case TR_CLASS_STECI:
		vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"STECI");
		break;
#endif
#ifdef CONFIG_TRACE_CLASS_EXTERNAL
	case TR_CLASS_EXTERNAL:
		vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"EXTERNAL");
		break;
#endif
    default:
        vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),"%08x", u32Class);
    }
    vl_len += snprintf((pO_Header+vl_len), (vI_HeaderBufferLen-vl_len),": ");


#ifdef CONFIG_TRACE_ASYNC
	printf("%s", pO_Header);
#endif 

exit:
	return(vl_len);
}
#endif
/**
 * @brief     OSALUTIL_s32TraceWrite
 *
 * @details   Writes a buffer to the trace device
 *
 * @param     io descriptor of io device (I)
 * @param     u32Level trace or debug level (I)
 * @param     u32Class trace or debug class (I)
 * @param     pcos8Buffer pointer to char buffer (I)
 * @param     u32Length size of buffer (I)
 *
 * @return
 *        - Number of written bytes
 *        - OSAL_ERROR otherwise
 *
 */
tS32
OSALUTIL_s32TraceWrite(OSAL_tIODescriptor io, tU32 u32Level, tU32 u32Class,
    tPCS8 pcos8Buffer, tU32 u32Length)
{
  (void) io; /* make Lint happy */

  if (osutil_trace_output_driver != OSAL_NULL)
    {
      if (OSALUTIL_s32TraceCheckFilter(u32Level, u32Class) == TRUE)
        {
          osutil_trace_output_driver(u32Level, u32Class,
              OSUTIL_TRACE_MSG_TYPE_BINARY, u32Length, (tU8*) pcos8Buffer);
        }
    }
  return 0;
}

/**
 * @brief     OSALUTIL_s32TracePrintf
 *
 * @details   Writes a string into the trace device
 *
 * @param     fd file descriptor (I)
 * @param     u32Level trace or debug level (I)
 * @param     u32Class trace or debug class (I)
 * @param     coszFormat format string (I)
 * @param     ... arguments
 *
 * @return
 *        - Number of written bytes
 *        - OSAL_ERROR if max buffer size is reached
 *
 */
tS32 OSALUTIL_s32TracePrintf(OSAL_tIODescriptor fd, tU32 u32Level,
        tU32 u32Class, tCString coszFormat, ...)	/*lint !e960 - MISRA 16.1 - function has variable number of arguments */
{

		tS32 ret = OSAL_OK;
    tChar szBuffer[OSAL_C_U32_TRACE_MAX_MESSAGESIZE];
    tChar* pszBuffer = &szBuffer[0];
    tS32 saved_errno = errno;
#ifndef CONFIG_TRACE_ASYNC
//	static tChar headerBuffer[headerBufferLen];
#endif

    (void) fd; /* make Lint happy */
    /* OSAL Linux does not properly implement per-thread error, rather relies on glib's errno */
    /* which is also per-thread, but any call to library function will modify it */
    /* Save a local copy so we don't overwrite it */

	memset(szBuffer, 0, sizeof(szBuffer));

    if((OSALUTIL_PrintfEnabled==FALSE) && (osutil_trace_output_driver==OSAL_NULL))
    {
        /* when no output is required, then exit */
        ret = OSAL_OK;
        goto exit;
    }
    if( OSALUTIL_s32TraceCheckFilter(u32Level,u32Class) == TRUE )
    {
        if (ostracesem_sem == (OSAL_tSemHandle)NULL)
        {
            (void)OSAL_s32SemaphoreCreate("tracesem", &ostracesem_sem, 1);
	}
        if (ostracesem_sem != (OSAL_tSemHandle)NULL)
        {
            (void)OSAL_s32SemaphoreWait(ostracesem_sem, OSAL_C_TIMEOUT_FOREVER);
        {
            tS32 s32Size;
            va_list argList;


            /* DeactivateLintMessage_ID0033 */ 
            /*lint -save -e530 - MISRA 9.1 - intialization done inside the function*/
            va_start(argList, coszFormat); //lint !e718 !e746 - MISRA 8.1 - __va_start part of posix includes
            /*lint -restore */            
            
            s32Size = print(&pszBuffer, coszFormat, argList);
	/* va_end call inside print function */

            if (s32Size < 0)
            {
                if (ostracesem_sem != (OSAL_tSemHandle)NULL)
                {
                    (void)OSAL_s32SemaphorePost(ostracesem_sem);
                }
                errno = saved_errno;
                ret = OSAL_ERROR;
                goto exit;
            }
            else
            {
                int n;
                n = OSAL_u32StringLength(szBuffer);
                if(OSALUTIL_PrintfEnabled == TRUE)
                {
                    /*
                     * cut the tailing non printable characters from the message
                     */
                    if( n > 0 )
                    {
                        char* s = &szBuffer[n-1];
                        while(((tU32)(*s) < (tU32)(' '))&(s>szBuffer))	//lint !e946 - MISRA 17.2 MISRA 17.3 - Code Readability
                        {
                            *s = '\0';
                            s--;
                        }
                    }

#if defined (CONFIG_TRACE_ASYNC)
                    extern tVoid ETAL_tracePrint(tU32 u32Level, tU32 u32Class, tCString coszFormat);

                    ETAL_tracePrint(u32Level, u32Class, szBuffer);
#else

		    /*
                     * output the header on the "printf"-terminal
                     */
			OS_PRINTF("****** %s \n", szBuffer);
#endif // CONFIG_TRACE_ASYNC
                }

           }
        }
        if (ostracesem_sem != (OSAL_tSemHandle)NULL)
        {
        (void)OSAL_s32SemaphorePost(ostracesem_sem);
    	}
    }
    }
    errno = saved_errno;
    ret = 0;

exit:
		return ret;
}
#endif // CONFIG_TRACE_ENABLE

#ifdef __cplusplus
}
#endif

#endif  /* OSUTILIO_C */

/** @} */

/* End of File */

