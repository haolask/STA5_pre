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
* Revision 1.1.1.1  2010/01/29 12:49:47  gol2hi
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
#include <stdarg.h>

#include "osal.h"
#include "osaltest_util.h"

   /*****************************************************************
   | defines and macros (scope: module-local)
   |----------------------------------------------------------------*/

   /*****************************************************************
   | variable defintion (scope: modul-local)
   |----------------------------------------------------------------*/

   static OSAL_tpfErrorHook pfUserErrorHook=OSAL_NULL;
   tU32 u32GlobalError = 0;
   tU32 u32ThreadCount = 0;


   /*****************************************************************
   | function prototype (scope: module-local)
   |----------------------------------------------------------------*/

   /*****************************************************************
   | function implementation (scope: module-local)
   |----------------------------------------------------------------*/

   /*****************************************************************
   | function implementation (scope: global)
   |----------------------------------------------------------------*/
   /*****************************************************************************
    *
    * FUNCTION:    CheckErr( tS32 RetValue, tU32 u32ExpectedError )
    *
    *
    * DESCRIPTION: This function is a printf() compatible with serial support
    *
    *
    * PARAMETER:   tS32 RetValue         : The value returned by a function
    *              tU32 u32ExpectedError : The error returned by a function
    *
    *
    * RETURNVALUE: none
    *
    *
    * HISTORY:
    * Date      |   Modification                         | Authors
    * 29.01.02  |   Initial revision                     |
    *****************************************************************************/


   tVoid CheckErr( tS32 RetValue, tU32 u32ExpectedError )
   {
      if( RetValue == OSAL_OK )
         TRACE3_MSG0("  SUCCESS\n\r");
      if( OSAL_u32ErrorCode() != u32ExpectedError )
         u32GlobalError++;
      OSAL_vSetErrorCode( OSAL_E_NOERROR );  // very important, it restore the flag u32ErrorCode each time!!!
      TRACE3_MSG1("  GlobalError = %d\n\r", u32GlobalError );
   }


   /*****************************************************************************
    *
    * FUNCTION:     CountUnmatched
    *
    * DESCRIPTION: This function compare the Expected error with
    *              error returned by OSAL_u32ErrorCode
    *
    * PARAMETER:   tU32 u32ExpectedError:  expected error code
    *
    * RETURNVALUE: none
    *
    * HISTORY:
    * Date      |   Modification                         | Authors
    * 29.01.02  |   Initial revision                     |
    *****************************************************************************/
   tVoid CountUnmatched( tU32 u32ExpectedError )
   {

      if( OSAL_u32ErrorCode() != u32ExpectedError )
         u32GlobalError++;
      OSAL_vSetErrorCode( OSAL_E_NOERROR );
      TRACE3_MSG1("  GlobalError = %d\n\n\r", u32GlobalError );
   }



   /*****************************************************************************
    *
    * FUNCTION:    OSAL_vTestErrorHook
    *
    * DESCRIPTION: This function shows the errorhook if it is called
    *
    *
    * PARAMETER:   tU32 u32ExpectedError:  expected error code
    *
    * RETURNVALUE: none
    *
    * HISTORY:
    * Date      |   Modification                         | Authors
    * 29.01.02  |   Initial revision                     |
    *****************************************************************************/
   void OSAL_vTestErrorHook(tU32 u32ErrorCode)
   {

      TRACE3_MSG1("  FAILED - ErrorMessage: %s \n\r", OSAL_coszErrorText(u32ErrorCode));
      if( pfUserErrorHook )
         pfUserErrorHook(u32ErrorCode);
   }

   /*****************************************************************************
    *
    * FUNCTION:    CountUnmatched
    *
    * DESCRIPTION: This function set the error Hook
    *
    * PARAMETER:   OSAL_tpfErrorHook pfErrorHook  pointer to error hook function
    *
    * RETURNVALUE: none
    *
    * HISTORY:
    * Date      |   Modification                         | Authors
    * 02.07.02  |   Initial revision                     |
    *****************************************************************************/
   tVoid OSAL_vTestSetErrorHook(const OSAL_tpfErrorHook pfErrorHook)
   {
      pfUserErrorHook=pfErrorHook;
   }


#endif


