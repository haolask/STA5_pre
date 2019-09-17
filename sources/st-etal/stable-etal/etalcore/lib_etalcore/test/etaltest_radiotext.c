//!
//!  \file 		etaltest_radiotext.c
//!  \brief 	<i><b> ETAL test, radiotext readout </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltml_api.h"

#include "etaltest.h"

#ifdef CONFIG_APP_TEST_GETRADIOTEXT
/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
#define ETAL_TEST_GETRADIOTEXT_DURATION       (6 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_RADIOTEXT_PATH_DURATION_DAB (60 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_RADIOTEXT_PATH_DURATION_FM  (20 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_RADIOTEXT_PATH_DURATION_HD  (2 * 60 * ETAL_TEST_ONE_SECOND)
#define GET_RADIOTEXT_DAB_MIN              1
#define GET_RADIOTEXT_FM_MIN               2
#define GET_RADIOTEXT_HD_MIN               2
#define ETAL_TEST_MAX_HDRADIO_RADIOTEXT    5
#define ETAL_TEST_MAX_DAB_RADIOTEXT        2


// set frequency and station for test
#define MFM	0
#define FUN 0
#define EUROPE_1 1

#if EUROPE_1
#define ETAL_VALID_FM_FREQ_RADIO_TEXT	104700
#endif
#if FUN
#define ETAL_VALID_FM_FREQ_RADIO_TEXT	100300
#endif
#if MFM
#define ETAL_VALID_FM_FREQ_RADIO_TEXT	106900
#endif

/*****************************************************************
| variable defintion (scope: module-local)
|----------------------------------------------------------------*/
tU32 RadiotextCbInvocations;
tBool RadiotextCbComparePass, RadiotextCbFirstCompare;
EtalBcastStandard RadiotextCbBcastStd;
EtalTextInfo radiotext;

/*
 * RDS data captured from the air, used to validate
 * data received during CONFIG_APP_TEST_GETRDS.
 * The first time the test is run in some geographical location
 * the array must be hand-filled.
 */
EtalTextInfo etalTestRadiotextReferenceFM =
{
#ifdef CONFIG_APP_TEST_IN_LE_MANS
#if EUROPE_1
    ETAL_BCAST_STD_FM,
    1,
    "EUROPE 1",
    0,
    1,
    "",
    0
#endif
#if FUN
    ETAL_BCAST_STD_FM,
    1,
    "   FUN  ",
    0,
    1,
    "",
    0
#endif
#if MFM
	ETAL_BCAST_STD_FM,
	1,
	"M RADIO",
	0,
	1,
	"",
	0
#endif 

#else // LMS
	ETAL_BCAST_STD_FM,
	1,
	"RADIO 24",
	0,
	1,
	"",
	0
#endif
};

EtalTextInfo etalTestRadiotextReferenceDAB[ETAL_TEST_MAX_DAB_RADIOTEXT] =
{
	{
		ETAL_BCAST_STD_DAB,
		1,
		ETAL_DAB_ETI_SERV1_LABEL,
		0,
		1,
		"Sie hoeren: Deutschland heute                                   ",
		0
	},
	{
		ETAL_BCAST_STD_DAB,
		1,
		ETAL_DAB_ETI_SERV1_LABEL,
		0,
		1,
		" 300 Jahre Farina in Koeln, Sabine Demmer                       ",
		0
	}
};

EtalTextInfo etalTestRadiotextReferenceHD[ETAL_TEST_MAX_HDRADIO_RADIOTEXT] =
{
	{
		ETAL_BCAST_STD_HD_FM,
		1,
		ETAL_HDRADIO_SERV1_SIS_STATION_NAME,
		0,
		1,
		"Bach's Lunch / Pilhofer Jazz Quartet",
		0
	},
	{
		ETAL_BCAST_STD_HD_FM,
		1,
		ETAL_HDRADIO_SERV1_SIS_STATION_NAME,
		0,
		1,
		"Subway / Jim Nuzzo",
		0
	},
	{
		ETAL_BCAST_STD_HD_FM,
		1,
		ETAL_HDRADIO_SERV1_SIS_STATION_NAME,
		0,
		1,
		"Without a Song / Bob Mintzer",
		0
	},
	{
		ETAL_BCAST_STD_HD_FM,
		1,
		ETAL_HDRADIO_SERV1_SIS_STATION_NAME,
		0,
		1,
		"Hungarian Rhapsody No. 2 / Cincinnati Pops",
		0
	},
	{
		ETAL_BCAST_STD_HD_FM,
		1,
		ETAL_HDRADIO_SERV1_SIS_STATION_NAME,
		0,
		1,
		"Hungarian Rhapsody No. 2 / Erich Kunzel / Cincinnati Pops",
		0
	}
};

