//!
//!  \file 		etaltest_rdsstrategy.c
//!  \brief 	<i><b> ETAL test, RDS strategy test, RDS Seek test. </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	SHENZHEN TUNER TEAM
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltml_api.h"

#include "etaltest.h"

#if defined (CONFIG_APP_TEST_RDS_SEEK) || defined (CONFIG_APP_TEST_RDS_AF_STRATEGY)

#include <stdio.h>
#include <string.h>
#include <errno.h>

// only for debug functions
#include "etalinternal.h"

#ifdef CONFIG_APP_TEST_RDS_SEEK
/* RDS seek semephore */
OSAL_tSemHandle etalTestRDSSeekSem = 0;
/* RDS seek status */
static EtalSeekStatus rdsSeekStatus;

tVoid etalTestRDSSeek_SetSeekResult(EtalSeekStatus * pSeekStatus)
{
	rdsSeekStatus = *pSeekStatus;
}
#endif


/***************************
 *
 * etalTestDoRDSConfig
 *
 **************************/
/*
 * Creates a valid RDS configuration 
 *
 * Returns:
 *  OSAL_OK if no error and valid handle is stored in *hReceiver
 *  OSAL_ERROR otherwise
 */
static tSInt etalTestDoRDSConfig(ETAL_HANDLE *hReceiver, tBool reconfigure, tU8 mode, ETAL_HANDLE hFrontend)
{
	EtalReceiverAttr attr;
	tChar standard[5] = "\0";
	ETAL_HANDLE hFrontend_local;
	EtalBcastStandard std;
	ETAL_STATUS ret;

	if (mode == ETAL_TEST_MODE_FM)
	{
		std = ETAL_BCAST_STD_FM;
        if (hFrontend == ETAL_INVALID_HANDLE)
        {
    		hFrontend_local = ETAL_FE_FOR_FM_TEST;
        }
        else
        {
            hFrontend_local = hFrontend;
        }
		OSAL_szStringCopy(standard, "FM");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		std = ETAL_BCAST_STD_HD_FM;
        if (hFrontend == ETAL_INVALID_HANDLE)
        {
            hFrontend_local = ETAL_FE_FOR_HD_TEST;
        }
        else
        {
            hFrontend_local = hFrontend;
        }
		OSAL_szStringCopy(standard, "HDFM");
	}
	else 
	{
		return OSAL_ERROR;
	}

	/* Create RDS receiver */

	if (hReceiver != NULL)
	{
		if ((!reconfigure) || (*hReceiver == ETAL_INVALID_HANDLE))
		{
			*hReceiver = ETAL_INVALID_HANDLE;
			etalTestPrint(TR_LEVEL_USER_4, "* Create RDS receiver for %s", standard);
		}
		else
		{
			etalTestPrint(TR_LEVEL_USER_4, "* Reconfiguring RDS receiver for %s", standard);
			if ((ret = etal_destroy_receiver(hReceiver)) != ETAL_RET_SUCCESS)
			{
				etalTestPrint(TR_LEVEL_ERRORS, "etal_destroy_receiver RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}

		}
		OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));
		attr.m_Standard = std;
		attr.m_FrontEnds[0] = hFrontend_local;
		attr.m_FrontEndsSize = 1;
		if ((ret = etal_config_receiver(hReceiver, &attr)) != ETAL_RET_SUCCESS)
		{
			etalTestPrint(TR_LEVEL_ERRORS, "etal_config_receiver RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}

	return OSAL_OK;
}


#endif // CONFIG_APP_TEST_RDS_SEEK || CONFIG_APP_TEST_RDS_AF_STRATEGY



/***************************
 *
 * etalTestRDSSeek
 *
 **************************/
tSInt etalTestRDSSeek(void)
{
#if defined( CONFIG_APP_TEST_RDS_SEEK) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)

#ifdef CONFIG_APP_TEST_FM
	EtalRDSData rdsfm;
#endif
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#ifdef  TUNER_DIGITAL_OUT
	EtalAudioInterfTy	vl_etalAudioInterfaceConfig;
#endif	

	EtalRDSSeekTy rdsSeekOption;
	EtalProcessingFeatures proc_features;
	tU32 waitTime;
	char line[256];

	
	tBool pass1 = TRUE;
	tBool pass2 = TRUE;
	tBool pass3 = TRUE;
	tBool pass4 = TRUE;
	tBool pass5 = TRUE;
	tBool pass6 = TRUE;

	tU32 freqMin  = 87500;
	tU32 freqMax = 108000;

	etalTestStartup();
	OSAL_pvMemorySet(&rdsfm, 0x00, sizeof(EtalRDSData));
	
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "<---RDS Seek Test Start--->");

//	Configure audio path if defined digital output
#ifdef  TUNER_DIGITAL_OUT
	memset(&vl_etalAudioInterfaceConfig, 0x00, sizeof(vl_etalAudioInterfaceConfig));
	vl_etalAudioInterfaceConfig.m_dac = false;                                        /* Enable / disable audio DAC */
	vl_etalAudioInterfaceConfig.m_sai_out = true;                                    /* Enable / disable audio SAI output */
	vl_etalAudioInterfaceConfig.m_sai_in = false;                                     /* Enable / disable audio SAI input */
	// now reserved
	//vl_etalAudioInterfaceConfig.m_output_sys_clock_gpio13 = false ;  // Outputs 2.9184MHz System Clock on GPIO13  to STA680 (DCOP) 
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	vl_etalAudioInterfaceConfig.m_sai_slave_mode = true;
#endif

	ret = etal_config_audio_path(0, vl_etalAudioInterfaceConfig);
	if (ret != ETAL_RET_SUCCESS) 
	{
		etalTestPrint(TR_LEVEL_USER_4, "Error: etal_config_audio_path err = 0x%x", ret);	
	}
#endif

	//Configure receiver
	if (etalTestDoRDSConfig(&handlefm, FALSE, ETAL_TEST_MODE_FM, ETAL_INVALID_HANDLE) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}
	
	//Change the receiver band to FM EU
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	ret = etal_change_band_receiver(handlefm, ETAL_BAND_FMEU, freqMin, freqMax, proc_features);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	} 

	//Select audio path
	ret = etal_audio_select(handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	//Tune to frequency	
	ret = etal_tune_receiver(handlefm, freqMin);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	/*
	 * start RDS decoding
	 */
	etalTestPrint(TR_LEVEL_USER_4, "* Start RDS decoding for FM *");
	if ((ret = etal_start_RDS(handlefm, FALSE, 0, ETAL_RDS_MODE, 0)) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}

	//Start RDS AF strategy
	etalTestPrint(TR_LEVEL_USER_4, "* Start RDS AF strategy for FM *");	
	ret = etaltml_RDS_AF(handlefm, handlefm, TRUE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	//semephores 
	if ((etalTestRDSSeekSem == 0) && 
		(OSAL_s32SemaphoreCreate("etalTestRDSSeekSem", &etalTestRDSSeekSem, 0) == OSAL_ERROR))
	{
		return OSAL_ERROR;
	}

	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");

	waitTime = 20000; 
	
	//RDS Seek, RDS station Seek;   (1)  RDS Station Seek;
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "RDS Seek Test(2) : RDS seek (None RDS station present).");	
	etalTestPrint(TR_LEVEL_USER_4, "- Tuned to frequency : %d kHz.", freqMin);
	etalTestPrint(TR_LEVEL_USER_4, "- Please set the signal to one frequency with RDS off (level > 30dBuV).");
	etalTestPrint(TR_LEVEL_USER_4, "- Expect not stop at the frequency during seek.");

#if defined (CONFIG_HOST_OS_LINUX)	
	etalTestPrint(TR_LEVEL_USER_4, "- Press a key to continue when ready");

	fgets(line, sizeof(line), stdin);
	
	etalTestPrint(TR_LEVEL_USER_4, "key pressed, let's continue");
#else
	etalTestPrint(TR_LEVEL_USER_4, "- RDS Seek will start in %d seconds.", waitTime /1000);

	OSAL_s32ThreadWait(waitTime);
#endif
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	memset(&rdsSeekStatus, 0, sizeof(EtalSeekStatus));
	rdsSeekOption.tpSeek = FALSE;
	rdsSeekOption.taSeek = FALSE;
	rdsSeekOption.ptySeek = FALSE;
	rdsSeekOption.ptyCode = 0xFF;
	rdsSeekOption.pi = 0;
	rdsSeekOption.piSeek = FALSE;

	ret = etaltml_RDS_seek_start(handlefm, cmdDirectionUp, 50, cmdAudioUnmuted, &rdsSeekOption);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	waitTime = 20000; 

	//Wait for notification
	OSAL_s32SemaphoreWait(etalTestRDSSeekSem, waitTime);
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");

	/* expeced not found the sation */
	if (rdsSeekStatus.m_frequencyFound)
	{
		etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed, RDS station found : %d kHz. RDS is on?", rdsSeekStatus.m_frequency);
		pass2 = FALSE;
	}
	else
	{
		if (rdsSeekStatus.m_fullCycleReached)
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : successed. fullCycleReached.");
		}
		else
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed. Seek not finished.");
			pass1 = FALSE;
		}
	}
	
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	ret = etal_autoseek_stop(handlefm, lastFrequency);
	if (ret != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(1000);
	/*  RDS Seek, RDS station Seek;   (1)  RDS Station Seek test end; */



	//Tune to frequency	
	ret = etal_tune_receiver(handlefm, freqMin);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	//RDS Seek, RDS station Seek;  (2)  RDS Non Station Seek;
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "RDS Seek Test(1) : RDS seek (One RDS station present).");	
	etalTestPrint(TR_LEVEL_USER_4, "- Tuned to frequency : %d kHz.", freqMin);
	etalTestPrint(TR_LEVEL_USER_4, "- Please set the signal to one frequency with RDS on (level > 30dBuV).");
	etalTestPrint(TR_LEVEL_USER_4, "- Expect to stop at the frequency.");
	etalTestPrint(TR_LEVEL_USER_4, "- RDS Seek will start in %d seconds.", waitTime /1000);

#if defined (CONFIG_HOST_OS_LINUX)	
		etalTestPrint(TR_LEVEL_USER_4, "- Press a key to continue when ready");
	
		fgets(line, sizeof(line), stdin);
		etalTestPrint(TR_LEVEL_USER_4, "key pressed, let's continue");
#else
		etalTestPrint(TR_LEVEL_USER_4, "- RDS Seek will start in %d seconds.", waitTime /1000);
	
		OSAL_s32ThreadWait(waitTime);
#endif

	etalTestPrint(TR_LEVEL_USER_4, "\r\n");

	memset(&rdsSeekStatus, 0, sizeof(EtalSeekStatus));
	rdsSeekOption.tpSeek = FALSE;
	rdsSeekOption.taSeek = FALSE;
	rdsSeekOption.ptySeek = FALSE;
	rdsSeekOption.ptyCode = 0xFF;
	rdsSeekOption.pi = 0;
	rdsSeekOption.piSeek = FALSE;

	ret = etaltml_RDS_seek_start(handlefm, cmdDirectionUp, 50, cmdAudioUnmuted, &rdsSeekOption);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	//Wait for notification
	OSAL_s32SemaphoreWait(etalTestRDSSeekSem, waitTime);
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");

	/* expeced found the sation */
	if (rdsSeekStatus.m_frequencyFound)
	{
		etalTestPrint(TR_LEVEL_USER_4, "- Test Result : successed, RDS station found : %d kHz.", rdsSeekStatus.m_frequency);
	}
	else
	{
		if (rdsSeekStatus.m_fullCycleReached)
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed. fullCycleReached.");
		}
		else
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed. Seek not finished.");
		}
		pass2 = FALSE;
	}

	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	ret = etal_autoseek_stop(handlefm, lastFrequency);
	if (ret != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(1000);
	/*  RDS Seek, RDS station Seek;   (2)  RDS Station Seek test end; */



	ret = etaltml_RDS_TA(handlefm, TRUE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
	//Tune to frequency
	ret = etal_tune_receiver(handlefm, freqMin);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	//RDS Seek, RDS station Seek;  (3)  RDS TA seek;  (enable/disable TA, TP)
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "RDS Seek Test(3) :  RDS TA seek (One RDS station present, TA, TP disabled) .");
	etalTestPrint(TR_LEVEL_USER_4, "- Tuned to frequency :  %d kHz.", freqMin);
	etalTestPrint(TR_LEVEL_USER_4, "- Please set the signal to one frequency with RDS on, TA off, TP off (level > 30dBuV).");
	etalTestPrint(TR_LEVEL_USER_4, "- Expect not stop at the frequency during seek.");
	
#if defined (CONFIG_HOST_OS_LINUX)	
		etalTestPrint(TR_LEVEL_USER_4, "- Press a key to continue when ready");
	
		fgets(line, sizeof(line), stdin);
		etalTestPrint(TR_LEVEL_USER_4, "key pressed, let's continue");
#else
		etalTestPrint(TR_LEVEL_USER_4, "- RDS Seek will start in %d seconds.", waitTime /1000);
	
		OSAL_s32ThreadWait(waitTime);
#endif

	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	memset(&rdsSeekStatus, 0, sizeof(EtalSeekStatus));
	rdsSeekOption.tpSeek = FALSE;
	rdsSeekOption.taSeek = TRUE;
	rdsSeekOption.ptySeek = FALSE;
	rdsSeekOption.ptyCode = 0xFF;
	rdsSeekOption.pi = 0;
	rdsSeekOption.piSeek = FALSE;

	ret = etaltml_RDS_seek_start(handlefm, cmdDirectionUp, 50, cmdAudioUnmuted, &rdsSeekOption);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	//Wait for notification
	OSAL_s32SemaphoreWait(etalTestRDSSeekSem, waitTime);
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	/* expeced found the sation */
	if (rdsSeekStatus.m_frequencyFound)
	{
		etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed, Station found : %d kHz.  Is the RDS TA/TP on?", rdsSeekStatus.m_frequency);
		pass3 = FALSE;
	}
	else
	{
		if (rdsSeekStatus.m_fullCycleReached)
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : successed. fullCycleReached.");
		}
		else
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed. Seek not finished. ");
			pass3 = FALSE;
		}
	}
	
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	ret = etal_autoseek_stop(handlefm, lastFrequency);
	if (ret != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}
	
	ret = etaltml_RDS_TA(handlefm, FALSE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(1000);
	/* RDS Seek, RDS station Seek;  (3)  RDS TA seek;  (enable/disable TA, TP)  test end */

	ret = etaltml_RDS_TA(handlefm, TRUE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	//Tune to frequency
	ret = etal_tune_receiver(handlefm, freqMin);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	//RDS Seek, RDS station Seek;  (4)  RDS TA seek;  (enable TA, TP)
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "RDS Seek Test(4) :  RDS TA seek (One RDS station present, TA or TP or both enabled).");	
	etalTestPrint(TR_LEVEL_USER_4, "- Tuned to frequency :  %d kHz.", freqMin);
	etalTestPrint(TR_LEVEL_USER_4, "- Please set the signal to one frequency with RDS on, TA on or TP on or both TA and TP on (level > 30dBuV).");
	etalTestPrint(TR_LEVEL_USER_4, "- Expect stop at the frequency during seek.");
	
#if defined (CONFIG_HOST_OS_LINUX)	
		etalTestPrint(TR_LEVEL_USER_4, "- Press a key to continue when ready");
	
		fgets(line, sizeof(line), stdin);

		etalTestPrint(TR_LEVEL_USER_4, "key pressed, let's continue");
#else
		etalTestPrint(TR_LEVEL_USER_4, "- RDS Seek will start in %d seconds.", waitTime /1000);
	
		OSAL_s32ThreadWait(waitTime);
#endif
	
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	memset(&rdsSeekStatus, 0, sizeof(EtalSeekStatus));
	rdsSeekOption.tpSeek = FALSE;
	rdsSeekOption.taSeek = TRUE;
	rdsSeekOption.ptySeek = FALSE;
	rdsSeekOption.ptyCode = 0xFF;
	rdsSeekOption.pi = 0;
	rdsSeekOption.piSeek = FALSE;

	ret = etaltml_RDS_seek_start(handlefm, cmdDirectionUp, 50, cmdAudioUnmuted, &rdsSeekOption);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	//Wait for notification
	OSAL_s32SemaphoreWait(etalTestRDSSeekSem, waitTime);
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	if (rdsSeekStatus.m_frequencyFound)
	{
		etalTestPrint(TR_LEVEL_USER_4, "- Test Result : successed, Station found : %d kHz.", rdsSeekStatus.m_frequency);
	}
	else
	{
		if (rdsSeekStatus.m_fullCycleReached)
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed. fullCycleReached.");
		}
		else
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed. Seek not finished.");
		}
		pass4 = FALSE;
	}
	
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	ret = etal_autoseek_stop(handlefm, lastFrequency);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	ret = etaltml_RDS_TA(handlefm, FALSE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(1000);
	/* RDS Seek, RDS station Seek;  (4)  RDS TA seek;  (enable/disable TA, TP)  test end */


	//Tune to frequency	
	ret = etal_tune_receiver(handlefm, freqMin);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	//RDS Seek, RDS station Seek;  (5)  RDS PTY seek;
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "RDS Seek Test(5)  RDS PTY seek (Named PTY RDS station present).");	
	etalTestPrint(TR_LEVEL_USER_4, "- Tuned to frequency :  %d kHz.", freqMin);
	etalTestPrint(TR_LEVEL_USER_4, "- Please set the signal to one frequency with RDS on, Set PTY to 5  (level > 30dBuV).");
	etalTestPrint(TR_LEVEL_USER_4, "- Expect stop at the frequency during seek.");
	
