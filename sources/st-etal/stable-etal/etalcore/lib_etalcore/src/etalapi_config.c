//!
//!  \file 		etalapi_config.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, configuration functions
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"
#if defined(CONFIG_DIGITAL_AUDIO) || defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
#include "bsp_sta1095evb.h"
#endif
#ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
#include "libDABDevice.h"
#include "dabdevadapt.h"
#endif
#ifndef CONFIG_HOST_OS_TKERNEL
#if defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETAL_HAVE_ALL_API)
	#include <math.h>  /* for round() */
#endif
#else // CONFIG_HOST_OS_TKERNEL
extern long round(double x);
#endif

/***************************
 *
 * Macros
 *
 **************************/
#define RDWR_PARAM_NUM          1
#define CONST_2_E_33            ((tU64)1 << 33)
#define CONST_2_E_24            ((tU64)1 << 24)

/***************************
 *
 * Local variables
 *
 **************************/

/***************************
 *
 * Local function
 *
 **************************/
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
static ETAL_STATUS ETAL_configReceiverVPAInternal(ETAL_HANDLE hReceiver, EtalReceiverAttr *pReceiverConfig);
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

/***************************
 *
 * Local function
 *
 **************************/

#ifdef CONFIG_ETAL_SUPPORT_CMOST
/***************************
 *
 * ETAL_FEHandleToChannel_CMOST
 *
 **************************/
/*!
 * \brief		Converts the ETAL FE Handle to ETAL channel identifier
 * \param[in]	address - the Frontend Handle
 * \return		the converted address, or ETAL_CHN_UNDEF if *address* is illegal
 * \callgraph
 * \callergraph
 */
etalChannelTy ETAL_FEHandleToChannel_CMOST(ETAL_HANDLE hFrontend)
{
	ETAL_HINDEX channelIndex;
	etalChannelTy channel;

	channelIndex = ETAL_handleFrontendGetChannel(hFrontend);
	switch (channelIndex)
	{
		case ETAL_FE_FOREGROUND:
			channel = ETAL_CHN_FOREGROUND;
			break;

		case ETAL_FE_BACKGROUND:
			channel = ETAL_CHN_BACKGROUND;
			break;

		default:
			ASSERT_ON_DEBUGGING(0);
			channel = ETAL_CHN_UNDEF;
			break;
	}

	return channel;
}


/***************************
 *
 * ETAL_initReceiverConfig_CMOST
 *
 **************************/
/*!
 * \brief		Initializes the channel and tunerType etalReceiverStatusTy fields
 * 				of the etalReceiverStatusTy
 * \details		Derives from the m_DiversityMode and the m_FeConfig array 
 * 				contained in *pstat* the ETAL internal identifiers for the
 * 				corresponding the Front Ends.
 * 				
 * 				Initializes the tunerType field based on the tunerId field.
 * \param[in,out] pstat - pointer to the receiver configuration
 * \return		OSAL_OK
 * \return		OSAL_ERROR - illegal number of front ends requested in m_DiversityMode
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_initReceiverConfig_CMOST(etalReceiverStatusTy *pstat)
{
	ETAL_HANDLE hFrontend;
	ETAL_HANDLE hTuner;
	tSInt ret = OSAL_OK;

	hTuner = pstat->CMOSTConfig.tunerId;
	switch (pstat->diversity.m_DiversityMode)
	{
		case 2:
			pstat->CMOSTConfig.channel = ETAL_CHN_BOTH;
			break;

		case 1:
			hFrontend = pstat->diversity.m_FeConfig[0];
			pstat->CMOSTConfig.channel = ETAL_FEHandleToChannel_CMOST(hFrontend);
			break;

		default:
			ASSERT_ON_DEBUGGING(0);
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Illegal number of FrontEnds (%d)", pstat->diversity.m_DiversityMode);
			ret = OSAL_ERROR;
			break;
	}

	if(ret == OSAL_OK)
	{
	pstat->tunerType = ETAL_tunerGetType(hTuner);
}
	
	return ret;
}
#endif // CONFIG_ETAL_SUPPORT_CMOST

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
/***************************
 *
 * ETAL_FEHandleToApplication_MDR
 *
 **************************/
/*!
 * \brief		Converts the ETAL Frontend handle to ETAL application identifier
 * \details		The *std* is needed because the MDR uses different
 * 				application identifiers based on the standard.
 * \param[in]	std - the broadcast standard
 * \param[in]	address - the Frontend handle
 * \return		the MDR application identifier, or DABMW_NONE_APP in
 * 				case of unknown *std*
 * \callgraph
 * \callergraph
 * \todo		the front end address cannot be used in multi-tuner configuration
 */
static DABMW_mwAppTy ETAL_FEHandleToApplication_MDR(EtalBcastStandard std, ETAL_HANDLE hFrontend)
{
	ETAL_HINDEX channelIndex;

	channelIndex = ETAL_handleFrontendGetChannel(hFrontend);	
	switch (std)
	{
		case ETAL_BCAST_STD_DAB:
			if (channelIndex == ETAL_FE_FOREGROUND)
			{
				return DABMW_MAIN_AUDIO_DAB_APP;
			}
			else if (channelIndex == ETAL_FE_BACKGROUND)
			{
				return DABMW_SECONDARY_AUDIO_DAB_APP;
			}
			break;

		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
			if (channelIndex == ETAL_FE_FOREGROUND)
			{
				return DABMW_MAIN_AMFM_APP;
			}
			else if (channelIndex == ETAL_FE_BACKGROUND)
			{
				return DABMW_BACKGROUND_AMFM_APP;
			}
			break;

		default:
			break;
	}
	ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "Unsupported DCOP DAB frontend handle (0x%.2x) or standard (%d)", hFrontend, std);
	return DABMW_NONE_APP;
}

/***************************
 *
 * ETAL_initReceiverConfig_MDR
 *
 **************************/
/*!
 * \brief		Initializes the application field of etalReceiverStatusTy
 * \details		Derives from the m_DiversityMode and the m_FeConfig array 
 * 				contained in *pstat* the ETAL internal identifiers for the
 * 				corresponding the Front Ends.
 * \remark		Does not support multiple tuner configurations
 * \param[in,out] pstat - pointer to the receiver configuration
 * \return		OSAL_OK
 * \return		OSAL_ERROR - illegal number of frontends requested in m_DiversityMode
 * \callgraph
 * \callergraph
 * \todo		for STA662/FM the second FrontEnd could be used for
 * 				diversity mode selection; currently not supported
 */
static tSInt ETAL_initReceiverConfig_MDR(etalReceiverStatusTy *pstat)
{
	ETAL_HANDLE hFrontend;
	tSInt vl_ret = OSAL_OK;

	/* TODO see function header */
	if (pstat->diversity.m_DiversityMode != (tU8)1)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_initReceiverConfig_MDR invalid diversity configuration %d", 
			pstat->diversity.m_DiversityMode);
		
		vl_ret = OSAL_ERROR;
	}
	else
	{
		hFrontend = pstat->diversity.m_FeConfig[0];

		if (hFrontend != ETAL_INVALID_HANDLE)
		{
			pstat->MDRConfig.application = ETAL_FEHandleToApplication_MDR(pstat->currentStandard, hFrontend);
		}
	}
	
	return vl_ret;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/***************************
 *
 * ETAL_FEaddressToInstanceId_HDRADIO
 *
 **************************/
/*!
 * \brief		Converts an ETAL FrontEnd handle to a HD Radio Instance Id
 * \param[in]	hFrontend - the ETAL FrontEnd handle
 * \return		The converted InstanceID, or INSTANCE_UNDEF if *address* cannot be converted
 * \callgraph
 * \callergraph
 */
static tyHDRADIOInstanceID ETAL_FEaddressToInstanceId_HDRADIO(ETAL_HANDLE hFrontend)
{
	ETAL_HINDEX channelIndex;
	tyHDRADIOInstanceID ID;

	channelIndex = ETAL_handleFrontendGetChannel(hFrontend);
	switch (channelIndex)
	{
		case ETAL_FE_FOREGROUND:
			ID = INSTANCE_1;
			break;

		case ETAL_FE_BACKGROUND:
			ID = INSTANCE_2;
			break;

		default:
			ID = INSTANCE_UNDEF;
			break;
	}
	
	return ID;
}

/***************************
 *
 * ETAL_initReceiverConfig_HDRADIO
 *
 **************************/
/*!
 * \brief		Initializes the instanceId field of etalReceiverStatusTy
 * \details		Derives from the m_DiversityMode and the m_FeConfig array 
 * 				contained in *pstat* the ETAL internal identifiers for the
 * 				corresponding the Front Ends.
 * \remark		In case of multiple FEs considers only the Foreground CMOST channel
 * 				and associates it to INSTANCE_1 
 * \param[in,out] pstat - pointer to the receiver configuration
 * \return		OSAL_OK
 * \return		OSAL_ERROR - illegal number of frontends requested in m_DiversityMode
 * \see			ETAL_InitCMOSTtoHDRADIOInterface, call to ETAL_directCmdChangeBandTune_CMOST
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_initReceiverConfig_HDRADIO(etalReceiverStatusTy *pstat)
{
	ETAL_HANDLE hFrontend;
	tSInt ret = OSAL_OK;

	/*
	 * Initialize the instanceId based on the requested CMOST foreground channel
	 * We assume that there is a one-to-one correspondence between CMOST channel
	 * and HDRADIO instance
	 */
	if (pstat->diversity.m_DiversityMode != (tU8)1) 
	{
		if (pstat->currentStandard != ETAL_BCAST_STD_HD_FM)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
		/* take foreground frontend */
		if (pstat->diversity.m_FeConfig[0] < pstat->diversity.m_FeConfig[1])
		{
			hFrontend = pstat->diversity.m_FeConfig[0];
		}
		else
		{
			hFrontend = pstat->diversity.m_FeConfig[1];
		}
	}
	else
	{
		hFrontend = pstat->diversity.m_FeConfig[0];
	}

	if (hFrontend != ETAL_INVALID_HANDLE)
	{
		pstat->HDRADIOConfig.instanceId = ETAL_FEaddressToInstanceId_HDRADIO(hFrontend);
	}

exit:
	return ret;

}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

/***************************
 *
 * ETAL_checkInitStandard
 *
 **************************/
/*!
 * \brief		Checks the Broadcast Standard and initializes the
 * 				currentStandard of etalReceiverStatusTy
 * \param[in]	pReceiverConfig - pointer to the requested receiver config
 * \param[out]	st - pointer to the receiver status
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid standard, or not supported by the current
 * 				             ETAL build
 * \callgraph
 * \callergraph
 * \todo		check if the tuner requested in pReceiverConfig is active; need
 * 				the new TUNER_HANDLE for that
 */
