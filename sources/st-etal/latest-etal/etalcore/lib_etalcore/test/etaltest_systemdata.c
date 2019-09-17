//!
//!  \file 		etaltest_systemdata.c
//!  \brief 	<i><b> ETAL test, systemdata </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

#ifdef CONFIG_APP_TEST_SYSTEMDATA

/***************************
 * Local Macros
 **************************/

/***************************
 * Local types
 **************************/

/***************************
 *
 * etalTestServiceTy
 *
 **************************/
typedef struct
{
	tU32 sid;
	tBool audio;
} etalTestServiceTy;

/***************************
 *
 * etalTestServiceDataTy
 *
 **************************/
typedef struct
{
	tU32  sid;
	tU16  bitmap;
	tU8   charset;
	tChar label[ETAL_DEF_MAX_LABEL_LEN];
} etalTestServiceDataTy;

/***************************
 *
 * etalTestServiceComponentDataTy
 *
 **************************/
typedef struct
{
	tU16  bitmap;
	tU8   charset;
	tChar label[ETAL_DEF_MAX_LABEL_LEN];
} etalTestServiceComponentDataTy;

typedef enum {
	SYSDATA_HDFM_GET_SERVICE_LIST = 1,
	SYSDATA_HDFM_VALIDATE_SERVICE_LIST,
	SYSDATA_HDAM_GET_SERVICE_LIST,
	SYSDATA_HDAM_VALIDATE_SERVICE_LIST
} SystemDataTestTy;

/***************************
 * Local variables
 **************************/
tU32 correctFicNumber;

#ifdef CONFIG_APP_TEST_DAB
etalTestServiceDataTy ServiceInfoReference[2] =
{
 {ETAL_DAB_ETI_SERV1_SID, ETAL_DAB_ETI_SERV1_BITMAP, ETAL_DAB_ETI_SERV1_CHARSET, ETAL_DAB_ETI_SERV1_LABEL},
 {ETAL_DAB_ETI_SERV2_SID, ETAL_DAB_ETI_SERV2_BITMAP, ETAL_DAB_ETI_SERV2_CHARSET, ETAL_DAB_ETI_SERV2_LABEL}
};

etalTestServiceComponentDataTy ServiceComponentInfoReference[2] =
{
 {ETAL_DAB_ETI_SERV1_SC_BITMAP, ETAL_DAB_ETI_SERV1_SC_CHARSET, ETAL_DAB_ETI_SERV1_SC_LABEL},
 {ETAL_DAB_ETI_SERV2_SC_BITMAP, ETAL_DAB_ETI_SERV2_SC_CHARSET, ETAL_DAB_ETI_SERV2_SC_LABEL}
};
#endif // CONFIG_APP_TEST_DAB

#ifdef CONFIG_APP_TEST_HDRADIO_FM
/***************************
 *
 * etalTestServiceListHDFM
 *
 **************************/
static tU32 etalTestServiceListHDFM(EtalServiceList *list)
{
	static etalTestServiceTy reference[ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE] =
	{{0x00, 0}, // second field not used for HD
	 {0x03, 0},
	 {0x05, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0}
	};
	tU32 i, j;

	i = 0;
	j = 0;
	if (list->m_serviceCount != 3)
	{
		return 1;
	}
	for (; (i < ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE) && (j < list->m_serviceCount); i++)
	{
		if (list->m_service[j] != reference[i].sid)
		{
			return 1;
		}
		j++;
	}
	return 0;
}
/***************************
 *
 * etalTestSystemDataHDFM
 *
 **************************/
