//!
//!  \file      service_following_background.c
//!  \brief     <i><b> Service following implementation : external interface definition => OSAL </b></i>
//!  \details   This file provides functionalities for service following background check, scan and AF check
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!



#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_OSAL_C
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_OSAL_C


#include "osal.h"
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"



#include "Service_following_externalInterface_OSAL.h"
#include "service_following_task.h"


/**
 * @brief     DABMW_ServiceFollowing_ExtInt_MemoryAllocate
 *
 * @details   This function reserve on the Heap-memory a block with
 *            the specific size
 *
 * @param     u32size Size of the memory block (I)
 *
 * @return
 *        - Pointer to start memory block
 *        - NULL in case of error
 *
 */
tPVoid DABMW_ServiceFollowing_ExtInt_MemoryAllocate(tU32 u32size)
{

   return((tPVoid) OSAL_pvMemoryAllocate(u32size));
}

/**
 * @brief     DABMW_ServiceFollowing_ExtInt_MemoryFree
 *
 * @details   This function releases on the Heap-memory a block with
 *            the specific size
 *
 * @param     pBlock Pointer to start memory block (I)
 *
 * @return    NULL
 *
 */
tVoid DABMW_ServiceFollowing_ExtInt_MemoryFree(tPVoid pBlock)
{
   OSAL_vMemoryFree(pBlock);
   return;
}



/**
 * @brief     DABMW_ServiceFollowing_ExtInt_MemorySet
 *
 * @details   This function initializes a memory with a specific character.
 *
 * @param     pvSource   Starting to the target memory (I)
 * @param     u32Char    character (I)
 * @param     u32Bytes   Number of bytes (I)
 *
 * @return    Starting of the target memory
 *
 */
tPVoid DABMW_ServiceFollowing_ExtInt_MemorySet(tPVoid pvSource, tU32 u32Char, tU32 u32Bytes)
{
   
   return((tPVoid) OSAL_pvMemorySet(pvSource,u32Char,u32Bytes));
}

/**
 * @brief     DABMW_ServiceFollowing_ExtInt_MemoryCopy
 *
 * @details   This function copies a memory area with the specified size.
 *
 *
 * @param     pvDest     Start to the target memory (I)
 * @param     pvSource   Start to the source memory (I)
 * @param     u32size    Size of the memory block (I)
 *
 * @return    Starting of the target memory
 *
 */
tPVoid DABMW_ServiceFollowing_ExtInt_MemoryCopy( tPVoid pvDest, tPCVoid pvSource, tU32 u32size )
{
   return((tPVoid ) OSAL_pvMemoryCopy(pvDest,pvSource,u32size));
}

/**
 *
 * @brief    OSAL_ClockGetElapsedTime;
 *
 * @details  This Function returns the elapsed time since the start of the
 *              system through OSAL_Boot() in milliseconds.
 *
 * @return   Time in milliseconds
 *
 */
SF_tMSecond DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime(void)
{
  return ((SF_tMSecond) OSAL_ClockGetElapsedTime());
}

tVoid DABMW_ServiceFollowing_ExtInt_TaskClearEvent (tU32 event)
{

	 // Clear old event if any (this can happen after a stop)
	 ETALTML_ServiceFollowing_TaskClearEvent(event);
	return;
}

tVoid DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(tU32 event)
{

	 // Clear old event if any (this can happen after a stop)
	ETALTML_ServiceFollowing_TaskWakeUpOnEvent(event);
	return;
}



#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_OSAL_C

#endif // #ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