static tSInt ETAL_checkInitStandard(etalReceiverStatusTy *st, const EtalReceiverAttr* pReceiverConfig)
{
	tSInt ret = OSAL_OK;
	switch (pReceiverConfig->m_Standard)
	{
#if defined CONFIG_ETAL_SUPPORT_DCOP_MDR
		case ETAL_BCAST_STD_DAB:
#endif
#if defined CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP)
			if (!ETAL_statusHardwareAttrIsDCOPActive())
			{
				ret = OSAL_ERROR;
				goto exit;
			}
			break;
#endif
		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
			/* TODO: see function header */
			break;
		default:
			ret = OSAL_ERROR;
			goto exit;
	}
	st->currentStandard = pReceiverConfig->m_Standard;

exit:
	return ret;
}

/***************************
 *
 * ETAL_countRequestedFrontEnd
 *
 **************************/
/*!
 * \brief		Returns the number of Frontends for the requested receiver configuration
 * \details		Counts the number of valid Frontends skipping non-initialized entries
 * \param[in]	pReceiverConfig - pointer to the requested receiver config
 * \return		The number of Frontends
 * \callgraph
 * \callergraph
 */
static tU32 ETAL_countRequestedFrontEnds(const EtalReceiverAttr* pReceiverConfig)
{
	tU32 i, count;

	count = 0;
	
	for (i = 0; i < ETAL_CAPA_MAX_FRONTEND; i++)
	{
		if (pReceiverConfig->m_FrontEnds[i] != ETAL_INVALID_HANDLE)
		{
			count++;
		}
	}
	return count;
}

/***************************
 *
 * ETAL_isAllFrontendFree
 *
 **************************/
/*!
 * \brief		Checks if at least one of the requested Front Ends is already used by a Receiver
 * \details		The function checks all the Front Ends in the *list* until it 
 * 				finds the first #ETAL_INVALID_HANDLE entry.
 * \param[in]	list - array of #ETAL_HANDLE containing *size* elements; the function
 * 				       considers all the entries on the list up to the first
 * 				       entry containing #ETAL_INVALID_HANDLE
 * \param[in]	size - dimension of the *list* array, considering all entries
 * 				       (valid and #ETAL_INVALID_HANDLE)
 * \return		TRUE if none of the Front Ends listed in *list* is currently used
 * \callgraph
 * \callergraph
 */
tBool ETAL_isAllFrontendFree(const ETAL_HANDLE *list, tU32 size)
{
	tU32 i, j;
	ETAL_HANDLE hReceiver;
	ETAL_HINDEX receiver_index;
	ETAL_HANDLE hFrontend;
	etalReceiverStatusTy *recvp;
	tBool ret = TRUE;

	for (i = 0; i < size; i++)
	{
		hFrontend = list[i];
		if (hFrontend == ETAL_INVALID_HANDLE)
		{
			break;
		}
		for (j = 0; j < ETAL_MAX_RECEIVERS; j++)
		{
			receiver_index = (ETAL_HINDEX)j;
			hReceiver = ETAL_handleMakeReceiver(receiver_index);
			if (ETAL_receiverIsValidHandle(hReceiver))
			{
				recvp = ETAL_receiverGet(hReceiver);
				if (recvp == NULL)
				{
					/* will never happen since hReceiver is correct by construction
					 * done to avoid SCC warning */
					ret = FALSE;
					goto exit;
				}
				else if (ETAL_receiverIsFrontendUsed(recvp, hFrontend))
				{
					ret = FALSE;
					goto exit;
				}
				else
				{
					/* Nothing to do */
			}
		}
	}
}

exit:
	return ret;
}

/***************************
 *
 * ETAL_checkInitFrontEnd
 *
 **************************/
/*!
 * \brief		Checks the Frontend list
 * \details		Validates the requested Frontend list and copies it to the
 * 				#etalReceiverStatusTy.
 * 				Performs the following checks:
 * 				- all Frontends are part of a unique Tuner
 * 				- all Frontends are present in the Capabilities
 * 				- the Frontends are compatible with the requested Broadcast Standard
 * 				  based on the Capabilities
 * 				- none of the Frontends is already employed in a Receiver
 * 				- the list is compatible with the requested VPA configuration
 * \param[in]	pReceiverConfig - pointer to the requested configuration
 * \param[out]	st - pointer to the structure where the validated list will be stored
 * \return		OSAL_OK
 * \return		OSAL_ERROR - one of the checks failed
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_checkInitFrontEnd(etalReceiverStatusTy *st, const EtalReceiverAttr* pReceiverConfig)
{
	tU32 i, size;
	ETAL_HANDLE hTuner;
	tSInt ret = OSAL_OK;

	size = ETAL_countRequestedFrontEnds(pReceiverConfig);

	if (size == 0)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkInitFrontEnd invalid FE size = 0");
		ret = OSAL_ERROR;
		goto exit;
	}

	/* Ensure all the FE are in one Tuner and get the tuner id */
	if (ETAL_tunerSearchAllFrontend(pReceiverConfig->m_FrontEnds, size, &hTuner) != OSAL_OK)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkInitFrontEnd invalid FE not all same tuner");
		ret = OSAL_ERROR;
		goto exit;
	}

	/* First round, ensure that all the FE handles are available in the Capabilities */
	for (i = 0; i < size; i++)
	{
		if (ETAL_tunerSearchFrontend(pReceiverConfig->m_FrontEnds[i]) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkInitFrontEnd : FE %d %x ETAL_tunerSearchFrontend failed",
				i, pReceiverConfig->m_FrontEnds[i]);
			ret = OSAL_ERROR;
			goto exit;
		}
	}

	/* Second round, ensure the requested FE is compatible with the standard */
	st->supportedStandards = (tU32)0xFFFFFFFF; // default to all standards supported
	for (i = 0; i < size; i++)
	{
		tU32 supported_standards;

		supported_standards = ETAL_tunerGetFEsupportedStandards(pReceiverConfig->m_FrontEnds[i]);
		if ((supported_standards & pReceiverConfig->m_Standard) != pReceiverConfig->m_Standard)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkInitFrontEnd : FE %d %x not supporting the standard", 
				i, pReceiverConfig->m_FrontEnds[i]);
			ret = OSAL_ERROR;
			goto exit;
		}

		/*
		 * in case more than one frontend is associated to this receiver,
		 * take only the subset of standards supported by all frontends
		 */
		st->supportedStandards &= supported_standards;
	}

	/* Third, check if the requested FE is already selected in some other ETAL receiver */
	if (!ETAL_isAllFrontendFree(pReceiverConfig->m_FrontEnds, size))
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkInitFrontEnd : all FE not free, size = %d", size);

		ret = OSAL_ERROR;
		goto exit;
	}

	// check the processing feature has been correctly updated.
	//
	if (pReceiverConfig->processingFeatures.u.m_processing_features == (tU8)ETAL_PROCESSING_FEATURE_UNSPECIFIED)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkInitFrontEnd : processing feature unspecified");
		ret = OSAL_ERROR;
		goto exit;
	}
	
	/* check if valid VPA or Antenna Diversity configuration */
	if (((pReceiverConfig->processingFeatures.u.m_processing_features & ((tU8)ETAL_PROCESSING_FEATURE_FM_VPA | (tU8)ETAL_PROCESSING_FEATURE_ANTENNA_DIVERSITY)) != (tU8)0)
		&&
		((pReceiverConfig->m_FrontEndsSize != (tU8)2) || (pReceiverConfig->m_FrontEnds[0] == pReceiverConfig->m_FrontEnds[1]))
		)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkInitFrontEnd : FE invalid VPA config");
		ret = OSAL_ERROR;
		goto exit;
	}

	// Check valid FE size
	// if FE size is 2, VPA or DIVERSITY needs to be ON.
	// VPA / Diversity supported only for FM
	//
	
	if (pReceiverConfig->m_FrontEndsSize == (tU8)2)
	{
		if ((pReceiverConfig->processingFeatures.u.m_processing_features & ((tU8)ETAL_PROCESSING_FEATURE_FM_VPA | (tU8)ETAL_PROCESSING_FEATURE_ANTENNA_DIVERSITY)) == (tU8)0)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkInitFrontEnd : FE invalid size configured 2 but VPA/Diversity not supported for configuration");
			ret = OSAL_ERROR;
			goto exit;
		}


		if ((pReceiverConfig->m_Standard != ETAL_BCAST_STD_FM) && (pReceiverConfig->m_Standard != ETAL_BCAST_STD_HD_FM))
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkInitFrontEnd : FE invalid configuration : VPA request for a non-FM standard %d", pReceiverConfig->m_Standard);
			ret = OSAL_ERROR;
			goto exit;
		}
	
	}
	/* copy the validated FE list to output */
	st->diversity.m_DiversityMode = (tU8)size; /* already checked to fit in a tU8 */
	for (i = 0; i < size; i++)
	{
		st->diversity.m_FeConfig[i] = pReceiverConfig->m_FrontEnds[i];
	}
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	st->CMOSTConfig.tunerId = hTuner;
#endif

exit:
	return ret;
}

/***************************
 *
 * ETAL_checkInitConfigRequest
 *
 **************************/
/*!
 * \brief		Checks if the requested configuration is valid
 * \param[in]	pReceiverConfig - pointer to the requested configuration
 * \param[out]	pstat - pointer to the structure where the validated configuration is stored
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_INVALID_BCAST_STANDARD
 * \return		ETAL_RET_FRONTEND_LIST_ERR
 * \see			ETAL_checkInitStandard, ETAL_checkInitFrontEnd
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_checkInitConfigRequest(etalReceiverStatusTy *pstat, const EtalReceiverAttr* pReceiverConfig)
{
	ETAL_STATUS ret;


	ret = ETAL_RET_SUCCESS;

	if (ETAL_checkInitStandard(pstat, pReceiverConfig) != OSAL_OK)
	{
		ret = ETAL_RET_INVALID_BCAST_STANDARD;
	}
	else if (ETAL_checkInitFrontEnd(pstat, pReceiverConfig) != OSAL_OK)
	{
		ret = ETAL_RET_FRONTEND_LIST_ERR;
	}
	else
	{
		/* Nothing to do */
	}
	
	return ret;
}

/***************************
 *
 * ETAL_checkStandardDatatypeCompatibility
 *
 **************************/
/*!
 * \brief		Checks if a data type is compatible with a Broadcast Standard
 * \remark		Uses hard-coded rules, not the Capabilities
 * \param[in]	std - the Broadcast Standard
 * \param[in]	dtype - the data type
 * \return		TRUE if the data type is compatible with the Broadcast Standard
 * \callgraph
 * \callergraph
 * \todo		add support for missing DAB data types
 */
