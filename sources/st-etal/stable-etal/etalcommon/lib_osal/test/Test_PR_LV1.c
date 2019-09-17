#include <stdio.h>
#include <stdlib.h>
#include "osal.h"
#include "osaltest.h"
#include "osaltest_util.h"


/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
#define TEST_TRACE
#undef TEST_TCB
#undef TEST_ACTIVATE
#undef TEST_SUSPEND
#undef TEST_PRIORITY

/*****************************************************************
| variable defintion (scope: modul-local)
|----------------------------------------------------------------*/
extern OSAL_tpfErrorHook pfUserErrorHook;
extern tU32 u32GlobalError;
extern tU32 u32ThreadCount;

/*****************************************************************
| function prototype (scope: module-local)
|----------------------------------------------------------------*/
tVoid vTest_PR_LV1(void);

/*****************************************************************
| function implementation (scope: module-local)
|----------------------------------------------------------------*/


/*****************************************************************
| function implementation (scope: global)
|----------------------------------------------------------------*/
static void taskFunction (void * notUsed)
{
  while(1)
  {
	//TRACE3_MSG0("\ntaskFunction called\n");
    OSAL_s32ThreadWait(20);
  }

}

/*****************************************************************************
 *
 * FUNCTION:    vTest_PR_LV1()
 *
 *
 * DESCRIPTION: This function is the main of the semaphore test
 *              using only 1 Thread
 *
 * PARAMETER:   none
 *
 *
 * RETURNVALUE: none
 *
 *
 * HISTORY:
 * Date      |   Modification                         | Authors
 * 05.07.02  |   Initial revision                     |
 *****************************************************************************/
