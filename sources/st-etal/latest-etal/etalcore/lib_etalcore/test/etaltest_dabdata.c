//!
//!  \file 		etaltest_dabdata.c
//!  \brief 	<i><b> ETAL test, DAB data service functions </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "dabmw_crc.h"
#include "etaltest.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef CONFIG_APP_TEST_DABDATA

/***************************
 * Local Macros
 **************************/
#define ETAL_TEST_DABDATA_MINPACKETLEN   5  // packet header (3 bytes) + packet CRC (2 bytes)
#define ETAL_TEST_DABDATA_SERV1_MIN      30
#define ETAL_TEST_DABDATA_SERV1_MAX      70
#define ETAL_TEST_DABDATA_SERV2_MIN     100
#define ETAL_TEST_DABDATA_SERV2_MAX     200
#define ETAL_TEST_DABDATA_DELTA           5
#define ETAL_TEST_DABDATA_DATA_DELAY   5500
#define ETAL_TEST_DABDATA_NODATA_DELAY ETAL_TEST_ONE_SECOND

#define DABMW_NETWORK_PACKET_CONTINUITY(_buf_)                 (((_buf_)[0] >> 4) & 0x03)
#define DABMW_NETWORK_PACKET_ADDRESS(_buf_)                   ((((_buf_)[0] & 0x03) << 8) | ((_buf_)[1] & 0xFF))
#define DABMW_NETWORK_PACKET_PACKET_CRC(_buf_, _len_)          ((((_buf_)[(_len_)-2] << 8) & 0xFF00) | ((_buf_)[(_len_)-1] & 0x00FF))

/***************************
 * Local types
 **************************/

/***************************
 * Variables
 **************************/
tU32 etalTestDabData1Count;
tU32 etalTestDabData2Count;
tU32 etalTestDabDataLenError;
tU32 etalTestDabDataCRCError;
tU32 etalTestDabDataContinuityError;
tU32 etalTestDabDataUnknownError;
tS8  etalTestDabDataPrevCont[2] = {-1, -1};

/***************************
 * Local function
 **************************/

/***************************
 *
 * etalTestDabDataCheckLen
 *
 **************************/