static tBool ETAL_checkStandardDatatypeCompatibility(EtalBcastStandard std, EtalBcastDataType dtype)
{
	tBool ret;
	
	if (dtype == ETAL_DATA_TYPE_AUDIO)
	{
		ret = FALSE;
		goto exit;
	}
	switch (std)
	{
		case ETAL_BCAST_STD_DAB:
			/* TODO see function header */
			if ((dtype == ETAL_DATA_TYPE_DAB_AUDIO_RAW) ||
				(dtype == ETAL_DATA_TYPE_DATA_SERVICE) ||
				(dtype == ETAL_DATA_TYPE_DAB_DATA_RAW) ||
				(dtype == ETAL_DATA_TYPE_DAB_DLS) ||
				(dtype == ETAL_DATA_TYPE_DAB_DLPLUS) ||
				(dtype == ETAL_DATA_TYPE_DAB_FIC))
			{
				ret = TRUE;
				goto exit;
			}
			else if (dtype == ETAL_DATA_TYPE_TEXTINFO)
			{
				/* ETAL_DATA_TYPE_TEXTINFO is defined for TML spec only */
#if defined (CONFIG_ETAL_HAVE_ETALTML)
				ret = TRUE;
				goto exit;
#else
				ret = FALSE;
				goto exit;
#endif // CONFIG_ETAL_HAVE_ETALTML
			}
			else
			{
				/* Nothing to do */
			}
			break;

		case ETAL_BCAST_STD_HD_FM:
			if (dtype == ETAL_DATA_TYPE_DATA_SERVICE)
			{
				ret = TRUE;
				goto exit;
			}
			/* fall through: an HD receiver has FM capabilities also */
		case ETAL_BCAST_STD_FM:
			if (dtype == ETAL_DATA_TYPE_FM_RDS_RAW)
			{
				ret = TRUE;
				goto exit;
			}
			else if ((dtype == ETAL_DATA_TYPE_TEXTINFO) ||
					(dtype == ETAL_DATA_TYPE_FM_RDS))
			{
				/* ETAL_DATA_TYPE_TEXTINFO , ETAL_DATA_TYPE_FM_RDS  are defined for TML spec only */
#if defined (CONFIG_ETAL_HAVE_ETALTML)
				ret = TRUE;
#else
				ret = FALSE;
#endif // CONFIG_ETAL_HAVE_ETALTML
				goto exit;
			}
			else
			{
				/* Nothing to do */
			}
			break;

		case ETAL_BCAST_STD_AM:
			break;	

		case ETAL_BCAST_STD_HD_AM:
			if (dtype == ETAL_DATA_TYPE_DATA_SERVICE)
			{
				ret = TRUE;
				goto exit;
			}
#if defined (CONFIG_ETAL_HAVE_ETALTML)
			else if (dtype == ETAL_DATA_TYPE_TEXTINFO)
			{
				/* ETAL_DATA_TYPE_TEXTINFO , ETAL_DATA_TYPE_FM_RDS  are defined for TML spec only */
				ret = TRUE;
				goto exit;
			}
#endif // CONFIG_ETAL_HAVE_ETALTML
			else
			{
				// this will be return false, just break
			}
			break;

		default:
			ASSERT_ON_DEBUGGING(0);
			ret = FALSE;
			goto exit;
	}
	ret = FALSE;

exit:
	return ret;
}

/***************************
 *
 * ETAL_checkDatapathRequest
 *
 **************************/
/*!
 * \brief		Checks if a Datapath configuration contains valid data type
 * \param[in]	pDatapathAttr - the Datapath configuration to be validated
 * \return		ETAL_RET_INVALID_RECEIVER - the configuration contains an invalid Receiver
 * \return		ETAL_RET_INVALID_DATA_TYPE - the data type/Broadcast Standard combination
 * 				                             is not allowed 
 * \return		ETAL_RET_SUCCESS
 * \see			ETAL_checkStandardDatatypeCompatibility
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_checkDatapathRequest(const EtalDataPathAttr *pDatapathAttr)
{
	ETAL_HANDLE hReceiver;
	etalReceiverStatusTy *recvp;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	hReceiver = pDatapathAttr->m_receiverHandle;
	if (hReceiver == ETAL_INVALID_HANDLE ||
		!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
		goto exit;
	}
	
	recvp = ETAL_receiverGet(hReceiver);
	if (recvp == NULL)
	{
		/* will never happen, already checked above
		 * done do avoid SCC warning */
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else if (!ETAL_checkStandardDatatypeCompatibility(recvp->currentStandard, pDatapathAttr->m_dataType))
	{
		ret = ETAL_RET_INVALID_DATA_TYPE;
	}
	else
	{
		/* Nothing to do */
	}

exit:
	return ret;
}

/***************************
 *
 * ETAL_setDefaultBand
 *
 **************************/
/*!
 * \brief		Sets the default frequency band based on the Broadcast Standard
 * \details		Initializes a Receiver's frequency band based on the
 * 				Broadcast Standard
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	std - the requested Broadcast Standard
 * \param[in]	processingFeatures - the processing features, not used directly
 * 				                     in this function but passed to #ETAL_changeBandInternal
 * \param[in]	isInternalOnly - Indicate if the change band should be internal only or sent to the CMOST
 *				values TRUE / FALSE
 *			A change band is audible, in early audio case, if tuner is well configured, avoid sending a change band
 *
 * \return		ETAL_RET_ERROR - unknown Broadcast Standard, or error returned by
 * 				                 #ETAL_changeBandInternal
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_setDefaultBand(ETAL_HANDLE hGeneric, EtalBcastStandard std, EtalProcessingFeatures processingFeatures, tBool isInternalOnly)
{
	EtalFrequencyBand band;
	ETAL_STATUS ret = ETAL_RET_ERROR;

	band = ETAL_BAND_UNDEF;
	switch (std)
	{
		case ETAL_BCAST_STD_FM:
			band = ETAL_BAND_FM;
			break;

		case ETAL_BCAST_STD_AM:
			band = ETAL_BAND_AM;
			break;

		case ETAL_BCAST_STD_DAB:
			band = ETAL_BAND_DAB3;
			break;

		case ETAL_BCAST_STD_DRM:
			band = ETAL_BAND_DRM30;
			break;

		case ETAL_BCAST_STD_HD_FM:
			band = ETAL_BAND_HD;
			break;

		case ETAL_BCAST_STD_HD_AM:
			band = ETAL_BAND_MWUS;
			break;

		default:
			break;
	}
	if (band != ETAL_BAND_UNDEF)
	{
//		printf("ETAL_setDefaultBand, isInternalOnly = %d\n", isInternalOnly);
		
		if (ETAL_changeBandInternal(hGeneric, band, 0, 0, processingFeatures, isInternalOnly) == ETAL_RET_SUCCESS)
		{
			ret = ETAL_RET_SUCCESS;
		}
	}
	return ret;
}

/***************************
 *
 * ETAL_audioSelectInternal
 *
 **************************/
/*!
 * \brief		Selects the audio source on the CMOST
 * \details		Sends to the CMOST the command to select the audio
 * 				source based on the *src* parameter.
 * 				In most of the cases the CMOST audio source
 * 				selection is one-to-one with the *src* parameter.
 * \remark		For CONFIG_DIGITAL_AUDIO the function supports only
 *				#ETAL_AUDIO_SOURCE_STAR_AMFM and #ETAL_AUDIO_SOURCE_DCOP_STA660
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	src - the audio source to enable
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_NOT_IMPLEMENTED
 * \return		ETAL_RET_ERROR - communication error with the CMOST
 * \callgraph
 * \callergraph
 * \todo		The function does not use hReceiver to address the Tuner
 * 				instead it always uses Tuner 0
 */
ETAL_STATUS ETAL_audioSelectInternal(ETAL_HANDLE hReceiver, EtalAudioSourceTy src)
{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
	ETAL_STATUS ret;
	etalStarBlendingModeEnumTy starInputSrc;
	ETAL_HANDLE hTuner;
#if (defined(CONFIG_DIGITAL_AUDIO)) || (defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST))
	etalAudioIntfStatusTy intf;
#endif

	ret = ETAL_RET_SUCCESS;
	switch (src)
	{
		case ETAL_AUDIO_SOURCE_AUTO_HD:
		/*
		 * Audio selection done by CMOST via GPIO driven by 
		 * the STA680, which performs quality estimation
		 * on the FM and HD and selects the best one
		 */
			starInputSrc = ETAL_STAR_BLENDING_AUTO_HD;
			break;
		case ETAL_AUDIO_SOURCE_STAR_AMFM:
			/*
			 * On the Accordo2 Rev2 there are at most one CMOST tuner and
			 * one DCOP; the DCOP PCM output is connected to the CMOST DAC
			 * input, so selection of DAB vs AM/FM is done in the CMOST.
			 */
			starInputSrc = ETAL_STAR_BLENDING_STAR_ANALOG_AMFM;
			break;
#ifndef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
		case ETAL_AUDIO_SOURCE_DCOP_STA680:
			starInputSrc = ETAL_STAR_BLENDING_STA680_DIGITAL_AMFMHD;
			break;
		case ETAL_AUDIO_SOURCE_HD_ALIGN:
			starInputSrc = ETAL_STAR_BLENDING_HD_ALIGN;
			break;
#endif
		case ETAL_AUDIO_SOURCE_DCOP_STA660:
			starInputSrc = ETAL_STAR_BLENDING_STA660_DIGITAL_DABDRM;
			break;

		default:
			ASSERT_ON_DEBUGGING(0);
			starInputSrc = ETAL_STAR_BLENDING_STAR_ANALOG_AMFM;
			ret = ETAL_RET_NOT_IMPLEMENTED;
			break;
	}
	if (ret == ETAL_RET_SUCCESS)
	{
#if defined(CONFIG_DIGITAL_AUDIO) || defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
		hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)0);
		intf = ETAL_statusGetTunerAudioStatus(hTuner);
		if (intf.bitfield.m_dac == 0)
		{
// No need to select STAR source and setup STAR analog output
// Digital audio selection is made on Host (Accordo5)
			switch (src)
			{
				case ETAL_AUDIO_SOURCE_STAR_AMFM:
					BSP_AudioSelect(BSP_AUDIO_SELECT_RX1);
					break;
				case ETAL_AUDIO_SOURCE_DCOP_STA660:
					BSP_AudioSelect(BSP_AUDIO_SELECT_RX2);
					break;
				default:
					ret = ETAL_RET_NOT_IMPLEMENTED;
					break;
			}
		}
		else
		{
			/* TODO see function header */
			hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)0);
			if (ETAL_cmdSelectAudioSource_CMOST(hTuner, starInputSrc) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}
