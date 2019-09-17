/************************************************************************
* PROJECT : OSAL CORE on OS21
* FILE : Test_SM_LV1.c
*
*-----------------------------------------------------------------------------
* DESCRIPTION:  This is the test file for the OSAL
*               (Operating System Abstraction Layer) test suite.
*               This file check the correct error code returned.
*
*               Semaphore_Create()
*                  OSAL_E_INVALIDVALUE
*                  OSAL_E_NAMETOOLONG
*                  OSAL_E_ALREADYEXISTS
*                  OSAL_E_NOSPACE
*                  OSAL_E_DOESNOTEXIST
*
*
*               Semaphore_Delete()
*                  OSAL_E_INVALIDVALUE
*                  OSAL_E_DOESNOTEXIST
*                  OSAL_E_BUSY
*
*               Semaphore_Open()
*                  OSAL_E_INVALIDVALUE
*                  OSAL_E_NAMETOOLONG (1) -- not used
*                  OSAL_E_DOESNOTEXIST
*
*              Semaphore_Close()
*                  OSAL_E_INVALIDVALUE
*                  OSAL_E_DOESNOTEXIST
*
*               Semaphore_Wait()
*                  OSAL_E_DOESNOTEXIST
*                  OSAL_E_TIMEOUT
*
*               Semaphore_Post()
*                  OSAL_E_INVALIDVALUE
*                  OSAL_E_DOESNOTEXIST
*
*               Semaphore_GetValue()
*                  OSAL_E_DOESNOTEXIST
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
* Revision 1.1  2009/09/11 09:51:37  shz2hi
* new OS21 version with ADR3 support
*
* Revision 1.4  2009/05/25 08:20:12  shz2hi
* new ST delivery OSAL_22_5_2009
*
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "osal.h"
#include "osaltest_util.h"


/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
#ifndef OSINIT_C
 #define OS21_C_STRING_SEMAPHORE_TABLELOCK      "SmphLock"
#endif
/*****************************************************************
| variable defintion (scope: modul-local)
|----------------------------------------------------------------*/
extern OSAL_tpfErrorHook pfUserErrorHook;
extern tU32 u32GlobalError;
extern tU32 u32ThreadCount;
/*****************************************************************
| function prototype (scope: module-local)
|----------------------------------------------------------------*/
tVoid vTest_SM_LV1(void);

/*****************************************************************
| function implementation (scope: module-local)
|----------------------------------------------------------------*/
tS32  semVal;

/*****************************************************************
| function implementation (scope: global)
|----------------------------------------------------------------*/

/*****************************************************************************
 *
 * FUNCTION:    vTest_SM_LV1()
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
tVoid vTest_SM_LV1(void)
{
   tS32 RetValue=0;
   OSAL_tSemHandle semphr0 = 0;
#ifndef OSAL_SEMAPHORE_SKIP_NAMES
   OSAL_tSemHandle semphr1 = 0;
#endif
   /* At the beginning of test?*/
   OSAL_tpfErrorHook * oldHook=OSAL_NULL;
#if 0
   OSAL_vErrorHook(OSAL_vTestErrorHook,oldHook);
#endif

   TRACE1_MSG0("\n\rTest_SM_LV1 starting ");


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_SM_LV1_TC00 - Creating,closing and deleting a valid semaphore\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create SM0\n\r");
   RetValue = OSAL_s32SemaphoreCreate("DEMO00", &semphr0,1);
   CheckErr(RetValue, OSAL_E_NOERROR );

#ifdef OSAL_SEMAPHORE_SKIP_NAMES
   TRACE3_MSG0("  Free the Smphr...\n\r");
   RetValue = OSAL_s32SemaphoreFree(semphr0);
   CheckErr(RetValue, OSAL_E_NOERROR );
#else
   TRACE3_MSG0("  Close the Smphr...\n\r");
   RetValue = OSAL_s32SemaphoreClose(semphr0);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  Delete the Smphr ...\n\r");
   RetValue = OSAL_s32SemaphoreDelete("DEMO00");
   CheckErr(RetValue, OSAL_E_NOERROR );

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC01 - Creating a semaphore with the name NULL\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the Smphr ...\n\r");
   RetValue = OSAL_s32SemaphoreCreate(NULL, &semphr0,1);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );
#endif
   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC01B - Creating a semaphore with NULL handle\n\r");
   /*********************************************************************************/

   TRACE3_MSG0("  T0 Create the Smphr ...\n\r");
   RetValue = OSAL_s32SemaphoreCreate("any",NULL ,1);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC02 - Create a semaphore with the name length at the max\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the Smphr ...\n\r");
   RetValue = OSAL_s32SemaphoreCreate("---------1---------2---------31", &semphr0,1);
   CheckErr(RetValue, OSAL_E_NOERROR );

#ifdef OSAL_SEMAPHORE_SKIP_NAMES
   TRACE3_MSG0("  Free the Smphr...\n\r");
   RetValue = OSAL_s32SemaphoreFree(semphr0);
   CheckErr(RetValue, OSAL_E_NOERROR );
