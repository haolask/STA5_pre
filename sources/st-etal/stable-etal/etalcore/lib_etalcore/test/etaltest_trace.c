//!
//!  \file 		etaltest_trace.c
//!  \brief 	<i><b> ETAL test trace and log functionalities </b></i>
//!  \details   
//!
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*!
 * \def		ETAL_TRACE_COMPACT_OUTPUT
 * 			If defined outputs spaces instead of tabs to obtain
 * 			a more compact print.
 *
 * 			Defaults to defined.
 */
#define ETAL_TRACE_COMPACT_OUTPUT

/*!
 * \def		ETAL_TRACE_STRING_LEN
 * 			Size in bytes of the intermediate buffer used to create and format
 * 			strings before outputting them.
 *
 * 			Defaults to 30, can be customized.
 */
#define ETAL_TRACE_STRING_LEN      30
/*!
 * \def		ETAL_TRACE_LONG_STRING_LEN
 * 			Size in bytes of the intermediate buffer used to create and format
 * 			**long** strings before outputting them.
 *
 * 			Defaults to 100, can be customized.
 */
#define ETAL_TRACE_LONG_STRING_LEN 100

#ifdef ETAL_TRACE_COMPACT_OUTPUT
/*! \def	PREFIX0
 * 			String prefix, 0 units */
#define PREFIX0 ""
/*! \def	PREFIX1
 * 			String prefix, 1 unit */
#define PREFIX1 "  "
/*! \def	PREFIX2
 * 			String prefix, 2 units */
#define PREFIX2 "    "
/*! \def	PREFIX3
 * 			String prefix, 3 units */
#define PREFIX3 "      "
/*! \def	PREFIX4
 * 			String prefix, 4 units */
#define PREFIX4 "        "
/*! \def	PREFIX5
 * 			String prefix, 5 units */
#define PREFIX5 "          "
/*! \def	PREFIX_UNIT
 * 			String prefix */
#define PREFIX_UNIT "  "
#else
/*! \def	PREFIX0
 * 			String prefix, 0 units */
#define PREFIX0 ""
/*! \def	PREFIX1
 * 			String prefix, 1 unit */
#define PREFIX1 "\t"
/*! \def	PREFIX2
 * 			String prefix, 2 units */
#define PREFIX2 "\t\t"
/*! \def	PREFIX3
 * 			String prefix, 3 units */
#define PREFIX3 "\t\t\t"
/*! \def	PREFIX4
 * 			String prefix, 4 units */
#define PREFIX4 "\t\t\t\t"
/*! \def	PREFIX5
 * 			String prefix, 5 units */
#define PREFIX5 "\t\t\t\t\t"
/*! \def	PREFIX0
 * 			String prefix */
#define PREFIX_UNIT "\t"
#endif
/*! \def	PREFIX_LEN
 * 			Longest prefix size */
#define PREFIX_LEN (sizeof(PREFIX5))

#define ETAL_TEST_REPORT_STRING_LEN   90
#define ETAL_TEST_REPORT_RES_COL      (ETAL_TEST_REPORT_STRING_LEN - 8)

/*****************************************************************
| local types

|----------------------------------------------------------------*/
/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
tChar etalTestReportString[ETAL_TEST_REPORT_STRING_LEN];
tU32  etalTestReportTestIndex;
tU32  etalTestReportPassIndex;

#if 0
/***************************
 *
 * etalTestPrintBuffer
 *
 **************************/
tVoid etalTestPrintBuffer(tChar *prefix, tU8* pBuffer, tU32 len)
{
	const tU32 MAX_PRINTED_DATA_BYTES = 16;
	tChar str[MAX_PRINTED_DATA_BYTES * 3 + 1];
	tU32 i;

	for (i = 0; (i < len) && (i < MAX_PRINTED_DATA_BYTES); i++)
	{
		OSAL_s32NPrintFormat(str + 3 * i, MAX_PRINTED_DATA_BYTES - 3 * i, "%.2x ", pBuffer[i]);
	}
	str[3 * i + 1] = '\0';

	if (len > MAX_PRINTED_DATA_BYTES)
	{
		etalTestPrint(TR_LEVEL_SYSTEM_MIN, "TRUNC %s %s", prefix, str);
	}
	else
	{
		etalTestPrint(TR_LEVEL_SYSTEM_MIN, "%s %s", prefix, str);
	}
}
#endif

/***************************
 *
 * ETAL_Standard2Ascii
 *
 **************************/
/*!
 * \brief		Convert an #EtalBcastStandard to a string
 * \param[in]	std - the standard to convert
 * \return		A pointer to the string, or to "Illegal" if *std* does
 * 				not correspond to any known value.
 * \callgraph
 * \callergraph
 */
tCString etalTestStandard2Ascii(EtalBcastStandard std)
{
#if defined(CONFIG_TRACE_CLASS_ETAL)
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
	}
	return "Illegal";
#else
	return NULL;
#endif // CONFIG_TRACE_ENABLE
}

#if defined(CONFIG_TRACE_CLASS_ETAL)

/***************************
 *
 * etalTestPrintBroadcastStandard
 *
 **************************/
/*!
 * \brief		Decodes and prints the broadcast standards
 * \param[in]	std - bitmap of #EtalBcastStandard
 * \param[in]	prefix - string of spaces used to align the output
 * \callgraph
 * \callergraph
 */
