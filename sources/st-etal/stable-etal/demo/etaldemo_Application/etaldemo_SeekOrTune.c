//!
//!  \file 		etaldemo_SeekOrTune.c
//!  \brief 	<i><b> ETAL application demo for simple seek and tune </b></i>
//!  \details   supports FM and DAB
//!  \details   Tune, Seek, Scan
//!  \details   supports Seamless Switching
//!  \author 	Erwan Preteseille Yann Hemon
//!

#include "target_config.h"

#include "osal.h"
#ifndef CONFIG_ETAL_HAVE_ETALTML
#error "CONFIG_ETAL_HAVE_ETALTML must be defined"
#else
#if defined(CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING) && (!defined(CONFIG_ETAL_HAVE_SEAMLESS)&&!defined(CONFIG_ETAL_HAVE_ALL_API))
#error "CONFIG_ETAL_HAVE_SEAMLESS must be defined too"
#endif
#endif // CONFIG_ETAL_HAVE_ETALTML
#if defined(HAVE_DAB) && !defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
#error "Check your configuration! Accordo is configured to support DAB and ETAL not"
#endif
#if defined (HAVE_HD) && !defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
#error "Check your configuration! Accordo is configured to support HD and ETAL not"
#endif

#include "etal_api.h"
#include "etaltml_api.h"

#ifdef CONFIG_HOST_OS_WIN32
	#include <windows.h>
#else
	#include <unistd.h>  // usleep
#endif

#include "etaldemo_SeekOrTune.h"

// Debug specific switch
#undef ETAL_DEMO_CREATE_FIC_DATAPATH


/***************************
 *
 * standard2Ascii
 *
 **************************/
static char *standard2Ascii(EtalBcastStandard std)
{
	switch (std)
	{
		case ETAL_BCAST_STD_UNDEF:
			return "Undefined";
		case ETAL_BCAST_STD_DRM:
			return "DRM";
		case ETAL_BCAST_STD_DAB:
			return "DAB";
		case ETAL_BCAST_STD_FM:
			return "FM";
		case ETAL_BCAST_STD_HD_FM:
			return "HDFM";
		case ETAL_BCAST_STD_HD_AM:
			return "HDAM";
		case ETAL_BCAST_STD_AM:
			return "AM";
		default:
			return "Illegal";
	}
}

#ifdef HAVE_FM

/***************************
 *
 * etalDemo_FMRadio
 *
 **************************/
static ETAL_HANDLE etalDemo_FMSeekRadio(tU32 vI_StartFreq, EtalFrequencyBand FMBand)
{
	EtalReceiverAttr attr;
	char line[256];
	ETAL_HANDLE vl_handlefm = ETAL_INVALID_HANDLE;
	etalSeekDirectionTy vl_direction;
    EtalProcessingFeatures processingFeatures;
	etalCtxBackupEarlyAudioTy EtalCtxEarlyAudio;

	ETAL_HANDLE vl_hDatapathRdsRadioText = ETAL_INVALID_HANDLE;
	ETAL_HANDLE vl_hDatapathRdsDecoded = ETAL_INVALID_HANDLE;
	etalDemoModeTy vl_mode = ETALDEMO_MODE_INVALID;
	ETAL_STATUS ret;
	tU32 seekFreq;
	tBool vl_continue = false;

#ifndef CONFIG_HOST_OS_WIN32
	char XMLstring[512];
#endif

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO TUNING ***************\n");

	if (ETAL_BAND_HD == FMBand)
	{
		ret = etaltml_getFreeReceiverForPath(&vl_handlefm, ETAL_PATH_NAME_FM_HD_FG, &attr);
	}
	else
	{
		ret = etaltml_getFreeReceiverForPath(&vl_handlefm, ETAL_PATH_NAME_FM_FG, &attr);
	}
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath %s ERROR (%d)\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", ret);
		return vl_handlefm;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath %s ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											(ETAL_BAND_HD==FMBand)?"HD":"FM",
											attr.m_Standard, 
											attr.m_FrontEndsSize,
											attr.m_FrontEnds[0], attr.m_FrontEnds[1]);
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set %s band\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");

	if (false == earlyAudio)
	{
		processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		if ((ret = etal_change_band_receiver(vl_handlefm, FMBand, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ERROR", (ETAL_BAND_HD==FMBand)?"HD":"FM");
			return vl_handlefm;
		}
		else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ok\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");
		}
	}
	
	/*
	 * Tune to an FM station
	 */

	if (!IsDemonstrationMode)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to %s freq %d\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", vI_StartFreq);
	}
	else
	{
#ifndef CONFIG_HOST_OS_WIN32
		fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
		lastFreqUsed = vI_StartFreq;
		snprintf(XMLstring, sizeof(XMLstring),
			"<info><Band></Band><Frequency>%d</Frequency><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
			lastFreqUsed);
		write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
		close(fd_RadioText);
#endif
	}

//HD mod: better use async interface to get quicker feedback
// then we should monitor through ETAL_INFO_TUNE
#ifndef HAVE_HD
	ret = etal_tune_receiver(vl_handlefm, vI_StartFreq);
#else
	if (ETAL_BAND_HD == FMBand)
	{
		ret = etal_tune_receiver_async(vl_handlefm, vI_StartFreq);
	}
	else
	{
		ret = etal_tune_receiver(vl_handlefm, vI_StartFreq);
	}
#endif
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver %s (%d)\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", ret);
		return ETAL_INVALID_HANDLE;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver %s ok, freq = %d\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", vI_StartFreq);
		if (true == earlyAudio)
		{
			EtalCtxEarlyAudio.Freq = vI_StartFreq;
			EtalCtxEarlyAudio.Band = FMBand;
			(void)etal_backup_context_for_early_audio(&EtalCtxEarlyAudio);
	}
	}

#ifndef HAVE_HD
	ret = etal_audio_select(vl_handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM);
#else
	if (ETAL_BAND_HD != FMBand)
	{
		ret = etal_audio_select(vl_handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM);
	}
	else
	{
		ret = etal_audio_select(vl_handlefm, ETAL_AUDIO_SOURCE_AUTO_HD);
	}
#endif
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select %s error\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handlefm;
	}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE DONE ***************\n");

	// start the RDS
	//
	if ((ret = etal_start_RDS(vl_handlefm, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_start_RDS (%d)\n", ret);
		return vl_handlefm;
	}

	
	// start RadioText
	etalDemo_startRadioText(vl_handlefm, &vl_hDatapathRdsRadioText);

	// seek loop
	do
	{
		vl_continue = false;

		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'u' -> to seek up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'd' -> to seek down\n");
#ifndef HAVE_HD
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 's' -> to scan up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'r' -> to scan down\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'l' -> to start learn procedure\n");
#endif
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'x' -> to toggle RDS on/off\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'z' -> to toggle RDS full decoded data on/off\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'm' -> to toggle Quality Monitor on/off\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'q' -> to quit seek/scan mode **** \n");

		if (!IsDemonstrationMode)
		{
		    if (fgets(line, sizeof(line), stdin)) vl_continue = true;
		}
		else
		{
#ifndef CONFIG_HOST_OS_WIN32
		    fd_CtrlFIFO = open(CtrlFIFO, O_RDONLY);
		    Sleep(10000);
		    if (read(fd_CtrlFIFO, line, sizeof(line))) vl_continue = true;
		    close(fd_CtrlFIFO);
#endif // !CONFIG_HOST_OS_WIN32
		}

		if (vl_continue)
		{
			switch (line[0])
			{
				case 'u' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				case 'd' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
#ifndef HAVE_HD
				case 's' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					vl_mode = ETALDEMO_MODE_SCAN;
					break;
				case 'r' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					vl_mode = ETALDEMO_MODE_SCAN;
					break;
				case 'l' :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_LEARN;
					break;
#endif
				case 'q' :
					vl_continue = false;
					vl_mode = ETALDEMO_MODE_INVALID;
					etalDemo_stopQualityMonitor();
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t Quality Monitor is OFF\n");
					break;
				case 'x' :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					if (RDS_on)
					{
						RDS_on = false;
						if ((ret = etal_stop_RDS(vl_handlefm)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_stop_RDS (%d)\n", ret);
							return vl_handlefm;
						}						
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t RDS is OFF\n");
						break;
					}
					else
					{
						RDS_on = true;
						if ((ret = etal_start_RDS(vl_handlefm, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_start_RDS (%d)\n", ret);
							return vl_handlefm;
						}
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t RDS is ON\n");
						break;
					}
					break;
				case 'z' :
						vl_continue = true;
						vl_mode = ETALDEMO_MODE_INVALID;
						if (RDS_decoded_on)
						{
							RDS_decoded_on = false;
							etalDemo_stopRadioRDSDecoded(vl_handlefm, vl_hDatapathRdsDecoded);							
							etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t RDS decoded is OFF\n");
							break;
						}
						else
						{
							RDS_decoded_on = true;
							etalDemo_startRadioRDSDecoded(vl_handlefm, &vl_hDatapathRdsDecoded);
							
							etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t RDS decoded is ON\n");
							break;
						}
						break;
						
				case 'm':
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					if (QualityMonitor_on)
					{
						etalDemo_stopQualityMonitor();
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t Quality Monitor is OFF\n");
						break;
					}
					else
					{
						etalDemo_startQualityMonitor(vl_handlefm, FMBand);
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t Quality Monitor is ON\n");
						break;
					}
				default :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					break;
			}

			switch (vl_mode)
			{
				case ETALDEMO_MODE_SEEK:
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SEEKING %s ***************\n",
							((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

					etalDemoSeek_CmdWaitResp(vl_handlefm, vl_direction, ETAL_DEMO_FREQ_STEP, ETAL_DEMO_SEEK_FM_WAIT_TIME);

					if ((ret = etal_autoseek_stop(vl_handlefm, lastFrequency)) != ETAL_RET_SUCCESS)
					{
						etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_seek (%d)", ret);
						return vl_handlefm;
					}

					//Get the new frequency
					etal_get_receiver_frequency(vl_handlefm, &seekFreq);
					if (!IsDemonstrationMode)
					{
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "Frequency %d\n", seekFreq);
					    if (true == earlyAudio)
					    {
						EtalCtxEarlyAudio.Freq = seekFreq;
						EtalCtxEarlyAudio.Band = FMBand;
					    	(void)etal_backup_context_for_early_audio(&EtalCtxEarlyAudio);
					}
					}
					else
					{
#ifndef CONFIG_HOST_OS_WIN32
					    fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
					    lastFreqUsed = seekFreq;
					    snprintf(XMLstring, sizeof(XMLstring),
							"<info><Band></Band><Frequency>%d</Frequency><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
							lastFreqUsed);
					    write(fd_RadioText,XMLstring, strlen(XMLstring)+1);
					    close(fd_RadioText);
#endif
					}

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SEEKING END ***************\n");

					break;
				case ETALDEMO_MODE_SCAN:
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SCANNING %s ***************\n",
							((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

					etalDemoScan(vl_handlefm, vl_direction, &vl_hDatapathRdsRadioText);

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SCANNING END ***************\n");
					break;
				case ETALDEMO_MODE_LEARN:
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO LEARNING ***************\n");

					etalDemoLearn(vl_handlefm, FMBand);

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO LEARNING END ***************\n");
					break;
				default:
					break;
			}
		}
	} while((true == vl_continue));

	if (ETAL_INVALID_HANDLE != vl_hDatapathRdsRadioText)
	{
		hInitialDatapath = vl_hDatapathRdsRadioText;
	}
		
	return vl_handlefm;
}


static ETAL_HANDLE etalDemo_TuneFMFgRadio(tU32 vI_freq, EtalFrequencyBand FMBand)
{
	EtalReceiverAttr attr_fm;
	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;

	ETAL_HANDLE vl_hDatapathRdsRadioText = ETAL_INVALID_HANDLE;
	ETAL_HANDLE vl_hDatapathRdsDecoded = ETAL_INVALID_HANDLE;
    EtalProcessingFeatures processingFeatures;
	etalCtxBackupEarlyAudioTy EtalCtxEarlyAudio;

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM FG RADIO TUNING ***************\n");

#ifdef HAVE_HD
	if (ETAL_BAND_HD == FMBand)
	{
		ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_FM_HD_FG, &attr_fm);
	}
	else
	{
		ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_FM_FG, &attr_fm);
	}
#else
	ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_FM_FG, &attr_fm);
#endif
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneFMFgRadio / etaltml_getFreeReceiverForPath %s ERROR\n",
			(ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneFMFgRadio / etaltml_getFreeReceiverForPath %s ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											(ETAL_BAND_HD==FMBand)?"HD":"FM",
											attr_fm.m_Standard, 
											attr_fm.m_FrontEndsSize,
											attr_fm.m_FrontEnds[0], attr_fm.m_FrontEnds[1]);
	
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set %s band\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");

	etalDemoPrintf(TR_LEVEL_COMPONENT, "TuneFMFgRadio: %s band (%x)\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", FMBand);

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, FMBand, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ERROR", (ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ok\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");
	}
	/*
	 * Tune to an FM station
	 */
	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to FM freq %d\n", vI_freq);

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
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver %s (%d)\n",
			(ETAL_BAND_HD==FMBand)?"HD":"FM", ret);
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver %s ok, freq = %d\n",
			(ETAL_BAND_HD==FMBand)?"HD":"FM", vI_freq);
		if (true == earlyAudio)
		{
			EtalCtxEarlyAudio.Freq = vI_freq;
			EtalCtxEarlyAudio.Band = FMBand;
			(void)etal_backup_context_for_early_audio(&EtalCtxEarlyAudio);
	}
	}

#ifndef HAVE_HD
	ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_STAR_AMFM);
#else
	if (ETAL_BAND_HD != FMBand)
	{
		ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_STAR_AMFM);
	}
	else
	{
		ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_AUTO_HD);
	}
