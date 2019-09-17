//!
//!  \file 		etalapi.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, initialization functions
//!  \author 	Raffaele Belardi
//!

#ifndef ETALTML_INTERNAL_C
#define ETALTML_INTERNAL_C

#include "osal.h"
#if defined (CONFIG_ETAL_HAVE_ETALTML)


#include "etalinternal.h"

/***********************************
 *
 * Types
 *
 **********************************/

typedef struct
{
	ETAL_HANDLE m_handle;
} ETALTML_PathAllocationTy;


/***********************************
 *
 * Variables
 *
 **********************************/

static ETALTML_PathAllocationTy v_etaltml_pathAllocationArray[ETAL_PATH_NAME_NUMBER];

/***********************************
 *
 * Function prototypes
 *
 **********************************/
 
static tVoid ETALTML_StorePathAllocation(EtalPathName vI_path, ETAL_HANDLE vI_Receiver);


static tVoid ETALTML_DeletePathAllocation(EtalPathName vI_path);

static tVoid ETALTML_ReceiverDestroyCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);





/***********************************
 *
 * Functions
 *
 **********************************/
 
/***********************************
 *
 * Function to get a receiver based on path
 *
 * this functions allocates a receiver which correspond to requested path
 * 
 **********************************/
ETAL_STATUS ETALTML_getFreeReceiverForPathInternal(ETAL_HANDLE *pReceiver, EtalPathName vI_path, EtalReceiverAttr *pO_attr)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt vl_tmp_res;
	etalDiversTy *pl_FeList;
	tU8 vl_indexFeList = (tU8)0;
	tBool vl_found = false, vl_endOfListReached;
	EtalReceiverAttr vl_ReceiverConfig;


	// browse all the FE list for the path
	// this is ranked in priority order.

	do
	{
	
		ETALTML_PRINTF(TR_LEVEL_USER_1, "ETALTML_getFreeReceiverForPathInternal, attempting to get index %d\n", vl_indexFeList);
		vl_tmp_res = ETALTML_pathGetNextFEList(&pl_FeList, vI_path, vl_indexFeList);

		if ((OSAL_OK == vl_tmp_res) && (NULL != pl_FeList))
		{

			// a combination of FE  is available in the list. 
		 	// for this FE list, check if it is available.
			vl_endOfListReached	= false;

			ETALTML_PRINTF(TR_LEVEL_COMPONENT, "ETALTML_getFreeReceiverForPathInternal, FE found , m_FrontEndsSize %d, FRONT END LIST : 1 => 0x%x, 2 => 0x%x\n", 
													   pl_FeList->m_DiversityMode,
													   pl_FeList->m_FeConfig[0], pl_FeList->m_FeConfig[1]);

			// all Front End Free ? 
			vl_found = ETAL_isAllFrontendFree(pl_FeList->m_FeConfig, (tU32)pl_FeList->m_DiversityMode);
			
			ETALTML_PRINTF(TR_LEVEL_USER_1, "ETALTML_getFreeReceiverForPathInternal, FE found , free status = %d\n", vl_found);
					

			// do we support VPA ? 
			if (pl_FeList->m_DiversityMode == (tU8)2)
				{
				// diversity not supported for now
				vl_found = false;
				}
		}
		else
		{
			vl_endOfListReached = true;
		}

		
		if (false == vl_found)
		{
			// increment the FeList for next attempt
			vl_indexFeList++;
		}		
	} while((false == vl_found) && (false == vl_endOfListReached));
	// loop until we did not reach the end of the list and did not find anything. 

	// did we found somehting ? 
	if (true == vl_found)
	{
		ETALTML_PRINTF(TR_LEVEL_USER_1,"ETALTML_getFreeReceiverForPathInternal, found, index = %d\n", vl_indexFeList);

		OSAL_pvMemorySet((tVoid *)&vl_ReceiverConfig, 0x00, sizeof(vl_ReceiverConfig));
		// config receiver
		OSAL_pvMemoryCopy((tVoid *)vl_ReceiverConfig.m_FrontEnds, (tPCVoid)pl_FeList->m_FeConfig, ETAL_CAPA_MAX_FRONTEND * sizeof(ETAL_HANDLE));
		vl_ReceiverConfig.m_FrontEndsSize = pl_FeList->m_DiversityMode;
		vl_ReceiverConfig.m_Standard = ETALTML_pathGetStandard(vI_path);
		vl_ReceiverConfig.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		
		*pReceiver =  ETAL_INVALID_HANDLE;

		
		ret = ETAL_configReceiverInternal(pReceiver, &vl_ReceiverConfig);	

		ETALTML_PRINTF(TR_LEVEL_COMPONENT, "ETALTML_getFreeReceiverForPathInternal : standard %d,	m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d, etal_handle = %d\n", 
												vl_ReceiverConfig.m_Standard, 
												vl_ReceiverConfig.m_FrontEndsSize,
												vl_ReceiverConfig.m_FrontEnds[0], vl_ReceiverConfig.m_FrontEnds[1],
												*pReceiver);
	

		if (ETAL_RET_SUCCESS != ret)
			{
			ETALTML_PRINTF(TR_LEVEL_COMPONENT,"ETALTML_getFreeReceiverForPathInternal config receiver : return error %d\n", ret);
			}

		ETALTML_StorePathAllocation(vI_path, *pReceiver);

		// register for receiver destroy
		// we need also to register on receiver destroy 	
		if (ETAL_RET_SUCCESS != ETAL_intCbRegister(callAtReceiverDestroy, &ETALTML_ReceiverDestroyCallback, ETAL_INVALID_HANDLE, ETAL_INTCB_CONTEXT_UNUSED))
		{
			ret = ETAL_RET_ERROR;
		}
	
	}
	else
	{
		ETALTML_PRINTF(TR_LEVEL_COMPONENT, "ETALTML_getFreeReceiverForPathInternal, not found\n");
		ret = ETAL_RET_ERROR;
	}

	if (NULL != pO_attr)
	{
		// feedback the information on found FE
		//
		OSAL_pvMemoryCopy((tVoid *)pO_attr, (tPCVoid)&vl_ReceiverConfig, sizeof(EtalReceiverAttr));
	}
	
	return ret;
}