#else
		/* TODO see function header */
		hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)0);
		if (ETAL_cmdSelectAudioSource_CMOST(hTuner, starInputSrc) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
#endif
	}
	if (ret == ETAL_RET_SUCCESS)
	{
		ETAL_statusSetAudioSource(hReceiver, src);
	}
	return ret;
#else
	ASSERT_ON_DEBUGGING(0);
	return ETAL_RET_SUCCESS;
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
}

#ifdef CONFIG_ETAL_RECEIVER_ALIVE_PERIODIC_CHECK
/***************************
 *
 * ETAL_receiver_alive_periodic_callback
 *
 **************************/
/*!
 * \brief		Pings the devices in the system for alive check
 * \details		This function is periodically invoked by ETAL to check
 * 				if all the devices in the system are alive by sending
 * 				a Ping command.
 * 				If a device does not respond to the Ping the function
 * 				sends an #ETAL_RECEIVER_ALIVE_ERROR to the Host and then
 * 				stops itself. The event contains the index of the Receiver
 * 				instead of the complete Receiver handle because there is only
 * 				one byte for the parameter.
 * \param[in]	hGeneric - handle of the Receiver to check
 * \callgraph
 * \callergraph
 * \todo		For Receiver employing Tuner + DCOP the function only pings
 * 				the DCOP.
 */
static tVoid ETAL_receiver_alive_periodic_callback(ETAL_HANDLE hGeneric)
{
	ETAL_HANDLE hReceiver;

	if (!ETAL_handleIsReceiver(hGeneric))
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}
	hReceiver = (ETAL_HANDLE)hGeneric;

	/* This function changes the global ETAL state,
	 * might need to take the lock but it is already taken for all periodic
	 * callbacks in #ETAL_Control_ThreadEntry */

	if (!ETAL_receiverIsValidHandle(hReceiver) ||
		(ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS))
	{
		/* may happen if the Receiver was deleted */
		goto exit;
	}

	switch (ETAL_receiverGetStandard(hReceiver))
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		case ETAL_BCAST_STD_DRM:
		case ETAL_BCAST_STD_DAB:
			if (ETAL_cmdPing_MDR() != OSAL_OK)
			{
				printf("ETAL_receiver_alive_periodic_callback : ETAL_cmdPing_MDR error\n");
				ETAL_intCbDeregisterPeriodic(ETAL_receiver_alive_periodic_callback, hReceiver);
				ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_RECEIVER_ALIVE_ERROR, (tVoid *)(&hReceiver), sizeof(hReceiver));
			}
			/* TODO see function header */
			break;
#endif

#ifdef CONFIG_ETAL_SUPPORT_CMOST
		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
			if (ETAL_cmdPing_CMOST(hReceiver) != OSAL_OK)
			{
				printf("ETAL_receiver_alive_periodic_callback : ETAL_cmdPing_CMOST error\n");
				ETAL_intCbDeregisterPeriodic(ETAL_receiver_alive_periodic_callback, hReceiver);
				ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_RECEIVER_ALIVE_ERROR, (tVoid *)(&hReceiver), sizeof(hReceiver));
			}
			break;
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
			if (ETAL_cmdPing_HDRADIO() != OSAL_OK)
			{
				(void)ETAL_intCbDeregisterPeriodic(&ETAL_receiver_alive_periodic_callback, hReceiver);
				ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_RECEIVER_ALIVE_ERROR, (tVoid *)(&hReceiver), sizeof(hReceiver));
			}
			/* TODO see function header */
			break;
#endif

		default:
			(void)ETAL_intCbDeregisterPeriodic(&ETAL_receiver_alive_periodic_callback, hReceiver);
			break;
	}

	ETAL_receiverReleaseLock(hReceiver);

exit:
	return;
}
#endif // CONFIG_ETAL_RECEIVER_ALIVE_PERIODIC_CHECK


/***************************
 *
 * ETAL_startDefaultActionsForReceiver
 *
 **************************/
/*!
 * \brief		Starts default actions at Receiver creation
 * \details		Invoked only at Receiver creation, this function
 * 				starts whichever default actions are needed.
 * 				Only two actions are performed:
 * 				- if the Receiver is DAB, requests the transmission
 * 				  of PAD data (but only for the first DAB receiver
 * 				  created, to avoid 'duplicate command' error answers
 * 				  from the DCOP)
 * 				- registers the internal periodic callback for receiver
 * 				  alive check.
 * \param[in]	hReceiver - the Receiver handle
 * \return		ETAL_RET_ERROR - communication error sending the PAD command to the DCOP
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_startDefaultActionsForReceiver(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
    etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);

	// Reset DAB auto-notification
	ETAL_ResetDabAutonotification_MDR(recvp);

	// Enable by default DAB notification for DAB receiver
	if (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB)
	{
		if ((ret = ETAL_updateAutonotification(hReceiver, ETAL_AUTONOTIF_TYPE_DAB_STATUS)) != ETAL_RET_SUCCESS)
		{
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Error in starting DAB status notification");

		}
	}

	// Enable external seek event
	ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalTuneRequestInProgress, cmdActionStart);

#endif

#ifdef CONFIG_ETAL_RECEIVER_ALIVE_PERIODIC_CHECK
	/* register internal Tuner Alive periodic callback */
	if (ETAL_intCbIsRegisteredPeriodic(&ETAL_receiver_alive_periodic_callback, hReceiver) == FALSE)
	{
		ret = ETAL_intCbRegisterPeriodic(&ETAL_receiver_alive_periodic_callback, hReceiver, CONFIG_ETAL_RECEIVER_ALIVE_PERIOD);
	}
#endif

	return ret;
}

/***************************
 *
 * ETAL_stopDefaultActionsForReceiver
 *
 **************************/
/*!
 * \brief		Stops the actions started by #ETAL_startDefaultActionsForReceiver
 * \details		Invoked a Receiver destruction, this function undoes the actions
 * 				performed by the #ETAL_startDefaultActionsForReceiver.
 * 				Also it stops RDS and #ETAL_INFO_FM_STEREO event generation
 * \param[in]	hReceiver - the Receiver handle
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_stopDefaultActionsForReceiver(ETAL_HANDLE hReceiver)
{
	EtalBcastStandard std;
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
	etalReceiverStatusTy *recvp;
	tU8 subch;
#endif

	std = ETAL_receiverGetStandard(hReceiver);
	switch (std)
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		case ETAL_BCAST_STD_DAB:
			/*
			 * a failure here should not be treated as fatal
			 */
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
			if (ETAL_receiverGetSubch_MDR(hReceiver, &subch) == OSAL_OK)
			{
				if (subch < ETAL_MAX_NUM_SUBCH_PER_ENSEMBLE)
				{
					recvp = ETAL_receiverGet(hReceiver);
					if ((recvp != NULL) && (recvp->isValid))
					{
						/* send end of stream */
						if (add_api_ms_notify_channel_selection(recvp->MDRConfig.application, INTERFACEREF_AUD_OUT, 0, ETAL_MSC_MODE_UNDEFINED, subch, 0) == OSAL_OK)
						{
							(void)ETAL_receiverClearSubch_MDR(hReceiver);
						}
					}
				}
			}
#endif
			break;
#endif
		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_FM:
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			if (ETAL_receiverSupportsRDS(hReceiver))
			{
				(LINT_IGNORE_RET) ETAL_stop_RDS(hReceiver);
			}
#if defined(CONFIG_ETAL_HAVE_FMSTEREO_EVENT_CONTROL) || defined(CONFIG_ETAL_HAVE_ALL_API)
			/* stop event FM stereo if started */
			(LINT_IGNORE_RET) ETAL_event_FM_stereo_stop(hReceiver);
#endif
#endif
			break;
		default:
			break;
	}

#ifdef CONFIG_ETAL_RECEIVER_ALIVE_PERIODIC_CHECK
	/* deregister internal periodic callback */
	(void)ETAL_intCbDeregisterPeriodic(&ETAL_receiver_alive_periodic_callback, hReceiver);
#endif
}


/***************************
 *
 * ETAL_destroyReceiverInternal
 *
 **************************/
/*!
 * \brief		Destroys a Receiver
 * \details		For HDRadio, sends a #HDRADIO_SEEK_STOP_SPECIAL
 * 				to the #ETAL_tuneFSM_HDRADIO to stop it.
 * 				For DAB, it clears the DAB tune status (see
 * 				#ETAL_statusClearDABTuneStatus). In all cases
 * 				it invokes the #ETAL_stopDefaultActionsForReceiver
 * 				and then actually destroys the Receiver through the
 *				#ETAL_statusDestroyReceiver.
 * \param[in]	hReceiver - the Receiver handle
 * \return		ETAL_RET_ERROR - the Receiver was already destroyed
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_destroyReceiverInternal(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret, rethd;
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	tU8 cmd[1];
	tyHDRADIOInstanceID instanceId;
	tU32 device_list;
	EtalBcastStandard std;
	etalReceiverStatusTy *recvp;
#endif
#if  defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	tU32 freq;
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR || CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
	tBool configVpaMode;
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	ETAL_HANDLE hreceiver_main, hreceiver_second;
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

	rethd = ETAL_RET_SUCCESS;

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	std = ETAL_receiverGetStandard(hReceiver);
	if (ETAL_IS_HDRADIO_STANDARD(std))
	{
		/*
		 * ETAL_tuneFSM_HDRADIO may be in progress, ensure it is
		 * stopped
		 * The FSM acknowledges it is stopped by releasing the semaphore
		 * so we just have to try to get it and immediately release it
		 */

		if ((recvp = ETAL_receiverGet(hReceiver)) == NULL)
		{
			rethd = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			if (ETAL_receiverGetHdInstance(hReceiver, &instanceId) != OSAL_OK)
			{
				rethd = ETAL_RET_ERROR;
			}
			else
			{
				cmd[0] = (tU8)HDRADIO_SEEK_STOP_SPECIAL;
				if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) == OSAL_OK)
				{
					device_list = ETAL_cmdRoutingCheck(hReceiver, commandTune);
					if ((ETAL_CMDROUTE_TO_DCOP(device_list)) && (std == ETAL_BCAST_STD_HD_FM))
					{
						/* is receiver tuned ? */
						if (((freq = ETAL_receiverGetFrequency(hReceiver)) != ETAL_INVALID_FREQUENCY) &&
							(recvp->CMOSTConfig.processingFeatures.u.bf.m_fm_vpa != (tU8)0))
						{
							/* disable HD FM MRC */
							if (ETAL_SetMRCCnfg_HDRADIO(hReceiver, freq, FALSE) != OSAL_OK)
							{
								rethd = ETAL_RET_ERROR;
							}
						}
					}

					// reset the fsm
					(LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);
		
					if (ETAL_getTuneFSMLock_HDRADIO(instanceId) == OSAL_OK)
					{
						ETAL_releaseTuneFSMLock_HDRADIO(instanceId);
					}
					else
					{
						rethd = ETAL_RET_ERROR;
					}
				}
				else
				{
					rethd = ETAL_RET_ERROR;
				}
			}
		}
	}