#endif
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select %s error", (ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handle;
	}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE FM FG DONE ***************\n");

	// start the RDS
	//
	if ((ret = etal_start_RDS(vl_handle, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_start_RDS (%d)", ret);
		return vl_handle;
	}

	// start RadioText
	etalDemo_startRadioText(vl_handle, &vl_hDatapathRdsRadioText);

	if (ETAL_INVALID_HANDLE != vl_hDatapathRdsRadioText)
	{
		hInitialDatapath = vl_hDatapathRdsRadioText;
	}

	etalDemo_startRadioRDSDecoded(vl_handle, &vl_hDatapathRdsDecoded);

	//start Quality Monitor
	etalDemo_startQualityMonitor(vl_handle, FMBand);

	return vl_handle;
}
#endif // HAVE FM

#ifdef HAVE_AM
/***************************
 *
 * etalDemo_FMRadio
 *
 **************************/
static ETAL_HANDLE etalDemo_AMSeekRadio(tU32 vI_StartFreq, EtalFrequencyBand AMBand)
{
	EtalReceiverAttr attr;
	char line[256];
	ETAL_HANDLE vl_handleam = ETAL_INVALID_HANDLE;
	etalSeekDirectionTy vl_direction;
    EtalProcessingFeatures processingFeatures;

	ETAL_HANDLE vl_hDatapathRds = ETAL_INVALID_HANDLE;
	etalDemoModeTy vl_mode = ETALDEMO_MODE_INVALID;
	ETAL_STATUS ret;
	tU32 seekFreq;
	tBool vl_continue = false;

	char XMLstring[512];

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** AM RADIO TUNING ***************\n");

	ret = etaltml_getFreeReceiverForPath(&vl_handleam, ETAL_PATH_NAME_AM, &attr);
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath %s ERROR (%d)\n", "AM", ret);
		return vl_handleam;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath %s ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											"AM",
											attr.m_Standard, 
											attr.m_FrontEndsSize,
											attr.m_FrontEnds[0], attr.m_FrontEnds[1]);
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set %s band\n", "AM");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handleam, AMBand, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ERROR", "AM");
		return vl_handleam;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ok\n", "AM");
	}
	
	/*
	 * Tune to an AM station
	 */

	if (!IsDemonstrationMode)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to %s freq %d\n", "AM", vI_StartFreq);
	}
	else
	{
		fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC);
		lastFreqUsed = vI_StartFreq;
		snprintf(XMLstring, sizeof(XMLstring),
			"<info><Band></Band><Frequency>%d</Frequency><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
			lastFreqUsed);
		write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
		close(fd_RadioText);
	}

//HD mod: better use async interface to get quicker feedback
// then we should monitor through ETAL_INFO_TUNE
	ret = etal_tune_receiver(vl_handleam, vI_StartFreq);

	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver %s (%d)\n", "AM", ret);
		return ETAL_INVALID_HANDLE;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver %s ok, freq = %d\n", "AM", vI_StartFreq);
	}

	ret = etal_audio_select(vl_handleam, ETAL_AUDIO_SOURCE_STAR_AMFM);

	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select %s error", "AM");
		return vl_handleam;
	}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE DONE ***************\n");

	// seek loop
	do
	{
		vl_continue = false;

		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'u' -> to seek up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'd' -> to seek down\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 's' -> to scan up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'r' -> to scan down\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'l' -> to start learn procedure\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'm' -> to toggle Quality Monitor on/off\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'q' -> to quit seek/scan mode **** \n");

		if (!IsDemonstrationMode)
		{
		    if (fgets(line, sizeof(line), stdin)) vl_continue = true;
		}
		else
		{
#ifndef CONFIG_HOST_OS_WIN32
		    fd_CtrlFIFO = open(CtrlFIFO, O_RDONLY);
		    Sleep(10000);
		    if (read(fd_CtrlFIFO, line, sizeof(line))) vl_continue = true;
		    close(fd_CtrlFIFO);
#endif // !CONFIG_HOST_OS_WIN32
		}

		if (vl_continue)
		{
			switch (line[0])
			{
				case 'u' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				case 'd' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				case 's' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					vl_mode = ETALDEMO_MODE_SCAN;
					break;
				case 'r' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					vl_mode = ETALDEMO_MODE_SCAN;
					break;
				case 'l' :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_LEARN;
					break;
				case 'm':
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					if (QualityMonitor_on)
					{
						etalDemo_stopQualityMonitor();
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t Quality Monitor is OFF\n");
						break;
					}
					else
					{
						etalDemo_startQualityMonitor(vl_handleam, AMBand);
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t Quality Monitor is ON\n");
						break;
					}
				case 'q' :
					vl_continue = false;
					vl_mode = ETALDEMO_MODE_INVALID;
					etalDemo_stopQualityMonitor();
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t Quality Monitor is OFF\n");
					break;
				default :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					break;
			}

			switch (vl_mode)
			{
				case ETALDEMO_MODE_SEEK:
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** AM RADIO SEEKING %s ***************\n",
							((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

					etalDemoSeek_CmdWaitResp(vl_handleam, vl_direction, ETAL_DEMO_AM_FREQ_STEP, ETAL_DEMO_SEEK_AM_WAIT_TIME);

					if ((ret = etal_autoseek_stop(vl_handleam, lastFrequency)) != ETAL_RET_SUCCESS)
					{
						etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_seek (%d)", ret);
						return vl_handleam;
					}

					//Get the new frequency
					etal_get_receiver_frequency(vl_handleam, &seekFreq);
					if (!IsDemonstrationMode)
					{
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "Frequency %d\n", seekFreq);
					}
					else
					{
					    fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC);
					    lastFreqUsed = seekFreq;
					    snprintf(XMLstring, sizeof(XMLstring),
							"<info><Band></Band><Frequency>%d</Frequency><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
							lastFreqUsed);
					    write(fd_RadioText,XMLstring, strlen(XMLstring)+1);
					    close(fd_RadioText);
					}

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** AM RADIO SEEKING END ***************\n");
								
					break;
				case ETALDEMO_MODE_SCAN:
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** AM RADIO SCANNING %s ***************\n",
							((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

					etalDemoScan(vl_handleam, vl_direction, &vl_hDatapathRds);

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** AM RADIO SCANNING END ***************\n");
					break;
				case ETALDEMO_MODE_LEARN:
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** AM RADIO LEARNING ***************\n");

					etalDemoLearn(vl_handleam, AMBand);

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** AM RADIO LEARNING END ***************\n");
					break;
				default:
					break;
			}
		}
	} while((true == vl_continue));

	if (ETAL_INVALID_HANDLE != vl_hDatapathRds)
	{
		hInitialDatapath = vl_hDatapathRds;
	}
		
	return vl_handleam;
}

static ETAL_HANDLE etalDemo_TuneAMRadio(tU32 vI_freq, EtalFrequencyBand AMBand)
{
	EtalReceiverAttr attr_fm;
	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;

	ETAL_HANDLE vl_hDatapathRds = ETAL_INVALID_HANDLE;
       EtalProcessingFeatures processingFeatures;

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** AM FG RADIO TUNING ***************\n");

	ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_AM, &attr_fm);
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneAMRadio / etaltml_getFreeReceiverForPath %s ERROR\n",
			"AM");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneAMRadio / etaltml_getFreeReceiverForPath %s ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											"AM",
											attr_fm.m_Standard, 
											attr_fm.m_FrontEndsSize,
											attr_fm.m_FrontEnds[0], attr_fm.m_FrontEnds[1]);
	
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set %s band\n", "AM");

	etalDemoPrintf(TR_LEVEL_COMPONENT, "TuneAMRadio: %s band (%x)\n", "AM", AMBand);

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, AMBand, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ERROR", "AM");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ok\n", "AM");
	}
	/*
	 * Tune to an FM station
	 */
	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to AM freq %d\n", vI_freq);

	// then we should monitor through ETAL_INFO_TUNE
	ret = etal_tune_receiver(vl_handle, vI_freq);

	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver %s (%d)\n",
			"AM", ret);
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver %s ok, freq = %d\n",
			"AM", vI_freq);
	}

	ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_STAR_AMFM);

	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select %s error", "AM");
		return vl_handle;
	}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE AM DONE ***************\n");
	
	//start QualityMonitor
	etalDemo_startQualityMonitor(vl_handle, AMBand);	
	if (ETAL_INVALID_HANDLE != vl_hDatapathRds)
	{
		hInitialDatapath = vl_hDatapathRds;
	}


	return vl_handle;
}

#endif //HAVE_AM


#ifdef HAVE_DAB

/***************************
 *
 * etalDemo_DABRadioSeek
 *
 **************************/