#if defined(CONFIG_APP_TEST_HDRADIO_FM) || defined(CONFIG_APP_TEST_DAB)
/***************************
 *
 * etalTestGetRadiotextCompareHD
 *
 **************************/
static tBool etalTestGetRadiotextCompareAll(EtalTextInfo *rt1, EtalTextInfo *rt2, tU32 rt2_size)
{
	tU8 i;
	tBool ret = FALSE;

	for (i = 0; i < rt2_size; i++)
	{
		if (rt1->m_broadcastStandard == rt2[i].m_broadcastStandard)
		{
			if (rt1->m_serviceNameIsNew == TRUE)
			{
				if (!OSAL_s32StringCompare(rt1->m_serviceName, rt2[i].m_serviceName))
				{
					ret = TRUE;
				}
			}

			if (rt1->m_currentInfoIsNew == TRUE)
			{
				if (!OSAL_s32StringCompare(rt1->m_currentInfo, rt2[i].m_currentInfo))
				{
					ret = TRUE;
				}
			}
		}
	}
	return ret;
}
#endif

#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB)
/***************************
 *
 * etalTestGetRadiotextCompare
 *
 **************************/
static tSInt etalTestGetRadiotextCompare(EtalTextInfo *rt1, EtalTextInfo *rt2)
{
	tSInt vl_res = OSAL_ERROR;

	if (rt1->m_broadcastStandard == rt2->m_broadcastStandard)
	{
		// in FM for now this is live test : 
		// check only that radio text received is there, without comparing
		//
	
		if (ETAL_BCAST_STD_FM == rt1->m_broadcastStandard)
		{
			if ((OSAL_s32StringCompare(rt1->m_serviceName, rt2->m_serviceName) == 0)
				&& (OSAL_u32StringLength(rt1->m_serviceName) > 0))
			{
				vl_res = OSAL_OK;
			}
			
		}
		else
		{
			if ((OSAL_s32StringCompare(rt1->m_serviceName, rt2->m_serviceName) == 0) &&
				(OSAL_s32StringCompare(rt1->m_currentInfo, rt2->m_currentInfo) == 0))
			{
				vl_res = OSAL_OK;
			}	
		}
	}
	
	return vl_res;
}
#endif

#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB) || defined (CONFIG_APP_TEST_HDRADIO_FM)
/***************************
 *
 * etalTestRadiotextCallback
 *
 **************************/
static void etalTestRadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	EtalTextInfo *pRadiotext;

	pRadiotext = (EtalTextInfo *)pBuffer;
	etalTestPrintRadioText(pRadiotext);
	/* Compare Radiotext */
	if (RadiotextCbBcastStd == ETAL_BCAST_STD_FM) 
	{
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB)
		if (etalTestGetRadiotextCompare(pRadiotext, &etalTestRadiotextReferenceFM) == OSAL_OK)
		{
			RadiotextCbComparePass = TRUE;
		}
