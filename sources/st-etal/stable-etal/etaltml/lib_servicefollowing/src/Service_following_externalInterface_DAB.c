//!
//!  \file      service_following_external_DAB.c
//!  \brief     <i><b> Service following implementation : external interface definition => dab part </b></i>
//!  \details   This file provides functionalities for external interface : dab part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!


#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DAB_C
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DAB_C

#include "osal.h"
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"

#include "Service_following_externalInterface_OSAL.h"
#include "Service_following_externalInterface_DABMWServices.h"

#include "service_following_internal.h"

#include "Service_following_externalInterface_DAB.h"

#include "service_following_meas_and_evaluate.h"


tU32 DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble (DABMW_SF_mwAppTy vI_app)
{
	// return (DABMW_GetEnsembleIdAndEcc(DABMW_GetCurrentEnsemble(vI_app)));
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	tU32 vl_currentUeId = DABMW_INVALID_ENSEMBLE_ID;

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{	
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			if (ETAL_cmdGetCurrentEnsemble_MDR(vl_etalHandle, &vl_currentUeId) != OSAL_OK)
			{
				 vl_currentUeId = DABMW_INVALID_ENSEMBLE_ID;
			}
		}
		else
		{
			vl_currentUeId = DABMW_INVALID_ENSEMBLE_ID;
		}
		
		ETAL_receiverReleaseLock(vl_etalHandle);
	}
	else
	{
		vl_currentUeId = DABMW_INVALID_ENSEMBLE_ID;
	}
	//	Lock addition change end
	

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble : app = %d, ensemble ID 0x%x\n",
					vI_app, 
					vl_currentUeId);
			
	return(vl_currentUeId);
}

tU32 DABMW_ServiceFollowing_ExtInt_GetApplicationEnsembleId(DABMW_SF_mwAppTy vI_app)
{

	// return (DABMW_GetEnsembleIdAndEcc(DABMW_GetApplicationEnsembleId(vI_app)));
	return(DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble(vI_app));
}



tSInt DABMW_ServiceFollowing_ExtInt_GetEnsembleList (DABMW_ensembleUniqueIdTy **pO_dataPtr)
{

	EtalEnsembleList vl_EnsembleList;
	static	DABMW_ensembleUniqueIdTy vl_ensembleUniqueId[DABMW_FIC_MAX_ENSEMBLE_NUMBER];
	tSInt vl_numEnsemble = 0;
	tU8 vl_cnt;
	tBool vl_haveData;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

 	// Addition of  Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	// We do not have receiver, so can't lock ... but may lead to error ?
	//

	if (ETAL_cmdGetEnsembleList_MDR(&vl_EnsembleList, &vl_haveData) != OSAL_OK)
	{
		ret = ETAL_RET_ERROR;
	}
	else
	{
		if (!vl_haveData)
		{
			ret = ETAL_RET_NO_DATA;
		}
	}

	if (ETAL_RET_SUCCESS == ret)
	{
		vl_numEnsemble = vl_EnsembleList.m_ensembleCount;
		
		for (vl_cnt=0;vl_cnt<vl_numEnsemble; vl_cnt++)
		{
			vl_ensembleUniqueId[vl_cnt].ecc = vl_EnsembleList.m_ensemble[vl_cnt].m_ECC;
			vl_ensembleUniqueId[vl_cnt].frequency = vl_EnsembleList.m_ensemble[vl_cnt].m_frequency;
			vl_ensembleUniqueId[vl_cnt].id = vl_EnsembleList.m_ensemble[vl_cnt].m_ensembleId;
			OSAL_pvMemorySet(vl_ensembleUniqueId[vl_cnt].alternativeFrequencies, DABMW_INVALID_FREQUENCY, sizeof(vl_ensembleUniqueId[vl_cnt].alternativeFrequencies));
		}

		*pO_dataPtr = &vl_ensembleUniqueId[0];
	}
	else
	{
		vl_numEnsemble = 0;
	}



	DABMW_SF_PRINTF(TR_LEVEL_USER_4, "--------------- DABMW_ServiceFollowing_ExtInt_GetEnsembleList-------------\n");


	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_GetEnsembleList : Nb ensembles %d\n",
					vl_numEnsemble);

	for (vl_cnt=0;vl_cnt<vl_numEnsemble; vl_cnt++)
	{
		DABMW_SF_PRINTF(TR_LEVEL_USER_4, "DABMW_ServiceFollowing_ExtInt_GetEnsembleList : ecc 0x%x, id 0x%x, frequency %d\n",
					vl_ensembleUniqueId[vl_cnt].ecc, vl_ensembleUniqueId[vl_cnt].id, vl_ensembleUniqueId[vl_cnt].frequency);
	}

	DABMW_SF_PRINTF(TR_LEVEL_USER_4, "--------------- DABMW_ServiceFollowing_ExtInt_GetEnsembleList-------------\n");

    return vl_numEnsemble;
}

	
	
