/************************************************************************
* PROJECT : OSAL CORE on OS21
* FILE : osaltest.c
*
* DESCRIPTION : This is the main file for the OSAL
*               (Operating System Abstraction Layer) test suite.
*
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
* Revision 1.1.1.2  2010/01/29 12:49:47  gol2hi
* Import from STM
*
* Revision 1.1  2009/09/11 09:51:37  shz2hi
* new OS21 version with ADR3 support
*
* Revision 1.5  2009/05/25 08:20:11  shz2hi
* new ST delivery OSAL_22_5_2009
*
************************************************************************/

#ifndef OSALTEST_C
#define OSALTEST_C

#include <stdio.h>
#include <stdlib.h>

#include "osal.h"
#include "osaltest.h"
#include "osaltest_util.h"

   /*****************************************************************
   | defines and macros (scope: module-local)
   |----------------------------------------------------------------*/

   /*****************************************************************
   | variable defintion (scope: modul-local)
   |----------------------------------------------------------------*/
	extern   tU32 u32ThreadCount;
   /*****************************************************************
   | function prototype (scope: module-local)
   |----------------------------------------------------------------*/


//extern void interrupt_function(void);
extern tVoid vTest_MS_LV1(void); /* Message Pool */
extern tVoid vTest_EV_LV1(void); /* Events */
extern tVoid vTest_EV_LV2_TC1(void);
extern tVoid vTest_MQ_LV1(void); /* Message Queue */
extern tVoid vTest_MQ_LV2_TC1(void);
extern tVoid vTest_MQ_LV2_TC2(void);
extern tVoid vTest_MQ_LV2_TC3(void);
extern tVoid vTest_PR_LV1(void); /* Threads*/
extern tVoid vTest_SH_LV1(void); /* Shared Memory */
extern tVoid vTest_SM_LV1(void); /* Semapohores*/
extern tVoid vTest_SM_LV2_TC1(void);
extern tVoid vTest_TM_LV1(void); /* Timer */
extern tVoid vTest_CH_LV1(void); // cache

   /*****************************************************************
   | function implementation (scope: module-local)
   |----------------------------------------------------------------*/

   /*****************************************************************
   | function implementation (scope: global)
   |----------------------------------------------------------------*/

static tU32 end_osalcore_test = 0;
   /*****************************************************************************
    *
    * FUNCTION:    osalcore_test()
    *
    *
    * DESCRIPTION: This function is the main thread for the test execution
    *
    *
    * PARAMETER:   tS32 cPar        number of arguments;
    *              tString aPar[]   pointer to arguments;
    *
    *
    * RETURNVALUE: none
    *
    *
    * HISTORY:
    * Date      |   Modification                         | Authors
    * 29.01.02  |   Initial revision                     |
    *****************************************************************************/
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid osalcore_test_function(tSInt stacd, tPVoid notUsed)
#else
static void osalcore_test_function (void * notUsed)
#endif
{
	tS32 s32Count = 0;

       while( s32Count<13)
      {
         if( !u32ThreadCount )
         {
            /* In this point select the main of the test case */
            switch( s32Count++ )
            {
#ifdef CONFIG_APP_OSALCORE_SEMAPHORE_TEST
               case(0):
                  {
            	   //  interrupt_function();

                     vTest_SM_LV1();  /*Initial Test To Try the new test suite enviroment*/
            	   break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_MSG_QUEUE_TEST
               case(1):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_MQ_LV1();  /*Initial Test To Try the new test suite enviroment*/
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_EVENTS_TEST
               case(2):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_EV_LV1();  /*Initial Test To Try the new test suite enviroment*/
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_THREADS_TEST
               case(3):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_PR_LV1();  /*Initial Test To Try the new test suite enviroment*/
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_TIMER_TEST
               case(4):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_TM_LV1();  /*Initial Test To Try the new test suite enviroment*/
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_SH_MEMORY_TEST
               case(5):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_SH_LV1();
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_MSG_POOL_TEST
               case(6):
                  {

                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_MS_LV1();
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_SEMAPHORE_TEST
               case(7):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_SM_LV2_TC1();
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_MSG_QUEUE_TEST
               case(8):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_MQ_LV2_TC1();
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_MSG_QUEUE_TEST
               case(9):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_MQ_LV2_TC2();
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_EVENTS_TEST
	#if 0 // commented aout due to lack of OSAL thread primitives (suspend, resume)
               case(10):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_EV_LV2_TC1();
                     break;
                  }
	#endif
#endif
#ifdef CONFIG_APP_OSALCORE_MSG_QUEUE_TEST
               case(11):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
                     vTest_MQ_LV2_TC3();
                     break;
                  }
#endif
#ifdef CONFIG_APP_OSALCORE_CACHE_TEST
               case(12):
                  {
                     //OSAL_s32ThreadPriority(OSAL_ThreadWhoAmI(), OSAL_C_U32_THREAD_PRIORITY_HIGHEST);
					  vTest_CH_LV1();
                     break;
                  }
#endif
               default:
                  {
                     break;
                  }
            }
         }
      }
       TRACE1_MSG0("\n ALL TESTS ARE FINISHED - FILE OSALTEST.C\n\n");

   end_osalcore_test = 1;

   }


tS32 osalcore_test(tS32 cPar, tString aPar[])
   {

	   OSAL_trThreadAttribute attr;

	   attr.szName = (char *) "OSALCORE_TEST";
	   attr.u32Priority = OSAL_C_U32_THREAD_PRIORITY_NORMAL;
	   attr.s32StackSize = CONST_TARGET_OSALTEST_TASKSTACKSIZE_INIT;
	   attr.pfEntry = osalcore_test_function;
	   attr.pvArg = NULL;

	   OSAL_ThreadSpawn(&attr);

	   while (end_osalcore_test == 0)
	   {
		   OSAL_s32ThreadWait(20);
	   }

	   return 0;
   }

#endif


