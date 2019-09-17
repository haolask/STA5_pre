//!
//!  \file      osutilio.c
//!  \brief     <i><b>OSAL IO utilities Functions</b></i>
//!  \details   This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) IO utilities Functions.
//!

/* DeactivateLintMessage_ID0026 */
/*lint -esym(750, OS*_C) */
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

#if defined (CONFIG_TRACE_ENABLE)
#include "osal.h"

// Local to manage the syslog logging

#ifdef CONFIG_TRACE_ETAL_SYSLOG 
#include "syslog.h"

// add syslog try
#define OS_PRINTF(level, ...) \
    {\
    syslog(((level==TR_LEVEL_FATAL)?LOG_ERR:((level==TR_LEVEL_ERRORS)?LOG_WARNING:((level<TR_LEVEL_USER_1)?LOG_INFO:LOG_DEBUG))),__VA_ARGS__);\
    }
#else
#define OS_PRINTF(level, ...) printf(__VA_ARGS__)
#endif 

#if defined (CONFIG_TRACE_ASYNC)
extern tVoid ETAL_tracePrint(tU32 u32Level, tU32 u32Class, tCString coszFormat);
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
static OSALUTIL_vOutputDriverCallbackType osutil_trace_output_driver = NULL;

tVoid OSALUTIL_vTraceSetOutputDriverCallback( OSALUTIL_vOutputDriverCallbackType cb )
{
    if( osutil_trace_output_driver == NULL )
    {
        osutil_trace_output_driver = cb;
    }
}

static tBool OSALUTIL_PrintfEnabled = TRUE;
#if 0
/**
 * @brief     OSALUTIL_s32TraceDisablePrintfOutput
 *
 * @details   disable the "printf" output
 *
 */
tVoid OSALUTIL_s32TraceDisablePrintfOutput( void )
{
    OSALUTIL_PrintfEnabled = FALSE;
}
#endif

static OSAL_tSemHandle ostracesem_sem = 0;

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
  if (u32Level <= tOSTraceConfig.u32DefaultLevel)
    {
      return TRUE;
    }
#ifdef CONFIG_OSUTIL_TRACE_NUM_FILTERS
#if (CONFIG_OSUTIL_TRACE_NUM_FILTERS != 0)
  else
    {
      int i;

      if(tOSTraceConfig.t32NumUsedFilters == 0)
        {
          return FALSE;
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
                      return TRUE;
                    }
                }
            }
        }
    }
#endif

#else
  (void) u32Class; /* make Lint happy */
#endif
  return FALSE;
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
    (LINT_IGNORE_RET)OSAL_s32SemaphoreCreate("tracesem", &ostracesem_sem, 1);
    OSAL_pvMemorySet((tVoid *)&tOSTraceConfig, 0x00, sizeof(tOSTraceConfig));
#ifdef CONFIG_TRACE_DEFAULT_LEVEL
    tOSTraceConfig.u32DefaultLevel = CONFIG_TRACE_DEFAULT_LEVEL;
#else
    tOSTraceConfig.u32DefaultLevel = TR_LEVEL_SYSTEM_MIN;
#endif
}

tVoid
OSALUTIL_vTraceDeInit(tVoid)
{
    OSAL_pvMemorySet((tVoid *)&tOSTraceConfig, 0x00, sizeof(tOSTraceConfig));
    if (OSAL_s32SemaphoreClose(ostracesem_sem) == OSAL_OK)
    {
        (LINT_IGNORE_RET)OSAL_s32SemaphoreDelete("tracesem");
    }
}


tVoid OSALUTIL_s32TraceSetFilterWithMask( tU32 u32Class, tU32 u32Mask, tU32 u32Level )
{
#ifdef CONFIG_OSUTIL_TRACE_NUM_FILTERS
  int i;
  if(u32Class == 0)
    {
      return;
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
              return;
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
          return;
        }
    }
  /* no space left in table */
#else
  (void) u32Class; /* make Lint happy */
  (void) u32Mask;  /* make Lint happy */
  (void) u32Level; /* make Lint happy */
#endif
}

#if 0
tU32 OSALUTIL_s32GetFilterLevel(tU32 u32Class)
{
    int i;
    for (i = 0; i < CONFIG_OSUTIL_TRACE_NUM_FILTERS; i++)
    {
        if (tOSTraceConfig.tTraceFilter[i].u32Class == u32Class)
        {
            return tOSTraceConfig.tTraceFilter[i].u32Level;
        }
    }
    return TR_LEVEL_FATAL;
}
#endif

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

