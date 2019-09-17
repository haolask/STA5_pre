//!
//!  \file 		etalconfig.c
//!  \brief 	<i><b> ETAL hardware description </b></i>
//!  \details   Contains the hardware initialization functions specific to the hardware board.
//!  \details	The ETAL_Init*Interface function (typically only one of those defined in this
//!				file) is invoked during ETAL startup procedure, indirectly through the function
//!				pointer #etalBoardInitFunctionPtr. The function pointer is initialized in the
//!				hardware-specific etalconfig_*.c file (e.g. etalconfig_accordo2.c).
//!  \details	The ETAL_Init*Interface function initialize the Tuner to DCOP hardware interface(s).
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*!
 * \def		ETAL_SETHDBLENDER_AUTO_CMOST
 * 			Value for TUNER_Set_Blend CMOST (0x14) command.
 *			Configures the CMOST HW for automatic selection of analogue
 *			or digital input based on a GPIO value.
 */
#define ETAL_SETHDBLENDER_AUTO_CMOST          ((tU8)0x00)
/*!
 * 			Value for TUNER_Set_Blend CMOST (0x14) command.
 *			Configures the CMOST HW for analogue input (AM/FM)
 */
#define ETAL_SETHDBLENDER_ANALOG_CMOST        ((tU8)0x01)
/*!
 * 			Value for TUNER_Set_Blend CMOST (0x14) command.
 *			Configures the CMOST HW for digital input (AM/FM HD)
 */
#define ETAL_SETHDBLENDER_DIGITAL_CMOST       ((tU8)0x02)

#ifdef CONFIG_MODULE_DCOP_HDRADIO_CLOCK_FROM_CMOST
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA
	#define HD_RADIO_AI_MODE	(AIMode3)
#else
	#define HD_RADIO_AI_MODE	(AIMode2)
#endif
#else
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA
	#define HD_RADIO_AI_MODE	(AIMode6)
#else
	#define HD_RADIO_AI_MODE	(AIMode1)
#endif
#endif



#if defined (CONFIG_DEBUG_CONFIG_VALIDATE_CHECK)
/***************************
 *
 * ETAL_configValidate
 *
 **************************/
/*!
 * \brief		Run-time checks for configuration tables errors
 * \details		Simple sanity checks on the hardware configuration tables to catch the most common errors
 * \remark		Useful only for builds with CONFIG_DEBUG_OSAL
 * \callgraph
 * \callergraph
 */
tVoid ETAL_configValidate(tVoid)
{
	//ASSERT_ON_DEBUGGING(etalCapabilities.m_FeCount <= (tU8)ETAL_CAPA_MAX_FRONTEND);
	ASSERT_ON_DEBUGGING(sizeof(etalFrontendCommandRouting_Tune)/sizeof(EtalDeviceType) == ETAL_BCAST_STD_NUMBER);
	ASSERT_ON_DEBUGGING(sizeof(etalFrontendCommandRouting_Quality)/sizeof(EtalDeviceType) == ETAL_BCAST_STD_NUMBER);
	ASSERT_ON_DEBUGGING(sizeof(etalFrontendCommandRouting_BandSpecific)/sizeof(EtalDeviceType) == ETAL_BCAST_STD_NUMBER);
}
#endif // CONFIG_DEBUG_CONFIG_VALIDATE_CHECK

#if defined (CONFIG_ETAL_SUPPORT_CMOST)
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR) || defined (CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS)
/**************************************
 *
 * ETAL_InitCMOSTtoMDRInterface
 *
 *************************************/
/*!
 * \brief		Configures the hardware interface between CMOST Tuner and DAB DCOP
 * \details		Board initialization for the combined module:
 * 				- CMOST: 6033-388.14
 * 				- DCOP:  6033-393.14
 *
 * \details		Features:
 *				- the DCOP clock is provided by the CMOST
 *				- the DCOP PCM audio is routed back to the CMOST for Digital to Audio conversion
 *
 * \return		OSAL_OK    - initialization complete
 * \return		OSAL_ERROR - Communication error
 * \callgraph
 * \callergraph
 * \todo		Assumes that the first Tuner handle corresponds to the Tuner
 * 				connected to the DCOP. Should use the etalTuner information instead
 */