// Process service functionality
tSInt DABMW_ServiceFollowing_ExtInt_GetServiceList(tU32 vI_ensembleId, tVoid** pO_dataPtr)
{
	
		static EtalServiceList vl_ServiceList;
		tBool vl_haveData = FALSE;
		tSInt vl_numService = 0;
		tU8 vl_cnt;

 	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	// We do not have receiver, so can't lock ... but may lead to error ?
	//
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		if (ETAL_cmdGetServiceList_MDR(vI_ensembleId, true, false, &vl_ServiceList, &vl_haveData) != OSAL_OK)
		{
			vl_numService = 0;
		}
		else 
		{
			vl_numService = vl_ServiceList.m_serviceCount;
	
			*pO_dataPtr = &vl_ServiceList.m_service[0];
		}
#else
		vl_numService = 0;
#endif


		DABMW_SF_PRINTF(TR_LEVEL_USER_4, "--------------- DABMW_ServiceFollowing_ExtInt_GetServiceList-------------\n");
		
		
		DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_GetServiceList : Ensemble ID 0x%d, Nb SID %d\n",
						vI_ensembleId, vl_numService);
		
		for (vl_cnt=0;vl_cnt<vl_numService; vl_cnt++)
		{
			DABMW_SF_PRINTF(TR_LEVEL_USER_4, "DABMW_ServiceFollowing_ExtInt_GetServiceList : Sid 0x%x\n",
						vl_ServiceList.m_service[vl_cnt]);
		}
		
		DABMW_SF_PRINTF(TR_LEVEL_USER_4, "--------------- DABMW_ServiceFollowing_ExtInt_GetServiceList-------------\n");


		return(vl_numService);
}
	

// Process service functionality
tSInt DABMW_ServiceFollowing_ExtInt_ServiceSelect(DABMW_SF_mwAppTy vI_app, tU8 vI_selectionType, tU32 vI_ensembleId, tU32 vI_Sid)
{

	tSInt vl_res;
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	tBool	vl_noDataRes;


	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_GetServiceList : app %d, selection type %d, EID 0x%x, SID 0x%x\n",
						vI_app, vI_selectionType, vI_ensembleId, vI_Sid);
		
	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{
			
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			vl_res = ETAL_cmdServiceSelect_MDR_internal(vl_etalHandle,						//receiver
												ETAL_SERVSEL_MODE_SERVICE, 					// mode is SID
												(EtalServiceSelectSubFunction)vI_selectionType,							// selection type : this is matching between ETAL enum and here
												vI_ensembleId, 								// Ensemble
												vI_Sid, 									// Sid
												DABMW_INVALID_DATA, 						// not used for SID selection type
												DABMW_INVALID_DATA,							// not used for SID selection typ
												&vl_noDataRes, 								// get the data info
												ETAL_DATA_TYPE_AUDIO);						// DATA type = audio

			// Update ETAL infos
			ETAL_statusSetDABService(vI_ensembleId, vI_Sid);
		}
		else
		{
			vl_res = OSAL_ERROR;
		}
		
		ETAL_receiverReleaseLock(vl_etalHandle);
	}
	else
	{
			vl_res = OSAL_ERROR;
	}
	//	Lock addition change end

	
	return(vl_res);
}
                          

tSInt DABMW_ServiceFollowing_ExtInt_GetServiceTypeFromServiceId(tU32 vI_ensembleId, tU32 vI_ServiceId, DABMW_componentEnumTy* pO_componentType, DABMW_componentTypeTy* pO_streamType)
{  
	// return(DABMW_GetServiceTypeFromServiceId(vI_ensembleId, vI_ServiceId, pO_componentType, pO_streamType));
	return(OSAL_ERROR);
}

