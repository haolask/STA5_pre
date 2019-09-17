#include <stdio.h>
#include <stdlib.h>
#include "osal.h"
#include "osaltest_util.h"


/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
#define EVENT_1     0x00000003
#define EVENT_2     0xC0000000


/*****************************************************************
| variable defintion (scope: modul-local)
|----------------------------------------------------------------*/
//extern OSAL_tpfErrorHook pfUserErrorHook=OSAL_NULL;
extern OSAL_tpfErrorHook pfUserErrorHook;
extern tU32 u32GlobalError;
extern tU32 u32ThreadCount;

/*****************************************************************
| function prototype (scope: module-local)
|----------------------------------------------------------------*/
tVoid vTest_EV_LV1(void);

/*****************************************************************
| function implementation (scope: module-local)
|----------------------------------------------------------------*/

/*****************************************************************
| function implementation (scope: global)
|----------------------------------------------------------------*/







/*****************************************************************************
 *
 * FUNCTION:    vTest_EV_LV1()
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
tVoid vTest_EV_LV1(void)
{
   tU32 RetValue;
   OSAL_tEventHandle event0, event1;

#ifndef   OSAL_EVENT_SKIP_NAMES
   OSAL_tEventHandle event2 = 0, event3;
#endif

   OSAL_tEventMask outmask;
   OSAL_tEventMask actual_flags;
   OSAL_tEventMask inmask=0xFFFFFFFF;
   OSAL_tpfErrorHook* oldHook=OSAL_NULL;
#if 0
   OSAL_vErrorHook(OSAL_vTestErrorHook,oldHook);
#endif

   TRACE3_MSG0("\n\rTest_EV_LV1 starting . . . . . . ");

#ifndef OSAL_EVENT_SKIP_NAMES
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC00 - Create an event with a name NULL\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the EV\n\r");
   RetValue = OSAL_s32EventCreate(NULL, &event0);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );
#endif
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC01 - Create an event with a handle NULL\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the EV\n\r");
   RetValue = OSAL_s32EventCreate("any", NULL);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC02 - Create an event with the name length at the max\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the EV\n\r");
   RetValue = OSAL_s32EventCreate("|---------|---------|---------1", &event0);
   CheckErr(RetValue, OSAL_E_NOERROR );

#ifdef OSAL_EVENT_SKIP_NAMES
   TRACE3_MSG0("  T0 Free the EV\n\r");
   RetValue = OSAL_s32EventFree(event0);
   CheckErr(RetValue, OSAL_E_NOERROR );
#else
   TRACE3_MSG0("  T0 Close the EV\n\r");
   RetValue = OSAL_s32EventClose(event0);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Delete the EV\n\r");
   RetValue = OSAL_s32EventDelete("|---------|---------|---------1");
   CheckErr(RetValue, OSAL_E_NOERROR );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC03 - Create an event with the name length at the max +1\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Create the EV with a name too long\n\r");
   RetValue = OSAL_s32EventCreate("|---------|---------|---------12", &event0);
   CheckErr(RetValue, OSAL_E_NAMETOOLONG );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC04 - Create two events with the same name\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the first time the EV\n\r");
   RetValue = OSAL_s32EventCreate("DEMO0", &event0);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Try to Create the EVENT a second time\n\r");
   RetValue = OSAL_s32EventCreate("DEMO0", &event0);
   CheckErr(RetValue, OSAL_E_ALREADYEXISTS );
#endif

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC05 - Create an event with regular name\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the EV\n\r");
   RetValue = OSAL_s32EventCreate("DEMO1", &event1);
   CheckErr(RetValue, OSAL_E_NOERROR );

#ifndef   OSAL_EVENT_SKIP_NAMES
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC06 - Delete an event using a NULL Handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Delete an EV using a NULL Handle\n\r");
   RetValue = OSAL_s32EventDelete(NULL);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC07 - Delete a non existing event\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Delete a non existing EV\n\r");
   RetValue = OSAL_s32EventDelete("DEMO2");
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC08 - Delete an event with a name exceeding the max length\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Delete the EV\n\r");
   RetValue = OSAL_s32EventDelete("|---------|---------|---------12");
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC09 - Delete an event still in use\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Create the EV\n\r");
   RetValue = OSAL_s32EventCreate("DEMO2", &event1);
   CheckErr(RetValue, OSAL_E_NOERROR );

   TRACE3_MSG0("  T0 Try to Delete the EV\n\r");
   RetValue = OSAL_s32EventDelete("DEMO2");
   CheckErr(RetValue, OSAL_E_BUSY );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC10 - Open an existing event\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Open the EV\n\r");
   RetValue = OSAL_s32EventOpen("DEMO0", &event1);
   CheckErr(RetValue, OSAL_E_NOERROR );

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC11 - Open an event with a NULL name\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Open an EV using a NULL Name\n\r");
   RetValue = OSAL_s32EventOpen(NULL, &event2);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC12 - Open an event with a NULL handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Open an EV using a NULL handle\n\r");
   RetValue = OSAL_s32EventOpen("any", NULL);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC13 - Open an non existing event\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Open the EV\n\r");
   RetValue = OSAL_s32EventOpen("DEMO3", &event3);
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC14 - Open an event with a name exceeding the max length\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Open the EV\n\r");
   RetValue = OSAL_s32EventOpen("|---------|---------|---------12", &event3);
   CheckErr(RetValue, OSAL_E_DOESNOTEXIST );
#endif

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC15 - Close a connection to a NULL event\n\r");
   /*********************************************************************************/