tVoid vTest_PR_LV1(void)
{
   tS32 RetValue;
   OSAL_tThreadID ThreadID,InvalidID;
   OSAL_tThreadID T1_ID, T2_ID, T3_ID, T4_ID, T5_ID, T6_ID, T7_ID;

   OSAL_trThreadAttribute attrGood, attr;

#ifdef TEST_TCB
   OSAL_trThreadControlBlock tcb;
#endif
   OSAL_tpfErrorHook* oldHook=OSAL_NULL;
#if 0
 //  OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
   OSAL_vErrorHook(OSAL_vTestErrorHook,oldHook);
#endif

   TRACE1_MSG0("\n\rTest_PR_LV1 starting ");

   attrGood.szName = (char *)"T1";
   attrGood.u32Priority = OSAL_C_U32_THREAD_PRIORITY_HIGHEST+1;
   attrGood.s32StackSize = CONST_TARGET_OSALTEST_TASKSTACKSIZE_INIT;
   attrGood.pfEntry = taskFunction;
   attrGood.pvArg = NULL;

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TABLE - Check the correct ThreadTableEntry usage\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  Verify the allocation of new Thread entries after 4 new Thread Creation\n\r");

   TRACE3_MSG0("  P0 Create a new thread T1\n\r");
   attr        = attrGood;
   attr.szName = (char *)"T1";
   T1_ID = OSAL_ThreadCreate(&attr);
   CheckErr( T1_ID, OSAL_E_NOERROR );
   TRACE3_MSG1("   T1_ID = %d\n\r",T1_ID);

   TRACE3_MSG0("  P0 Create a new thread T2\n\r");
   attr        = attrGood;
   attr.szName = (char *)"T2";
   T2_ID = OSAL_ThreadCreate(&attr);
   CheckErr( T2_ID, OSAL_E_NOERROR );
   TRACE3_MSG1("   T2_ID = %d\n\r",T2_ID);

 //  OSAL_s32ThreadActivate( T1_ID );
 //  OSAL_s32ThreadActivate( T2_ID );

    TRACE3_MSG0("  P0 Create a new thread T3\n\r");
   attr        = attrGood;
   attr.szName = (char *)"T3";
   T3_ID = OSAL_ThreadCreate(&attr);
   CheckErr( T1_ID, OSAL_E_NOERROR );
   TRACE3_MSG1("   T3_ID = %d\n\r",T3_ID);

   TRACE3_MSG0("  P0 Create a new thread T4\n\r");
   attr        = attrGood;
   attr.szName = (char *)"T4";
   T4_ID = OSAL_ThreadCreate(&attr);
   CheckErr( T4_ID, OSAL_E_NOERROR );
   TRACE3_MSG1("   T4_ID = %d\n\r",T4_ID);

//   OSAL_s32ThreadActivate( T4_ID );
//   OSAL_s32ThreadActivate( T3_ID );

   TRACE3_MSG0("  P0 Create a new thread T5\n\r");
   attr        = attrGood;
   attr.szName = (char *)"T5";
   T5_ID = OSAL_ThreadCreate(&attr);
   CheckErr( T5_ID, OSAL_E_NOERROR );
   TRACE3_MSG1("   T5_ID = %d\n\r",T5_ID);

   TRACE3_MSG0("  P0 Create a new thread T6\n\r");
   attr        = attrGood;
   attr.szName = (char *)"T6";
   T6_ID = OSAL_ThreadCreate(&attr);
   CheckErr( T6_ID, OSAL_E_NOERROR );
   TRACE3_MSG1("   T6_ID = %d\n\r",T6_ID);

   TRACE3_MSG0("  P0 Delete the thread T6\n\r");
   RetValue = OSAL_s32ThreadDelete(T6_ID);
   CheckErr (RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Create a new thread T7\n\r");
   attr        = attrGood;
   attr.szName = (char *)"T7";
   T7_ID = OSAL_ThreadCreate(&attr);
   CheckErr( T7_ID, OSAL_E_NOERROR );
   TRACE3_MSG1("   T7_ID = %d\n\r",T7_ID);

   TRACE3_MSG0("  P0 Delete the thread T7");
   RetValue = OSAL_s32ThreadDelete(T7_ID);
   CheckErr (RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Delete the thread T5");
   RetValue = OSAL_s32ThreadDelete(T5_ID);
   CheckErr (RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Delete the thread T4");
   RetValue = OSAL_s32ThreadDelete(T4_ID);
   CheckErr (RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Delete the thread T3");
   RetValue = OSAL_s32ThreadDelete(T3_ID);
   CheckErr (RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Delete the thread T2");
   RetValue = OSAL_s32ThreadDelete(T2_ID);
   CheckErr (RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Delete the thread T1");
   RetValue = OSAL_s32ThreadDelete(T1_ID);
   CheckErr (RetValue, OSAL_E_NOERROR );

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC00 - Create a Thread with a name NULL\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Create the THREAD T1\n\r");
   attr        = attrGood;
   attr.szName = NULL;
   InvalidID = OSAL_ThreadCreate(&attr);
   CheckErr (InvalidID, OSAL_E_INVALIDVALUE );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC01 - Create a Thread with the name too long\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Create a THREAD with invalid name\n\r");
   attr        = attrGood;
   attr.szName = (char *)"|---------|---------|---------123";
   InvalidID = OSAL_ThreadCreate(&attr);
   CheckErr (InvalidID, OSAL_E_NAMETOOLONG );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC02 - Create a Thread with wrong priority (69)\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Create T1 with invalid priority\n\r");
   attr        = attrGood;
   attr.u32Priority = 69;
   InvalidID = OSAL_ThreadCreate(&attr);
   CheckErr (InvalidID, OSAL_E_INVALIDVALUE );

   //  the check on minimal size has been removed
//   /*********************************************************************************/
//   TRACE2_MSG0("\n\r Test_PR_LV1_TC03 - Create a Thread with undersized stack (1 byte)\n\r");
//   /*********************************************************************************/
//   TRACE3_MSG0("  P0 Try to Create T1 with invalid stack\n\r");
//   attr        = attrGood;
//   attr.s32StackSize = 1;
//   InvalidID = OSAL_ThreadCreate(&attr);
//   CheckErr (InvalidID, OSAL_E_INVALIDVALUE );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC04 - Create a Thread using a NULL pointer to the entry function\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Create T1 using NULL entry func\n\r");
   attr        = attrGood;
   attr.pfEntry = OSAL_NULL;
   InvalidID = OSAL_ThreadCreate(&attr);
   CheckErr (InvalidID, OSAL_E_INVALIDVALUE );


//  ABORT: the check if the value is incorrect has been cancelled
//   /*********************************************************************************/
//   TRACE2_MSG0("\n\r Test_PR_LV1_TC05 - Delete a not existing thread\n\r");
//   /*********************************************************************************/
//   TRACE3_MSG0("  P0 Try to Delete a thread with non existing ID\n\r");
//   RetValue = OSAL_s32ThreadDelete(7);
//   CheckErr (RetValue, OSAL_E_WRONGTHREAD );
//

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC06 - Delete a Thread with a NULL ID value (NULL)\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Delete a thread with invalid ID\n\r");
   RetValue = OSAL_s32ThreadDelete(0);
   CheckErr (RetValue, OSAL_E_INVALIDVALUE );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC07 - Create and Delete a valid thread\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Create a thread T1\n\r");
   attr        = attrGood;
   ThreadID = OSAL_ThreadCreate(&attr);
   CheckErr (ThreadID, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Delete the thread T1\n\r");
   RetValue = OSAL_s32ThreadDelete(ThreadID);
   CheckErr (RetValue, OSAL_E_NOERROR );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC08 - Create two threads with the same name\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Create the thread T1\n\r");
   ThreadID = OSAL_ThreadCreate(&attr);
   CheckErr (ThreadID, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Try to Create the thread T1 a second time\n\r");
   InvalidID = OSAL_ThreadCreate(&attr);
   CheckErr (InvalidID, OSAL_E_ALREADYEXISTS );

   TRACE3_MSG0("  P0 Delete the thread T1\n\r");
   RetValue = OSAL_s32ThreadDelete(ThreadID);
   CheckErr (RetValue, OSAL_E_NOERROR );

   //  ABORT: the check if the value is incorrect has been cancelled
//   /*********************************************************************************/
//   TRACE2_MSG0("\n\r Test_PR_LV1_TC09 - Activate a not existing thread\n\r");
//   /*********************************************************************************/
//   TRACE3_MSG0("  P0 Try to Activate a non existing thread\n\r");
//   RetValue = OSAL_s32ThreadActivate(7);
//   CheckErr (RetValue, OSAL_E_WRONGTHREAD );

#ifdef TEST_ACTIVATE
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC10 - Activate a Thread with a NULL ID value \n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Activate a thread with invalid ID\n\r");
   RetValue = OSAL_s32ThreadActivate((OSAL_tThreadID)NULL);
   CheckErr (RetValue, OSAL_E_INVALIDVALUE);


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC11 - Activate two times the same thread\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Create the thread T1\n\r");
   ThreadID = OSAL_ThreadCreate(&attr);
   CheckErr (ThreadID, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Activate the thread T1\n\r");
   RetValue = OSAL_s32ThreadActivate(ThreadID);
   CheckErr (RetValue, OSAL_E_NOERROR);

   TRACE3_MSG0("  P0 Try to Activate again the thread\n\r");
   RetValue = OSAL_s32ThreadActivate(ThreadID);
   CheckErr (RetValue,OSAL_E_WRONGTHREAD);

   TRACE3_MSG0("  P0 Delete the thread T1\n\r");
   RetValue = OSAL_s32ThreadDelete(ThreadID);
   CheckErr( RetValue, OSAL_E_NOERROR );
#endif // TEST_ACTIVATE

#ifdef TEST_SUSPEND
   //  ABORT: the check if the value is incorrect has been cancelled
//   /*********************************************************************************/
//   TRACE2_MSG0("\n\r Test_PR_LV1_TC12 - Suspend a not existing thread\n\r");
//   /*********************************************************************************/
//   TRACE3_MSG0("  P0 Try to Suspend a thread with non existing ID\n\r");
//   RetValue = OSAL_s32ThreadSuspend(7);
//   CheckErr (RetValue,OSAL_E_WRONGTHREAD);


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC13 - Suspend a Thread with a NULL ID value \n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Suspend a thread with invalid ID\n\r");
   RetValue = OSAL_s32ThreadSuspend((OSAL_tThreadID)NULL);
   CheckErr (RetValue,OSAL_E_INVALIDVALUE);

   //  ABORT: the check if the value is incorrect has been cancelled
//   /*********************************************************************************/
//   TRACE2_MSG0("\n\r Test_PR_LV1_TC14 - Resume a not existing thread\n\r");
//   /*********************************************************************************/
//   TRACE3_MSG0("  P0 Try to Resume a thread with non existing ID\n\r");
//   RetValue = OSAL_s32ThreadResume(7);
//   CheckErr (RetValue,OSAL_E_WRONGTHREAD);


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC15 - Resume a Thread with a NULL ID value \n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Resume a thread with invalid ID\n\r");
   RetValue = OSAL_s32ThreadResume((OSAL_tThreadID)NULL);
   CheckErr (RetValue,OSAL_E_INVALIDVALUE);

   //  ABORT: the check if the value is incorrect has been cancelled
//   /*********************************************************************************/
//   TRACE2_MSG0("\n\r Test_PR_LV1_TC16 - Change priority to a not existing thread\n\r");
//   /*********************************************************************************/
//   TRACE3_MSG0("  P0 Try to Change priority to a thread with non existing ID\n\r");
//   RetValue = OSAL_s32ThreadPriority(7,OSAL_C_U32_THREAD_PRIORITY_LOWEST);
//   CheckErr(RetValue, OSAL_E_WRONGTHREAD );
#endif // TEST_SUSPEND

#ifdef TEST_PRIORITY
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC17 - Change priority to a Thread with a NULL ID value \n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Change priority to a thread with invalid ID\n\r");
   RetValue = OSAL_s32ThreadPriority((OSAL_tThreadID)NULL,OSAL_C_U32_THREAD_PRIORITY_LOWEST);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC18 - Change priority to a Thread using a wrong value (69)\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Create the thread T1\n\r");
   attr     = attrGood;
   ThreadID = OSAL_ThreadCreate(&attr);
   CheckErr(ThreadID,  OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Try to Change T1 priority using an invalid value\n\r");
   RetValue = OSAL_s32ThreadPriority(ThreadID,69);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   TRACE3_MSG0("  P0 Delete T1\n\r");
   RetValue = OSAL_s32ThreadDelete(ThreadID);
   CheckErr(RetValue, OSAL_E_NOERROR );

   //  ABORT: the check if the value is incorrect has been cancelled
//     /*********************************************************************************/
//   TRACE2_MSG0("\n\r Test_PR_LV1_TC19 - Read the control block to a not existing thread\n\r");
//   /*********************************************************************************/
//   TRACE3_MSG0("  P0 Try to read TCB of a thread with non existing ID\n\r");
//   RetValue = OSAL_s32ThreadControlBlock(7,&tcb);
//   CheckErr( RetValue, OSAL_E_WRONGTHREAD );
#endif // TEST_PRIORITY

#ifdef TEST_TCB
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC20 - Read the control block to a Thread with a NULL ID value \n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Try to Read TCB of a thread with invalid ID\n\r");
   RetValue = OSAL_s32ThreadControlBlock((OSAL_tThreadID)NULL,&tcb);
   CheckErr( RetValue, OSAL_E_INVALIDVALUE );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_PR_LV1_TC21 - Read the control block in different status condition\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  P0 Spawn a new thread T1\n\r");
   attr        = attrGood;
   ThreadID = OSAL_ThreadSpawn(&attr);
   CheckErr( ThreadID, OSAL_E_NOERROR );

   TRACE3_MSG0("  P0 Read the TCB of T1\n\r");
   RetValue = OSAL_s32ThreadControlBlock(ThreadID,&tcb);
   CheckErr( RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  Thread Control Block:\n\r");
   TRACE3_MSG1("   ID           = %d\n\r",tcb.id);
   TRACE3_MSG1("   Name         = %s\n\r",tcb.szName);
   TRACE3_MSG1("   InitPriority = %d\n\r",tcb.u32Priority);
   TRACE3_MSG1("   CurrPriority = %d\n\r",tcb.u32CurrentPriority);
   TRACE3_MSG1("   StackSize    = %d\n\r",tcb.s32StackSize);
   TRACE3_MSG1("   UsedStack    = %d\n\r",tcb.s32UsedStack);
   TRACE3_MSG1("   Starttime    = %d ms\n\r",tcb.startTime);
   TRACE3_MSG1("   Slicetime    = %d ms\n\r",tcb.sliceTime);
   TRACE3_MSG1("   Runtime      = %d ms\n\r",tcb.runningTime);
   TRACE3_MSG1("   Status       = %d\n\r",tcb.enStatus);
   TRACE3_MSG1("   Error        = %#X\n\r",tcb.u32ErrorCode);


   TRACE3_MSG0("  P0 Suspend T1\n\r");
   RetValue = OSAL_s32ThreadSuspend(ThreadID);
   CheckErr( RetValue, OSAL_E_NOERROR );


   TRACE3_MSG0("  P0 Read the TCB of T1\n\r");
   RetValue = OSAL_s32ThreadControlBlock(ThreadID,&tcb);
   CheckErr( RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  Thread Control Block:\n\r");
   TRACE3_MSG1("   ID           = %d\n\r",tcb.id);
   TRACE3_MSG1("   Name         = %s\n\r",tcb.szName);
   TRACE3_MSG1("   InitPriority = %d\n\r",tcb.u32Priority);
   TRACE3_MSG1("   CurrPriority = %d\n\r",tcb.u32CurrentPriority);
   TRACE3_MSG1("   StackSize    = %d\n\r",tcb.s32StackSize);
   TRACE3_MSG1("   UsedStack    = %d\n\r",tcb.s32UsedStack);
   TRACE3_MSG1("   Starttime    = %d ms\n\r",tcb.startTime);
   TRACE3_MSG1("   Slicetime    = %d ms\n\r",tcb.sliceTime);
   TRACE3_MSG1("   Runtime      = %d ms\n\r",tcb.runningTime);
   TRACE3_MSG1("   Status       = %d\n\r",tcb.enStatus);
   TRACE3_MSG1("   Error        = %#X\n\r",tcb.u32ErrorCode);

   TRACE3_MSG0("  P0 Resume T1\n\r");
  RetValue = OSAL_s32ThreadResume(ThreadID);
  CheckErr( RetValue, OSAL_E_NOERROR );

  TRACE3_MSG0("  P0 Read the TCB of T1\n\r");
  RetValue = OSAL_s32ThreadControlBlock(ThreadID,&tcb);
  CheckErr( RetValue, OSAL_E_NOERROR );

  TRACE3_MSG0("  Thread Control Block:\n\r");
  TRACE3_MSG1("   ID           = %d\n\r",tcb.id);
  TRACE3_MSG1("   Name         = %s\n\r",tcb.szName);
  TRACE3_MSG1("   InitPriority = %d\n\r",tcb.u32Priority);
  TRACE3_MSG1("   CurrPriority = %d\n\r",tcb.u32CurrentPriority);
  TRACE3_MSG1("   StackSize    = %d\n\r",tcb.s32StackSize);
  TRACE3_MSG1("   UsedStack    = %d\n\r",tcb.s32UsedStack);
  TRACE3_MSG1("   Starttime    = %d ms\n\r",tcb.startTime);
  TRACE3_MSG1("   Slicetime    = %d ms\n\r",tcb.sliceTime);
  TRACE3_MSG1("   Runtime      = %d ms\n\r",tcb.runningTime);
  TRACE3_MSG1("   Status       = %d\n\r",tcb.enStatus);
  TRACE3_MSG1("   Error        = %#X\n\r",tcb.u32ErrorCode);


   TRACE3_MSG0("  P0 Change priority of T1\n\r");
   RetValue = OSAL_s32ThreadPriority(ThreadID,45);
   CheckErr( RetValue, OSAL_E_NOERROR );


   TRACE3_MSG0("  P0 Read the TCB of T1\n\r");
   RetValue = OSAL_s32ThreadControlBlock(ThreadID,&tcb);
   CheckErr( RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  Thread Control Block:\n\r");
   TRACE3_MSG1("   ID           = %d\n\r",tcb.id);
   TRACE3_MSG1("   Name         = %s\n\r",tcb.szName);
   TRACE3_MSG1("   InitPriority = %d\n\r",tcb.u32Priority);
   TRACE3_MSG1("   CurrPriority = %d\n\r",tcb.u32CurrentPriority);
   TRACE3_MSG1("   StackSize    = %d\n\r",tcb.s32StackSize);
   TRACE3_MSG1("   UsedStack    = %d\n\r",tcb.s32UsedStack);
   TRACE3_MSG1("   Starttime    = %d ms\n\r",tcb.startTime);
   TRACE3_MSG1("   Slicetime    = %d ms\n\r",tcb.sliceTime);
   TRACE3_MSG1("   Runtime      = %d ms\n\r",tcb.runningTime);
   TRACE3_MSG1("   Status       = %d\n\r",tcb.enStatus);
   TRACE3_MSG1("   Error        = %#X\n\r",tcb.u32ErrorCode);

   TRACE3_MSG0("  P0 Delete T1\n\r");
   RetValue = OSAL_s32ThreadDelete(ThreadID);
   CheckErr( RetValue, OSAL_E_NOERROR );

#endif // TEST_TCB

   if ( u32GlobalError== 0 )
   {
      TRACE1_MSG0("Test_PR_LV1 PASSED---\n\n\n\r" );
   }
   else
   {
      TRACE1_MSG0("Test_PR_LV1 FAILED---\n\n\n\r" );
      TRACE2_MSG1("u32GlobalError= %d\n\n\n\r", u32GlobalError );
   }
   TRACE1_MSG0("\n TEST OF FILE TEST_PR_LV1.C FINISHED\n\n\n");
   return;
}

