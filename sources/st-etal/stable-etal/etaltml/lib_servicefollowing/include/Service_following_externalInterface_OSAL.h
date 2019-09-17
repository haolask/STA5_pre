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



#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_OSAL_H
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_OSAL_H


/*
*********************
* DEFINE SECTION
**********************
*/



/*
*********************
* STRUCTURE SECTION
**********************
*/
	
//!
//! \typedef The type of the SF  time variable	
typedef OSAL_tMSecond SF_tMSecond;


/*
*********************
* VARIABLE SECTION
**********************
*/



/*
*********************
* FUNCTIONS SECTION
**********************
*/

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_OSAL_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif


//
// @brief     DABMW_ServiceFollowing_ExtInt_MemoryAllocate
//
GLOBAL tPVoid DABMW_ServiceFollowing_ExtInt_MemoryAllocate(tU32 u32size);

//
// * @brief     DABMW_ServiceFollowing_ExtInt_MemoryFree
//
GLOBAL tVoid DABMW_ServiceFollowing_ExtInt_MemoryFree(tPVoid pBlock);

//
// @brief     DABMW_ServiceFollowing_ExtInt_MemorySet
//
GLOBAL tPVoid DABMW_ServiceFollowing_ExtInt_MemorySet(tPVoid pvSource, tU32 u32Char, tU32 u32Bytes);

//
// @brief     DABMW_ServiceFollowing_ExtInt_MemoryCopy
//
GLOBAL tPVoid DABMW_ServiceFollowing_ExtInt_MemoryCopy( tPVoid pvDest, tPCVoid pvSource, tU32 u32size );

//
// * @brief    OSAL_ClockGetElapsedTime;
 
GLOBAL SF_tMSecond DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime(void);

GLOBAL tVoid DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent (tU32 vI_event);
	
GLOBAL tVoid DABMW_ServiceFollowing_ExtInt_TaskClearEvent (tU32 vI_event);

#undef GLOBAL

#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_OSAL_H