/***********************************
 *
 * Function to get a receiver based on path
 *
 * if Reuse is requested, this function checks if a receiver exist for the path
 * if not, looks to one
*
 **********************************/
ETAL_STATUS ETALTML_getReceiverForPathInternal(ETAL_HANDLE *pReceiver, EtalPathName vI_path, EtalReceiverAttr *pO_attr, tBool vI_reuse)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	etalDiversTy *pl_FeList;
	ETAL_HANDLE vl_Receiver = ETAL_INVALID_HANDLE;
	etalReceiverStatusTy *pl_ReceiverStatus;


	if (true == vI_reuse)
	{
		vl_Receiver = ETALTML_GetPathAllocation(vI_path);

		ETALTML_PRINTF(TR_LEVEL_USER_1, "ETALTML_getReceiverForPathInternal : reuse requested, path = %d, current receiver = %d\n",vI_path, vl_Receiver);

		// if a receiver exist, check it is valid
		if ((ETAL_INVALID_HANDLE != vl_Receiver) && (false == ETAL_receiverIsValidHandle(vl_Receiver)))
		{
			vl_Receiver = ETAL_INVALID_HANDLE;
		}

		//	if receiver found, allocate the attributes infos in case...
		if ((NULL != pO_attr) && (ETAL_INVALID_HANDLE != vl_Receiver))
		{
			// feedback the information on found FE
			//
			pl_ReceiverStatus = ETAL_receiverGet(vl_Receiver);
			pO_attr->m_FrontEndsSize = pl_ReceiverStatus->diversity.m_DiversityMode;
			pl_FeList =  &pl_ReceiverStatus->diversity ;

			if ((pO_attr->m_FrontEndsSize != (tU8)0) && (NULL != pl_FeList))
			{
				OSAL_pvMemoryCopy((tVoid *)pO_attr->m_FrontEnds, (tPCVoid)pl_FeList->m_FeConfig, ETAL_CAPA_MAX_FRONTEND * sizeof(ETAL_HANDLE));
			}

			pO_attr->m_Standard = ETAL_receiverGetStandard(vl_Receiver);
		}
	
	}


	if (ETAL_INVALID_HANDLE == vl_Receiver)
	{
		ret = ETALTML_getFreeReceiverForPathInternal(&vl_Receiver, vI_path, pO_attr);
	}

	*pReceiver = vl_Receiver;

	return ret;
	
}


static tVoid ETALTML_StorePathAllocation(EtalPathName vI_path, ETAL_HANDLE vI_Receiver)
{
	v_etaltml_pathAllocationArray[vI_path].m_handle = vI_Receiver;
}

static tVoid ETALTML_DeletePathAllocation(EtalPathName vI_path)
{
	v_etaltml_pathAllocationArray[vI_path].m_handle = ETAL_INVALID_HANDLE;
}

ETAL_HANDLE ETALTML_GetPathAllocation(EtalPathName vI_path)
{
	return(v_etaltml_pathAllocationArray[vI_path].m_handle);
}


tVoid ETALTML_PathAllocationInit()
{
	tU32 vl_path;

	for (vl_path=0;vl_path<ETAL_PATH_NAME_NUMBER;vl_path++)
	{
		v_etaltml_pathAllocationArray[vl_path].m_handle = ETAL_INVALID_HANDLE;
	}
}


// Procedure to manage the handler destruction : path should be init
//

static tVoid ETALTML_ReceiverDestroyCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
	// look if it is stored in the data path.
	// if so reset it
	tU32 vl_path;

	ETALTML_PRINTF(TR_LEVEL_USER_1, "ETALTML_ReceiverDestroyCallback, receiver %d\n", hGeneric);
	
	for (vl_path=0;vl_path<ETAL_PATH_NAME_NUMBER;vl_path++)
	{
		if (hGeneric == v_etaltml_pathAllocationArray[vl_path].m_handle)
		{
			ETALTML_DeletePathAllocation((EtalPathName)vl_path);
		}
	}
}



#endif // #if defined (CONFIG_ETAL_HAVE_ETALTML)

#endif // ETALTML_INTERNAL_C