#ifdef OSAL_EVENT_SKIP_NAMES
   TRACE3_MSG0("  T0 Free the EV\n\r");
   RetValue = OSAL_s32EventFree((OSAL_tEventHandle)NULL);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );
#else
   TRACE3_MSG0("  T0 Close the EV\n\r");
   RetValue = OSAL_s32EventClose((OSAL_tEventHandle)NULL);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );
#endif


#if !defined (OSAL_EVENT_SKIP_NAMES) && !defined (OSAL_EVENT_LIGHT)
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC16 - Close a connection using an invalid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Close the EV\n\r");
   RetValue = OSAL_s32EventClose(event2);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );
#endif

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC17 - Get the event status using a valid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Get the EV status\n\r");
   RetValue = OSAL_s32EventStatus(event1,inmask, &outmask );
   TRACE3_MSG1("  outmask=0x%x\n", outmask);
   CheckErr(RetValue, OSAL_E_NOERROR );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC18 - Get the event status using a NULL handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Get the EV status\n\r");
   RetValue = OSAL_s32EventStatus((OSAL_tEventHandle)NULL,inmask, &outmask );
   TRACE3_MSG1("  outmask=0x%x\n", outmask);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

#if !defined (OSAL_EVENT_SKIP_NAMES) && !defined (OSAL_EVENT_LIGHT)
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC19 - Get the event status using an invalid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Get the EV status\n\r");
   RetValue = OSAL_s32EventStatus(event2,inmask, &outmask );
   TRACE3_MSG1("  outmask=0x%x\n", outmask);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );
#endif

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC20 - Post an event using a valid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Post the EV\n\r");
   RetValue = OSAL_s32EventPost(event1, EVENT_1, OSAL_EN_EVENTMASK_OR);
   CheckErr(RetValue, OSAL_E_NOERROR );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC21 - Post an event using a NULL handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Post the EV\n\r");
   RetValue = OSAL_s32EventPost((OSAL_tEventHandle)NULL, EVENT_1, OSAL_EN_EVENTMASK_OR);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

#if !defined (OSAL_EVENT_SKIP_NAMES) && !defined (OSAL_EVENT_LIGHT)
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC22 - Post an event using an invalid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Post the EV\n\r");
   RetValue = OSAL_s32EventPost(event2, EVENT_2, OSAL_EN_EVENTMASK_OR);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );
#endif

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC23 - Wait a posted event using a valid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Wait the EV\n\r");
   RetValue = OSAL_s32EventWait(event1,
                                EVENT_1,
                                OSAL_EN_EVENTMASK_OR,
                                OSAL_C_TIMEOUT_NOBLOCKING,
                                &actual_flags);
   TRACE3_MSG1("  actual_flags=0x%x\n", actual_flags);
   CheckErr(RetValue, OSAL_E_NOERROR );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC24 - Wait a posted event using a NULL handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Wait the EV\n\r");
   RetValue = OSAL_s32EventWait((OSAL_tEventHandle)NULL,
                                EVENT_1,
                                OSAL_EN_EVENTMASK_OR,
                                OSAL_C_TIMEOUT_NOBLOCKING,
                                &actual_flags);
   TRACE3_MSG1("  actual_flags=0x%x\n", actual_flags);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );

#if !defined (OSAL_EVENT_SKIP_NAMES) && !defined (OSAL_EVENT_LIGHT)
   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC25 - Wait a posted event using an invalid handle\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Wait the EV\n\r");
   RetValue = OSAL_s32EventWait(event2,
                                EVENT_1,
                                OSAL_EN_EVENTMASK_OR,
                                OSAL_C_TIMEOUT_NOBLOCKING,
                                &actual_flags);
   TRACE3_MSG1("  actual_flags=0x%x\n", actual_flags);
   CheckErr(RetValue, OSAL_E_INVALIDVALUE );
#endif

   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC26 - Wait an unposted event without timeout\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Wait the EV\n\r");
   RetValue = OSAL_s32EventWait(event1,
                         EVENT_2,
                         OSAL_EN_EVENTMASK_OR,
                         OSAL_C_TIMEOUT_NOBLOCKING,
                         &actual_flags);
   TRACE3_MSG1("  actual_flags=0x%x\n", actual_flags);
   CheckErr(RetValue, OSAL_E_NOERROR );


   /*********************************************************************************/
   TRACE2_MSG0("\n\r Test_EV_LV1_TC27 - Wait an unposted event with timeout\n\r");
   /*********************************************************************************/
   TRACE3_MSG0("  T0 Try to Wait the EV\n\r");
   RetValue = OSAL_s32EventWait(event1,
                         EVENT_2,
                         OSAL_EN_EVENTMASK_OR,
                         20,
                         &actual_flags);
   TRACE3_MSG1("  actual_flags=0x%x\n\r", actual_flags);
   CheckErr(RetValue, OSAL_E_TIMEOUT ); // OSAL_E_NOERROR in the TUNER_MODULE version of the test


   if ( u32GlobalError== 0 )
   {
      TRACE3_MSG0("Test_EV_LV1 PASSED---\n\n\n\r" );
   }
   else
   {
      TRACE3_MSG0("Test_SM_LV1 FAILED---\n\n\n\r" );
      TRACE3_MSG1("u32GlobalError= %d\n\n\n\r", u32GlobalError );
   }
}