#endif

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	/* if this is a DAB receiver, we should clear the DAB status */
	if (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB)
	{
		ETAL_statusClearDABTuneStatus(TRUE);

		hreceiver_main = ETAL_receiverSearchFromApplication(DABMW_MAIN_AUDIO_DAB_APP);
		hreceiver_second = ETAL_receiverSearchFromApplication(DABMW_SECONDARY_AUDIO_DAB_APP);

		// Disable default DAB notification only if this is the last active DAB receiver
		if(((hreceiver_main == ETAL_INVALID_HANDLE) || (hreceiver_main == hReceiver)) &&
		   ((hreceiver_second == ETAL_INVALID_HANDLE) || (hreceiver_second == hReceiver)))
		{
			// Enable by default DAB notification for DAB receiver
			if ((ret = ETAL_updateAutonotification(hReceiver, ETAL_AUTONOTIF_TYPE_NONE)) != ETAL_RET_SUCCESS)
			{
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Error in stopping DAB status notification");
			}

			// Disable external seek event
			ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalTuneRequestInProgress, cmdActionStop);
		}
	}
#endif

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
	/* is receiver tuned in FM VPA ? */
	if (ETAL_receiverGetFrequency(hReceiver) != ETAL_INVALID_FREQUENCY)
	{
		if ((ETAL_receiverGetConfigVPAMode(hReceiver, &configVpaMode) == ETAL_RET_SUCCESS) &&
		    (configVpaMode == TRUE))
		{
			/* disable FM VPA */
			if (ETAL_cmdSetFMProc_CMOST(hReceiver, ETAL_FM_MODE_SINGLE_TUNER) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}
	}
#endif

	/* set the FE in idle to release the FE. */
	ret = ETAL_setTunerIdleInternal(hReceiver);

	if ((ret != ETAL_RET_SUCCESS) || (rethd != ETAL_RET_SUCCESS))
	{
		ret = ETAL_RET_ERROR;
	}
	else
	{
		ETAL_intCbScheduleCallbacks(hReceiver, callAtReceiverDestroy, NULL, 0);
		ETAL_stopDefaultActionsForReceiver(hReceiver);
		ETAL_statusDestroyReceiver(hReceiver);
	}

	return ret;
}

/***************************
 *
 * ETAL_configReceiverInternal
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS ETAL_configReceiverInternal(ETAL_HANDLE *pReceiver, EtalReceiverAttr *pRecvCfg)
{
	etalReceiverStatusTy st, *recvp;
	EtalReceiverAttr ReceiverConfig, *pReceiverConfig;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;
	tBool is_reconfiguration;
	tBool is_reconfiguration_VPA;
	tBool do_change_band, isInternalOnly = FALSE;
#if defined(CONFIG_ETAL_HAVE_DEBUG_COMMANDS) || defined(CONFIG_ETAL_HAVE_ALL_API)
	EtalDebugVPAModeEnumTy debugVpaMode;
#endif
	tU32 scsr0, freqFG, freqBG;
	ETAL_HANDLE hTuner;

	is_reconfiguration = FALSE;
	/*
	 * This variable is used to avoid destroying the receiver
	 * during a reconfiguration because this impacts on the
	 * audio. In particular it is the change band sent
	 * during the receiver destruction that impacts on audio
	 */
	is_reconfiguration_VPA = FALSE;

	if ((pReceiver == NULL) || (pRecvCfg == NULL))
	{
		ret = ETAL_RET_PARAMETER_ERR;
		goto exit;
	}

	/* copy pRecvCfg into ReceiverConfig to be able to modify ReceiverConfig values */
	ReceiverConfig = *pRecvCfg;
	pReceiverConfig = &ReceiverConfig;
	/* set processing features with default values when unspecified */
	ETAL_receiverSetDefaultProcessingFeatures(pReceiverConfig->m_Standard, pReceiverConfig->m_FrontEndsSize, &(pReceiverConfig->processingFeatures));

	if (ETAL_receiverIsValidHandle(*pReceiver))
	{
		/*
		 * reconfigure an existing receiver
		 */

		 /* preliminary: check if a change of VPA mode */
		recvp = ETAL_receiverGet(*pReceiver);
		if (recvp)
		{
			if ((recvp->CMOSTConfig.processingFeatures.u.bf.m_fm_vpa != pReceiverConfig->processingFeatures.u.bf.m_fm_vpa) &&
				(((std = ETAL_receiverGetStandard(*pReceiver)) == ETAL_BCAST_STD_FM) || (std == ETAL_BCAST_STD_HD_FM)) &&
				((pReceiverConfig->m_Standard == ETAL_BCAST_STD_FM) || (pReceiverConfig->m_Standard == ETAL_BCAST_STD_HD_FM)) &&
				(pReceiverConfig->m_FrontEndsSize != recvp->diversity.m_DiversityMode))
			{
				if (ETAL_receiverFreeAllFrontend(*pReceiver) != ETAL_RET_SUCCESS)
				{
					ret = ETAL_RET_ERROR;
					goto exit;
				}
				st = *recvp;
				is_reconfiguration_VPA = TRUE;
				is_reconfiguration = TRUE;
			}
		}
		/*
		 * reconfiguration is done in two steps, here we just clean up the
		 * existing receiver
		 */
		if (is_reconfiguration_VPA == FALSE)
		{
			if (ETAL_destroyReceiverInternal(*pReceiver) == ETAL_RET_SUCCESS)
			{
				is_reconfiguration = TRUE;
			}
		}
	}

	if ((*pReceiver == ETAL_INVALID_HANDLE) || is_reconfiguration)
	{
		/* 
		 * create new receiver or reconfigure an existing one that was already cleaned
		 */

		if (is_reconfiguration_VPA == FALSE)
		{
			ETAL_receiverInitPtr(&st);
		}

		if (!ETAL_receiverHaveFreeSpace() && !is_reconfiguration)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			/*
			 * all checks on frontend, tuner and broadcast standard
			 * compatibility done here
			 */
			ret = ETAL_checkInitConfigRequest(&st, pReceiverConfig);
		}

		if (ret == ETAL_RET_SUCCESS)
		{
			std = st.currentStandard;
			st.isValid = TRUE;
			if ((std == ETAL_BCAST_STD_DAB) || (std == ETAL_BCAST_STD_DRM))
			{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
				if (ETAL_initReceiverConfig_MDR(&st) != OSAL_OK)
				{
					st.isValid = FALSE;
					ret = ETAL_RET_FRONTEND_LIST_ERR;
				}
				else
				{
					/*
					 * this is only for STA662 devices, where there is no CMOST
					 * frontend; for all other cases the tunerType will be
					 * overwritten below by ETAL_initReceiverConfig_CMOST
					 */
					st.tunerType = deviceDCOP;
				}
#else
				st.isValid = FALSE;
#endif
			}
			if (st.isValid)
			{
				if (ETAL_IS_HDRADIO_STANDARD(std))
				{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					if (ETAL_initReceiverConfig_HDRADIO(&st) != OSAL_OK)
					{
						st.isValid = FALSE;
					}
#else
					st.isValid = FALSE;
#endif
				}
#ifdef CONFIG_ETAL_SUPPORT_CMOST
				if (ETAL_initReceiverConfig_CMOST(&st) != OSAL_OK)
				{
					st.isValid = FALSE;
				}
#endif
			}
			if (st.isValid)
			{
				ETAL_receiverAddInternal(pReceiver, &st);
				if (*pReceiver == ETAL_INVALID_HANDLE)
				{
					ret = ETAL_RET_ERROR;
				}
				// Ticket STForge : #5059
				// Audio source should not be modify by a receiver configuration
				// Even in case of reconfiguration, we should let the application control the audio source
				// In any case, the audio source cannot be linked to a receiver configuration
				// because audio source may be on a different receiver/background.
				//

#if 0
				/* TODO: this part is conditioned to CONFIG_ETAL_SUPPORT_CMOST_STAR and this is
				 * fine with current A2 architecture where in any case the audio is routed
				 * through the STAR; but on different systems it could not be the same
				 * (e.g. DCOP audio routed directly to the Host)
				 */
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
				else if ((is_reconfiguration) && (is_reconfiguration_VPA == FALSE) && ETAL_receiverSupportsAudio(*pReceiver))
				{
					/*
					 * In HD mode the CMOST is configured at startup to let the DCOP
					 * decide when to do the FM to HD switch. This works fine until we
					 * change standard so we must ensure that the CMOST does not use auto mode
					 * anymore
					 */
					switch (ETAL_receiverGetStandard(*pReceiver))
					{
						/* we are in config receiver command thus an error
						 * configuring the audio interface should not be considered fatal
						 */
						case ETAL_BCAST_STD_DAB:
						case ETAL_BCAST_STD_DRM:
							(LINT_IGNORE_RET) ETAL_audioSelectInternal(*pReceiver, ETAL_AUDIO_SOURCE_DCOP_STA660);
							break;
						case ETAL_BCAST_STD_FM:
						case ETAL_BCAST_STD_AM:
							(LINT_IGNORE_RET) ETAL_audioSelectInternal(*pReceiver, ETAL_AUDIO_SOURCE_STAR_AMFM);
							break;
						case ETAL_BCAST_STD_HD_FM:
						case ETAL_BCAST_STD_HD_AM:
							(LINT_IGNORE_RET) ETAL_audioSelectInternal(*pReceiver, ETAL_AUDIO_SOURCE_AUTO_HD);
							break;
						default:
							ASSERT_ON_DEBUGGING(0);
							break;
					}
				}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
#endif
				else
				{
					/* Nothing to do */
				}
				if (ret == ETAL_RET_SUCCESS)
				{
					do_change_band = FALSE;
					if (is_reconfiguration_VPA == FALSE)
					{
						do_change_band = TRUE;

						// Early audio : check current CMOST band configuration
						// if identical
						// then avoid band change
						//
						/* get Tuner band and frequency of FG and BG channels */
							
						if (ETAL_receiverGetTunerId(*pReceiver, &hTuner) != OSAL_OK)
						{
							/* since the hReceiver was already validated above,
							 * we may get here only if CMOST support is not built in ETAL */
							ret = ETAL_RET_ERROR;
						}
						else
						{
							if (ETAL_readStatusRegister_CMOST(hTuner, &scsr0, &freqFG, &freqBG) != OSAL_OK)
							{
								
									
								ret = ETAL_RET_ERROR;
							}
							else
							{
		
								// if tuned & correct band : do not change band
								//
								//  check the right channel
								if (ETAL_CHN_FOREGROUND == st.CMOSTConfig.channel)
								{

									/*
									printf("ETAL_readStatusRegister_CMOST : scsr0 0x%x, ETAL_CMOST_SCSR0_TUNER_BAND_CHA(scsr0) %d, std, %d, ETAL_EtalStandard_To_CMOST_Band(std) %d\n",
										scsr0, ETAL_CMOST_SCSR0_TUNER_BAND_CHA(scsr0),
										std, ETAL_EtalStandard_To_CMOST_Band(std));
									*/
									
									if ((ETAL_CMOST_SCSR0_TUNED_STATUS_CHA(scsr0) == 0x000002) 
										&&
										(ETAL_EtalStandard_To_CMOST_Band(std) == ETAL_CMOST_SCSR0_TUNER_BAND_CHA(scsr0)))
									{
										// this is already correct configuration
										//
										isInternalOnly = TRUE;
										//printf("ETAL_readStatusRegister_CMOST : is internal = TRUE\n");
									}
								}
								else
								{
									/*
									printf("ETAL_readStatusRegister_CMOST : scsr0 0x%x, ETAL_CMOST_SCSR0_TUNER_BAND_CHB(scsr0) %d, std, %d, ETAL_EtalStandard_To_CMOST_Band(std) %d\n",
										scsr0, ETAL_CMOST_SCSR0_TUNER_BAND_CHB(scsr0),
										std, ETAL_EtalStandard_To_CMOST_Band(std));
									*/
									
									if ((ETAL_CMOST_SCSR0_TUNED_STATUS_CHB(scsr0) == 0x000002) 
										&&
										(ETAL_EtalStandard_To_CMOST_Band(std) == ETAL_CMOST_SCSR0_TUNER_BAND_CHB(scsr0)))
									{
										// this is already correct configuration
										//
										isInternalOnly = TRUE;
										//printf("ETAL_readStatusRegister_CMOST : is internal = TRUE\n");
									}									
								}
									
							}
						}
					}
#if defined(CONFIG_ETAL_HAVE_DEBUG_COMMANDS) || defined(CONFIG_ETAL_HAVE_ALL_API)
					if (ETAL_receiverGetDebugVPAMode(*pReceiver, &debugVpaMode) == ETAL_RET_SUCCESS)
					{
						if (debugVpaMode != ETAL_DEBUG_VPA_MODE_NONE)
						{
							do_change_band = TRUE;
						}
					}
#endif

			
					if (do_change_band)
					{
						/* issue a change band to the Tuner */
						ret = ETAL_setDefaultBand(*pReceiver, std, pReceiverConfig->processingFeatures, isInternalOnly);
					}
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
					/* TODO: should check for STAR vs DOT receiver before calling the ETAL_cmdSetFMProc_CMOST */
					else if (is_reconfiguration_VPA == TRUE)
					{
						/* configure FM VPA and HD FM MRC */
						ret = ETAL_configReceiverVPAInternal(*pReceiver, pReceiverConfig);
					}
#endif
				}
			}
			else
			{
				ret = ETAL_RET_ERROR;
			}
		}
	}
	else
	{
		ret = ETAL_RET_ERROR;
	}

	if (ret == ETAL_RET_SUCCESS)
	{
		ret = ETAL_startDefaultActionsForReceiver(*pReceiver);
	}
	else if (is_reconfiguration)
	{
		/*
		 * during a receiver reconfiguration we destroyed the
		 * receiver but then an error occurred during the creation
		 * of the new receiver, invalidate the handle
		 */
		*pReceiver = ETAL_INVALID_HANDLE;
	}
	else
	{
		/* Nothing to do */
	}