#if defined (CONFIG_HOST_OS_LINUX)	
		etalTestPrint(TR_LEVEL_USER_4, "- Press a key to continue when ready");
	
		fgets(line, sizeof(line), stdin);

		etalTestPrint(TR_LEVEL_USER_4, "key pressed, let's continue");
#else
		etalTestPrint(TR_LEVEL_USER_4, "- RDS Seek will start in %d seconds.", waitTime /1000);
	
		OSAL_s32ThreadWait(waitTime);
#endif
	
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	memset(&rdsSeekStatus, 0, sizeof(EtalSeekStatus));
	rdsSeekOption.tpSeek = FALSE;
	rdsSeekOption.taSeek = FALSE;
	rdsSeekOption.ptySeek = TRUE;
	rdsSeekOption.ptyCode = 0x5;
	rdsSeekOption.pi = 0;
	rdsSeekOption.piSeek = FALSE;

	ret = etaltml_RDS_seek_start(handlefm, cmdDirectionUp, 50, cmdAudioUnmuted, &rdsSeekOption);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	//Wait for notification
	OSAL_s32SemaphoreWait(etalTestRDSSeekSem, waitTime);
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	if (rdsSeekStatus.m_frequencyFound)
	{
		etalTestPrint(TR_LEVEL_USER_4, "- Test Result : successed, Station found : %d kHz.", rdsSeekStatus.m_frequency);	
	}
	else
	{
		if (rdsSeekStatus.m_fullCycleReached)
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed. fullCycleReached.");
		}
		else
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed. Seek not finished.");
		}
		pass5 = FALSE;
	}
	
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	ret = etal_autoseek_stop(handlefm, lastFrequency);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(1000);
	/* RDS Seek, RDS station Seek;   (5)  RDS PTY seek;  test end */



	//Tune to frequency	
	ret = etal_tune_receiver(handlefm, freqMin);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	//RDS Seek, RDS station Seek;  (6)  RDS PTY seek;
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "RDS Seek Test(6)  RDS PTY seek (Named PTY RDS station not present).");
	etalTestPrint(TR_LEVEL_USER_4, "- Tuned to frequency :  %d kHz.", freqMin);
	etalTestPrint(TR_LEVEL_USER_4, "- Please set the signal to one frequency with RDS on, Set PTY to 1- 31 (not 5)  (level > 30dBuV).");
	etalTestPrint(TR_LEVEL_USER_4, "- Expect not stop at the frequency during seek.");
	
