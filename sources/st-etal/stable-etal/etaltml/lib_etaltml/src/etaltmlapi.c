//!
//!  \file 		etalapi.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, initialization functions
//!  \author 	Raffaele Belardi
//!

#ifndef ETALTML_API_C
#define ETALTML_API_C

#include "osal.h"
#if defined (CONFIG_ETAL_HAVE_ETALTML)


#include "etalinternal.h"

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
#include "rds_landscape.h"
#endif
#include "dabmw_import.h"
#include "fic_common.h"

/***************************
 *
 * etaltml_getFreeReceiverForPath
 *
 **************************/
ETAL_STATUS etaltml_getFreeReceiverForPath(ETAL_HANDLE *pReceiver, EtalPathName vI_path,  EtalReceiverAttr *pO_attr)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU32 i;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_getFreeReceiverForPath(vIPat: %d)", vI_path);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
	    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_getFreeReceiverForPath() = %s", ETAL_STATUS_toString(ETAL_RET_ERROR));
		return ETAL_RET_ERROR;
	}

	ret = ETALTML_getFreeReceiverForPathInternal(pReceiver, vI_path, pO_attr);

	ETAL_statusReleaseLock();

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_getFreeReceiverForPath() = %s", ETAL_STATUS_toString(ret));
	if((pReceiver != NULL) && (pO_attr != NULL))
	{
	    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "rec: %d, recAtt: (sta: %d, froEndSiz: %d, proFea: %d)", *pReceiver,
	            pO_attr->m_Standard, pO_attr->m_FrontEndsSize, pO_attr->processingFeatures.u.m_processing_features);

		if (ret == ETAL_RET_SUCCESS)
		{
			// if Ret ok, print the front end.
			// else it is invalide
	    	for(i = 0; i < pO_attr->m_FrontEndsSize; i++)
		    {
		        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "froEnd[%d]: 0x%x", i, pO_attr->m_FrontEnds[i]);
		    }
		}
	}
	return ret;
}

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
/***************************
 *
 * etaltml_TuneOnServiceId
 *
 **************************/

ETAL_STATUS etaltml_TuneOnServiceId(EtalPathName vI_path,  tU32 vI_SearchedServiceID)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_TuneOnServiceId(vIPat: %d, seaSerID: %d)", vI_path, vI_SearchedServiceID);

    if(OSAL_OK != DABMW_ServiceFollowing_TuneServiceId(vI_path, vI_SearchedServiceID))
    {
        ret = ETAL_RET_ERROR;
    }

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_TuneOnServiceId() = %s", ETAL_STATUS_toString(ret));
    return(ret);
}

/***************************
 *
 * etaltml_ActivateServiceFollowing
 *
 **************************/

ETAL_STATUS etaltml_ActivateServiceFollowing()
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_ActivateServiceFollowing()");

    if (OSAL_OK != DABMW_ServiceFollowing_ExtInt_ActivateSF())
    {
        ret = ETAL_RET_ERROR;
    }

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_ActivateServiceFollowing() = %s", ETAL_STATUS_toString(ret));
    return(ret);
}

/***************************
 *
 * etaltml_DisableServiceFollowing
 *
 **************************/

ETAL_STATUS etaltml_DisableServiceFollowing()
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_DisableServiceFollowing()");
    if (OSAL_OK != DABMW_ServiceFollowing_ExtInt_DisableSF())
    {
        ret = ETAL_RET_ERROR;
    }

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_DisableServiceFollowing() = %s", ETAL_STATUS_toString(ret));
    return(ret);
}

/***************************
 *
 * etaltml_SelectKindOfSwitchServiceFollowing
 *
 **************************/

ETAL_STATUS etaltml_SelectKindOfSwitchServiceFollowing(tBool vI_fmfm, tBool vI_dabfm, tBool vI_dabdab)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_SelectKindOfSwitchServiceFollowing(fmfm:%d, dabfm:%d, dabdab:%d)", vI_fmfm, vI_dabfm, vI_dabdab);
	
	
    if (OSAL_OK != DABWM_ServiceFollowing_ExtInt_SelectKindOfSwitch(vI_fmfm, vI_dabfm, vI_dabdab))
    {
        ret = ETAL_RET_ERROR;
    }

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_DisableServiceFollowing() = %s", ETAL_STATUS_toString(ret));
	
    return(ret);
}