tSInt ETAL_InitCMOSTtoMDRInterface(tVoid)
{
	ETAL_HANDLE hTuner;
	ETAL_HINDEX tunerIndex;
	etalAudioIntfStatusTy vl_audioConfig;

	tunerIndex = (ETAL_HINDEX)0; // TODO see function header

	if (!ETAL_initStatusIsTunerStatusError((tU32)tunerIndex))
	{
		hTuner = ETAL_handleMakeTuner(tunerIndex);
#ifdef CONFIG_ETAL_ENABLE_CMOST_SDM_CLOCK
		if (ETAL_directCmdSetBBIf_CMOST(hTuner, BBMode2, AIMode1) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Enabling SDM interface");
			return OSAL_ERROR;
		}
#endif // CONFIG_ETAL_ENABLE_CMOST_SDM_CLOCK

		vl_audioConfig.all = (tU8)0;
#ifdef CONFIG_DIGITAL_AUDIO
		vl_audioConfig.bitfield.m_dac	  = (tU8)0; /* disable audio DAC */
#else
		vl_audioConfig.bitfield.m_dac	  = (tU8)1; /* Enable audio DAC */
#endif
		vl_audioConfig.bitfield.m_sai_out = (tU8)1; /* Enable audio SAI output */
		vl_audioConfig.bitfield.m_sai_in  = (tU8)1; /* Enable audio SAI input */
		vl_audioConfig.bitfield.m_sai_slave_mode = (tU8)0; /* 0x0: SAI is configured in Master Mode*/

		if (ETAL_directCmdSetAudioIf_CMOST(hTuner, vl_audioConfig) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "CMOST Audio input/output configuration");
			return OSAL_ERROR;
		}

#if defined (CONFIG_MODULE_INDEPENDENT)
		if (ETAL_directCmdSetGPIOforAudioInput_CMOST(hTuner) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
#endif
	}

	return OSAL_OK;
}

#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

/**************************************
 *
 * ETAL_InitCMOSTtoHDRADIOInterface
 *
 *************************************/
/*!
 * \brief		Configures the hardware interface between CMOST Tuner and HD DCOP
 * \details		Board initialization for the combined module:
 * 				- CMOST: 6033-388.14
 * 				- DCOP:  6033-379.13
 *
 * \details		Features:
 * 				- the DCOP clock is provided by the CMOST
 * 				- the DCOP PCM audio is routed back to the CMOST for Digital to Audio conversion
 * 				- audio selection (digital from DCOP vs analog from CMOST) is done based on
 * 				a signal output by the DCOP
 *
 * \return		OSAL_OK    - initialization complete
 * \return		OSAL_ERROR - Communication error
 * \callgraph
 * \callergraph
 * \todo		Assumes that the first Tuner handle corresponds to the Tuner
 * 				connected to the DCOP. Should use the etalTuner information instead
 */
tSInt ETAL_InitCMOSTtoHDRADIOInterface(tVoid)
{
	ETAL_HANDLE hTuner;
	ETAL_HINDEX tunerIndex;
	tSInt ret = OSAL_OK;
	etalAudioIntfStatusTy vl_audioConfig;

	tunerIndex = (ETAL_HINDEX)0; // TODO see function header

	if (!ETAL_initStatusIsTunerStatusError((tU32)tunerIndex))
	{
		hTuner = ETAL_handleMakeTuner(tunerIndex);

		if (ETAL_directCmdSetSAI_BB_CMOST(hTuner) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Activating CMOST 912 KHz");
			ret = OSAL_ERROR;
			goto exit;
		}

		if (ETAL_directCmdSetBBIf_CMOST(hTuner, BBMode1, HD_RADIO_AI_MODE) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Enabling SAI interface");
			ret = OSAL_ERROR;
			goto exit;
		}

		/*
		 * This is necessary for HDRadio to configure the CMOST to STA680 interface
		 */
		if (ETAL_directCmdChangeBandTune_CMOST(hTuner) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Change band or tune");
			ret = OSAL_ERROR;
			goto exit;
		}

		vl_audioConfig.all = (tU8)0;

		vl_audioConfig.bitfield.m_dac	  = (tU8)1; /* Enable audio DAC */

		vl_audioConfig.bitfield.m_sai_out = (tU8)1; /* Enable audio SAI output */
		vl_audioConfig.bitfield.m_sai_in  = (tU8)1; /* Enable audio SAI input */
		
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
		// HD RADIO is SAI master, so CMOST SAI is slave
		vl_audioConfig.bitfield.m_sai_slave_mode = (tU8)1; /* 0x1: SAI is configured in Slave Mode*/
#else
		vl_audioConfig.bitfield.m_sai_slave_mode = (tU8)0; /* 0x0: SAI is configured in Master Mode*/
#endif

		if (ETAL_directCmdSetAudioIf_CMOST(hTuner, vl_audioConfig) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "CMOST Audio input/output configuration");
			ret = OSAL_ERROR;
			goto exit;
		}
		
		if (ETAL_directCmdSetHDBlender_CMOST(hTuner, ETAL_SETHDBLENDER_AUTO_CMOST) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "CMOST Audio blender configuration");
			ret = OSAL_ERROR;
			goto exit;
		}