#if 0
tVoid
OSALUTIL_s32TraceSetFilter(tU32 u32Class, tU32 u32Level)
{
    OSALUTIL_s32TraceSetFilterWithMask(u32Class,0xFFFFFFFF,u32Level);
}
#endif // #if 0

#ifdef CONFIG_TRACE_ASYNC
tU8 OSALUTIL_s32TracePrintHeader(OSAL_tMSecond t, tU32 u32Level, tU32 u32Class)
#else
static tU8 OSALUTIL_s32TracePrintHeader(tU32 u32Level, tU32 u32Class, tPChar pO_Header, tU32 vI_HeaderBufferLen)
#endif
{
    int m, s, ms;
#ifdef CONFIG_TRACE_ASYNC
    tChar header[100];
    tPChar pO_Header = header;
    tU32 vI_HeaderBufferLen = 100;
#else
    OSAL_tMSecond t;
#endif 

    tU32 vl_len = 0;

    if (tOSTraceConfig.boolOmitHeader || (pO_Header == OSAL_NULL))
    {
        return (tU8)0;
    }

    /* print time */
    {
#ifndef CONFIG_TRACE_ASYNC
        t = OSAL_ClockGetElapsedTime();
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
            s  = (int)(t - (m * 60000)) / 1000;
            ms = (int)t - (s * 1000) - (m * 60000);
        }
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len), "%02dm,%02d.%03ds:", m, s, ms);
    }
    /* print level */
    switch (u32Level)
    {
    case TR_LEVEL_FATAL:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"FATAL");
        break;
    case TR_LEVEL_ERRORS:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"ERROR");
        break;
    case TR_LEVEL_SYSTEM_MIN:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"SYSMIN");
        break;
    case TR_LEVEL_SYSTEM:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"SYS");
        break;
    case TR_LEVEL_COMPONENT:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"COMP");
        break;
    case TR_LEVEL_USER_1:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"USER1");
        break;
    case TR_LEVEL_USER_2:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"USER2");
        break;
    case TR_LEVEL_USER_3:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"USER3");
        break;
    case TR_LEVEL_USER_4:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"USER4");
        break;
    default:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"%08x", u32Level);
    }
    vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),":");
    /* print class */
    switch (u32Class)
    {
#ifdef CONFIG_TRACE_CLASS_OSALCORE
    case TR_CLASS_OSALCORE:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"OSALCORE");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_IPFORWARD
    case TR_CLASS_APP_IPFORWARD:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"IPF");
        break;
#endif
#ifdef CONFIG_ENABLE_CLASS_APP_DABMW
    case TR_CLASS_APP_DABMW:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"DABMW");
        break;
#endif
#ifdef CONFIG_ENABLE_CLASS_APP_DABMW_SF
    case TR_CLASS_APP_DABMW_SF:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"SERVFOLL");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_ETAL
    case TR_CLASS_APP_ETAL:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"ETAL");
        break;
    case TR_CLASS_APP_ETAL_COMM:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"ETALCOMM");
        break;
    case TR_CLASS_APP_ETAL_CMD:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"ETALCMD");
        break;
    case TR_CLASS_APP_ETAL_API:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"ETALAPI");
        break;
    case TR_CLASS_APP_ETAL_TEST:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"ETALTEST");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_TUNERDRIVER
    case TR_CLASS_TUNERDRIVER:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"TUNERDRV");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_CMOST
    case TR_CLASS_CMOST:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"CMOST");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_HDRADIO
    case TR_CLASS_HDRADIO:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"HDRADIO");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_BOOT
    case TR_CLASS_BOOT:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"BOOT");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_BSP
    case TR_CLASS_BSP:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"BSP");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_STECI
    case TR_CLASS_STECI:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"STECI");
        break;
#endif
#ifdef CONFIG_TRACE_CLASS_EXTERNAL
    case TR_CLASS_EXTERNAL:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"EXTERNAL");
        break;
#endif
    default:
        vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),"%08x", u32Class);
    }
    vl_len += snprintf((pO_Header+vl_len), (size_t)(vI_HeaderBufferLen-vl_len),": ");


#ifdef CONFIG_TRACE_ASYNC
    printf("%s", pO_Header);