#endif // CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

// landscape functionnality
/***************************
 *
 * ETALTML_GetEnsembleList
 *
 **************************/
/*!
 * \brief		Returns the ensemble information stored in DAB landscape
 * \param[in]	pointer to carry the list of ensemble and ensemble nformation
 * \param[out] pO_dataPtr	- pointer to carry the list of ensemble and ensemble nformation.
 * \param[out] pO_NbEnsemble	 - pointer to number of ensemble in database
 * \return		#ETAL_RET_ERROR - communication or semaphore access error
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
ETAL_STATUS ETALTML_GetEnsembleList (DABMW_ensembleUniqueIdTy *pO_dataPtr, tPU16 pO_NbEnsemble )
{



	EtalEnsembleList vl_EnsembleList;
//	static	DABMW_ensembleUniqueIdTy vl_ensembleUniqueId[DABMW_FIC_MAX_ENSEMBLE_NUMBER];
	tU8 vl_cnt;
	tBool vl_haveData;
	ETAL_STATUS vl_ret = ETAL_RET_SUCCESS;

 	// Addition of  Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	// We do not have receiver, so can't lock ... but may lead to error ?
	//

	if (ETAL_cmdGetEnsembleList_MDR(&vl_EnsembleList, &vl_haveData) != OSAL_OK)
	{
		vl_ret = ETAL_RET_ERROR;
	}
	else
	{
		if (!vl_haveData)
		{
			vl_ret = ETAL_RET_NO_DATA;
		}
	}

	if (ETAL_RET_SUCCESS == vl_ret)
	{
		*pO_NbEnsemble = vl_EnsembleList.m_ensembleCount;
		
		for (vl_cnt=0;vl_cnt<*pO_NbEnsemble; vl_cnt++)
		{
			pO_dataPtr[vl_cnt].ecc = vl_EnsembleList.m_ensemble[vl_cnt].m_ECC;
			pO_dataPtr[vl_cnt].frequency = vl_EnsembleList.m_ensemble[vl_cnt].m_frequency;
			pO_dataPtr[vl_cnt].id = vl_EnsembleList.m_ensemble[vl_cnt].m_ensembleId;
			OSAL_pvMemorySet(pO_dataPtr[vl_cnt].alternativeFrequencies, DABMW_INVALID_FREQUENCY, sizeof(pO_dataPtr[vl_cnt].alternativeFrequencies));
		}

	}
	else
	{
		*pO_NbEnsemble = 0;
	}


	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETALTML_GetEnsembleList : Nb ensembles %d\n",
					*pO_NbEnsemble);

	for (vl_cnt=0;vl_cnt<*pO_NbEnsemble; vl_cnt++)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETALTML_GetEnsembleList : ecc 0x%x, id 0x%x, frequency %d\n",
					pO_dataPtr[vl_cnt].ecc, pO_dataPtr[vl_cnt].id, pO_dataPtr[vl_cnt].frequency);
	}

    return vl_ret;
}

// Process service functionality
tSInt ETALTML_GetServiceList(tU32 vI_ensembleId, EtalServiceList *pO_dataPtr)
{
	
		EtalServiceList vl_ServiceList;
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
	
			memcpy(pO_dataPtr, &vl_ServiceList, sizeof(EtalServiceList));
		}
#else
		vl_numService = 0;
#endif


		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETALTML_GetServiceList : Ensemble ID 0x%d, Nb SID %d\n",
						vI_ensembleId, vl_numService);
		
		for (vl_cnt=0;vl_cnt<vl_numService; vl_cnt++)
		{
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETALTML_GetServiceList : Sid 0x%x\n",
						vl_ServiceList.m_service[vl_cnt]);
		}
		
		return(vl_numService);
}

	
#endif // #ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR


/***************************
 *
 * etaltml_landscape_GetNbServices
 *
 **************************/
