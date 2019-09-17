//!
//!  \file 		osutilio.h
//!  \brief 	<i><b>OSAL IO utilities Header File</b></i>
//!  \details	This is the HeaderFile for the OSAL
//!             (Operating System Abstraction Layer) IO utilities Functions.
//!
#ifndef OSUTILIO_H
#define OSUTILIO_H

#ifdef __cplusplus
extern"C"
{
#endif

//-----------------------------------------------------------------------
//defines and macros (scope: global)
//-----------------------------------------------------------------------
#define OSAL_TRACE_PRINTF(level, module, ...) \
	do { OSALUTIL_s32TracePrintf (0, (level), (module), ## __VA_ARGS__); } while (0)

// define a min and max macro, this can help 
#define OSAL_MIN_FUNCTION(a,b)	((a<b)?a:b)
#define OSAL_MAX_FUNCTION(a,b)	((a>b)?a:b)

//-----------------------------------------------------------------------
//typedefs and struct defs (scope: global)
//-----------------------------------------------------------------------

extern tS32  OSALUTIL_s32TraceWrite(OSAL_tIODescriptor io, tU32 u32Level, tU32 u32Class, tPCS8 pcos8Buffer, tU32 u32Length);
extern tS32 /*@alt void@*/OSALUTIL_s32TracePrintf(OSAL_tIODescriptor fd, tU32 u32Level, tU32 u32Class, tCString coszFormat, ...);	//lint !e960 - MISRA 16.1 - Function with variable argument.

extern tBool OSALUTIL_s32TraceCheckFilter(tU32 u32Level, tU32 u32Class);
extern tVoid OSALUTIL_vTraceSetDefaultLevel( tU32 u32Level );
extern tVoid OSALUTIL_s32TraceSetFilter( tU32 u32Class, tU32 u32Level );
extern tVoid OSALUTIL_vTraceDisableHeader( tVoid );
extern tVoid OSALUTIL_vTraceInit(tVoid);
extern tVoid OSALUTIL_vTraceDeInit(tVoid);

#ifdef CONFIG_TRACE_ASYNC
extern tU8 OSALUTIL_s32TracePrintHeader(OSAL_tMSecond t, tU32 u32Level, tU32 u32Class);
#endif

#define OSALUTIL_PROVIDE_s32SetFilterWithMask
extern tVoid OSALUTIL_s32TraceSetFilterWithMask( tU32 u32Class, tU32 u32Mask, tU32 u32Level );

#undef OSALUTIL_PROVIDE_s32GetFilterLevel
extern tU32 OSALUTIL_s32GetFilterLevel( tU32 u32Class );

#define OSALUTIL_PROVIDE_s32TraceClearFilter
tVoid OSALUTIL_s32TraceClearFilter( void );

#undef OSALUTIL_PROVIDE_s32TraceDisablePrintfOutput
extern tVoid OSALUTIL_s32TraceDisablePrintfOutput( void );

#define OSALUTIL_PROVIDE_s32TraceSetOutputDriverCallback
#define OSUTIL_TRACE_MSG_TYPE_BINARY 1
#define OSUTIL_TRACE_MSG_TYPE_ASCII  2
typedef void (*OSALUTIL_vOutputDriverCallbackType) ( tU32 u32Level, tU32 u32Class, tU32 u16TraceMsgType, tU32 u32TraceMsgLength, tU8* u32TraceMsg);
extern tVoid OSALUTIL_vTraceSetOutputDriverCallback( OSALUTIL_vOutputDriverCallbackType cb );

#ifdef __cplusplus
}
#endif

#else
#error osutilio.h included several times
#endif