// Get all Dab Quality Info
DABMW_SF_dabQualityTy DABMW_ServiceFollowing_ExtInt_GetDabQuality(DABMW_SF_mwAppTy vI_app)
{
	DABMW_SF_dabQualityTy vl_dabQuality = DABMW_ServiceFollowing_DabQualityInit();
	EtalBcastQualityContainer vl_etalDabQuality;
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);

		
	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{		
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			// Get Fic Ber
			ETAL_get_reception_quality_internal(vl_etalHandle, &vl_etalDabQuality);

			vl_dabQuality.fic_ber = vl_etalDabQuality.EtalQualityEntries.dab.m_FicBitErrorRatio;

			vl_dabQuality.sync_status = vl_etalDabQuality.EtalQualityEntries.dab.m_syncStatus;

			// Get Audio Info
			ETAL_cmdGetAudioQuality_MDR(vl_etalHandle, &vl_etalDabQuality);

		    // get the audio ber
		    vl_dabQuality.audio_ber = vl_etalDabQuality.EtalQualityEntries.dab.m_audioSubChBitErrorRatio;

		    // get reed solomon
		     vl_dabQuality.reed_solomon_information = vl_etalDabQuality.EtalQualityEntries.dab.m_reedSolomonInformation;  

		    // get audio_ber_level
		    vl_dabQuality.audio_ber_level = vl_etalDabQuality.EtalQualityEntries.dab.m_audioBitErrorRatioLevel; 


		    // get mute flag
		    vl_dabQuality.mute_flag = vl_etalDabQuality.EtalQualityEntries.dab.m_muteFlag;


		}
		else
		{
			// nothing
		}
		
		ETAL_receiverReleaseLock(vl_etalHandle);

	}
	else
	{
		// nothing
	}
	//	Lock addition change end

	
	return(vl_dabQuality);
}

// NOTE : not used anymore
tU32 DABMW_ServiceFollowing_ExtInt_GetDabQualityFicBer(DABMW_SF_mwAppTy vI_app)
{

	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	EtalBcastQualityContainer vl_etalDabQuality;

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			ETAL_get_reception_quality_internal(vl_etalHandle, &vl_etalDabQuality);
		}
		else
		{
			//nothing
		}
		
		ETAL_receiverReleaseLock(vl_etalHandle);
	}
	else
	{
		// nothing
	}
	//	Lock addition change end

    return(vl_etalDabQuality.EtalQualityEntries.dab.m_FicBitErrorRatio);
	
}


// NOTE : not used anymore
tU32 DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioBer(DABMW_SF_mwAppTy vI_app)
{
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);

	ETAL_cmdGetAudioQuality_MDR(vl_etalHandle, &v_etalDabQuality);
		
	// return(DABMW_GetDabQualityAudioBer(vI_app));
	return(v_etalDabQuality.EtalQualityEntries.dab.m_audioSubChBitErrorRatio);
}

tU8 DABMW_ServiceFollowing_ExtInt_GetDabQualityReedSolomon(DABMW_mwAppTy vI_app)
{
	// return(DABMW_GetDabQualityReedSolomon(vI_app));  
	return(v_etalDabQuality.EtalQualityEntries.dab.m_reedSolomonInformation);
}

tU8 DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioBerLevel(DABMW_SF_mwAppTy vI_app)
{
	// Request to get the update of audio status 
	//DABMW_GetAudioStatusInformation(vI_app);
	
 // 	return(DABMW_GetDabQualityAudioBerLevel(vI_app));
	 return(v_etalDabQuality.EtalQualityEntries.dab.m_audioBitErrorRatioLevel);
}

// get the last DabSyncStatus Info
//
tU8 DABMW_ServiceFollowing_ExtInt_GetDabSyncStatus(DABMW_SF_mwAppTy vI_app)
{
	

	return(v_etalDabQuality.EtalQualityEntries.dab.m_syncStatus);
}

tU8 DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioMuteFlag(DABMW_mwAppTy vI_app)
{
	// return(DABMW_GetDabQualityAudioMuteFlag(vI_app));
	return(v_etalDabQuality.EtalQualityEntries.dab.m_muteFlag);
}


tBool DABMW_ServiceFollowing_ExtInt_DabFigLandscapeRegisterForFigAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_SF_DabFigCallbackTy callback, tU8 fig, tU8 ext, tBool reuse)
{
	return(false);
	
	// return(DABMW_DabFigLandscapeRegisterForFigAtFreq(frequency, paramPtr, callback, fig, ext, reuse));
}

tBool DABMW_ServiceFollowing_ExtInt_DabFigLandscapeDeRegister(DABMW_SF_DabFigCallbackTy callback, tU32 frequency, tU8 fig, tU8 ext)
{
	return(true);
	// return(DABMW_DabFigLandscapeDeRegister(callback, frequency, fig, ext));
}


tSInt DABMW_ServiceFollowing_ExtInt_BrDbLinkageSetBuildCheckArray(DABMW_SF_mwAppTy app, tU32 id, DABMW_idTy kindOfId, DABMW_BrDbLinkageSetTy **lsarray, tU32 *size, tU32 filter)
{

	return(OSAL_ERROR);
	/*
	tSInt vl_res = OSAL_OK;

	if (DABMW_BRDB_LS_ERROR == DABMW_BrDbLinkageSetBuildCheckArray(app, id, kindOfId, lsarray, size, filter))
		{
		vl_res = OSAL_ERROR;
		}

	return(vl_res);
	*/
}



#endif // #if defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING) 		

#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DAB_C