exit:
	return ret;
}

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
/***************************
 *
 * ETAL_configReceiverVPAInternal
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
static ETAL_STATUS ETAL_configReceiverVPAInternal(ETAL_HANDLE hReceiver, EtalReceiverAttr *pReceiverConfig)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU32 scsr0, freqFG, freqBG;
	etalReceiverStatusTy *recvp;
#if defined(CONFIG_ETAL_SUPPORT_CMOST) || defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	tSInt retval;
	tU32 device_list_tune;
#endif // CONFIG_ETAL_SUPPORT_CMOST || CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	ETAL_HANDLE hTuner, hFrontend;
	tBool doBGTune, doSetFmProc;
	EtalProcessingFeatures processingFeaturesBG;
#endif // CONFIG_ETAL_SUPPORT_CMOST

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp == NULL)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	if (ret == ETAL_RET_SUCCESS)
	{
#if defined(CONFIG_ETAL_SUPPORT_CMOST) || defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
		device_list_tune = ETAL_cmdRoutingCheck(hReceiver, commandTune);
#endif // CONFIG_ETAL_SUPPORT_CMOST || CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		/* get Tuner handle from receiver handle */
		if (ETAL_receiverGetTunerId(hReceiver, &hTuner) != OSAL_OK)
		{
			/* since the hReceiver was already validated above,
			 * we may get here only if CMOST support is not built in ETAL */
			ret = ETAL_RET_NOT_IMPLEMENTED;
		}
		else
		{
			/* get Tuner band and frequency of FG and BG channels */
			if (ETAL_readStatusRegister_CMOST(hTuner, &scsr0, &freqFG, &freqBG) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}
	}
	if (ret == ETAL_RET_SUCCESS)
	{
		/* get receiver frequency of FG and BG channels */
		if (pReceiverConfig->processingFeatures.u.bf.m_fm_vpa != (tU8)0)
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
			if (ETAL_CMDROUTE_TO_CMOST(device_list_tune))
			{
				doBGTune = FALSE;
				doSetFmProc = TRUE;
				/* is CMOST Backgorund channel band not in FM ? */
				if (ETAL_CMOST_SCSR0_TUNER_BAND_CHB(scsr0) != (tU32)ETAL_CMOST_TUNER_MODE_FM)
				{
					/* Change band of CMOST Background channel without Receiver handle */
					processingFeaturesBG.u.m_processing_features = (tU8)ETAL_PROCESSING_FEATURE_UNSPECIFIED;
					ETAL_receiverSetDefaultProcessingFeatures(recvp->currentStandard, (tU8)1, &processingFeaturesBG);
					hFrontend = ETAL_handleMakeFrontend(ETAL_handleTunerGetIndex(hTuner), (ETAL_HINDEX)ETAL_FE_BACKGROUND);
					ret = ETAL_setDefaultBand(hFrontend, pReceiverConfig->m_Standard, processingFeaturesBG, FALSE);
					doBGTune = TRUE;
				}
			}
#endif // CONFIG_ETAL_SUPPORT_CMOST
			if (ret == ETAL_RET_SUCCESS)
			{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
				if (ETAL_CMDROUTE_TO_CMOST(device_list_tune))
				{
					/* is CMOST Foreground channel band not in FM ? */
					if ((ETAL_CMOST_SCSR0_TUNER_BAND_CHA(scsr0) != ETAL_CMOST_TUNER_MODE_FM)
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_DA
					/* workaround for TDA7707 cut DA FW 6.3.5 issue of SCSR0 (0x20100) that can return band selected 9 reserved value */
						&& ((ETAL_CMOST_SCSR0_TUNER_BAND_CHA(scsr0) != 9) || 
						((ETAL_CMOST_SCSR0_TUNER_BAND_CHA(scsr0) == 9) && ((freqBG < ETAL_BAND_FM_MIN) || (freqBG > ETAL_BAND_FM_MAX))))
#endif
						)
					{
						/* Change band of CMOST Foreground channel with VPA Enable */
						ETAL_receiverSetDefaultProcessingFeatures(recvp->currentStandard, recvp->diversity.m_DiversityMode, &(pReceiverConfig->processingFeatures));
						ret = ETAL_setDefaultBand(hReceiver, recvp->currentStandard, pReceiverConfig->processingFeatures, FALSE);
						doSetFmProc = FALSE;
						freqFG = 0;	// force foreground tune
					}
				}
#endif // CONFIG_ETAL_SUPPORT_CMOST
			}
			if (ret == ETAL_RET_SUCCESS)
			{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
				if (ETAL_CMDROUTE_TO_CMOST(device_list_tune))
				{
					if ((ETAL_isValidFrequency(hReceiver, recvp->frequency) == TRUE) && ((freqBG != recvp->frequency) || (doBGTune)))
					{
						/* nothing to do because tune of background channel is automatically 	managed with the Set_FM_Proc command */
					}
				}
#endif // CONFIG_ETAL_SUPPORT_CMOST
			}
			if (ret == ETAL_RET_SUCCESS)
			{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
				if (ETAL_CMDROUTE_TO_CMOST(device_list_tune))
				{
					/* is Foreground channel tuned on correct frequency ? */
					if ((ETAL_isValidFrequency(hReceiver, recvp->frequency) == TRUE) && (freqFG != recvp->frequency))
					{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
						/* Change frequency of Foreground channel */
						ret = ETAL_tuneReceiverInternal(hReceiver, recvp->frequency, cmdTuneNormalResponse);
						if (ETAL_CMDROUTE_TO_DCOP(device_list_tune))
						{
							if (recvp->currentStandard == ETAL_BCAST_STD_HD_FM)
							{
								/* if retval == ETAL_RET_NO_DATA, keep returning ETAL_RET_SUCCESS in case of HDRADIO */
								if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
								{
									/* return error if not ETAL_RET_NO_DATA in case of HDRADIO */
									// it is an error
								}
								else
								{
									// we consider success : set frequency
									freqFG = recvp->frequency;
								}
							}
							else
							{
								if (ret == ETAL_RET_SUCCESS)
								{
									freqFG = recvp->frequency;
								}
							}
						}
						else
						{

							if (ret == ETAL_RET_SUCCESS)
							{
								freqFG = recvp->frequency;
							}
						}
#else
						/* Change frequency of Foreground channel */
						ret = ETAL_tuneReceiverInternal(hReceiver, recvp->frequency, cmdTuneNormalResponse);
						if (ret == ETAL_RET_SUCCESS)
						{
							freqFG = recvp->frequency;
						}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					}
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
					if (ret  == ETAL_RET_SUCCESS)
					{
						if ((doSetFmProc == TRUE) && (ETAL_isValidFrequency(hReceiver, recvp->frequency) == TRUE) &&
							(freqFG == recvp->frequency))
						{
							/* Change frequency of Background channel seamlessly with Set_FM_Proc */
							if (ETAL_cmdSetFMProc_CMOST(hReceiver, ETAL_FM_MODE_VPA) != OSAL_OK)
							{
								ret = ETAL_RET_ERROR;
							}
						}
					}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
				}
#endif // CONFIG_ETAL_SUPPORT_CMOST
			}
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			if (ret == ETAL_RET_SUCCESS)
			{
				if (ETAL_isValidFrequency(hReceiver, recvp->frequency) == TRUE)
				{
					if (ETAL_CMDROUTE_TO_DCOP(device_list_tune) &&
						(recvp->currentStandard == ETAL_BCAST_STD_HD_FM))
					{
						/* enable HD FM MRC */
						if (ETAL_SetMRCCnfg_HDRADIO(hReceiver, recvp->frequency, TRUE) != OSAL_OK)
						{
							ret = ETAL_RET_ERROR;
						}
					}
				}
			}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		}
		else
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
			if (ETAL_CMDROUTE_TO_CMOST(device_list_tune))
			{
				if (ETAL_cmdSetFMProc_CMOST(hReceiver, ETAL_FM_MODE_SINGLE_TUNER) != OSAL_OK)
				{
					ret = ETAL_RET_ERROR;
				}
			}
			if (ret == ETAL_RET_SUCCESS)
			{
				if ((ETAL_handleFrontendGetChannel(recvp->diversity.m_FeConfig[0]) == ETAL_FE_BACKGROUND) &&
				    (ETAL_isValidFrequency(hReceiver, recvp->frequency) == TRUE))
				{
					/* Tune background channel to same frequency */
					if (ETAL_CMDROUTE_TO_CMOST(device_list_tune))
					{
						retval = ETAL_cmdTune_CMOST(hReceiver, recvp->frequency);
						if (retval != OSAL_OK)
						{
							ret = ETAL_RET_ERROR;
						}
					}
				}
				else
				{
					/* CMOST Foreground channel is already tuned or no frequency tuned, nothing to do */
				}
			}