static tVoid etalTestPrintBroadcastStandard(tU32 std, tChar *prefix)
{
	etalTestPrintNormal("%sBroadcastStandard 0x%x", prefix, std);

	if ((std & ETAL_BCAST_STD_DAB) == ETAL_BCAST_STD_DAB)
	{
		etalTestPrintNormal("%s%sDAB", prefix, PREFIX_UNIT);
	}
	if ((std & ETAL_BCAST_STD_DRM) == ETAL_BCAST_STD_DRM)
	{
		etalTestPrintNormal("%s%sDRM", prefix, PREFIX_UNIT);
	}
	if ((std & ETAL_BCAST_STD_FM) == ETAL_BCAST_STD_FM)
	{
		etalTestPrintNormal("%s%sFM", prefix, PREFIX_UNIT);
	}
	if ((std & ETAL_BCAST_STD_AM) == ETAL_BCAST_STD_AM)
	{
		etalTestPrintNormal("%s%sAM", prefix, PREFIX_UNIT);
	}
	if ((std & ETAL_BCAST_STD_HD_FM) == ETAL_BCAST_STD_HD_FM)
	{
		etalTestPrintNormal("%s%sHDFM", prefix, PREFIX_UNIT);
	}
	if ((std & ETAL_BCAST_STD_HD_AM) == ETAL_BCAST_STD_HD_AM)
	{
		etalTestPrintNormal("%s%sHDAM", prefix, PREFIX_UNIT);
	}
}

/***************************
 *
 * etalTestPrintDataType
 *
 **************************/
/*!
 * \brief		Decodes and prints the broadcast standards
 * \param[in]	dt - bitmap of #EtalBcastDataType
 * \param[in]	prefix - string of spaces used to align the output
 * \callgraph
 * \callergraph
 */
static tVoid etalTestPrintDataType(tU32 dt, tChar *prefix)
{
	etalTestPrintNormal("%sDataType          0x%x", prefix, dt);

	if ((dt & ETAL_DATA_TYPE_AUDIO) == ETAL_DATA_TYPE_AUDIO)
	{
		etalTestPrintNormal("%s%sAudio", prefix, PREFIX_UNIT);
	}
	if ((dt & ETAL_DATA_TYPE_DCOP_AUDIO) == ETAL_DATA_TYPE_DCOP_AUDIO)
	{
		etalTestPrintNormal("%s%sDCOP Audio", prefix, PREFIX_UNIT);
	}
	if ((dt & ETAL_DATA_TYPE_DATA_SERVICE) == ETAL_DATA_TYPE_DATA_SERVICE)
	{
		etalTestPrintNormal("%s%sData Service", prefix, PREFIX_UNIT);
	}
	if ((dt & ETAL_DATA_TYPE_DAB_DATA_RAW) == ETAL_DATA_TYPE_DAB_DATA_RAW)
	{
		etalTestPrintNormal("%s%sDAB Data RAW", prefix, PREFIX_UNIT);
	}
	if ((dt & ETAL_DATA_TYPE_DAB_AUDIO_RAW) == ETAL_DATA_TYPE_DAB_AUDIO_RAW)
	{
		etalTestPrintNormal("%s%sDAB Audio RAW", prefix, PREFIX_UNIT);
	}
	if ((dt & ETAL_DATA_TYPE_DAB_FIC) == ETAL_DATA_TYPE_DAB_FIC)
	{
		etalTestPrintNormal("%s%sDAB FIC", prefix, PREFIX_UNIT);
	}
	if ((dt & ETAL_DATA_TYPE_TEXTINFO) == ETAL_DATA_TYPE_TEXTINFO)
	{
		etalTestPrintNormal("%s%sTextinfo", prefix, PREFIX_UNIT);
	}
	if ((dt & ETAL_DATA_TYPE_FM_RDS) == ETAL_DATA_TYPE_FM_RDS)
	{
		etalTestPrintNormal("%s%sFM RDS", prefix, PREFIX_UNIT);
	}
	if ((dt & ETAL_DATA_TYPE_FM_RDS_RAW) == ETAL_DATA_TYPE_FM_RDS_RAW)
	{
		etalTestPrintNormal("%s%sFM RDS RAW", prefix, PREFIX_UNIT);
	}
}
/***************************
 *
 * etalTestPrintFrontend
 *
 **************************/
/*!
 * \brief		Prints all the standards and data types supported by a tuner
 * \param[in]	p - pointer to the Tuner description
 * \callgraph
 * \callergraph
 */
static tVoid etalTestPrintFrontend(EtalTuner *tun)
{
	tU16 i;
	tChar prefix[PREFIX_LEN];
		
	for (i = 0; i < ETAL_CAPA_MAX_FRONTEND_PER_TUNER; i++)
	{
	OSAL_szStringCopy(prefix, PREFIX1);
	etalTestPrintNormal("%sFrontend %d", prefix, i);
	OSAL_szStringCopy(prefix, PREFIX2);
	etalTestPrintBroadcastStandard(tun->m_standards[i], prefix);
	etalTestPrintDataType(tun->m_dataType[i], prefix);
	}
}

/***************************
 *
 * etalTestDeviceToString
 *
 **************************/
/*!
 * \brief		Prints the device type
 * \param[in]	dev_type - the device
 * \callgraph
 * \callergraph
 */
static tChar *etalTestDeviceToString(EtalDeviceType dev_type)
{
	switch (dev_type)
	{
		case deviceUnknown:
			return "undefined";
		case deviceCMOST:
			return "CMOST";
		case deviceSTAR:
			return "STAR";
		case deviceSTARS:
			return "STAR-S";
		case deviceSTART:
			return "STAR-T";
		case deviceDOT:
			return "DOT";
		case deviceDOTS:
			return "DOT-S";
		case deviceDOTT:
			return "DOT-T";
		case deviceDCOP:
			return "DCOP";
		case deviceMDR:
			return "DAB DCOP";
		case deviceHD:
			return "HD DCOP";
		default:
			return "Illegal";
	}
}