#if defined (CONFIG_HOST_OS_LINUX)	
		etalTestPrint(TR_LEVEL_USER_4, "- Press a key to continue when ready");
	
		fgets(line, sizeof(line), stdin);
		
		etalTestPrint(TR_LEVEL_USER_4, "key pressed, let's continue");
#else
		etalTestPrint(TR_LEVEL_USER_4, "- RDS Seek will start in %d seconds.", waitTime /1000);
	
		OSAL_s32ThreadWait(waitTime);
#endif

	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	memset(&rdsSeekStatus, 0, sizeof(EtalSeekStatus));
	rdsSeekOption.tpSeek = FALSE;
	rdsSeekOption.taSeek = FALSE;
	rdsSeekOption.ptySeek = TRUE;
	rdsSeekOption.ptyCode = 0x5;
	rdsSeekOption.pi = 0;
	rdsSeekOption.piSeek = FALSE;

	ret = etaltml_RDS_seek_start(handlefm, cmdDirectionUp, 50, cmdAudioUnmuted, &rdsSeekOption);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	//Wait for notification
	OSAL_s32SemaphoreWait(etalTestRDSSeekSem, waitTime);
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	if (rdsSeekStatus.m_frequencyFound)
	{
		etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed, Station found : %d kHz. Is the PTY code is 5?", rdsSeekStatus.m_frequency);
		pass6 = FALSE;
	}
	else
	{
		if (rdsSeekStatus.m_fullCycleReached)
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : successed. fullCycleReached.");
		}
		else
		{
			etalTestPrint(TR_LEVEL_USER_4, "- Test Result : failed. Seek not finished.");
			pass6 = FALSE;
		}
	}
	
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	
	ret = etal_autoseek_stop(handlefm, lastFrequency);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(1000);
	/* RDS Seek, RDS station Seek;   (6)  RDS PTY seek;  test end */

 	//StopRDS AF strategy
	ret = etaltml_RDS_AF(handlefm, handlefm, FALSE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

 	if ((ret = etal_stop_RDS(handlefm)) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}

	if (!pass1 || !pass2 || !pass3 || !pass4 || !pass5 || !pass6)
	{
		return OSAL_ERROR;
	}

#endif //  CONFIG_APP_TEST_RDS_SEEK && CONFIG_ETALTML_HAVE_RDS_STRATEGY

    if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

	return OSAL_OK;
}


