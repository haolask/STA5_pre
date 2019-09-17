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

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_C
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_C

#include "osal.h"

#if defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING) 

#include "etalinternal.h"

#include "dabmw_import.h"

#include "Service_following_externalInterface_DABMWServices.h"


#include "Service_following_externalInterface_API.h"
#include "Service_following_externalInterface_radio.h"


#include "service_following.h"


#include "service_following_internal.h"

#include "service_following_seamless.h"

#include "service_following_log_status_info.h"


//
// Function to retrieve the country information
//

/* procedure to sent the auto notification on the API to the Host
*/
tVoid DABMW_ServiceFollowing_SendChangeNotificationsToHost (tVoid)
{

	EtalTuneServiceIdStatus vl_TuneServiceIdStatus;
	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
	tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
	tU32  vl_dabFrequency;
    tU32  vl_dabEid;
    tU32  vl_dabSid;
    tU32  vl_fmPi;
  	DABMW_SS_EstimationResutDatabaseTy* pl_StoredReport = NULL;

	// if SF is in INIT state, do not send update
	// it means we did not yet start the SF, no point to send a notification

	if ((false == DABMW_serviceFollowingData.enableServiceFollowing)
		|| (DABMW_SF_STATE_INIT == DABMW_serviceFollowingStatus.status))
	{
		return;
	}

	if ((false == DABMW_serviceFollowing_LogInfo.originalCellUpdate)
		&&
		(false == DABMW_serviceFollowing_LogInfo.alternateCellUpdate)
		&&
		(false == DABMW_serviceFollowing_LogInfo.seamlessUpdate)
		)
		{
		// nothing to report to host
		return;
		}

	

	vl_TuneServiceIdStatus.m_receiverHandle = DABMW_serviceFollowingStatus.originalHandle;
	vl_TuneServiceIdStatus.m_IsFoundstatus = (DABMW_serviceFollowingStatus.originalAudioUserApp != DABMW_NONE_APP)?true:false;
	vl_TuneServiceIdStatus.m_freq = DABMW_serviceFollowingStatus.originalFrequency;
	vl_TuneServiceIdStatus.m_Ueid = DABMW_serviceFollowingStatus.originalEid;
	vl_TuneServiceIdStatus.m_SidPi = DABMW_serviceFollowingStatus.originalSid;
	vl_TuneServiceIdStatus.m_freqIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
	
	if (DABMW_NONE_APP != DABMW_serviceFollowingStatus.alternateApp )
		{
		vl_TuneServiceIdStatus.m_AFisAvailable = true;
		}
	else
		{
		vl_TuneServiceIdStatus.m_AFisAvailable = false;
		}


	
	if (false == DABMW_serviceFollowingData.seamlessSwitchingMode)
		 {
		 vl_TuneServiceIdStatus.m_AFisSync = false;
		 // seamless not applicable
		 vl_TuneServiceIdStatus.m_ssApplicableOnAF = false;
		 }
	 else
		 {
		 if ((DABMW_serviceFollowingStatus.originalAudioUserApp == DABMW_NONE_APP)
		 || (DABMW_serviceFollowingStatus.alternateApp == DABMW_NONE_APP)
		 || (vl_originalIsDab == vl_alternateIsDab))	
			 {
			 // seamless not applicable
			 vl_TuneServiceIdStatus.m_ssApplicableOnAF = false;
			 vl_TuneServiceIdStatus.m_AFisSync = false;
			 }
		 else
			 {
			 // we have some information to provide from Database
	 		 vl_TuneServiceIdStatus.m_ssApplicableOnAF = true;
			 
			 if (true == vl_originalIsDab)
				 {
				 vl_dabFrequency = DABMW_serviceFollowingStatus.originalFrequency;
				 vl_dabEid = DABMW_serviceFollowingStatus.originalEid;
				 vl_dabSid = DABMW_serviceFollowingStatus.originalSid;
				 vl_fmPi = DABMW_serviceFollowingStatus.alternateSid;
				 }
			 else
				 {
				 vl_dabFrequency = DABMW_serviceFollowingStatus.alternateFrequency;
				 vl_dabEid = DABMW_serviceFollowingStatus.alternateEid;
				 vl_dabSid = DABMW_serviceFollowingStatus.alternateSid;
				 vl_fmPi = DABMW_serviceFollowingStatus.originalSid;
				 }
			 
				 /* Look if already present : if so just update */
	
				 pl_StoredReport = DABMW_ServiceFollowing_SSGetStoredInfoFromDatabase(vl_dabFrequency,
																		 vl_dabEid,
																		 vl_dabSid,
																		 vl_fmPi,
																		 DAMBW_SF_MEASURE_VALIDITY_INFINITE);
	
				 /* if found & result == SUCCESS 
				 */
				 if ((NULL != pl_StoredReport)
					 &&
					 (SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS == pl_StoredReport->SS_EstimationResult.status))
					 {
					 vl_TuneServiceIdStatus.m_AFisSync = true;
					 }
				 else
				 	{
				 	vl_TuneServiceIdStatus.m_AFisSync = false;
				 	}
		 	}
	 	}
				 
	
	ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO, &vl_TuneServiceIdStatus, sizeof(vl_TuneServiceIdStatus));

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "notification : ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO, handle = %d, freq = %d, SidPi = 0x%x, UEID = 0x%x, FoundStatus = %d, freqIsDab = %d,  AFisAvailable = %d, ssApplicable on AF = %d, AFisSync = %d ",
						vl_TuneServiceIdStatus.m_receiverHandle,
						vl_TuneServiceIdStatus.m_freq,						
						vl_TuneServiceIdStatus.m_SidPi,
						vl_TuneServiceIdStatus.m_Ueid,
						vl_TuneServiceIdStatus.m_IsFoundstatus,
						vl_TuneServiceIdStatus.m_freqIsDab,
						vl_TuneServiceIdStatus.m_AFisAvailable,
						vl_TuneServiceIdStatus.m_ssApplicableOnAF,
						vl_TuneServiceIdStatus.m_AFisSync);


	// reset flag : it has been notified
	DABMW_serviceFollowing_LogInfo.originalCellUpdate = false;
	DABMW_serviceFollowing_LogInfo.alternateCellUpdate = false;
	DABMW_serviceFollowing_LogInfo.seamlessUpdate = false;
			

	return;    
        
}