ETAL_STATUS etaltml_landscape_GetNbServices(tU32 *pO_NbFreq, tBool vI_AmFmRequested,  tBool vI_DabRequested	, OSAL_tMSecond vI_piValidityDuration)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#if CONFIG_ETAL_SUPPORT_DCOP_MDR
	DABMW_ensembleUniqueIdTy vl_ensembleId[DABMW_FIC_MAX_ENSEMBLE_NUMBER];
	tU16 vl_nbEnsemble;
	tU16 vl_nbService;
	tSInt vl_eidCnt;
	DABMW_ensembleUniqueIdTy *pl_ensembleIdsPtr_2 ;
	tU32 vl_ensembleUniqueId;
	EtalServiceList vl_ServiceList;
#endif

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_landscape_GetNbServices(AmFm = %d, Dab = %d, ValDur = %d)",
			vI_AmFmRequested, vI_DabRequested, vI_piValidityDuration);

	if (NULL == pO_NbFreq)
	{
		ret = ETAL_RET_ERROR;
	}
	else
	{
		*pO_NbFreq = 0;
		
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE		
		if (true == vI_AmFmRequested)
		{
			*pO_NbFreq += DABMW_AmFmLandscapeGetNbValid_FreqList(vI_piValidityDuration);
		}
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		if (true == vI_DabRequested)
		{
			// get the ensemble information 
			// 

			// for each ensemble, get the service information
			//
			if (ETAL_RET_SUCCESS == ETALTML_GetEnsembleList((DABMW_ensembleUniqueIdTy *)&vl_ensembleId, &vl_nbEnsemble))
			{	
				// if ensemble exists, parse each of those to get the service list on each ensemble and check if one match
						
				for (vl_eidCnt = 0; vl_eidCnt < vl_nbEnsemble; vl_eidCnt++)
				{
					
					pl_ensembleIdsPtr_2 = &vl_ensembleId[vl_eidCnt];
					
					vl_ensembleUniqueId = DABMW_ENSEMBLE_UNIQUE_ID (pl_ensembleIdsPtr_2->ecc, pl_ensembleIdsPtr_2->id);
					vl_nbService = ETALTML_GetServiceList (vl_ensembleUniqueId, &vl_ServiceList);
					*pO_NbFreq += vl_nbService;
							
					/* move to next ensemble */
					pl_ensembleIdsPtr_2++;
				}
			}
			else
			{	
			
				ret = ETAL_RET_ERROR;
				vl_nbEnsemble = 0;
			}
		}
#endif
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_landscape_GetNbServices(pO_NbFreq = %d) = %s", *pO_NbFreq, ETAL_STATUS_toString(ret));
    return(ret);
}

ETAL_STATUS etaltml_landscape_GetServices(ETAL_DatabaseExternalInfo *pO_LanscapeInfo, tBool vI_AmFmRequested,  tBool vI_DabRequested, OSAL_tMSecond vI_piValidityDuration)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#if CONFIG_ETAL_SUPPORT_DCOP_MDR
	unsigned char vl_charset;
	DABMW_ensembleUniqueIdTy vl_ensembleId[DABMW_FIC_MAX_ENSEMBLE_NUMBER];
	tU16 vl_nbEnsemble;
	tU16 vl_nbService;
	tSInt vl_eidCnt, vl_sidCnt;
	DABMW_ensembleUniqueIdTy *pl_ensembleIdsPtr_2 ;
	tU32 vl_ensembleUniqueId;
	EtalServiceList vl_ServiceList;
	tU16 vl_bitmap;
	tBool vl_havedata;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size
	tSInt vl_res;