/***************************
 *
 * etalTestRDSAFStrategy
 *
 **************************/
tSInt etalTestRDSAFStrategy(void)
{
#if defined (CONFIG_APP_TEST_RDS_AF_STRATEGY) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)

#ifdef CONFIG_APP_TEST_FM
	EtalRDSData rdsfm;
#endif
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#ifdef  TUNER_DIGITAL_OUT
	EtalAudioInterfTy	vl_etalAudioInterfaceConfig;
#endif	
	EtalProcessingFeatures proc_features;

	tU32 freqMin = 87500;
	tU32 freqMax = 108000;
	tU32 freqAF = 95000;


	etalTestStartup();
	OSAL_pvMemorySet(&rdsfm, 0x00, sizeof(EtalRDSData));

	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "<---RDS AF Strategy Test Start-->");
#if 0

//	Configure audio path if defined digital output
#ifdef  TUNER_DIGITAL_OUT
	memset(&vl_etalAudioInterfaceConfig, 0x00, sizeof(vl_etalAudioInterfaceConfig));
	vl_etalAudioInterfaceConfig.m_dac = false;                                        /* Enable / disable audio DAC */
	vl_etalAudioInterfaceConfig.m_sai_out = true;                                    /* Enable / disable audio SAI output */
	vl_etalAudioInterfaceConfig.m_sai_in = false;                                     /* Enable / disable audio SAI input */
	// now reserved
	//vl_etalAudioInterfaceConfig.m_output_sys_clock_gpio13 = false ;  // Outputs 2.9184MHz System Clock on GPIO13  to STA680 (DCOP) 
	vl_etalAudioInterfaceConfig.m_sai_slave_mode = false;                      /* Enable / disable SAI Slave Mode */

	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	vl_etalAudioInterfaceConfig.m_sai_slave_mode = true;
