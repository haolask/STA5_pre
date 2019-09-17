//!
//!  \file 		etal_ProdTestApp.c
//!  \brief 	<i><b> ETAL application for MTD production test </b></i>
//!  \details   supports FM/DAB/HD
//!  \details   Tune, Report Quality
//!  \author 	Yann Hemon
//!

#include "osal.h"
#ifndef CONFIG_ETAL_HAVE_ETALTML
#error "CONFIG_ETAL_HAVE_ETALTML must be defined"
#endif // CONFIG_ETAL_HAVE_ETALTML
#if defined(HAVE_DAB) && !defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
#error "Check your configuration! Accordo is configured to support DAB and ETAL not"
#endif
#if defined (HAVE_HD) && !defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
#error "Check your configuration! Accordo is configured to support HD and ETAL not"
#endif

#include "etal_api.h"
#include "etaltml_api.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>  // usleep

#include "etal_ProdTestApp.h"

#ifdef HAVE_FM

/***************************
 *
 * etal_ProdTestApp_TuneFMRadio
 *
 **************************/
static ETAL_HANDLE etal_ProdTestApp_TuneFMRadio(tU32 vI_freq, EtalFrequencyBand FMBand, tBool isForeGround, tBool isAudioOn)
{
	EtalReceiverAttr attr_fm;
	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;

    EtalProcessingFeatures processingFeatures;

	EtalPathName FMPathName;
#if 0
  	EtalAudioInterfTy	vl_etalAudioInterfaceConfig;
#endif

	/*
	 * Create a receiver configuration
	 */

	if (isForeGround)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FG RADIO TUNING ***************\n");
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** BG RADIO TUNING ***************\n");
	}

	if (ETAL_BAND_FM == FMBand)
	{
		if (isForeGround)
		{
			FMPathName = ETAL_PATH_NAME_FM_FG;
		}
		else
		{
			FMPathName = ETAL_PATH_NAME_FM_BG;
		}
	}
	else if (ETAL_BAND_AM == FMBand)
	{
		if (isForeGround)
		{
			FMPathName = ETAL_PATH_NAME_AM;
		}
		else
		{
			FMPathName = ETAL_PATH_NAME_AM_BG;
		}
	}
#ifdef HAVE_HD
	else if (ETAL_BAND_HD == FMBand)
	{
		if (isForeGround)
		{
			FMPathName = ETAL_PATH_NAME_FM_HD_FG;
		}
		else
		{
			FMPathName = ETAL_PATH_NAME_FM_HD_BG;
		}
	}
#endif
	else
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "Unexpected band\n");
		return vl_handle;
	}

	ret = etaltml_getFreeReceiverForPath(&vl_handle, FMPathName, &attr_fm);
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "etalDemo_TuneFMRadio / etaltml_getFreeReceiverForPath %s ERROR\n",
			(ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneFMRadio / etaltml_getFreeReceiverForPath %s ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											(ETAL_BAND_HD==FMBand)?"HD":"FM",
											attr_fm.m_Standard, 
											attr_fm.m_FrontEndsSize,
											attr_fm.m_FrontEnds[0], attr_fm.m_FrontEnds[1]);
	
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set %s band\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");

	etalDemoPrintf(TR_LEVEL_COMPONENT, "TuneFMRadio: %s band (%x)\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", FMBand);

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, FMBand, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "etal_change_band_receiver %s ERROR", (ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ok\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");
	}
	/*
	 * Tune to an FM station
	 */
	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to freq %d\n", vI_freq);

//HD mod: better use async interface to get quicker feedback
// then we should monitor through ETAL_INFO_TUNE
#ifndef HAVE_HD
	ret = etal_tune_receiver(vl_handle, vI_freq);
#else
	if (ETAL_BAND_HD == FMBand)
	{
		//ret = etal_tune_receiver_async(vl_handle, vI_freq);
		ret = etal_tune_receiver_async(vl_handle, vI_freq);
	}
	else
	{
		ret = etal_tune_receiver(vl_handle, vI_freq);
	}
#endif
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_tune_receiver %s (%d)\n",
			(ETAL_BAND_HD==FMBand)?"HD":"FM", ret);
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver %s ok, freq = %d\n",
			(ETAL_BAND_HD==FMBand)?"HD":"FM", vI_freq);
	}

	if (isAudioOn)
	{
		if (ETAL_BAND_AM == FMBand)
		{
#if 0
			// config analog audio path
			
			vl_etalAudioInterfaceConfig.reserved = 0;
			vl_etalAudioInterfaceConfig.reserved2 = 0;

			vl_etalAudioInterfaceConfig.m_dac = true;						/* Enable / disable audio DAC */
			vl_etalAudioInterfaceConfig.m_sai_out = false;					/* Enable / disable audio SAI output : not needed in AM*/
			vl_etalAudioInterfaceConfig.m_sai_in = false;					/* Enable / disable audio SAI input :  needed for DCOP audio to be played by CMOST */

			vl_etalAudioInterfaceConfig.m_sai_slave_mode = false;			/* Enable / disable SAI Slave Mode */
		
			if ((ret = etal_config_audio_path(0, vl_etalAudioInterfaceConfig)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_ERRORS, "etal_config_audio_path DAB error" );
				return ETAL_INVALID_HANDLE;
			}
			
			system("amixer -c 3 sset Source adcauxdac > /dev/null" );
			// select the audio channel
			system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
			ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_STAR_AMFM);

		}
#ifdef HAVE_HD
		else if (ETAL_BAND_HD == FMBand)
		{
			// nothing to select
			// config should already be ok at start-up
			//
			//			etal_ProdTestApp_ConfigureAudioForTuner();
			// system("amixer -c 3 sset Source adcauxdac > /dev/null" );
			// select the audio channel
			// system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
			ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_AUTO_HD);
		}
#endif
		else
		{
			ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_STAR_AMFM);
		}
		if (ETAL_RET_SUCCESS != ret)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "etal_audio_select %s error", (ETAL_BAND_HD==FMBand)?"HD":"FM");
			return vl_handle;
		}
	}
	
	if (isForeGround)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE FM FG DONE ***************\n");
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE FM BG DONE ***************\n");
	}

	return vl_handle;
}

