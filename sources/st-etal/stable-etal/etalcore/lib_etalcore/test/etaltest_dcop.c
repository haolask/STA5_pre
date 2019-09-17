//!
//!  \file 		etaltest_dcop.c
//!  \brief 	<i><b> ETAL test, DCOP functions </b></i>
//!  \details   Tests some DCOP functionalities
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"
#include "dabmw_crc.h"
#include "etalinternal.h" // for ETAL_sendCommandTo_MDR, ETAL_sendCommunicationErrorEvent...

/***************************
 * Local Macros
 **************************/
#define ETAL_TEST_SERVSEL_PASS1_RAWAUDIO_COUNT_MIN   20
#define ETAL_TEST_SERVSEL_PASS1_RAWAUDIO_COUNT_MAX    1
#define ETAL_TEST_SERVSEL_PASS1_RAWDATA_COUNT_MIN     4
#define ETAL_TEST_SERVSEL_PASS1_RAWDATA_COUNT_MAX     1
#define ETAL_TEST_SERVSEL_PASS2_RAWDATA_COUNT_MIN   130
#define ETAL_TEST_SERVSEL_PASS3_RAWAUDIO_COUNT_MIN  250

#define ETAL_TEST_SERVSEL_MAX_ADDRESS                20

#define DABMW_NETWORK_PACKET_PACKET_LENGTH(_buf_)              (((_buf_)[0] >> 6) & 0x03)
#define DABMW_NETWORK_PACKET_CONTINUITY(_buf_)                 (((_buf_)[0] >> 4) & 0x03)
#define DABMW_NETWORK_PACKET_FIRSTLAST_FLAG(_buf_)             (((_buf_)[0] >> 2) & 0x03)
#define DABMW_NETWORK_PACKET_ADDRESS(_buf_)                    ((((_buf_)[0] << 8) & 0x0300) | ((_buf_)[1] & 0x00FF))
#define DABMW_NETWORK_PACKET_DATACOMMAND_FLAG(_buf_)           (((_buf_)[2] >> 7) & 0x01)
#define DABMW_NETWORK_PACKET_DATALENGTH(_buf_)                 (((_buf_)[2] >> 0) & 0x7F)
#define DABMW_NETWORK_PACKET_PACKET_CRC(_buf_, _len_)          ((((_buf_)[(_len_)-2] << 8) & 0xFF00) | ((_buf_)[(_len_)-1] & 0x00FF))

#define ETAL_TEST_INVALID_ADDRESS                 -1

#define ETAL_TEST_SERVSEL_WAIT                    2128   // use a number easilily recognizable in the signal traces

/***************************
 * Local types
 **************************/
typedef struct {
	tS16 address;
	tU32 count;
	tU8  prevContinuity;
} etalTestPacketAddressTy;

typedef struct
{
	tU32 rawAudioCounter;
	tU32 rawDataCounter;
	tU32 rawDataContinuityErrorCount;
	tU32 rawDataCRCErrorCount;
	etalTestPacketAddressTy addressCache[ETAL_TEST_SERVSEL_MAX_ADDRESS];
} etalTestCountersTy;

/***************************
 * Local variables
 **************************/
etalTestCountersTy counters;
tBool stopTest;
//static tBool etalTestKeepGoing;

/***************************
 * function prototypes
 **************************/
tSInt etalTestServiceSelectWaitAndCheck(tU32 delay);

#ifdef CONFIG_APP_TEST_TUNE_DCOP
/*
 * This should go to the etalcmd_mdr.c file if needed for some functionality other than the APP_TEST_TUNE_DCOP
 */