#if defined (CONFIG_MODULE_INDEPENDENT)
		if (ETAL_directCmdSetGPIOforAudioInput_CMOST(hTuner) != OSAL_OK)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
#endif
	}

exit:
	return ret;
}
#else // !CONFIG_ETAL_SUPPORT_DCOP_MDR && !CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/**************************************
 *
 * ETAL_InitCMOSTOnlyAudioInterface
 *
 *************************************/
/*!
 * \brief		Configures the pins for the Digital Audio interface
 * \details		Board initialization for the 
 * 				- CMOST: 6033-388.14 (STAR-T, STAR-S)
 * 				- CMOST: 6033-429.15 (STAR-S)
 * \details		The function configures the way the digital audio signals
 * 				are routed to the CMOST GPIOs. On the above boards
 * 				these signals are available on the 'DCOP' connector.
 * \return		OSAL_OK    - initialization complete
 * \return		OSAL_ERROR - Communication error
 * \callgraph
 * \callergraph
 * \todo		Assumes that the first Tuner handle corresponds to the Tuner
 * 				connected to the DCOP. Should use the etalTuner information instead
 */
tSInt ETAL_InitCMOSTOnlyAudioInterface(tVoid)
{
	ETAL_HANDLE hTuner;
	ETAL_HINDEX tunerIndex;
	etalAudioIntfStatusTy vl_audioConfig;

	tunerIndex = (ETAL_HINDEX)0; // TODO see function header

	if (!ETAL_initStatusIsTunerStatusError((tU32)tunerIndex))
	{
		hTuner = ETAL_handleMakeTuner(tunerIndex);

		// in Digital Audio, we need to configure the BB IF
		
#ifdef CONFIG_DIGITAL_AUDIO

#ifdef CONFIG_BOARD_ACCORDO2_CUSTOM2_CMOST_AUDIO_OUT_DO2
		// TEMPORARY PATCH TO SUPPORT DIFFERENT OUTPUT OF AUDIO
		// HW setup = CMOST + DCOP HD, but run in FM only.
		// This impacts the audio path routing.
		// Audio Output is expected on DO2 
		// AIMode should 6.
		// Else, default is AImode1 in FM only.

		if (ETAL_directCmdSetBBIf_CMOST(hTuner, BBMode0, AIMode6) != OSAL_OK)
#else
		if (ETAL_directCmdSetBBIf_CMOST(hTuner, BBMode0, AIMode1) != OSAL_OK)
#endif

#else
		// in analog audio, default interface AIMode 0 is assumed.
		if (ETAL_directCmdSetBBIf_CMOST(hTuner, BBMode0, AIMode0) != OSAL_OK)
#endif // #ifdef CONFIG_DIGITAL_AUDIO


		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Enabling SDM interface");
			return OSAL_ERROR;
		}


		// configure the audio interface : SAI & DAC.
		//
		vl_audioConfig.all = (tU8)0;
#ifdef CONFIG_DIGITAL_AUDIO
		vl_audioConfig.bitfield.m_dac	  = (tU8)0; /* disable audio DAC */
		vl_audioConfig.bitfield.m_sai_out = (tU8)1; /* Enable audio SAI output */
#else
		vl_audioConfig.bitfield.m_dac	  = (tU8)1; /* Enable audio DAC */
		vl_audioConfig.bitfield.m_sai_out = (tU8)0; /* disable audio SAI output */
#endif

		vl_audioConfig.bitfield.m_sai_in  = (tU8)0; /* Enable audio SAI input */
		
		vl_audioConfig.bitfield.m_sai_slave_mode = (tU8)0; /* 0x0: SAI is configured in Master Mode*/

		if (ETAL_directCmdSetAudioIf_CMOST(hTuner, vl_audioConfig) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "CMOST Audio input/output configuration");
			return OSAL_ERROR;
		}
	}

	return OSAL_OK;
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
#endif // CONFIG_ETAL_SUPPORT_CMOST