static ETAL_HANDLE etalDemo_DABRadioSeek(tU32 vI_StartFreq)
{
	EtalReceiverAttr attr;
	ETAL_HANDLE handleDab = ETAL_INVALID_HANDLE;
	ETAL_HANDLE handleDab2 = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;
	etalSeekDirectionTy vl_direction = cmdDirectionUp;
	etalDemoModeTy vl_mode;

	ETAL_HANDLE vl_hDatapathRds = ETAL_INVALID_HANDLE;
    EtalProcessingFeatures processingFeatures;
	etalCtxBackupEarlyAudioTy EtalCtxEarlyAudio;

	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size
	tU8 charset;
	char label[17];
	tU16 bitmap;
	unsigned int ueid;
	int i;

	char line[256];
	int Service = 0;
	tU32 seekFreq;
	tBool vl_continue = false;
	tBool vl_manualSeek = false;
	tBool vl_serviceSelectLoop = true;
	tBool vl_2ndSeek = false;

#ifndef CONFIG_HOST_OS_WIN32
	char XMLstring[512];
#endif

	/*
	 * Create a receiver configuration
	 */

	if ((ret = etaltml_getFreeReceiverForPath(&handleDab, ETAL_PATH_NAME_DAB_1, &attr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath DAB ERROR (%d)\n", ret);
		return handleDab;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr.m_Standard, 
											attr.m_FrontEndsSize,
											attr.m_FrontEnds[0], attr.m_FrontEnds[1]);
	}

	/*
	 * Create a 2nd receiver configuration
	  */

	if ((ret = etaltml_getFreeReceiverForPath(&handleDab2, ETAL_PATH_NAME_DAB_2, &attr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath DAB ERROR (%d)\n", ret);
		return handleDab;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr.m_Standard, 
											attr.m_FrontEndsSize,
											attr.m_FrontEnds[0], attr.m_FrontEnds[1]);
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(handleDab, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB ERROR");
		return handleDab;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB  ok\n");
	}

	// set band for 2nd receiver
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband for 2nd receiver\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(handleDab2, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB2 ERROR");
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB2  ok\n");
	}

	/*
	 * Tune to a DAB ensemble
	 */

	if (!IsDemonstrationMode)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to DAB freq %d\n", vI_StartFreq);
	}
	else
	{
#ifndef CONFIG_HOST_OS_WIN32
		fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
		lastFreqUsed = vI_StartFreq;
		snprintf(XMLstring, sizeof(XMLstring),
			"<info><Band>DAB</Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
			lastFreqUsed, tunedEnsemble);
		write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
		close(fd_RadioText);
#endif
	}

	ret = etal_tune_receiver(handleDab, vI_StartFreq);
	if ((ETAL_RET_SUCCESS == ret) || (ret == ETAL_RET_NO_DATA))
	{
		Sleep(USLEEP_ONE_SEC);
		ret = etalDemo_DABServiceListAndSelect(handleDab);

		if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_DABServiceListAndSelect error" );
			return handleDab;
		}

		tunedEnsemble = true;
		seekFreq = vI_StartFreq;
		
		if (true == earlyAudio)
		{
			EtalCtxEarlyAudio.Freq = vI_StartFreq;
			EtalCtxEarlyAudio.Band = ETAL_BAND_DAB3;
			(void)etal_backup_context_for_early_audio(&EtalCtxEarlyAudio);
		}

		if (IsDemonstrationMode)
		{
#ifndef CONFIG_HOST_OS_WIN32
			//Print the new frequency
			fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
			snprintf(XMLstring, sizeof(XMLstring),
				"<info><Band>DAB</Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
				lastFreqUsed, tunedEnsemble);
			write(fd_RadioText,XMLstring, strlen(XMLstring)+1);
			close(fd_RadioText);
#endif
		}
		if (ret != ETAL_RET_NO_DATA)
		{
			// start RadioText
			etalDemo_startRadioText(handleDab, &vl_hDatapathRds);
		}
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver DAB (%d)\n", ret);
	}

	if ((ret = etal_audio_select(handleDab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error" );
		return handleDab;
	}


	do
	{
		vl_continue = false;


		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'u' -> to seek up\n");
		if (tunedEnsemble)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 's' -> to select a service in current ensemble\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'n' -> to select next service in current ensemble\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'p' -> to select previous service in current ensemble\n");
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'd' -> to seek down\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'l' -> to start learn procedure\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'i' -> to manual seek up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'k' -> to manual seek down\n");
		if (vl_manualSeek)
		    etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'c' -> to test continue manual seek\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'x' -> NA in DAB\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'm' -> to toggle Quality Monitor on/off\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t '2' -> launch a seek on second DAB receiver and display service list\n");
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'q' -> continue test with Service Following **** \n");
#else
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'q' -> proceed to the exit **** \n");
#endif

		if (!IsDemonstrationMode)
		{
		    if (fgets(line, sizeof(line), stdin)) vl_continue = true;
		}
		else
		{
#ifndef CONFIG_HOST_OS_WIN32
		    fd_CtrlFIFO = open(CtrlFIFO, O_RDONLY);
		    Sleep(10000);
		    if (read(fd_CtrlFIFO, line, sizeof(line))) vl_continue = true;
		    close(fd_CtrlFIFO);
#endif // !CONFIG_HOST_OS_WIN32
		}

		if (vl_continue)
		{
			switch (line[0])
			{
				case 'u' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					tunedEnsemble = false;
					Service = 0;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = false;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				case 'd' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					tunedEnsemble = false;
					Service = 0;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = false;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				case 's' :
					if (tunedEnsemble)
					{
						vl_continue = true;
						vl_mode = ETALDEMO_MODE_SERVICE_SELECT;
					}
					else
					{
						vl_continue = true;
						vl_mode = ETALDEMO_MODE_INVALID;
					}
					break;
				case 'n' :
					if (tunedEnsemble)
					{
						vl_continue = true;
						vl_direction = cmdDirectionUp;
						vl_mode = ETALDEMO_MODE_SEEK;
					}
					else
					{
						vl_continue = true;
						vl_mode = ETALDEMO_MODE_INVALID;
					}
					break;
				case 'p' :
					if (tunedEnsemble)
					{
						vl_continue = true;
						vl_direction = cmdDirectionDown;
						vl_mode = ETALDEMO_MODE_SEEK;
					}
					else
					{
						vl_continue = true;
						vl_mode = ETALDEMO_MODE_INVALID;
					}
					break;
				case 'l' :
					vl_continue = true;
					tunedEnsemble = false;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = false;
					vl_mode = ETALDEMO_MODE_LEARN;
					break;
				case 'i' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					tunedEnsemble = false;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = true;
					vl_mode = ETALDEMO_MODE_MANUAL_SEEK;
					break;
				case 'k' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					tunedEnsemble = false;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = true;
					vl_mode = ETALDEMO_MODE_MANUAL_SEEK;
					break;
				case 'c' :
					vl_continue = true;
					tunedEnsemble = false;
					vl_mode = ETALDEMO_MODE_CONTINUE_MANUAL_SEEK;
					break;
				case 'q' :
					vl_continue = false;
					vl_mode = ETALDEMO_MODE_INVALID;
					etalDemo_stopQualityMonitor();
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t Quality Monitor is OFF\n");
					break;
				case 'm':
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					if (QualityMonitor_on)
					{
						etalDemo_stopQualityMonitor();
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t Quality Monitor is OFF\n");
						break;
					}
					else
					{
						etalDemo_startQualityMonitor(handleDab, ETAL_BAND_DAB3);
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t Quality Monitor is ON\n");
						break;
					}
				case 'x' :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					break;
	
				default :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					break;
			}

			if (vl_mode == ETALDEMO_MODE_SEEK)
			{
				/* This is the case of 'next service' or 'previous service' command
				 * It's looping around service list within the current ensemble
				 */
				if (tunedEnsemble)
				{
				    if (vl_direction)
				    {
					    //down
					    if (Service == 0)
					    {
					        Service = serv_list.m_serviceCount - 1;
					    }
					    else Service--;
				    }
				    else
				    {
					    //up
					    if (Service == (serv_list.m_serviceCount - 1))
					    {
					        Service = 0;
					    }
					    else Service++;
				    }
				}
				else
				{
				    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

				    etalDemoSeek_CmdWaitResp(handleDab, vl_direction, ETAL_DEMO_FREQ_STEP, ETAL_DEMO_SEEK_DAB_WAIT_TIME);

				    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** GET ENSEMBLE DATA ***************\n");

				    etal_get_receiver_frequency(handleDab, &seekFreq);

				    if ((ret = etal_autoseek_stop(handleDab, lastFrequency)) != ETAL_RET_SUCCESS)
				    {
				        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_seek (%d)", ret);
				    }

				    //Print the new frequency
				    etalDemoPrintf(TR_LEVEL_COMPONENT, "Frequency %d\n", seekFreq);
				}

				if (IsDemonstrationMode)
				{
#ifndef CONFIG_HOST_OS_WIN32
					//Print the new frequency
					fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
					lastFreqUsed = seekFreq;
					snprintf(XMLstring, sizeof(XMLstring),
						"<info><Band>DAB</Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
						lastFreqUsed, tunedEnsemble);
					write(fd_RadioText,XMLstring, strlen(XMLstring)+1);
					close(fd_RadioText);
#endif
				}

				// get the new ensemble id
				etalDemoPrintf(TR_LEVEL_COMPONENT, "Reading the current ensemble\n");

				// wait the dab to be ready before tune
				//
				Sleep(USLEEP_ONE_SEC *1);
			}
			
			if (vl_mode == ETALDEMO_MODE_LEARN)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO LEARNING ***************\n");

				etalDemoLearn(handleDab, ETAL_BAND_DAB3);

				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO LEARNING END ***************\n");

				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** GET ENSEMBLE DATA ***************\n");	
				etal_get_receiver_frequency(handleDab, &seekFreq);
			}
		
			if ((vl_mode == ETALDEMO_MODE_SEEK) || (vl_mode == ETALDEMO_MODE_SERVICE_SELECT) || (vl_mode == ETALDEMO_MODE_LEARN))
			{
				if ((ret = etal_get_current_ensemble(handleDab, &ueid)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)", ret);
				}
				else
				{
					// get the new ensemble id
					etalDemoPrintf(TR_LEVEL_COMPONENT, "current ensemble 0x%x\n", ueid );

					if (ueid != ETAL_INVALID_UEID)
					{
						if ((ret = etal_get_ensemble_data(ueid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)", ret);
							return handleDab;
						}

						etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s, charset: %d, frequency is %d\n", label, charset, seekFreq);

						/*
						 * A DAB ensemble contains one or more services: fetch the list
						 * and present it to the user for selection. Some services will be
						 * audio only, others data only. The ETAL interface permits to select
						 * which service list to fetch, here we fetch only the audio services
						 */
						if ((ret = etal_get_service_list(handleDab, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)", ret);
							return handleDab;
						}

						etalDemoPrintf(TR_LEVEL_COMPONENT, "found: %d services \n", serv_list.m_serviceCount);
						
						/*
						 * etal_get_service_list returns a list of numerical ids, not very useful
						 * to show; so now for every service we read the service label
						 * and present it
						 */
						for (i = 0; i < serv_list.m_serviceCount; i++)
						{
							memset(&svinfo, 0, sizeof(EtalServiceInfo));
							memset(&scinfo, 0, sizeof(EtalServiceComponentList));
							if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
							{
								etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x\n", ret, i, ueid, serv_list.m_service[i] );
							}
							etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s) charset: %d\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel, svinfo.m_serviceLabelCharset);

						}

						if ((!IsDemonstrationMode) && ((vl_mode == ETALDEMO_MODE_SERVICE_SELECT) || (tunedEnsemble == false)))
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");

							do
							{
								if (fgets(line, sizeof(line), stdin))
								{
									if (1 == sscanf(line, "%d", &Service))
									{
										if ((Service > serv_list.m_serviceCount)||(Service <= 0))
										{
											etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong service selection !\n");
											etalDemoPrintf(TR_LEVEL_COMPONENT, "Please enter your selection:\n");
											vl_serviceSelectLoop = true;
										}
										else
										{
											/* i can be safely used */
											Service--;
											memset(&svinfo, 0, sizeof(EtalServiceInfo));
											memset(&scinfo, 0, sizeof(EtalServiceComponentList));
											if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
											{
										        etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, ueid, serv_list.m_service[i] );
											}
											vl_serviceSelectLoop = false;
										}
									}
								}
							}while(vl_serviceSelectLoop);

							etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s, Charset: %d\n",
												Service+1,
												serv_list.m_service[Service], svinfo.m_serviceLabel, svinfo.m_serviceLabelCharset);
						}
						if ((ret = etal_service_select_audio(handleDab, ETAL_SERVSEL_MODE_SERVICE, ueid, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)", ret);
					        return handleDab;
						}
						else
						{
							tunedEnsemble = true;

							if (IsDemonstrationMode)
							{
#ifndef CONFIG_HOST_OS_WIN32
								//Print the new frequency
								fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
								snprintf(XMLstring, sizeof(XMLstring),
									"<info><Band>DAB</Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
									lastFreqUsed, tunedEnsemble);
								write(fd_RadioText,XMLstring, strlen(XMLstring)+1);
								close(fd_RadioText);
#endif
							}

							// start RadioText
							etalDemo_startRadioText(handleDab, &vl_hDatapathRds);
							

						}
					}
				}
				
				if (vl_mode == ETALDEMO_MODE_SEEK) etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO SEEKING END ***************\n");

				if ((ret = etal_audio_select(handleDab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error" );
					return handleDab;
				}
			}
			else if (vl_mode == ETALDEMO_MODE_MANUAL_SEEK)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

				if ((ret = etal_seek_start_manual(handleDab, vl_direction, 0, &seekFreq)) != ETAL_RET_NO_DATA)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);

					tunedEnsemble = true;

					// wait the dab to be ready before tune
					Sleep(USLEEP_ONE_SEC *1);

				    if ((ret = etalDemo_DABServiceListAndSelect(handleDab)) != ETAL_RET_SUCCESS)
				    {
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_DABServiceListAndSelect error" );
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
					    return handleDab;
				    }

				    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");

				    if ((ret = etal_audio_select(handleDab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
				    {
					   etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error" );
					   return handleDab;
				    }

				    // start RadioText
				    etalDemo_startRadioText(handleDab, &vl_hDatapathRds);

				}
				else
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);
					etalDemoPrintf(TR_LEVEL_COMPONENT, "No DAB service found !\n");
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
				}
			}
			else if (vl_mode == ETALDEMO_MODE_CONTINUE_MANUAL_SEEK)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

				if ((ret = etal_seek_continue_manual(handleDab, &seekFreq)) == ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);

					tunedEnsemble = true;

					// wait the dab to be ready before tune
					//
					Sleep(USLEEP_ONE_SEC *1);

				    if ((ret = etalDemo_DABServiceListAndSelect(handleDab)) != ETAL_RET_SUCCESS)
				    {
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_DABServiceListAndSelect error" );
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
					    return handleDab;
				    }

				    if ((ret = etal_audio_select(handleDab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
				    {
					   etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error" );
					   etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
					   return handleDab;
				    }

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");

				    // start RadioText
				    etalDemo_startRadioText(handleDab, &vl_hDatapathRds);

				}
				else if (ETAL_RET_NO_DATA == ret)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);
					etalDemoPrintf(TR_LEVEL_COMPONENT, "No DAB service found !\n");
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
				}
			}
			if (true == vl_2ndSeek)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB2 RADIO SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

			    etalDemoSeek_CmdWaitResp(handleDab2, vl_direction, ETAL_DEMO_FREQ_STEP, ETAL_DEMO_SEEK_DAB_WAIT_TIME);

				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** GET ENSEMBLE DATA ***************\n");

				etal_get_receiver_frequency(handleDab2, &seekFreq);

			    if ((ret = etal_autoseek_stop(handleDab2, lastFrequency)) != ETAL_RET_SUCCESS)
			    {
			        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_seek (%d)", ret);
			    }

			    //Print the new frequency
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "Frequency %d\n", seekFreq);

				// get the new ensemble id
				etalDemoPrintf(TR_LEVEL_COMPONENT, "Reading the current ensemble\n");

				// wait the dab to be ready before tune
				//
				Sleep(USLEEP_ONE_SEC *1);

			    if ((ret = etalDemo_DABServiceList(handleDab2)) != ETAL_RET_SUCCESS)
			    {
				    etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_DABServiceList error" );
			    }
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB2 RADIO SEEKING END ***************\n");
				
				vl_2ndSeek = false;
			}
		}
	} while((true == vl_continue));

	if (ETAL_INVALID_HANDLE != vl_hDatapathRds)
	{
		hInitialDatapath = vl_hDatapathRds;
	}

	if (ETAL_INVALID_HANDLE != handleDab2)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy secondary receiver\n");
		if ((ret = etal_destroy_receiver(&handleDab2)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}

	return handleDab;
}


/***************************
 *
 * etalDemo_TuneDAB1Radio
 *
 **************************/