/***************************
 *
 * etalTestBusToString
 *
 **************************/
/*!
 * \brief		Prints the device type
 * \param[in]	dev_type - the device
 * \callgraph
 * \callergraph
 */
static tChar *etalTestBusToString(EtalDeviceBus bus)
{
	switch (bus)
	{
		case ETAL_BusI2C:
			return "I2C";
		case ETAL_BusSPI:
			return "SPI";
		default:
			return "Illegal";
	}
}

/***************************
 *
 * etalTestPrintDevice
 *
 **************************/
/*!
 * \brief		Prints a device description
 * \param[in]	dev - pointer to the ETAL device description
 * \callgraph
 * \callergraph
 */
static tVoid etalTestPrintDevice(const EtalDeviceDesc *dev)
{
	tChar prefix[PREFIX_LEN];
		
	OSAL_szStringCopy(prefix, PREFIX0);

	etalTestPrintNormal("%sDevice type: %s", prefix, etalTestDeviceToString(dev->m_deviceType));
	etalTestPrintNormal("%sDevice  bus: %s", prefix, etalTestBusToString(dev->m_busType));
	etalTestPrintNormal("%sDevice addr: 0x%x", prefix, dev->m_busAddress);
	etalTestPrintNormal("%sDevice  chn: %d", prefix, dev->m_channels);
}
#endif // CONFIG_TRACE_CLASS_ETAL

/***************************
 *
 * etalTestPrintCapabilites
 *
 **************************/
/*!
 * \brief		Prints the content of the ETAL capabilities
 * \details		The function prints the list of Frontend identifiers and,
 * 				for each supported broadcast standard, the list
 * 				of supported modes (through the #etalTestPrintFrontend).
 * \param[in]	cap - pointer to the ETAL capability variable
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintCapabilites(EtalHwCapabilities *cap)
{
#ifdef CONFIG_TRACE_CLASS_ETAL
	tChar prefix[PREFIX_LEN];
	tU32 i;
		
	OSAL_szStringCopy(prefix, PREFIX0);

	etalTestPrintNormal("%sHardware Capabilities", prefix);
	etalTestPrintNormal("%s", prefix);

	etalTestPrintNormal("%sDCOP", prefix);
	etalTestPrintDevice(&cap->m_DCOP);

	etalTestPrintNormal("%sTUNERs", prefix);
	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		etalTestPrintDevice(&cap->m_Tuner[i].m_TunerDevice);
		etalTestPrintFrontend(&cap->m_Tuner[i]);
	}

#endif // CONFIG_TRACE_CLASS_ETAL
}


/***************************
 *
 * etalTestPrintRDSRaw
 *
 **************************/
/*!
 * \brief		Prints a RAW RDS data container
 * \remark		The function produces output only if CONFIG_TRACE_CLASS_ETAL is TR_LEVEL_USER_4 or greater
 * \param[in]	rds - pointer to a RAW RDS data container
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintRDSRaw(EtalRDSRaw *rds, ETAL_HANDLE hReceiver)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)
    tU32 RNR, i, raw_rds_data;

    if (rds->len >= 3)
    {
        etalTestPrintNormal("%d RDS Raw data, Len: %d", hReceiver, rds->len);

		etalTestGetParameterCMOST(rds->raw_data.m_RNR, &RNR);

        etalTestPrintNormal("RNR: %s %s %s %s %s %s dev %d Hz", ((RNR & STAR_RNR_DATARDY)?"DATARDY":"       "), ((RNR & STAR_RNR_SYNC)?"SYNC":"    "), 
                    ((RNR & STAR_RNR_BOFL)?"BOFL":"    "), ((RNR & STAR_RNR_BNE)?"BNE":"   "), ((RNR & STAR_RNR_TUNCH2)?"TUNCH2":"      "), 
                    ((RNR & STAR_RNR_TUNCH1)?"TUNCH1":"      "), ((RNR & STAR_RNR_RDS_deviation) >> 4));

        for(i = 0; i < (rds->len - 3); i += 3)
        {
			etalTestGetParameterCMOST(&(rds->raw_data.m_RDS_Data[i]), &raw_rds_data);

            etalTestPrintNormal("ERRCOUNT: %d, RDS block offset %c%c, RDS raw data: 0x%04X", ((raw_rds_data & START_RDS_DATA_ERRCOUNT_MASK) >> START_RDS_DATA_ERRCOUNT_SHIFT),
                        ((raw_rds_data & START_RDS_DATA_BLOCKID_MASK) >> START_RDS_DATA_BLOCKID_SHIFT) + 65,
                        ((raw_rds_data & START_RDS_DATA_CTYPE_MASK) >> START_RDS_DATA_CTYPE_SHIFT)?'\'':' ',
                        (raw_rds_data & START_RDS_DATA_MASK));
        }
    }
#endif // defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)
}

#if defined (CONFIG_ETAL_HAVE_ETALTML)
/***************************
 *
 * etalTestPrintRDS
 *
 **************************/