#if defined (CONFIG_MODULE_INTEGRATED)
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR) || defined (CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS)
/**************************************
 *
 * ETAL_InitCMOSTtoMDRInterface_MTD
 *
 *************************************/
/*!
 * \brief		Configures the hardware interface between CMOST Tuner and DAB DCOP on the MTD
 * \details		Board initialization for the integrated module:
 * 				- 6033-403.14
 *
 * 				The MTD includes one STAR-T for (VPA) FM, one DOT-T for DAB, one
 * 				DAB DCOP connected to the DOT.
 * \details		Features:
 * 				- one STAR-T (T1, address 0xC2) and one DOT-T (T2, address 0xC8)
 * 				- the DCOP clock is provided by T1
 * 				- the DCOP PCM audio is routed back to the T2 for Digital to Audio conversion
 *
 * \remark		This hardware module is deprecated
 * \return		OSAL_OK    - initialization complete
 * \return		OSAL_ERROR - Communication error
 * \callgraph
 * \callergraph
 */
tSInt ETAL_InitCMOSTtoMDRInterface_MTD(tVoid)
{
	// MTD may be configured with 2 STARs, so manage 1st and 2nd tuner independantly of the tuner type
	// Keep 1st star as a reference for audio
	
	ETAL_HANDLE vl_first_hTuner, vl_second_hTuner;
	tBool haveFirstTuner, haveSecondTuner;
	ETAL_HINDEX tunerIndex;
	etalAudioIntfStatusTy vl_audioConfig;

	haveFirstTuner = FALSE;

	vl_first_hTuner = ETAL_tunerGetFirstTuner_CMOST();

	if (vl_first_hTuner != ETAL_INVALID_HANDLE)
	{
		tunerIndex= ETAL_handleTunerGetIndex(vl_first_hTuner);

		if (false == ETAL_initStatusIsTunerStatusReadyToUse((tU32)tunerIndex))
		{
			/* the first STAR is present but there was an error initializing it */
		}
		else
		{
			haveFirstTuner = TRUE;
		}
	}

	haveSecondTuner = FALSE;

	vl_second_hTuner = ETAL_tunerGetSecondTuner_CMOST();
	if (vl_second_hTuner != ETAL_INVALID_HANDLE)
	{	
		tunerIndex= ETAL_handleTunerGetIndex(vl_second_hTuner);
		if (false == ETAL_initStatusIsTunerStatusReadyToUse((tU32)tunerIndex))
		{
			/* the second STAR is present but there was an error initializing it */
		}
		else
		{
			haveSecondTuner = TRUE;
		}
	}

	// 1st tuner needs to be a STAR
	if (false == ETAL_DEVICE_IS_STAR(ETAL_tunerGetType(vl_first_hTuner)))
	{
		return OSAL_ERROR;
	}

	
	// The DCOP should be clocked by the CMOST Tuners
	// the SDM interface for the 2 tuners needs to be configured
	// Tuner 1 = is the master tuner and provide the clock to the DCOP
	// Tuner 2 = provides the I&Q to DCOP and SDM interface is also needed.

	if (haveFirstTuner)
	{
		if (ETAL_directCmdSetBBIf_CMOST(vl_first_hTuner, BBMode2, AIMode1) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Enabling SDM interface on Tuner 1");
				return OSAL_ERROR;
		}
	}

	if (haveSecondTuner)
	{
		if (ETAL_directCmdSetBBIf_CMOST(vl_second_hTuner, BBMode2, AIMode1) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Enabling SDM interface on Tuner 2");
			return OSAL_ERROR;
		}
	}


	vl_audioConfig.all = (tU8)0;
#ifdef CONFIG_DIGITAL_AUDIO
	vl_audioConfig.bitfield.m_dac	  = (tU8)0; /* disable audio DAC */
#else
	vl_audioConfig.bitfield.m_dac	  = (tU8)1; /* Enable audio DAC */
#endif
	vl_audioConfig.bitfield.m_sai_out = (tU8)1; /* Enable audio SAI output */
	vl_audioConfig.bitfield.m_sai_in  = (tU8)1; /* Enable audio SAI input */
	vl_audioConfig.bitfield.m_sai_slave_mode = (tU8)0; /* 0x0: SAI is configured in Master Mode*/
			

	if (haveFirstTuner)
	{
		if (ETAL_directCmdSetAudioIf_CMOST(vl_first_hTuner, vl_audioConfig) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "CMOST Audio input/output configuration");
			return OSAL_ERROR;
		}
	}

	return OSAL_OK;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/**************************************
 *
 * ETAL_InitCMOSTtoHDRADIOInterface_MTD
 *
 *************************************/