static ETAL_HANDLE etalDemo_TuneDAB1Radio(tU32 vI_freq)
{
	EtalReceiverAttr attr_dab_1;

	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;

	ETAL_HANDLE vl_hDatapathRds = ETAL_INVALID_HANDLE;
    EtalProcessingFeatures processingFeatures;

	ETAL_STATUS ret;

	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size
	tU8 charset;
	char label[17];
	tU16 bitmap;
	tU32 ueid;
	int i;
	etalCtxBackupEarlyAudioTy EtalCtxEarlyAudio;

	char line[256];
	int Service;
	tBool vl_selectedSidPresent = false;
	tBool vl_continue = false;
#ifdef ETAL_DEMO_CREATE_FIC_DATAPATH
	ETAL_HANDLE vl_hDatapath;
	EtalDataPathAttr vl_dataPathAttr;
#endif

	/*
	 * Create a receiver configuration
	 */

		
	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB1 RADIO TUNING ***************\n");
		
	if ((ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_DAB_1, &attr_dab_1)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB1Radio / etaltml_getFreeReceiverForPath DAB ERROR\n");
		goto exit;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB1Radio / etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr_dab_1.m_Standard, 
											attr_dab_1.m_FrontEndsSize,
											attr_dab_1.m_FrontEnds[0], attr_dab_1.m_FrontEnds[1]);
	
	}


	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB ERROR\n");
		goto exit;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB  ok\n");
	}

	/*
	 * Tune to a DAB ensemble
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to DAB freq %d\n", vI_freq);
	ret = etal_tune_receiver(vl_handle, vI_freq);

	if ((ETAL_RET_SUCCESS == ret) || (ETAL_RET_NO_DATA == ret))
	{
		if (ETAL_RET_SUCCESS == ret)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB ok, freq = %d\n", vI_freq);
		}
		else if (ETAL_RET_NO_DATA == ret)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB no data, freq = %d\n", vI_freq);
		}

		if (true == earlyAudio)
		{
			EtalCtxEarlyAudio.Freq = vI_freq;
			EtalCtxEarlyAudio.Band = ETAL_BAND_DAB3;
			(void)etal_backup_context_for_early_audio(&EtalCtxEarlyAudio);
		}
		/*
		 * Unlike FM, for DAB we need to allow the tuner some time
		 * to capture and decode the ensemble information
		 * Without this information the tuner is unable to perform
		 * any operation on the ensemble except quality measure
		 */
		Sleep(USLEEP_ONE_SEC);

		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, first pass\n");

		if ((ret = etal_get_current_ensemble(vl_handle, &ueid)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)\n", ret);
            goto exit;
		}

		/*
		 * Now that we know the UEId let's get the ensemble label; this is not required
		 * but nicer to show rather than the ensemble frequency
		 */
		if ((ret = etal_get_ensemble_data(ueid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)\n", ret);
			goto exit;
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

		/*
		 * A DAB ensemble contains one or more services: fetch the list
		 * and present it to the user for selection. Some services will be
		 * audio only, others data only. The ETAL interface permits to select
		 * which service list to fetch, here we fetch only the audio services
		 */
		if ((ret = etal_get_service_list(vl_handle, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)\n", ret);
			goto exit;
		}
		/*
		 * etal_get_service_list returns a list of numerical ids, not very useful
		 * to show; so now for every service we read the service label
		 * and present it
		 */
		for (i = 0; i < serv_list.m_serviceCount; i++)
		{
			memset(&svinfo, 0, sizeof(EtalServiceInfo));
			memset(&scinfo, 0, sizeof(EtalServiceComponentList));
			if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d)\n", ret);
			}
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);
		}

		if (false == vl_selectedSidPresent)
		{
			do
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");

				if (fgets(line, sizeof(line), stdin))
				{
					if (1 == sscanf(line, "%d", &Service))
					{
						if ((Service > serv_list.m_serviceCount)||(Service <= 0))
						{
							etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong service selection !\n");
							vl_continue=true;
						}
						else
						{
							/* i can be safely used */
							Service--;
							memset(&svinfo, 0, sizeof(EtalServiceInfo));
							memset(&scinfo, 0, sizeof(EtalServiceComponentList));
							if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
							{
						        etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x\n", ret, i, ueid, serv_list.m_service[i] );
							}
							vl_continue=false;
						}
					}
				}
			}while (true == vl_continue);
		}

		/*
		 * Now the user could select the service
		 * We do it for him
		 */

		/*
		 * This is the DAB equivalent of tuning to an FM station.
		 * Audio may be available after this step, depending on the CMOST CUT:
		 * - CUT 1.0 no audio
		 * - CUT 2.x audio
		 * See above for hints on how to find out the CUT version.
		 */
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s\n", 
						Service+1, 
						serv_list.m_service[Service], svinfo.m_serviceLabel); 
	
		if ((ret = etal_service_select_audio(vl_handle, ETAL_SERVSEL_MODE_SERVICE, ueid, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)\n", ret);
			goto exit;
		}

		if ((ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error\n" );
			goto exit;
		}

		// start RadioText
		etalDemo_startRadioText(vl_handle, &vl_hDatapathRds);

#ifdef ETAL_DEMO_CREATE_FIC_DATAPATH
// create a data path for fic

	//config_datapath ETAL_DATA_TYPE_DAB_FIC
				 vl_hDatapath = ETAL_INVALID_HANDLE;
				 OSAL_pvMemorySet((tPVoid)&vl_dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
				 vl_dataPathAttr.m_receiverHandle = vl_handle;
				 vl_dataPathAttr.m_dataType = ETAL_DATA_TYPE_DAB_FIC;
				 vl_dataPathAttr.m_sink.m_BufferSize = 0x00;
				 vl_dataPathAttr.m_sink.m_CbProcessBlock = etalDemoSeekOrTuneDataCallback;
			 
				 if ((ret = etal_config_datapath(&vl_hDatapath,&vl_dataPathAttr)) != ETAL_RET_SUCCESS)
				 {
					  etalDemoPrintf(TR_LEVEL_ERRORS,"etal_config_datapath for FIC (%d)",  ret);
					 return OSAL_ERROR;
				 }
			 
				 if ((ret = etal_get_fic(vl_handle)) != ETAL_RET_SUCCESS)
				 {
					 etalDemoPrintf(TR_LEVEL_ERRORS,"etal_get_fic (%d)",  ret);
					 return OSAL_ERROR;
				 }
#endif				 

		     // start Quality Monitor
	   	etalDemo_startQualityMonitor(vl_handle, ETAL_BAND_DAB3);
		if (ETAL_INVALID_HANDLE != vl_hDatapathRds)
		{
			hInitialDatapath = vl_hDatapathRds;
		}
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver DAB (%d)\n", ret);
	}
exit:
	return vl_handle;
}

/***************************
 *
 * etalDemo_TuneDAB2Radio
 *
 **************************/
static ETAL_HANDLE etalDemo_TuneDAB2Radio(tU32 vI_freq)
{
	EtalReceiverAttr attr_dab_2;

	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;

	ETAL_HANDLE vl_hDatapathAudioDataChannel = ETAL_INVALID_HANDLE;
	EtalDataPathAttr dataPathAttr;

    EtalProcessingFeatures processingFeatures;

	etalCtxBackupEarlyAudioTy EtalCtxEarlyAudio;

	ETAL_STATUS ret;

	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size
	tU8 charset;
	char label[17];
	tU16 bitmap;
	tU32 ueid;
	int i;

	char line[256];
	int Service;
	tBool vl_selectedSidPresent = false;
	tBool vl_continue = false;

	if (0 == vI_freq)
	{
		goto exit;
	}

	/*
	 * Create a receiver configuration
	 */


	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB2 RADIO TUNING ***************\n");

	if ((ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_DAB_2, &attr_dab_2)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB2Radio / etaltml_getFreeReceiverForPath DAB ERROR\n");
		goto exit;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB2Radio / etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr_dab_2.m_Standard,
											attr_dab_2.m_FrontEndsSize,
											attr_dab_2.m_FrontEnds[0], attr_dab_2.m_FrontEnds[1]);
	
	}


	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB ERROR\n");
		goto exit;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB  ok\n");
	}

	/*
	 * Tune to a DAB ensemble
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to DAB freq %d\n", vI_freq);
	ret = etal_tune_receiver(vl_handle, vI_freq);

	if ((ETAL_RET_SUCCESS == ret) || (ETAL_RET_NO_DATA == ret))
	{
		if (ETAL_RET_SUCCESS == ret)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB ok, freq = %d\n", vI_freq);
		}
		else if (ETAL_RET_NO_DATA == ret)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB no data, freq = %d\n", vI_freq);
		}

		if (true == earlyAudio)
		{
			EtalCtxEarlyAudio.Freq = vI_freq;
			EtalCtxEarlyAudio.Band = ETAL_BAND_DAB3;
			(void)etal_backup_context_for_early_audio(&EtalCtxEarlyAudio);
		}
		/*
		 * Unlike FM, for DAB we need to allow the tuner some time
		 * to capture and decode the ensemble information
		 * Without this information the tuner is unable to perform
		 * any operation on the ensemble except quality measure
		 */
		Sleep(USLEEP_ONE_SEC);

		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, first pass\n");

		if ((ret = etal_get_current_ensemble(vl_handle, &ueid)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)\n", ret);
            goto exit;
		}

		/*
		 * Now that we know the UEId let's get the ensemble label; this is not required
		 * but nicer to show rather than the ensemble frequency
		 */
		if ((ret = etal_get_ensemble_data(ueid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)\n", ret);
            goto exit;
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

		/*
		 * A DAB ensemble contains one or more services: fetch the list
		 * and present it to the user for selection. Some services will be
		 * audio only, others data only. The ETAL interface permits to select
		 * which service list to fetch, here we fetch only the audio services
		 */
		if ((ret = etal_get_service_list(vl_handle, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)\n", ret);
			goto exit;
		}
		/*
		 * etal_get_service_list returns a list of numerical ids, not very useful
		 * to show; so now for every service we read the service label
		 * and present it
		 */
		for (i = 0; i < serv_list.m_serviceCount; i++)
		{
			memset(&svinfo, 0, sizeof(EtalServiceInfo));
			memset(&scinfo, 0, sizeof(EtalServiceComponentList));
			if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d)\n", ret);
			}
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);
		}

		if (false == vl_selectedSidPresent)
		{
			do
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");

				if (fgets(line, sizeof(line), stdin))
				{
					if (1 == sscanf(line, "%d", &Service))
					{
						if ((Service > serv_list.m_serviceCount)||(Service <= 0))
						{
							etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong service selection !\n");
							vl_continue=true;
						}
						else
						{
							/* i can be safely used */
							Service--;
							memset(&svinfo, 0, sizeof(EtalServiceInfo));
							memset(&scinfo, 0, sizeof(EtalServiceComponentList));
							if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
							{
						        etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x\n", ret, i, ueid, serv_list.m_service[i] );
							}
							vl_continue=false;
						}
					}
				}
			}while (true == vl_continue);
		}

		/*
		 * Now the user could select the service
		 * We do it for him
		 */

		/*
		 * This is the DAB equivalent of tuning to an FM station.
		 * Audio may be available after this step, depending on the CMOST CUT:
		 * - CUT 1.0 no audio
		 * - CUT 2.x audio
		 * See above for hints on how to find out the CUT version.
		 */
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s\n",
						Service+1,
						serv_list.m_service[Service], svinfo.m_serviceLabel);

		/*
		 * Create a datapath for RAW audio
		 */

		memset(&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
		dataPathAttr.m_receiverHandle = vl_handle;
		dataPathAttr.m_dataType = ETAL_DATA_TYPE_DAB_AUDIO_RAW;
		dataPathAttr.m_sink.m_context = (void *)0;
		dataPathAttr.m_sink.m_BufferSize = 0; // unused
		dataPathAttr.m_sink.m_CbProcessBlock = etalDemo_DAB2DataCallback;
		if ((ret = etal_config_datapath(&vl_hDatapathAudioDataChannel, &dataPathAttr)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_config_datapath for DAB2 (%d)\n", ret);
			goto exit;
		}

		/*
		 * Select a data service
		 */
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a DATA service from the ensemble\n");
		if ((ret = etal_service_select_data(vl_hDatapathAudioDataChannel, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ueid, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)", ret);
			goto exit;
		}

	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver DAB (%d)\n", ret);
	}
exit:
	return vl_handle;
}


/***************************
 *
 * etalDemo_DAB2DataCallback
 *
 **************************/

void etalDemo_DAB2DataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalDemoPrintf(TR_LEVEL_COMPONENT, "DAB2 data received\n");
}