/*!
 * \brief		Prints a decoded RDS data container
 * \param[in]	rds - pointer to a decoded RDS data container
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintRDS(EtalRDSData *rds, tPChar prefix)
{
#ifdef CONFIG_TRACE_CLASS_ETAL
	tU32 i;

	if (rds->m_validityBitmap == 0)
	{
		return;
	}
	etalTestPrintNormal("%sRDS decoded data 0x%03x", prefix, rds->m_validityBitmap);

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
	{
	etalTestPrintNormal("%sPI          : 0x%x", PREFIX0, rds->m_PI);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_DI) == ETAL_DECODED_RDS_VALID_DI)
	{
	etalTestPrintNormal("%sDI          : 0x%x", PREFIX0, rds->m_DI);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
	{
	etalTestPrintNormal("%sPS          : %.*s", PREFIX0, ETAL_DEF_MAX_PS_LEN, rds->m_PS);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
	{
	etalTestPrintNormal("%sRT          : %.*s", PREFIX0, ETAL_DEF_MAX_RT_LEN, rds->m_RT);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TOM) == ETAL_DECODED_RDS_VALID_TOM)
	{
	etalTestPrintNormal("%sTime(hour)  : %d", PREFIX0, rds->m_timeHour);
	etalTestPrintNormal("%sTime(min)   : %d", PREFIX0, rds->m_timeMinutes);
	etalTestPrintNormal("%sTime(offs)  : %d", PREFIX0, rds->m_offset);
	etalTestPrintNormal("%sTime(MJD)   : %d", PREFIX0, rds->m_MJD);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_AF) == ETAL_DECODED_RDS_VALID_AF)
	{
	etalTestPrintNormal("%sAFListPI    : 0x%x", PREFIX0, rds->m_AFListPI);
	etalTestPrintNormal("%sAFListLen   : %d", PREFIX0, rds->m_AFListLen);
	etalTestPrintNormal("%sAFList      : ", PREFIX0);
	for (i = 0; i < rds->m_AFListLen; i++)
	{
	etalTestPrintNormal("%s%6d MHz", PREFIX1, 87500 + 100 * (tU32)rds->m_AFList[i]);
	}
	}
	
	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PTY) == ETAL_DECODED_RDS_VALID_PTY)
	{
	etalTestPrintNormal("%sPTY         : %d", PREFIX0, rds->m_PTY);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TP) == ETAL_DECODED_RDS_VALID_TP)
	{
	etalTestPrintNormal("%sTP          : %d", PREFIX0, rds->m_TP);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_TA) == ETAL_DECODED_RDS_VALID_TA)
	{
	etalTestPrintNormal("%sTA          : %d", PREFIX0, rds->m_TA);
	}

	if ((rds->m_validityBitmap & ETAL_DECODED_RDS_VALID_MS) == ETAL_DECODED_RDS_VALID_MS)
	{
	etalTestPrintNormal("%sMS          : %d", PREFIX0, rds->m_MS);
	}
#endif // CONFIG_TRACE_CLASS_ETAL
}

/***************************
 *
 * etalTestPrintRadioText
 *
 **************************/
/*!
 * \brief		Prints a RadioText (i.e. TextInfo) container
 * \remark		The function prints the fields only if they are flagged
 * 				as new. If there is nothing new, it prints "Nothing new".
 * \param[in]	radio_text - pointer to a TextInfo container
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintRadioText(EtalTextInfo *radio_text)
{
#if defined  CONFIG_TRACE_CLASS_ETAL

	etalTestPrintNormal("%sRadio text:\n", PREFIX0);
	etalTestPrintNormal("%sBroadcast Standard: %s\n", PREFIX1, etalTestStandard2Ascii(radio_text->m_broadcastStandard));
	etalTestPrintNormal("%sService name %s: %.*s\n", PREFIX1, (radio_text->m_serviceNameIsNew == TRUE)?"is new":"      ", 
						ETAL_DEF_MAX_SERVICENAME, radio_text->m_serviceName);
	etalTestPrintNormal("%sCurrent Info %s: %s\n", PREFIX1, (radio_text->m_currentInfoIsNew == TRUE)?"is new":"      ", 
						radio_text->m_currentInfo);
#endif // CONFIG_TRACE_CLASS_ETAL
}
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML)

/***************************
 *
 * etalTestPrintEnsembleList
 *
 **************************/
/*!
 * \brief		Prints the contents of an ensemble list container
 * \details		For each Ensemble the function prints the ECC, Eid
 * 				and frequency information.
 * \param[in]	list - pointer to an ensemble list container
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintEnsembleList(EtalEnsembleList *list)
{
#ifdef CONFIG_TRACE_CLASS_ETAL
	tU32 i;

	etalTestPrintNormal("%sEnsemble list", PREFIX0);
	etalTestPrintNormal("%s", PREFIX0);
	for (i = 0; i < list->m_ensembleCount; i++)
	{
	etalTestPrintNormal("%sECC        : 0x%x", PREFIX1, list->m_ensemble[i].m_ECC);
	etalTestPrintNormal("%sEid        : 0x%x", PREFIX1, list->m_ensemble[i].m_ensembleId);
	etalTestPrintNormal("%sFrequency  : %d",   PREFIX1, list->m_ensemble[i].m_frequency);
	etalTestPrintNormal("%s", PREFIX1);
	}
#endif // CONFIG_TRACE_CLASS_ETAL
}

/***************************
 *
 * etalTestPrintEnsembleData
 *
 **************************/
/*!
 * \brief		Prints the detailed Ensemble information
 * \param[in]	eid - the Ensemble EId
 * \param[in]	charset - the charset used in the *label*
 * \param[in]	label - the Ensemble label
 * \param[in]	bitmap - the bitmap i.e. the sub-string to be printed for low-end receivers
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintEnsembleData(tU32 eid, tU8 charset, tChar *label, tU16 bitmap)
{
#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintNormal("%sEnsemble 0x%x data", PREFIX0, eid);
	etalTestPrintNormal("%sCharset    : %d", PREFIX1, charset);
	etalTestPrintNormal("%sLabel      : %s",   PREFIX1, label);
	etalTestPrintNormal("%sBitmap     : 0x%x", PREFIX1, bitmap);
	etalTestPrintNormal("%s", PREFIX1);
#endif // CONFIG_TRACE_CLASS_ETAL
}

/***************************
 *
 * etalTestPrintServiceList
 *
 **************************/