void etal_ProdTestApp_FMCheckQuality()
{
	int ret;
	EtalBcastQualityContainer qualfm;

	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Expected level = %ddBuV +/-%ddB\n", FM_ExpectedLevel, FM_Margin);

	if ((ret = etal_get_reception_quality(handleFG1, &qualfm)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "FG1 ERROR: etal_get_reception_quality %d\n", ret);
	}
	else
	{
		if ((qualfm.EtalQualityEntries.amfm.m_RFFieldStrength >= FM_ExpectedLevel-FM_Margin) &&
			(qualfm.EtalQualityEntries.amfm.m_RFFieldStrength <= FM_ExpectedLevel+FM_Margin))
		{
			verdictRF_FM_1A = TRUE;
		}
		else
		{
			verdictRF_FM_1A = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner1 ChA RF field strength = %ddBuV\t%s\n", \
			qualfm.EtalQualityEntries.amfm.m_RFFieldStrength, verdictRF_FM_1A?"PASSED":"FAILED");
		if ((qualfm.EtalQualityEntries.amfm.m_BBFieldStrength >= FM_ExpectedLevel-FM_Margin) &&
			(qualfm.EtalQualityEntries.amfm.m_BBFieldStrength <= FM_ExpectedLevel+FM_Margin))
		{
			verdictBB_FM_1A = TRUE;
		}
		else
		{
			verdictBB_FM_1A = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner1 ChA BB field strength = %ddBuV\t%s\n", \
			qualfm.EtalQualityEntries.amfm.m_BBFieldStrength, verdictBB_FM_1A?"PASSED":"FAILED");
	}

#ifndef HAVE_STAR_S

	if ((ret = etal_get_reception_quality(handleBG1, &qualfm)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "BG1 ERROR: etal_get_reception_quality %d\n", ret);
	}
	else
	{
		if ((qualfm.EtalQualityEntries.amfm.m_RFFieldStrength >= FM_ExpectedLevel-FM_Margin) &&
			(qualfm.EtalQualityEntries.amfm.m_RFFieldStrength <= FM_ExpectedLevel+FM_Margin))
		{
			verdictRF_FM_1B = TRUE;
		}
		else
		{
			verdictRF_FM_1B = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner1 ChB RF field strength = %ddBuV\t%s\n", \
			qualfm.EtalQualityEntries.amfm.m_RFFieldStrength, verdictRF_FM_1B?"PASSED":"FAILED");
		if ((qualfm.EtalQualityEntries.amfm.m_BBFieldStrength >= FM_ExpectedLevel-FM_Margin) &&
			(qualfm.EtalQualityEntries.amfm.m_BBFieldStrength <= FM_ExpectedLevel+FM_Margin))
		{
			verdictBB_FM_1B = TRUE;
		}
		else
		{
			verdictBB_FM_1B = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner1 ChB BB field strength = %ddBuV\t%s\n", \
			qualfm.EtalQualityEntries.amfm.m_BBFieldStrength, verdictBB_FM_1B?"PASSED":"FAILED");
	}

#ifndef HAVE_STAR_T

	if ((ret = etal_get_reception_quality(handleFG2, &qualfm)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "FG2 ERROR: etal_get_reception_quality %d\n", ret);
	}
	else
	{
		if ((qualfm.EtalQualityEntries.amfm.m_RFFieldStrength >= FM_ExpectedLevel-FM_Margin) &&
			(qualfm.EtalQualityEntries.amfm.m_RFFieldStrength <= FM_ExpectedLevel+FM_Margin))
		{
			verdictRF_FM_2A = TRUE;
		}
		else
		{
			verdictRF_FM_2A = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner2 ChA RF field strength = %ddBuV\t%s\n", \
			qualfm.EtalQualityEntries.amfm.m_RFFieldStrength, verdictRF_FM_2A?"PASSED":"FAILED");
		if ((qualfm.EtalQualityEntries.amfm.m_BBFieldStrength >= FM_ExpectedLevel-FM_Margin) &&
			(qualfm.EtalQualityEntries.amfm.m_BBFieldStrength <= FM_ExpectedLevel+FM_Margin))
		{
			verdictBB_FM_2A = TRUE;
		}
		else
		{
			verdictBB_FM_2A = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner2 ChA BB field strength = %ddBuV\t%s\n", \
			qualfm.EtalQualityEntries.amfm.m_BBFieldStrength, verdictBB_FM_2A?"PASSED":"FAILED");
	}
	if ((ret = etal_get_reception_quality(handleBG2, &qualfm)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "BG2 ERROR: etal_get_reception_quality %d\n", ret);
	}
	else
	{
		if ((qualfm.EtalQualityEntries.amfm.m_RFFieldStrength >= FM_ExpectedLevel-FM_Margin) &&
			(qualfm.EtalQualityEntries.amfm.m_RFFieldStrength <= FM_ExpectedLevel+FM_Margin))
		{
			verdictRF_FM_2B = TRUE;
		}
		else
		{
			verdictRF_FM_2B = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner2 ChB RF field strength = %ddBuV\t%s\n", \
			qualfm.EtalQualityEntries.amfm.m_RFFieldStrength, verdictRF_FM_2B?"PASSED":"FAILED");
		if ((qualfm.EtalQualityEntries.amfm.m_BBFieldStrength >= FM_ExpectedLevel-FM_Margin) &&
			(qualfm.EtalQualityEntries.amfm.m_BBFieldStrength <= FM_ExpectedLevel+FM_Margin))
		{
			verdictBB_FM_2B = TRUE;
		}
		else
		{
			verdictBB_FM_2B = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner2 ChB BB field strength = %ddBuV\t%s\n", \
			qualfm.EtalQualityEntries.amfm.m_BBFieldStrength, verdictBB_FM_2B?"PASSED":"FAILED");
	}
	
#endif // #ifndef HAVE_STAR_T

#endif // #ifndef HAVE_STAR_S

}

void etal_ProdTestApp_AMCheckQuality()
{
	int ret;
	EtalBcastQualityContainer qualam;

	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Expected level = %ddBuV +/-%ddB\n", AM_ExpectedLevel, AM_Margin);

	if ((ret = etal_get_reception_quality(handleFG1, &qualam)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "FG1 ERROR: etal_get_reception_quality %d\n", ret);
	}
	else
	{
		if ((qualam.EtalQualityEntries.amfm.m_RFFieldStrength >= AM_ExpectedLevel-AM_Margin) &&
			(qualam.EtalQualityEntries.amfm.m_RFFieldStrength <= AM_ExpectedLevel+AM_Margin))
		{
			verdictRF_AM_1A = TRUE;
		}
		else
		{
			verdictRF_AM_1A = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner1 ChA RF field strength = %ddBuV\t%s\n", \
			qualam.EtalQualityEntries.amfm.m_RFFieldStrength, verdictRF_AM_1A?"PASSED":"FAILED");
		if ((qualam.EtalQualityEntries.amfm.m_BBFieldStrength >= AM_ExpectedLevel-AM_Margin) &&
			(qualam.EtalQualityEntries.amfm.m_BBFieldStrength <= AM_ExpectedLevel+AM_Margin))
		{
			verdictBB_AM_1A = TRUE;
		}
		else
		{
			verdictBB_AM_1A = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner1 ChA BB field strength = %ddBuV\t%s\n", \
			qualam.EtalQualityEntries.amfm.m_BBFieldStrength, verdictBB_AM_1A?"PASSED":"FAILED");
	}

#ifndef HAVE_STAR_S
#ifndef HAVE_STAR_T

	if ((ret = etal_get_reception_quality(handleFG2, &qualam)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "FG2 ERROR: etal_get_reception_quality %d\n", ret);
	}
	else
	{
		if ((qualam.EtalQualityEntries.amfm.m_RFFieldStrength >= AM_ExpectedLevel-AM_Margin) &&
			(qualam.EtalQualityEntries.amfm.m_RFFieldStrength <= AM_ExpectedLevel+AM_Margin))
		{
			verdictRF_AM_2A = TRUE;
		}
		else
		{
			verdictRF_AM_2A = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner2 ChA RF field strength = %ddBuV\t%s\n", \
			qualam.EtalQualityEntries.amfm.m_RFFieldStrength, verdictRF_AM_2A?"PASSED":"FAILED");
		if ((qualam.EtalQualityEntries.amfm.m_BBFieldStrength >= AM_ExpectedLevel-AM_Margin) &&
			(qualam.EtalQualityEntries.amfm.m_BBFieldStrength <= AM_ExpectedLevel+AM_Margin))
		{
			verdictBB_AM_2A = TRUE;
		}
		else
		{
			verdictBB_AM_2A = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Tuner2 ChA BB field strength = %ddBuV\t%s\n", \
			qualam.EtalQualityEntries.amfm.m_BBFieldStrength, verdictBB_AM_2A?"PASSED":"FAILED");

	}

#endif // HAVE_STAR_T
		
#endif // HAVE_STAR_S

}
#endif // HAVE FM

void etal_ProdTestApp_CheckAudio(tBool *verdictAudio)
{
	char audioAnswer[8];

	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
		"************* Can you hear audio tone or music playing (y/n) *****************\n");
	if (NULL != fgets(audioAnswer, (int)sizeof(audioAnswer), stdin))
	{
		if ('y' == audioAnswer[0])
		{
			*verdictAudio = TRUE;
			etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
				"Test Audio: \tPASSED\n");
		}
		else
		{
			*verdictAudio = FALSE;
			etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
				"Test Audio: \tFAILED\n");
		}
	}
}

#ifdef HAVE_DAB
/***************************
 *
 * etal_ProdTestApp_TuneDABRadio
 *
 **************************/
static ETAL_HANDLE etal_ProdTestApp_TuneDABRadio(tU32 vI_freq, EtalFrequencyBand vI_band, tBool isForeGround)
{
	EtalReceiverAttr attr_dab;

	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;

    EtalProcessingFeatures processingFeatures;

	ETAL_STATUS ret;

	EtalEnsembleList ens_list;
	EtalServiceList serv_list;
	tU8 charset;
	char label[17];
	tU16 bitmap;
	unsigned int ueid;

	int Service;
#if 0
	EtalAudioInterfTy	vl_etalAudioInterfaceConfig;
#endif

	EtalPathName DABPathName;

	if (isForeGround)
	{
		DABPathName = ETAL_PATH_NAME_DAB_1;
	}
	else
	{
		DABPathName = ETAL_PATH_NAME_DAB_2;
	}

	/*
	 * Create a receiver configuration
	 */

		
	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO TUNING ***************\n");
		
	if ((ret = etaltml_getFreeReceiverForPath(&vl_handle, DABPathName, &attr_dab)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "etalDemo_TuneDABRadio / etaltml_getFreeReceiverForPath DAB ERROR\n");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDABRadio / etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr_dab.m_Standard, 
											attr_dab.m_FrontEndsSize,
											attr_dab.m_FrontEnds[0], attr_dab.m_FrontEnds[1]);
	
	}

	// set audio path
#if 0
	memset(&vl_etalAudioInterfaceConfig, 0, sizeof(EtalAudioInterfTy));
	
	vl_etalAudioInterfaceConfig.reserved = 0;
	vl_etalAudioInterfaceConfig.reserved2 = 0;
#ifdef CONFIG_DIGITAL_AUDIO
	vl_etalAudioInterfaceConfig.m_dac = false;						/* Enable / disable audio DAC */
	vl_etalAudioInterfaceConfig.m_sai_out = true;					/* Enable / disable audio SAI output : needed for DAB/DCOP Seamless*/
	vl_etalAudioInterfaceConfig.m_sai_in = false;					/* Enable / disable audio SAI input : not needed for Digital case */
#else 
	vl_etalAudioInterfaceConfig.m_dac = true;						/* Enable / disable audio DAC */
	vl_etalAudioInterfaceConfig.m_sai_out = true;					/* Enable / disable audio SAI output : needed for DAB/DCOP Seamless*/
	vl_etalAudioInterfaceConfig.m_sai_in = true;					/* Enable / disable audio SAI input : not needed for Digital case */
#endif
	
	vl_etalAudioInterfaceConfig.m_sai_slave_mode = false;			/* Enable / disable SAI Slave Mode */

	if ((ret = etal_config_audio_path(0, vl_etalAudioInterfaceConfig)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "etal_config_audio_path DAB error" );
		return ETAL_INVALID_HANDLE;
	}
#endif
	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, vI_band, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "etal_change_band_receiver DAB ERROR");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB  ok\n");
	}

	/*
	 * Tune to a DAB ensemble
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to freq %d\n", vI_freq);
	ret = etal_tune_receiver(vl_handle, vI_freq);

	if (ETAL_RET_SUCCESS == ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB ok, freq = %d\n", vI_freq);

		/*
		 * Unlike FM, for DAB we need to allow the tuner some time
		 * to capture and decode the ensemble information
		 * Without this information the tuner is unable to perform
		 * any operation on the ensemble except quality measure
		 */
		usleep(2*USLEEP_ONE_SEC);


		/*
		 * Before selecting a service we need to set up an Audio
		 * datapath
		 */

		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, first pass\n");


		/*
		 * Get the Ensemble list; this is necessary to learn the unique Ensemble ID which
		 * is required by many DAB commands. The etal_get_ensemble_list actually
		 * returns an array containing all the currently known emsembles (also
		 * on frequencies different from the current one) but for simplicity
		 * we assume there is only one entry
		 * 
		 * For a more realistic approach we should parse the list
		 * and take the entry with m_frequency equal to the currently tuned
		 * frequency
		 */
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Reading the ensemble list\n");
		memset(&ens_list, 0x00, sizeof(ens_list));
		if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_get_ensemble_list (%d)", ret);
			return vl_handle;
		}

		/*
		 * Ensembles are uniquely identified by the 'Unique EnsembleId (UEId)' which is a combination
		 * of the m_ensembleId and the m_ECC, as shown here
		 */
		ueid = ens_list.m_ensemble[0].m_ECC << 16 | ens_list.m_ensemble[0].m_ensembleId;

		/*
		 * Now that we know the UEId let's get the ensemble label; this is not required
		 * but nicer to show rather than the ensemble frequency
		 */
		if ((ret = etal_get_ensemble_data(ueid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_get_ensemble_data (%d)", ret);
			return vl_handle;
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

		/*
		 * A DAB ensemble contains one or more services: fetch the list.
		 * Some services will be audio only, others data only.
		 * The ETAL interface permits to select which service list to fetch,
		 * here we fetch only the audio services
		 */
		if ((ret = etal_get_service_list(vl_handle, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_get_service_list (%d)", ret);
			return vl_handle;
		}

		/*
		 * This is the DAB equivalent of tuning to an FM station.
		 * Audio may be available after this step, depending on the CMOST CUT:
		 * - CUT 1.0 no audio
		 * - CUT 2.x audio
		 * See above for hints on how to find out the CUT version.
		 */

		/*
		 * In this production test application, we always select the first service by default
		 */
		 
		Service = 0;
		
		if ((ret = etal_service_select_audio(vl_handle, ETAL_SERVSEL_MODE_SERVICE, ueid, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_service_select (%d)", ret);
			return vl_handle;
		}

		if ((ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "etal_audio_select DAB error" );
			return vl_handle;
		}
	}
	else if (ret == ETAL_RET_NO_DATA)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "No DAB ensemble found\n");			
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_tune_receiver DAB (%d)\n", ret);
	}

	return vl_handle;
}

void etal_ProdTestApp_DAB3CheckQuality(ETAL_HANDLE vl_handle)
{
	int ret;
	EtalBcastQualityContainer qualdab;
	char currentTunerCh[16];
	tBool *verdict;

	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Expected level = %ddBm +/-%ddB\n", DAB3_ExpectedLevel, DAB3_Margin);

	if (handleFG2 == vl_handle)
	{
		snprintf(currentTunerCh, sizeof(currentTunerCh), "Tuner2 ChA");
		verdict = &verdictRF_DAB3_2A;
	}
	else if (handleBG2 == vl_handle)
	{
		snprintf(currentTunerCh, sizeof(currentTunerCh), "Tuner2 ChB");
		verdict = &verdictRF_DAB3_2B;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "Unexpected DAB handle\n");
		return;
	}
	
	if ((ret = etal_get_reception_quality(vl_handle, &qualdab)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "%s ERROR: etal_get_reception_quality %d\n", currentTunerCh, ret);
	}
	else
	{
		if ((qualdab.EtalQualityEntries.dab.m_RFFieldStrength >= DAB3_ExpectedLevel-DAB3_Margin) &&
			(qualdab.EtalQualityEntries.dab.m_RFFieldStrength <= DAB3_ExpectedLevel+DAB3_Margin))
		{
			*verdict = TRUE;
		}
		else
		{
			*verdict = FALSE;
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "%s RF field strength = %ddBm\t%s\n", currentTunerCh, \
			qualdab.EtalQualityEntries.dab.m_RFFieldStrength, *verdict?"PASSED":"FAILED");
	}
}

void etal_ProdTestApp_DABLCheckQuality(ETAL_HANDLE vl_handle)
{
/* RF field strength quality measurement is not available for DAB-L band
 *  It results in inconsistent values, thus not meaningful for the test
 *  Do not use this code for now
 */ 
#if 0
	int ret;
	EtalBcastQualityContainer qualdab;
	char currentTunerCh[16];

	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Expected level = %ddBm +/-%ddB\n", DABL_ExpectedLevel, DABL_Margin);

	if (handleFG2 == vl_handle)
	{
		snprintf(currentTunerCh, sizeof(currentTunerCh), "Tuner2 ChA");
	}
	else if (handleBG2 == vl_handle)
	{
		snprintf(currentTunerCh, sizeof(currentTunerCh), "Tuner2 ChB");
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "Unexpected DAB handle\n");
		return;
	}
	
	if ((ret = etal_get_reception_quality(vl_handle, &qualdab)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "%s ERROR: etal_get_reception_quality %d\n", currentTunerCh, ret);
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "%s RF field strength = %ddBm\tINCONCLUSIVE\n", currentTunerCh, \
			qualdab.EtalQualityEntries.dab.m_RFFieldStrength);
	}
#endif
}

#endif // HAVE_DAB

static tVoid etal_ProdTestAppEndTest()
{
    ETAL_STATUS ret;

	/*
	  * Destroy all receivers
	  */

	if (ETAL_INVALID_HANDLE != handleFG1)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy receiver\n");
		if ((ret = etal_destroy_receiver(&handleFG1)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}
	if (ETAL_INVALID_HANDLE != handleBG1)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy receiver\n");
		if ((ret = etal_destroy_receiver(&handleBG1)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}
	if (ETAL_INVALID_HANDLE != handleFG2)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy receiver\n");
		if ((ret = etal_destroy_receiver(&handleFG2)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}
	if (ETAL_INVALID_HANDLE != handleBG2)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy receiver\n");
		if ((ret = etal_destroy_receiver(&handleBG2)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}
	return;
}

/***************************
 *
 * etalDemo_userNotificationHandler
 *
 **************************/
static void etal_ProdTestApp_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus)
{
#ifdef HAVE_HD
	char XMLstring[512];
#endif

	if (dwEvent == ETAL_INFO_TUNE)
	{
		EtalTuneStatus *status = (EtalTuneStatus *)pstatus;
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune info event, Frequency %d, good station found %d\n", 
						(int)status->m_stopFrequency, 
						(int)((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND));
		
#ifdef HAVE_HD
		etaldemo_HD_status = ETALPTA_HD_INVALID;

		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_SYNC)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "HD signal synced\n");
			etaldemo_HD_status = ETALPTA_HD_SYNC_OK;
		}

		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_SYNC_FAILURE) == ETAL_TUNESTATUS_SYNCMASK_SYNC_FAILURE)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "HD signal not synced\n");
			etaldemo_HD_status = ETALPTA_HD_SYNC_FAILED;
		}

		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED) == ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "HD digital audio failed\n");
			etaldemo_HD_status = ETALPTA_HD_AUDIO_FAILED;
		}

		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "HD digital audio acquired\n");
			etaldemo_HD_status = ETALPTA_HD_AUDIO_ACQUIRED;
		}

		if (IsDemonstrationMode)
		{
			fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC);
			lastFreqUsed = status->m_stopFrequency;
			if (etaldemo_HD_status == 0)
			{
				snprintf(XMLstring, sizeof(XMLstring),
					"<info><Band></Band><Frequency>%d</Frequency><HDStatus></HDStatus><HDServiceId></HDServiceId><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
					lastFreqUsed);
			}
			if (etaldemo_HD_status == 1)
			{
				snprintf(XMLstring, sizeof(XMLstring),
					"<info><Band></Band><Frequency>%d</Frequency><HDStatus>HD Sync</HDStatus><HDServiceId></HDServiceId><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
					lastFreqUsed);
			}
			if (etaldemo_HD_status == 2)
			{
				snprintf(XMLstring, sizeof(XMLstring),
					"<info><Band></Band><Frequency>%d</Frequency><HDStatus>HD Audio</HDStatus><HDServiceId>Service ID: %d</HDServiceId><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
					lastFreqUsed, status->m_serviceId);
			}
			write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
			close(fd_RadioText);
		}