static ETAL_STATUS etalDemo_DABServiceList(ETAL_HANDLE handleDab)
{
	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size

	tU32 uEId;

	tU8 charset;
	char label[17];
	tU16 bitmap;

	int i;

	ETAL_STATUS ret;

    if ((ret = etal_get_current_ensemble(handleDab, &uEId)) != ETAL_RET_SUCCESS)
	{
        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)", ret);
    }
    else
	{
	    // get the new ensemble id
		etalDemoPrintf(TR_LEVEL_COMPONENT, "current ensemble 0x%x\n", uEId);

		if (uEId != ETAL_INVALID_UEID)
		{
		    if ((ret = etal_get_ensemble_data(uEId, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
			{
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)", ret);
				return 1;
			}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

			/*
			 * A DAB ensemble contains one or more services: fetch the list
			 * and present it to the user for selection. Some services will be
			 * audio only, others data only. The ETAL interface permits to select
			 * which service list to fetch, here we fetch only the audio services
			 */
			if ((ret = etal_get_service_list(handleDab, uEId, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)", ret);
				return ETAL_RET_ERROR;
			}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "found: %d services \n", serv_list.m_serviceCount);

			/*
			 * etal_get_service_list returns a list of numerical ids, not very useful
			 * to show; so now for every service we read the service label
			 * and present it
			 */
			for (i = 0; i < serv_list.m_serviceCount; i++)
			{
				memset(&svinfo, 0, sizeof(EtalServiceInfo));
				memset(&scinfo, 0, sizeof(EtalServiceComponentList));
				if ((ret = etal_get_specific_service_data_DAB(uEId, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, uEId, serv_list.m_service[i] );
				}
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);
			}

		}
	}
	return ret;
}


static ETAL_STATUS etalDemo_DABServiceListAndSelect(ETAL_HANDLE handleDab)
{
	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size

	tU32 uEId;

	tU8 charset;
	char label[17];
	tU16 bitmap;

	char line[256];
	int Service;
	int i;

	tBool vl_serviceSelectLoop = true;

	ETAL_STATUS ret;

    if ((ret = etal_get_current_ensemble(handleDab, &uEId)) != ETAL_RET_SUCCESS)
	{
        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)", ret);
    }
    else
	{
	    // get the new ensemble id
		etalDemoPrintf(TR_LEVEL_COMPONENT, "current ensemble 0x%x\n", uEId);

		if (uEId != ETAL_INVALID_UEID)
		{
		    if ((ret = etal_get_ensemble_data(uEId, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
			{
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)", ret);
				return 1;
			}

			
			etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

			/*
			 * A DAB ensemble contains one or more services: fetch the list
			 * and present it to the user for selection. Some services will be
			 * audio only, others data only. The ETAL interface permits to select
			 * which service list to fetch, here we fetch only the audio services
			 */
			if ((ret = etal_get_service_list(handleDab, uEId, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)", ret);
				return ETAL_RET_ERROR;
			}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "found: %d services \n", serv_list.m_serviceCount);

			/*
			 * etal_get_service_list returns a list of numerical ids, not very useful
			 * to show; so now for every service we read the service label
			 * and present it
			 */
			for (i = 0; i < serv_list.m_serviceCount; i++)
			{
				memset(&svinfo, 0, sizeof(EtalServiceInfo));
				memset(&scinfo, 0, sizeof(EtalServiceComponentList));
				if ((ret = etal_get_specific_service_data_DAB(uEId, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, uEId, serv_list.m_service[i] );
				}
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);
			}

			if (!IsDemonstrationMode)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");

				do
				{
					if (fgets(line, sizeof(line), stdin))
					{
						if (1 == sscanf(line, "%d", &Service))
						{
							if ((Service > serv_list.m_serviceCount)||(Service <= 0))
							{
								etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong service selection !\n");
								etalDemoPrintf(TR_LEVEL_COMPONENT, "Please enter your selection:\n");
								vl_serviceSelectLoop = true;
							}
							else
							{
								/* i can be safely used */
								Service--;
								memset(&svinfo, 0, sizeof(EtalServiceInfo));
								memset(&scinfo, 0, sizeof(EtalServiceComponentList));
								if ((ret = etal_get_specific_service_data_DAB(uEId, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
								{
						    	    etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, uEId, serv_list.m_service[i] );
								}
								vl_serviceSelectLoop = false;
							}
						}
					}
				}while(vl_serviceSelectLoop);
			}
			else
			{
				Service = 0;
				memset(&svinfo, 0, sizeof(EtalServiceInfo));
				memset(&scinfo, 0, sizeof(EtalServiceComponentList));
				if ((ret = etal_get_specific_service_data_DAB(uEId, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
				{
			        etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, uEId, serv_list.m_service[i] );
				}
			}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s\n",
							Service+1, serv_list.m_service[Service], svinfo.m_serviceLabel);
			if ((ret = etal_service_select_audio(handleDab, ETAL_SERVSEL_MODE_SERVICE, uEId, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)", ret);
				return ret;
			}
		}
		else
		{
			ret = ETAL_RET_NO_DATA;
		}
	}
	return ret;
}

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
/***************************
 *
 * etalDemo_GetDCOPImageCb
 *
 **************************/
int etalDemo_GetDCOPImageCb(void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap)
{
	static FILE *fhbs = NULL, *fhpg = NULL;
	static long file_bs_size = 0, file_pg_size = 0;
	FILE **fhgi;
	long file_position = 0, *file_size = NULL;
	size_t retfread;
	int do_get_file_size = 0;

	/* open file bootstrap or program firmware if not already open */
	if (isBootstrap != 0)
	{
		fhgi = &fhbs;
		file_size = &file_bs_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(ETALDcop_bootstrap_filename, "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, ETALDcop_bootstrap_filename);
			return errno;
		}
	}
	else
	{
		fhgi = &fhpg;
		file_size = &file_pg_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(ETALDcop_sections_filename, "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, ETALDcop_sections_filename);
			return errno;
		}
	}

	if (do_get_file_size == 1)
	{
		/* read size of file */
		if (fseek(*fhgi, 0, SEEK_END) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_END) %d\n", errno);
			return errno;
		}
		if ((*file_size = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell end of file %d\n", errno);
			return errno;
		}
		if (fseek(*fhgi, 0, SEEK_SET) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_SET) %d\n", errno);
			return errno;
		}
	}

	/* set remaining bytes number */
	if (remainingByteNum != NULL)
	{
		if ((file_position = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell %d\n", errno);
			return errno;
		}
		*remainingByteNum = (*file_size - file_position);
	}

	/* read requestedByteNum bytes in file */
	retfread = fread(block, 1, requestedByteNum, *fhgi);
	*returnedByteNum = (tU32) retfread;
	if (*returnedByteNum != requestedByteNum)
	{
		if (ferror(*fhgi) != 0)
		{
			/* error reading file */
			if (isBootstrap != 0)
			{
				printf("Error %d reading file %s\n", errno, ETALDcop_bootstrap_filename);
			}
			else
			{
				printf("Error %d reading file %s\n", errno, ETALDcop_sections_filename);
			}
			clearerr(*fhgi);
			fclose(*fhgi);
			*fhgi = NULL;
			return -1;
		}
	}

	/* Close file if EOF */
	if (feof(*fhgi) != 0)
	{
		/* DCOP bootstrap or flash program successful */
		clearerr(*fhgi);
		fclose(*fhgi);
		*fhgi = NULL;
	}
	return 0;
}

#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE

#endif // HAVE_DAB


static tVoid etalDemoEndTest()
{
    ETAL_STATUS ret;

	/*
	  * Destroy all receivers
	  */

	if (ETAL_INVALID_HANDLE != hCurrentReceiver)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy main receiver\n");
		if ((ret = etal_destroy_receiver(&hCurrentReceiver)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}

	if (ETAL_INVALID_HANDLE != hSecondReceiver)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy secondary receiver\n");
		if ((ret = etal_destroy_receiver(&hSecondReceiver)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}
	return;
}

/***************************
 *
 * etalDemoSeek_CmdWaitResp sends seek command and wait response
 *
 **************************/
tSInt etalDemoSeek_CmdWaitResp(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, tU32 step, OSAL_tMSecond response_timeout)
{
    ETAL_STATUS ret;
	tSInt vl_res = OSAL_OK;
    OSAL_tEventMask vl_resEvent;
	
	  etalDemoPrintf(TR_LEVEL_COMPONENT, "***************  RADIO SEEKING ***************\n");
	  
	  	
	  if ((ret = etal_autoseek_start(receiver, vI_direction, step, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
	  {
		  etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR:etalDemoSeek_CmdWaitResp  etal_service_seek (%d)", ret);
		  return 1;
	  }
	  
	etaldemo_seek_on_going = true;

	etalDemo_ClearEvent(ETALDEMO_EVENT_SEEK_FINISHED);

	// Wait here 
	 vl_res = OSAL_s32EventWait (etalDemo_EventHandler,
							  ETALDEMO_EVENT_SEEK_WAIT_MASK, 
							  OSAL_EN_EVENTMASK_OR, 
							  response_timeout,
							  &vl_resEvent);
	
	etalDemo_ClearEventFlag(vl_resEvent);

	if (OSAL_ERROR == vl_res) 
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response error");
		}	
	
	if (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res) 
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response timeout");
		}	
	else if (vl_resEvent == ETALDEMO_EVENT_TUNED_RECEIVED_FLAG)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response event received \n");
		}
	else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response event received 0x%x, vl_res = %d\n", vl_resEvent, vl_res);
		}
	
    return vl_res;
}

/***************************
 *
 * etalDemoScan
 *
 **************************/

static tSInt etalDemoScan(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, ETAL_HANDLE *vl_hDatapathRds)
{
	ETAL_STATUS ret;
	tSInt vl_res = OSAL_OK;
	OSAL_tEventMask vl_resEvent;
	OSAL_tThreadID userInterruptThreadID;
	OSAL_trThreadAttribute userInterruptThread_attr;
	etalDemoThreadAttrTy threadAttr;

	tU32 scanFreq;

	if ((ret = etaltml_scan_start(receiver, ETAL_DEMO_SCAN_PLAY_TIME, vI_direction, ETAL_DEMO_FREQ_STEP)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etalDemoScan etaltml_scan_start (%d)", ret);
		return 1;
	}

// abort scan on user key press
// Need one thread alive for keyboard scanning, then calling etaltml_scan_stop

	threadAttr.hReceiver = receiver;
	threadAttr.mode = ETALDEMO_MODE_SCAN;

	userInterruptThread_attr.szName = "USER_INT_THREAD";
	userInterruptThread_attr.u32Priority = OSAL_C_U32_THREAD_PRIORITY_NORMAL;
	userInterruptThread_attr.s32StackSize = 4096;
	userInterruptThread_attr.pfEntry = (tPVoid)etalDemo_UserInt_ThreadEntry;
	userInterruptThread_attr.pvArg = (tPVoid)&threadAttr;

	vl_res = OSAL_ThreadSpawn(&userInterruptThread_attr);
	if (OSAL_ERROR != vl_res)
	{
		userInterruptThreadID = vl_res;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "Can't spawn user interrupt thread");
		return vl_res;
	}

// check for ETALDEMO_EVENT_SCAN_FINISHED_FLAG before exiting the loop

	do
	{

		// Wait here
		vl_res = OSAL_s32EventWait (etalDemo_EventHandler,
							  ETALDEMO_EVENT_SCAN_WAIT_MASK,
							  OSAL_EN_EVENTMASK_OR,
							  ETAL_DEMO_SEEK_FM_WAIT_TIME,
							  &vl_resEvent);

		etalDemo_ClearEventFlag(vl_resEvent);

		if (OSAL_ERROR == vl_res)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_scan response error");
		}
		if (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_scan response timeout");
		}
		else if (vl_resEvent & ETALDEMO_EVENT_SCAN_FOUND_FLAG)
		{
			// display Frequency + RadioText when station is found

			etal_get_receiver_frequency(receiver, &scanFreq);

			etalDemoPrintf(TR_LEVEL_COMPONENT, "Frequency %d\n", scanFreq);

			etalDemo_startRadioText(receiver, vl_hDatapathRds);
		}
		else if (vl_resEvent & ETALDEMO_EVENT_SCAN_FINISHED_FLAG)
		{
			OSAL_s32ThreadDelete(userInterruptThreadID);
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_scan finished event received \n");
		}
		else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_scan response event received 0x%x, vl_res = %d\n", vl_resEvent, vl_res);
		}
	}while ((vl_resEvent & ETALDEMO_EVENT_SCAN_FINISHED_FLAG) == 0);

    return vl_res;
}

/***************************
 *
 * etalDemoLearn
 *
 **************************/

static tSInt etalDemoLearn(ETAL_HANDLE receiver, EtalFrequencyBand vI_band)
{
	ETAL_STATUS ret;
	tSInt vl_res = OSAL_OK;
	OSAL_tEventMask vl_resEvent;
	OSAL_tThreadID userInterruptThreadID;
	OSAL_trThreadAttribute userInterruptThread_attr;
	etalDemoThreadAttrTy threadAttr;
	int i;
	tBool vl_continue = false;
	int Service = 0;
	etalSeekDirectionTy vl_direction;
	etalDemoModeTy vl_mode;
	tBool vl_serviceSelectLoop = false;
	char line[256];
	tU32 step = ETAL_DEMO_FREQ_STEP;

	etaldemo_learn_freq_nb = 0;
	

	if (ETAL_BAND_AM == vI_band)
	{
		step = ETAL_DEMO_AM_FREQ_STEP;
	}

	if ((ret = etaltml_learn_start(receiver, vI_band, step, ETAL_LEARN_MAX_NB_FREQ, normalMode, etaldemo_learn_list)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etalDemoLearn etaltml_learn_start (%d)", ret);
		return 1;
	}

// abort learn on user key press
// Need one thread alive for keyboard scanning, then calling etaltml_learn_stop

	threadAttr.hReceiver = receiver;
	threadAttr.mode = ETALDEMO_MODE_LEARN;

	userInterruptThread_attr.szName = "USER_INT_THREAD";
	userInterruptThread_attr.u32Priority = OSAL_C_U32_THREAD_PRIORITY_NORMAL;
	userInterruptThread_attr.s32StackSize = 4096;
	userInterruptThread_attr.pfEntry = (tPVoid)etalDemo_UserInt_ThreadEntry;
	userInterruptThread_attr.pvArg = (tPVoid)&threadAttr;

	vl_res = OSAL_ThreadSpawn(&userInterruptThread_attr);
	if (OSAL_ERROR != vl_res)
	{
		userInterruptThreadID = vl_res;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "Can't spawn user interrupt thread");
		return vl_res;
	}

// check for ETALDEMO_EVENT_LEARN_FINISHED_FLAG before exiting the loop

	do
	{

		// Wait here
		vl_res = OSAL_s32EventWait (etalDemo_EventHandler,
							  ETALDEMO_EVENT_LEARN_WAIT_MASK,
							  OSAL_EN_EVENTMASK_OR,
							  ETAL_DEMO_SEEK_FM_WAIT_TIME,
							  &vl_resEvent);

		etalDemo_ClearEventFlag(vl_resEvent);

		if (OSAL_ERROR == vl_res)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_learn response error");
		}
		if (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_learn response timeout");
		}
		else if (vl_resEvent & ETALDEMO_EVENT_LEARN_FINISHED_FLAG)
		{
			OSAL_s32ThreadDelete(userInterruptThreadID);
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_learn finished event received \n");
		}
		else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_learn response event received 0x%x, vl_res = %d\n", vl_resEvent, vl_res);
		}
	}while ((vl_resEvent & ETALDEMO_EVENT_LEARN_FINISHED_FLAG) == 0);

	if (etaldemo_learn_freq_nb)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Learned %d frequencies\n", etaldemo_learn_freq_nb);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Freq List\n");
		for (i=0; i<etaldemo_learn_freq_nb; i++)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "%d: %dkHz\t", i+1, etaldemo_learn_list[i].m_frequency);
			if (((i+1)%5==0) && (i>0))
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "\n");
			}
		}
	}
	etalDemoPrintf(TR_LEVEL_COMPONENT, "\n");

	if (!etaldemo_learn_freq_nb)
		return vl_res;
	do
	{
		vl_continue = false;

		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 's' -> to select a frequency in current list\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'n' -> to select next frequency in current list\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'p' -> to select previous frequency in current list\n");

		if (!IsDemonstrationMode)
		{
		    if (fgets(line, sizeof(line), stdin)) vl_continue = true;
		}
		else
		{
#ifndef CONFIG_HOST_OS_WIN32
		    fd_CtrlFIFO = open(CtrlFIFO, O_RDONLY);
		    Sleep(10000);
		    if (read(fd_CtrlFIFO, line, sizeof(line))) vl_continue = true;
		    close(fd_CtrlFIFO);
#endif // !CONFIG_HOST_OS_WIN32
		}

		if (vl_continue)
		{
			switch (line[0])
			{

				case 's' :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_SERVICE_SELECT;
					break;
				case 'n' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				case 'p' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				default :
					vl_continue = false;
					vl_mode = ETALDEMO_MODE_INVALID;
					break;
			}

			if (vl_mode == ETALDEMO_MODE_SEEK)
			{
				/* This is the case of 'next service' or 'previous service' command
				 * It's looping around service list within the current ensemble
				 */
			    if (vl_direction)
			    {
				    //down
				    if (Service == 0)
				    {
				        Service = etaldemo_learn_freq_nb - 1;
				    }
				    else Service--;
			    }
			    else
			    {
				    //up
				    if (Service == (etaldemo_learn_freq_nb - 1))
				    {
				        Service = 0;
				    }
				    else Service++;
			    }
				
			}
			
			if ((!IsDemonstrationMode) && (vl_mode == ETALDEMO_MODE_SERVICE_SELECT  || vl_mode == ETALDEMO_MODE_SEEK))
			{
				if (vl_mode == ETALDEMO_MODE_SEEK )
				{
					ret = etal_tune_receiver(receiver, etaldemo_learn_list[Service].m_frequency);
					if (ret != ETAL_RET_SUCCESS)
					{
				        	etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver (%d)\n", ret);
					}
				}
				else
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the frequency ID to select frequency in the list : --> \n");
					do
					{
						if (fgets(line, sizeof(line), stdin))
						{
							if (1 == sscanf(line, "%d", &Service))
							{
								if ((Service > etaldemo_learn_freq_nb)||(Service <= 0))
								{
									etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong frequency selection !\n");
									etalDemoPrintf(TR_LEVEL_COMPONENT, "Please enter your selection:\n");
									vl_serviceSelectLoop = true;
								}
								else
								{
									/* i can be safely used */
									Service--;
									ret = etal_tune_receiver(receiver, etaldemo_learn_list[Service].m_frequency);
									if (ret != ETAL_RET_SUCCESS)
									{
								        	etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver (%d)\n", ret);
									}
									vl_serviceSelectLoop = false;
								}
							}
						}
					}while(vl_serviceSelectLoop);
				}
				etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a frequency from the list, frequency ID = %d, frequency = %d\n",Service+1,etaldemo_learn_list[Service].m_frequency);
			}

		}

	}while(vl_continue);
    return vl_res;
}