#endif // CONFIG_ETAL_SUPPORT_CMOST
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			if (ret == ETAL_RET_SUCCESS)
			{
				if (ETAL_isValidFrequency(hReceiver, recvp->frequency) == TRUE)
				{
					if (ETAL_CMDROUTE_TO_DCOP(device_list_tune) &&
						recvp->currentStandard == ETAL_BCAST_STD_HD_FM)
					{
						/* disable HD FM MRC */
						if (ETAL_SetMRCCnfg_HDRADIO(hReceiver, recvp->frequency, FALSE) != OSAL_OK)
						{
							ret = ETAL_RET_ERROR;
						}
					}
				}
			}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		}
	}

	return ret;
}
#endif //CONFIG_ETAL_SUPPORT_CMOST_STAR

/***************************
 *
 * ETAL_configDatapathInternal
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS ETAL_configDatapathInternal(ETAL_HANDLE *pDatapath, const EtalDataPathAttr *pDatapathAttr)
{
	ETAL_STATUS ret;

	if ((pDatapath == NULL) || (pDatapathAttr == NULL))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else if (*pDatapath != ETAL_INVALID_HANDLE)
	{
		// avoid overwriting an existing datapath
		ret = ETAL_RET_ERROR;
	}
	else if ((pDatapathAttr->m_sink.m_CbProcessBlock == NULL) &&
		((pDatapathAttr->m_dataType & (ETAL_DATA_TYPE_AUDIO | ETAL_DATA_TYPE_TEXTINFO)) == 0))
	{
		ret = ETAL_RET_DATAPATH_SINK_ERR;
	}
	else
	{
		/* create new datapath */
		ret = ETAL_checkDatapathRequest(pDatapathAttr);
		if (ret == ETAL_RET_SUCCESS)
		{
			*pDatapath = ETAL_receiverAddDatapath(pDatapathAttr);
			if (*pDatapath == ETAL_INVALID_HANDLE)
			{
				ret = ETAL_RET_ALREADY_USED;
			}
#if 0
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
			else if (pDatapathAttr->m_dataType == ETAL_DATA_TYPE_DAB_AUDIO_RAW)
			{
				ETAL_statusSetDatapathForRawAudio(*pDatapath);
			}
#endif
#endif
		}
	}

	return ret;
}

/***************************
 *
 * ETAL_destroyDatapathInternal
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS ETAL_destroyDatapathInternal(ETAL_HANDLE *pDatapath)
{
	ETAL_STATUS ret;

	ret = ETAL_RET_SUCCESS;
	if ((pDatapath == NULL) ||
		(*pDatapath == ETAL_INVALID_HANDLE))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (!ETAL_receiverDatapathIsValid(*pDatapath))
	{
		if (ETAL_receiverDatapathIsValidAndEmpty(*pDatapath))
		{
			/*
			 * the handle might point to a datapath that has been destroyed
			 * automatically by ETAL due e.g. to a receiver reconfiguration.
			 * In this case the ETAL internal status for the datapath is 'invalid'
			 * but the user handle is still valid. To re-align we force the
			 * handle to invalid (but don't actually destroy the datapath since
			 * it was already destroyed)
			 */
			*pDatapath = ETAL_INVALID_HANDLE;
		}
		else
		{
			ret = ETAL_RET_ERROR;
		}
	}
	else
	{
		ETAL_receiverDatapathDestroy(*pDatapath);
		*pDatapath = ETAL_INVALID_HANDLE;
	}

	return ret;
}

/***************************
 *
 * ETAL_traceConfig
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS ETAL_traceConfig(EtalTraceConfig *config)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

#if defined (CONFIG_TRACE_ENABLE)
#if defined (CONFIG_TRACE_INCLUDE_FILTERS)
	tU32 i;
#endif
	
	/* parameter check */

	if (config == NULL)
	{
		ret = ETAL_RET_PARAMETER_ERR;
		goto exit;
	}
	if (config->m_defaultLevelUsed &&
		(config->m_defaultLevel > ETAL_TR_LEVEL_ERROR) &&
		(config->m_reserved == 0))
	{
		ret = ETAL_RET_PARAMETER_ERR;
		goto exit;
	}
#if defined (CONFIG_TRACE_INCLUDE_FILTERS)
	if (config->m_filterNum > CONFIG_OSUTIL_TRACE_NUM_FILTERS)
	{
		ret = ETAL_RET_PARAMETER_ERR;
		goto exit;
	}
#endif

	/* set the default trace level */

#if !defined (CONFIG_APP_ETAL_TEST)
	/* only in the ETAL test environment allow any level to be selected */
	if (config->m_defaultLevelUsed &&
		(config->m_defaultLevel > ETAL_TR_LEVEL_SYSTEM))
	{
		ret = ETAL_RET_PARAMETER_ERR;
		goto exit;
	}
#endif

	if (true == config->m_defaultLevelUsed)
	{
		OSALUTIL_vTraceSetDefaultLevel(config->m_defaultLevel);
	}
	else
	{
		// no request for trace setting so nothing to update
		// the default log level is set at init in OSAL
	}
	
	/* disable the trace header if requested */

	if (config->m_disableHeaderUsed &&
		(config->m_disableHeader != FALSE))
	{
		OSALUTIL_vTraceDisableHeader();
	}

	/* add filters */

#if defined (CONFIG_TRACE_INCLUDE_FILTERS)
	if (config->m_filterNum != 0)
	{
		OSALUTIL_s32TraceClearFilter();
		for (i = 0; i < CONFIG_OSUTIL_TRACE_NUM_FILTERS; i++)
		{
			OSALUTIL_s32TraceSetFilterWithMask(config->m_filterClass[i], config->m_filterMask[i], config->m_filterLevel[i]);
		}
	}
#endif
	exit:
#endif // CONFIG_TRACE_ENABLE

	return ret;
}

/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * etal_config_receiver
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_config_receiver(ETAL_HANDLE *pReceiver, const EtalReceiverAttr *pReceiverConfig)
{
	ETAL_STATUS ret;
	tBool vl_lockReceiver;
	ETAL_HANDLE local_hReceiver;
	tU32 i;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_config_receiver()");
	if (pReceiverConfig != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%srec: %d, *recCon: (sta: %d FroEndSiz: %d proFea: 0x%x)", 
			(pReceiver != NULL)?"*":"", (pReceiver != NULL)?(*pReceiver):0, pReceiverConfig->m_Standard, 
			pReceiverConfig->m_FrontEndsSize, pReceiverConfig->processingFeatures.u.m_processing_features );
		for (i = 0; i < ((pReceiverConfig->m_FrontEndsSize != (tU8)0) ? (tU32)pReceiverConfig->m_FrontEndsSize : 1); i++)
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "recCon: (froEnd[%d]: 0x%x)", i, pReceiverConfig->m_FrontEnds[i]);
		}
	}
	else
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "%srec: 0x%x, recCon: %p", 
			(pReceiver != NULL)?"*":"", (pReceiver != NULL)?(*pReceiver):0, pReceiverConfig);
	}

	ret = ETAL_RET_SUCCESS;

	if ((pReceiver == NULL) || (pReceiverConfig == NULL))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		/* start real command processing */
		if (ETAL_statusGetLock() != OSAL_OK)
		{
			ret = ETAL_RET_NOT_INITIALIZED;
		}
		else
		{
			vl_lockReceiver = FALSE;
			local_hReceiver = *pReceiver;

			// get the receiver lock if needed
			// 
			if (*pReceiver != ETAL_INVALID_HANDLE)
			{
				if (ETAL_receiverIsValidHandle(*pReceiver))
				{
					// receiver exist, lock it
					//
					if ( ETAL_receiverGetLock(*pReceiver) == ETAL_RET_SUCCESS)
					{
						vl_lockReceiver = TRUE;
					}
					else
					{
						ret = ETAL_RET_ERROR;
					}
					
				}
				else
				{
					ret = ETAL_RET_INVALID_RECEIVER;
				}
			}
			else
			{
				// new receiver to be configured : no lock needed it is new 
				//
				vl_lockReceiver = FALSE;
			}

			if (ETAL_RET_SUCCESS == ret)
			{
				/*
				 * this call may invalidate the hReceiver parameter if an error occurs
				 * but we need the old handle to release the lock, so we use a local variable
				 */
				ret = ETAL_configReceiverInternal(&local_hReceiver, (EtalReceiverAttr *)pReceiverConfig);
			}

			if (TRUE == vl_lockReceiver)
			{
				ETAL_receiverReleaseLock(*pReceiver);
			}
			
			ETAL_statusReleaseLock();
			
			*pReceiver = local_hReceiver;
		}
	}

	if ((ETAL_RET_SUCCESS == ret) || (pReceiver != NULL))
	{
    	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_config_receiver(rec: %d) = %s", *pReceiver, ETAL_STATUS_toString(ret));
	}
	else
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_config_receiver() = %s",  ETAL_STATUS_toString(ret));
	}
	return ret;
}

