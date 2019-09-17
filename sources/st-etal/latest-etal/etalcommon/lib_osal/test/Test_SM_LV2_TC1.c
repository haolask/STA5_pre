/*****************************************************************************
| FILE:
| PROJECT:      ELeNa
| SW-COMPONENT: OSAL
|-----------------------------------------------------------------------------
| DESCRIPTION:  This is the test semaphore level 1 for OSAL
|               (Operating System Abstraction Layer) test suite.
|               Two thread shared a semaphore, vThread0 use the semaphore
|               for 5 ms, then second open it and wait the avaliability.
|               Finally after the close function the semaphore is destroyed
|-----------------------------------------------------------------------------
| COPYRIGHT:    (c) 2001 Tecne s.r.l., Cagliari (ITALY)
| HISTORY:
| Date      | Modification               | Author
| --.--.--  | Initial revision           | -------, -----
| --.--.--  | ----------------           | -------, -----
|
|*****************************************************************************/
#include <stdio.h>
#include "osal.h"
#include "osaltest.h"
#include "osaltest_util.h"

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
#define SEM_1     0x00000003
/*****************************************************************
| variable defintion (scope: modul-local)
|----------------------------------------------------------------*/
extern OSAL_tpfErrorHook pfUserErrorHook;
extern tU32 u32GlobalError;
extern tU32 u32ThreadCount;

#ifdef OSAL_SEMAPHORE_SKIP_NAMES
static OSAL_tSemHandle semphr_0;
#endif
static OSAL_tSemHandle semphr_1,semphr_2;
/*****************************************************************
| function prototype (scope: module-local)
|----------------------------------------------------------------*/

tVoid vTest_SM_LV2_TC1(void);
tVoid vThread_1(tPVoid);
tVoid vThread_2(tPVoid);

/*****************************************************************
| function implementation (scope: module-local)
|----------------------------------------------------------------*/

/*****************************************************************
| function implementation (scope: global)
|----------------------------------------------------------------*/