#endif
		if (((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND)
			||
			(status->m_stopFrequency == etaldemo_seek_frequency_start)
			)
			{
			// found or end of the seek
			
			etaldemo_seek_on_going = false;
			etal_ProdTestApp_PostEvent((tU8)ETALDEMO_EVENT_TUNED_RECEIVED);
			}
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Unexpected event %d %p\n", dwEvent, context);
	}
}

void etal_ProdTestApp_readSettings(void)
{
#define DELIM "="
	FILE* fp;
	char buffer[256];
	char tmp[256];

	char *cfline;
	tBool USBrootFound = FALSE;
#if defined(HAVE_DAB) || defined(HAVE_HD)
	tBool dcopfwfileFound = FALSE;
#endif

	sprintf(buffer, "mount | grep sda | awk '{print $3}' > /tmp/usb_root");
	system(buffer);
	fp = fopen("/tmp/usb_root", "r");
	if (fp != NULL)
	{
		memset(tmp, 0, (size_t)sizeof(tmp));
		memset(buffer, 0, (size_t)sizeof(buffer));
		if (NULL != fgets(buffer, (int)sizeof(buffer), fp))
		{
			strncpy(tmp, buffer, strlen(buffer)-1);
		}
		fclose(fp);

		USBrootFound = TRUE;
		strcat(LogFile, tmp);
		strcat(LogFile, "/");
		strcat(LogFile, ETAL_PTA_LOGFILE_NAME);
	}
	else
	{
		strcat(tmp, ETAL_PTA_A5_USB_DEVICE_ROOT);
	}

	strcat(SettingsFile, tmp);
	strcat(SettingsFile, "/");
	strcat(SettingsFile, ETAL_PTA_SETTINGS_FILE);

	fp = fopen(SettingsFile, "r");

	if (fp != NULL)
	{
		while(fgets(buffer, (int)sizeof(buffer), fp) != NULL)
		{
#ifdef HAVE_DAB
			if (NULL != (cfline = strstr((char *)buffer,"dabdcopfwfile")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				strncpy(DABFWFile, cfline, strcspn(cfline, "\r\n"));
				dcopfwfileFound = TRUE;
			}
#endif
#ifdef HAVE_HD
			if (NULL != (cfline = strstr((char *)buffer,"hddcopfwfile")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				strncpy(HDFWFile, cfline, strcspn(cfline, "\r\n"));
				dcopfwfileFound = TRUE;
			}
#endif
#if defined(HAVE_DAB) || defined(HAVE_HD)
			else 
#endif
			if (NULL != (cfline = strstr((char *)buffer,"FMfreq")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				FM_Freq = (tU32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"FMexpectedlevel")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				FM_ExpectedLevel = (tS32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"FMmargin")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				FM_Margin = (tS32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"AMfreq")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				AM_Freq = (tU32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"AMexpectedlevel")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				AM_ExpectedLevel = (tS32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"AMmargin")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				AM_Margin = (tS32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"DAB3freq")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				DAB3_Freq = (tU32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"DAB3expectedlevel")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				DAB3_ExpectedLevel = (tS32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"DAB3margin")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				DAB3_Margin = (tS32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"DABLfreq")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				DABL_Freq = (tU32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"DABLexpectedlevel")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				DABL_ExpectedLevel = (tS32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"DABLmargin")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				DABL_Margin = (tS32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"FMHDfreq")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				HDFM_Freq = (tU32)atoi(tmp);
			}
			else if (NULL != (cfline = strstr((char *)buffer,"HDaudiowait")))
			{
				cfline = strstr((char *)buffer,DELIM);
				cfline = cfline + strlen(DELIM);
				memset(tmp, 0, (size_t)sizeof(tmp));
				strncpy(tmp, cfline, strlen(cfline));
				HD_AudioAcqWait = (tU8)atoi(tmp);
			}
		}
		fclose(fp);
	}
	else
	{
		printf("\n\tNo personnal settings found. Default settings are applied!\n\n");
	}

	/* Assign default values when needed */

	memset(tmp, 0, (size_t)sizeof(tmp));
	strcat(tmp, ETAL_PTA_LOCAL_LOGFILE_ROOT);
	strcat(tmp, ETAL_PTA_LOGFILE_NAME);
	if (!USBrootFound)
	{
		strcat(LogFile, tmp);
		etal_ProdTestApp_createDir(LogFile);
	}
	else
	{
		etal_ProdTestApp_createDir(LogFile);
		etal_ProdTestApp_createDir(tmp);
	}

#ifdef HAVE_DAB
	if (!dcopfwfileFound)
	{
		strcat(DABFWFile, ETAL_PTA_DCOP_FW_FILENAME);
	}
	else if (NULL != fopen(DABFWFile, "r")) //Custom file exists
	{
		fclose(fp);
	}
	else //Fall-back
	{
		printf("Unable to find custom FW %s ! In case DCOP flashing is needed, default FW will be used!\n", DABFWFile);
		memset(DABFWFile, 0, (size_t)sizeof(DABFWFile));
		strcat(DABFWFile, ETAL_PTA_DCOP_FW_FILENAME);
	}
#endif
#ifdef HAVE_HD
	if (!dcopfwfileFound)
	{
		strcat(HDFWFile, ETAL_PTA_DCOP_HD_FW_FILENAME);
	}
	else if (NULL != fopen(HDFWFile, "r")) //Custom file exists
	{
		fclose(fp);
	}
	else //Fall-back
	{
		printf("Unable to find custom FW %s ! In case DCOP flashing is needed, default FW will be used!\n", HDFWFile);
		memset(HDFWFile, 0, (size_t)sizeof(HDFWFile));
		strcat(HDFWFile, ETAL_PTA_DCOP_HD_FW_FILENAME);
	}
#endif
}

/***************************
 *
 * etal_ProdTestApp_createDir
 *
 **************************/
void etal_ProdTestApp_createDir(char* filePath)
{
	char cmd[256];
	char tmp[256];

	char *ptr;

	/* Try to create directory */
	ptr = filePath;
	while (NULL != strstr(ptr, "/"))
	{
		ptr = strstr(ptr, "/");
		ptr++;
	}
	memset(cmd, 0, (size_t)sizeof(cmd));
	memset(tmp, 0, (size_t)sizeof(tmp));
	strncpy(tmp, filePath, strlen(filePath)-strlen(ptr));
	sprintf(cmd, "mkdir -p %s", tmp);
	system(cmd);
}

/***************************
 *
 * etal_ProdTestApp_parseParameters
 *
 **************************/
int etal_ProdTestApp_parseParameters(int argc, char **argv)
{
#define ETAL_PTA_BAD_PARAM -1
#define ETAL_PTA_RETURN_OK 0


	int vl_res = ETAL_PTA_RETURN_OK;
	int i = 0;
	int argvLen;
	char bNum[8];
	char cmd[300];
	char dateTime[100];
	startTime = time(NULL);
	struct tm *t = localtime(&startTime);

	strcat(ExtLogFile, "_");
#ifdef HAVE_HD
	strcat(ExtLogFile, "HD");
#endif
#ifdef HAVE_DAB
	strcat(ExtLogFile, "DAB");
#endif

	for (i=1; i<argc; i++)
	{
		switch (argv[i][0])
		{
			case 'p':
				IsDemonstrationMode = true;
				RDS_on = false;
				sprintf(cmd, "if [ -f %s ]; then rm %s ; fi ; if [ -f %s ]; then rm %s ; fi",
					CtrlFIFO, CtrlFIFO, RadioTextFile, RadioTextFile);
				system(cmd);
				mkfifo(CtrlFIFO, 0666);
				break;
			case 'b':
				if (i+1 < argc)
				{
					strcat(ExtLogFile, "_");
					strcat(ExtLogFile, argv[i+1]);
					i++; //Skip parsing next parameter again
				}
				else
				{
					argvLen = strlen(argv[i]);
					if (1 < argvLen)
					{
						memset(bNum, 0, (size_t)sizeof(bNum));
						strncpy(bNum, argv[i]+1, argvLen - 1);
						strcat(ExtLogFile, "_");
						strcat(ExtLogFile, bNum);
					}
				}
				break;
			default:
				break;
		}
	}

	strcat(ExtLogFile, "_");
	strftime(dateTime, sizeof(dateTime)-1, "%Y%m%d%H%M", t);
	strcat(ExtLogFile, dateTime);
	strcat(LogFile, ExtLogFile);
	
	return vl_res;
}

// event creation
// Init globals
tSInt etal_ProdTestApp_initEvent()
{
	tSInt vl_res;


	vl_res = OSAL_s32EventCreate ((tCString)"EtalDemo", &etalDemo_EventHandler);

	etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_initEvent : ret %d\n", vl_res);
		
	return(vl_res);
}

tSInt etal_ProdTestApp_PostEvent(tU8 event)
{
	tSInt vl_res;

	etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_PostEvent : postEvent %d\n", event);
		
	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
                       ((tU32)0x01 << event), OSAL_EN_EVENTMASK_OR);

	return(vl_res);

}

tSInt etal_ProdTestApp_ClearEvent(tU8 event)
{
	tSInt vl_res;
	
	// Clear old event if any (this can happen after a stop)

	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
						  (~((tU32)0x01 << event)), OSAL_EN_EVENTMASK_AND);

	return(vl_res);

}

tSInt etal_ProdTestApp_ClearEventFlag(OSAL_tEventMask event)
{
	tSInt vl_res;

	// Clear old event if any (this can happen after a stop)

	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
						  (~event), OSAL_EN_EVENTMASK_AND);

	return(vl_res);
}