/***************************
 *
 * etalDemo_UserInt_ThreadEntry
 *
 **************************/
static tVoid etalDemo_UserInt_ThreadEntry(tPVoid param)
{
	etalDemoThreadAttrTy *ThreadAttr;
	etalDemoModeTy Mode;
	ETAL_HANDLE InScanReceiver;
	char line[256];

	ThreadAttr = (etalDemoThreadAttrTy*) param;
	InScanReceiver = ThreadAttr->hReceiver;
	Mode = ThreadAttr->mode;

	if (Mode == ETALDEMO_MODE_SCAN)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** HIT ANY KEY TO STOP SCANNING ***************\n");

		fgets(line, sizeof(line), stdin);

		etaltml_scan_stop(InScanReceiver, lastFrequency);
	}
	else if (Mode == ETALDEMO_MODE_LEARN)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** HIT ANY KEY TO STOP LEARNING ***************\n");

		fgets(line, sizeof(line), stdin);

		etaltml_learn_stop(InScanReceiver, initialFrequency);
	}

	// do not kill the thread
	// it will be killed on the FINISHED reception

	while(1)
	{
		Sleep(USLEEP_ONE_SEC *1);
	}
	// OSAL_vThreadExit();
}

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
// etalDemo_SFEventHandling
static tVoid etalDemo_SFEventHandling(EtalTuneServiceIdStatus *pI_SFInformation)
{

	// Print information
	static ETAL_HANDLE hDatapathData = ETAL_INVALID_HANDLE;

	if (false == pI_SFInformation->m_IsFoundstatus)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_SFEventHandling : Service Not Found \n");
		return;
	}
	else
	{
		if ((tU32)0x00FFFFFF != pI_SFInformation->m_Ueid)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_SFEventHandling : DAB SELECTED\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t Frequency %d, Ueid = 0x%x, Sid = 0x%x\n",
					pI_SFInformation->m_freq,
					pI_SFInformation->m_Ueid,
					pI_SFInformation->m_SidPi);
		}
		else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_SFEventHandling : FM SELECTED\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t Frequency %d, Sid = 0x%x\n",
					pI_SFInformation->m_freq,
					pI_SFInformation->m_SidPi);
		}

		if (ETAL_INVALID_HANDLE != hInitialDatapath)
		{
			etal_destroy_datapath(&hInitialDatapath);
			hInitialDatapath = ETAL_INVALID_HANDLE;
		}
		etalDemo_startRadioText(pI_SFInformation->m_receiverHandle, &hDatapathData);
	}
}
#endif

/***************************
 *
 * etalDemo_userNotificationHandler
 *
 **************************/
static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus)
{
#ifndef CONFIG_HOST_OS_WIN32
	char XMLstring[512];
#endif

	if (dwEvent == ETAL_INFO_TUNE)
	{
		// if we are in SEEK we should not care about this one

		EtalTuneStatus *status = (EtalTuneStatus *)pstatus;
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune info event, Frequency %d, good station found %d\n", 
						status->m_stopFrequency, 
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND);
		
#ifdef HAVE_HD
		etaldemo_HD_status = 0;

		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_SYNC)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "HD signal found\n");
			etaldemo_HD_status = 1;
		}

		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "HD digital audio acquired\n");
			etaldemo_HD_status = 2;
		}

		if (IsDemonstrationMode)
		{
#ifndef CONFIG_HOST_OS_WIN32
			fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
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
#endif
		}
#endif
		if (((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND)
			||
			(status->m_stopFrequency == etaldemo_seek_frequency_start)
			)
			{

			etalDemo_PostEvent(ETALDEMO_EVENT_TUNED_RECEIVED);
			}
	}
	else if (dwEvent == ETAL_INFO_SEEK)
	{
		EtalSeekStatus *status = (EtalSeekStatus *)pstatus;

		//HD mod: consider bits for HD
		etaldemo_HD_status = 0;

		if (IsDemonstrationMode)
		{
#ifndef CONFIG_HOST_OS_WIN32
			fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
			if ((status->m_frequency >= ETAL_BAND_FMJP_MIN) && (status->m_frequency <= ETAL_BAND_FMEU_MAX))
			{
				lastFreqUsed = status->m_frequency;
				if (etaldemo_HD_status == 0)
				{
					snprintf(XMLstring, sizeof(XMLstring),
						"<info><Band></Band><Frequency>%d</Frequency><HDStatus></HDStatus><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
						lastFreqUsed);
				}
				write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
				close(fd_RadioText);
			}
#endif
		}

		if (((true == status->m_frequencyFound)
			||
			(status->m_frequency == etaldemo_seek_frequency_start)
			||
			(true == status->m_fullCycleReached))
			&&
			(ETAL_SEEK_FINISHED == status->m_status)
			)
		{
			// found or end of  the seek
			etalDemoPrintf(TR_LEVEL_COMPONENT, "AutoSeek info event, Frequency %d, good station found %d, status = %d\n", 
						status->m_frequency, 
						status->m_frequencyFound,
						status->m_status);
			
			etaldemo_seek_on_going = false;
			etalDemo_PostEvent(ETALDEMO_EVENT_SEEK_FINISHED);
		}

	}
	else if (dwEvent == ETAL_INFO_SCAN)
	{
		EtalScanStatusTy *status = (EtalScanStatusTy *)pstatus;

		if (true == status->m_frequencyFound)
		{
			// found during scan
			etalDemoPrintf(TR_LEVEL_COMPONENT, "Scan info event, good station found at Frequency %d\n",
						status->m_frequency);
			etalDemo_PostEvent(ETALDEMO_EVENT_SCAN_FOUND);
		}
		if (ETAL_SCAN_FINISHED == status->m_status)
		{
			etalDemo_PostEvent(ETALDEMO_EVENT_SCAN_FINISHED);
		}
	}
	else if (dwEvent == ETAL_INFO_LEARN)
	{
		EtalLearnStatusTy *status = (EtalLearnStatusTy *)pstatus;

		if ((ETAL_LEARN_FINISHED == status->m_status) || (ETAL_LEARN_ERROR == status->m_status))
		{
			etaldemo_learn_freq_nb = status->m_nbOfFrequency;
			etalDemo_PostEvent(ETALDEMO_EVENT_LEARN_FINISHED);
		}
	}
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	else if (dwEvent == ETAL_INFO_SEAMLESS_ESTIMATION_END)
	{
		EtalSeamlessEstimationStatus *seamless_estimation_status = (EtalSeamlessEstimationStatus *)pstatus;

		if (NULL != pstatus)
			{

			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_userNotificationHandler : Seamless estimation complete, Status=%d\n",
					seamless_estimation_status->m_status);
			}
	}
	else if (dwEvent == ETAL_INFO_SEAMLESS_SWITCHING_END)
	{
		EtalSeamlessSwitchingStatus *seamless_switching_status = (EtalSeamlessSwitchingStatus *)pstatus;

		if (NULL != pstatus)
			{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_userNotificationHandler : Seamless switching complete, status=%d\n",
					seamless_switching_status->m_status);
			}
	}
	else if (dwEvent == ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO)
	{

		EtalTuneServiceIdStatus *pl_TuneServiceIdStatus = (EtalTuneServiceIdStatus *) pstatus;

		etalDemo_SFEventHandling(pl_TuneServiceIdStatus);

		hCurrentReceiver = pl_TuneServiceIdStatus->m_receiverHandle;

	}
#endif
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Unexpected event %d\n", dwEvent);
	}
}

/***************************
 *
 * etalDemo_parseParameters
 *
 **************************/
int etalDemo_parseParameters(int argc, char **argv, EtalFrequencyBand *pI_Band, tBool *pO_isSeek, tU32 *pO_freq, tU32 *pO_BGfreq)
{
#define ETAL_DEMO_BAD_PARAM -1
#define ETAL_DEMO_RETURN_OK 0


	int vl_res = ETAL_DEMO_RETURN_OK;
	int i = 0;
	
// Let's set the default parameters
	*pI_Band = ETAL_BAND_UNDEF;
	*pO_isSeek = true;
	*pO_freq = 0;
	*pO_BGfreq = 0;

	for (i=1; i<argc; i++)
	{
#ifndef CONFIG_HOST_OS_WIN32
		if (argv[i][0] == 'p')
		{
			char cmd[300];
			IsDemonstrationMode = true;
			RDS_on = false;
			sprintf(cmd, "if [ -f %s ]; then rm %s ; fi ; if [ -f %s ]; then rm %s ; fi",
				CtrlFIFO, CtrlFIFO, RadioTextFile, RadioTextFile);
			system(cmd);
			mkfifo(CtrlFIFO, 0666);
		}
#endif // !CONFIG_HOST_OS_WIN32
	}

	if(!IsDemonstrationMode)
	{
		for (i=1; i<argc; i++)
		{
			switch (argv[i][0])
			{
				case 'f':
#ifdef HAVE_FM
					*pI_Band = ETAL_BAND_FMEU;
					if (0 == *pO_freq) *pO_freq = ETAL_BAND_FMEU_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return ETAL_DEMO_BAD_PARAM;
#endif
					break;
				
				case 'F':
#ifdef HAVE_FM
					*pI_Band = ETAL_BAND_FMEU;
					if (0 == *pO_freq) *pO_freq = ETAL_BAND_FMEU_MIN;
						earlyAudio = true;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return ETAL_DEMO_BAD_PARAM;
#endif
												
					break;

				case 'j':
#ifdef HAVE_FM
					*pI_Band = ETAL_BAND_FMJP;
					if (0 == *pO_freq) *pO_freq = ETAL_BAND_FMJP_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return ETAL_DEMO_BAD_PARAM;
#endif
					break;
				case 'u':
#ifdef HAVE_FM
					*pI_Band = ETAL_BAND_FMUS;
					if (0 == *pO_freq) *pO_freq = ETAL_BAND_FMUS_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return ETAL_DEMO_BAD_PARAM;
#endif
					break;
				case 'a':
#ifdef HAVE_AM
					*pI_Band = ETAL_BAND_AM;
					if (0 == *pO_freq) *pO_freq = ETAL_BAND_AM_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_AM was not activated during build\n");
					return ETAL_DEMO_BAD_PARAM;
#endif
					break;
				case 'h':
#ifdef HAVE_HD
					*pI_Band = ETAL_BAND_HD;
					if (0 == *pO_freq) *pO_freq = ETAL_BAND_FMUS_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_HD was not activated during build\n");
					return ETAL_DEMO_BAD_PARAM;
#endif
					break;
				case 'd':
#ifdef HAVE_DAB
					*pI_Band = ETAL_BAND_DAB3;
					if (0 == *pO_freq) *pO_freq = ETAL_START_DAB_FREQ;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_DAB was not activated during build\n");
					return ETAL_DEMO_BAD_PARAM;
#endif
					break;
				case 's':
					*pO_isSeek = true;
                    // TODO : check if frequency is valid (is a number and is in adequate range)
					if (i+1 < argc)
					{
                        if ((argv[i+1][0] >= '0') && (argv[i+1][0] <= '9'))
                        {
						*pO_freq = atoi(argv[i+1]);
						i++; //Skip parsing next parameter again
					}
                    }
					break;
				case 't':
					*pO_isSeek = false;
					// TODO : check if frequency is valid (is a number and is in adequate range)
					if (i+1 < argc)
					{
                        if ((argv[i+1][0] >= '0') && (argv[i+1][0] <= '9'))
                        {
						*pO_freq = atoi(argv[i+1]);
						i++; //Skip parsing next parameter again
					}
                    }
					break;
				case 'b':
					// TODO : check if frequency is valid (is a number and is in adequate range)
					if (i+1 < argc)
					{
                        if ((argv[i+1][0] >= '0') && (argv[i+1][0] <= '9'))
                        {
						*pO_BGfreq = atoi(argv[i+1]);
						i++; //Skip parsing next parameter again
					}
                    }
					break;
				case 'v':
					DCOP_is_NVMless = true;
					break;
				default:
					break;
			}
		}

		// Check if anything is missing
		if (*pI_Band == ETAL_BAND_UNDEF || (*pO_isSeek == false && *pO_freq == 0))
		{
			vl_res = ETAL_DEMO_BAD_PARAM;
		}

		if ( ETAL_DEMO_BAD_PARAM == vl_res)
		{
#if defined(HAVE_DAB)
			etalDemoPrintf(TR_LEVEL_COMPONENT, "Usage : %s [F|f|j|u|d] [s|t] [<freq>] [b] [<freq>] [v]\n", argv[0]);
#elif defined(HAVE_HD)
			etalDemoPrintf(TR_LEVEL_COMPONENT, "Usage : %s [F|f|j|u|h] [s|t] [<freq>]\n", argv[0]);
#else
			etalDemoPrintf(TR_LEVEL_COMPONENT, "Usage : %s [F|f|j|u] [s|t] [<freq>]\n", argv[0]);
#endif
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tF = started from Early FM radio (Europe band) as start\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tf = FM radio (Europe band) as start\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tj = FM radio (Japanese band) as start\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tu = FM radio (US band) as start\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\ta = AM radio (Europe band) as start\n");
#ifdef HAVE_HD
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\th = FM/HD radio (US band) as start\n");
#endif
#ifdef HAVE_DAB
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\td = DAB radio as start\n");
#endif
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\ts = start in seek/scan mode, defaut frequency tune on <freq>\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tt = selection by tuning on frequency <freq>\n");
#ifdef HAVE_DAB
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tb = tune secondary DAB on frequency <freq>\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tv = DCOP has no NVM\n");
#endif
		}
		else
		{
			if (true == *pO_isSeek)
			{
				switch(*pI_Band)
				{
					case ETAL_BAND_DAB3:
						etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer DAB, seek method \n");
						break;
					case ETAL_BAND_HD:
						etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer FM/HD, seek method \n");
						break;
					case ETAL_BAND_AM:
						etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer AM, seek method \n");
						break;
					default:
						etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer FM, seek method \n");
						break;
				}
			}
			else
			{
				switch(*pI_Band)
				{
					case ETAL_BAND_DAB3:
						etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer DAB, tune method on freq %d\n", *pO_freq);
						break;
					case ETAL_BAND_HD:
						etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer FM/HD, tune method on freq %d\n", *pO_freq);
						break;
					case ETAL_BAND_AM:
						etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer AM, tune method on freq %d\n", *pO_freq);
						break;
					default:
						etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer FM, tune method on freq %d\n", *pO_freq);
						break;
				}
			}
		}
	}
	
	return vl_res;
}