/*!
 * \brief		Prints the list of services for HD Radio or DAB
 * \param[in]	eid - the Ensemble Id to which the *list* belongs; set to any negative
 * 				      value for HD
 * \param[in]	list - pointer to service list container
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintServiceList(tS32 eid, EtalServiceList *list)
{
#ifdef CONFIG_TRACE_CLASS_ETAL
	tU32 i;

	if (eid < 0)
	{
		etalTestPrintNormal("%sService list for hd", PREFIX0);
	}
	else
	{
		etalTestPrintNormal("%sService list for ensemble 0x%x", PREFIX0, eid);
	}
	etalTestPrintNormal("%s", PREFIX0);
	for (i = 0; i < list->m_serviceCount; i++)
	{
	etalTestPrintNormal("%sSid        : 0x%x", PREFIX1, list->m_service[i]);
	}
	etalTestPrintNormal("%s", PREFIX1);
#endif // CONFIG_TRACE_CLASS_ETAL
}

/***************************
 *
 * etalTestPrintServiceDataDAB
 *
 **************************/
/*!
 * \brief		Prints detailed data for DAB service
 * \param[in]	sid - the DAB Service Id
 * \param[in]	serv - pointer to a Service Information container
 * \param[in]	sc - pointer to a Service Component Information container
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintServiceDataDAB(tU32 sid, EtalServiceInfo *serv, EtalServiceComponentList *sc)
{
#ifdef CONFIG_TRACE_CLASS_ETAL
	tU32 i;
	EtalSCInfo *sci;

	etalTestPrintNormal("%sService Data for service 0x%x", PREFIX0, sid);
	etalTestPrintNormal("%sCharset    : %d",   PREFIX1, serv->m_serviceLabelCharset);
	etalTestPrintNormal("%sLabel      : %s",   PREFIX1, serv->m_serviceLabel);
	etalTestPrintNormal("%sBitmap     : 0x%x", PREFIX1, serv->m_serviceLabelCharflag);
	etalTestPrintNormal("%sBitrate    : %dkbit/s",   PREFIX1, serv->m_serviceBitrate);
	etalTestPrintNormal("%sSubChId    : %d",   PREFIX1, serv->m_subchId);
	etalTestPrintNormal("%sPacketAddr : 0x%x", PREFIX1, serv->m_packetAddress);
	etalTestPrintNormal("%sLanguage   : %d",   PREFIX1, serv->m_serviceLanguage);
	etalTestPrintNormal("%sComp Type  : %d",   PREFIX1, serv->m_componentType);
	etalTestPrintNormal("%sComp Number: %d",   PREFIX1, serv->m_scCount);
	etalTestPrintNormal("%sStream Type: %d",   PREFIX1, serv->m_streamType);
	etalTestPrintNormal("%s", PREFIX1);
	etalTestPrintNormal("%sComponent List (%d elements)", PREFIX1, sc->m_scCount);
	for (i = 0; i < (tU32)sc->m_scCount; i++)
	{
	sci = &sc->m_scInfo[i];
	etalTestPrintNormal("%sIndex      : %d",   PREFIX2, sci->m_scIndex);
	etalTestPrintNormal("%sType       : %d",   PREFIX2, sci->m_scType);
	etalTestPrintNormal("%sCharset    : %d",   PREFIX2, sci->m_scLabelCharset);
	etalTestPrintNormal("%sLabel      : %s",   PREFIX2, sci->m_scLabel);
	etalTestPrintNormal("%sBitmap     : 0x%x", PREFIX2, sci->m_scLabelCharflag);
	etalTestPrintNormal("%sDataServ Ty: %d",   PREFIX2, sci->m_dataServiceType);
	etalTestPrintNormal("%s", PREFIX2);
	}
	etalTestPrintNormal("%s", PREFIX1);
#endif // CONFIG_TRACE_CLASS_ETAL
}

/***************************
 *
 * etalTestPrintVerboseContainer
 *
 **************************/
/*
 * Prints the content of a Quality container.
 * This function is a duplicate of the ETAL CORE ETAL_tracePrintQuality
 * The duplication is necessary because the two have different message filter requirements
 */