#endif // #if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB)
	}
	else if (RadiotextCbBcastStd == ETAL_BCAST_STD_DAB)
	{
		/* do not compare first received radiotext if currentInfo is not available */
		if (((RadiotextCbFirstCompare == TRUE) && (pRadiotext->m_currentInfoIsNew == TRUE)) || 
			(RadiotextCbFirstCompare == FALSE))
		{
#if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_DAB)
			if (!etalTestGetRadiotextCompareAll(pRadiotext, etalTestRadiotextReferenceDAB, ETAL_TEST_MAX_DAB_RADIOTEXT))
			{
				RadiotextCbComparePass = FALSE;
				etalTestPrintNormal("pass2cb FAILED");
			}
#endif // #if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_DAB)
		}
		RadiotextCbFirstCompare = FALSE;
	}
	else if (RadiotextCbBcastStd == ETAL_BCAST_STD_HD_FM)
	{
		/* do not compare first received radiotext if currentInfo is not available */
		if (((RadiotextCbFirstCompare == TRUE) && (pRadiotext->m_currentInfoIsNew == TRUE)) || 
			(RadiotextCbFirstCompare == FALSE))
		{
#ifdef CONFIG_APP_TEST_HDRADIO_FM
			if (!etalTestGetRadiotextCompareAll(pRadiotext, etalTestRadiotextReferenceHD, ETAL_TEST_MAX_HDRADIO_RADIOTEXT))
			{
				RadiotextCbComparePass = FALSE;
				etalTestPrintNormal("pass2cb FAILED");
			}
#endif // #ifdef CONFIG_APP_TEST_HDRADIO_FM
		}
		RadiotextCbFirstCompare = FALSE;
	}

	RadiotextCbInvocations++;
}

/***************************
 *
 * etalTestRadiotextDatapath
 *
 **************************/