#endif

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_landscape_GetServices(AmFm = %d, Dab = %d, ValDur = %d)",
			vI_AmFmRequested, vI_DabRequested, vI_piValidityDuration);

	if (NULL == pO_LanscapeInfo)
	{
		ret = ETAL_RET_ERROR;
	}
	else
	{	
		pO_LanscapeInfo->m_FMLanscapeLen = 0;

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE	
		if (true == vI_AmFmRequested)
		{
			pO_LanscapeInfo->m_FMLanscapeLen += DABMW_AmFmLandscapeGetValid_FreqList((EtalRdsLandscapeExternalInfo *)&(pO_LanscapeInfo->m_FMLandscapeInfo), ETAL_DEF_MAX_FM_LANDSCAPE_SIZE, vI_piValidityDuration);
		}
#endif

#if CONFIG_ETAL_SUPPORT_DCOP_MDR
		if (true == vI_DabRequested)
		{

			// get the ensemble information 
			// 
			// init the data
			pO_LanscapeInfo->m_DABdbLen = 0;

			// for each ensemble, get the service information
			//
			if (ETAL_RET_SUCCESS == ETALTML_GetEnsembleList((DABMW_ensembleUniqueIdTy *)&vl_ensembleId, &vl_nbEnsemble))
			{	
				// if ensemble exists, parse each of those to get the service list on each ensemble and check if one match
				pO_LanscapeInfo->m_DabLandscapeInfo.m_ensembleCount = vl_nbEnsemble;
				
				for (vl_eidCnt = 0; vl_eidCnt < vl_nbEnsemble; vl_eidCnt++)
				{
					// for each ensemble, fill the information : ECC/ID/FREQ
					// 
					pl_ensembleIdsPtr_2 = &vl_ensembleId[vl_eidCnt];
					
					vl_ensembleUniqueId = DABMW_ENSEMBLE_UNIQUE_ID (pl_ensembleIdsPtr_2->ecc, pl_ensembleIdsPtr_2->id);
	
					pO_LanscapeInfo->m_DabLandscapeInfo.m_ensembleList[vl_eidCnt].m_UeId = vl_ensembleUniqueId;
					pO_LanscapeInfo->m_DabLandscapeInfo.m_ensembleList[vl_eidCnt].m_frequency = vl_ensembleId[vl_eidCnt].frequency;

					// get the ensemble label
					vl_res = ETAL_cmdGetEnsembleData_MDR(vl_ensembleUniqueId, &vl_charset, pO_LanscapeInfo->m_DabLandscapeInfo.m_ensembleList[vl_eidCnt].m_ensembleLabel, &vl_bitmap, &vl_havedata);

					if (OSAL_OK == vl_res)
					{
						// now get the service info for each 			
						vl_nbService = ETALTML_GetServiceList (vl_ensembleUniqueId, &vl_ServiceList);

						// increment the total nb of services
						pO_LanscapeInfo->m_DABdbLen += vl_ServiceList.m_serviceCount;

						pO_LanscapeInfo->m_DabLandscapeInfo.m_serviceList[vl_eidCnt].m_serviceCount = vl_nbService;

						for (vl_sidCnt=0; vl_sidCnt<vl_nbService; vl_sidCnt++)
						{
							pO_LanscapeInfo->m_DabLandscapeInfo.m_serviceList[vl_eidCnt].m_service[vl_sidCnt] = vl_ServiceList.m_service[vl_sidCnt];

							ETAL_cmdGetSpecificServiceData_MDR(vl_ensembleUniqueId, vl_ServiceList.m_service[vl_sidCnt], &svinfo, &scinfo, &vl_havedata);
							strcpy( pO_LanscapeInfo->m_DabLandscapeInfo.m_serviceList[vl_eidCnt].m_serviceLabel[vl_sidCnt], svinfo.m_serviceLabel );
						}
					}
					else
					{
						// an error occured
						ret = ETAL_RET_ERROR;
						vl_nbEnsemble = 0;
					}

				}
			}
			else
			{	
			
				ret = ETAL_RET_ERROR;
				vl_nbEnsemble = 0;
			}
		}
#endif

	}
	
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_landscape_GetServices(pO_LanscapeInfo->FMLanscapeLen = %d) = %s", pO_LanscapeInfo->m_FMLanscapeLen, ETAL_STATUS_toString(ret));
    return(ret);
}


#endif // #if defined (CONFIG_ETAL_HAVE_ETALTML)

#endif // ETALTML_API_C