tVoid etalTestPrintVerboseContainer(EtalBcastQualityContainer *q)
{
#ifdef CONFIG_TRACE_CLASS_ETAL
	tChar prefix[PREFIX_LEN];

	OSAL_szStringCopy(prefix, PREFIX0);

	etalTestPrintVerbose("%sQuality Container for %s", prefix, etalTestStandard2Ascii(q->m_standard));
	etalTestPrintVerbose("%s", prefix);

	OSAL_szStringCopy(prefix, PREFIX1);
	switch (q->m_standard)
	{
		case ETAL_BCAST_STD_DAB:
			{
				EtalDabQualityEntries *qq = &(q->EtalQualityEntries.dab);
				etalTestPrintVerbose("%sTimestamp:                      %ld ms", prefix, q->m_TimeStamp);
				etalTestPrintVerbose("%sRFFieldStrength:                %ld dBm", prefix, qq->m_RFFieldStrength);
				etalTestPrintVerbose("%sBBFieldStrength:                %ld", prefix, qq->m_BBFieldStrength);
				etalTestPrintVerbose("%sFicBitErrorRatio:               %d (*10-6)", prefix, qq->m_FicBitErrorRatio);
				etalTestPrintVerbose("%sisValidFicBitErrorRatio:        %d", prefix, qq->m_isValidFicBitErrorRatio);
				etalTestPrintVerbose("%sMscBitErrorRatio:               %d (*10-6)", prefix, qq->m_MscBitErrorRatio);
				etalTestPrintVerbose("%sisValidMscBitErrorRatio:        %d", prefix, qq->m_isValidMscBitErrorRatio);
				etalTestPrintVerbose("%sDataSubChBitErrorRatio:         %d (*10-6)", prefix, qq->m_dataSubChBitErrorRatio);
				etalTestPrintVerbose("%sisValidDataSubChBitErrorRatio:  %d", prefix, qq->m_isValidDataSubChBitErrorRatio);
				etalTestPrintVerbose("%sAudioSubChBitErrorRatio:        %d (*10-6)", prefix, qq->m_audioSubChBitErrorRatio);
				etalTestPrintVerbose("%sisValidAudioSubChBitErrorRatio: %d", prefix, qq->m_isValidAudioSubChBitErrorRatio);
				etalTestPrintVerbose("%sAudioBitErrorRatioLevel:        %d (0=bad-9=good)", prefix, qq->m_audioBitErrorRatioLevel);
				etalTestPrintVerbose("%sReedSolomonInformation:         %d", prefix, qq->m_reedSolomonInformation);
				etalTestPrintVerbose("%sSyncStatus:                     %d", prefix, qq->m_syncStatus);
				etalTestPrintVerbose("%sMuteFlag:                       %d", prefix, qq->m_muteFlag);
			}
			break;

		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
			{
				EtalFmQualityEntries *qq = &(q->EtalQualityEntries.amfm);
			    etalTestPrintVerbose("%sTimestamp:           %ld ms", prefix, q->m_TimeStamp);
				etalTestPrintVerbose("%sRFFieldStrength:     %ld dBuV", prefix, qq->m_RFFieldStrength);
				etalTestPrintVerbose("%sBBFieldStrength:     %ld dBuV", prefix, qq->m_BBFieldStrength);
				etalTestPrintVerbose("%sFrequencyOffset:     %lu Hz", prefix, qq->m_FrequencyOffset);
				etalTestPrintVerbose("%sModulationDetector:  %lu Hz", prefix, qq->m_ModulationDetector);
				etalTestPrintVerbose("%sMultipath:           %lu (in percentage)", prefix, qq->m_Multipath);
				etalTestPrintVerbose("%sUltrasonicNoise:     %lu (in percentage)", prefix, qq->m_UltrasonicNoise);
				etalTestPrintVerbose("%sAdjacentChannel:     %ld (log ratio)", prefix, qq->m_AdjacentChannel);
				etalTestPrintVerbose("%sSNR:                 %lu (0=bad-255=good)", prefix, qq->m_SNR);
				etalTestPrintVerbose("%scoChannel:           %lu (in percentage)", prefix, qq->m_coChannel);
				etalTestPrintVerbose("%sStereoMonoReception: %ld", prefix, qq->m_StereoMonoReception);
			}
			break;

		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
			{
				EtalHdQualityEntries *qq = &(q->EtalQualityEntries.hd);
				EtalFmQualityEntries *qqfm = &(q->EtalQualityEntries.hd.m_analogQualityEntries);

                etalTestPrintVerbose("%sTimestamp:           %ld ms", prefix, q->m_TimeStamp);
                etalTestPrintVerbose("%sisValidDigital:      %d", prefix, qq->m_isValidDigital);
				etalTestPrintVerbose("%sQI:                  %d", prefix, qq->m_QI);
				etalTestPrintVerbose("%sCdToNo:              %d", prefix, qq->m_CdToNo);
				etalTestPrintVerbose("%sDSQM:                %.2f", prefix, qq->m_DSQM/ETAL_HDRADIO_DSQM_DIVISOR);
				etalTestPrintVerbose("%sAudioAlignment:      %d", prefix, qq->m_AudioAlignment);

				etalTestPrintVerbose("%sRFFieldStrength:     %ld dBuV", prefix, qqfm->m_RFFieldStrength);
				etalTestPrintVerbose("%sBBFieldStrength:     %ld dBuV", prefix, qqfm->m_BBFieldStrength);
				etalTestPrintVerbose("%sFrequencyOffset:     %lu Hz", prefix, qqfm->m_FrequencyOffset);
				etalTestPrintVerbose("%sModulationDetector:  %lu Hz", prefix, qqfm->m_ModulationDetector);
				etalTestPrintVerbose("%sMultipath:           %lu (in percentage)", prefix, qqfm->m_Multipath);
				etalTestPrintVerbose("%sUltrasonicNoise:     %lu (in percentage)", prefix, qqfm->m_UltrasonicNoise);
				etalTestPrintVerbose("%sAdjacentChannel:     %ld (log ratio)", prefix, qqfm->m_AdjacentChannel);
				etalTestPrintVerbose("%sSNR:                 %lu (0=bad-255=good)", prefix, qqfm->m_SNR);
				etalTestPrintVerbose("%scoChannel:           %lu (in percentage)", prefix, qqfm->m_coChannel);
				etalTestPrintVerbose("%sStereoMonoReception: %ld", prefix, qqfm->m_StereoMonoReception);

			}
			break;

		default:
			etalTestPrintVerbose("%sIllegal standard (%d)", prefix, q->m_standard);
			break;
	}
	etalTestPrintVerbose("%s", prefix);
#endif // CONFIG_TRACE_CLASS_ETAL
}