// event creation
// Init globals
tSInt etalDemo_initEvent()
{
	tSInt vl_res;


	vl_res = OSAL_s32EventCreate ((tCString)"EtalDemo", &etalDemo_EventHandler);

	etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_initEvent : ret %d\n", vl_res);
		
	return(vl_res);

 

}

tSInt etalDemo_PostEvent(tU8 event)
{
	tSInt vl_res;

	etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_PostEvent : postEvent %d\n", event);
		
	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
                       ((tU32)0x01 << event), OSAL_EN_EVENTMASK_OR);

	return(vl_res);

}

tSInt etalDemo_ClearEvent(tU8 event)
{
	tSInt vl_res;

	
		// Clear old event if any (this can happen after a stop)


	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
						  (~((tU32)0x01 << event)), OSAL_EN_EVENTMASK_AND);

	return(vl_res);

}

tSInt etalDemo_ClearEventFlag(OSAL_tEventMask event)
{
	tSInt vl_res;

	
	// Clear old event if any (this can happen after a stop)


	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
						  (~event), OSAL_EN_EVENTMASK_AND);

	return(vl_res);

}

/***************************
 *
 * printRadioText
 *
 **************************/
static void etalDemo_printRadioText(char * prefix, EtalTextInfo *radio_text)
{
	char XMLstring[512];

    if ((!IsDemonstrationMode)&&(RDS_on))
    {
        if ((radio_text->m_serviceNameIsNew) || (radio_text->m_currentInfoIsNew))
        {
            printf("%s Radio text Broadcast Standard: %s\n", prefix, standard2Ascii(radio_text->m_broadcastStandard));
        }
        if (radio_text->m_serviceNameIsNew)
        {
            printf("%s Radio text Service name: %s, charset: %d\n", prefix, radio_text->m_serviceName,radio_text->m_serviceNameCharset);
        }

        if (radio_text->m_currentInfoIsNew)
        {
            printf("%s Radio text Current Info: %s, charset: %d\n", prefix, radio_text->m_currentInfo, radio_text->m_currentInfoCharset);
        }
    }
    else
    {
		if (RDS_on)
		{
            snprintf(XMLstring, sizeof(XMLstring),
				"<info><Band></Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><HDStatus></HDStatus>"\
				"<BroadcastStandard>%s</BroadcastStandard><ServiceName>%s</ServiceName><RadioText>%s</RadioText></info>\n",
				lastFreqUsed, tunedEnsemble,
			standard2Ascii(radio_text->m_broadcastStandard), radio_text->m_serviceName, radio_text->m_currentInfo);
		}
		else
		{
            snprintf(XMLstring, sizeof(XMLstring),
				"<info><Band></Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><HDStatus></HDStatus>"\
				"<BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
			lastFreqUsed, tunedEnsemble);
		}

#ifndef CONFIG_HOST_OS_WIN32
        fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
        write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
        close(fd_RadioText);
#endif
    }
}

/***************************
 *
 * startRadioText
 *
 **************************/
static int etalDemo_startRadioText(ETAL_HANDLE vl_handle, ETAL_HANDLE *vl_hDatapathRds)
{
	int ret;
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	EtalTextInfo vl_Radiotext;
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)

	EtalDataPathAttr vl_dataRadioTextPathAttr;

	// manage the RadioText

	/*
	 * [Optional] set up an event to get signalled when new Radio Text Data are available.
	 * since the (dynamic info
	 */
	if (ETAL_INVALID_HANDLE != vl_handle)
	{
		// stop prior radioText
		etaltml_stop_textinfo(vl_handle);
	}

    if (ETAL_INVALID_HANDLE != *vl_hDatapathRds)
    {
		// destroy the datapath
		etal_destroy_datapath(vl_hDatapathRds);
	}

	*vl_hDatapathRds = ETAL_INVALID_HANDLE;
	memset(&vl_dataRadioTextPathAttr, 0x00, sizeof(EtalDataPathAttr));
	vl_dataRadioTextPathAttr.m_receiverHandle = vl_handle;
	vl_dataRadioTextPathAttr.m_dataType = ETAL_DATA_TYPE_TEXTINFO;
	vl_dataRadioTextPathAttr.m_sink.m_context = (void *)0;
	vl_dataRadioTextPathAttr.m_sink.m_BufferSize = sizeof(EtalTextInfo);
	vl_dataRadioTextPathAttr.m_sink.m_CbProcessBlock = etalDemo_RadiotextCallback;

	if ((ret = etal_config_datapath(vl_hDatapathRds, &vl_dataRadioTextPathAttr)) != ETAL_RET_SUCCESS)
		{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_config_datapath for Radiotext (%d)\n", ret);
		return ret;
		}

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	if ((ret = etaltml_start_textinfo(vl_handle)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etaltml_start_textinfo (%d)\n", ret);
		return ret;
	}

	// radio Text notification will happen only if something new
	// so get current
	etaltml_get_textinfo(vl_handle, &vl_Radiotext);
	etalDemo_printRadioText("  etalDemo_currentRadioTextInfo : ", &vl_Radiotext);
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)

	return ret;
}

/***************************
 *
 * etalTestRadiotextCallback
 *
 **************************/

void etalDemo_RadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalDemo_printRadioText("  etalDemo_RadioTextInfo : ", (EtalTextInfo *) pBuffer);
}

/***************************
 *
 * etalDemoRDSCallback
 *
 **************************/
static void etalDemoRDSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	EtalRDSData *prdsfm;

	prdsfm = (EtalRDSData *) pBuffer;
	
	etalDemo_PrintRDS(prdsfm);
}


/***************************
 *
 * ETAL_PrintRDS
 *
 **************************/
 /*!
 * \brief		Prints a decoded RDS data container
 * \param[in]	rds - pointer to a decoded RDS data container
 * \callgraph
 * \callergraph
 */
static tVoid etalDemo_PrintRDS(EtalRDSData *rds)
{
	tU32 i;

	if (rds->m_validityBitmap == 0)
	{
		return;
	}
	etalDemoPrintf(TR_LEVEL_SYSTEM,"*************** RDS DATA DECODING ***************\n");
	
	etalDemoPrintf(TR_LEVEL_SYSTEM,"RDS decoded data 0x%03x\n", rds->m_validityBitmap);

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"PI          : 0x%x\n", rds->m_PI);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_DI) == ETAL_DECODED_RDS_VALID_DI)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"DI          : 0x%x\n", rds->m_DI);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"PS          : %.*s\n",  ETAL_DEF_MAX_PS_LEN, rds->m_PS);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"RT          : %.*s\n",  ETAL_DEF_MAX_RT_LEN, rds->m_RT);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TOM) == ETAL_DECODED_RDS_VALID_TOM)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"Time(hour)  : %d\n", rds->m_timeHour);
	etalDemoPrintf(TR_LEVEL_SYSTEM,"Time(min)   : %d\n", rds->m_timeMinutes);
	etalDemoPrintf(TR_LEVEL_SYSTEM,"Time(offs)  : %d\n", rds->m_offset);
	etalDemoPrintf(TR_LEVEL_SYSTEM,"Time(MJD)   : %d\n", rds->m_MJD);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_AF) == ETAL_DECODED_RDS_VALID_AF)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"AFListPI    : 0x%x\n", rds->m_AFListPI);
	etalDemoPrintf(TR_LEVEL_SYSTEM,"AFListLen   : %d\n", rds->m_AFListLen);
	etalDemoPrintf(TR_LEVEL_SYSTEM,"AFList      : \n");
	for (i = 0; i < rds->m_AFListLen; i++)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"%6d MHz\n",  87500 + (100 * (tU32)rds->m_AFList[i]));
	}
	}
	
	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PTY) == ETAL_DECODED_RDS_VALID_PTY)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"PTY         : %d\n", rds->m_PTY);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TP) == ETAL_DECODED_RDS_VALID_TP)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"TP          : %d\n", rds->m_TP);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TA) == ETAL_DECODED_RDS_VALID_TA)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"TA          : %d\n", rds->m_TA);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_MS) == ETAL_DECODED_RDS_VALID_MS)
	{
	etalDemoPrintf(TR_LEVEL_SYSTEM,"MS          : %d\n", rds->m_MS);
	}

	etalDemoPrintf(TR_LEVEL_SYSTEM,"*************** RDS DATA DECODING DONE ***************\n");
		
}

static void etalDemo_FMAMQualityMonitorCallback(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	if ((!IsDemonstrationMode)&&(QualityMonitor_on))
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: FM/AM BBFieldStrength is %d (dBuV)\n", pQuality->EtalQualityEntries.amfm.m_BBFieldStrength);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: FM/AM RFFieldStrength is %d (dBuV)\n", pQuality->EtalQualityEntries.amfm.m_RFFieldStrength);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: FM/AM FrequencyOffset is %d \n", pQuality->EtalQualityEntries.amfm.m_FrequencyOffset);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: FM/AM ModulationDetector is %d \n", pQuality->EtalQualityEntries.amfm.m_ModulationDetector);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: FM/AM AdjacentChannel is %d \n", pQuality->EtalQualityEntries.amfm.m_AdjacentChannel);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: FM/AM SNR is %d \n", pQuality->EtalQualityEntries.amfm.m_SNR);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: FM/AM CoChannel is %d \n", pQuality->EtalQualityEntries.amfm.m_coChannel);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: FM/AM StereoMonoReception is %d \n", pQuality->EtalQualityEntries.amfm.m_StereoMonoReception);
	}
}

static void etalDemo_DABQualityMonitorCallback(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	if ((!IsDemonstrationMode)&&(QualityMonitor_on))
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: DAB BBFieldStrength is %d(dBuV)\n", pQuality->EtalQualityEntries.dab.m_BBFieldStrength);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: DAB RFFieldStrength is %d(dBuV)\n", pQuality->EtalQualityEntries.dab.m_RFFieldStrength);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: DAB FicBitErrorRatio is %d\n", pQuality->EtalQualityEntries.dab.m_FicBitErrorRatio);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: DAB MscBitErrorRatio is %d\n", pQuality->EtalQualityEntries.dab.m_MscBitErrorRatio);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Quality callback: DAB isValidMscBitErrorRatio is %d\n", pQuality->EtalQualityEntries.dab.m_isValidMscBitErrorRatio);
	}
}


static ETAL_HANDLE monitor_amfm_handle;