static tSInt etalTestRadiotextDatapath(ETAL_HANDLE handle, EtalBcastStandard std, tBool *pass, tU8 count, tS32 duration)
{
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
	EtalDataPathAttr dataPathAttr;
	ETAL_STATUS ret;
	tBool pass1 = TRUE;
	tBool pass2 = TRUE;

	OSAL_pvMemorySet(&radiotext, 0x00, sizeof(EtalTextInfo));
	RadiotextCbFirstCompare = TRUE;
	if (std == ETAL_BCAST_STD_FM)
	{
		RadiotextCbComparePass = FALSE;
	}
	else
	{
		RadiotextCbComparePass = TRUE;
	}
	RadiotextCbBcastStd = std;
	etalTestPrintNormal("");
	etalTestPrintNormal("GetRadiotext from datapath for %s", etalTestStandard2Ascii(std));

	etalTestPrintNormal("* Config radiotext datapath for %s", etalTestStandard2Ascii(std));
	dataPathAttr.m_receiverHandle = handle;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_TEXTINFO;
	dataPathAttr.m_sink.m_context = NULL;
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalTextInfo);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestRadiotextCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for %s (%s, %d)", etalTestStandard2Ascii(std), ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * reset callback counter
	 */
	RadiotextCbInvocations = 0;

	/*
	 * start datapath
	 */
	etalTestPrintNormal("* Start radiotext datapath for %s", etalTestStandard2Ascii(std));
	etalTestPrintNormal("* Wait %ds to allow radiotext delivery", duration / ETAL_TEST_ONE_SECOND);
	if ((ret = etaltml_start_textinfo(handle)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_start_textinfo for %s (%s, %d)", etalTestStandard2Ascii(std), ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	OSAL_s32ThreadWait(duration);
	etalTestPrintNormal("= complete, %d radiotext callback invocations", RadiotextCbInvocations);
	etalTestPrintNormal("= (expected more than %d)", count);
	/* check number of callback invocations */
	if (RadiotextCbInvocations < count)
	{
		etalTestPrintNormal("pass FAILED"); 
		pass1 = FALSE;
	}
	/* check if radiotext comparition is fail */
	if (RadiotextCbComparePass == FALSE)
	{
		pass1 = FALSE;
	}

	/*
	 * stop datapath, ensure no more messages are delivered
	 */
	etalTestPrintNormal("* Stop radiotext datapath for %s", etalTestStandard2Ascii(std));
	etalTestPrintNormal("* Wait %ds to ensure no more radiotext is delivered", duration / 2000);
	if ((ret = etaltml_stop_textinfo(handle)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_stop_textinfo for %s (%s, %d)", etalTestStandard2Ascii(std), ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	RadiotextCbInvocations = 0;
	OSAL_s32ThreadWait(duration / 2);
	if (RadiotextCbInvocations > 1)
	{
		etalTestPrintNormal("RadiotextCbInvocations larger than one FAILED");
		pass2 = FALSE;
	}
	etalTestPrintNormal("* Destroy datapath for %s", etalTestStandard2Ascii(std));
	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_datapath for %s (%s, %d)", etalTestStandard2Ascii(std), ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if (!pass1 || !pass2)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif //defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB) || defined (CONFIG_APP_TEST_HDRADIO_FM)
#endif // CONFIG_APP_TEST_GETRADIOTEXT

/***************************
 *
 * etalTestGetRadiotext
 *
 **************************/
tSInt etalTestGetRadiotext(void)
{
#ifdef CONFIG_APP_TEST_GETRADIOTEXT
#if defined (CONFIG_APP_TEST_FM)
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
	EtalTextInfo radiotextfm;
    EtalDataPathAttr dataPathAttr;
#endif
#ifdef CONFIG_APP_TEST_DAB
	EtalTextInfo radiotextdab;
	tBool radiotextDabIsFirstNewReceived;
#endif
#ifdef CONFIG_APP_TEST_HDRADIO_FM
	EtalTextInfo radiotexthd;
	tBool radiotextHdIsFirstNewReceived;
#endif

#if	defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB) || defined (CONFIG_APP_TEST_HDRADIO_FM)
	OSAL_tMSecond curtime, endtime;
	tBool pass2tmp = TRUE;
	ETAL_STATUS ret;
#endif //defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB) || defined (CONFIG_APP_TEST_HDRADIO_FM)

	tBool pass2 = TRUE;
	tBool pass1 = TRUE;

	etalTestStartup();

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME); // allow some time for FIC decoding
	OSAL_pvMemorySet(&radiotextdab, 0x00, sizeof(EtalTextInfo));
	OSAL_pvMemorySet(&radiotext, 0x00, sizeof(EtalTextInfo));
	radiotextDabIsFirstNewReceived = FALSE;
	etalTestPrintNormal("");
	if (etalTestDoServiceSelectAudio(handledab, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV1_SID))
	{
		return OSAL_ERROR;
	}
	etalTestPrintNormal("GetRadiotext for DAB");
	etalTestPrintNormal("Testing for %ds", ETAL_TEST_RADIOTEXT_PATH_DURATION_DAB / ETAL_TEST_ONE_SECOND);
	endtime = OSAL_ClockGetElapsedTime() + ETAL_TEST_RADIOTEXT_PATH_DURATION_DAB;
	curtime = 0;

	while (curtime < endtime)
	{
		if ((ret = etaltml_get_textinfo(handledab, &radiotextdab)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etaltml_get_textinfo for DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		/* print only if different */
		if (memcmp(&radiotextdab, &radiotext, sizeof(EtalTextInfo)) != 0)
		{
			etalTestPrintRadioText(&radiotextdab);
		}
		memcpy(&radiotext, &radiotextdab, sizeof(EtalTextInfo));

		if (radiotextdab.m_currentInfoIsNew == TRUE)
		{
			radiotextDabIsFirstNewReceived = TRUE;
		}
		if ((radiotextDabIsFirstNewReceived == TRUE) && 
			(!etalTestGetRadiotextCompareAll(&radiotextdab, etalTestRadiotextReferenceDAB, ETAL_TEST_MAX_DAB_RADIOTEXT)))
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass1a FAILED");
		}

		OSAL_s32ThreadWait(300);
		curtime = OSAL_ClockGetElapsedTime();
	}
	if ((radiotextDabIsFirstNewReceived == FALSE) || 
		(!etalTestGetRadiotextCompareAll(&radiotextdab, etalTestRadiotextReferenceDAB, ETAL_TEST_MAX_DAB_RADIOTEXT)))
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1a FAILED");
	}
#endif // CONFIG_APP_TEST_DAB

#ifdef CONFIG_APP_TEST_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	// for Radio Text, let's try a frequency specific to radio text
	etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ_RADIO_TEXT);
	if ((ret = etal_tune_receiver(handlefm, ETAL_VALID_FM_FREQ_RADIO_TEXT)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	(tVoid) curtime;
	(tVoid) endtime;
	OSAL_pvMemorySet(&radiotextfm, 0x00, sizeof(EtalTextInfo));

	/*
	 * start RDS
	 */
	etalTestPrintNormal("* Start RDS for FM");
	if ((ret = etal_start_RDS(handlefm, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_start_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/*
	 * configure RadioText datapath
	 */
	etalTestPrintNormal("* Config RadioText datapath for FM");
	dataPathAttr.m_receiverHandle = handlefm;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_TEXTINFO;
	dataPathAttr.m_sink.m_context = NULL;
	dataPathAttr.m_sink.m_BufferSize = 0;
	dataPathAttr.m_sink.m_CbProcessBlock = NULL;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * start RadioText
	 */
	etalTestPrintNormal("* Start RadioText for FM");
	if ((ret = etaltml_start_textinfo(handlefm)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_start_textinfo for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * wait for RadioText reception
	 */
	etalTestPrintNormal("GetRadioText for FM");
	etalTestPrintNormal("Testing for %ds", ETAL_TEST_GETRADIOTEXT_DURATION /ETAL_TEST_ONE_SECOND);
    OSAL_s32ThreadWait(ETAL_TEST_GETRADIOTEXT_DURATION);
	/*
	 * get decoded RadioText
	 */
	if ((ret = etaltml_get_textinfo(handlefm, &radiotextfm)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_get_textinfo for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintRadioText(&radiotextfm);
	if (etalTestGetRadiotextCompare(&radiotextfm, &etalTestRadiotextReferenceFM) == OSAL_OK)
	{
		pass1 = TRUE;
	}
	if (!pass1)
	{
		etalTestPrintNormal("pass1b FAILED");
	}

	/*
	 * stop Radiotext
	 */
    etalTestPrintNormal("* Stop RadioText for FM");
    if ((ret = etaltml_stop_textinfo(handlefm)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_stop_textinfo for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
	/*
	 * stop RDS
	 */
	etalTestPrintNormal("* Stop RDS for FM");
	if ((ret = etal_stop_RDS(handlefm)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	OSAL_pvMemorySet(&radiotexthd, 0x00, sizeof(EtalTextInfo));
	OSAL_pvMemorySet(&radiotext, 0x00, sizeof(EtalTextInfo));
	radiotextHdIsFirstNewReceived = FALSE;
	etalTestPrintNormal("");
	etalTestPrintNormal("GetRadiotext for HDRADIO");
	etalTestPrintNormal("Testing for %ds", ETAL_TEST_RADIOTEXT_PATH_DURATION_HD / ETAL_TEST_ONE_SECOND);
	endtime = OSAL_ClockGetElapsedTime() + ETAL_TEST_RADIOTEXT_PATH_DURATION_HD;
	curtime = 0;
	while (curtime < endtime)
	{
		if ((ret = etaltml_get_textinfo(handlehd, &radiotexthd)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etaltml_get_textinfo for HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		/* print only if different */
		if (memcmp(&radiotexthd, &radiotext, sizeof(EtalTextInfo)) != 0)
		{
			etalTestPrintRadioText(&radiotexthd);
		}
		memcpy(&radiotext, &radiotexthd, sizeof(EtalTextInfo));

		if (radiotexthd.m_currentInfoIsNew == TRUE)
		{
			radiotextHdIsFirstNewReceived = TRUE;
		}
		if ((radiotextHdIsFirstNewReceived == TRUE) && 
			(!etalTestGetRadiotextCompareAll(&radiotexthd, etalTestRadiotextReferenceHD, ETAL_TEST_MAX_HDRADIO_RADIOTEXT)))
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass1c FAILED");
		}

		OSAL_s32ThreadWait(300);
		curtime = OSAL_ClockGetElapsedTime();
	}
	if ((radiotextHdIsFirstNewReceived == FALSE) || 
		(!etalTestGetRadiotextCompareAll(&radiotexthd, etalTestRadiotextReferenceHD, ETAL_TEST_MAX_HDRADIO_RADIOTEXT)))
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1c FAILED");
	}
#endif

	/*
	 * pass2, get radiotext from datapath
	 *
	 */
#ifdef CONFIG_APP_TEST_DAB
	pass2tmp = TRUE;
	if (etalTestRadiotextDatapath(handledab, ETAL_BCAST_STD_DAB, &pass2tmp, GET_RADIOTEXT_DAB_MIN, ETAL_TEST_RADIOTEXT_PATH_DURATION_DAB) == OSAL_ERROR)
	{
		etalTestPrintNormal("etalTestRadiotextDatapath(ETAL_BCAST_STD_DAB):1 FAILED");
		pass2 = FALSE;
	}
	if (!pass2tmp)
	{
		etalTestPrintNormal("pass2a FAILED");
	}
	/*
	 * repeat the test but destroy the receiver first and ensure that the
	 * service name is delivered with the PAD (destroying the receiver resets the 'already sent' state)
	 */
	etalTestPrintNormal("");
	etalTestPrintNormal("Repeat the test to ensure that by destroying the receiver the radiotext is re-sent");
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	// Wait some time to receive all data before selecting audio

	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);
	
	if (etalTestDoServiceSelectAudio(handledab, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV1_SID))
	{
		return OSAL_ERROR;
	}
	pass2tmp = TRUE;
	if (etalTestRadiotextDatapath(handledab, ETAL_BCAST_STD_DAB, &pass2tmp, GET_RADIOTEXT_DAB_MIN, ETAL_TEST_RADIOTEXT_PATH_DURATION_DAB) == OSAL_ERROR)
	{
		etalTestPrintNormal("etalTestRadiotextDatapath(ETAL_BCAST_STD_DAB):2 FAILED");
		pass2 = FALSE;
	}
	if (!pass2tmp)
	{
		etalTestPrintNormal("pass2b FAILED");
	}

	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_DAB

#ifdef CONFIG_APP_TEST_FM
	/*
	 * Reconfigure the receiver so to force ETAL to reset RDS
	 * data, otherwise after the pass1 all data is considered
	 * already sent by the RDS decoder and not sent again
	 */
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	 * After a reconfiguration the tuned frequency is reset to 0
	 * and this causes problems with the RDS landscape, so a new
	 * tune is required
	 */
	if (etalTestDoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	
	// for Radio Text, let's try a frequency specific to radio text
	etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ_RADIO_TEXT);
	if ((ret = etal_tune_receiver(handlefm, ETAL_VALID_FM_FREQ_RADIO_TEXT)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/*
	 * start RDS
	 */
	etalTestPrintNormal("* Start RDS for FM");
	if ((ret = etal_start_RDS(handlefm, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_start_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	pass2tmp = TRUE;
	if (etalTestRadiotextDatapath(handlefm, ETAL_BCAST_STD_FM, &pass2tmp, GET_RADIOTEXT_FM_MIN, ETAL_TEST_RADIOTEXT_PATH_DURATION_FM) == OSAL_ERROR)
	{
		etalTestPrintNormal("etalTestRadiotextDatapath(ETAL_BCAST_STD_FM) FAILED");
		pass2 = FALSE;
	}

	/*
	 * stop RDS
	 */
	etalTestPrintNormal("* Stop RDS for FM");
	if ((ret = etal_stop_RDS(handlefm)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if (!pass2tmp)
	{
		etalTestPrintNormal("pass2c FAILED");
	}

	if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	/*
	 * Reconfigure the receiver so to force ETAL to reset RDS
	 * data, otherwise after the pass1 all data is considered
	 * already sent by the RDS decoder and not sent again
	 */
	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	 * After a reconfiguration the tuned frequency is reset to 0
	 * and this causes problems with the RDS landscape, so a new
	 * tune is required
	 */
	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	pass2tmp = TRUE;
	if (etalTestRadiotextDatapath(handlehd, ETAL_BCAST_STD_HD_FM, &pass2tmp, GET_RADIOTEXT_HD_MIN, ETAL_TEST_RADIOTEXT_PATH_DURATION_HD) == OSAL_ERROR)
	{
		etalTestPrintNormal("etalTestRadiotextDatapath(ETAL_BCAST_STD_HD_FM) FAILED");
		pass2 = FALSE;
	}
	if (!pass2tmp)
	{
		etalTestPrintNormal("pass2d FAILED");
	}

	if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_FM

	etalTestPrintNormal("");

	if (!pass1 || !pass2)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_GETRADIOTEXT
	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