tSInt etal_ProdTestApp_DelEvent()
{
	tSInt vl_res;

	vl_res = OSAL_s32EventDelete ((tCString)"EtalDemo");

	etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_DelEvent : ret %d\n", vl_res);
		
	return(vl_res);
}

void etal_ProdTestApp_PrintReport(void)
{
	tBool verdictFM, verdictAM, verdictDAB3, verdictDABL, verdictHD;
	verdictFM = verdictAM = verdictDAB3 = verdictDABL = verdictHD = TRUE;

	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tFM\n");
	if (!verdictRF_FM_1A){verdictFM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA RF PASSED\n");}
	if (!verdictBB_FM_1A){verdictFM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA BB FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA BB PASSED\n");}

#ifndef HAVE_STAR_S
	if (!verdictRF_FM_1B){verdictFM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelB RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelB RF PASSED\n");}
	if (!verdictBB_FM_1B){verdictFM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelB BB FAILED\n");}	
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelB BB PASSED\n");}
	
#ifndef HAVE_STAR_T
	if (!verdictRF_FM_2A){verdictFM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA RF PASSED\n");}
	if (!verdictBB_FM_2A){verdictFM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA BB FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA BB PASSED\n");}
	if (!verdictRF_FM_2B){verdictFM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB RF PASSED\n");}
	if (!verdictBB_FM_2B){verdictFM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB BB FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB BB PASSED\n");}
#endif // HAVE_STAR_T

#endif // HAVE_STAR_S

	if (!verdictAudio_FM_1A){verdictFM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA Audio FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA Audio PASSED\n");}
	if (verdictFM) etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tPASSED!\n");

	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tAM\n");
	if (!verdictRF_AM_1A){verdictAM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA RF PASSED\n");}
	if (!verdictBB_AM_1A){verdictAM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA BB FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA BB PASSED\n");}

#ifndef HAVE_STAR_S
#ifndef HAVE_STAR_T

	if (!verdictRF_AM_2A){verdictAM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA RF PASSED\n");}
	if (!verdictBB_AM_2A){verdictAM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA BB FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA BB PASSED\n");}
#endif // HAVE_STAR_T
	
#endif // HAVE_STAR_S

	if (!verdictAudio_AM_1A){verdictAM=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA Audio FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA Audio PASSED\n");}
	if (verdictAM) etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tPASSED!\n");

#ifdef HAVE_DAB
	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tDAB3\n");
	if (!verdictRF_DAB3_2A){verdictDAB3=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA RF PASSED\n");}
	if (!verdictAudio_DAB3_2A){verdictDAB3=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA Audio FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA Audio PASSED\n");}
	if (!verdictRF_DAB3_2B){verdictDAB3=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB RF PASSED\n");}
	if (!verdictAudio_DAB3_2B){verdictDAB3=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB Audio FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB Audio PASSED\n");}
	if (verdictDAB3) etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tPASSED!\n");
	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tDABL\n");
#if 0
	if (!verdictRF_DABL_2A){verdictDABL=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA RF PASSED\n");}
	if (!verdictRF_DABL_2B){verdictDABL=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB RF FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB RF PASSED\n");}
#endif
	if (!verdictAudio_DABL_2A){verdictDABL=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA Audio FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelA Audio PASSED\n");}
	if (!verdictAudio_DABL_2B){verdictDABL=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB Audio FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner2ChannelB Audio PASSED\n");}
	if (verdictDABL) etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tPASSED!\n");
#endif
#ifdef HAVE_HD
	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tHD\n");
	if (!verdict_HD_sync_1A){verdictHD=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA HD Sync FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA HD Sync PASSED\n");}
	if (!verdictAudio_HD_sync_1A){verdictHD=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
		"\tTuner1ChannelA HD Audio Acquisition FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA HD Audio Acquisition PASSED\n");}
	if (!verdictAudio_HD_1A){verdictHD=FALSE;etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA Audio FAILED\n");}
	else{etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\tTuner1ChannelA Audio PASSED\n");}
	if (verdictHD) etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "\t\tPASSED!\n");
#endif
}

void etal_ProdTestApp_PrintElapsedTime(void)
{
	int m, s;
	tU32 t;

	endTime = time(NULL);

	t = endTime - startTime;
	
	if (t < 60)
	{
		m = 0;
		s  = (int)t;
	}
	else
	{
		m  = (int)t / 60;
		s  = (int)(t - (m * 60));
	}

	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "Elapsed Time = %02dm,%02ds\n", m, s);
}

/***************************
 *
 * main
 *
 **************************/
/*
 * Returns:
 * 1 error(s)
 * 0 success
 */
int main(int argc, char **argv)
{
	etal_ProdTestApp_readSettings();

	if (etal_ProdTestApp_parseParameters(argc, argv))
	{
		return 1;
	}

	etal_ProdTestApp_entryPoint();

	return 0;
}

// Configure the audio path for Tuner
//
static void etal_ProdTestApp_ConfigureAudioForTuner(EtalFrequencyBand vI_Band)
{
	// this is based on alsa mixer in Linux
	// This requires the appropriate audio fwk

	// ETAL audio select has been modified to support digital audio configuration,
	// so no need to care about digital audio or band choice anymore


	int ret;
	EtalAudioInterfTy audioIf;

//HD mod: in this case, DCOP680 is the I2S master, so CMOST has to be configured as SAI slave

	/* Configure audio path on CMOST */
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));

#ifdef HAVE_HD

	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif

#else	 // HAVE_HD

	audioIf.m_sai_slave_mode = false;			/* Enable / disable SAI Slave Mode */

	if (((vI_Band & ETAL_BAND_DAB_BIT) == ETAL_BAND_DAB_BIT)
		||
		((vI_Band & ETAL_BAND_FM_BIT) == ETAL_BAND_FM_BIT))
	{
		// in DAB or FM for Prod Test we want digital audio
		
#ifdef CONFIG_DIGITAL_AUDIO
		audioIf.m_dac = false;						/* Enable / disable audio DAC */
		audioIf.m_sai_out = true;					/* Enable / disable audio SAI output : needed for DAB/DCOP Seamless*/
		audioIf.m_sai_in = false;					/* Enable / disable audio SAI input : not needed for Digital case */
#else 
		audioIf.m_dac = true;						/* Enable / disable audio DAC */
		audioIf.m_sai_out = true;					/* Enable / disable audio SAI output : needed for DAB/DCOP Seamless*/
		audioIf.m_sai_in = true;					/* Enable / disable audio SAI input : not needed for Digital case */
#endif
			
	}
	else if ((vI_Band & ETAL_BAND_AM_BIT) == ETAL_BAND_AM_BIT)
	{
		// in AM for prod test we want analog audio
		audioIf.m_dac = true;						/* Enable / disable audio DAC */
		audioIf.m_sai_out = false;					/* Enable / disable audio SAI output : needed for DAB/DCOP Seamless*/
		audioIf.m_sai_in = false;					/* Enable / disable audio SAI input : not needed for Digital case */		
	}
	else
	{
		// default 
		audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
	}

#endif 

	ret = etal_config_audio_path(0, audioIf);

	if (ret != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "etal_config_audio_path (%d)", ret);
		return;
	}


	// check Dac for config

	if (TRUE == audioIf.m_dac)
	{
		// we are in analog
		
		system("amixer -c 3 sset Source adcauxdac > /dev/null" );
		// select the audio channel
		system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
	}
#ifdef CONFIG_BOARD_ACCORDO5
	if (!IsDemonstrationMode) system("amixer -c 3 sset \"Scaler Primary Media Volume Master\" 1200 > /dev/null");
#endif

	// Set the output volume 
	if (!IsDemonstrationMode) system("amixer -c 3 sset \"Volume Master\" 1200 > /dev/null");
}

//
void etal_ProdTestApp_entryPoint()
{
	int ret;
	EtalHardwareAttr init_params;
	char standard[32];
	char pause[32];
	tBool vl_continue = true;

#if defined(HAVE_DAB) || defined(HAVE_HD)
	char cmd[300];
#endif
	char XMLstring[512];

	tU32 vl_freq = 0;
	EtalFrequencyBand vl_band = ETAL_BAND_UNDEF;

#ifdef HAVE_DAB
	DABCheckQualityFunc etal_PTA_DABCheckQuality = NULL;
#endif

	/*
	 * Initialize audio path
	 */
#if defined(CONFIG_BOARD_ACCORDO5) && defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST) && (defined(CONFIG_ETAL_HAVE_SEAMLESS) || defined(CONFIG_ETAL_HAVE_ALL_API))
#if defined(CONFIG_DIGITAL_AUDIO) || defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
	// audio path selection
	system("amixer -c 3 sset Source tunerss > /dev/null" );

	// select the MSP clock
	system("amixer -c2 set Clock ext > /dev/null");
#endif
#endif

	/*
	 * Initialize ETAL
	 */
	
	lastFreqUsed = -1;
	tunedEnsemble = false;
	memset(&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = etal_ProdTestApp_userNotificationHandler;

	ret = etal_initialize(&init_params);

	if (ETAL_RET_SUCCESS != ret)
	{
		
#if defined(HAVE_DAB) || defined (HAVE_HD)
	//On first ETAL init failure, we assume that DCOP has no FW ; let's try to flash one

#ifdef HAVE_DAB
		sprintf(cmd, "./etalDcopMdrFlash -p flasher.bin %s -x", \
				DABFWFile);
#endif
#ifdef HAVE_HD
		sprintf(cmd, "./etalDcopHdrFlash -p phase1.bin %s", \
				HDFWFile);
#endif
		printf("Sending command to flash DCOP\n%s\n", cmd);
		system(cmd);

		if (etal_deinitialize() != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_deinitialize\n");
		}

		if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: 2nd failure for etal_initialize (%d)\n", ret);
			etal_ProdTestApp_exitPoint();
			return;
		}
#else // defined(HAVE_DAB) || defined (HAVE_HD)
	printf("ERROR: failure for etal_initialize (%d)\n", ret);
	etal_ProdTestApp_exitPoint();
	return;
#endif // defined(HAVE_DAB) || defined (HAVE_HD)

	}
	
	// create an event to work on notification
	//
	etal_ProdTestApp_initEvent();

	etal_ProdTestApp_ConfigureAudioForTuner(ETAL_BAND_FMEU);

	standard[0] = 'f';

	do
	{
		if (IsDemonstrationMode)
		{
			//Destroy current receiver, if any
			etal_ProdTestAppEndTest();
		}

		if (IsDemonstrationMode)
		{
			fd_CtrlFIFO = open(CtrlFIFO, O_RDONLY);
			usleep(10000);
			if (read(fd_CtrlFIFO, standard, sizeof(standard))) vl_continue = true;
			close(fd_CtrlFIFO);
		}

		if (IsDemonstrationMode)
		{
			// Create a new RadioTextfile to unblock A5 GUI
			fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC);
			snprintf(XMLstring, sizeof(XMLstring),
				"<info><Band></Band><Frequency></Frequency><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n");
			write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
			close(fd_RadioText);
		}

		switch (standard[0])
		{
			case 'f':
#ifdef HAVE_FM
				vl_band = ETAL_BAND_FM;
				vl_freq = FM_Freq;
				printf("\n\n");
				etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
					"************* GET READY FOR FM tests @%dkHz *************\n", vl_freq);
				etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
					"***************** PRESS ENTER TO CONTINUE  *****************\n");
				fgets(pause, sizeof(pause), stdin);
				standard[0] = 'a';
#else
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
				return;
#endif
				break;
			case 'a':
#ifdef HAVE_FM
				vl_band = ETAL_BAND_AM;
				vl_freq = AM_Freq;
				printf("\n\n");
				etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
					"************* GET READY FOR AM tests @%dkHz *************\n", vl_freq);
				etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
					"***************** PRESS ENTER TO CONTINUE  ****************\n");
				fgets(pause, sizeof(pause), stdin);
#if defined(HAVE_DAB) || defined (HAVE_HD)				
#ifdef HAVE_DAB
				standard[0] = 'd';
#endif
#ifdef HAVE_HD
				standard[0] = 'h';
#endif
#else // #if defined(HAVE_DAB) || defined (HAVE_HD)
				// this is the end :)
				vl_continue = false;
#endif
#else // HAVE_FM
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
				return;
#endif
				break;
			case 'h':
#ifdef HAVE_HD
				vl_band = ETAL_BAND_HD;
				vl_freq = HDFM_Freq;
				printf("\n\n");
				etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
					"************* GET READY FOR HD tests @%dkHz *************\n", vl_freq);
				etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
					"***************** PRESS ENTER TO CONTINUE  *****************\n");
				fgets(pause, sizeof(pause), stdin);
				vl_continue = false;
#else
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_HD was not activated during build\n");
				return;
#endif
				break;
			case 'd':
#ifdef HAVE_DAB
				vl_band = ETAL_BAND_DAB3;
				vl_freq = DAB3_Freq;
				etal_PTA_DABCheckQuality = etal_ProdTestApp_DAB3CheckQuality;
				printf("\n\n");
				etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
					"*********** GET READY FOR DAB3 tests @%dkHz ***********\n", vl_freq);
				standard[0] = 'l';
#else
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_DAB was not activated during build\n");
				return;
#endif
				break;
			case 'l':
#ifdef HAVE_DAB
				vl_band = ETAL_BAND_DABL;
				vl_freq = DABL_Freq;
				etal_PTA_DABCheckQuality = etal_ProdTestApp_DABLCheckQuality;
				printf("\n\n");
				etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, \
					"*********** GET READY FOR DABL tests @%dkHz ***********\n", vl_freq);
				vl_continue = false;
#else
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_DAB was not activated during build\n");
				return;
#endif
				break;
			case 'q' :
				vl_band = ETAL_BAND_UNDEF;
				vl_continue = false;
				break;
			default:
				break;
		}

		//Destroy current receiver, if any
		etal_ProdTestAppEndTest();
#ifdef HAVE_FM
		if (ETAL_BAND_FM == vl_band)
		{
			// audio already set coorectly at start up
			
			// select FE 1
			handleFG1 = etal_ProdTestApp_TuneFMRadio(vl_freq, vl_band, ETAL_PTA_FOREGROUND, ETAL_PTA_AUDIO_ON);
#ifndef HAVE_STAR_S
			// Select FE 2 (TML/BG 1)
			handleBG1 = etal_ProdTestApp_TuneFMRadio(vl_freq, vl_band, ETAL_PTA_BACKGROUND, ETAL_PTA_AUDIO_OFF);

#ifndef HAVE_STAR_T
			// from TML next BG is FE 4
			handleBG2 = etal_ProdTestApp_TuneFMRadio(vl_freq, vl_band, ETAL_PTA_BACKGROUND, ETAL_PTA_AUDIO_OFF);

			// from TML next BG is FE 3 => name FG2 here 
			handleFG2 = etal_ProdTestApp_TuneFMRadio(vl_freq, vl_band, ETAL_PTA_BACKGROUND, ETAL_PTA_AUDIO_OFF);

#endif //  ndef HAVE_STAR_T
#endif // ndef HAVE_STAR_S

			OSAL_s32ThreadWait(ETAL_PTA_CMOST_QUALITY_SETTLE_TIME);
			etal_ProdTestApp_FMCheckQuality();
			etal_ProdTestApp_CheckAudio(&verdictAudio_FM_1A);
			etal_ProdTestAppEndTest();
		}
		else if (ETAL_BAND_AM == vl_band)
		{
			// change audio configuration 1st
			etal_ProdTestApp_ConfigureAudioForTuner(ETAL_BAND_AM);
			
			handleFG1 = etal_ProdTestApp_TuneFMRadio(vl_freq, vl_band, ETAL_PTA_FOREGROUND, ETAL_PTA_AUDIO_ON);
#ifndef HAVE_STAR_S
			// Select FE 2 (TML/BG 1)
			handleBG1 = etal_ProdTestApp_TuneFMRadio(vl_freq, vl_band, ETAL_PTA_BACKGROUND, ETAL_PTA_AUDIO_OFF);


#ifndef HAVE_STAR_T

			// from TML next BG is FE 4
			handleBG2 = etal_ProdTestApp_TuneFMRadio(vl_freq, vl_band, ETAL_PTA_BACKGROUND, ETAL_PTA_AUDIO_OFF);
			// from TML next BG is FE 3
			handleFG2 = etal_ProdTestApp_TuneFMRadio(vl_freq, vl_band, ETAL_PTA_BACKGROUND, ETAL_PTA_AUDIO_OFF);

#endif // ndef HAVE_STAR_T
#endif // ndef HAVE_STAR_S

			OSAL_s32ThreadWait(ETAL_PTA_CMOST_QUALITY_SETTLE_TIME);
			etal_ProdTestApp_AMCheckQuality();
			etal_ProdTestApp_CheckAudio(&verdictAudio_AM_1A);
			etal_ProdTestAppEndTest();
		}
#endif // HAVE_FM
#ifdef HAVE_DAB
		else if ((ETAL_BAND_DAB3 == vl_band)||(ETAL_BAND_DABL == vl_band))
		{		

			// change audio configuration 1st
			etal_ProdTestApp_ConfigureAudioForTuner(ETAL_BAND_DAB3);
			
			etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "************* PRESS ENTER TO CONTINUE ON CHANNEL A *************\n");
			fgets(pause, sizeof(pause), stdin);
			handleFG2 = etal_ProdTestApp_TuneDABRadio(vl_freq, vl_band, ETAL_PTA_FOREGROUND);
			OSAL_s32ThreadWait(ETAL_PTA_CMOST_QUALITY_SETTLE_TIME);
			if (NULL != etal_PTA_DABCheckQuality) etal_PTA_DABCheckQuality(handleFG2);			
			OSAL_s32ThreadWait(2000);
			if (ETAL_BAND_DAB3 == vl_band) etal_ProdTestApp_CheckAudio(&verdictAudio_DAB3_2A);
			else etal_ProdTestApp_CheckAudio(&verdictAudio_DABL_2A);
			etal_ProdTestAppEndTest();
			
			etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "************* PRESS ENTER TO CONTINUE ON CHANNEL B *************\n");
			fgets(pause, sizeof(pause), stdin);
	
			handleBG2 = etal_ProdTestApp_TuneDABRadio(vl_freq, vl_band, ETAL_PTA_BACKGROUND);
			OSAL_s32ThreadWait(ETAL_PTA_CMOST_QUALITY_SETTLE_TIME);
			if (NULL != etal_PTA_DABCheckQuality) etal_PTA_DABCheckQuality(handleBG2);
			OSAL_s32ThreadWait(2000);
			if (ETAL_BAND_DAB3 == vl_band) etal_ProdTestApp_CheckAudio(&verdictAudio_DAB3_2B);
			else etal_ProdTestApp_CheckAudio(&verdictAudio_DABL_2B);
			etal_ProdTestAppEndTest();
		}
#endif
#ifdef HAVE_HD
		else if (ETAL_BAND_HD == vl_band)
		{
			// change audio configuration 1st
			etal_ProdTestApp_ConfigureAudioForTuner(ETAL_BAND_HD);
		
			handleFG1 = etal_ProdTestApp_TuneFMRadio(vl_freq, vl_band, ETAL_PTA_FOREGROUND, ETAL_PTA_AUDIO_ON);
			while (etaldemo_HD_status == ETALPTA_HD_INVALID)
			{
				OSAL_s32ThreadWait(1000);
			}
			if (etaldemo_HD_status == ETALPTA_HD_SYNC_FAILED)
			{
				etalDemoPrintf(TR_LEVEL_ERRORS, "HD sync FAILED\n");
				verdict_HD_sync_1A = FALSE;
				verdictAudio_HD_sync_1A = FALSE;
				verdictAudio_HD_1A = FALSE;
			}
			else
			{			
				etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "HD sync SUCCEED\n");
				verdict_HD_sync_1A = TRUE;
				while (etaldemo_HD_status < ETALPTA_HD_AUDIO_ACQUIRED)
				{
					OSAL_s32ThreadWait(1000);
				}					
				if (etaldemo_HD_status == ETALPTA_HD_AUDIO_FAILED)
				{
					etalDemoPrintf(TR_LEVEL_ERRORS, "HD audio acquisition FAILED\n");
					verdictAudio_HD_sync_1A = FALSE;
					verdictAudio_HD_1A = FALSE;
				}
				else
				{			
					etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "HD audio acquisition SUCCEED\n");
					verdictAudio_HD_sync_1A = TRUE;
					etal_ProdTestApp_CheckAudio(&verdictAudio_HD_1A);
					etal_ProdTestAppEndTest();
				}

			}
		}
#endif
		else
		{
			// not supported
		}

	}while(true == vl_continue);

	printf("\n");
	etalDemoPrintf(TR_LEVEL_SYSTEM_MIN, "************* PRESS ENTER FOR SUM-UP *************\n\n");
	fgets(pause, sizeof(pause), stdin);

	etal_ProdTestApp_PrintReport();

	printf("\n");
	etal_ProdTestApp_PrintElapsedTime();

	etal_ProdTestApp_exitPoint();
}

void etal_ProdTestApp_exitPoint()
{
	char cmd[256];
	
	// destroy handles
	etal_ProdTestAppEndTest();

	// keep a copy of logfile locally if stored on USB storage
	if (0 != strncmp(LogFile, ETAL_PTA_LOCAL_LOGFILE_ROOT, strlen(ETAL_PTA_LOCAL_LOGFILE_ROOT)))
	{
		sprintf(cmd,"cp -rf %s %s%s%s", LogFile, ETAL_PTA_LOCAL_LOGFILE_ROOT, ETAL_PTA_LOGFILE_NAME, ExtLogFile);
		system(cmd);
	}

	// make sure log file is properly written
	system("sync");

	/*
	 * Final cleanup
	 */
	
	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_deinitialize\n");
	}

	// delete event
	etal_ProdTestApp_DelEvent();

    if (IsDemonstrationMode)
    {
		unlink(CtrlFIFO);
    }
}