/***************************
 *
 * etalTestPrintInitStatus
 *
 **************************/
/*!
 * \brief		Prints the init status
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintInitStatus(tVoid)
{
#ifdef CONFIG_TRACE_CLASS_ETAL
	EtalInitStatus status;
	tU32 i;

	if (etal_get_init_status(&status) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("Unable to access init status");
		return;
	}

	etalTestPrintVerbose("%sm_lastInitState : %d", PREFIX0, status.m_lastInitState);
	etalTestPrintVerbose("%sm_warningStatus : %d", PREFIX0, status.m_warningStatus);
	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		etalTestPrintVerbose("%sTuner %d", PREFIX1, i);
		etalTestPrintVerbose("%sm_deviceStatus    : %d", PREFIX2, status.m_tunerStatus[i].m_deviceStatus);
		etalTestPrintVerbose("%sm_expectedSilicon : %s", PREFIX2, status.m_tunerStatus[i].m_expectedSilicon);
		etalTestPrintVerbose("%sm_detectedSilicon : %s", PREFIX2, status.m_tunerStatus[i].m_detectedSilicon);
	}
	etalTestPrintVerbose("%sDCOP", PREFIX1);
	etalTestPrintVerbose("%sm_deviceStatus    : %d", PREFIX2, status.m_DCOPStatus.m_deviceStatus);
	etalTestPrintVerbose("%sm_expectedSilicon : %s", PREFIX2, status.m_DCOPStatus.m_expectedSilicon);
	etalTestPrintVerbose("%sm_detectedSilicon : %s", PREFIX2, status.m_DCOPStatus.m_detectedSilicon);
#endif // CONFIG_TRACE_CLASS_ETAL
}

/***************************
 *
 * etalTestStandard2String
 *
 **************************/
tCString etalTestStandard2String(etalTestBroadcastTy std)
{
	switch (std)
	{
		case ETAL_TEST_MODE_FM:
			return "FM:";
		case ETAL_TEST_MODE_AM:
			return "AM:";
		case ETAL_TEST_MODE_DAB:
			return "DAB:";
		case ETAL_TEST_MODE_HD_FM:
			return "HDFM:";
		case ETAL_TEST_MODE_HD_AM:
			return "HDAM:";
		case ETAL_TEST_MODE_NONE:
			return "NONE:";
	}	
	return "UNDEFINED";
}

/***************************
 *
 * etalTestResult2String
 *
 **************************/
static tCString etalTestResult2String(etalTestResultTy res)
{
	switch (res)
	{
		case testPassed:
			return "OK";
		case testFailed:
			return "FAILED";
		case testSkipped:
			return "SKIP";
	}
	return "UNDEF";
}

/***************************
 *
 * etalTestPrintReportTestStart
 *
 **************************/
/*!
 * \brief		Mark the start of a test for ETAL_TEST_VERBOSE_REPORT and lower
 * \details		A test is normally composed of several passes.
 * 				This function must be called at the start of a test
 * 				to prepare internal state variables used by the
 * 				print facilities and to output the "Test start" string.
 * 				At the end of the test call #etalTestPrintReportTestEnd.
 * \remark		Do not call etalTestPrintReportTestStart between
 * 				etalTestPrintReportTestStart and etalTestPrintReportTestEnd:
 * 				these function do not support nesting function calls.
 * \param[in]	coszFormat - the string containing the test name
 * \param[in]	test_index - the test index (as found in the #etalTestControlTy array)
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintReportTestStart(tCString coszFormat, tU32 test_index)
{
	if (etalTestReportString[0] != '\0')
	{
		/* protect against start without end */
		ASSERT_ON_DEBUGGING(0);
	}
	etalTestReportPassIndex = 1;
	etalTestReportTestIndex = test_index;
	etalTestReportString[0] = '\0';

	etalTestPrint(ETAL_TEST_REPORT_LEVEL, "%02d: %s Test start", test_index, coszFormat);
}

/***************************
 *
 * etalTestPrintReportTestEnd
 *
 **************************/
/*!
 * \brief		Mark the end of a test for ETAL_TEST_VERBOSE_REPORT and lower
 * \details		Ouptuts the test result in predefined format.
 * \remark		Must be preceded by one and only one #etalTestPrintReportTestStart
 * \param[in]	coszFormat - the same string passed to #etalTestPrintReportTestStart
 * \param[in]	test_index - the same index passed to #etalTestPrintReportTestStart
 * \param[in]	retosal - the test result
 * \see			etalTestPrintReportTestStart
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintReportTestEnd(tCString coszFormat, tU32 test_index, tSInt retosal)
{
	etalTestResultTy res;
	tU32 pad;

	if (etalTestReportString[0] != '\0')
	{
		/* protect against start without end */
		ASSERT_ON_DEBUGGING(0);
	}

	if (retosal == OSAL_OK)
	{
		res = testPassed;
	}
	else
	{
		res = testFailed;
	}

	/* don't remove the ':' from the printed string, useful for external parsing */
	OSAL_s32NPrintFormat(etalTestReportString, ETAL_TEST_REPORT_STRING_LEN, "%02d: %s Test complete:", test_index, coszFormat);
	pad = ETAL_TEST_REPORT_RES_COL - OSAL_u32StringLength(etalTestReportString);

	etalTestPrint(ETAL_TEST_REPORT_LEVEL, "%s%*s", etalTestReportString, pad, etalTestResult2String(res));
	etalTestReportString[0] = '\0';
}

/***************************
 *
 * etalTestPrintReportPassStart
 *
 **************************/
