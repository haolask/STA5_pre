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



#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_H
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_H


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

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

GLOBAL tVoid DABMW_ServiceFollowing_SendChangeNotificationsToHost (tVoid);
GLOBAL tSInt DABMW_ServiceFollowing_TuneServiceId(EtalPathName vI_path,  tU32 vI_SearchedServiceID);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_ActivateSF(tVoid);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_DisableSF(tVoid);
GLOBAL tBool DABWM_ServiceFollowing_ExtInt_IsEnable(tVoid);
GLOBAL tSInt DABWM_ServiceFollowing_ExtInt_SelectKindOfSwitch(tBool vI_fmfm, tBool vI_dabfm, tBool vI_dabdab);
#undef GLOBAL


/*
*********************
* FUNCTIONS IMPORT SECTION
**********************
*/

#ifdef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_C
// from #include "Api_cmdlist.h"
#define DABMW_NOTIFICATION_IS_SF_STATUS      	    (tU8)0x07 // Notification for service following

// import from external
extern tBool DABMW_GetServiceFollowingAutoNotifcationActivationStatus (tVoid); 
extern tSInt DABMW_ApiAutoNotificationTxManager (tPU8 dataBuffer, tU16 dataLen, tU8 notificationNum);
extern tU8 DABMW_ServiceFollowing_BuildPayloadNotifications (tPU8 pO_payloadBuffer, tU16 vI_bufferLen);

#endif




#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_H