static tSInt ETAL_cmdClearDb_MDR(ETAL_HANDLE hReceiver)
{
	tU8 DABMWcmd_ClearDb[]  = {0x90, 0x84, 0x26};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	retval = ETAL_sendCommandTo_MDR(DABMWcmd_ClearDb, sizeof(DABMWcmd_ClearDb), &cstatus, TRUE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		/*
		 * The correct way would be:
		 * ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		 * but ETAL_commandStatusToEventErrStatus_MDR is not visible from here so since this is 
		 * just an ugly test anyway, we use a generic error. If moved to etalcmd_mdr.c use the proper implementation
		 */
		ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_GenericError, cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if ((resplen > 1) && (resp[1] != 0))
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_TUNE_DCOP

#ifdef CONFIG_APP_TEST_SERVICE_SELECT_DCOP
#if defined (CONFIG_ETAL_HAVE_ADVTUNE) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * EtalBcastDataType2Ascii
 *
 **************************/
static tChar *EtalBcastDataType2Ascii(EtalBcastDataType type)
{
	switch (type)
	{
		case ETAL_DATA_TYPE_AUDIO:
			return "AUDIO";
		case ETAL_DATA_TYPE_DATA_SERVICE:
			return "DATA";
		case ETAL_DATA_TYPE_DAB_DATA_RAW:
			return "DATA RAW";
		case ETAL_DATA_TYPE_DAB_AUDIO_RAW:
			return "DAB AUDIO RAW";
		case ETAL_DATA_TYPE_DAB_FIC:
			return "FIC";
		default:
			return "undefined";
	}
}

/***************************
 *
 * etalTestInitAddressCache
 *
 **************************/
static tVoid etalTestInitAddressCache(tVoid)
{
	tU32 i;

	for (i = 0; i < ETAL_TEST_SERVSEL_MAX_ADDRESS; i++)
	{
		counters.addressCache[i].address = -1;
	}
}

/***************************
 *
 * etalTestResetCounters
 *
 **************************/
static tVoid etalTestResetCounters(tVoid)
{
	OSAL_pvMemorySet((tVoid *)&counters, 0x00, sizeof(etalTestCountersTy));
	etalTestInitAddressCache();
	stopTest = FALSE;
}

/***************************
 *
 * etalTestSetupDatapath
 *
 **************************/
static tSInt etalTestSetupDatapath(ETAL_HANDLE hReceiver, ETAL_HANDLE *hDatapath, EtalBcastDataType type, EtalCbProcessBlock sink)
{
	EtalDataPathAttr dataPathAttr;
	ETAL_STATUS ret;

	etalTestPrintNormal("* Config datapath, type %s", EtalBcastDataType2Ascii(type));
	*hDatapath = ETAL_INVALID_HANDLE;
	dataPathAttr.m_receiverHandle = hReceiver;
	dataPathAttr.m_dataType = type;
	OSAL_pvMemorySet(&dataPathAttr.m_sink, 0x00, sizeof(EtalSink));
	dataPathAttr.m_sink.m_CbProcessBlock = sink;
	if ((ret = etal_config_datapath(hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestDestroyDatapath
 *
 **************************/
static tSInt etalTestDestroyDatapath(ETAL_HANDLE *hDatapath, EtalBcastDataType type)
{
	ETAL_STATUS ret;

	etalTestPrintNormal("* Destroy datapath, type %s", EtalBcastDataType2Ascii(type));
	if ((ret = etal_destroy_datapath(hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_datapath (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestSearchAddress
 *
 **************************/
static tS32 etalTestSearchAddress(tU16 address)
{
	tU32 i;

	for (i = 0; i < ETAL_TEST_SERVSEL_MAX_ADDRESS; i++)
	{
		if (counters.addressCache[i].address == address)
		{
			return i;
		}
	}
	return -1;
}

/***************************
 *
 * etalTestAddAddress
 *
 **************************/
static tVoid etalTestAddAddress(tU16 address, tU8 continuity)
{
	tU32 i;

	for (i = 0; i < ETAL_TEST_SERVSEL_MAX_ADDRESS; i++)
	{
		if (counters.addressCache[i].address == ETAL_TEST_INVALID_ADDRESS)
		{
			counters.addressCache[i].address = address;
			counters.addressCache[i].count = 1;
			counters.addressCache[i].prevContinuity = continuity;
			return;
		}
	}
	etalTestPrintError("Overflow in etalTestAddAddress");
}

/***************************
 *
 * etalTestPrintAddressCache
 *
 **************************/
static tVoid etalTestPrintAddressCache(tVoid)
{
	tU32 i;

	for (i = 0; i < ETAL_TEST_SERVSEL_MAX_ADDRESS; i++)
	{
		if (counters.addressCache[i].address == ETAL_TEST_INVALID_ADDRESS)
		{
			continue;
		}
		etalTestPrintNormal("Address 0x%.4x, count %4d", counters.addressCache[i].address, counters.addressCache[i].count);
	}
}

/***************************
 *
 * etalTestCheckCRC
 *
 **************************/
static tSInt etalTestCheckCRC(tU8 *dataPtr, tU16 packetLength, tU16 packetCrc)
{
	tU16 crc = (tU16)0xFFFF;
	tS32 crcPacketLength = packetLength - 2; // CRC bytes not part of CRC calculation
	tPU8 crcDataPtr = dataPtr;

	while (crcPacketLength > 0)
	{
		crc = DABMW_CRCccitt (crc, *crcDataPtr++);
		crcPacketLength--;
	}

	if ((tU16)0xFFFF != (crc ^ packetCrc))
	{
		etalTestPrintNormal("CRC ERROR detected, packet length %d", packetLength);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestSinkRawAudio
 *
 **************************/
static tVoid etalTestSinkRawAudio(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	//etalTestPrintNormal("raw audio frame, size %d", dwActualBufferSize);
	counters.rawAudioCounter++;
}

/***************************
 *
 * etalTestSinkRawData
 *
 **************************/
static tVoid etalTestSinkRawData(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	tS32 index;
	tU16 packetCRC;
	tU32 packetLength;

	//etalTestPrintNormal("raw data frame, size %d", dwActualBufferSize);
	counters.rawDataCounter++;

	if (status)
	{
		if (status->m_isValid)
		{
			if (status->m_continuityError)
			{
#ifdef INCLUDE_CONTINUITY_COUNTER_DEBUG
				/*
				 * tU8 m_expectedContinuity and tU8 m_receivedContinuity are available only for
				 * internal debug, when needed they need to be defined in etal_types.h
				 * Since this is an external API file their definition is not
				 * committed to the repository.
				 */
				etalTestPrintNormal("Continuity ERROR (exp %d, recv %d) packets from start of pass: %d", status->m_expectedContinuity, status->m_receivedContinuity, counters.rawAudioCounter + counters.rawDataCounter);
#else
				etalTestPrintNormal("Continuity ERROR, packets from start of pass: %d", counters.rawAudioCounter + counters.rawDataCounter);
#endif
				goto stopTest;
			}
		}
	}

	/* Network Packet continuity counter check (disabled) */
	index = etalTestSearchAddress(DABMW_NETWORK_PACKET_ADDRESS(pBuffer));
	if (index == ETAL_TEST_INVALID_ADDRESS)
	{
		etalTestAddAddress(DABMW_NETWORK_PACKET_ADDRESS(pBuffer), DABMW_NETWORK_PACKET_CONTINUITY(pBuffer));
	}
	else
	{
		counters.addressCache[index].count++;
		/*
		 * Continuity based on network packet counter is bound to fail when
		 * the signal generator loops around so it is not very significant
		 * Use only the data channel based continuity check
		 */
#if 0
		if (DABMW_NETWORK_PACKET_CONTINUITY(pBuffer) != (counters.addressCache[index].prevContinuity + 1) % 4)
		{
			counters.rawDataContinuityErrorCount++;
		}
		counters.addressCache[index].prevContinuity = DABMW_NETWORK_PACKET_CONTINUITY(pBuffer);
#endif
	}

	/* Network Packet CRC check */
	packetLength = (DABMW_NETWORK_PACKET_PACKET_LENGTH(pBuffer) + 1) * 24;
	packetCRC = DABMW_NETWORK_PACKET_PACKET_CRC(pBuffer, packetLength);
	if (etalTestCheckCRC(pBuffer, dwActualBufferSize, packetCRC) != OSAL_OK)
	{
		counters.rawDataCRCErrorCount++;
	}
	return;

stopTest:
	stopTest = TRUE;
	return;
}


/***************************
 *
 * etalTestServiceSelectPass1
 *
 **************************/
tSInt etalTestServiceSelectWaitAndCheck(tU32 delay)
{
	OSAL_tMSecond now, end;

	now = OSAL_ClockGetElapsedTime();
	end = now + delay;

	if (etalTestOption.oStopOnErrors)
	{
		while (now < end)
		{
			if (stopTest)
			{
				etalTestPrintNormal("Forcing test stop");
				return OSAL_ERROR;
			}
			now = OSAL_ClockGetElapsedTime();
			OSAL_s32ThreadWait(5);
		}
	}
	else
	{
		OSAL_s32ThreadWait(delay);
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestServiceSelectPass1
 *
 **************************/
/*
 * pass1
 *
 *   APPEND AUDIO1 (RAW)
 *   APPEND DATA1  (RAW)
 *   check number of received packets
 *   Network Packet continuity and CRC
 *   REMOVE AUDIO1 (RAW) and DATA1 (RAW)
 */
static tSInt etalTestServiceSelectPass1(tBool *pass)
{
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hDatapathAudio, hDatapathData;

	etalTestPrintNormal("* pass1, Select SID audio1+data1, remove audio1+data1");

	/*
	 * create datapaths
	 */
	if ((etalTestSetupDatapath(handledab, &hDatapathAudio, ETAL_DATA_TYPE_DAB_AUDIO_RAW, etalTestSinkRawAudio) != OSAL_OK) ||
		(etalTestSetupDatapath(handledab, &hDatapathData, ETAL_DATA_TYPE_DAB_DATA_RAW, etalTestSinkRawData) != OSAL_OK))
	{
		return OSAL_ERROR;
	}

	etalTestResetCounters();

	etalTestPrintNormal("* Select raw audio SID");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Select data SID");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD2_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if (etalTestServiceSelectWaitAndCheck(ETAL_TEST_SERVSEL_WAIT) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	
	etalTestPrintNormal("= %d raw audio frames, %d raw data frames received", counters.rawAudioCounter, counters.rawDataCounter);
	etalTestPrintNormal("= (expected more than %d and more than %d)", ETAL_TEST_SERVSEL_PASS1_RAWAUDIO_COUNT_MIN, ETAL_TEST_SERVSEL_PASS1_RAWDATA_COUNT_MIN);
	if ((counters.rawAudioCounter < ETAL_TEST_SERVSEL_PASS1_RAWAUDIO_COUNT_MIN) ||
		(counters.rawDataCounter < ETAL_TEST_SERVSEL_PASS1_RAWDATA_COUNT_MIN))
	{
		*pass = FALSE;
		etalTestPrintNormal("pass1a FAILED");
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}
	// this code is disabled (see etalTestSinkRawData)
	if (counters.rawDataContinuityErrorCount) 
	{
		*pass = FALSE;
		etalTestPrintNormal("pass1b FAILED (%d continuity errors)", counters.rawDataContinuityErrorCount);
	}
	if (counters.rawDataCRCErrorCount)
	{
		*pass = FALSE;
		etalTestPrintNormal("pass1c FAILED (%d CRC errors)", counters.rawDataCRCErrorCount);
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintNormal("* Remove audio SID");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Remove data SID");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD2_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* allow some time for the last packets to leave the FIFOs */
	OSAL_s32ThreadWait(10);
	etalTestResetCounters();
	if (etalTestServiceSelectWaitAndCheck(ETAL_TEST_SERVSEL_WAIT) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("= %d raw audio frames, %d raw data frames received", counters.rawAudioCounter, counters.rawDataCounter);
	etalTestPrintNormal("= (expected less than %d and less than %d)", ETAL_TEST_SERVSEL_PASS1_RAWAUDIO_COUNT_MAX, ETAL_TEST_SERVSEL_PASS1_RAWDATA_COUNT_MAX);

	if ((counters.rawAudioCounter > ETAL_TEST_SERVSEL_PASS1_RAWAUDIO_COUNT_MAX) ||
		(counters.rawDataCounter > ETAL_TEST_SERVSEL_PASS1_RAWDATA_COUNT_MAX))
	{
		*pass = FALSE;
		etalTestPrintNormal("pass1d FAILED");
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}

	if (!*pass)
	{
		etalTestPrintNormal("pass1 FAILED");
	}

	/*
	 * destroy datapaths
	 */
	if ((etalTestDestroyDatapath(&hDatapathAudio, ETAL_DATA_TYPE_DAB_AUDIO_RAW) != OSAL_OK) ||
		(etalTestDestroyDatapath(&hDatapathData, ETAL_DATA_TYPE_DAB_DATA_RAW) != OSAL_OK))
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestServiceSelectPass2
 *
 **************************/
/*
 * pass2
 *
 *   APPEND DATA1  (RAW)
 *   APPEND DATA2  (RAW)
 *   APPEND DATA3  (RAW)
 *   APPEND DATA4  (RAW)
 *   APPEND DATA5  (RAW)
 *   check number of received packets
 *   Network Packet continuity and CRC
 *   REMOVE all DATA (RAW)
 *
 *   DATA services are all contained in the same subch so must be all accepted
 */
static tSInt etalTestServiceSelectPass2(tBool *pass)
{
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hDatapathData;

	etalTestPrintNormal("* pass2, Select five data sevices from one subch");

	/*
	 * create datapaths
	 */
	if (etalTestSetupDatapath(handledab, &hDatapathData, ETAL_DATA_TYPE_DAB_DATA_RAW, etalTestSinkRawData) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestResetCounters();

	etalTestPrintNormal("* Select data SID1");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Select data SID2");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD2_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Select data SID3");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD3_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Select data SID4");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD4_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Select data SID5");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD5_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if (etalTestServiceSelectWaitAndCheck(ETAL_TEST_SERVSEL_WAIT) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	
	etalTestPrintNormal("= %d raw data frames received", counters.rawDataCounter);
	etalTestPrintNormal("= (expected more than %d )", ETAL_TEST_SERVSEL_PASS2_RAWDATA_COUNT_MIN);
	etalTestPrintAddressCache();
	if (counters.rawDataCounter < ETAL_TEST_SERVSEL_PASS2_RAWDATA_COUNT_MIN)
	{
		*pass = FALSE;
		etalTestPrintNormal("pass2a FAILED");
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}
	// this code is disabled (see etalTestSinkRawData)
	if (counters.rawDataContinuityErrorCount) 
	{
		*pass = FALSE;
		etalTestPrintNormal("pass2b FAILED (%d continuity errors)", counters.rawDataContinuityErrorCount);
	}
	if (counters.rawDataCRCErrorCount)
	{
		*pass = FALSE;
		etalTestPrintNormal("pass2c FAILED (%d CRC errors)", counters.rawDataCRCErrorCount);
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintNormal("* Remove data SID1");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Remove data SID2");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD2_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Remove data SID3");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD3_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Remove data SID4");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD4_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Remove data SID5");
	if ((ret = etal_service_select_data(hDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD5_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* allow some time for the last packets to leave the FIFOs */
	OSAL_s32ThreadWait(10);
	etalTestResetCounters();
	if (etalTestServiceSelectWaitAndCheck(ETAL_TEST_SERVSEL_WAIT) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("= %d raw data frames received", counters.rawDataCounter);
	etalTestPrintNormal("= (expected less than %d)", ETAL_TEST_SERVSEL_PASS1_RAWDATA_COUNT_MAX);

	if (counters.rawDataCounter > ETAL_TEST_SERVSEL_PASS1_RAWDATA_COUNT_MAX)
	{
		*pass = FALSE;
		etalTestPrintNormal("pass2d FAILED");
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}

	if (!*pass)
	{
		etalTestPrintNormal("pass2 FAILED");
	}

	/*
	 * destroy datapaths
	 */
	if (etalTestDestroyDatapath(&hDatapathData, ETAL_DATA_TYPE_DAB_DATA_RAW) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}


/***************************
 *
 * etalTestServiceSelectPass3
 *
 **************************/
/*
 * pass3
 *
 *   APPEND AUDIO1 (RAW)
 *   APPEND AUDIO2 (RAW)
 *   APPEND AUDIO3 (RAW)
 *   APPEND AUDIO4 (RAW)
 *   check number of received packets
 *   REMOVE all
 */
static tSInt etalTestServiceSelectPass3(tBool *pass)
{
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hDatapathAudio;

	etalTestPrintNormal("* pass3, Select four audio subch as raw data");

	/*
	 * create datapaths
	 */
	if (etalTestSetupDatapath(handledab, &hDatapathAudio, ETAL_DATA_TYPE_DAB_AUDIO_RAW, etalTestSinkRawAudio) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestResetCounters();

	etalTestPrintNormal("* Select raw audio SID1");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Select raw audio SID2");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV2_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Select raw audio SID3");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV3_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Select raw audio SID4");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV4_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if (etalTestServiceSelectWaitAndCheck(ETAL_TEST_SERVSEL_WAIT) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	
	etalTestPrintNormal("= %d raw audio frames received", counters.rawAudioCounter);
	etalTestPrintNormal("= (expected more than %d)", ETAL_TEST_SERVSEL_PASS3_RAWAUDIO_COUNT_MIN);
	if (counters.rawAudioCounter < ETAL_TEST_SERVSEL_PASS3_RAWAUDIO_COUNT_MIN)
	{
		*pass = FALSE;
		etalTestPrintNormal("pass3a FAILED");
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintNormal("* Remove raw audio SID1");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Remove raw audio SID2");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV2_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Remove raw audio SID3");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV3_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("* Remove raw audio SID4");
	if ((ret = etal_service_select_data(hDatapathAudio, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV4_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* allow some time for the last packets to leave the FIFOs */
	OSAL_s32ThreadWait(10);
	etalTestResetCounters();
	if (etalTestServiceSelectWaitAndCheck(ETAL_TEST_SERVSEL_WAIT) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("= %d raw audio frames", counters.rawAudioCounter);
	etalTestPrintNormal("= (expected less than %d)", ETAL_TEST_SERVSEL_PASS1_RAWAUDIO_COUNT_MAX);

	if (counters.rawAudioCounter > ETAL_TEST_SERVSEL_PASS1_RAWAUDIO_COUNT_MAX)
	{
		*pass = FALSE;
		etalTestPrintNormal("pass3b FAILED");
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}

	if (!*pass)
	{
		etalTestPrintNormal("pass3 FAILED");
	}

	/*
	 * destroy datapaths
	 */
	if (etalTestDestroyDatapath(&hDatapathAudio, ETAL_DATA_TYPE_DAB_AUDIO_RAW) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

#endif // CONFIG_ETAL_HAVE_ADVTUNE || CONFIG_ETAL_HAVE_ALL_API
#endif // CONFIG_APP_TEST_SERVICE_SELECT_DCOP

/***************************
 *
 * etalTestServiceSelectDCOP
 *
 **************************/
tSInt etalTestServiceSelectDCOP(void)
{
#ifdef CONFIG_APP_TEST_SERVICE_SELECT_DCOP
	ETAL_STATUS ret;
	tBool pass1 = TRUE;
	tBool pass2 = TRUE;
	tBool pass3 = TRUE;
	static tBool firstLoop = TRUE;

	etalTestStartup();

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* tune to DAB (remember that this test uses DE-Bayern unlike most of the others) */

	if (firstLoop)
	{
		/*
		 * To debug the tune timeout issue we enable the DCOP DAB Status autonotifications
		 * We do it by directly accessing the ETAL DCOP communication layer, which is a NO-NO
		 * but currently this command is not integrated in ETAL so this is the only way
		 */
		firstLoop = FALSE;
	}

	etalTestPrintNormal("* Tune to DAB freq %d", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Wait 1s to allow DAB data capture");
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	/*
	 * select an audio service for some feedback
	 */
	if (etalTestDoServiceSelectAudio(handledab, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV3_SID) != OSAL_OK)
	{
		etalTestPrintError("This test requires DE-Bayern ETI file");
		return OSAL_ERROR;
	}

	if (etalTestServiceSelectPass1(&pass1) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("");

	if (etalTestServiceSelectPass2(&pass2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("");

	if (etalTestServiceSelectPass3(&pass3) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	 * clean up
	 */
	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass1 || !pass2 || !pass3)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_DAB
#endif // CONFIG_APP_TEST_SERVICE_SELECT_DCOP
	return OSAL_OK;
}

#ifdef CONFIG_APP_TEST_TUNE_DCOP
/***************************
 *
 * etalTestAutonotifDABStatus
 *
 **************************/
static tVoid etalTestAutonotifDABStatus(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
	// ETAL_HANDLE hReceiver;
	etalAutoNotificationStatusTy *notif;

#if 0
	if (!ETAL_handleIsReceiver(hGeneric))
	{
		ASSERT_ON_DEBUGGING(0);
		return;
	}
#endif
	// hReceiver = hGeneric; // not used
	notif = (etalAutoNotificationStatusTy *)param;
	if (notif->type != autoDABStatus)
	{
		etalTestPrintNormal("Unhandled autonotification");
		return;
	}
	etalTestPrintNormal("DAB Status: reason=%d, ber=%d, mute=%d, sync=%d", notif->status.DABStatus.reason, notif->status.DABStatus.ber, notif->status.DABStatus.mute, notif->status.DABStatus.sync);
}

/***************************
 *
 * etalTestCheckEventsCallback
 *
 **************************/
static void etalTestCheckEventsCallback (void *dataPtr)
{
    //etalTestKeepGoing = false;

    etalTestPrintError("Communication error, ending test");

    (void)dataPtr;
}

#endif // CONFIG_APP_TEST_TUNE_DCOP

/***************************
 *
 * etalTestTuneDCOP
 *
 **************************/
/*
 * Test sequence:
 * 1. Tune <f>
 * 2. Tune 0
 * 3. Clear Database (Memory+NVRAM)
 * 4. goto 1 if required
 *
 * If 1. returns 'No Data' don't execute 2 and 3
 *
 * The test is intended to reproduce the problem of the DCOP returning 'No Data'
 * even when tuned to a valid frequency.
 *
 * This test breaks all the ETAL integration rules because it acceses directly
 * the etalcmd* interfaces instead of using the  ETAL API interfaces which do not
 * provide enough flexibility for this special case.
 *
 * This test does not properly clean the ETAL status so execute on its own
 * or unexpected results will occur.
 *
 * DO NOT USE AS AN EXAMPLE!!!!
 */
tSInt etalTestTuneDCOP(void)
{
#ifdef CONFIG_APP_TEST_TUNE_DCOP // implies CONFIG_APP_TEST_DAB
	ETAL_STATUS ret;
	tBool pass = TRUE;
	tSInt retval;
	static tBool changeBandDone = FALSE;
	static ETAL_HANDLE hReceiver = ETAL_INVALID_HANDLE;
	EtalReceiverAttr attr;
    tSInt callbackHandle;
    EtalProcessingFeatures proc_features;

	etalTestStartup();

    // Register callbacks
    callbackHandle = etalTestRegisterCallbacks(etalTestCheckEventsCallback, ETAL_CALLBACK_REASON_IS_COMM_ERROR);

    if (callbackHandle < 0)
    {
        // Error in registering function, something went wrong
        etalTestPrintError("Failed to register callback");
    }

/* tune to DAB (remember that this test uses DE-Bayern unlike most of the others) */

	if (!changeBandDone)
	{
		etalTestPrintNormal("* Create DAB receiver");
		OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));
		attr.m_Standard = ETAL_BCAST_STD_DAB;
		attr.m_FrontEnds[0] = ETAL_FE_FOR_DAB_TEST;
		attr.m_FrontEndsSize = 1;
		if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_config_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		etalTestPrintNormal("* Created DAB receiver, handle %d", hReceiver);

		etalTestPrintNormal("* Register internal callback");
		if (ETAL_intCbRegister(callAtDABAutonotification, etalTestAutonotifDABStatus, hReceiver, 0) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("ETAL_intCbRegister");
			return OSAL_ERROR;
		}

		etalTestPrintNormal("* Change band to DAB");
        proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		if ((ret = etal_change_band_receiver(hReceiver, ETAL_BAND_DAB3, 0, 0, proc_features)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_change_band_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		changeBandDone = TRUE;
	}

	/*
	 * Tune to valid frequency
	 *
	 * DONT'T USE THIS INTERFACE IN NORMAL APPLICATION CODE!
	 */
	etalTestPrintNormal("* Tune to DAB freq %d", ETAL_VALID_DAB_FREQ);
	retval = ETAL_cmdTune_CMOST(hReceiver, ETAL_VALID_DAB_FREQ);
	if (retval != OSAL_OK)
	{
		etalTestPrintError("CMOST Tune returned error %d", retval);
		/* CMOST error is unexpected, stop here */
		return OSAL_ERROR;
	}
	retval = ETAL_cmdTune_MDR(hReceiver, ETAL_VALID_DAB_FREQ, cmdTuneNormalResponse, 0);
	if ((retval != OSAL_ERROR_TIMEOUT_EXPIRED) &&
		(retval != OSAL_OK))
	{
		pass = FALSE;
		ret = ETAL_RET_ERROR;
		etalTestPrintError("DAB Tune returned error %d", retval);
	}
	else if (retval == OSAL_ERROR_TIMEOUT_EXPIRED)
	{
		pass = FALSE;
		etalTestPrintError("DAB Tune returned NO DATA");
	}
	else
	{
		etalTestPrintNormal("Wait 1s to allow DAB data capture");
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
	}

	/*
	 * tune to 0 and clear DB, only if the Tune <f> did not return an error
	 */
	if (pass)
	{
		/*
		 * Tune to 0
		 *
		 * DONT'T USE THIS INTERFACE IN NORMAL APPLICATION CODE!
		 */
		etalTestPrintNormal("* Tune to DAB freq 0");
		retval = ETAL_cmdTune_MDR(hReceiver, 0, cmdTuneNormalResponse, 0);
		if (retval != OSAL_OK)
		{
			etalTestPrintError("Tune 0 unexpected return: %d", retval);
		}

		/*
		 * clear DB
		 *
		 * DONT'T USE THIS INTERFACE IN NORMAL APPLICATION CODE!
		 */
		etalTestPrintNormal("* Clear DAB Db");
		retval = ETAL_cmdClearDb_MDR(hReceiver);
		if (retval != OSAL_OK)
		{
			etalTestPrintError("Clear Db unexpected return: %d", retval);
		}
	}
	else
	{
		return OSAL_ERROR;
	}
#endif
	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