static tSInt etalTestSystemDataHDFM(tBool *pass, tChar *string1)
{
	ETAL_STATUS ret;
	EtalServiceList serv_list;

	/*
	 * Get Service List HD
	 */
	etalTestPrintReportPassStart(SYSDATA_HDFM_GET_SERVICE_LIST, ETAL_TEST_MODE_HD_FM, "Get service list");
	etalTestPrintNormal("* %s, Get Service List for HD FM", string1);
	if (ETAL_FE_FOR_HD_TEST != ETAL_FE_HANDLE_1)
	{
		/*
		 * for INSTANCE_2 ETAL does not wait for audio acquisition, since it
		 * is a data-only channel; but this means that after the tune
		 * the SIS data may not yet be available, so we need to put an additional
		 * wait here
		 */
		etalTestPrintNormal("Waiting some time for SIS aquisition (7s)");
		OSAL_s32ThreadWait(7 * ETAL_TEST_ONE_SECOND);
	}
	if ((ret = etal_get_service_list(handlehd, ETAL_INVALID_UEID, 0, 0, &serv_list)) != ETAL_RET_SUCCESS)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintError("etal_get_service_list HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	
		etalTestPrintReportPassStart(SYSDATA_HDFM_VALIDATE_SERVICE_LIST, ETAL_TEST_MODE_HD_FM, "Validate service list");
#ifdef CONFIG_TRACE_CLASS_ETAL
		etalTestPrintServiceList(-1, &serv_list);
#endif
		if (etalTestServiceListHDFM(&serv_list))
		{
			*pass = FALSE;
			etalTestPrintNormal("%s FAILED", string1);
			etalTestPrintReportPassEnd(testFailed);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}
	return OSAL_OK;
}
#endif //CONFIG_APP_TEST_HDRADIO_FM

#ifdef CONFIG_APP_TEST_HDRADIO_AM
/***************************
 *
 * etalTestServiceListHDAM
 *
 **************************/
static tU32 etalTestServiceListHDAM(EtalServiceList *list)
{
	static etalTestServiceTy reference[ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE] =
	{{0x00, 0}, // second field not used for HD
	 {0x03, 0},
	 {0x05, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0},
	 {0x00, 0}
	};
	tU32 i, j;

	i = 0;
	j = 0;
	if (list->m_serviceCount != 1)
	{
		return 1;
	}
	for (; (i < ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE) && (j < list->m_serviceCount); i++)
	{
		if (list->m_service[j] != reference[i].sid)
		{
			return 1;
		}
		j++;
	}
	return 0;
}
/***************************
 *
 * etalTestSystemDataHDAM
 *
 **************************/
static tSInt etalTestSystemDataHDAM(tBool *pass, tChar *string1)
{
	ETAL_STATUS ret;
	EtalServiceList serv_list;

	/*
	 * Get Service List HD
	 */
	etalTestPrintReportPassStart(SYSDATA_HDAM_GET_SERVICE_LIST, ETAL_TEST_MODE_HD_AM, "Get service list");
	etalTestPrintNormal("* %s, Get Service List for HD AM", string1);
	if (ETAL_FE_FOR_HD_TEST != ETAL_FE_HANDLE_1)
	{
		/*
		 * for INSTANCE_2 ETAL does not wait for audio acquisition, since it
		 * is a data-only channel; but this means that after the tune
		 * the SIS data may not yet be available, so we need to put an additional
		 * wait here
		 */
		etalTestPrintNormal("Waiting some time for SIS aquisition (7s)");
		OSAL_s32ThreadWait(7 * ETAL_TEST_ONE_SECOND);
	}
	if ((ret = etal_get_service_list(handlehdam, ETAL_INVALID_UEID, 0, 0, &serv_list)) != ETAL_RET_SUCCESS)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintError("etal_get_service_list HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		etalTestPrintReportPassStart(SYSDATA_HDAM_VALIDATE_SERVICE_LIST, ETAL_TEST_MODE_HD_AM, "Validate service list");
#ifdef CONFIG_TRACE_CLASS_ETAL
		etalTestPrintServiceList(-1, &serv_list);
#endif
		if (etalTestServiceListHDAM(&serv_list))
		{
			*pass = FALSE;
			etalTestPrintNormal("%s FAILED", string1);
			etalTestPrintReportPassEnd(testFailed);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}
	return OSAL_OK;
}
#endif //CONFIG_APP_TEST_HDRADIO_AM


#ifdef CONFIG_APP_TEST_DAB
/***************************
 *
 * etalTestServiceListDAB
 *
 **************************/
/*
 * Checks if the passed parameter contains the
 * services listed in the DE-Augsburg-20090713
 */
static tU32 etalTestServiceListDAB(EtalServiceList *list, tBool audio, tBool data)
{
	static etalTestServiceTy reference[ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE] =
	{{0x1715, 1},
	 {0x1716, 1},
	 {0x1d12, 1},
	 {0x1d17, 1},
	 {0xd210, 1},
	 {0xd220, 1},
	 {0xd31a, 1},
	 {0xe0d0106e, 0},
	 {0xe0d1106e, 0},
	 {0xe0d5106e, 0},
	 {0xe0d6106e, 0},
	 {0xe0d7106e, 0}
	};
	tU32 i, j;

	i = 0;
	j = 0;
	if (audio)
	{
		for (; (i < ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE) && (j < list->m_serviceCount); i++)
		{
			if (reference[i].audio)
			{
				if (list->m_service[j] != reference[i].sid)
				{
					return 1;
				}
				j++;
			}
		}
	}
	if (data)
	{
		for (; (i < ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE) && (j < list->m_serviceCount); i++)
		{
			if (!reference[i].audio)
			{
				if (list->m_service[j] != reference[i].sid)
				{
					return 1;
				}
				j++;
			}
		}
	}
	return 0;
}

/***************************
 *
 * etalTestGetServiceListDAB
 *
 **************************/
static tSInt etalTestGetServiceListDAB(tBool *pass, tChar *string, ETAL_HANDLE hReceiver, tU32 eid, tBool audio, tBool data, tBool is_1_5) 
{
	tChar str[20];
	ETAL_STATUS ret;
	EtalServiceList serv_list;

	if (audio && data)
	{
		sprintf(str, "audio and data");
	}
	else if (audio)
	{
		sprintf(str, "audio only");
	}
	else if (data)
	{
		sprintf(str, "data only");
	}

	etalTestPrintNormal("* %s, Get Service List for ensemble 0x%x, %s", string, eid, str);
	OSAL_pvMemorySet(&serv_list, 0x00, sizeof(serv_list));
	if ((ret = etal_get_service_list(hReceiver, eid, audio, data, &serv_list)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_service_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if (!is_1_5)
	{
		if (etalTestServiceListDAB(&serv_list, audio, data))
		{
			*pass = FALSE;
		}
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintServiceList(eid, &serv_list);
#endif
	if (!*pass)
	{
		etalTestPrintNormal("%s FAILED", string);
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestGetServiceDataDAB
 *
 **************************/
/*
 * Get Service and Service Component information and compare with
 * stored values. To keep things simple, only two pre-defined services
 * are used, each one containing only one service component; only
 * the label-related information are checked.
 *
 * <index> is an index into the predefined array identifying
 * one of the two services of the test.
 */
static tSInt etalTestGetServiceDataDAB(tBool *pass, tChar *string, tU32 eid, tU8 index) 
{
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo;
	etalTestServiceDataTy *sref = &ServiceInfoReference[index];
	etalTestServiceComponentDataTy *scref = &ServiceComponentInfoReference[index];
	ETAL_STATUS ret;

	etalTestPrintNormal("* %s, Get Service Data for service 0x%x", string, sref->sid);
	if ((ret = etal_get_specific_service_data_DAB(ETAL_DAB_ETI_UEID, sref->sid, &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_service_data_DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if ((OSAL_s32StringCompare(sref->label, svinfo.m_serviceLabel) != 0) ||
		(sref->bitmap != svinfo.m_serviceLabelCharflag) ||
		(sref->charset != svinfo.m_serviceLabelCharset))
	{
		etalTestPrintError("***sref compared failed expecting ***\n");
		etalTestPrintError("expecting sref->label %s, received svinfo.m_serviceLabel %s, compare result\n", sref->label, svinfo.m_serviceLabel, OSAL_s32StringCompare(sref->label, svinfo.m_serviceLabel));
		etalTestPrintError("bitmap expecting scref->bitmap %d, received scinfo.m_scInfo[0].m_scLabelCharflag %d\n", sref->bitmap, svinfo.m_serviceLabelCharflag);
		etalTestPrintError("charset expecting scref->charset %d, received scinfo.m_scInfo[0].m_scLabelCharset %d\n", sref->charset, svinfo.m_serviceLabelCharset);
		etalTestPrintError("----------------\n");
		*pass = FALSE;
	}
	else if ((scinfo.m_scCount != 1) ||
		(OSAL_s32StringCompare(scref->label, scinfo.m_scInfo[0].m_scLabel) != 0) ||
		(scref->bitmap != scinfo.m_scInfo[0].m_scLabelCharflag) ||
		(scref->charset != scinfo.m_scInfo[0].m_scLabelCharset))
	{
		etalTestPrintError("***scref compared failed expecting ***\n");
		etalTestPrintError("scinfo.m_scCount %d expect 1\n", scinfo.m_scCount);
		etalTestPrintError("expecting scref->label %s, received scinfo.m_scInfo[0].m_scLabel %s, compare result\n", scref->label, scinfo.m_scInfo[0].m_scLabel, OSAL_s32StringCompare(scref->label, scinfo.m_scInfo[0].m_scLabel));
		etalTestPrintError("bitmap expecting scref->bitmap %d, received scinfo.m_scInfo[0].m_scLabelCharflag %d\n", scref->bitmap, scinfo.m_scInfo[0].m_scLabelCharflag);
		etalTestPrintError("charset expecting scref->charset %d, received scinfo.m_scInfo[0].m_scLabelCharset %d\n", scref->charset, scinfo.m_scInfo[0].m_scLabelCharset);
		etalTestPrintError("----------------\n");	
		*pass = FALSE;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintServiceDataDAB(sref->sid, &svinfo, &scinfo);
#endif

	if (!*pass)
	{
		etalTestPrintNormal("%s FAILED", string);
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestSystemDataCallback
 *
 **************************/
static tVoid etalTestSystemDataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	#define ETAL_TEST_SYSDATA_NAMELEN 32

	tChar name[ETAL_TEST_SYSDATA_NAMELEN];

	FILE *fp;
	static tU32 count = 0;

	etalTestPrintNormal("FIC callback received, packet size %d", dwActualBufferSize);

	OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_SYSDATA_NAMELEN, "FIC-%d.raw", count++);
	fp = fopen(name, "w");
	if (fp == NULL)
	{
		etalTestPrintError("opening file %s", name);
	}
	else
	{
		if (fwrite((tPVoid)(pBuffer), (size_t)1, (size_t)(dwActualBufferSize), fp) != (size_t)(dwActualBufferSize))
		{
			etalTestPrintError("writing to file %s", name);
		}
		if (0==(dwActualBufferSize%32))
		{
			correctFicNumber++;
		}
		fclose(fp);
	}
}

/***************************
 *
 * etalTestSystemDataDAB
 *
 **************************/
static tSInt etalTestSystemDataDAB(tBool *pass1, tChar *string1, tBool *pass2, tChar *string2, tBool *pass3, tChar *string3, tBool *pass4, tChar *string4, tBool *pass5, tChar *string5, tBool is_1_5)
{
	#define SYSDATA_STRING_LEN 15
	ETAL_STATUS ret;
	ETAL_HANDLE hDatapath;
	EtalDataPathAttr dataPathAttr;
	EtalEnsembleList ens_list;
	tU8 charset;
	tChar label[ETAL_DEF_MAX_LABEL_LEN];
	tU16 bitmap;
	tChar string[SYSDATA_STRING_LEN];
	tU32 eid, index;

	/*
	 * pass1 , Get Ensemble list
	 */
	etalTestPrintNormal("* %s, Get Ensemble List", string1);
	OSAL_pvMemorySet(&ens_list, 0x00, sizeof(ens_list));
	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if (is_1_5)
	{
		if (ens_list.m_ensembleCount < 2)
		{
			*pass1 = FALSE;
		}
	}
	else
	{
		/* in AGRATE the DCOP might detect signal also from the air,
		 * depending on the environment condition. In this case we
		 * can only hope that the first entry is the one detected
		 * from the signal generator, otherwise the test will fail
		 */
		if ((ens_list.m_ensembleCount < 1) ||
			(ens_list.m_ensemble[0].m_ECC != ETAL_DAB_ETI_ECC) ||
			(ens_list.m_ensemble[0].m_ensembleId != ETAL_DAB_ETI_EID) ||
			(ens_list.m_ensemble[0].m_frequency != ETAL_VALID_DAB_FREQ))
		{
			*pass1 = FALSE;
		}
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);
#endif

	if (!*pass1)
	{
		etalTestPrintNormal("%s FAILED", string1);
	}

	for (index = 0; index < ens_list.m_ensembleCount; index++)
	{
		if (is_1_5)
		{
			eid = ens_list.m_ensemble[index].m_ECC << 16 | ens_list.m_ensemble[index].m_ensembleId;
		}
		else
		{
			eid = ETAL_DAB_ETI_UEID;
		}
		/*
		 * pass2, Get Ensemble Data
		 */
		etalTestPrintNormal("* %s, Get Ensemble Data for ensemble 0x%x", string2, eid);
		OSAL_pvMemorySet(&label, 0x00, sizeof(ETAL_DEF_MAX_LABEL_LEN));
		if ((ret = etal_get_ensemble_data(eid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_get_ensemble_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		if (!is_1_5)
		{
			if ((!OSAL_s32StringCompare(ETAL_DAB_ETI_LABEL, label)) ||
				(bitmap != ETAL_DAB_ETI_BITMAP) ||
				(charset != ETAL_DAB_ETI_CHARSET))
			{
				*pass2 = FALSE;
			}
		}

#ifdef CONFIG_TRACE_CLASS_ETAL
		etalTestPrintEnsembleData(eid, charset, label, bitmap);
#endif
		if (!*pass2)
		{
			etalTestPrintNormal("%s FAILED", string2);
		}
		
		/*
		 * pass3, Get Service List
		 *
		 * etal_get_service_list uses the receiver handle
		 * to check if the broadcast mode is DAB or HD; for DAB
		 * it is not used afterwards because the information is fetched
		 * in the landscape so independently from the application
		 */
		OSAL_s32NPrintFormat(string, SYSDATA_STRING_LEN, "%sa", string3);
		if (etalTestGetServiceListDAB(pass3, string, handledab, eid, 1, 1, is_1_5) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		OSAL_s32NPrintFormat(string, SYSDATA_STRING_LEN, "%sb", string3);
		if (etalTestGetServiceListDAB(pass3, string, handledab, eid, 1, 0, is_1_5) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		OSAL_s32NPrintFormat(string, SYSDATA_STRING_LEN, "%sc", string3);
		if (etalTestGetServiceListDAB(pass3, string, handledab, eid, 0, 1, is_1_5) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (!is_1_5)
		{
			break;
		}
	}

	/*
	 * pass4, Get Service Data
	 */
	OSAL_s32NPrintFormat(string, SYSDATA_STRING_LEN, "%sa", string4);
	if (etalTestGetServiceDataDAB(pass4, string, ETAL_DAB_ETI_UEID, 0) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	OSAL_s32NPrintFormat(string, SYSDATA_STRING_LEN, "%sb", string4);
	if (etalTestGetServiceDataDAB(pass4, string, ETAL_DAB_ETI_UEID, 1) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	 * pass5, Get FIC
	 */
	etalTestPrintNormal("* %s, Get FIC", string5);

	correctFicNumber=0;

//config_datapath ETAL_DATA_TYPE_DAB_FIC
	hDatapath = ETAL_INVALID_HANDLE;
	OSAL_pvMemorySet((tPVoid)&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
	dataPathAttr.m_receiverHandle = handledab;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_DAB_FIC;
	dataPathAttr.m_sink.m_BufferSize = 0x00;
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestSystemDataCallback;

	if ((ret = etal_config_datapath(&hDatapath,&dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for FIC (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if ((ret = etal_get_fic(handledab)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_fic (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(500);

	if (!correctFicNumber) *pass5=false;


	// stop fic reception
	
	if ((ret = etal_stop_fic(handledab)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_fic (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}	

	etal_destroy_datapath(&hDatapath);

	return OSAL_OK;
}

/***************************
 *
 * etalTestSystemDataDAB1_5
 *
 **************************/
#if defined (CONFIG_APP_TEST_SYSTEMDATA_1_5)
static tSInt etalTestSystemDataDAB1_5(tBool *pass)
{
	ETAL_STATUS ret;
	tBool pass10 = TRUE;
	tBool pass11 = TRUE;
	tBool pass12 = TRUE;
	tBool pass13 = TRUE;
	ETAL_HANDLE handledab1_5 = ETAL_INVALID_HANDLE;

	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB1_5, &handledab1_5) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	 * tune the AUDIO receiver
	 */
	if (etalTestDoServiceSelectAudio(handledab, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV3_SID) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	 * tune the DATA receiver
	 */
	etalTestPrintNormal("* Tune to DAB freq %d", ETAL_VALID_DAB_FREQ1_5);
	if ((ret = etal_tune_receiver(handledab1_5, ETAL_VALID_DAB_FREQ1_5)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver DAB 1.5 (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("Wait some time to allow DAB data capture");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	/*
	 * test data:
	 * should get data from the foreground and also the background, so
	 * two sets of data
	 */

	if (etalTestSystemDataDAB(&pass10, "pass10", &pass11, "pass11", &pass12, "pass12", &pass13, "pass13", TRUE) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass10 || !pass11 || !pass12 || !pass13)
	{
		*pass = FALSE;
	}

	/*
	 * cleanup
	 */
	if (etalTestUndoConfigSingle(&handledab1_5) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_SYSTEMDATA_1_5
#endif // CONFIG_APP_TEST_DAB
#endif // CONFIG_APP_TEST_SYSTEMDATA
	
/***************************
 *
 * etalTestSystemData
 *
 **************************/
tSInt etalTestSystemData(void)
{ 
#ifdef CONFIG_APP_TEST_SYSTEMDATA
	tBool pass1 = TRUE;
	tBool pass2 = TRUE;
	tBool pass3 = TRUE;
	tBool pass4 = TRUE;
	tBool pass5 = TRUE;
	tBool pass6 = TRUE;
	tBool pass7 = TRUE;
	tBool pass8 = TRUE;

	etalTestStartup();

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_DAB2, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Wait some time to allow DAB data capture");
	OSAL_s32ThreadWait(2 * ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	if (etalTestSystemDataDAB(&pass1, "pass1", &pass2, "pass2", &pass3, "pass3", &pass4, "pass4", &pass5, "pass5", FALSE) != OSAL_OK)
	{
		return OSAL_ERROR;
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

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestSystemDataHDFM(&pass6, "pass6") != OSAL_OK)
	{
		return OSAL_ERROR;
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

#ifdef CONFIG_APP_TEST_HDRADIO_AM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_AM, &handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestSystemDataHDAM(&pass7, "pass7") != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_AM

#if defined (CONFIG_APP_TEST_SYSTEMDATA_1_5) && defined (CONFIG_APP_TEST_DAB)
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestSystemDataDAB1_5(&pass8) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_SYSTEMDATA_1_5

	if (!pass1 || !pass2 || !pass3 || !pass4 || !pass5 || !pass6 || !pass7 || !pass8)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_SYSTEMDATA
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