static void etalDemo_stopQualityMonitor(void)
{
	ETAL_STATUS ret;

	if ((!IsDemonstrationMode) && (QualityMonitor_on))
	{
		QualityMonitor_on = false;

		//etal_destroy_reception_quality_monitor(&monitor_amfm_handle);
		if ((ret = etal_destroy_reception_quality_monitor(&monitor_amfm_handle)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_reception_quality_monitor (%d)\n", ret);
			return;
		}
	}

}
static void etalDemo_startQualityMonitor(ETAL_HANDLE vl_handle, EtalFrequencyBand vI_band)
{
	EtalBcastQaIndicators MonitoredIndicator = EtalQualityIndicator_FmFieldStrength;

	if (vI_band == ETAL_DEMO_DAB_BAND)
	{
		//MonitoredIndicator = EtalQualityIndicator_DabFieldStrength;
		MonitoredIndicator = EtalQualityIndicator_DabMscBer;
	}
	else if (vI_band == ETAL_DEMO_FMAM_BAND)
	{
		MonitoredIndicator = EtalQualityIndicator_FmFieldStrength;
	}
	if ((!IsDemonstrationMode) && (!QualityMonitor_on))
	{
		QualityMonitor_on = true;

		EtalBcastQualityMonitorAttr monfm;
		EtalBcastQualityContainer qualfm;
		ETAL_HANDLE handle_amfm = vl_handle;
		//ETAL_HANDLE monitor_amfm_handle;
		ETAL_STATUS ret;
		/*
		* Read the signal strength, one shot
		*/
		
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Etal Demo Get %s Quality\n", (vI_band == ETAL_DEMO_DAB_BAND) ? "DAB" : "FM/AM" );
		if ((ret = etal_get_reception_quality(handle_amfm, &qualfm)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_reception_quality (%d)\n", ret);
			return;
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "RF signal strength = %d\n", qualfm.EtalQualityEntries.amfm.m_RFFieldStrength);

		/*
		 * Set up a quality monitor
		 * We want to be notified only when the FM Signal strength is higher than 10.0,
		 * and only every 1000ms
		 */

		etalDemoPrintf(TR_LEVEL_COMPONENT, "Creating %s Quality monitor\n", (vI_band == ETAL_DEMO_DAB_BAND) ? "DAB" : "FM/AM");
		monitor_amfm_handle = ETAL_INVALID_HANDLE;
		memset(&monfm, 0x00, sizeof(EtalBcastQualityMonitorAttr));
		monfm.m_receiverHandle = handle_amfm;
		monfm.m_CbBcastQualityProcess = ((vI_band == ETAL_DEMO_DAB_BAND) ? etalDemo_DABQualityMonitorCallback : etalDemo_FMAMQualityMonitorCallback);
		monfm.m_monitoredIndicators[0].m_MonitoredIndicator = MonitoredIndicator;
		monfm.m_monitoredIndicators[0].m_InferiorValue = ETAL_INVALID_MONITOR;
		monfm.m_monitoredIndicators[0].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
		monfm.m_monitoredIndicators[0].m_UpdateFrequency = ETAL_DEMO_QUALITY_PERIODICITY;  // millisec
		if ((ret = etal_config_reception_quality_monitor(&monitor_amfm_handle, &monfm)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_config_reception_quality_monitor (%d)\n", ret);
			return;
		}
	}
}

/***************************
 *
 * etalDemoSeekOrTuneDataCallback
 *
 **************************/
#ifdef ETAL_DEMO_CREATE_FIC_DATAPATH
static tVoid etalDemoSeekOrTuneDataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{


	etalDemoPrintf(TR_LEVEL_COMPONENT,"FIC callback received, packet size %d\n", dwActualBufferSize);

}
#endif
/***************************
 *
 * etalDemo_startRadioRDSDecoded
 *
 **************************/
static int etalDemo_startRadioRDSDecoded(ETAL_HANDLE vl_handle, ETAL_HANDLE *pO_hDatapathRds)
{
	int ret;

	EtalDataPathAttr vl_dataPathRDS;

	// manage the RadioText

	/*
	 * [Optional] set up an event to get signalled when new Radio Text Data are available.
	 * since the (dynamic info
	 */
	if (ETAL_INVALID_HANDLE != vl_handle)
	{
		// stop prior radioText
		etaltml_stop_decoded_RDS(vl_handle, ETAL_DECODED_RDS_VALID_ALL);
	}

    if (ETAL_INVALID_HANDLE != *pO_hDatapathRds)
    {
		// destroy the datapath
		etal_destroy_datapath(pO_hDatapathRds);
	}

	*pO_hDatapathRds = ETAL_INVALID_HANDLE;


	vl_dataPathRDS.m_receiverHandle = vl_handle;
	vl_dataPathRDS.m_dataType = ETAL_DATA_TYPE_FM_RDS;
	vl_dataPathRDS.m_sink.m_context = NULL;
	vl_dataPathRDS.m_sink.m_BufferSize = sizeof(EtalRDSData);
	vl_dataPathRDS.m_sink.m_CbProcessBlock = etalDemoRDSCallback;
	if ((ret = etal_config_datapath(pO_hDatapathRds, &vl_dataPathRDS)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS,"etal_config_datapath for (%d)\n", ret);
		return OSAL_ERROR;
	}

	/*
	 * start RDS reception
	 */
		if ((ret = etal_start_RDS(vl_handle, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS,"etal_start_RDS for(%d)\n", ret);
		return OSAL_ERROR;
	}
	/*
	 * start RDS decoding
	 */
	
	if ((ret = etaltml_start_decoded_RDS(vl_handle, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS,"etaltml_start_decoded_RDS for (%d)\n",  ret);
		return OSAL_ERROR;
	}


	return ret;
}

/***************************
 *
 * etalDemo_stopRadioRDSDecoded
 *
 **************************/
static int etalDemo_stopRadioRDSDecoded(ETAL_HANDLE vl_handle, ETAL_HANDLE vI_hDatapathRds)
{
	int ret;

    if (ETAL_INVALID_HANDLE != vI_hDatapathRds)
    {
		// destroy the datapath
		etal_destroy_datapath(&vI_hDatapathRds);
	}


	/*
	 * stop RDS decoding
	 */
	
	if ((ret = etaltml_stop_decoded_RDS(vl_handle, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS,"etaltml_stop_decoded_RDS for (%d)\n",  ret);
		return OSAL_ERROR;
	}


	return ret;
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
	EtalFrequencyBand vl_Band;
	tBool vl_isSeek;
	tU32 vl_freq, vl_BGfreq;
	
	if (etalDemo_parseParameters(argc, argv, &vl_Band, &vl_isSeek, &vl_freq, &vl_BGfreq))
	{
		return 1;
	}

	hCurrentReceiver = hInitialDatapath = hSecondReceiver = ETAL_INVALID_HANDLE;

	etalDemo_Appli_entryPoint(vl_Band, vl_isSeek, vl_freq, vl_BGfreq);

	return 0;
}


// Configure the audio path for Tuner
//
static void etalDemo_ConfigureAudioForTuner()
{
	// this is based on alsa mixer in Linux
	// This requires the appropriate audio fwk

	// ETAL audio select has been modified to support digital audio configuration,
	// so no need to care about digital audio or band choice anymore
	
#ifdef HAVE_HD
	int ret;
	EtalAudioInterfTy audioIf;

//HD mod: in this case, DCOP680 is the I2S master, so CMOST has to be configured as SAI slave

	/* Configure audio path on CMOST */
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif
	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_config_audio_path (%d)", ret);
		return;
	}
#endif // have_hd	

#ifdef CONFIG_BOARD_ACCORDO5
#if (!defined(CONFIG_DIGITAL_AUDIO)) && (!defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST))
	system("amixer -c 3 sset Source adcauxdac > /dev/null" );

	// select the audio channel
	system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
	if (!IsDemonstrationMode) system("amixer -c 3 sset \"Scaler Primary Media Volume Master\" 1200 > /dev/null");
#endif

#ifdef CONFIG_BOARD_ACCORDO2
	// select the audio source
	system("amixer -c 3 sset Source adcauxdac > /dev/null" );

	// select the audio channel
	system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
#if (defined(CONFIG_BOARD_ACCORDO2) || defined(CONFIG_BOARD_ACCORDO5))
	// Set the output volume 
	if (!IsDemonstrationMode) system("amixer -c 3 sset \"Volume Master\" 1200 > /dev/null");
#endif // CONFIG_BOARD_ACCORDO2 || CONFIG_BOARD_ACCORDO5
}

// vI_Band : indicate if we work on FM or DAB
// vI_modeIsSeek : indicates if we start by a tune or a seek
// vI_frequency (for tune case)
//
void etalDemo_Appli_entryPoint(EtalFrequencyBand vI_Band, tBool vI_modeIsSeek, tU32 vI_frequency, tU32 vI_BGfrequency)
{
	int ret;
	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;
	EtalHardwareAttr init_params;
	char line[256];
	tBool vl_continue = true;
	tBool looped = false;

#ifndef CONFIG_HOST_OS_WIN32
	char XMLstring[512];
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
	init_params.m_cbNotify = etalDemo_userNotificationHandler;

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
    if (true == DCOP_is_NVMless)
   	{
    	init_params.m_DCOPAttr.m_doDownload = TRUE;
		init_params.m_DCOPAttr.m_cbGetImage = etalDemo_GetDCOPImageCb;
        init_params.m_DCOPAttr.m_sectDescrFilename = ETALDcop_sections_filename;
   	}
#endif
		
		
	// Step init : 
	// 1st step : init ETAL
	// Then tuner 1
	// Then Tuner 2
	// then DCOP
	if (true == earlyAudio)
	{
		init_params.m_tunerAttr[0].m_isDisabled = TRUE;
		init_params.m_tunerAttr[1].m_isDisabled = TRUE;
		init_params.m_DCOPAttr.m_isDisabled = TRUE;
	}

	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_initialize (%d)\n", ret);
		return;
	}

		// boot 1st tuner
	if (true == earlyAudio)
	{
		init_params.m_tunerAttr[0].m_isDisabled = FALSE;

		if ((ret = etal_tuner_initialize(0, &init_params.m_tunerAttr[0], true)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_tuner_initialize (%d)\n", ret);
			return;
		}
	}

	
#ifdef CONFIG_MODULE_INTEGRATED
	if (true == earlyAudio)
	{
		init_params.m_tunerAttr[1].m_isDisabled = FALSE;
	
				
		if ((ret = etal_tuner_initialize(1, &init_params.m_tunerAttr[1], true)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_tuner_initialize (%d)\n", ret);
			return;
		
		}
	}
	
#endif

	if (true == earlyAudio)
	{	
		init_params.m_DCOPAttr.m_isDisabled = FALSE;
				 
		if ((ret = etal_dcop_initialize(&init_params.m_DCOPAttr, ETAL_DCOP_INIT_FULL)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_tuner_initialize (%d)\n", ret);
			return;
		}		
	}		


	// create an event to work on notification
	//
	etalDemo_initEvent();

	do
	{
		if ((IsDemonstrationMode) || (looped))
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "******************* BAND SELECTION ********************\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tf = FM radio (Europe band)\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tj = FM radio (Japanese band)\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tu = FM radio (US band)\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\ta = AM radio (Europe band)\n");
#ifdef HAVE_HD
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\th = FM/HD radio (US band)\n");
#endif
#ifdef HAVE_DAB
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\td = DAB radio\n");
#endif
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'q' -> proceed to the exit **** \n");
		}
		else
		{

				if ((false == IsDemonstrationMode) && (false == earlyAudio))
				{
					etalDemo_ConfigureAudioForTuner();
				}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		}

		if (IsDemonstrationMode)
		{
			//Destroy current receiver, if any
			etalDemoEndTest();
		}

		if (!IsDemonstrationMode)
		{
			if (fgets(line, sizeof(line), stdin)) vl_continue = true;
		}
		else
		{
#ifndef CONFIG_HOST_OS_WIN32
			fd_CtrlFIFO = open(CtrlFIFO, O_RDONLY);
			Sleep(10000);
			if (read(fd_CtrlFIFO, line, sizeof(line))) vl_continue = true;
			close(fd_CtrlFIFO);
#endif // !CONFIG_HOST_OS_WIN32
		}

		if (IsDemonstrationMode)
		{
#ifndef CONFIG_HOST_OS_WIN32
			// Create a new RadioTextfile to unblock A5 GUI
			fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
			snprintf(XMLstring, sizeof(XMLstring),
				"<info><Band></Band><Frequency></Frequency><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n");
			write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
			close(fd_RadioText);
#endif
		}

		if ((IsDemonstrationMode)  && (false == earlyAudio))
		{
			etalDemo_ConfigureAudioForTuner();
		}

		if ((IsDemonstrationMode) || (looped))
		{
			switch (line[0])
			{
				case 'f':
#ifdef HAVE_FM
					vI_Band = ETAL_BAND_FMEU;
					vI_frequency = ETAL_BAND_FMEU_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return;
#endif
					break;
				case 'j':
#ifdef HAVE_FM
					vI_Band = ETAL_BAND_FMJP;
					vI_frequency = ETAL_BAND_FMJP_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return;
#endif
					break;
				case 'u':
#ifdef HAVE_FM
					vI_Band = ETAL_BAND_FMUS;
					vI_frequency = ETAL_BAND_FMUS_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return;
#endif
					break;
				case 'a':
#ifdef HAVE_AM
					vI_Band = ETAL_BAND_AM;
					vI_frequency = ETAL_BAND_AM_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_AM was not activated during build\n");
					return;
#endif
					break;
				case 'h':
#ifdef HAVE_HD
					vI_Band = ETAL_BAND_HD;
					vI_frequency = ETAL_BAND_FMUS_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_HD was not activated during build\n");
					return;
#endif
					break;
				case 'd':
#ifdef HAVE_DAB
					vI_Band = ETAL_BAND_DAB3;
					vI_frequency = ETAL_START_DAB_FREQ;

#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_DAB was not activated during build\n");
					return;
#endif
					break;
				case 'q' :
					vl_continue = false;
					break;
				default:
					break;
			}
			vI_modeIsSeek = true;
		}

		if (vl_continue)
		{
			//Destroy current receiver, if any
			etalDemoEndTest();
			if ((ETAL_BAND_FMEU == vI_Band)||(ETAL_BAND_FMJP == vI_Band)||
				(ETAL_BAND_FMUS == vI_Band)||(ETAL_BAND_HD == vI_Band))
			{
#ifdef HAVE_FM
				if (true == vI_modeIsSeek)
				{
					vl_handle = etalDemo_FMSeekRadio(vI_frequency, vI_Band);
				}
				else
				{
					vl_handle = etalDemo_TuneFMFgRadio(vI_frequency, vI_Band);
				}
#endif
			}
			else if (ETAL_BAND_AM == vI_Band)
			{
#ifdef HAVE_AM
				if (true == vI_modeIsSeek)
				{
					vl_handle = etalDemo_AMSeekRadio(vI_frequency, vI_Band);
				}
				else
				{
					vl_handle = etalDemo_TuneAMRadio(vI_frequency, vI_Band);
				}
				
#endif
			}
			else if (ETAL_BAND_DAB3 == vI_Band)
			{				
#ifdef HAVE_DAB
				if (true == vI_modeIsSeek)
				{			
					vl_handle = etalDemo_DABRadioSeek(vI_frequency);
				}
				else
				{
					vl_handle = etalDemo_TuneDAB1Radio(vI_frequency);
					hSecondReceiver = etalDemo_TuneDAB2Radio(vI_BGfrequency);
				}
#endif
			}
			else
			{
				// not supported
			}
			hCurrentReceiver = vl_handle;
		}

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
    	if (!IsDemonstrationMode)
	    {
		    //now activate the SF

		    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		    etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'f' -> to activate SF (then any key to exit)\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'F' -> to activate SF in FM-FM only (then any key to exit)\n");
		    etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t anyother -> end test **** \n");

		    if (fgets(line, sizeof(line), stdin))
		    {
			    if (line[0] == 'F')
	    	        {
		                etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** SF FM-FM configuration ***************\n");

	    	            if ((ret = etaltml_SelectKindOfSwitchServiceFollowing(true, false, false)) != ETAL_RET_SUCCESS)
	        	        {
	            	        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etaltml_SelectKindOfSwitchServiceFollowing (%d)\n", ret);
	                	    return;
		                }

	    	            etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** SF FM-FM configured ***************\n");

						line[0] = 'f';
	        	    }
				
	            if (line[0] == 'f')
    	        {
	                etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** SF activation ***************\n");

    	            if ((ret = etaltml_ActivateServiceFollowing()) != ETAL_RET_SUCCESS)
        	        {
            	        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etaltml_ActivateServiceFollowing (%d)", ret);
                	    return;
	                }

    	            etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** SF activated ***************\n");
        	    }
		    }
	    }
#endif

		if (!IsDemonstrationMode)
		{
	    	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** Do you want to quit? (y/n) ***************\n");
			etalDemo_stopQualityMonitor();

		    if (fgets(line, sizeof(line), stdin))
		    {
		    	if (line[0] == 'y') 
				{	
					vl_continue = false;
		    	}
				else
				{
					vl_continue = true;
				}
				
		    }
			looped = true;
		}

	}while(true == vl_continue);
		
	// destroy handles
	etalDemoEndTest();
	
	/*
	 * Final cleanup
	 */
	
	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_deinitialize\n");
	}

    if (IsDemonstrationMode)
    {
#ifndef CONFIG_HOST_OS_WIN32
		unlink(CtrlFIFO);
#endif // !CONFIG_HOST_OS_WIN32
    }
	
	return;
}