#endif



	ret = etal_config_audio_path(0, vl_etalAudioInterfaceConfig);
	if (ret != ETAL_RET_SUCCESS) 
	{
		etalTestPrint(TR_LEVEL_USER_4, "Error: etal_config_audio_path err = 0x%x", ret);
	}
#endif
#endif 

	//Configure receiver
	if (etalTestDoRDSConfig(&handlefm, FALSE, ETAL_TEST_MODE_FM, ETAL_INVALID_HANDLE) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}
	
	//Change the receiver band to FM EU
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	ret = etal_change_band_receiver(handlefm, ETAL_BAND_FMEU, freqMin, freqMax, proc_features);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	} 

	//Select audio path
	ret = etal_audio_select(handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	/*
	 * start RDS decoding
	 */
	etalTestPrint(TR_LEVEL_USER_4, "* Start RDS decoding for FM *");
	if ((ret = etal_start_RDS(handlefm, FALSE, 0, ETAL_RDS_MODE, 0)) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}

	//Start RDS AF strategy
	etalTestPrint(TR_LEVEL_USER_4, "* Start RDS AF Strategy for FM *");
	ret = etaltml_RDS_AF(handlefm, handlefm, TRUE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	ret = etaltml_RDS_TA(handlefm, TRUE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	ret = etaltml_RDS_EON(handlefm, TRUE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
#endif


	/* Here we did not turn the RDS REG on */
	/*
	ret = etaltml_RDS_REG(hReceiver_FMAM, TRUE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	*/

	//Tune to frequency	
	ret = etal_tune_receiver(handlefm, freqAF);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "RDS AF Strategy Test");
	etalTestPrint(TR_LEVEL_USER_4, "- Here just lists some simple test cases for RDS AF strategy, which all need subjective checking.");
	etalTestPrint(TR_LEVEL_USER_4, "- Tuned to frequency : %d kHz", freqAF);


	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "RDS AF strategy test case 1 : AF check");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "- Signal generator setting: At least two RDS stations present.");
	etalTestPrint(TR_LEVEL_USER_4, "- Signal 1 :  %d kHz, RDS on, AF list : 105MHz, 98.1MHz, 95MHz etc. TA off, TP off, audio : 1kHz,  Level : 50dBuV.", freqAF);
	etalTestPrint(TR_LEVEL_USER_4, "- Signal 2 : 105000 kHz, RDS on, AF list : 105MHz, 98.1MHz, 95MHz etc. TA off, TP off, audio : 400Hz, Level : 30dBuV.");
	etalTestPrint(TR_LEVEL_USER_4, "                (Here we use 105MHz for test, you can use other frequency.)");
	etalTestPrint(TR_LEVEL_USER_4, "- Signal 1 and signal 2 have the same PI setting.");
	etalTestPrint(TR_LEVEL_USER_4, "- Now tune to signal 1.");
	etalTestPrint(TR_LEVEL_USER_4, "- Wait for 30 seconds for RDS data receiving.");
	etalTestPrint(TR_LEVEL_USER_4, "- Checking audio output with scope.");
	etalTestPrint(TR_LEVEL_USER_4, "- Change signal 1's level to 30dBuV, signal 2' level to 50 dbuV.");
	etalTestPrint(TR_LEVEL_USER_4, "- Waiting, expect AF switch, radio will switch to signal 2.");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");


	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "RDS AF strategy test case 2 : TA switch");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "- One signal needed, frquency : %d kHz, RDS on, TA off,  Level : 40 dBuV.", freqAF );
	etalTestPrint(TR_LEVEL_USER_4, "- Now tune to the signal.");
	etalTestPrint(TR_LEVEL_USER_4, "- Wait for 30 seconds for RDS data receiving.");
	etalTestPrint(TR_LEVEL_USER_4, "- Switch Accordo2 system to other mode, for example CD mode.");
	etalTestPrint(TR_LEVEL_USER_4, "- Turn the signal TA On, TP on.");
	etalTestPrint(TR_LEVEL_USER_4, "- Wait some time, expect Accordo 2 system switch to radio for the traffic listening.");
	etalTestPrint(TR_LEVEL_USER_4, "- Turn the signal TA off, expect Accordo2 system switch back to CD mode etc.");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");


	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "RDS AF strategy test case 3 : EON TA switch");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "- Signal generator setting: At least two RDS stations present.");
	etalTestPrint(TR_LEVEL_USER_4, "- Signal 1:  %dkHz, RDS on, AF list : 105MHz, 98.1MHz, 95MHz etc. audio : 1kHz, Level : 50dBuV.", freqAF);	
	etalTestPrint(TR_LEVEL_USER_4, "                       TA on, TP off.  EON TA off, EON AF: 88MHz, EON AF PI set to signal 2's PI.");
	etalTestPrint(TR_LEVEL_USER_4, "- Signal 2:  88MHz, RDS on, audio : 400Hz, Level : 40dBuV, TA on, TP on", freqAF);
	etalTestPrint(TR_LEVEL_USER_4, "- Now tune to signal 1.");
	etalTestPrint(TR_LEVEL_USER_4, "- Wait for 30 seconds for RDS data receiving.");
	etalTestPrint(TR_LEVEL_USER_4, "- Switch Accord2 system to other mode, like CD mode.");
	etalTestPrint(TR_LEVEL_USER_4, "- Turn singal 1's EON TA on.");
	etalTestPrint(TR_LEVEL_USER_4, "- Wait some time, expect Accordo 2 system switch to radio, frequency switch to signal 2 (88MHz) for traffic listening.");
	etalTestPrint(TR_LEVEL_USER_4, "- Turn signal 2's TA off, expect Accordo2 system switch back to CD mode etc.");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");

	
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "RDS AF strategy test case 4 : PI seek");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "- One signal present : %d kHz, RDS on, TA off,  Level : 35 dBuV.", freqAF);
	etalTestPrint(TR_LEVEL_USER_4, "- Now tune to the signal.");
	etalTestPrint(TR_LEVEL_USER_4, "- Wait for 30 seconds for RDS data receiving.");
	etalTestPrint(TR_LEVEL_USER_4, "- Change signal's frequency to 88MHz for example (not in AF list).");
	etalTestPrint(TR_LEVEL_USER_4, "- Wait about 1 minute.");
	etalTestPrint(TR_LEVEL_USER_4, "- Expect radio start PI seek and stop at 88MHz.");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");


	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "RDS AF strategy test case 5 : AF search.");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");
	etalTestPrint(TR_LEVEL_USER_4, "- Signal generator setting: At least two RDS stations present.");
	etalTestPrint(TR_LEVEL_USER_4, "- Signal 1: %dkHz, RDS on, AF list : 105MHz, 98.1MHz, 95MHz etc. TA off, TP off, audio :  1kHz, Level : 50dBuV.", freqAF);	
	etalTestPrint(TR_LEVEL_USER_4, "- Signal 2:   105MHz, RDS on, AF list : 105MHz, 98.1MHz, 95MHz etc. TA off, TP off, audio : 400Hz, Level : 30dBuV.");
	etalTestPrint(TR_LEVEL_USER_4, "- Signal 1 and signal 2 have the same PI setting.");
	etalTestPrint(TR_LEVEL_USER_4, "- Now tune to signal 1.");
	etalTestPrint(TR_LEVEL_USER_4, "- Wait for 30 seconds for RDS data receiving.");
	etalTestPrint(TR_LEVEL_USER_4, "- Checking audio output with scope.");
	etalTestPrint(TR_LEVEL_USER_4, "- Set signal 1 RDS off. (RF is still on).");
	etalTestPrint(TR_LEVEL_USER_4, "- Wait, expect etal start AF search soon and radio switch to signal 2.");
	etalTestPrint(TR_LEVEL_USER_4, "*********************************************************************");


	etalTestPrint(TR_LEVEL_USER_4, "\r\n");
	etalTestPrint(TR_LEVEL_USER_4, "Good Luck.");
	etalTestPrint(TR_LEVEL_USER_4, "\r\n");

	//Need wait enough time for subjective checking.
	/*Here need wait enough time to finish the test for subjective checking*/
	OSAL_s32ThreadWait(1000);
	

	//When finished, need call the following functions
	/*	
	ret = etaltml_RDS_TA(hReceiver_FMAM, FALSE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
	ret = etaltml_RDS_EON(hReceiver_FMAM, FALSE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}

	//StopRDS AF strategy
	ret = etaltml_RDS_AF(handlefm, handlefm, FALSE);
	if (ret != ETAL_RET_SUCCESS) 
	{
		return OSAL_ERROR;
	}
	
 	if ((ret = etal_stop_RDS(handlefm)) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}
	*/

#endif // CONFIG_APP_TEST_RDS_AF_STRATEGY && CONFIG_ETALTML_HAVE_RDS_STRATEGY

	return OSAL_OK;
}


#endif // CONFIG_APP_ETAL_TEST


