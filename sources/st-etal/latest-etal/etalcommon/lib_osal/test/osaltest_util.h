/************************************************************************
* PROJECT : OSAL CORE on OS21
* FILE : osaltest.h
*
* DESCRIPTION : This is the headerfile for the OSAL test suite  Component.
*
* COPYRIGHT :(c) 2008, STMicroelectronics
*
* VERSION : 0.1
* DATE (mm.dd.yyyy) : 11.06.2008
* AUTHOR : L. Pesenti - G. Di Martino - K. Singhi
* HISTORY :
* Revision 1.4  2010/02/01 09:00:52  shz2hi
* VENDOR_STM_2010_01_29_01 integrated
*
* Revision 1.1.1.1  2009/12/09 16:13:40  gol2hi
* Import from STM
*
* Revision 1.1  2009/09/11 09:51:32  shz2hi
* new OS21 version with ADR3 support
*
* Revision 1.5  2009/05/25 08:20:11  shz2hi
* new ST delivery OSAL_22_5_2009
*
************************************************************************/


#ifndef OSALTEST_UTIL_H
#define OSALTEST_UTIL_H

#ifdef CONFIG_TARGET_TEST_ENABLE_ST_OSALCORE_TEST
	#include "osaltest.h"
#endif

#ifdef CONFIG_TARGET_TEST_ENABLE_ST_OSALIO_TEST
	#include "osalIOtest.h"
#endif

/*****************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|----------------------------------------------------------------------------*/

/*****************************************************************************
| defines and macros (scope: global)
|----------------------------------------------------------------------------*/

#define OSAL_C_TRACELEVEL1
#define OSAL_C_TRACELEVEL2
#define OSAL_C_TRACELEVEL3

#define OSAL_C_MAXTESTS 1

#if 1
#define PRINTF(args...)  (OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, args))
#else
#define PRINTF printf
#endif

/* Definitions of different trace message levels */
#ifdef OSAL_C_TRACELEVEL1
   #define TRACE1_MSG0(arg)              PRINTF(arg)
   #define TRACE1_MSG1(arg1,arg2)        PRINTF((arg1),(arg2))
   #define TRACE1_MSG2(arg1,arg2,arg3)   PRINTF((arg1),(arg2),(arg3))
   #define TRACE1_MSG3(arg1,arg2,arg3,arg4)  PRINTF((arg1),(arg2),(arg3),(arg4))
#else
   #define TRACE1_MSG0(arg)
   #define TRACE1_MSG1(arg1,arg2)
   #define TRACE1_MSG2(arg1,arg2, arg3)
   #define TRACE1_MSG3(arg1,arg2,arg3,arg4)
#endif

#ifdef OSAL_C_TRACELEVEL2
   #define TRACE2_MSG0(arg)              PRINTF(arg)
   #define TRACE2_MSG1(arg1,arg2)        PRINTF((arg1),(arg2))
   #define TRACE2_MSG2(arg1,arg2,arg3)   PRINTF((arg1),(arg2),(arg3))
   #define TRACE2_MSG3(arg1,arg2,arg3,arg4)  PRINTF((arg1),(arg2),(arg3),(arg4))
#else
   #define TRACE2_MSG0(arg)
   #define TRACE2_MSG1(arg1,arg2)
   #define TRACE2_MSG2(arg1,arg2, arg3)
   #define TRACE2_MSG3(arg1,arg2,arg3,arg4)
#endif

#ifdef OSAL_C_TRACELEVEL3
   #define TRACE3_MSG0(arg)              PRINTF(arg)
   #define TRACE3_MSG1(arg1,arg2)        PRINTF((arg1),(arg2))
   #define TRACE3_MSG2(arg1,arg2,arg3)   PRINTF((arg1),(arg2),(arg3))
   #define TRACE3_MSG3(arg1,arg2,arg3,arg4)  PRINTF((arg1),(arg2),(arg3),(arg4))
#else
   #define TRACE3_MSG0(arg)
   #define TRACE3_MSG1(arg1,arg2)
   #define TRACE3_MSG2(arg1,arg2,arg3)
   #define TRACE3_MSG3(arg1,arg2,arg3,arg4)
#endif

/*****************************************************************************
|typedefs and struct defs (scope: global)
|----------------------------------------------------------------------------*/

/*****************************************************************************
| variable declaration (scope: global)
|----------------------------------------------------------------------------*/

/*****************************************************************************
|function prototypes (scope: global)
|----------------------------------------------------------------------------*/

extern tVoid CheckErr( tS32 RetValue, tU32 u32ExpectedError );
extern tVoid CountUnmatched( tU32 u32ExpectedError );
extern tVoid OSAL_vTestErrorHook(tU32 u32ErrorCode);
extern tVoid OSAL_vTestSetErrorHook(const OSAL_tpfErrorHook pfErrorHook);



#else
#error osaltest_util.h included several times
#endif