tSInt DABMW_ServiceFollowing_TuneServiceId(EtalPathName vI_path,  tU32 vI_SearchedServiceID)
{
#define	DABMW_SF_TUNE_INJECTION_SIDE_AUTO		0

	tSInt vl_res = OSAL_OK;
	tSInt vl_activate_SF_needed = false;

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_TuneServiceId : search requested : path = %d, Service = 0x%x\n",
				vI_path,
				vI_SearchedServiceID);

	if (false == DABWM_ServiceFollowing_ExtInt_IsEnable())
		{
		vl_activate_SF_needed = true;
		}

	vl_res = DABMW_ServiceFollowing_ServiceIDSelection(DABMW_ServiceFollowing_ExtInt_GetAppFromEtalPath(vI_path), vI_SearchedServiceID, false, DABMW_SF_TUNE_INJECTION_SIDE_AUTO, false);

	// now activate the SF task if needed.
	if (true == vl_activate_SF_needed)
	{
		ETALTML_ServiceFollowing_TaskWakeUpOnEvent(DABMW_SF_EVENT_WAKE_UP);
	}
	
	return(vl_res);
}


tSInt DABMW_ServiceFollowing_ExtInt_ActivateSF()
{

	tSInt vl_res = OSAL_OK;
	tSInt vl_activate_SF_needed = false;

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ActivateSF \n");

	if (false == DABWM_ServiceFollowing_ExtInt_IsEnable())
		{
		vl_activate_SF_needed = true;
		}

	DABWM_ServiceFollowing_Enable();

		// now activate the SF task if needed.
	if (true == vl_activate_SF_needed)
	{
		ETALTML_ServiceFollowing_TaskWakeUpOnEvent(DABMW_SF_EVENT_WAKE_UP);
	}

	return(vl_res);
}

tSInt DABMW_ServiceFollowing_ExtInt_DisableSF()
{

	tSInt vl_res = OSAL_OK;

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_DisableSF \n");


	DABWM_ServiceFollowing_Disable();

	return(vl_res);
}

// Procedure to enable Service Following
tBool DABWM_ServiceFollowing_ExtInt_IsEnable()
{

    return (DABWM_ServiceFollowing_IsEnable());
        
}

// Procedure to control the Kind of Switch for SF

// Procedure to enable Service Following
tSInt DABWM_ServiceFollowing_ExtInt_SelectKindOfSwitch(tBool vI_fmfm, tBool vI_dabfm, tBool vI_dabdab)
{
	tU16 al_CellParam[1];
	tS32 al_CellValue[1];
	tSInt vl_res = OSAL_OK;
	tSInt vl_numParam = 1;

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABWM_ServiceFollowing_ExtInt_SelectKindOfSwitch fmfm:%d, dabfm:%d, dabdab:%d\n",
		vI_fmfm, vI_dabfm, vI_dabdab);
	
	// set the param type
	al_CellParam[0] = DABMW_API_SERVICE_FOLLOWING_PARAM_S + DABMW_API_SF_PARAM_KIND_OF_SWITCH;

	//init  the value
	al_CellValue[0] = 0;

	if (TRUE == vI_fmfm)
	{
		al_CellValue[0] |= DABMW_SF_MASK_FM_FM;
	}

	if (TRUE == vI_dabfm)
	{
		al_CellValue[0] |= DABMW_SF_MASK_DAB_FM;
	}
	
	if (TRUE == vI_dabdab)
	{
		al_CellValue[0] |= DABMW_SF_MASK_DAB_FM;
	}
	

	// DABWM_SetServiceFollowingParameters : return the number of parameter updated.
	// 
	if (vl_numParam != DABWM_SetServiceFollowingParameters(DABMW_ALL_APPLICATIONS, al_CellParam, al_CellValue, vl_numParam))
	{
		 vl_res = OSAL_ERROR;
	}


	return vl_res;
	
}

#endif // HAVE_SF_DEMO

#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_C