static tSInt etalTestDabDataCheckLen(tU8 *pBuffer, tU32 len)
{
	if (len < ETAL_TEST_DABDATA_MINPACKETLEN)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestDabDataCheckCRC
 *
 **************************/
static tSInt etalTestDabDataCheckCRC(tU8 *dataPtr, tU32 packetLength)
{
	tU16 crc = (tU16)0xFFFF;
	tS32 crcPacketLength = packetLength - 2; // CRC bytes not part of CRC calculation
	tPU8 crcDataPtr = dataPtr;
	tU16 packetCrc;

	packetCrc = DABMW_NETWORK_PACKET_PACKET_CRC(dataPtr, packetLength);
	while (crcPacketLength > 0)
	{
		crc = DABMW_CRCccitt (crc, *crcDataPtr++);
		crcPacketLength--;
	}

	if ((tU16)0xFFFF != (crc ^ packetCrc))
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestDabDataCheckContinuity
 *
 **************************/
static tSInt etalTestDabDataCheckContinuity(tU8 index, tU8 *pBuffer, tU32 len)
{
	tU8 cont, expected_cont;

	cont = DABMW_NETWORK_PACKET_CONTINUITY(pBuffer);
	
	if (etalTestDabDataPrevCont[index] < 0)
	{
		etalTestDabDataPrevCont[index] = cont;
		return OSAL_OK;
	}
	expected_cont = (etalTestDabDataPrevCont[index] + 1) & 0x03;
	etalTestDabDataPrevCont[index] = (tS8)cont;
	if (cont == expected_cont)
	{
		return OSAL_OK;
	}
	return OSAL_ERROR;
}

/***************************
 *
 * etalTestDabDataCallback
 *
 **************************/
static void etalTestDabDataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
#if 0
	etalTestPrintNormal("DAB Data1 callback invoked for %d bytes", dwActualBufferSize);
	etalTestPrintBuffer(pBuffer, dwActualBufferSize);
#endif
	tU8 index;
	tU16 address;

	address = DABMW_NETWORK_PACKET_ADDRESS(pBuffer);

	if (address == ETAL_DAB_ETI_SERVD1_ADDRESS)
	{
		etalTestDabData1Count++;
		index = 0;
	}
	else if (address == ETAL_DAB_ETI_SERVD2_ADDRESS)
	{
		etalTestDabData2Count++;
		index = 1;
	}
	else
	{
		etalTestPrintError("DAB Data callback received unknown packet address 0x%x", address);
		etalTestDabDataUnknownError++;
		return;
	}

	if (etalTestDabDataCheckLen(pBuffer, dwActualBufferSize) == OSAL_ERROR)
	{
		etalTestDabDataLenError++;
	}
	if (etalTestDabDataCheckCRC(pBuffer, dwActualBufferSize) == OSAL_ERROR)
	{
		etalTestDabDataCRCError++;
	}
	if (etalTestDabDataCheckContinuity(index, pBuffer, dwActualBufferSize) == OSAL_ERROR)
	{
		etalTestDabDataContinuityError++;
	}
}
#endif // CONFIG_APP_TEST_DABDATA

/***************************
 *
 * etalTestDabData
 *
 **************************/
tSInt etalTestDabData(void)
{
#ifdef CONFIG_APP_TEST_DABDATA
	ETAL_HANDLE hDatapath;
	ETAL_STATUS ret;
	tBool pass1 = TRUE;
	tBool pass2 = TRUE;
	EtalDataPathAttr dataPathAttr;
#ifdef CONFIG_APP_TEST_DABDATA_CONTINUOUS
	tU32 test_loop_count;
#endif

	etalTestStartup();

	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Wait some time to allow DAB data capture");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	/*
	 * pass1
	 */

	etalTestDabDataLenError = 0;
	etalTestDabDataCRCError = 0;
	etalTestDabDataContinuityError = 0;
	etalTestDabDataUnknownError = 0;

	etalTestPrintNormal("* Config datapath for Service Select, DATA service");
	hDatapath = ETAL_INVALID_HANDLE;
	dataPathAttr.m_receiverHandle = handledab;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_DAB_DATA_RAW;
	OSAL_pvMemorySet(&dataPathAttr.m_sink, 0x00, sizeof(EtalSink));
	dataPathAttr.m_sink.m_BufferSize = 0x00;
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestDabDataCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for Service Select (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* pass1a, select one RAW data stream */

	etalTestDabData1Count = 0;
	etalTestPrintNormal("pass1a, check single RAW data stream");
	etalTestPrintNormal("* start RAW");
	if ((ret = etal_service_select_data(hDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERVD1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Wait some time to allow DAB data capture (%ds)", ETAL_TEST_DABDATA_DATA_DELAY/ETAL_TEST_ONE_SECOND);
	OSAL_s32ThreadWait(ETAL_TEST_DABDATA_DATA_DELAY);

	etalTestPrintNormal("= pass1a complete, %d callback invocations", etalTestDabData1Count);
	etalTestPrintNormal("= (expected more than %d, less than %d,", ETAL_TEST_DABDATA_SERV1_MIN, ETAL_TEST_DABDATA_SERV1_MAX);
	if ((etalTestDabData1Count > ETAL_TEST_DABDATA_SERV1_MIN) && (etalTestDabData1Count < ETAL_TEST_DABDATA_SERV1_MAX))
	{
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1a FAILED");
	}

	/* pass1b, delete the selected stream */

	etalTestPrintNormal("pass1b, delete RAW data stream");
	etalTestDabData1Count = 0;
	if ((ret = etal_service_select_data(hDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERVD1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Wait some time to ensure DAB stream is stopped (%ds)", ETAL_TEST_DABDATA_NODATA_DELAY/ETAL_TEST_ONE_SECOND);
	OSAL_s32ThreadWait(ETAL_TEST_DABDATA_NODATA_DELAY);

	if (etalTestDabData1Count < ETAL_TEST_DABDATA_DELTA)
	{
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1b FAILED");
	}
	etalTestDabDataPrevCont[0] = -1;

	/* pass1c, select two RAW streams */

	etalTestDabData1Count = 0;
	etalTestDabData2Count = 0;
	etalTestPrintNormal("pass1c, select two RAW data stream");
	etalTestPrintNormal("* Config datapath for Service Select, second DATA service");

	if ((ret = etal_service_select_data(hDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERVD1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data 1 (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if ((ret = etal_service_select_data(hDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERVD2_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data 2 (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Wait some time to allow DAB data capture (%ds)", ETAL_TEST_DABDATA_DATA_DELAY/ETAL_TEST_ONE_SECOND);
	OSAL_s32ThreadWait(ETAL_TEST_DABDATA_DATA_DELAY);

	etalTestPrintNormal("= pass1c complete, %d invocations for address1", etalTestDabData1Count);
	etalTestPrintNormal("=             and  %d invocations for address2", etalTestDabData2Count);
	etalTestPrintNormal("= (expected more than %d, less than %d for address1,", ETAL_TEST_DABDATA_SERV1_MIN, ETAL_TEST_DABDATA_SERV1_MAX);
	etalTestPrintNormal("=           more than %d, less than %d for address2)", ETAL_TEST_DABDATA_SERV2_MIN, ETAL_TEST_DABDATA_SERV2_MAX);
	if (((etalTestDabData1Count > ETAL_TEST_DABDATA_SERV1_MIN) && (etalTestDabData1Count < ETAL_TEST_DABDATA_SERV1_MAX)) &&
		((etalTestDabData2Count > ETAL_TEST_DABDATA_SERV2_MIN) && (etalTestDabData2Count < ETAL_TEST_DABDATA_SERV2_MAX)))
	{
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1c FAILED");
	}

	/* pass1d, delete one of the two streams and ensure no data is received */

	etalTestPrintNormal("pass1d, delete RAW data stream 1");
	etalTestDabData1Count = 0;
	etalTestDabData2Count = 0;
	if ((ret = etal_service_select_data(hDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERVD1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Wait some time to ensure DAB stream is stopped (%ds)", ETAL_TEST_DABDATA_DATA_DELAY/ETAL_TEST_ONE_SECOND);
	OSAL_s32ThreadWait(ETAL_TEST_DABDATA_DATA_DELAY);

	etalTestPrintNormal("= pass1d complete, %d invocations for address1", etalTestDabData1Count);
	etalTestPrintNormal("=             and  %d invocations for address2", etalTestDabData2Count);
	etalTestPrintNormal("= (expected less than %d for address1,", ETAL_TEST_DABDATA_DELTA);
	etalTestPrintNormal("=           more than %d, less than %d for address2)", ETAL_TEST_DABDATA_SERV2_MIN, ETAL_TEST_DABDATA_SERV2_MAX);
	if ((etalTestDabData1Count < ETAL_TEST_DABDATA_DELTA) &&
		((etalTestDabData2Count > ETAL_TEST_DABDATA_SERV2_MIN) && (etalTestDabData2Count < ETAL_TEST_DABDATA_SERV2_MAX)))
	{
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1d FAILED");
	}
	etalTestDabDataPrevCont[0] = -1;

	/* pass1e, delete the other stream */

	etalTestPrintNormal("pass1e, delete RAW data stream 2");
	etalTestDabData1Count = 0;
	etalTestDabData2Count = 0;
	if ((ret = etal_service_select_data(hDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERVD2_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Wait some time to ensure DAB stream is stopped (%ds)", ETAL_TEST_DABDATA_NODATA_DELAY/ETAL_TEST_ONE_SECOND);
	OSAL_s32ThreadWait(ETAL_TEST_DABDATA_NODATA_DELAY);

	if ((etalTestDabData1Count < ETAL_TEST_DABDATA_DELTA) &&
		(etalTestDabData2Count < ETAL_TEST_DABDATA_DELTA))
	{
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1e FAILED");
	}
	etalTestDabDataPrevCont[1] = -1;

	/* pass1f, continuity and CRC checks */

	etalTestPrintNormal("pass1f, continuity and CRC checks");
	if ((etalTestDabDataLenError == 0) &&
		(etalTestDabDataCRCError == 0) &&
		(etalTestDabDataContinuityError == 0) &&
		(etalTestDabDataUnknownError == 0))
	{
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("len errors %d, CRC errors %d, continuity errors %d, unknown address %d", etalTestDabDataLenError, etalTestDabDataCRCError, etalTestDabDataContinuityError, etalTestDabDataUnknownError);
		etalTestPrintNormal("pass1f FAILED");
	}

	if (!pass1)
	{
		etalTestPrintNormal("pass1 FAILED");
	}

#ifdef CONFIG_APP_TEST_DABDATA_CONTINUOUS
	etalTestPrintNormal("Continuous test start");

	test_loop_count       = 0;
	etalTestDabData1Count = 0;
	etalTestDabData2Count = 0;
	if (etalTestDoServiceSelectAudio(handledab, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV3_SID) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if ((ret = etal_service_select_data(hDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERVD1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data 1 (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if ((ret = etal_service_select_data(hDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERVD2_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data 2 (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	while (test_loop_count < 100)
	{
		etalTestPrintNormal("test loop number               %.3d", test_loop_count + 1);
		etalTestPrintNormal("ETI loops                      %.3d", test_loop_count / 4 + 1);
		etalTestPrintNormal("etalTestDabData1Count %d, etalTestDabData2Count %d", etalTestDabData1Count, etalTestDabData2Count);
		etalTestPrintNormal("etalTestDabDataLenError        %.3d", etalTestDabDataLenError);
		etalTestPrintNormal("etalTestDabDataCRCError        %.3d", etalTestDabDataCRCError);
		etalTestPrintNormal("etalTestDabDataContinuityError %.3d", etalTestDabDataContinuityError);
		etalTestPrintNormal("etalTestDabDataUnknownError    %.3d", etalTestDabDataUnknownError);
		etalTestPrintNormal("");
		test_loop_count++;
		OSAL_s32ThreadWait(30 * ETAL_TEST_ONE_SECOND); // 30 sec; note that the ETI repeates every 2 minutes, so every 4 loops
	}
#endif

	/*
	 * end tests
	 */ 

	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass1 || !pass2)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_DABDATA
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