/***************************
 *
 * etal_destroy_receiver
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_destroy_receiver(ETAL_HANDLE *pReceiver)
{
	ETAL_HANDLE hReceiver;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_destroy_receiver(%p -> %d)", pReceiver, (pReceiver != NULL)?*pReceiver:0);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (pReceiver == NULL)
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else if (!ETAL_handleIsValid(*pReceiver))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else if (!ETAL_receiverIsValidHandle(*pReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			hReceiver = *pReceiver;

			if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
			{
				ret = ETAL_RET_ERROR;
			}
			else
			{
				if (ETAL_destroyReceiverInternal(hReceiver) == ETAL_RET_SUCCESS)
				{
					*pReceiver = ETAL_INVALID_HANDLE;
				}
				else
				{
					// TODO an error here means the receiver is in an unknown state,
					// partly deleted partly not
				}

				ETAL_receiverReleaseLock(hReceiver);
			}
		}

		ETAL_statusReleaseLock();
	}
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_destroy_receiver() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * ETAL_config_datapath
 *
 **************************/
/*!
 * \brief
 * \details
 * \remark
 * \param[in]
 * \param[out]
 * \param[in,out]
 * \return
 * \see
 * \callgraph
 * \callergraph
 * \todo
 */
ETAL_STATUS ETAL_config_datapath(ETAL_HANDLE *pDatapath, const EtalDataPathAttr *pDatapathAttr)
{
	ETAL_STATUS ret;

    if ((pDatapath == NULL) || (pDatapathAttr == NULL))
    {
    	ret = ETAL_RET_PARAMETER_ERR;
    }
    else if (*pDatapath != ETAL_INVALID_HANDLE)
    {
    	// avoid overwriting an existing datapath
    	ret = ETAL_RET_ERROR;
    }
    else if ((pDatapathAttr->m_sink.m_CbProcessBlock == NULL) &&
    		((pDatapathAttr->m_dataType & (ETAL_DATA_TYPE_AUDIO | ETAL_DATA_TYPE_TEXTINFO)) == 0))
    {
    	ret = ETAL_RET_DATAPATH_SINK_ERR;
    }
    else
    {
    	/* create new datapath */
    	ret = ETAL_checkDatapathRequest(pDatapathAttr);
    	if (ret == ETAL_RET_SUCCESS)
    	{
    		*pDatapath = ETAL_receiverAddDatapath(pDatapathAttr);
    		if (*pDatapath == ETAL_INVALID_HANDLE)
    		{
    			ret = ETAL_RET_ALREADY_USED;
    		}
    	}
    }
	return ret;
}

/***************************
 *
 * etal_config_datapath
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_config_datapath(ETAL_HANDLE *pDatapath, const EtalDataPathAttr *pDatapathAttr)
{
	ETAL_STATUS ret;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_config_datapath()");
    if((pDatapath != NULL)&&(pDatapathAttr != NULL))
    {
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "datPat: %d, datPatAtt: (recHan: %d, DatTyp: 0x%x)",
                *pDatapath, pDatapathAttr->m_receiverHandle, pDatapathAttr->m_dataType);
    }

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_config_datapath(pDatapath, pDatapathAttr);
		ETAL_statusReleaseLock();
	}
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_config_datapath(datPat: 0x%x) = %s", *pDatapath, ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * ETAL_destroy_datapath
 *
 **************************/
/*!
 * \brief
 * \details
 * \remark
 * \param[in]
 * \param[out]
 * \param[in,out]
 * \return
 * \see
 * \callgraph
 * \callergraph
 * \todo
 */
ETAL_STATUS ETAL_destroy_datapath(ETAL_HANDLE *pDatapath)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if ((pDatapath == NULL) ||
		(*pDatapath == ETAL_INVALID_HANDLE))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (!ETAL_receiverDatapathIsValid(*pDatapath))
	{
		if (ETAL_receiverDatapathIsValidAndEmpty(*pDatapath))
		{
			/*
			 * the handle might point to a datapath that has been destroyed
			 * automatically by ETAL due e.g. to a receiver reconfiguration.
			 * In this case the ETAL internal status for the datapath is 'invalid'
			 * but the user handle is still valid. To re-align we force the
			 * handle to invalid (but don't actually destroy the datapath since
			 * it was already destroyed)
			 */
			*pDatapath = ETAL_INVALID_HANDLE;
		}
		else
		{
			ret = ETAL_RET_ERROR;
		}
	}
	else
	{
		ETAL_receiverDatapathDestroy(*pDatapath);
		*pDatapath = ETAL_INVALID_HANDLE;
	}
	return ret;
}


/***************************
 *
 * etal_destroy_datapath
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_destroy_datapath(ETAL_HANDLE *pDatapath)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_destroy_datapath()");
    if(pDatapath != NULL)
    {
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "datPat: 0x%x", *pDatapath);
    }

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_destroy_datapath(pDatapath);
		ETAL_statusReleaseLock();
	}
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_destroy_datapath() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

#if defined (CONFIG_ETAL_SUPPORT_CMOST)
#if defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_xtal_alignment
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_xtal_alignment(ETAL_HANDLE hReceiver, tU32 *calculatedAlignment)
{
	ETAL_HANDLE hTuner;
	tU32 paramValueArray[RDWR_PARAM_NUM * ETAL_WRITE_PARAM_ENTRY_SIZE];
	tU32 bbpX_y1High;
	tU16 resp_len;
	tF64 detRel;
	ETAL_STATUS ret;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_xtal_alignment(rec: %d)", hReceiver);

	if (calculatedAlignment == NULL)
	{
        ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		if (ETAL_statusGetLock() != OSAL_OK)
		{
			ret = ETAL_RET_NOT_INITIALIZED;
		}
		else
		{
			ret = ETAL_RET_SUCCESS;
			if (!ETAL_receiverIsValidHandle(hReceiver) ||
				(ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_FM))
			{
				ret = ETAL_RET_INVALID_RECEIVER;
			}
			if (ret == ETAL_RET_SUCCESS)
			{
				/*
				 * Set Baseband processing
				 */
				if (ETAL_cmdDebugSetBBProc_CMOST(hReceiver) != OSAL_OK)
				{
					ret = ETAL_RET_ERROR;
				}
				else if (ETAL_receiverGetTunerId(hReceiver, &hTuner) != OSAL_OK)
				{
					ret = ETAL_RET_ERROR;
				}
				else
				{
					/* Nothing to do */
			}
			}
			if (ret == ETAL_RET_SUCCESS)
			{
				/*
				 * Reset the compensation coefficient according to the following note
				 * in the CMOST spec:
				 *
				 * "Note: if the procedure is run multiple times after boot it is mandatory
				 * to clear the variable containing compensation coefficient before starting
				 * the procedure (GUI command: WD systemConfig.tuneDetCompCoeff 0)"
				 *
				 * Since this is mandatory only when executing the procedure multiple times
				 * which is normally not necessary, ignore the return value and continue
				 * in case of error.
				 */
				paramValueArray[0] = IDX_CMT_systemConfig_tuneDetCompCoeff;
				paramValueArray[1] = 0;
				(LINT_IGNORE_RET) ETAL_write_parameter_internal(hTuner, fromIndex, paramValueArray, RDWR_PARAM_NUM);

				/*
				 * Write  tunApp0.tm.outSwitch 1
				 */
				paramValueArray[0] = IDX_CMT_tunApp0_tm_outSwitch;
				paramValueArray[1] = 1;
				if (ETAL_write_parameter_internal(hTuner, fromIndex, paramValueArray, RDWR_PARAM_NUM) != ETAL_RET_SUCCESS)
				{
					ret = ETAL_RET_ERROR;
				}
			}
			if (ret == ETAL_RET_SUCCESS)
			{
				/*
				 * Write  tunApp0.tm.iqShift 2
				 */
				paramValueArray[0] = IDX_CMT_tunApp0_tm_iqShift;
				paramValueArray[1] = 2;
				if (ETAL_write_parameter_internal(hTuner, fromIndex, paramValueArray, RDWR_PARAM_NUM) != ETAL_RET_SUCCESS)
				{
					ret = ETAL_RET_ERROR;
				}
			}
			if (ret == ETAL_RET_SUCCESS)
			{
				/*
				* Tune to 89.3 MHz high side injection
				*/
				if (ETAL_cmdTuneXTAL_CMOST(hReceiver) != OSAL_OK)
				{
					ret = ETAL_RET_ERROR;
				}
			}
			if (ret == ETAL_RET_SUCCESS)
			{
				/*
				* Write bbpX.detFlags 1
				*/
				paramValueArray[0] = IDX_CMT_bbpX_detFlags;
				paramValueArray[1] = 1;
				if (ETAL_write_parameter_internal(hTuner, fromIndex, paramValueArray, RDWR_PARAM_NUM) != ETAL_RET_SUCCESS)
				{
					ret = ETAL_RET_ERROR;
				}
				else
				{
					(void)OSAL_s32ThreadWait(100);
				}
			}
			if (ret == ETAL_RET_SUCCESS)
			{
				/*
				* Read out bbpX.y1High
				*/
				paramValueArray[0] = IDX_CMT_bbpX_y1High;
				if (ETAL_read_parameter_nolock(hTuner, fromIndex, paramValueArray, RDWR_PARAM_NUM, &bbpX_y1High, &resp_len) != ETAL_RET_SUCCESS)
				{
					ret = ETAL_RET_ERROR;
				}
			}
			if (ret == ETAL_RET_SUCCESS)
			{
				detRel = (tF64) bbpX_y1High / ((tF64)36.8 * 89612500);
				if (detRel >= 0.0)
				{
					*calculatedAlignment = (tU32)round(detRel * (tF64)CONST_2_E_33);
				}
				else
				{
					*calculatedAlignment = (tU32)(CONST_2_E_24 + round(detRel * (tF64)CONST_2_E_33));
				}
				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "calAli: %d", *calculatedAlignment);
			}
			ETAL_statusReleaseLock();
		}
	}
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_xtal_alignment() = %s", ETAL_STATUS_toString(ret));
	return ret;
}
#endif // CONFIG_ETAL_HAVE_XTAL_ALIGNMENT || CONFIG_ETAL_HAVE_ALL_API
#endif // CONFIG_ETAL_SUPPORT_CMOST

/***************************
 *
 * etal_trace_config
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_trace_config(EtalTraceConfig *config)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
	ret = ETAL_traceConfig(config);
	ETAL_statusReleaseLock();
	}
	return ret;
}