/*!
 * \brief		Configures the hardware interface between CMOST Tuner and DAB DCOP on the MTD v2.1
 * \details		Board initialization for the integrated module:
 * 				- 6033-415.17
 *
 * 				The MTD includes one STAR-T for (VPA) FM or HD, one STAR-T for FM, one
 * 				HD DCOP connected to the first STAR. In some modified versions of 
 * 				the board the second tuner may be a STAR-S instead but the audio and
 * 				system clock routing is the same.
 * \details		Features:
 * 				- one STAR-T (T1, address 0xC2) and one STAR-T (T2, address 0xC8)
 * 				- the DCOP clock is provided by T1
 * 				- the DCOP PCM audio is routed back to the T1 for Digital to Audio conversion
 *
 * \remark		The above configuration may be changed through PCB jumpers! Check
 * 				your board and the schematics in case of problems (e.g. no audio or no
 * 				HD ping response) and customize this function
 * \return		OSAL_OK    - initialization complete
 * \return		OSAL_ERROR - Communication error
 * \callgraph
 * \callergraph
 */
tSInt ETAL_InitCMOSTtoHDRADIOInterface_MTD(tVoid)
{
	ETAL_HANDLE vl_first_hTuner;
	ETAL_HINDEX tunerIndex;
	etalAudioIntfStatusTy vl_audioConfig;

	vl_first_hTuner = ETAL_tunerGetFirstTuner_CMOST();
	if (vl_first_hTuner == ETAL_INVALID_HANDLE)
	{
		return OSAL_ERROR;
	}

	tunerIndex= ETAL_handleTunerGetIndex(vl_first_hTuner);
	if (ETAL_initStatusIsTunerStatusError((tU32)tunerIndex))
	{
		/* the first STAR is present but there was an error initializing it */
	}
	else
	{

		// 1st tuner needs to be a STAR
		if (FALSE == ETAL_DEVICE_IS_STAR(ETAL_tunerGetType(vl_first_hTuner)))
		{
			return OSAL_ERROR;
		}

		if (ETAL_directCmdSetSAI_BB_CMOST(vl_first_hTuner) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Activating CMOST 912 KHz");
			return OSAL_ERROR;
		}

		if (ETAL_directCmdSetBBIf_CMOST(vl_first_hTuner, BBMode1, HD_RADIO_AI_MODE) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Enabling SAI interface");
			return OSAL_ERROR;
		}

		/*
		 * This is necessary for HDRadio to configure the CMOST to STA680 interface
		 */
		if (ETAL_directCmdChangeBandTune_CMOST(vl_first_hTuner) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Change band or tune");
			return OSAL_ERROR;
		}


		vl_audioConfig.all = (tU8)0;

		vl_audioConfig.bitfield.m_dac	  = (tU8)1; /* Enable audio DAC */
		vl_audioConfig.bitfield.m_sai_out = (tU8)1; /* Enable audio SAI output */
		vl_audioConfig.bitfield.m_sai_in  = (tU8)1; /* Enable audio SAI input */
		
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
		// HD RADIO is SAI master, so CMOST SAI is slave
		vl_audioConfig.bitfield.m_sai_slave_mode = (tU8)1; /* 0x1: SAI is configured in Slave Mode*/
#else
		vl_audioConfig.bitfield.m_sai_slave_mode = (tU8)0; /* 0x0: SAI is configured in Master Mode*/
#endif			

		if (ETAL_directCmdSetAudioIf_CMOST(vl_first_hTuner, vl_audioConfig) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "CMOST Audio input/output configuration");
			return OSAL_ERROR;
		}
		
		if (ETAL_directCmdSetHDBlender_CMOST(vl_first_hTuner, ETAL_SETHDBLENDER_AUTO_CMOST) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "CMOST Audio blender configuration");
			return OSAL_ERROR;
		}
	}

	return OSAL_OK;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#endif // CONFIG_MODULE_INTEGRATED