/*!
 * \brief		Mark the start of a pass for ETAL_TEST_VERBOSE_REPORT and lower
 * \details		A test is normally composed of several passes.
 * 				This function must be called at the start of a test pass.
 * 				It prepares a string composed as follows:
 * 				"test index.test pass:standard:coszFormat"
 * 				where:
 * 				test index = the test_index passed to #etalTestPrintReportTestStart
 * 				test pass = a counter reset by the #etalTestPrintReportTestStart and
 * 				            incremented for each call to this function
 * 				standard = the Broadcast Standard
 * 				coszFormat = the printf-style string passed as parameter to this function
 * \remark		
 * \param[in]	std - the Broadcast Standard for this pass
 * \param[in]	coszFormat - pass description
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintReportPassStart(tU8 subTestNb, etalTestBroadcastTy std, tCString coszFormat, ...)
{
	tU32 header_len;
	va_list argList;

	if (etalTestReportString[0] != '\0')
	{
		/* protect against start without end */
		ASSERT_ON_DEBUGGING(0);
	}
	/* don't remove the ':' from the printed string, useful for external parsing */
	OSAL_s32NPrintFormat(etalTestReportString, ETAL_TEST_REPORT_STRING_LEN, "Pass %d Test %02d.%02d: %.5s ", etalTestReportPassIndex, etalTestReportTestIndex, subTestNb, etalTestStandard2String(std));
	header_len = OSAL_u32StringLength(etalTestReportString);

	OSAL_VarArgStart(argList, coszFormat);
	OSAL_s32VarNPrintFormat(etalTestReportString + header_len, ETAL_TEST_REPORT_STRING_LEN - header_len, coszFormat, argList);
	OSAL_VarArgEnd(argList);
}


/***************************
 *
 * etalTestPrintReportPassEnd
 *
 **************************/
/*!
 * \brief		Mark the end of a pass for ETAL_TEST_VERBOSE_REPORT and lower
 * \details		Outputs the string prepared by #etalTestPrintReportPassStart
 * 				adding the test pass result.
 * \param[in]	res - the test pass result
 * \callgraph
 * \callergraph
 */
tVoid etalTestPrintReportPassEnd(etalTestResultTy res)
{
	tU32 desc_len;
	tU32 pad;

	desc_len = OSAL_u32StringLength(etalTestReportString);
	pad = ETAL_TEST_REPORT_RES_COL - desc_len;
	/* add the ':' at the end of the pass description */
	pad = pad - 1;
	/* don't remove the ':' from the printed string, useful for external parsing */
	OSAL_s32NPrintFormat(etalTestReportString + desc_len, ETAL_TEST_REPORT_STRING_LEN - desc_len, ":%*s", pad, etalTestResult2String(res));

	etalTestPrint(ETAL_TEST_REPORT_LEVEL, etalTestReportString);
	etalTestReportString[0] = '\0';

	etalTestReportPassIndex++;
}

/***************************
 *
 * etalTestPrint
 *
 **************************/
tVoid etalTestPrint(tU16 level, tCString coszFormat, ...)
{
	static tChar szBuffer[OSAL_C_U32_TRACE_MAX_MESSAGESIZE];
	va_list argList;

	/* filter the test application messages based on run-time option (-q <num>) */

	switch (etalTestOption.oQuietLevel)
	{
		case ETAL_TEST_VERBOSE_ALL:
			break;

		case ETAL_TEST_VERBOSE_NORMAL:
			if (level == ETAL_TEST_VERBOSE_LEVEL)
			{
				return;
			}
			break;

		case ETAL_TEST_VERBOSE_REPORT_ERROR:
			if ((level != ETAL_TEST_REPORT_LEVEL) && (level != ETAL_TEST_REPORT0_LEVEL) && (level != ETAL_TEST_ERROR_LEVEL))
			{
				return;
			}
			break;

		case ETAL_TEST_VERBOSE_REPORT:
			if ((level != ETAL_TEST_REPORT_LEVEL) && (level != ETAL_TEST_REPORT0_LEVEL))
			{
				return;
			}
			break;

		case ETAL_TEST_VERBOSE_REPORT0:
			if (level != ETAL_TEST_REPORT0_LEVEL)
			{
				return;
			}
			break;

		case ETAL_TEST_VERBOSE_NONE:
			return;

		default:
			 /* illegal value should be catched by etalTestProcessParams,
			  * nothing to do here */
			break;
	}

	/*
	 * test application messages that arrive to this point must be
	 * output with ETAL_TEST_OUTPUT_LEVEL (normally TR_LEVEL_SYSTEM_MIN)
	 */
	level = ETAL_TEST_OUTPUT_LEVEL;

	OSAL_VarArgStart(argList, coszFormat);
	OSAL_s32VarNPrintFormat(szBuffer, sizeof(szBuffer), coszFormat, argList);
	OSAL_VarArgEnd(argList);

#ifdef CONFIG_TRACE_ENABLE
	/* if ETAL was initialized use the ETAL facilities for
	 * printing, otherwise map to plain printf */
	if (etalTestInitializeDone)
	{
		OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL_TEST, szBuffer);
		return;
	}
#endif
	/* 
	 * ETAL trace function not available either because not configured
	 * or becuse ETAL not yet started, ETAL quality and status
	 * will not be printed but print at least the test output
	 *
	 * mimik the print header for ETALTEST
	 */
	if (etalTestOption.oDisableTraceHeader)
	{
		printf("%s\n", szBuffer);
	}
	else
	{
		printf("xxxxxxxxxxx:SYSMIN:ETALTEST: %s\n", szBuffer);
	}
}

#endif // CONFIG_APP_ETAL_TEST