#endif 

    if (vl_len > 0xFF)
    {
        return (tU8)0xFF;
    }
    return (tU8)(vl_len);
}

#if 0
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

  if (osutil_trace_output_driver != NULL)
    {
      if (OSALUTIL_s32TraceCheckFilter(u32Level, u32Class) == TRUE)
        {
          osutil_trace_output_driver(u32Level, u32Class,
              OSUTIL_TRACE_MSG_TYPE_BINARY, u32Length, (tU8*) pcos8Buffer);
        }
    }
  return 0;
}
#endif

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
        tU32 u32Class, tCString coszFormat, ...)
{

    static tChar szBuffer[OSAL_C_U32_TRACE_MAX_MESSAGESIZE];

#if !defined (CONFIG_TRACE_ASYNC) || (!defined (CONFIG_APP_ETAL_LIBRARY) && !defined (CONFIG_APP_ETAL_TEST))
    #define headerBufferLen 100

    static tChar headerBuffer[headerBufferLen];
#endif

    /* OSAL Linux does not properly implement per-thread error, rather relies on glib's errno */
    /* which is also per-thread, but any call to library function will modify it */
    /* Save a local copy so we don't overwrite it */
    tS32 saved_errno = errno;

    (void) fd; /* make Lint happy */

    if((OSALUTIL_PrintfEnabled==FALSE) && (osutil_trace_output_driver==NULL))
    {
        /* when no output is required, then exit */
        return OSAL_OK;
    }

    if( OSALUTIL_s32TraceCheckFilter(u32Level,u32Class) == TRUE )
    {
        if (ostracesem_sem != 0)
        {
            OSAL_s32SemaphoreWait(ostracesem_sem, OSAL_C_TIMEOUT_FOREVER);
        {
            tS32 s32Size;
            va_list argList;

            /* DeactivateLintMessage_ID0033 */ 
            /*lint -save -e530 */
            OSAL_VarArgStart(argList, coszFormat);
            /*lint -restore */            
            
            s32Size = OSAL_s32VarNPrintFormat(szBuffer, (size_t)sizeof(szBuffer), coszFormat, argList);
            OSAL_VarArgEnd(argList);
            if (s32Size < 0)
            {
                if (ostracesem_sem != 0)
                {
                    OSAL_s32SemaphorePost(ostracesem_sem);
                }
                errno = saved_errno;
                return OSAL_ERROR;
            }
            else
            {
                int n;
                n = (int)OSAL_u32StringLength(szBuffer);

                if(OSALUTIL_PrintfEnabled == TRUE)
                {
                    /*
                     * cut the tailing non printable characters from the message
                     */
                    if( n > 0 )
                    {
                        char* s = &szBuffer[n-1];
                        while((*s < ' ')&(s>szBuffer))
                        {
                            *s = '\0';
                            s--;
                        }
                    }

#if defined (CONFIG_TRACE_ASYNC)
    #if defined (CONFIG_APP_ETAL_LIBRARY) || defined (CONFIG_APP_ETAL_TEST)
                    ETAL_tracePrint(u32Level, u32Class, szBuffer);
    #else
                    OSALUTIL_s32TracePrintHeader(OSAL_ClockGetElapsedTime(),u32Level, u32Class);
                    OS_PRINTF(u32Level,"%s %s\n", headerBuffer, szBuffer);
    #endif
#else
                    /*
                     * output the header on the "printf"-terminal
                     */
                    OSALUTIL_s32TracePrintHeader(u32Level, u32Class, headerBuffer, headerBufferLen);

                    OS_PRINTF(u32Level,"%s %s\n", headerBuffer, szBuffer);
#endif // CONFIG_TRACE_ASYNC
                }

                /*
                 * output the message on the custom callback terminal
                 */
                if((osutil_trace_output_driver != NULL) && (n > 0))
                {
                    osutil_trace_output_driver(u32Level, u32Class, OSUTIL_TRACE_MSG_TYPE_ASCII, (tU32)n, (tU8*)szBuffer);
                }
            }
        }
        if (ostracesem_sem != 0)
        {
        OSAL_s32SemaphorePost(ostracesem_sem);
        }
    }
    }
    errno = saved_errno;
    return 0;
}
#endif // CONFIG_TRACE_ENABLE

#ifdef __cplusplus
}
#endif

#endif  /* OSUTILIO_C */

/** @} */

/* End of File */

