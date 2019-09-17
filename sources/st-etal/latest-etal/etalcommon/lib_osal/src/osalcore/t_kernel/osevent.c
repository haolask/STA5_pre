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
#include <tk/tkernel.h>


#include "osal.h"
// #include <sys/timeb.h>
//#include <timeb.h>
#include <typedef.h>
#include <tk/syscall.h>




/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/

/* Event Table Entry */
typedef struct trEventElement
{
  ID tk_EventFlgId;
} trEventElement;


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
       tS32 s32ReturnValue=OSAL_ERROR;
       tU32 u32ErrorCode=OSAL_E_NOERROR;
	   T_CFLG flg;
	   ER ercd = 0;

       /* The name is not NULL */

       (void) coszName; /* make Lint happy */
       if (phEvent)
       {
			trEventElement *pCurrentEntry;

			/* an Event Element is based on 
			
			VP exinf Extended information
			ATR flgatr Event flag attributes
			UINT iflgptn Initial event flag pattern
			UB dsname[8] DS object name

			*/
	
			/* The name is not already in use*/
			pCurrentEntry = (trEventElement *)malloc(sizeof(trEventElement));

			if (pCurrentEntry!= OSAL_NULL)
			{


			flg.exinf = (VP) 0x00000000;
			flg.flgatr = TA_TFIFO | TA_FIRST;
			flg.iflgptn = 0x0000;

			pCurrentEntry->tk_EventFlgId = (OSAL_tEventHandle) tk_cre_flg (&flg);
			
			*phEvent = (OSAL_tEventHandle)pCurrentEntry;

			ercd = tk_clr_flg ((ID)pCurrentEntry->tk_EventFlgId, 0x0000);
			
			if (ercd < 0)
			{
				s32ReturnValue=OSAL_ERROR;
			}

#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_USER_1)
          (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"\n ------------------------- \n\n\n");
		  (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"OSAL_s32EventCreate: hEvent = %d \n", pCurrentEntry->tk_EventFlgId);
		  (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"\n ------------------------- \n\n\n");
#endif
 
			s32ReturnValue = OSAL_OK;
		}
		else
		{
			  u32ErrorCode = OSAL_E_NOSPACE;
		}

       }
	   else 
	   {
	   		u32ErrorCode = OSAL_E_INVALIDVALUE;
	   }


	if (OSAL_E_NOERROR != u32ErrorCode)
	{
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
		 (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32EventCreate:  ERROR %d\n",  u32ErrorCode);
#endif
	}


       return( s32ReturnValue );
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
       tS32 s32ReturnValue=OSAL_ERROR;
       tU32 u32ErrorCode=OSAL_E_NOERROR;
       trEventElement *pCurrentEntry;

		if(hEvent)
		{

		   pCurrentEntry = ((trEventElement *)hEvent);


          (void)tk_del_flg(pCurrentEntry->tk_EventFlgId);
          free(pCurrentEntry);
          s32ReturnValue = OSAL_OK;


		} 
		else
		{
			u32ErrorCode = OSAL_E_INVALIDVALUE;
		}
		
			if (u32ErrorCode != OSAL_E_NOERROR)
			{
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
			  (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32EventClose failed!!! errorcode=%d", u32ErrorCode);
#endif
			  //os21_vSetErrorCode(OSAL_C_THREAD_ID_SELF, u32ErrorCode);
			}
			
	return( s32ReturnValue );
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
       tU32 u32ErrorCode=OSAL_E_NOERROR;
       tS32 s32ReturnValue=OSAL_ERROR;
       trEventElement *pCurrentEntry;
       OSAL_tEventMask RetFlags = 0;
   

	   ER ercd = 0;

	   UINT vl_tkEnFlags = 0;

	 if(hEvent && (enFlags == OSAL_EN_EVENTMASK_OR || enFlags == OSAL_EN_EVENTMASK_AND))
	 {
	
		   pCurrentEntry = ((trEventElement *)hEvent);

#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_USER_1)
          (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE," \n\n\n-------------- OSAL_s32EventWait ---------\n ");
		  (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE," OSAL_s32EventWait  hEvent = %d, wait mask 0x%x, flag %d, time %d\n ", 
		   		pCurrentEntry->tk_EventFlgId,
		   		mask,
		   		enFlags,
		   		msec);
#endif


		  switch (enFlags)
			  {
					case OSAL_EN_EVENTMASK_AND:
						 vl_tkEnFlags = TWF_ANDW | TWF_BITCLR;
					break;
					case OSAL_EN_EVENTMASK_OR:
						vl_tkEnFlags = TWF_ORW | TWF_BITCLR;
					break;
					default:
					break;
			  }


                  //os21_tThreadChangePendedFlag();
                  switch ((tU32)msec)
                  {
                     case OSAL_C_TIMEOUT_FOREVER:
                        // status = event_wait_all((event_group_t *)pCurrentEntry->os21EventGroup,mask,&RetFlags,TIMEOUT_INFINITY);
						 ercd = tk_wai_flg (pCurrentEntry->tk_EventFlgId, mask, vl_tkEnFlags, &RetFlags, TMO_FEVR);
                        break;
                     case OSAL_C_TIMEOUT_NOBLOCKING:
                         //status = event_wait_all((event_group_t *)pCurrentEntry->os21EventGroup,mask,&RetFlags,TIMEOUT_IMMEDIATE);
                         ercd = tk_wai_flg (pCurrentEntry->tk_EventFlgId, mask, vl_tkEnFlags, &RetFlags, TMO_POL);
                        break;
                     default:
                         //status = event_wait_all((event_group_t *)pCurrentEntry->os21EventGroup,mask,&options,&os_msec);
                         ercd = tk_wai_flg (pCurrentEntry->tk_EventFlgId, mask, vl_tkEnFlags, &RetFlags, msec);
                        break;
                  }
                  //os21_tThreadChangePendedFlag();

 #if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_USER_1)
          (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"OSAL_s32EventWait end: hEvent = %d , received_envent mask = 0x%x\n", 
          			pCurrentEntry->tk_EventFlgId, RetFlags);
		  (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"\n --------- OSAL_s32EventWait ------------- \n\n\n");
#endif   
  	   				   

             if(pResultMask != NULL)
             {
                *pResultMask = RetFlags;
			 				}
			 				
             switch (ercd)
             {
                case E_OK:
                   s32ReturnValue = OSAL_OK;
                 //  os21_tThreadChangePendedFlag();
                   break;

                case E_TMOUT:
                   u32ErrorCode = OSAL_E_TIMEOUT;
				   s32ReturnValue = OSAL_ERROR_TIMEOUT_EXPIRED;
                   break;

                default:
                   u32ErrorCode = OSAL_E_UNKNOWN;
                   break;
             }
	 	}
	 else
	 {
	 		u32ErrorCode = OSAL_E_INVALIDVALUE;
	 }


 if ((u32ErrorCode != OSAL_E_NOERROR)&&(u32ErrorCode!=OSAL_E_TIMEOUT))
       {
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
          (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32EventWait failed!!! errorcode=%d", u32ErrorCode);
#endif
         // os21_vSetErrorCode(OSAL_C_THREAD_ID_SELF, u32ErrorCode);
       }

       return( s32ReturnValue );
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
       tS32 s32ReturnValue=OSAL_ERROR;
       tU32 u32ErrorCode=OSAL_E_NOERROR;
	   tU32 Present_Mask = 0;
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_USER_1)
       tU32 Final_Mask = 0;
#endif
       trEventElement *pCurrentEntry;
	   ER ercd = 0;
	   T_RFLG vl_currentTkFlg;
	   tU32 vl_PostedMask = mask;

	  
	 if(hEvent)
	 {

		 pCurrentEntry = ((trEventElement *)hEvent);

 #if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_USER_1)
				  (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE," \n\n\n-------------- OSAL_s32EventPost ---------\n ");
				  (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"OSAL_s32EventPost request hEvent = %d, wait mask 0x%x, flag %d\n ", 
					pCurrentEntry->tk_EventFlgId,
					mask,
					enFlags);
#endif


	
	   	// get the current event flag status
     	ercd = tk_ref_flg (pCurrentEntry->tk_EventFlgId, &vl_currentTkFlg);

		Present_Mask = vl_currentTkFlg.flgptn;
		
         switch (enFlags)
              {
                 case OSAL_EN_EVENTMASK_AND:
				 	vl_PostedMask = mask & Present_Mask;
					(void)tk_clr_flg(pCurrentEntry->tk_EventFlgId, 0);
					(void)tk_set_flg(pCurrentEntry->tk_EventFlgId, vl_PostedMask);
				 	
                    break;
                 case OSAL_EN_EVENTMASK_OR:
                     // this is the way default is used in tk
                     // mask = mask | Present_Mask;
                     // event_post((event_group_t *)pCurrentEntry->os21EventGroup,mask);
                     // just post
                     vl_PostedMask = mask;
                     (void)tk_set_flg(pCurrentEntry->tk_EventFlgId, vl_PostedMask);
                    break;
                 case OSAL_EN_EVENTMASK_REPLACE:
				 	// let's do a clear and post
				 	// event_clear((event_group_t *)pCurrentEntry->os21EventGroup,0xFFFFFFFF);
                    // event_post((event_group_t *)pCurrentEntry->os21EventGroup,mask);
					vl_PostedMask = mask;
				 	(void)tk_clr_flg(pCurrentEntry->tk_EventFlgId, 0);
					(void)tk_set_flg(pCurrentEntry->tk_EventFlgId, vl_PostedMask);

                    break;
                 case OSAL_EN_EVENTMASK_XOR:
                    vl_PostedMask = mask ^ Present_Mask;
                    // event_clear((event_group_t *)pCurrentEntry->os21EventGroup,0xFFFFFFFF);
                    // event_post((event_group_t *)pCurrentEntry->os21EventGroup,mask);
					
				 	(void)tk_clr_flg(pCurrentEntry->tk_EventFlgId, 0);
					(void)tk_set_flg(pCurrentEntry->tk_EventFlgId, mask);
                    break;
                 default:
                    u32ErrorCode = OSAL_E_NOTSUPPORTED;
                    break;
              }

             s32ReturnValue = OSAL_OK;

		 // get the current event flag status
		 ercd = tk_ref_flg (pCurrentEntry->tk_EventFlgId, &vl_currentTkFlg);
		 
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_USER_1)
		 Final_Mask = vl_currentTkFlg.flgptn;
#endif

 #if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_USER_1)
						   (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"\n ------------------------- \n\n\n");
						   (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"OSAL_s32EventPost: hEvent = %d , requested event  = 0x%x, event_mask = %d, present_mask = 0x%x, posted = 0x%x, current = 0x%x\n", 
				 			pCurrentEntry->tk_EventFlgId, 
				 			mask, 
				 			enFlags,
				 			Present_Mask,
				 			vl_PostedMask,
				 			Final_Mask);
						    (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"\n ------------------------- \n\n\n");
#endif



  }
  else
	{
	 u32ErrorCode = OSAL_E_INVALIDVALUE;
	}

	if (ercd != E_OK)
	{
		// something went wrong
		u32ErrorCode = OSAL_E_UNKNOWN;
	}

       if (u32ErrorCode != OSAL_E_NOERROR)
       {
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
          (void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32EventPost failed!!! errorcode=%d", u32ErrorCode);
#endif
       //   os21_vSetErrorCode(OSAL_C_THREAD_ID_SELF, u32ErrorCode);
       }

       return( s32ReturnValue );
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
	
		tS32 s32ReturnValue=OSAL_ERROR;
	
		trEventElement *pCurrentEntry;
		
		if(hEvent)
		{
	
			pCurrentEntry = ((trEventElement *)hEvent);
			
			// clear the flag : 
			// it does a 'AND mask' clearing the bits set to 0
			// so mask is to be ~
			
			(void)tk_clr_flg(pCurrentEntry->tk_EventFlgId, ~mask);


			 
 #if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_USER_1)
		(void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"\n ------------------------- \n\n\n");
		(void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"OSAL_s32EventClear: hEvent = %d , requested event	= 0x%x\n", 
						pCurrentEntry->tk_EventFlgId, 
						mask);
		(void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_OSALCORE,"\n ------------------------- \n\n\n");
#endif


	
			 
			s32ReturnValue = OSAL_OK;
	
		}
		
	  else 
	  {
	  	s32ReturnValue = OSAL_E_INVALIDVALUE;
	  }
	
	
		return( s32ReturnValue );
	
}


#ifdef __cplusplus
}
#endif



#endif  /* OSEVENT_C */

/** @} */

/* End of File */