tVoid vTest_SM_LV2_TC1(void)
{
   OSAL_tThreadID Thread1ID, Thread2ID;
   OSAL_tpfErrorHook* oldHook=OSAL_NULL;
   OSAL_trThreadAttribute  attr1,attr2;
   tS32 RetValue;

#if 0
   OSAL_vErrorHook(OSAL_vTestErrorHook,oldHook);
#endif



   TRACE1_MSG0("\n\rTest_SM_LV2_TC1 starting ");

	RetValue = OSAL_s32SemaphoreCreate("SM1", &semphr_1,0);
	CheckErr (RetValue, OSAL_E_NOERROR );
	TRACE3_MSG0("  T1 Create SM0\n\r");
	RetValue = OSAL_s32SemaphoreCreate("SM2", &semphr_2,0);
	CheckErr (RetValue, OSAL_E_NOERROR );

#ifdef OSAL_SEMAPHORE_SKIP_NAMES
	TRACE3_MSG0("  T1 Create SM0\n\r");
	RetValue = OSAL_s32SemaphoreCreate("SM0", &semphr_0,0);
	CheckErr (RetValue, OSAL_E_NOERROR );
	TRACE3_MSG0("  T1 Create SM0\n\r");
#endif
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_SM_LV1_TC01 - Access to a SM by two different threads\n\r");
   /*********************************************************************************/

   TRACE2_MSG0("  T0 Spawn T1\n\r");
   attr1.szName = (char *)"Thread_1";
   attr1.u32Priority = OSAL_C_U32_THREAD_PRIORITY_HIGHEST;//0;
   attr1.s32StackSize = CONST_TARGET_OSALTEST_TASKSTACKSIZE_INIT;
   attr1.pfEntry = vThread_1;
   attr1.pvArg = OSAL_NULL;
   Thread1ID = OSAL_ThreadSpawn(&attr1);
   CheckErr (Thread1ID, OSAL_E_NOERROR );
   if (Thread1ID != 0 )
      u32ThreadCount++;
   else
	  return;

   OSAL_s32ThreadWait(100);

   TRACE2_MSG0("  T0 Spawn T2\n\r");
   attr2.szName = (char *)"Thread_2";
   attr2.u32Priority = OSAL_C_U32_THREAD_PRIORITY_HIGHEST;//0;
   attr2.s32StackSize = CONST_TARGET_OSALTEST_TASKSTACKSIZE_INIT;
   attr2.pfEntry = vThread_2;
   attr2.pvArg = OSAL_NULL;
   Thread2ID = OSAL_ThreadSpawn(&attr2);
   CheckErr (Thread2ID, OSAL_E_NOERROR );
   if (Thread2ID != 0 )
      u32ThreadCount++;
   else
	  return;

	TRACE3_MSG0("  T1 Wait SM1\n\r");
	RetValue =OSAL_s32SemaphoreWait(semphr_1,OSAL_C_TIMEOUT_FOREVER);
	CheckErr (RetValue, OSAL_E_NOERROR );

	TRACE3_MSG0("  T1 Wait SM2\n\r");
	RetValue =OSAL_s32SemaphoreWait(semphr_2,OSAL_C_TIMEOUT_FOREVER);
	CheckErr (RetValue, OSAL_E_NOERROR );

#ifdef OSAL_SEMAPHORE_SKIP_NAMES

	TRACE3_MSG0("  T1 Free SM2\n\r");
	RetValue = OSAL_s32SemaphoreFree(semphr_1);
	CheckErr (RetValue, OSAL_E_NOERROR );

	TRACE3_MSG0("  T1 Free SM1\n\r");
	RetValue = OSAL_s32SemaphoreFree(semphr_2);
	CheckErr (RetValue, OSAL_E_NOERROR );

	TRACE3_MSG0("  T1 Free SM0\n\r");
	RetValue = OSAL_s32SemaphoreFree(semphr_0);
	CheckErr (RetValue, OSAL_E_NOERROR );

#else

	TRACE3_MSG0("  T1 Close SM2\n\r");
	RetValue = OSAL_s32SemaphoreClose(semphr_1);
	CheckErr (RetValue, OSAL_E_NOERROR );

	TRACE3_MSG0("  T1 Close SM1\n\r");
	RetValue = OSAL_s32SemaphoreClose(semphr_2);
	CheckErr (RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T1 Delete SM1\n\r");
   RetValue = OSAL_s32SemaphoreDelete("SM1");
   CheckErr (RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T1 Delete SM2\n\r");
   RetValue = OSAL_s32SemaphoreDelete("SM2");
   CheckErr (RetValue, OSAL_E_NOERROR );
#endif

}




 

tVoid vThread_1(tPVoid pvArg)
{
   tS32 RetValue;
#ifndef OSAL_SEMAPHORE_SKIP_NAMES
   OSAL_tSemHandle semphr_0;
#endif
   OSAL_tpfErrorHook* oldHook=OSAL_NULL;
#if 0
   OSAL_vErrorHook(OSAL_vTestErrorHook,oldHook);
#endif

#ifndef OSAL_SEMAPHORE_SKIP_NAMES
   TRACE3_MSG0("  T1 Create SM0\n\r");
   RetValue = OSAL_s32SemaphoreCreate("SM0", &semphr_0,1);
   CheckErr (RetValue, OSAL_E_NOERROR );
#endif
   TRACE3_MSG0("  T1 Wait SM0 with blocking TIMEOUT\n\r");
   RetValue = OSAL_s32SemaphoreWait(semphr_0,OSAL_C_TIMEOUT_FOREVER);
   CheckErr (RetValue, OSAL_E_NOERROR );

#ifndef OSAL_SEMAPHORE_SKIP_NAMES
   TRACE3_MSG0("  T1 Close SM0\n\r");
   RetValue = OSAL_s32SemaphoreClose(semphr_0);
   CheckErr (RetValue, OSAL_E_NOERROR );
#endif

   TRACE3_MSG0("  T1 Wait for 150 ticks\n\r");
   RetValue = OSAL_s32ThreadWait(150);
   CheckErr (RetValue, OSAL_E_NOERROR );

#ifndef OSAL_SEMAPHORE_SKIP_NAMES
   TRACE3_MSG0("  T1 Delete SM0\n\r");
   RetValue = OSAL_s32SemaphoreDelete("SM0");
   CheckErr (RetValue, OSAL_E_NOERROR );
#endif
   u32ThreadCount--;

   TRACE3_MSG0("  T1 Exit\n\r ");

   if(u32ThreadCount == 0)
   {
      if ( u32GlobalError== 0 )
	  {
         TRACE1_MSG0("Test_SM_LV2_TC1 PASSED---\n\n\n\r" );
	  }
      else
	  {
         TRACE1_MSG0("Test_SM_LV2_TC1 FAILED---\n\n\n\r" );
         TRACE2_MSG1("u32GlobalError= %d\n\n\n\r", u32GlobalError );
	  }
   }

   OSAL_s32SemaphorePost(semphr_1);

   OSAL_vThreadExit();
}


tVoid vThread_2(tPVoid pvArg)
{
   tS32 RetValue;
#ifndef OSAL_SEMAPHORE_SKIP_NAMES
   OSAL_tSemHandle semphr_0;
#endif
   OSAL_tpfErrorHook* oldHook=OSAL_NULL;
#if 0
   OSAL_vErrorHook(OSAL_vTestErrorHook,oldHook);
#endif
#ifndef OSAL_SEMAPHORE_SKIP_NAMES
   TRACE3_MSG0("  T2 Open SM0\n\r");
   RetValue = OSAL_s32SemaphoreOpen("SM0", &semphr_0);
   CheckErr (RetValue, OSAL_E_NOERROR );
#endif

   TRACE3_MSG0("  T2 Post SM0\n\r");
   RetValue = OSAL_s32SemaphorePost(semphr_0);
   CheckErr (RetValue, OSAL_E_NOERROR );

#ifndef OSAL_SEMAPHORE_SKIP_NAMES
   TRACE3_MSG0("  T2 Close SM0\n\r");
   RetValue = OSAL_s32SemaphoreClose(semphr_0);
   CheckErr (RetValue, OSAL_E_NOERROR );
#endif
   u32ThreadCount--;

   TRACE3_MSG0("  T2 Exit\n\r ");

   if(u32ThreadCount == 0)
   {
      if ( u32GlobalError== 0 )
	  {
         TRACE1_MSG0("Test_SM_LV2_TC1 PASSED---\n\n\n\r" );
	  }
      else
	  {
         TRACE1_MSG0("Test_SM_LV2_TC1 FAILED---\n\n\n\r" );
         TRACE2_MSG1("u32GlobalError= %d\n\n\n\r", u32GlobalError );
	  }
   }

   OSAL_s32SemaphorePost(semphr_2);

   OSAL_vThreadExit();
}