/**************************************
 *
 * ETAL_getTunerIdForAudioCommands
 *
 *************************************/
/*!
 * \brief		Identifies the Tuner to which audio commands should be sent
 * \details		Normally the audio selection commands for the CMOST tuner (e.g. set_HDblender 0x14)
 * 				are issued to the same Tuner used by the ETAL Receiver.
 * 				Some hardware configurations require exceptions:
 * 				- On the MTD board (CONFIG_MODULE_INTEGRATED) the DAB uses the DOT, but the
 * 				audio is routed to the STAR device
 *
 * \param[in]	hReceiver - Receiver handle
 * \param[out]	phTuner   - pointer to a location where the function stores the handle of the Tuner
 * \return		OSAL_OK    - no errors
 * \return		OSAL_ERROR - the Receiver is invalid; the value stored in phTuner should be ignored
 * \return		OSAL_ERROR_NOT_SUPPORTED - only for Receiver involving DCOP, the audio is routed to the Host
 * \callgraph
 * \callergraph
 */
tSInt ETAL_getTunerIdForAudioCommands(ETAL_HANDLE hReceiver, ETAL_HANDLE *phTuner)
{
	tU8 tuner_index;
	etalReceiverStatusTy *recvp;
	EtalBcastStandard std;
	tSInt ret = OSAL_ERROR;

	std = ETAL_receiverGetStandard(hReceiver);
	switch (std)
	{
		case ETAL_BCAST_STD_DAB:
		case ETAL_BCAST_STD_DRM:
		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
			/*
			 * DCOP audio may be routed to a different device
			 * than the hReceiver's Tuner.
			 *
			 * For exaple on some CONFIG_MODULE_INTEGRATED boards
			 * the DAB gets the baseband from a DOT and routes the audio
			 * to the STAR so audio selection commands must be sent to the STAR,
			 * not the DOT which is part of the Receiver
			 */
			ETAL_configGetDCOPAudioTunerIndex(&tuner_index);
			if (tuner_index != ETAL_INVALID_HINDEX)
			{
				*phTuner = ETAL_handleMakeTuner(tuner_index);
				ret = OSAL_OK;
			}
			else
			{
				/*
				 * DCOP audio is not routed to a CMOST,
				 * signal to the caller with a different error return
				 */
				ret = OSAL_ERROR_NOT_SUPPORTED;
			}
			break;

		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
			recvp = ETAL_receiverGet(hReceiver);
			if (recvp != NULL)
			{
				*phTuner = recvp->CMOSTConfig.tunerId;
				ret = OSAL_OK;
			}
			else
			{
				ASSERT_ON_DEBUGGING(0);
				ret = OSAL_ERROR;
			}
			break;
			
		default:
			/* catch unhandled standard */
			ASSERT_ON_DEBUGGING(0);
	}
	return ret;
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/**************************************
 *
 * ETAL_getDeviceDescription_HDRADIO
 *
 *************************************/
/*!
 * \brief		Returns the device description of the HDRadio DCOP
 * \remark		The address is an 8-bit address for I/O bus addressing, not a memory address
 * \param[out]	deviceDescription - pointer to the device description
 * \callgraph
 * \callergraph
 */
tVoid ETAL_getDeviceDescription_HDRADIO(EtalDeviceDesc *deviceDescription)
{
	*deviceDescription = etalDCOP.deviceDescr;
}

/**************************************
 *
 * ETAL_getDeviceConfig_HDRADIO
 *
 *************************************/
/*!
 * \brief		Returns the device configuration of HDRADIO DCOP
 * \param[out]	deviceConfiguration - pointer to device configuration
 *
 * \callgraph
 * \callergraph
 */
tVoid ETAL_getDeviceConfig_HDRADIO(tyHDRADIODeviceConfiguration *deviceConfiguration)
{
	EtalDeviceDesc deviceDescription;

	ETAL_getDeviceDescription_HDRADIO(&deviceDescription);

	deviceConfiguration->GPIO_RESET = HDRADIO_ACCORDO2_RESET_GPIO;
	deviceConfiguration->communicationBusType = (deviceDescription.m_busType == ETAL_BusI2C) ? BusI2C : BusSPI;

	if (deviceConfiguration->communicationBusType == BusI2C)
	{
#ifdef CONFIG_COMM_HDRADIO_I2C
		strncpy(deviceConfiguration->communicationBus.i2c.busName, (tChar *)HDRADIO_ACCORDO2_I2C_DEVICE, MAX_SIZE_BUS_NAME);
		deviceConfiguration->communicationBus.i2c.deviceAddress = deviceDescription.m_busAddress;
#endif
	}
	else
	{
#ifdef CONFIG_COMM_HDRADIO_SPI
		strncpy(deviceConfiguration->communicationBus.spi.busName, (tChar *)HDRADIO_ACCORDO2_SPI_DEVICE, MAX_SIZE_BUS_NAME);
		deviceConfiguration->communicationBus.spi.GPIO_CS = HDRADIO_ACCORDO2_CS_GPIO;
		deviceConfiguration->communicationBus.spi.mode = HDRADIO_ACCORDO2_SPI_MODE;
		deviceConfiguration->communicationBus.spi.speed = HDRADIO_ACCORDO2_SPI_SPEED;
#endif
	}
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
/**************************************
 *
 * ETAL_getDeviceDescription_DAB
 *
 *************************************/
/*!
 * \brief       Returns the device description of the DAB DCOP
 * \remark      The address is an 8-bit address for I/O bus addressing, not a memory address
 * \param[out]  deviceDescription - pointer to the device description
 * \callgraph
 * \callergraph
 */
tVoid ETAL_getDeviceDescription_DAB(EtalDeviceDesc *deviceDescription)
{
    *deviceDescription = etalDCOP.deviceDescr;
}

/**************************************
 *
 * ETAL_getDeviceConfig_DAB
 *
 *************************************/
/*!
 * \brief       Returns the device configuration of DAB DCOP
 * \param[out]  deviceConfiguration - pointer to device configuration
 *
 * \callgraph
 * \callergraph
 */
tVoid ETAL_getDeviceConfig_DAB(tyDABDeviceConfiguration *deviceConfiguration)
{
    EtalDeviceDesc deviceDescription;

    ETAL_getDeviceDescription_DAB(&deviceDescription);

    deviceConfiguration->GPIO_RESET = DAB_ACCORDO2_RESET_GPIO;
    deviceConfiguration->GPIO_REQ = DAB_ACCORDO2_REQ_GPIO;
    deviceConfiguration->GPIO_BOOT = DAB_ACCORDO2_BOOT_GPIO;
    deviceConfiguration->isBootMode = DAB_ACCORDO2_IS_BOOT_MODE;

    deviceConfiguration->communicationBusType = (deviceDescription.m_busType == ETAL_BusI2C) ? BusI2C : BusSPI;

    if (deviceConfiguration->communicationBusType == BusI2C)
    {
        strncpy(deviceConfiguration->communicationBus.i2c.busName, (tChar *)"Not supported", MAX_SIZE_BUS_NAME);
        deviceConfiguration->communicationBus.i2c.deviceAddress = 0;
    }
    else
    {
        strncpy(deviceConfiguration->communicationBus.spi.busName, (tChar *)DAB_ACCORDO2_SPI_DEVICE, MAX_SIZE_BUS_NAME);
        deviceConfiguration->communicationBus.spi.GPIO_CS = DAB_ACCORDO2_CS_GPIO;
        deviceConfiguration->communicationBus.spi.mode = DAB_ACCORDO2_SPI_MODE;
        deviceConfiguration->communicationBus.spi.speed = DAB_ACCORDO2_SPI_SPEED_NORMAL_MODE;
    }
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR


/**************************************
 *
 * ETAL_configGetDCOPAudioTunerIndex
 *
 *************************************/
/*!
 * \brief		Returns the tuner index of the Tuner connected to the DCOP audio out
 * \remark		The tuner index is not an ETAL_HANDLE
 * \param[out]	tuner_index - pointer to a location where the function stores the tuner index
 * \callgraph
 * \callergraph
 */
tVoid ETAL_configGetDCOPAudioTunerIndex(tU8 *tuner_index)
{
	*tuner_index = etalDCOP.tunerIndexForAudioCommands;
}

