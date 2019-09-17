//!
//!  \file 		osevent.c
//!  \brief 	<i><b>OSAL Event Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) Event Functions.
//!  \author 	Luca Pesenti
//!  \author 	(original version) Luca Pesenti
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

/* DeactivateLintMessage_ID0026 */
#ifndef OSEVENT_C
#define OSEVENT_C

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "event_groups.h"


#include "osal.h"
// #include <sys/timeb.h>
//#include <timeb.h>
//#include <typedef.h>




/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/


/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
| variable definition (scope: global)
|-----------------------------------------------------------------------*/



/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/


/**
* @brief     OSAL_s32EventCreate
*
* @details   This function creates an OSAL event using the OS21 ones.
*
* @param     coszName event name to create (I)
* @param     phEvent pointer to the event handle (->O)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
tS32 OSAL_s32EventCreate(tCString coszName, OSAL_tEventHandle* phEvent)
{
 
       return( OSAL_ERROR );
}



/**
* @brief     OSAL_s32EventFree
*
* @details   This function closes an OSAL event.
*
* @param     hEvent event handle (I)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
tS32 OSAL_s32EventFree(OSAL_tEventHandle hEvent)
{
        return( OSAL_ERROR );
}

/**
* @brief     OSAL_s32EventWait
*
* @details   This function waits for an OSAL event to occur,
*            where an event occurrence means the link operation
*            (given by enFlags) between the EventField present
*            in the EventGroup structure and the provided EventMask
*            is catched. Allowed link operation are AND/OR
*            So the event resets the calling thread only if within
*            the requested timeout one of the following conditions
*            is verified:
*               EventMask || EventField is TRUE or
*               EventMask && EventField is true
*            depending on the requested link operation.
*
* @param     hEvent event handle (I)
* @param     mask event mask (I)
* @param     enFlags event flag (I)
* @param     msec waiting time (I)
* @param     pResultMask pointer to the previous event mask (->O)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
tS32 OSAL_s32EventWait(OSAL_tEventHandle      hEvent,
                       OSAL_tEventMask        mask,
                       OSAL_tenEventMaskFlag  enFlags,
                       OSAL_tMSecond          msec,
                       OSAL_tEventMask       *pResultMask)
{
        return( OSAL_ERROR );
}
/**
* @brief     OSAL_s32EventPost()
*
* @details   This function Posts an OSAL event using the OS21 ones.
*
* @param     hEvent event handle (I)
* @param     mask event mask (I)
* @param     enFlags event flag (I)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
tS32 OSAL_s32EventPost(OSAL_tEventHandle hEvent,
                       OSAL_tEventMask mask,
                       OSAL_tenEventMaskFlag enFlags)
{
       return( OSAL_ERROR );
}

/**
* @brief     OSAL_s32EventStatus
*
* @details   This function creates an OSAL event using the OS21 ones.
*
* @param     hEvent event handle (I)
* @param     mask event mask (I)
* @param     pMask Pointer to the Return status value (I)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
/* Not used
*/
#if 0
tS32 OSAL_s32EventStatus(OSAL_tEventHandle hEvent,
                         OSAL_tEventMask mask,
                         OSAL_tEventMask* pMask)
{
       tS32 s32ReturnValue=OSAL_ERROR;
       tU32 u32ErrorCode=OSAL_E_NOERROR;
       trEventElement *pCurrentEntry;
       OSAL_tEventMask Present_Mask;
	   ER ercd = 0;
	   UINT p_flg_tmp ;
	   T_RFLG vl_currentTkFlg;

	if(hEvent)
	{
//		PROTECT(event_table_lock);


	   	pCurrentEntry = ((trEventElement *)hEvent);
    
	    // get the current event flag status
		ercd = tk_ref_flg (pCurrentEntry->tk_EventFlgId, &vl_currentTkFlg);
		
		Present_Mask = vl_currentTkFlg.flgptn;

		*pMask = (mask & Present_Mask);

		s32ReturnValue = OSAL_OK;


//	  UNPROTECT(event_table_lock);

  }
  else u32ErrorCode = OSAL_E_INVALIDVALUE;

       if (u32ErrorCode != OSAL_E_NOERROR)
       {
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
          (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32EventStatus failed!!! errorcode=%d", u32ErrorCode);
#endif
         // os21_vSetErrorCode(OSAL_C_THREAD_ID_SELF, u32ErrorCode);
       }

       return( s32ReturnValue );
}
#endif

tS32 OSAL_s32EventClear(OSAL_tEventHandle hEvent,
							   OSAL_tEventMask mask)
{
	
       return( OSAL_ERROR );
	
}


#ifdef __cplusplus
}
#endif



#endif  /* OSEVENT_C */

/** @} */

/* End of File */