#else
   TRACE3_MSG0("  Close the Smphr...\n\r");
   RetValue = OSAL_s32SemaphoreClose(semphr0);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Delete the Smphr ...\n\r");
   RetValue = OSAL_s32SemaphoreDelete("---------1---------2---------31");
   CheckErr(RetValue, OSAL_E_NOERROR );


   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC03 - Create a semaphore with the name length at the max +1\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the Smphr ...\n\r");
   RetValue = OSAL_s32SemaphoreCreate("---------1---------2---------3123", &semphr0,1);
   CheckErr(RetValue, OSAL_E_NAMETOOLONG );


   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC04 - Creating two semaphores with the same name\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the first Smphr\n\r");
   RetValue = OSAL_s32SemaphoreCreate("DEMO00", &semphr0,1);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Create the second Smphr\n\r");
   RetValue = OSAL_s32SemaphoreCreate("DEMO00", &semphr0,1);
   CheckErr(RetValue, OSAL_E_ALREADYEXISTS );


   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC05 - Deleting a semaphore with a NULL name\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Delete a Smphr\n\r");
   RetValue = OSAL_s32SemaphoreDelete(NULL);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC06 - Deleting a semaphore with not_exist name \n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Delete a Smphr\n\r");
   RetValue = OSAL_s32SemaphoreDelete("WRONG");
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC07 - Deleting a semaphore busy\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Delete the Smphr when it is still in use\n\r");
   RetValue = OSAL_s32SemaphoreDelete("DEMO00");
   CheckErr(RetValue, OSAL_E_BUSY );

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC08 - Opening a semaphore with a NULL name\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Open a NULL SM\n\r");
   RetValue = OSAL_s32SemaphoreOpen(NULL, &semphr0);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC08B - Opening a semaphore with a NULL handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Open a NULL handle SM\n\r");
   RetValue = OSAL_s32SemaphoreOpen("Any", NULL);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC09 - Opening a semaphore with a name exceeding 32 chars\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Open a SM with a name too long\n\r");
   RetValue = OSAL_s32SemaphoreOpen("---------1---------2---------3123", &semphr0);
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );


   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC10 - Opening a semaphore with not_exist name\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Open a not existing SM\n\r");
   RetValue = OSAL_s32SemaphoreOpen("NOTEXIST", &semphr0);
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );
#endif

#if !defined (OSAL_SEMAPHORE_SKIP_NAMES) && !defined (OSAL_SEMAPHORE_LIGHT)
   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC11 - Closing a semaphore with not_valid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Close a SM using an invalid handle\n\r");
   RetValue = OSAL_s32SemaphoreClose(semphr1);
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );
#endif

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC12 - Waiting/posting a valid semaphore\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Obtain SM0\n\r");
   RetValue = OSAL_s32SemaphoreWait(semphr0,50);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Release SM0\n\r");
   RetValue = OSAL_s32SemaphorePost(semphr0);
   CheckErr(RetValue, OSAL_E_NOERROR );

#if !defined (OSAL_SEMAPHORE_SKIP_NAMES) && !defined (OSAL_SEMAPHORE_LIGHT)
   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC13 - Waiting a semaphore with not_valid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Obtain a SM using an invalid handle\n\r");
   RetValue = OSAL_s32SemaphoreWait(semphr1,50);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC14 - Release a semaphore with not_valid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Post a SM using an invalid handle\n\r");
   RetValue = OSAL_s32SemaphorePost(semphr1);
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );
#endif

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC15 - Get a regular semaphoreValue \n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Get a SM value\n\r");
   RetValue = OSAL_s32SemaphoreGetValue(semphr0,&semVal);
   CheckErr(RetValue, OSAL_E_NOERROR );
   TRACE3_MSG1("  semVal = %d\n\r", semVal);

#if !defined (OSAL_SEMAPHORE_SKIP_NAMES) && !defined (OSAL_SEMAPHORE_LIGHT)
   /*********************************************************************************/
   TRACE3_MSG0("\r\n Test_SM_LV1_TC16 - Get a semaphoreValue with not_valid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Get a SM value using an invalid handle\n\r");
   RetValue = OSAL_s32SemaphoreGetValue(semphr1,&semVal);
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );
#endif

   /*********************************************************************************/
   TRACE2_MSG0("\r\n Test_SM_LV1_TC17 - Waiting & posting a valid semaphore one more than MaxValue\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Obtain Smphr with regular name\n\r");
   RetValue = OSAL_s32SemaphoreWait(semphr0,50);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Release Smphr with regular name\n\r");
   RetValue = OSAL_s32SemaphorePost(semphr0);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Get the SM value\n\r");
   RetValue = OSAL_s32SemaphoreGetValue(semphr0,&semVal);
   TRACE3_MSG1("  %d\n\r",semVal);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Release Smphr with regular name\n\r");
   RetValue = OSAL_s32SemaphorePost(semphr0);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Get the SM value\n\r");
   RetValue = OSAL_s32SemaphoreGetValue(semphr0,&semVal);
   TRACE3_MSG1("  %d\n\r",semVal);
   CheckErr(RetValue, OSAL_E_NOERROR );


   if ( u32GlobalError== 0 )
   {
      TRACE1_MSG0("Test_SM_LV1 PASSED---\n\n\n\r" );
   }
   else
   {
      TRACE1_MSG0("Test_SM_LV1 FAILED---\n\n\n\r" );
      TRACE2_MSG1("u32GlobalError= %d\n\n\n\r", u32GlobalError );
   }
   TRACE1_MSG0("\n TEST OF FILE TEST_SM_LV1.C FINISHED\n\n\n");
}

