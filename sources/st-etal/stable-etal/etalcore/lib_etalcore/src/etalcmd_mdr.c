//!
//!  \file 		etalcmd_mdr.c
//!  \brief 	<i><b> ETAL command protocol layer for MDR devices </b></i>
//!  \details   MDR-specific command implementation
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR


/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
#define ETAL_DABMW_GET_SYNC_STATUS(_d_)					(tU8) (((((_d_)[37] >> 4) & 0x03)))

#define ETAL_DABMW_SETMONITORFILTER_PAYLOAD   4
#define ETAL_DABMW_SETMONITORFILTER_MODE      (ETAL_DABMW_SETMONITORFILTER_PAYLOAD + 0)
#define ETAL_DABMW_SETMONITORFILTER_FILTERID  (ETAL_DABMW_SETMONITORFILTER_PAYLOAD + 1)
#define ETAL_DABMW_SETMONITORFILTER_PAROFFSET (ETAL_DABMW_SETMONITORFILTER_PAYLOAD + 2)
#define ETAL_DABMW_SETMONITORFILTER_PARSIZE   (ETAL_DABMW_SETMONITORFILTER_PAYLOAD + 4)
#define ETAL_DABMW_SETMONITORFILTER_THMIN     (ETAL_DABMW_SETMONITORFILTER_PAYLOAD + 5)
#define ETAL_DABMW_SETMONITORFILTER_THMAX     (ETAL_DABMW_SETMONITORFILTER_PAYLOAD + 9)
#define ETAL_DABMW_SETMONITORFILTER_SKIPCOUNT (ETAL_DABMW_SETMONITORFILTER_PAYLOAD + 13)

/* Get Ensemble List */
#define ETAL_DABMW_GET_ENSLIST_RECORD_SIZE     7
#define ETAL_DABMW_GET_ENSLIST_ECC(_d_)                ((_d_)[0] & 0xFF) // offset from start of record
#define ETAL_DABMW_GET_ENSLIST_EID(_d_)              ((((tU16)(_d_)[1] << 8) & 0xFF00) | ((tU16)(_d_)[2] & 0x00FF))
#define ETAL_DABMW_GET_ENSLIST_FREQ(_d_)             ((((tU32)(_d_)[3] << 24) & 0xFF000000) | (((tU32)(_d_)[4] << 16) & 0x00FF0000) | (((tU32)(_d_)[5] << 8) & 0x0000FF00) | ((tU32)(_d_)[6] & 0x000000FF))

/* Get Ensemble Data */
#define ETAL_DABMW_GET_ENSDATA_RESP_SIZE      26
#define ETAL_DABMW_GET_ENSDATA_CHARSET(_d_)            ((_d_)[3] & 0xFF)
#define ETAL_DABMW_GET_ENSDATA_LABELSTART(_d_)         ((_d_) + 4)
#define ETAL_DABMW_GET_ENSDATA_CHARFLAG(_d_)         ((((tU16)(_d_)[20] << 8) & 0xFF00) | ((_d_)[21] & 0xFF))

/* Get Service List */
#define ETAL_DABMW_GET_SERVLIST_RECORD_SIZE    4
#define ETAL_DABMW_GET_SERVLIST_SID(_d_)            ((((tU32)(_d_)[0] << 24) & 0xFF000000) | (((tU32)(_d_)[1] << 16) & 0x00FF0000) | (((tU32)(_d_)[2] << 8) & 0x0000FF00) | ((tU32)(_d_)[3] & 0x000000FF))

/* Get Specific Service Data */
#define ETAL_DABMW_GET_SERVICEDATA_BITRATE(_d_)     ((((tU16)(_d_)[7] << 8) & 0xFF00) | ((_d_)[8] & 0x00FF))
#define ETAL_DABMW_GET_SERVICEDATA_SUBCHID(_d_)       ((_d_)[9] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_PACKETADDR(_d_)  ((((tU16)(_d_)[10] << 8) & 0xFF00)| ((_d_)[11] & 0x00FF))
#define ETAL_DABMW_GET_SERVICEDATA_SERVLANG(_d_)      ((_d_)[12] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_COMPTYPE(_d_)      ((_d_)[13] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_STREAMTYPE(_d_)    ((_d_)[14] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_SCCOUNT(_d_) (tU32)((_d_)[15] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_SERVLABCHARSET(_d_)((_d_)[16] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_SERVLABSTART(_d_)   (_d_ + 17)
#define ETAL_DABMW_GET_SERVICEDATA_SERVLABCHARFLAG(_d_)((((tU16)(_d_)[33] << 8) & 0xFF00) | ((_d_)[34] & 0x00FF))
#define ETAL_DABMW_GET_SERVICEDATA_RECORD_SIZE     35

#define ETAL_DABMW_GET_SERVICEDATA_SC_RECORD_SIZE  22
#define ETAL_DABMW_GET_SERVICEDATA_SCINDEX(_d_)       ((_d_)[0] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_SCDTTYPE(_d_)      ((_d_)[1] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_SCTYPE(_d_)        ((_d_)[2] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_SCLABCHARSET(_d_)  ((_d_)[3] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATA_SCLABSTART(_d_)    ((_d_) + 4)
#define ETAL_DABMW_GET_SERVICEDATA_SCLABCHARFLAG(_d_)((((tU16)(_d_)[20] << 8) & 0xFF00) | ((_d_)[21] & 0x00FF))

#define ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE  36
#define ETAL_DABMW_GET_SERVICEDATAEXT_SCINDEX(_d_)          ((_d_)[0] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATAEXT_SCNUMBER(_d_)         ((((tU16)(_d_)[1] << 8) & 0xFF00) | ((_d_)[2] & 0x00FF))
#define ETAL_DABMW_GET_SERVICEDATAEXT_SCIDS(_d_)            ((_d_)[3] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATAEXT_SUBCHNUM(_d_)         ((_d_)[4] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATAEXT_PACKADDR(_d_)         ((((tU16)(_d_)[5] << 8) & 0xFF00) | ((_d_)[6] & 0x00FF))
#define ETAL_DABMW_GET_SERVICEDATAEXT_USERAPPDATATYPE(_d_,_i_)  ((((tU16)(_d_)[7 + (_i_ * 2)] << 8) & 0xFF00) | ((_d_)[8 + (_i_ * 2)] & 0x00FF))
#define ETAL_DABMW_GET_SERVICEDATAEXT_SCDTTYPE(_d_)         ((_d_)[15] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATAEXT_SCTYPE(_d_)           ((_d_)[16] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATAEXT_SCLABCHARSET(_d_)     ((_d_)[17] & 0xFF)
#define ETAL_DABMW_GET_SERVICEDATAEXT_SCLABSTART(_d_)       ((_d_) + 18)
#define ETAL_DABMW_GET_SERVICEDATAEXT_SCLABCHARFLAG(_d_)    ((((tU16)(_d_)[34] << 8) & 0xFF00) | ((_d_)[35] & 0x00FF))

/* Get Current Ensemble */
#define ETAL_DABMW_GET_ENSEMBLE_PAYLOAD        4
#define ETAL_DABMW_GET_ENSEMBLE_ENSEMBLEID(_d_)     ((((tU32)(_d_)[1] & 0xFF) << 16) | (((tU32)(_d_)[2] & 0xFF) << 8) | ((_d_)[3] & 0xFF))

#define ETAL_DABMW_SERVICE_SELECT_ROUTING_DATA        (tU8)(1 << 6)
#define ETAL_DABMW_SERVICE_SELECT_ROUTING_AUDIO       (tU8)(1 << 7)

/* Get Firmware Version */
#define ETAL_DABMW_GET_VERSION_PAYLOAD                13
#define ETAL_DABMW_BYTE2BCD(_d_)                     (((((tU32)(_d_) & 0xF0) >> 4) * 10) + ((tU32)(_d_) & (0x0F)))
#define ETAL_DABMW_GET_VERSION_YEAR(_d_)              ((ETAL_DABMW_BYTE2BCD((_d_)[0]) * 100) + ETAL_DABMW_BYTE2BCD((_d_)[1])) 
#define ETAL_DABMW_GET_VERSION_MONTH(_d_)              (ETAL_DABMW_BYTE2BCD((_d_)[2]))
#define ETAL_DABMW_GET_VERSION_DAY(_d_)                (ETAL_DABMW_BYTE2BCD((_d_)[3]))
#define ETAL_DABMW_GET_VERSION_MAJOR(_d_)              ((tU32)(_d_)[4])
#define ETAL_DABMW_GET_VERSION_MINOR(_d_)              ((tU32)(_d_)[5])
#define ETAL_DABMW_GET_VERSION_INTERNAL(_d_)           ((tU32)(_d_)[6])
#define ETAL_DABMW_GET_VERSION_BUILD(_d_)             (((tU32)(_d_)[7] << 8) | (tU32)(_d_)[8])

/* Get Time */
#define ETAL_DABMW_GET_TIME_WITH_LTO_PAYLOAD          10
#define ETAL_DABMW_GET_TIME_WITHOUT_LTO_PAYLOAD       9

#define DABMW_STATUS_DUPLICATED_AUTO_COMMAND          0x0C


// General useful macro
#define ETAL_DABMW_EXTRACT_U32(_b_) ((((tU32)(_b_)[0] << 24) & 0xFF000000) | (((tU32)(_b_)[1] << 16) & 0x00FF0000) | (((tU32)(_b_)[2] << 8) & 0x0000FF00) | (((tU32)(_b_)[3] << 0) & 0x000000FF))
#define ETAL_DABMW_EXTRACT_U16(_b_) ((((tU16)(_b_)[0] << 8) & 0xFF00) | (((_b_)[1] << 0) & 0x00FF))
#define ETAL_DABMW_EXTRACT_U8(_b_)  (((_b_)[0] << 0) & 0xFF)


/*****************************************************************
| Local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*****************************************************************
| prototypes
|----------------------------------------------------------------*/
static tU32 ETAL_addFilter_MDR(void);
static tVoid ETAL_extractDabAudioQualityFromNotification_MDR(EtalDabQualityEntries *d, tU8 *payload);

/***************************
 *
 * ETAL_getGenericPayloadLen_MDR
 *
 **************************/
static tU32 ETAL_getGenericPayloadLen_MDR(tU8 *buf)
{
	if (ETAL_DABMW_HEADER_HAS_LONG_PAYLOAD(buf))
	{
		return (((tU32)buf[3] << 8) & 0xFF00) | (buf[4] & 0xFF);
	}
	else
	{
		return (tU32)buf[3] & 0xFF;
	}
}

/***************************
 *
 * ETAL_getGenericPayload_MDR
 *
 **************************/
static tU8 *ETAL_getGenericPayload_MDR(tU8 *buf)
{
	if (ETAL_DABMW_HEADER_HAS_LONG_PAYLOAD(buf))
	{
		return buf + 5;
	}
	else
	{
		return buf + 4;
	}
}

/***************************
 *
 * ETAL_paramSetApplicationFromReceiver_MDR
 *
 **************************/
static tVoid ETAL_paramSetApplicationFromReceiver_MDR(tU8 *cmd, ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp = NULL;
	DABMW_mwAppTy app;

	recvp = ETAL_receiverGet(hReceiver);

	if((recvp != NULL) && (cmd != NULL))
	{
		app = recvp->MDRConfig.application;
		cmd[1] |= app << 2;
	}
	else
	{
		ASSERT_ON_DEBUGGING(0);
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_paramSetApplicationFromReceiver_MDR recvp: 0x%x, cmd: 0x%x", recvp, cmd);
	}
}


#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_paramSetApplicationFromMonitor_MDR
 *
 **************************/
static tVoid ETAL_paramSetApplicationFromMonitor_MDR(tU8 *cmd, ETAL_HANDLE hMonitor)
{
	ETAL_HANDLE hReceiver;
	etalMonitorTy *pmon = NULL;

	pmon = ETAL_statusGetMonitor(hMonitor);
	hReceiver = ETAL_statusGetReceiverFromMonitor(pmon);
	ETAL_paramSetApplicationFromReceiver_MDR(cmd, hReceiver);
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_paramSetLearnBand_MDR
 *
 **************************/
static tVoid ETAL_paramSetLearnBand_MDR(tU8 *cmd, EtalFrequencyBand bandIndex)
{
	if (bandIndex == ETAL_BAND_DAB3)
	{
		cmd[1] |= 0x40;
	}
	if (bandIndex == ETAL_BAND_DABL)
	{
		cmd[1] |= 0x80;
	}
}

/***************************
 *
 * ETAL_paramSetEID_MDR
 *
 **************************/
static tVoid ETAL_paramSetEID_MDR(tU8 *cmd, tU32 eid)
{
	cmd[4] = (tU8)((eid >> 16) & 0xFF);
	cmd[5] = (tU8)((eid >>  8) & 0xFF);
	cmd[6] = (tU8)((eid >>  0) & 0xFF);
}

/***************************
 *
 * ETAL_setSid_MDR
 *
 **************************/
/*
 * Sets the sid at the locations starting
 * from <cmd>
 *
 * WARNING: <cmd> usage is different than
 * other commands!
 */
static tVoid ETAL_setSid_MDR(tU8 *cmd, tU32 sid)
{
	cmd[0] = (tU8)((sid >> 24) & 0xFF);
	cmd[1] = (tU8)((sid >> 16) & 0xFF);
	cmd[2] = (tU8)((sid >>  8) & 0xFF);
	cmd[3] = (tU8)((sid >>  0) & 0xFF);
}


/***************************
 *
 * ETAL_paramSetFrequency_MDR
 *
 **************************/
static tVoid ETAL_paramSetFrequency_MDR(tU8 *cmd, tU32 f)
{
	cmd[4] = (tU8)((f & 0xFF000000) >> 24);
	cmd[5] = (tU8)((f & 0x00FF0000) >> 16);
	cmd[6] = (tU8)((f & 0x0000FF00) >>  8);
	cmd[7] = (tU8)((f & 0x000000FF) >>  0);
}

/***************************
 *
 * ETAL_paramSetCountry_MDR
 *
 **************************/
static tVoid ETAL_paramSetCountry_MDR(tU8 *cmd, tU32 c)
{
	cmd[4] = (tU8)(c & 0xFF);
}

/***************************
 *
 * ETAL_paramSetNvmFlags_MDR
 *
 **************************/
static tVoid ETAL_paramSetNvmFlags_MDR(tU8 *cmd, tU8 c)
{
	cmd[1] = (tU8)(c & 0x0C);
}

/***************************
 *
 * ETAL_paramSetSaveNvm_MDR
 *
 **************************/
static tVoid ETAL_paramSetSaveNvm_MDR(tU8 *cmd, tU8 c)
{
	cmd[1] = (tU8)(c & 0x04);
}

/***************************
 *
 * ETAL_paramSetClearDatabase_MDR
 *
 **************************/
static tVoid ETAL_paramSetClearDatabase_MDR(tU8 *cmd, tU8 c)
{
	cmd[1] = (tU8)(c & 0x84);
}

/***************************
 *
 * ETAL_commandStatusToEventErrStatus_MDR
 *
 **************************/
static EtalCommErr ETAL_commandStatusToEventErrStatus_MDR(tU16 cstatus)
{
	switch (cstatus)
	{
		case DABMW_CMD_STATUS_ERR_TIMEOUT:
			return EtalCommStatus_TimeoutError;

		case DABMW_CMD_STATUS_ERR_HEADER0_FORMAT:
		case DABMW_CMD_STATUS_ERR_WRONG_STOP_BIT:
			return EtalCommStatus_ProtocolHeaderError;

		case DABMW_CMD_STATUS_ERR_PARAM_WRONG:
		case DABMW_CMD_STATUS_ERR_ILLEGAL_CMDNUM:
		case DABMW_CMD_STATUS_ERR_RESERVED_CMDNUM:
		case DABMW_CMD_STATUS_ERR_PAYLOAD_TOO_LONG:
		case DABMW_CMD_STATUS_ERR_WRONG_CMD_LENGHT:
			return EtalCommStatus_MessageFormatError;

		case DABMW_CMD_STATUS_ERR_CHECKSUM:
			return EtalCommStatus_ChecksumError;

		default:
			return EtalCommStatus_GenericError;
	}
}

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_translateParameters_MDR
 *
 **************************/
static tSInt ETAL_translateParameters_MDR(etalMonitorTy *monp, tU8 index, tU16 *offp, tU8 *lenp, tU32 *thminp, tU32 *thmaxp, tU16 *skipp)
{
	EtalQaMonitoredEntryAttr *reqp = NULL;
	tSInt ret = OSAL_OK;

	if((monp != NULL) && (offp != NULL) && (lenp != NULL) && (thminp != NULL) && (thmaxp != NULL) && (skipp!= NULL))
	{
		reqp = &monp->requested.m_monitoredIndicators[index];

		if(reqp != NULL)
		{
			switch (reqp->m_MonitoredIndicator)
			{
				case EtalQualityIndicator_DabFicErrorRatio:
					*offp = 13;
					*lenp = (tU8)4;
					break;
				case EtalQualityIndicator_DabFieldStrength:
					// Indicator monitored by the CMOST
					*offp = 0;
					*lenp = (tU8)0;
					break;
				case EtalQualityIndicator_DabMscBer:
					*offp = 25;
					*lenp = (tU8)4;
					break;
				case EtalQualityIndicator_FmFieldStrength:
					*offp = 7;
					*lenp = (tU8)3;
					break;
				case EtalQualityIndicator_FmFrequencyOffset:
					*offp = 19;
					*lenp = (tU8)3;
					break;
				case EtalQualityIndicator_FmModulationDetector:
					*offp = 22;
					*lenp = (tU8)3;
					break;
				case EtalQualityIndicator_FmMultipath:
					*offp = 13;
					*lenp = (tU8)3;
					break;
				case EtalQualityIndicator_FmUltrasonicNoise:
					*offp = 10;
					*lenp = (tU8)3;
					break;
				default:
					return OSAL_ERROR;
			}
			if (reqp->m_InferiorValue == ETAL_INVALID_MONITOR)
			{
				*thminp = 0xFFFFFFFF;
			}
			else
			{
				// TODO truncating the requested value; what about negatives?
				*thminp = (tU32)reqp->m_InferiorValue;
			}
			if (reqp->m_SuperiorValue == ETAL_INVALID_MONITOR)
			{
				*thmaxp = 0xFFFFFFFF;
			}
			else
			{
				// TODO truncating the requested value; what about negatives?
				*thmaxp = (tU32)reqp->m_SuperiorValue;
			}
			// TODO truncating the requested value
			*skipp = (tU16)reqp->m_UpdateFrequency;
		}
		else
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_translateParameters_MDR reqp: 0x%x", reqp);
			ret = OSAL_ERROR;
			ASSERT_ON_DEBUGGING(0);
		}
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_translateParameters_MDR monp: 0x%x, offp: 0x%x, lenp: 0x%x, thminp: 0x%x, thmaxp: 0x%x, skipp: 0x%x",
				monp, offp, lenp, thminp, thmaxp, skipp);
		ret = OSAL_ERROR;
		ASSERT_ON_DEBUGGING(0);
	}
	return ret;
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_extractDabQualityFromNotification_MDR
 *
 **************************/
tVoid ETAL_extractDabQualityFromNotification_MDR(EtalDabQualityEntries *d, tU8 *payload)
{
	/* m_RFFieldStrength is provided by the CMOST */
	d->m_BBFieldStrength                 = ETAL_DABMW_GET_DABMON_BBLEVEL(payload);
	d->m_FicBitErrorRatio                = ETAL_DABMW_GET_DABMON_FICBER(payload);
	d->m_isValidFicBitErrorRatio         = true;
	d->m_audioSubChBitErrorRatio         = ETAL_DABMW_GET_DABMON_AUDIOSUBCHBER(payload);
	d->m_isValidAudioSubChBitErrorRatio  = true;
	d->m_dataSubChBitErrorRatio          = ETAL_DABMW_GET_DABMON_DATASUBCHBER(payload);
	d->m_isValidDataSubChBitErrorRatio   = true;
	d->m_MscBitErrorRatio                = ETAL_DABMW_GET_DABMON_MSCBER(payload);
	d->m_isValidMscBitErrorRatio         = true;
	d->m_syncStatus                      = ETAL_DABMW_GET_SYNC_STATUS(payload);

	/* fic_ber may not be available if no service was selected, in this case the DCOP sends all 1 */
	if (d->m_FicBitErrorRatio == (tU32)0xFFFFFFFF)
	{
		d->m_FicBitErrorRatio = ETAL_VALUE_NOT_AVAILABLE(tU32);
		d->m_isValidFicBitErrorRatio = false;
	}

	/* msc_ber may not be available if no service was selected, in this case the DCOP sends all 1 */
	if (d->m_MscBitErrorRatio == (tU32)0xFFFFFFFF)
	{
		d->m_MscBitErrorRatio = ETAL_VALUE_NOT_AVAILABLE(tU32);
		d->m_isValidMscBitErrorRatio = false;
	}

	/* data_ber may not be available if no service was selected, in this case the DCOP sends all 1 */
	if (d->m_dataSubChBitErrorRatio == (tU32)0xFFFFFFFF)
	{
		d->m_dataSubChBitErrorRatio = ETAL_VALUE_NOT_AVAILABLE(tU32);
		d->m_isValidDataSubChBitErrorRatio = false;
	}

	/* data_ber may not be available if no service was selected, in this case the DCOP sends all 1 */
	if (d->m_audioSubChBitErrorRatio == (tU32)0xFFFFFFFF)
	{
		d->m_audioSubChBitErrorRatio = ETAL_VALUE_NOT_AVAILABLE(tU32);
		d->m_isValidAudioSubChBitErrorRatio = false;
	}
}

/***************************
 *
 * ETAL_extractDabAudioQualityFromNotification_MDR
 *
 **************************/
static tVoid ETAL_extractDabAudioQualityFromNotification_MDR(EtalDabQualityEntries *d, tU8 *payload)
{
	tU32 vl_index = 0;

	// 1 byte is the app, skip it
	vl_index+=1;

	// audio_ber is on 4 bytes
	d->m_audioSubChBitErrorRatio = ETAL_DABMW_EXTRACT_U32((payload+vl_index));
	d->m_isValidAudioSubChBitErrorRatio = true;

	if (d->m_audioSubChBitErrorRatio == (tU32)0xFFFFFFFF)
	{
		d->m_audioSubChBitErrorRatio = ETAL_VALUE_NOT_AVAILABLE(tU32);
		d->m_isValidAudioSubChBitErrorRatio = false;
	}
	vl_index+=4;

	// audio ber level = 1 byte
	d->m_audioBitErrorRatioLevel = ETAL_DABMW_EXTRACT_U8((payload+vl_index));
	vl_index+=1;

	// reed_solomon_information= 1 byte
	d->m_reedSolomonInformation = ETAL_DABMW_EXTRACT_U8((payload+vl_index));
	vl_index+=1;

	// mute flag 1 byte
	d->m_muteFlag = (ETAL_DABMW_EXTRACT_U8((payload+vl_index)) != (tU8)0) ? TRUE : FALSE;
	vl_index+=1;

}

/***************************
 *
 * ETAL_parseServiceInfo
 *
 **************************/
static tVoid ETAL_parseServiceInfo(tU8 *resp, EtalServiceInfo *serv_info, EtalServiceComponentList *sclist)
{
	tU8 *payload, *scstart;
	tU32 list_size;
	EtalSCInfo *scinfo;
	tU32 i;

	payload = ETAL_getGenericPayload_MDR(resp);

	/*
	 * Parse Service Info
	 */
	if (serv_info)
	{
		serv_info->m_serviceBitrate       = ETAL_DABMW_GET_SERVICEDATA_BITRATE(payload);
		serv_info->m_subchId              = ETAL_DABMW_GET_SERVICEDATA_SUBCHID(payload);
		serv_info->m_packetAddress        = ETAL_DABMW_GET_SERVICEDATA_PACKETADDR(payload);
		serv_info->m_serviceLanguage      = ETAL_DABMW_GET_SERVICEDATA_SERVLANG(payload);
		serv_info->m_componentType        = ETAL_DABMW_GET_SERVICEDATA_COMPTYPE(payload);
		serv_info->m_streamType           = ETAL_DABMW_GET_SERVICEDATA_STREAMTYPE(payload);
		serv_info->m_serviceLabelCharset  = ETAL_DABMW_GET_SERVICEDATA_SERVLABCHARSET(payload);
		serv_info->m_serviceLabelCharflag = ETAL_DABMW_GET_SERVICEDATA_SERVLABCHARFLAG(payload);
		OSAL_pvMemoryCopy((tVoid *)serv_info->m_serviceLabel, (tPCVoid)ETAL_DABMW_GET_SERVICEDATA_SERVLABSTART(payload), ETAL_DEF_MAX_LABEL_LEN - 1);
		serv_info->m_serviceLabel[ETAL_DEF_MAX_LABEL_LEN - 1] = '\0';

		list_size = ETAL_DABMW_GET_SERVICEDATA_SCCOUNT(payload);
		if (list_size > ETAL_DEF_MAX_SC_PER_SERVICE)
		{
			/* truncate to supported size */
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "Unsupported number of service components per service (%d), max %d", list_size, ETAL_DEF_MAX_SC_PER_SERVICE);
			list_size = ETAL_DEF_MAX_SC_PER_SERVICE;
		}
		serv_info->m_scCount = (tU8)list_size;
	}

	/*
	 * Parse Service Component Info (an array)
	 */
	if (sclist)
	{
		scstart = payload + ETAL_DABMW_GET_SERVICEDATA_RECORD_SIZE;
		for (i = 0; i < list_size; i++)
		{
			scinfo = &sclist->m_scInfo[i];
			scinfo->m_scIndex         = ETAL_DABMW_GET_SERVICEDATA_SCINDEX(scstart + ETAL_DABMW_GET_SERVICEDATA_SC_RECORD_SIZE * i);
			scinfo->m_dataServiceType = ETAL_DABMW_GET_SERVICEDATA_SCDTTYPE(scstart + ETAL_DABMW_GET_SERVICEDATA_SC_RECORD_SIZE * i);
			scinfo->m_scType          = ETAL_DABMW_GET_SERVICEDATA_SCTYPE(scstart + ETAL_DABMW_GET_SERVICEDATA_SC_RECORD_SIZE * i);
			scinfo->m_scLabelCharset  = ETAL_DABMW_GET_SERVICEDATA_SCLABCHARSET(scstart + ETAL_DABMW_GET_SERVICEDATA_SC_RECORD_SIZE * i);
			scinfo->m_scLabelCharflag = ETAL_DABMW_GET_SERVICEDATA_SCLABCHARFLAG(scstart + ETAL_DABMW_GET_SERVICEDATA_SC_RECORD_SIZE * i);
			OSAL_pvMemoryCopy((tVoid *)scinfo->m_scLabel, (tPCVoid)ETAL_DABMW_GET_SERVICEDATA_SCLABSTART(scstart + ETAL_DABMW_GET_SERVICEDATA_SC_RECORD_SIZE * i), ETAL_DEF_MAX_LABEL_LEN - 1);
			scinfo->m_scLabel[ETAL_DEF_MAX_LABEL_LEN - 1] = '\0';
		}
		sclist->m_scCount = (tU8)list_size;
	}
}

/***************************
 *
 * ETAL_parseServiceExtendedInfo
 *
 **************************/
static tSInt ETAL_parseServiceExtendedInfo(tU8 *resp, tU32 resplen, EtalServiceInfo *serv_info, EtalServiceComponentExtendedList *sclist)
{
	tSInt ret = OSAL_OK;
	tU8 *payload, *scstart;
	tU32 list_size;
	EtalSCExtendedInfo *scinfo;
	tU32 i, j;

	if (resplen < ETAL_DABMW_GET_SERVICEDATA_RECORD_SIZE)
	{
		ret = OSAL_ERROR;
	}

	if (ret == OSAL_OK)
	{
		ETAL_parseServiceInfo(resp, serv_info, OSAL_NULL);

		payload = ETAL_getGenericPayload_MDR(resp);

		/*
		 * Parse Service Component Extended Info (an array)
		 */
		if (sclist)
		{
			list_size = ETAL_DABMW_GET_SERVICEDATA_SCCOUNT(payload);
			if (list_size > ETAL_DEF_MAX_SC_PER_SERVICE)
			{
				/* truncate to supported size */
				ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "Unsupported number of service components per service (%d), max %d", list_size, ETAL_DEF_MAX_SC_PER_SERVICE);
				list_size = ETAL_DEF_MAX_SC_PER_SERVICE;
			}
			(void)OSAL_pvMemorySet((tVoid *)sclist, 0x00, sizeof(scinfo));
			scstart = payload + ETAL_DABMW_GET_SERVICEDATA_RECORD_SIZE;
			for (i = 0; i < list_size; i++)
			{
				if (resplen < (ETAL_DABMW_GET_SERVICEDATA_RECORD_SIZE + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * (i + 1))))
				{
					ret = OSAL_ERROR;
					break;
				}
				if (ret == OSAL_OK)
				{
					scinfo = &sclist->m_scInfo[i];
					scinfo->m_scIndex         = ETAL_DABMW_GET_SERVICEDATAEXT_SCINDEX(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i));
					scinfo->m_scNumber        = ETAL_DABMW_GET_SERVICEDATAEXT_SCNUMBER(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i));
					scinfo->m_scids           = ETAL_DABMW_GET_SERVICEDATAEXT_SCIDS(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i));
					scinfo->m_subchId         = ETAL_DABMW_GET_SERVICEDATAEXT_SUBCHNUM(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i));
					scinfo->m_packetAddress   = ETAL_DABMW_GET_SERVICEDATAEXT_PACKADDR(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i));
					for(j = 0; j < ETAL_DEF_MAX_USER_APPLICATION_DATA_TYPE; j++)
					{
						scinfo->m_userApplicationDataType[j] = ETAL_DABMW_GET_SERVICEDATAEXT_USERAPPDATATYPE(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i), j);
					}
					scinfo->m_dataServiceType = ETAL_DABMW_GET_SERVICEDATAEXT_SCDTTYPE(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i));
					scinfo->m_scType          = ETAL_DABMW_GET_SERVICEDATAEXT_SCTYPE(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i));
					scinfo->m_scLabelCharset  = ETAL_DABMW_GET_SERVICEDATAEXT_SCLABCHARSET(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i));
					scinfo->m_scLabelCharflag = ETAL_DABMW_GET_SERVICEDATAEXT_SCLABCHARFLAG(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i));
					OSAL_pvMemoryCopy((tVoid *)scinfo->m_scLabel, (tPCVoid)ETAL_DABMW_GET_SERVICEDATAEXT_SCLABSTART(scstart + (ETAL_DABMW_GET_SERVICEDATAEXT_SC_RECORD_SIZE * i)), ETAL_DEF_MAX_LABEL_LEN - 1);
					scinfo->m_scLabel[ETAL_DEF_MAX_LABEL_LEN - 1] = '\0';
					sclist->m_scCount++;
				}
			}
		}
	}

	return ret;
}

/***************************
 *
 * ETAL_parseCurrentEnsemble
 *
 **************************/
static tVoid ETAL_parseCurrentEnsemble(tU8 *resp, tU32 *ueid)
{
	tU8 *payload;

	payload = ETAL_getGenericPayload_MDR(resp);
	*ueid = ETAL_DABMW_GET_ENSEMBLE_ENSEMBLEID(payload);
}

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_filterMaskFromIndex_MDR
 *
 **************************/
static etalFilterMaskTy ETAL_filterMaskFromIndex_MDR(tU32 index)
{
	return (etalFilterMaskTy) (1 << index);
}

/***************************
 *
 * ETAL_countIndicatorsPerMonitor
 *
 **************************/
static tU32 ETAL_countIndicatorsPerMonitor(const EtalBcastQualityMonitorAttr *mon)
{
	tU32 i;
	tU32 count = 0;

	for (i = 0; i < ETAL_MAX_QUALITY_PER_MONITOR; i++)
	{
		if (mon->m_monitoredIndicators[i].m_MonitoredIndicator != EtalQualityIndicator_Undef)
		{
			count++;
		}
	}
	return count;
}

/***************************
 *
 * ETAL_cmdMonitorAddFilters_MDR
 *
 **************************/
 /*
  * Parse the monitor and create the filters in the global filter array.
  */
static tSInt ETAL_cmdMonitorAddFilters_MDR(ETAL_HANDLE hMonitor)
{
	tU16 cstatus = 0;
	tU8 DABMWcmd_SetMonitorFilter[] = {0x96, 0x00, 0x07, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	etalDCOPFilterTy *flt = NULL;
	etalMonitorTy *mon = NULL;
	tU32 filter_index;
	tU16 par_off;
	tU8 par_size;
	tU32 th_min, th_max;
	tU16 skip;
	tU32 i, total_filters, total_filters_for_dcop = 0;
	tSInt retval = OSAL_OK;

	mon = ETAL_statusGetMonitor(hMonitor);
	total_filters = ETAL_statusCountIndicatorsForMonitor(mon);
	ETAL_paramSetApplicationFromMonitor_MDR(DABMWcmd_SetMonitorFilter, hMonitor);
	for (i = 0; i < total_filters; i++)
	{
		/* total_filters is bound by ETAL_MAX_QUALITY_PER_MONITOR which is less than 255
		 * so the cast for 'i' is safe */
		retval = ETAL_translateParameters_MDR(mon, (tU8)i, &par_off, &par_size, &th_min, &th_max, &skip);

		if (retval == OSAL_ERROR)
		{
			break;
		}
		filter_index = ETAL_addFilter_MDR();

		if (mon->requested.m_monitoredIndicators[i].m_MonitoredIndicator != EtalQualityIndicator_DabFieldStrength)
		{
			ETAL_utilitySetU8 (DABMWcmd_SetMonitorFilter, ETAL_DABMW_SETMONITORFILTER_FILTERID,  (tU8)filter_index);
			ETAL_utilitySetU16(DABMWcmd_SetMonitorFilter, ETAL_DABMW_SETMONITORFILTER_PAROFFSET, par_off);
			ETAL_utilitySetU8 (DABMWcmd_SetMonitorFilter, ETAL_DABMW_SETMONITORFILTER_PARSIZE,   par_size);
			ETAL_utilitySetU32(DABMWcmd_SetMonitorFilter, ETAL_DABMW_SETMONITORFILTER_THMIN,     th_min);
			ETAL_utilitySetU32(DABMWcmd_SetMonitorFilter, ETAL_DABMW_SETMONITORFILTER_THMAX,     th_max);
			ETAL_utilitySetU16(DABMWcmd_SetMonitorFilter, ETAL_DABMW_SETMONITORFILTER_SKIPCOUNT, skip);
			retval = ETAL_sendCommandTo_MDR(DABMWcmd_SetMonitorFilter, sizeof(DABMWcmd_SetMonitorFilter), &cstatus, FALSE, NULL, NULL);

			flt = ETAL_statusFilterGet(filter_index);
			if(flt != NULL)
			{
				if (retval == OSAL_OK)
				{
					total_filters_for_dcop++;
					flt->isValidForDcop = TRUE;
					flt->paramOffset = par_off;
					flt->paramSize = par_size;
				}
				else
				{
					flt->isValid = FALSE;
					break;
				}
			}
		}
		else
		{
			// Monitored by the CMOST
			flt = ETAL_statusFilterGet(filter_index);
			if(flt != NULL)
			{
				flt->isValidForDcop = FALSE;
				flt->paramOffset = par_off;
				flt->paramSize = par_size;
			}
		}

		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Create filterIndex[%d]", filter_index);

		mon->monitorConfig.MDR.filterIndex[i] = (tU8)filter_index;
		mon->monitorConfig.MDR.filterCountMonitor++;
		mon->monitorConfig.MDR.filterMask |= ETAL_filterMaskFromIndex_MDR(filter_index);
		ETAL_statusFilterCountIncrement();
	}

	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, NULL, 0);
	}

	if (total_filters_for_dcop == 0)
	{
		// No filter to be configured for DCOP
		retval = OSAL_ERROR_NOT_SUPPORTED;
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdStopMonitor_MDR
 *
 **************************/
static tVoid ETAL_cmdStopMonitor_MDR(ETAL_HANDLE hMonitor)
{
	tU16 cstatus = 0;
	tU8 DABMWcmd_GetMonitorDabInformationStop[] =  {0xB3, 0x00, 0x03, 0x02, 0x00, ETAL_MDR_GETMONITORINFO_SKIP_COUNT};
	tU8 DABMWcmd_GetMonitorAmFmInformationStop[] = {0xB3, 0x00, 0x05, 0x02, 0x00, ETAL_MDR_GETMONITORINFO_SKIP_COUNT};
	EtalBcastStandard standard;
	etalReceiverStatusTy *recvp = NULL;
	etalMonitorTy *pmon = NULL;
	tU8 *cmd = NULL;
	tU32 cmd_len;
	tSInt retval = OSAL_OK;

	pmon = ETAL_statusGetMonitor(hMonitor);

	if(pmon != NULL)
	{
		if(pmon->monitorConfig.MDR.isDCOPMonitoringStarted == TRUE)
		{
			recvp = ETAL_receiverGet(ETAL_statusGetReceiverFromMonitor(pmon));
			if(recvp != NULL)
			{
				standard = recvp->currentStandard;
				switch (standard)
				{
					case ETAL_BCAST_STD_DAB:
						cmd = DABMWcmd_GetMonitorDabInformationStop;
						cmd_len = sizeof(DABMWcmd_GetMonitorDabInformationStop);

						ETAL_paramSetApplicationFromMonitor_MDR(cmd, hMonitor);
						retval = ETAL_sendCommandTo_MDR(cmd, cmd_len, &cstatus, FALSE, NULL, NULL);

						if (retval != OSAL_OK)
						{
							ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, NULL, 0);
						}
						break;

					case ETAL_BCAST_STD_FM:
					case ETAL_BCAST_STD_AM:
						cmd = DABMWcmd_GetMonitorAmFmInformationStop;
						cmd_len = sizeof(DABMWcmd_GetMonitorAmFmInformationStop);

						ETAL_paramSetApplicationFromMonitor_MDR(cmd, hMonitor);
						retval = ETAL_sendCommandTo_MDR(cmd, cmd_len, &cstatus, FALSE, NULL, NULL);

						if (retval != OSAL_OK)
						{
							ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, NULL, 0);
						}
						break;

					default:
						break;
				}
			}
			else
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_cmdStopMonitor_MDR recvp: 0x%x", recvp);
				ASSERT_ON_DEBUGGING(0);
			}
		}
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_cmdStopMonitor_MDR pmon: 0x%x", pmon);
		ASSERT_ON_DEBUGGING(0);
	}
	return;
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API


/***************************
 *
 * ETAL_setSeamlessEstimationConfig_MDR
 *
 **************************/
static tVoid ETAL_setSeamlessEstimationConfig_MDR(tU8 *cmd, etalSeamlessEstimationConfigTy *seamlessEstimationConfig)
{
	/* Set mode : 0 (Stop), 1 (Start), 2 (Start only delay est.), 3 (Start only loudness est.) */
	cmd[4] = (tU8)seamlessEstimationConfig->mode;

	/* Set start position */
	cmd[5] = (tU8)((tU32)(seamlessEstimationConfig->startPosition & 0xFF000000) >> 24);
	cmd[6] = (tU8)((tU32)(seamlessEstimationConfig->startPosition & 0x00FF0000) >> 16);
	cmd[7] = (tU8)((tU32)(seamlessEstimationConfig->startPosition & 0x0000FF00) >>  8);
	cmd[8] = (tU8)((tU32)(seamlessEstimationConfig->startPosition & 0x000000FF) >>  0);

	/* Set stop position */
	cmd[9] = (tU8)((tU32)(seamlessEstimationConfig->stopPosition & 0xFF000000) >> 24);
	cmd[10] = (tU8)((tU32)(seamlessEstimationConfig->stopPosition & 0x00FF0000) >> 16);
	cmd[11] = (tU8)((tU32)(seamlessEstimationConfig->stopPosition & 0x0000FF00) >>  8);
	cmd[12] = (tU8)((tU32)(seamlessEstimationConfig->stopPosition & 0x000000FF) >>  0);

	/* Set down-sampling factor */
	cmd[13] = (tU8)0x08;

	/* Set minimum loudness duration */
	cmd[14] = (tU8)0x03;
}

/***************************
 *
 * ETAL_setSeamlessSwitchingConfig_MDR
 *
 **************************/
static tVoid ETAL_setSeamlessSwitchingConfig_MDR(tU8 *cmd, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig)
{
	/* Set system to switch */
	cmd[4] = (tU8)seamlessSwitchingConfig->systemToSwitch;

	/* Set provider type */
	cmd[5] = (tU8)seamlessSwitchingConfig->providerType;

	/* Set absolute delay estimate */
	cmd[6] = (tU8)((seamlessSwitchingConfig->absoluteDelayEstimate & 0xFF000000) >> 24);
	cmd[7] = (tU8)((seamlessSwitchingConfig->absoluteDelayEstimate & 0x00FF0000) >> 16);
	cmd[8] = (tU8)((seamlessSwitchingConfig->absoluteDelayEstimate & 0x0000FF00) >>  8);
	cmd[9] = (tU8)((seamlessSwitchingConfig->absoluteDelayEstimate & 0x000000FF) >>  0);

	/* Set delay estimate */
	cmd[10] = (tU8)((seamlessSwitchingConfig->delayEstimate & 0xFF000000) >> 24);
	cmd[11] = (tU8)((seamlessSwitchingConfig->delayEstimate & 0x00FF0000) >> 16);
	cmd[12] = (tU8)((seamlessSwitchingConfig->delayEstimate & 0x0000FF00) >>  8);
	cmd[13] = (tU8)((seamlessSwitchingConfig->delayEstimate & 0x000000FF) >>  0);

	/* Set time stamp FAS */
	cmd[14] = (tU8)((seamlessSwitchingConfig->timestampFAS & 0xFF000000) >> 24);
	cmd[15] = (tU8)((seamlessSwitchingConfig->timestampFAS & 0x00FF0000) >> 16);
	cmd[16] = (tU8)((seamlessSwitchingConfig->timestampFAS & 0x0000FF00) >>  8);
	cmd[17] = (tU8)((seamlessSwitchingConfig->timestampFAS & 0x000000FF) >>  0);

	/* Set time stamp SAS */
	cmd[18] = (tU8)((seamlessSwitchingConfig->timestampSAS & 0xFF000000) >> 24);
	cmd[19] = (tU8)((seamlessSwitchingConfig->timestampSAS & 0x00FF0000) >> 16);
	cmd[20] = (tU8)((seamlessSwitchingConfig->timestampSAS & 0x0000FF00) >>  8);
	cmd[21] = (tU8)((seamlessSwitchingConfig->timestampSAS & 0x000000FF) >>  0);

	/* Set average RMS for FAS */
	cmd[22] = (tU8)((seamlessSwitchingConfig->averageRMS2FAS & 0xFF000000) >> 24);
	cmd[23] = (tU8)((seamlessSwitchingConfig->averageRMS2FAS & 0x00FF0000) >> 16);
	cmd[24] = (tU8)((seamlessSwitchingConfig->averageRMS2FAS & 0x0000FF00) >>  8);
	cmd[25] = (tU8)((seamlessSwitchingConfig->averageRMS2FAS & 0x000000FF) >>  0);

	/* Set average RMS for SAS */
	cmd[26] = (tU8)((seamlessSwitchingConfig->averageRMS2SAS & 0xFF000000) >> 24);
	cmd[27] = (tU8)((seamlessSwitchingConfig->averageRMS2SAS & 0x00FF0000) >> 16);
	cmd[28] = (tU8)((seamlessSwitchingConfig->averageRMS2SAS & 0x0000FF00) >>  8);
	cmd[29] = (tU8)((seamlessSwitchingConfig->averageRMS2SAS & 0x000000FF) >>  0);
}

/***************************
 *
 * ETAL_savePacketAddressAndSubchIdForDatapath
 *
 **************************/
static tSInt ETAL_savePacketAddressAndSubchIdForDatapath(ETAL_HANDLE hDatapath, tU32 eid, tU32 sid)
{
	EtalServiceInfo serv_info;
	tBool have_data = FALSE;
	tU16 packet_address;
	tU8 subchid;

	if (ETAL_cmdGetSpecificServiceData_MDR(eid, sid, &serv_info, NULL, &have_data) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!have_data)
	{
		return OSAL_ERROR;
	}
	packet_address = serv_info.m_packetAddress;
	subchid = serv_info.m_subchId;
	return ETAL_statusAddDataServiceInfo(hDatapath, packet_address, subchid);
}

/***************************
 *
 * ETAL_removePacketAddressAndSubchIdForDatapath
 *
 **************************/
static tSInt ETAL_removePacketAddressAndSubchIdForDatapath(ETAL_HANDLE hDatapath, tU32 eid, tU32 sid)
{
	EtalServiceInfo serv_info;
	tBool have_data = FALSE;
	tU16 packet_address;
	tU8 subchid;

	if (ETAL_cmdGetSpecificServiceData_MDR(eid, sid, &serv_info, NULL, &have_data) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!have_data)
	{
		return OSAL_ERROR;
	}
	packet_address = serv_info.m_packetAddress;
	subchid = serv_info.m_subchId;
	return ETAL_statusRemoveDataServiceInfo(hDatapath, packet_address, subchid);
}

/***************************
 *
 * ETAL_extractPADDLS_MDR
 *
 **************************/
tSInt ETAL_extractPADDLS_MDR(tU8 *data, tU32 len, etalPADDLSTy *pad, tU8 *app)
{
	tU32 string_len;
	tU8 *payload; // payload contains a string but is pre-pended by PADtype and charset (tU8)
	tU32 payload_len;

	if (len < ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN)
	{
		return OSAL_ERROR;
	}

	payload_len = ETAL_getGenericPayloadLen_MDR(data);
	if (payload_len < 2)
	{
		// at least the PAD TYPE and CHARSET bytes should be present
		return OSAL_ERROR;
	}
	payload = (tU8 *)ETAL_getGenericPayload_MDR(data);


	string_len = payload_len - 2;
	if (string_len >= ETAL_DEF_MAX_PAD_STRING - 1)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "PAD DLS string length (%d) not supported, truncated to %d", string_len, ETAL_DEF_MAX_PAD_STRING - 1);
		string_len = ETAL_DEF_MAX_PAD_STRING - 1;
	}

	*app = (((payload[0] >> 4) & 0x0F) != 0x01) ? DABMW_SECONDARY_AUDIO_DAB_APP : DABMW_MAIN_AUDIO_DAB_APP;

	pad->m_charset = payload[1];
	OSAL_pvMemoryCopy((tVoid *)pad->m_PAD_DLS, (tPCVoid)(payload + 2), string_len);
	pad->m_PAD_DLS[string_len] = '\0';
	return OSAL_OK;
}

/***************************
 *
 * ETAL_extractPADDLPLUS_MDR
 *
 **************************/
tSInt ETAL_extractPADDLPLUS_MDR(tU8 *data, tU32 len, etalPADDLPLUSTy *pad, tU8 *app)
{
	tU8 *payload;
	tS32 payload_len;
	tS32 ret = OSAL_OK;

	if (len < ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN)
	{
		return OSAL_ERROR;
	}

	payload_len = (tS32)ETAL_getGenericPayloadLen_MDR(data);
	payload = (tU8 *)ETAL_getGenericPayload_MDR(data);

	*app = ((payload[payload_len-1] & 0xFF) != 0x10) ? DABMW_SECONDARY_AUDIO_DAB_APP : DABMW_MAIN_AUDIO_DAB_APP;

	/* Remove app put at the end of the payload */
	payload_len = payload_len - 1;

	pad->m_nbOfItems = 0;
	while(payload_len > 0)
	{
		if(pad->m_nbOfItems < ETAL_DEF_MAX_DL_PLUS_ITEM)
		{
			pad->m_item[pad->m_nbOfItems].m_labelLength = ETAL_utilityGetU8(payload, 0) - 3;
			pad->m_item[pad->m_nbOfItems].m_contentType = ETAL_utilityGetU8(payload, 1) & 0x7F;
			pad->m_item[pad->m_nbOfItems].m_runningStatus = (ETAL_utilityGetU8(payload, 1) & 0x80)?TRUE:FALSE;
			pad->m_item[pad->m_nbOfItems].m_charset = ETAL_utilityGetU8(payload, 2);

			/* insert a check point to avoid memory leakage */
			if(pad->m_item[pad->m_nbOfItems].m_labelLength <= ETAL_DEF_MAX_PAD_STRING - 1)
			{
				OSAL_pvMemoryCopy((tPVoid)pad->m_item[pad->m_nbOfItems].m_label, (tPCVoid)&(payload[3]), pad->m_item[pad->m_nbOfItems].m_labelLength);
			}
			else
			{
				OSAL_pvMemoryCopy((tPVoid)pad->m_item[pad->m_nbOfItems].m_label, (tPCVoid)&(payload[3]), ETAL_DEF_MAX_PAD_STRING-1);
				pad->m_item[pad->m_nbOfItems].m_label[ETAL_DEF_MAX_PAD_STRING-1] = '\0';
			}

			payload_len -= ETAL_utilityGetU8(payload, 0);
			payload += ETAL_utilityGetU8(payload, 0);
			pad->m_nbOfItems++;
		}
		else
		{
			ret = OSAL_ERROR;
			break;
		}
	}

	/* Basic check on the payload length */
	if(payload_len != 0)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "Error in extraction of DL PLUS items : payload_len: %d\n", payload_len);
		ASSERT_ON_DEBUGGING(0);
		ret = OSAL_ERROR;
	}
	return ret;
}


#if defined (CONFIG_ETALTML_HAVE_LEARN)

/***************************
 *
 * ETAL_extractLearn_MDR
 *
 **************************/
tSInt ETAL_extractLearn_MDR(tU8 *buf, tU32 len, etalLearnConfigTy *learnCfgp)
{
    tU8 i, nbOfFrequency, totalNbOfEnsemble = 0, totalFoundNbOfEnsemble = 0, offset = 0, entryLen = 0;
    tU8 *ensembleInfo = NULL;
    EtalLearnStatusTy *learnStatusp = &(learnCfgp->learnStatus);
	tU8 vl_tmp_PayloadOffset;

    learnStatusp->m_receiverHandle = ETAL_receiverSearchActiveSpecial(cmdSpecialLearn);
    if (learnStatusp->m_receiverHandle == ETAL_INVALID_HANDLE)
    {
        return OSAL_ERROR;
    }

	if (ETAL_DABMW_HEADER_HAS_LONG_PAYLOAD(buf))
	{
		vl_tmp_PayloadOffset = ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 1;
	}
	else
	{
		vl_tmp_PayloadOffset = ETAL_MDR_RESPONSE_PAYLOAD_OFFSET;
	}
	
		// payload[0] = app
		// payload[1] = total num
		// payload[2] = total num new
		// payload[3] = total confirmed
		// payload[4] = total old
		// payload[5] = start entry ensemble

    totalNbOfEnsemble = buf[vl_tmp_PayloadOffset + 1];
	totalFoundNbOfEnsemble = buf[vl_tmp_PayloadOffset + 2] + buf[vl_tmp_PayloadOffset + 3];
	
    learnCfgp->nbOfFreq = 0;
    learnStatusp->m_status        = ETAL_LEARN_FINISHED;
    learnStatusp->m_frequency     = ETAL_receiverGetFrequency(learnStatusp->m_receiverHandle);

	printf("totalNbOfEnsemble %d, totalFoundNbOfEnsemble %d\n", totalNbOfEnsemble, totalFoundNbOfEnsemble);
	// let's keep only the one found
	// i.e. new and confirmed
	//
		
    while(totalFoundNbOfEnsemble > 0)
    {


		
    	// Ensemble Info start
    	// Offset = start of ensemble info
    	// Ensemble info, verbose mode, is 
    	// Entry Len (1 byte) [0] 
    	// UEID (3 bytes) [1-2-3]
    	// Freq_.. 1 (4 bytes) [4-5-6-7]
    	// Freq..i (4 bytes) [4+i*4... 7+i*4]
    	//
    	
    	printf("payload %d, /offset = %d, totalFoundNbOfEnsemble %d\n", 
				vl_tmp_PayloadOffset, offset, 
				totalFoundNbOfEnsemble);
			
        ensembleInfo = &buf[vl_tmp_PayloadOffset + 5 + offset]; 
		// entry len
        entryLen = ensembleInfo[0];
		// nb Freq = (entryLen - UEID Len (3) ) / 4 (4 bytes/freq);
        nbOfFrequency = (entryLen - 3) / 4;

		printf("entryLen %d, nb freq %d\n",  entryLen, nbOfFrequency );
		

        for(i = 0; i < nbOfFrequency; i++)
        {
            learnCfgp->frequencyListTmp[learnCfgp->nbOfFreq].m_fieldStrength = 0;
            learnCfgp->frequencyListTmp[learnCfgp->nbOfFreq].m_frequency = ((((tU32)ensembleInfo[4*i + 4]) << 24) | (((tU32)ensembleInfo[4*i + 5]) << 16) |
                    (((tU32)ensembleInfo[4*i + 6]) << 8) | ((tU32)ensembleInfo[4*i + 7]));
            learnCfgp->frequencyListTmp[learnCfgp->nbOfFreq].m_HDServiceFound = FALSE;
            learnCfgp->frequencyListTmp[learnCfgp->nbOfFreq].m_ChannelID = 0;

            learnCfgp->nbOfFreq++;

            if(!(learnCfgp->nbOfFreq < ETAL_LEARN_MAX_NB_FREQ))
            {
                learnStatusp->m_status = ETAL_LEARN_ERROR;
                return OSAL_ERROR;
            }
        }

		// update next offset to jump next freq
		// 
		offset += entryLen+1;
		
		// Update remaining ensemble number
		totalFoundNbOfEnsemble--;
		
    }

    /* Build the event */
    for(i = 0; i < learnCfgp->nbOfFreq; i++)
    {
        learnStatusp->m_frequencyList[i] = learnCfgp->frequencyListTmp[i];
    }
    learnStatusp->m_nbOfFrequency = learnCfgp->nbOfFreq;

    return OSAL_OK;
}
#endif // CONFIG_ETALTML_HAVE_LEARN

#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)
/***************************
 *
 * ETAL_extractSeamlessEstimation_MDR
 *
 **************************/
tSInt ETAL_extractSeamlessEstimation_MDR(tU8* buf, tU32 len, EtalSeamlessEstimationStatus *seamlessEstimation)
{
	seamlessEstimation->m_receiverHandle = ETAL_receiverSearchActiveSpecial(cmdSpecialSeamlessEstimation);
	if (seamlessEstimation->m_receiverHandle == ETAL_INVALID_HANDLE)
	{
		return OSAL_ERROR;
	}
	/*
	 * byte 0 = status
	 * byte 1 = provider type
	 * byte 2-5 = absolute delay estimate
	 * byte 6-9 = delay estimate
	 * byte 10-13 = time stamp FAS
	 * byte 14-17 = time stamp SAS
	 * byte 18-21 = average RMS on FAS
	 * byte 22-25 = average RMS on SAS
	 * byte 26-29 = confidence level
	 */
	seamlessEstimation->m_status = buf[ETAL_MDR_RESPONSE_PAYLOAD_OFFSET];
	seamlessEstimation->m_providerType = buf[ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 1];
	seamlessEstimation->m_absoluteDelayEstimate = (tS32)ETAL_utilityGetU32(buf, ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 2);
	seamlessEstimation->m_delayEstimate = (tS32)ETAL_utilityGetU32(buf, ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 6);
	seamlessEstimation->m_timestamp_FAS = ETAL_utilityGetU32(buf, ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 10);
	seamlessEstimation->m_timestamp_SAS = ETAL_utilityGetU32(buf, ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 14);
	seamlessEstimation->m_RMS2_FAS = ETAL_utilityGetU32(buf, ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 18);
	seamlessEstimation->m_RMS2_SAS = ETAL_utilityGetU32(buf, ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 22);
	seamlessEstimation->m_confidenceLevel = ETAL_utilityGetU32(buf, ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 26);
	return OSAL_OK;
}

/***************************
 *
 * ETAL_extractSeamlessSwitching_MDR
 *
 **************************/
tSInt ETAL_extractSeamlessSwitching_MDR(tU8* buf, tU32 len, EtalSeamlessSwitchingStatus *seamlessSwitching)
{
	seamlessSwitching->m_receiverHandle = ETAL_receiverSearchActiveSpecial(cmdSpecialSeamlessSwitching);
	if (seamlessSwitching->m_receiverHandle == ETAL_INVALID_HANDLE)
	{
		return OSAL_ERROR;
	}
	/*
	 * byte 0-3 = delay estimate
	 * byte 4-7 = confidence level
	 * byte 8   = status
	 */
	seamlessSwitching->m_absoluteDelayEstimate = (tS32)ETAL_utilityGetU32(buf, ETAL_MDR_RESPONSE_PAYLOAD_OFFSET);
	seamlessSwitching->m_status = buf[ETAL_MDR_RESPONSE_PAYLOAD_OFFSET + 16];

	return OSAL_OK;
}
#endif //#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)


/***************************
 *
 * ETAL_extractTune_MDR
 *
 **************************/
tSInt ETAL_extractTune_MDR(tU8* buf, tU32 len, EtalTuneStatus *tune)
{

	tune->m_receiverHandle = ETAL_receiverSearchActiveSpecial(cmdSpecialTune);
	if (tune->m_receiverHandle == ETAL_INVALID_HANDLE)
	{
		return OSAL_ERROR;
	}

	tune->m_stopFrequency 	= ((((tU32)buf[ETAL_MDR_RESPONSE_PAYLOAD_OFFSET+1] << 16) & (0x03)) |
								((tU32)buf[ETAL_MDR_RESPONSE_PAYLOAD_OFFSET+2] << 8) |
								((tU32)buf[ETAL_MDR_RESPONSE_PAYLOAD_OFFSET+3] << 0)) * 16;
	tune->m_sync 			= (tU32)buf[ETAL_MDR_RESPONSE_PAYLOAD_OFFSET+4]&0x07;;
	tune->m_muteStatus 		= ETAL_INVALID_MUTE_STATUS;
	tune->m_serviceId 		= ETAL_INVALID_PROG;

	return OSAL_OK;
}

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_resetFilter_MDR
 *
 **************************/
tVoid ETAL_resetFilter_MDR(etalMonitorTy *pmon)
{
	if(pmon != NULL)
	{
		OSAL_pvMemorySet((tVoid *)&pmon->monitorConfig.MDR.filterIndex, 0x00, sizeof(pmon->monitorConfig.MDR.filterIndex));
		pmon->monitorConfig.MDR.filterCountMonitor = (tS8)0;
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_resetFilter_MDR pmon: 0x%x", pmon);
		ASSERT_ON_DEBUGGING(0);
	}
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_initFilterMask_MDR
 *
 **************************/
tVoid ETAL_initFilterMask_MDR(etalFilterMaskTy *mask)
{
	*mask = 0xFFFF;
}

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_checkFilter_MDR
 *
 **************************/
tSInt ETAL_checkFilter_MDR(ETAL_HANDLE hMonitor, const EtalBcastQualityMonitorAttr* pMonitorAttr)
{
	etalMonitorTy *pmon = NULL;
	tS32 new_filter_count, old_filter_count = 0;
	tSInt ret = OSAL_OK;
	/*
	 * verify if the new filters requested by the monitor can be accomodated globally
	 */
	if (hMonitor != ETAL_INVALID_HANDLE)
	{
		pmon = ETAL_statusGetMonitor(hMonitor);
		if(pmon != NULL)
		{
			old_filter_count = (tS32)pmon->monitorConfig.MDR.filterCountMonitor;
		}
		else
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_checkFilter_MDR pmon: 0x%x", pmon);
			ASSERT_ON_DEBUGGING(0);
		}
	}

	new_filter_count = (tS32)ETAL_countIndicatorsPerMonitor(pMonitorAttr);
	if (ETAL_statusFilterCountGet() + new_filter_count - old_filter_count >= ETAL_MAX_FILTERS)
	{
		ret = OSAL_ERROR;
	}

	/*
	 * verify if the new filters can be accomodated locally in the monitor
	 */
	if (new_filter_count >= ETAL_MAX_QUALITY_PER_MONITOR)
	{
		ret = OSAL_ERROR;
	}

	return ret;
}

/***************************
 *
 * ETAL_addFilter_MDR
 *
 **************************/
static tU32 ETAL_addFilter_MDR(void)
{
	etalDCOPFilterTy *flt;
	tU32 i;

	for (i = 0; i < ETAL_MAX_FILTERS; i++)
	{
		flt = ETAL_statusFilterGet(i);
		if (flt->isValid)
		{
			continue;
		}
		flt->isValid = TRUE;
		return i;
	}

	return 0;
}

/***************************
 *
 * ETAL_cmdRemoveFiltersForMonitor_MDR
 *
 **************************/
/*
 * Remove all the filters configured for <hMonitor> by sending DABMW commands
 * and then clearing the monitor status.
 * Since removing by the filters the GetMonitor*Information may saturate the bus,
 * stops Monitor before removing the filters.
 */
tSInt ETAL_cmdRemoveFiltersForMonitor_MDR(ETAL_HANDLE hMonitor)
{
	tU16 cstatus = 0;
	tU8 DABMWcmd_SetMonitorFilter[] = {0x96, 0x00, 0x07, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	etalDCOPFilterTy *flt = NULL;
	etalMonitorTy *mon = NULL;
	tU8 filter_index;
	tU32 i, total;
	tSInt retval = OSAL_OK;

	ETAL_cmdStopMonitor_MDR(hMonitor);

	ETAL_paramSetApplicationFromMonitor_MDR(DABMWcmd_SetMonitorFilter, hMonitor);
	mon = ETAL_statusGetMonitor(hMonitor);

	if(mon != NULL)
	{
		if (mon->monitorConfig.MDR.filterCountMonitor >= (tS8)0)
		{
			total = (tU32)mon->monitorConfig.MDR.filterCountMonitor;
		}
		else
		{
			total = 0;
		}
		for (i = 0; i < total; i++)
		{
			filter_index = mon->monitorConfig.MDR.filterIndex[i];
			flt = ETAL_statusFilterGet((tU32)filter_index);

			if(flt != NULL)
			{
				if(flt->isValidForDcop == TRUE)
				{
					DABMWcmd_SetMonitorFilter[ETAL_DABMW_SETMONITORFILTER_FILTERID] = filter_index;
					DABMWcmd_SetMonitorFilter[ETAL_DABMW_SETMONITORFILTER_MODE] |= 0x01; // set OPERATION bit to remove
					retval = ETAL_sendCommandTo_MDR(DABMWcmd_SetMonitorFilter, sizeof(DABMWcmd_SetMonitorFilter), &cstatus, FALSE, NULL, NULL);
					if (retval != OSAL_OK)
					{
						break;
					}
				}
				if ((ETAL_statusFilterCountGet() <= 0) || (mon->monitorConfig.MDR.filterCountMonitor <= (tS8)0))
				{
					ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "Illegal filtersCount (%d) or filterCountMonitor (%d) in ETAL_cmdRemoveFiltersForMonitor_MDR (monitor_handle %d)", ETAL_statusFilterCountGet(), mon->monitorConfig.MDR.filterCountMonitor, hMonitor);
					ASSERT_ON_DEBUGGING(0);
					return OSAL_ERROR;
				}

				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Remove filterIndex[%d]", filter_index);

				OSAL_pvMemorySet((tVoid *)flt, 0x00, sizeof(etalDCOPFilterTy));
				mon->monitorConfig.MDR.filterCountMonitor--;
				ETAL_statusFilterCountDecrement();
			}
		}

		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, NULL, 0);
		}
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_cmdRemoveFiltersForMonitor_MDR mon: 0x%x", mon);
		ASSERT_ON_DEBUGGING(0);
		retval = OSAL_ERROR;
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdStartMonitor_MDR
 *
 **************************/
 /*
  * Configure DABMW filters and then start the GetMonitorDabInformation or GetMonitorAmFmInformation in auto mode
  */
tSInt ETAL_cmdStartMonitor_MDR(ETAL_HANDLE hMonitor)
{
	tU16 cstatus = 0;
	tU8 DABMWcmd_GetMonitorDabInformationStart[] =  {0xB2, 0x00, 0x03, 0x02, 0x00, ETAL_MDR_GETMONITORINFO_SKIP_COUNT};

	EtalBcastStandard standard;
	etalReceiverStatusTy *recvp = NULL;
	etalMonitorTy *pmon = NULL;
	tU8 *cmd = NULL;
	tU32 cmd_len;
	ETAL_HANDLE hReceiver;
	tSInt retval = OSAL_OK;

	pmon = ETAL_statusGetMonitor(hMonitor);

	if(pmon != NULL)
	{
		hReceiver = ETAL_statusGetReceiverFromMonitor(pmon);
		recvp = ETAL_receiverGet(hReceiver);

		if(recvp != NULL)
		{
			standard = recvp->currentStandard;
			switch (standard)
			{
				case ETAL_BCAST_STD_DAB:
					cmd = DABMWcmd_GetMonitorDabInformationStart;
					cmd_len = sizeof(DABMWcmd_GetMonitorDabInformationStart);
					break;
				case ETAL_BCAST_STD_FM:
				case ETAL_BCAST_STD_AM:
					/* nothing to do, AMFM quality measured on STAR */
					return OSAL_OK;
				default:
					return OSAL_ERROR;
			}

			retval = ETAL_cmdMonitorAddFilters_MDR(hMonitor);

			if (retval == OSAL_OK)
			{
				ETAL_paramSetApplicationFromMonitor_MDR(cmd, hMonitor);
				/*
				 * This is an Auto command, one or more responses are expected but should not
				 * be considered part of the send FSM, because due to the filter settings
				 * the first response might arrive _after_ FSM timeout expiration and this
				 * should not be considered an error.
				 */
				retval = ETAL_sendCommandTo_MDR(cmd, cmd_len, &cstatus, FALSE, NULL, NULL);
				/*
				 * special case: Auto command was not canceled by previous run,
				 * the MDR3 will fail; so issue a stop then try again
				 */
				if ((retval != OSAL_OK) && (cstatus == DABMW_STATUS_DUPLICATED_AUTO_COMMAND))
				{
					ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "Trying to recover from abnormal situation (DAB)");
					ETAL_cmdStopMonitor_MDR(hMonitor);
					retval = ETAL_sendCommandTo_MDR(cmd, cmd_len, &cstatus, FALSE, NULL, NULL);
				}
				else
				{
					pmon->monitorConfig.MDR.isDCOPMonitoringStarted = TRUE;
				}
				if (retval != OSAL_OK)
				{
					ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, NULL, 0);
				}
			}

			if (retval == OSAL_ERROR_NOT_SUPPORTED)
			{
				// Filter defined for DAB but monitored by the CMOST
				if(pmon->monitorConfig.MDR.isDCOPMonitoringStarted == TRUE)
				{
					ETAL_cmdStopMonitor_MDR(hMonitor);
				}
				retval = OSAL_OK;
			}
		}
		else
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_cmdStartMonitor_MDR recvp: 0x%x", recvp);
			ASSERT_ON_DEBUGGING(0);
			retval = OSAL_ERROR;
		}
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_cmdStartMonitor_MDR pmon: 0x%x", pmon);
		ASSERT_ON_DEBUGGING(0);
		retval = OSAL_ERROR;
	}
	return retval;
}

/***************************
 *
 * ETAL_cmdResetMonitorAndFilters_MDR
 *
 **************************/
/*
 * Send DABMW commands to remove all filters and all Monitor messages
 */
tSInt ETAL_cmdResetMonitorAndFilters_MDR(tVoid)
{
	tU16 cstatus = 0;
	tU8 DABMWcmd_SetFilter[] = {0x96, 0x04, 0x07, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // TARGET APPLICATION is ignored since REMOVE ALL is set, but needs to be correct anyway
#if defined (CONFIG_APP_TEST_TUNE_TIME_MEASURE)
	/*
	 * for measurement purpose, instead of shutting down the notifications
	 * enable the DAB STATUS with VERBOSE to see the DAB transitioning to SYNC state
	 */
	tU8 DABMWcmd_SetupEventNotification[] = {0x96, 0x00, 0x10, 0x02, 0x00, 0x04};
#else
	tU8 DABMWcmd_SetupEventNotification[] = {0x96, 0x00, 0x10, 0x02, 0x00, 0x00};
#endif
	tSInt retval;

	/* remove all filters */
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_SetFilter, sizeof(DABMWcmd_SetFilter), &cstatus, FALSE, NULL, NULL);
	if (retval != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/* shut down all notifications */
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_SetupEventNotification, sizeof(DABMWcmd_SetupEventNotification), &cstatus, FALSE, NULL, NULL);
	if (retval != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_cmdPowerUp_MDR
 *
 **************************/
tSInt ETAL_cmdPowerUp_MDR(const DABMW_mwCountryTy country, tU8 nvmFlags)
{
	tU16 cstatus = 0;
	tU8 DABMWcmd_PowerUp[] = {0x92, 0x00, 0x01, 0x01, 0x00};
	tSInt retval = OSAL_OK;

	ETAL_paramSetCountry_MDR(DABMWcmd_PowerUp, country);
	ETAL_paramSetNvmFlags_MDR(DABMWcmd_PowerUp, nvmFlags);
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_PowerUp, sizeof(DABMWcmd_PowerUp), &cstatus, TRUE, NULL, NULL);
	if (retval != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdPowerDown_MDR
 *
 **************************/
tSInt ETAL_cmdPowerDown_MDR(tU8 saveNvm)
{
	tU16 cstatus = 0;
	tU8 DABMWcmd_PowerDown[] = {0x90, 0x00, 0x04};
	tSInt retval = OSAL_OK;

	ETAL_paramSetSaveNvm_MDR(DABMWcmd_PowerDown, saveNvm);
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_PowerDown, sizeof(DABMWcmd_PowerDown), &cstatus, TRUE, NULL, NULL);
	if (retval != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdClearDatabase_MDR
 *
 **************************/
tSInt ETAL_cmdClearDatabase_MDR(tU8 clearDatabase)
{
	tU16 cstatus = 0;
	tU8 DABMWcmd_ClearDatabase[] = {0x90, 0x00, 0x26};
	tSInt retval = OSAL_OK;

	ETAL_paramSetClearDatabase_MDR(DABMWcmd_ClearDatabase, clearDatabase);
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_ClearDatabase, sizeof(DABMWcmd_ClearDatabase), &cstatus, TRUE, NULL, NULL);
	if (retval != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdPing_MDR
 *
 **************************/
tSInt ETAL_cmdPing_MDR(void)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tU8 DABMWcmdPing[3] = {0x90, 0x00, 0x00};
	tSInt retval;

	retval = ETAL_sendCommandTo_MDR(DABMWcmdPing, sizeof(DABMWcmdPing), &cstatus, TRUE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "Pinging the DCOP DAB");
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);

	}
	return retval;
}

/***************************
 *
 * ETAL_cmdTune_MDR
 *
 **************************/
tSInt ETAL_cmdTune_MDR(ETAL_HANDLE hReceiver, tU32 dwFrequency, etalCmdTuneActionTy type, tU16 tuneTimeout)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tU8 DABMWcmd_Tune[]   =  {0x92, 0x00, 0x60, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00}; // cmdId may be overwritten below!
	tU8 DABMWcmd_TuneTm[] =  {0x92, 0x00, 0x60, 0x07, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}; // cmdId may be overwritten below!
	tU8 DABMWcmd_RdsOn[] = {0x96, 0x3C, 0x3A, 0x03, 0x01, 0x06, 0x01};
	tSInt retval;
	tU32 device_list;
	tU8 *cmd;
	tU32 cmd_len;

    if (0 == tuneTimeout)
    {
		cmd = DABMWcmd_Tune;
		cmd_len = sizeof(DABMWcmd_Tune);
    }
    else
    {
		cmd = DABMWcmd_TuneTm;
		cmd_len = sizeof(DABMWcmd_TuneTm);

        DABMWcmd_TuneTm[9] = (tU8)((tuneTimeout >> 8) & 0xFF);
        DABMWcmd_TuneTm[10] = (tU8)((tuneTimeout >> 0) & 0xFF);
    }

	ETAL_paramSetApplicationFromReceiver_MDR(cmd, hReceiver);
	ETAL_paramSetFrequency_MDR(cmd, dwFrequency);
	if (type == cmdTuneImmediateResponse)
	{
		/*
		 * Use the special version of the Tune that sends an immediate response
		 */
		cmd[2] = 0x60 + 0x1A;
	}
	if (type == cmdTuneDoNotWaitResponseDcopDirectResponseOnStatus)
	{
		/*
		 * Request the DCOP immediate response
		 */
		cmd[8] |= 0x40;
	}

	if (type == cmdTuneDoNotWaitResponseDcopDirectResponseOnStatus)
	{
		retval = ETAL_sendCommandTo_MDR(cmd, cmd_len, &cstatus, FALSE, &resp, &resplen);
	}
	else
	{
		retval = ETAL_sendCommandTo_MDR(cmd, cmd_len, &cstatus, TRUE, &resp, &resplen);
	}

	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if (type != cmdTuneDoNotWaitResponseDcopDirectResponseOnStatus)
	{
		if (resplen >= ETAL_DABMW_RESPONSE_NO_PAYLOAD_LEN)
		{
			if (ETAL_MDR_RESPONSE_STATUS(resp) != 0)
			{
				if (ETAL_MDR_RESPONSE_STATUS(resp) == DABMW_CMD_STATUS_ERR_TIMEOUT)
				{
					/*
					 * Tune was successful but no signal detected on the requested
					 * frequency; this is not really an error so needs to be
					 * specially treated
					 */
					return OSAL_ERROR_TIMEOUT_EXPIRED;
				}
				return OSAL_ERROR;
			}
		}
		if (type == cmdTuneImmediateResponse)
		{
			/*
			 * this version of the command used for Tune Request FSM, RDS not needed
			 */
			return OSAL_OK;
		}
		device_list = ETAL_cmdRoutingCheck(hReceiver, commandRDS);

		if (ETAL_CMDROUTE_TO_DCOP(device_list))
		{
			retval = ETAL_sendCommandTo_MDR(DABMWcmd_RdsOn, sizeof(DABMWcmd_RdsOn), &cstatus, FALSE, NULL, NULL);
			if (retval != OSAL_OK)
			{
				ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, NULL, 0);
				return OSAL_ERROR;
			}
		}
	}

	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetQuality_MDR
 *
 **************************/
tSInt ETAL_cmdGetQuality_MDR(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp = NULL, *payload;
	tU8 DABMWcmd_GetMonitorDABInformation[] = {0x92, 0x00, 0x03, 0x02, 0x00, 0x00}; // SINGLE request, no SKIP COUNTER
	OSAL_tMSecond timestamp;
	EtalBcastStandard std;
	tU8 *cmd;
	tU32 cmd_len;
	tSInt retval = OSAL_OK;

	std = ETAL_receiverGetStandard(hReceiver);
	switch (std)
	{
		case ETAL_BCAST_STD_DAB:
			cmd = DABMWcmd_GetMonitorDABInformation;
			cmd_len = sizeof(DABMWcmd_GetMonitorDABInformation);
			break;

		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
#ifdef CONFIG_ETAL_SUPPORT_CMOST
			/* nothing to do, AMFM quality measured on CMOST */
			return OSAL_OK;
#else
			return OSAL_ERROR;
#endif
		default:
			return OSAL_ERROR;
	}

	ETAL_paramSetApplicationFromReceiver_MDR(cmd, hReceiver);
	/*
	 * GetMonitor*Information commands are implemented in MDR so that if
	 * the status was already sent and there is no update, no response is
	 * provided (only a notification with no errors).
	 * Thus using the normal ETAL_sendCommandTo_MDR it happens that
	 * ETAL sends the request, the MDR has nothing to send, ETAL
	 * times out after the ETAL_TO_MDR_CMD_TIMEOUT_IN_MSEC. To avoid this
	 * use ETAL_sendCommandWithRetryTo_MDR which resends the
	 * command if does not receive a response within the timeout
	 */
	retval = ETAL_sendCommandWithRetryTo_MDR(Cmd_Mdr_Type_Normal, cmd, cmd_len, &cstatus, TRUE, &resp, &resplen, ETAL_TO_MDR_RETRY_TIMEOUT_IN_MSEC);

	timestamp = OSAL_ClockGetElapsedTime();
	if ((retval == OSAL_OK) && (resp != NULL))
	{
		p->m_TimeStamp = timestamp;
		p->m_standard = std;
		p->m_Context = NULL; /* TODO what to put in EtalBcastQualityContainer.m_Context */

		payload = ETAL_getGenericPayload_MDR(resp);
		ETAL_extractDabQualityFromNotification_MDR(&(p->EtalQualityEntries.dab), payload);
	}
	else
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdGetAudioQuality_MDR
 *
 **************************/
tSInt ETAL_cmdGetAudioQuality_MDR(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp = NULL, *payload;
	tU8 DABMWcmd_GetAudioDABInformation[] = {0x94, 0x00, 0x0E}; // SINGLE , FAST request

	OSAL_tMSecond timestamp;
	EtalBcastStandard std;
	tU8 *cmd;
	tU32 cmd_len;
	tSInt retval = OSAL_OK;

	std = ETAL_receiverGetStandard(hReceiver);
	switch (std)
	{
		case ETAL_BCAST_STD_DAB:
			cmd = DABMWcmd_GetAudioDABInformation;
			cmd_len = sizeof(DABMWcmd_GetAudioDABInformation);
			break;

		default:
			return OSAL_ERROR;
	}

	ETAL_paramSetApplicationFromReceiver_MDR(cmd, hReceiver);
	// Set a fast command instead
	retval = ETAL_sendCommandTo_MDR(cmd, cmd_len, &cstatus, FALSE, &resp, &resplen);

	timestamp = OSAL_ClockGetElapsedTime();
	if ((retval == OSAL_OK) && (resp != NULL))
	{
		p->m_TimeStamp = timestamp;
		p->m_standard = std;
		p->m_Context = NULL; /* TODO what to put in EtalBcastQualityContainer.m_Context */

		payload = ETAL_getGenericPayload_MDR(resp);
		if (std == ETAL_BCAST_STD_DAB)
		{
			ETAL_extractDabAudioQualityFromNotification_MDR(&(p->EtalQualityEntries.dab), payload);
		}
	}
	else
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
	}

	return retval;
}

#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_cmdGetEnsembleList_MDR
 *
 **************************/
tSInt ETAL_cmdGetEnsembleList_MDR(EtalEnsembleList *list, tBool *have_data)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp, *payload;
	tU8 DABMWcmd_GetEnsembleList[] = {0x90, 0x00, 0x40};
	tSInt retval;
	tU32 list_size, i;

	*have_data = FALSE;
	list->m_ensembleCount = 0;

	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetEnsembleList, sizeof(DABMWcmd_GetEnsembleList), &cstatus, TRUE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if (resplen > ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN)
	{
		payload = ETAL_getGenericPayload_MDR(resp);
		list_size = ETAL_getGenericPayloadLen_MDR(resp) / ETAL_DABMW_GET_ENSLIST_RECORD_SIZE;
		if (list_size > ETAL_DEF_MAX_ENSEMBLE)
		{
			/* truncate to supported size */
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "Unsupported ensemble list size (%d), max %d", list_size, ETAL_DEF_MAX_ENSEMBLE);
			list_size = ETAL_DEF_MAX_ENSEMBLE;
		}
		for (i = 0; i < list_size; i++)
		{
			list->m_ensemble[i].m_ECC =        ETAL_DABMW_GET_ENSLIST_ECC(payload + ETAL_DABMW_GET_ENSLIST_RECORD_SIZE * i);
			list->m_ensemble[i].m_ensembleId = ETAL_DABMW_GET_ENSLIST_EID(payload + ETAL_DABMW_GET_ENSLIST_RECORD_SIZE * i);
			list->m_ensemble[i].m_frequency =  ETAL_DABMW_GET_ENSLIST_FREQ(payload + ETAL_DABMW_GET_ENSLIST_RECORD_SIZE * i);
		}
		list->m_ensembleCount = list_size;
		*have_data = TRUE;
	}

	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetEnsembleData_MDR
 *
 **************************/
tSInt ETAL_cmdGetEnsembleData_MDR(tU32 eid, tU8 *charset, tChar *label, tU16 *bitmap, tBool *have_data)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp, *payload;
	tU8 DABMWcmd_GetEnsembleData[] = {0x92, 0x00, 0x41, 0x03, 0x00, 0x00, 0x00};
	tSInt retval;

	*have_data = FALSE;

	ETAL_paramSetEID_MDR(DABMWcmd_GetEnsembleData, eid);
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetEnsembleData, sizeof(DABMWcmd_GetEnsembleData), &cstatus, TRUE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if (resplen == ETAL_DABMW_GET_ENSDATA_RESP_SIZE)
	{
		payload = ETAL_getGenericPayload_MDR(resp);
		*charset  = ETAL_DABMW_GET_ENSDATA_CHARSET(payload);
		OSAL_pvMemoryCopy((tVoid *)label, (tPCVoid)ETAL_DABMW_GET_ENSDATA_LABELSTART(payload), ETAL_DEF_MAX_LABEL_LEN - 1);
		label[ETAL_DEF_MAX_LABEL_LEN - 1] = '\0';
		*bitmap = ETAL_DABMW_GET_ENSDATA_CHARFLAG(payload);
		*have_data = TRUE;
	}

	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetFIC_MDR
 *
 **************************/
tSInt ETAL_cmdGetFIC_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy action)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tU8 DABMWcmd_GetFIC[] = {0x94, 0x00, 0x42};
	tSInt retval = OSAL_OK;
	static tU8 vl_appRequested = DABMW_NONE_APP;
	DABMW_mwAppTy app;
	etalReceiverStatusTy *recvp;

	recvp = ETAL_receiverGet(hReceiver);
	app = recvp->MDRConfig.application;
		
	if (action == cmdActionStart)
	{
		// parameter depends on what app already requested 
		if ((DABMW_ALL_APPLICATIONS == vl_appRequested)
			||
			(app == vl_appRequested)
			)
		{
			// nothing to do all is already configured
			retval = OSAL_OK;
			goto exit;
		}
		if (DABMW_NONE_APP == vl_appRequested)
		{
			ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetFIC, hReceiver);
			vl_appRequested = app;
		}
		else 
		{
			// we need to add the request for both application
			DABMWcmd_GetFIC[1] = DABMW_ALL_APPLICATIONS << 2;
			vl_appRequested = DABMW_ALL_APPLICATIONS;
		}
	}
	else
	{
		// it is a stop
		// 
		if ((DABMW_NONE_APP == vl_appRequested)
			||
			(app != vl_appRequested)
			)
		{
			// nothing to do, this is already stopped for application
			//
			retval = OSAL_OK;
		}
		else if (app == vl_appRequested)
		{
			DABMWcmd_GetFIC[1] = 0x00;
			vl_appRequested = DABMW_NONE_APP;
		}
		else
		{
			// both have been activated.
			// send an update with remaining app
			//
			if (DABMW_MAIN_AUDIO_DAB_APP == app)
			{
				// we need to add the request for both application			
				vl_appRequested = DABMW_SECONDARY_AUDIO_DAB_APP;
			}
			else
			{
				vl_appRequested = DABMW_MAIN_AUDIO_DAB_APP;
			}
			
			DABMWcmd_GetFIC[1] = vl_appRequested << 2;
		}			
	}
	
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetFIC, sizeof(DABMWcmd_GetFIC), &cstatus, FALSE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
	}

exit:
	return retval;
}
#endif // CONFIG_ETAL_HAVE_SYSTEMDATA_DAB || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_cmdGetServiceList_MDR
 *
 **************************/
tSInt ETAL_cmdGetServiceList_MDR(tU32 eid, tBool bGetAudioServices, tBool bGetDataServices, EtalServiceList *list, tBool *have_data)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp, *payload;
	tU8 DABMWcmd_GetServiceListDAB[] = {0x92, 0x00, 0x43, 0x03, 0x00, 0x00, 0x00};
	tSInt retval;
	tU32 list_size, i;

	*have_data = FALSE;

	ETAL_paramSetEID_MDR(DABMWcmd_GetServiceListDAB, eid);
	/*
	 * set command specific flags
	 */
	DABMWcmd_GetServiceListDAB[1] = ((tU8)(bGetAudioServices ? 1 : 0) << 6) | ((tU8)(bGetDataServices ? 1 : 0) << 7);
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetServiceListDAB, sizeof(DABMWcmd_GetServiceListDAB), &cstatus, TRUE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if (resplen > ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN)
	{
		payload = ETAL_getGenericPayload_MDR(resp);
		list_size = ETAL_getGenericPayloadLen_MDR(resp) / ETAL_DABMW_GET_SERVLIST_RECORD_SIZE;
		if (list_size > ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE)
		{
			/* truncate to supported size */
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "Unsupported number of service per ensemble (%d), max %d", list_size, ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE);
			list_size = ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE;
		}
		for (i = 0; i < list_size; i++)
		{
			list->m_service[i] = ETAL_DABMW_GET_SERVLIST_SID(payload + ETAL_DABMW_GET_SERVLIST_RECORD_SIZE * i);
		}
		list->m_serviceCount = list_size;
		*have_data = TRUE;
	}

	return OSAL_OK;
}
#endif // CONFIG_ETAL_HAVE_SYSTEMDATA || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_cmdGetSpecificServiceData_MDR
 *
 **************************/
tSInt ETAL_cmdGetSpecificServiceData_MDR(tU32 eid, tU32 sid, EtalServiceInfo *serv_info, EtalServiceComponentList *sclist, tBool *have_data)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	// TODO wrong! this hard-codes MAIN DAB APP, it should use hReceiver!
	tU8 DABMWcmd_GetSpecificServiceData[] = {0x92, 0x40, 0x44, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tSInt retval;

	*have_data = FALSE;

	ETAL_paramSetEID_MDR(DABMWcmd_GetSpecificServiceData, eid);
	ETAL_setSid_MDR(&DABMWcmd_GetSpecificServiceData[7], sid);
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetSpecificServiceData, sizeof(DABMWcmd_GetSpecificServiceData), &cstatus, TRUE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if (resplen > ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN)
	{
		ETAL_parseServiceInfo(resp, serv_info, sclist);
		*have_data = TRUE;
	}

	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetSpecificServiceDataExtended_MDR
 *
 **************************/
tSInt ETAL_cmdGetSpecificServiceDataExtended_MDR(ETAL_HANDLE hGeneric, tU32 eid, tU32 sid, EtalServiceInfo *serv_info, EtalServiceComponentExtendedList *sclist, tBool *have_data)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tU8 DABMWcmd_GetSpecificServiceData[] = {0x92, 0xC0, 0x44, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tSInt retval = OSAL_OK;
	ETAL_HANDLE hReceiver = ETAL_INVALID_HANDLE;

	// get hReceiver
	if (ETAL_handleIsReceiver(hGeneric))
	{
		hReceiver = hGeneric;
	}
	else if (ETAL_handleIsDatapath(hGeneric))
	{
		hReceiver = ETAL_receiverGetFromDatapath(hGeneric);
	}
	if (hReceiver == ETAL_INVALID_HANDLE)
	{
		retval = OSAL_ERROR_INVALID_PARAM;
	}

	if (retval == OSAL_OK)
	{
		*have_data = FALSE;

		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetSpecificServiceData, hReceiver);
		ETAL_paramSetEID_MDR(DABMWcmd_GetSpecificServiceData, eid);
		ETAL_setSid_MDR(&DABMWcmd_GetSpecificServiceData[7], sid);
		retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetSpecificServiceData, sizeof(DABMWcmd_GetSpecificServiceData), &cstatus, TRUE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
		else if (resplen > ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN)
		{
			retval = ETAL_parseServiceExtendedInfo(resp, resplen, serv_info, sclist);
			*have_data = TRUE;
		}
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdGetCurrentEnsemble_MDR
 *
 **************************/
tSInt ETAL_cmdGetCurrentEnsemble_MDR(ETAL_HANDLE hReceiver, tU32 *pUEId)
{
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tU8 DABMWcmd_GetCurrentEnsemble[] = {0x94, 0x00, 0x6D};
	tSInt retval;

	if (pUEId == NULL)
	{
		return OSAL_ERROR_INVALID_PARAM;
	}
	ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetCurrentEnsemble, hReceiver);
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetCurrentEnsemble, sizeof(DABMWcmd_GetCurrentEnsemble), &cstatus, FALSE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if (resplen != ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN + ETAL_DABMW_GET_ENSEMBLE_PAYLOAD)
	{
		return OSAL_ERROR;
	}

	ETAL_parseCurrentEnsemble(resp, pUEId);
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdServiceSelect_MDR
 *
 **************************/
/*
 * Returns <no_data>=TRUE only if the MDR response is NO_DATA
 */
tSInt ETAL_cmdServiceSelect_MDR(ETAL_HANDLE hDatapath, ETAL_HANDLE hReceiver, EtalServiceSelectMode mode, EtalServiceSelectSubFunction type, tU32 eid, tU32 sid, tU32 sc, tU32 subch, tBool *no_data)
{
	EtalBcastDataType data_type;

	if (hDatapath == ETAL_INVALID_HANDLE)
	{
		// audio service select: use the hReceiver passed by the caller
		data_type = ETAL_DATA_TYPE_AUDIO;
	}
	else
	{
		data_type = ETAL_receiverGetDataTypeForDatapath(hDatapath);
		hReceiver = ETAL_receiverGetFromDatapath(hDatapath);
	}

	if (OSAL_ERROR == ETAL_cmdServiceSelect_MDR_internal(hReceiver, mode, type, eid, sid, sc, subch, no_data, data_type))
	{
		return OSAL_ERROR;
	}

	/*
	 * If the command was successful for ETAL_DATA_TYPE_DAB_DATA_RAW we need additionally
	 * to store (or remove) the subch and the packet address of the selected service so that when
	 * the raw data packets will arrive the communication layer will know from the
	 * MDR packet header which callback to invoke
	 */
	if (((mode == ETAL_SERVSEL_MODE_SERVICE) || (mode == ETAL_SERVSEL_MODE_DAB_SC)) &&
		(data_type == ETAL_DATA_TYPE_DAB_DATA_RAW))
	{
		switch (type)
		{
			case ETAL_SERVSEL_SUBF_REMOVE:
				if (ETAL_removePacketAddressAndSubchIdForDatapath(hDatapath, eid, sid) != OSAL_OK)
				{
					return OSAL_ERROR;
				}
				break;
			case ETAL_SERVSEL_SUBF_SET:
				/*
				 * remove all Datapath entries before storing just this one
				 */
				ETAL_statusRemoveDataServiceInfoForDatapath(hDatapath);
				/* fall through */
			case ETAL_SERVSEL_SUBF_APPEND:
				if (ETAL_savePacketAddressAndSubchIdForDatapath(hDatapath, eid, sid) != OSAL_OK)
				{
					return OSAL_ERROR;
				}
				break;
			default:
				ASSERT_ON_DEBUGGING(0);
				return OSAL_ERROR;
		}
	}
	
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdServiceSelect_MDR_internal
 *
 **************************/
tSInt ETAL_cmdServiceSelect_MDR_internal(ETAL_HANDLE hReceiver, EtalServiceSelectMode mode, EtalServiceSelectSubFunction type, tU32 eid, tU32 sid, tU32 sc, tU32 subch, tBool *no_data, EtalBcastDataType dataType)
{
	                                        //  0     1     2     3     4     5     6     7     8     9    10    11    12
	tU8 DABMWcmd_ServiceSelectBySid[]  = {0x92, 0x00, 0x61, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU8 DABMWcmd_ServiceSelectBySC[]   = {0x92, 0x00, 0x61, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU8 DABMWcmd_ServiceSelectBySubc[] = {0x92, 0x00, 0x61, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU8 *cmd;
	tU32 cmdlen;
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tU8 routing = (tU8)0;
	tSInt retval = OSAL_OK;

	// infer the routing bits from the data type
	switch (dataType)
	{
		case ETAL_DATA_TYPE_DATA_SERVICE:
			// default (0) is fine
			break;
		case ETAL_DATA_TYPE_DAB_DATA_RAW:
		case ETAL_DATA_TYPE_DAB_AUDIO_RAW:
			routing = ETAL_DABMW_SERVICE_SELECT_ROUTING_DATA;
			break;
		case ETAL_DATA_TYPE_AUDIO:
			routing = ETAL_DABMW_SERVICE_SELECT_ROUTING_AUDIO;
			break;
		default:
			break;
	}

	switch (mode)
	{
		case ETAL_SERVSEL_MODE_SERVICE:
			cmd = DABMWcmd_ServiceSelectBySid;
			cmdlen = sizeof(DABMWcmd_ServiceSelectBySid);
			ETAL_utilitySetU32(cmd, 8, sid);
			break;

		case ETAL_SERVSEL_MODE_DAB_SC:
			cmd = DABMWcmd_ServiceSelectBySC;
			cmdlen = sizeof(DABMWcmd_ServiceSelectBySC);
			ETAL_utilitySetU32(cmd, 8, sid);
			ETAL_utilitySetU8(cmd, 12, (tU8)sc);
			break;

		case ETAL_SERVSEL_MODE_DAB_SUBCH:
			cmd = DABMWcmd_ServiceSelectBySubc;
			cmdlen = sizeof(DABMWcmd_ServiceSelectBySubc);
			ETAL_utilitySetU8(cmd, 8, (tU8)subch);
			break;
		default:
			ASSERT_ON_DEBUGGING(0);
			return OSAL_ERROR;
	}
	// WARNING: the function call modifies cmd[1], so do not change the order of the next two lines!
	cmd[1] = (tU8)mode << 6;
	ETAL_paramSetApplicationFromReceiver_MDR(cmd, hReceiver);
	// end of WARNING
	cmd[4] = (tU8)type | routing;
	ETAL_utilitySetU24(cmd, 5, eid);

	*no_data = FALSE;
	retval = ETAL_sendCommandTo_MDR(cmd, cmdlen, &cstatus, TRUE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if (resplen != ETAL_DABMW_RESPONSE_NO_PAYLOAD_LEN)
	{
		return OSAL_ERROR;
	}
	else if (resp[1] != (tU8)0)
	{
		if (ETAL_MDR_NOTIFICATION_STATUS(resp) == DABMW_CMD_STATUS_RSP_NO_DATA_AVAILABLE)
		{
			*no_data = TRUE;
		}
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetPAD_MDR
 *
 **************************/
/*
 * Returns <no_data>=TRUE only if the MDR response is NO_DATA
 */
tSInt ETAL_cmdGetPAD_MDR(ETAL_HANDLE hReceiver, etalPADDLSTy *paddata, tBool *no_data)
{
	tU8 DABMWcmd_GetPAD[]  = {0x90, 0x00, 0x51};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp, app;
	tSInt retval = OSAL_OK;

	*no_data = FALSE;
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetPAD, sizeof(DABMWcmd_GetPAD), &cstatus, TRUE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if ((resplen > 1) && (resp[1] != (tU8)0))
	{
		if (ETAL_MDR_NOTIFICATION_STATUS(resp) == DABMW_CMD_STATUS_RSP_NO_DATA_AVAILABLE)
		{
			*no_data = TRUE;
		}
		return OSAL_ERROR;
	}
	return ETAL_extractPADDLS_MDR(resp, resplen, paddata, &app);
}

/***************************
 *
 * ETAL_cmdGetPADDLSEvent_MDR
 *
 **************************/
tSInt ETAL_cmdGetPADDLSEvent_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd)
{
	tU8 DABMWcmd_GetPADevent[]  = {0xB0, 0x00, 0x51};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	if (cmd == cmdActionStop)
	{
		DABMWcmd_GetPADevent[0] |= 1;
	}
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetPADevent, sizeof(DABMWcmd_GetPADevent), &cstatus, FALSE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if ((resplen > 1) && (resp[1] != (tU8)0))
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetPADDLPlusevent_MDR
 *
 **************************/
tSInt ETAL_cmdGetPADDLPlusEvent_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd)
{
	tU8 DABMWcmd_GetPADevent[]  = {0xB0, 0x00, 0x53};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	if (cmd == cmdActionStop)
	{
		DABMWcmd_GetPADevent[0] |= 1;
	}
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetPADevent, sizeof(DABMWcmd_GetPADevent), &cstatus, FALSE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if ((resplen > 1) && (resp[1] != (tU8)0))
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdSetPADDLPlusevent_MDR
 *
 **************************/
tSInt ETAL_cmdSetPADDLPlusEvent_MDR(ETAL_HANDLE hReceiver)
{
	tU8 DABMWcmd_SetPADevent[]  = {0x96, 0x00, 0x52, 0X01, 0x01};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	retval = ETAL_sendCommandTo_MDR(DABMWcmd_SetPADevent, sizeof(DABMWcmd_SetPADevent), &cstatus, FALSE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if ((resplen > 1) && (resp[1] != (tU8)0))
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetTPEGRAW_MDR
 *
 **************************/
tSInt ETAL_cmdGetTPEGRAW_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd)
{
	tU8 DABMWcmd_GetTPEGRAW[]  = {0x94, 0x01, 0x00}; // lower command id byte set below
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetTPEGRAW, hReceiver);
	if (cmd == cmdActionStop)
	{
		DABMWcmd_GetTPEGRAW[2] = (tU8)0x03;
	}
	else
	{
		DABMWcmd_GetTPEGRAW[2] = (tU8)0x02;
	}
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetTPEGRAW, sizeof(DABMWcmd_GetTPEGRAW), &cstatus, FALSE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetTPEGSNI_MDR
 *
 **************************/
tSInt ETAL_cmdGetTPEGSNI_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd)
{
	tU8 DABMWcmd_GetTPEGSNI[]  = {0x94, 0x01, 0x00}; // lower command id byte set below
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetTPEGSNI, hReceiver);
	if (cmd == cmdActionStop)
	{
		DABMWcmd_GetTPEGSNI[2] = (tU8)0x05;
	}
	else
	{
		DABMWcmd_GetTPEGSNI[2] = (tU8)0x04;
	}
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetTPEGSNI, sizeof(DABMWcmd_GetTPEGSNI), &cstatus, FALSE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetEPGBIN_MDR
 *
 **************************/
tSInt ETAL_cmdGetEPGBIN_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd)
{
	tU8 DABMWcmd_GetEPGBIN[]  = {0x94, 0x01, 0x10};
	tU8 DABMWcmd_StopEPGBIN[]  = {0x96, 0x01, 0x11, 0x01, 0x01};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	if (cmd == cmdActionStart)
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetEPGBIN, hReceiver);
		retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetEPGBIN, sizeof(DABMWcmd_GetEPGBIN), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	else
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_StopEPGBIN, hReceiver);
		retval = ETAL_sendCommandTo_MDR(DABMWcmd_StopEPGBIN, sizeof(DABMWcmd_StopEPGBIN), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetEPGSRV_MDR
 *
 **************************/
tSInt ETAL_cmdGetEPGSRV_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd, tU8 ecc, tU16 eid)
{
	tU8 DABMWcmd_GetEPGSRV[]  = {0x96, 0x01, 0x12, 0x03, 0x00, 0x00, 0x00}; //last 3 bytes : ECC and EID to be filled below
	tU8 DABMWcmd_StopEPGSRV[]  = {0x96, 0x01, 0x11, 0x01, 0x02};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	if (cmd == cmdActionStart)
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetEPGSRV, hReceiver);

		DABMWcmd_GetEPGSRV[4] = ecc;
		DABMWcmd_GetEPGSRV[5] = (tU8)((eid >> 8) & 0xFF);
		DABMWcmd_GetEPGSRV[6] = (tU8)((eid >> 0) & 0xFF);

		retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetEPGSRV, sizeof(DABMWcmd_GetEPGSRV), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	else
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_StopEPGSRV, hReceiver);
		retval = ETAL_sendCommandTo_MDR(DABMWcmd_StopEPGSRV, sizeof(DABMWcmd_StopEPGSRV), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetEPGPRG_MDR
 *
 **************************/
tSInt ETAL_cmdGetEPGPRG_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd, tU8 ecc, tU16 eid, tU32 sid)
{
	tU8 DABMWcmd_GetEPGPRG[]  = {0x96, 0x01, 0x13, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU8 DABMWcmd_StopEPGPRG[]  = {0x96, 0x01, 0x11, 0x01, 0x04};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	if (cmd == cmdActionStart)
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetEPGPRG, hReceiver);

		DABMWcmd_GetEPGPRG[4]  = ecc;
		DABMWcmd_GetEPGPRG[5]  = (tU8)((eid >> 8) & 0xFF);
		DABMWcmd_GetEPGPRG[6]  = (tU8)((eid >> 0) & 0xFF);
		DABMWcmd_GetEPGPRG[7]  = (tU8)((sid >> 24) & 0xFF);
		DABMWcmd_GetEPGPRG[8]  = (tU8)((sid >> 16) & 0xFF);
		DABMWcmd_GetEPGPRG[9]  = (tU8)((sid >> 8) & 0xFF);
		DABMWcmd_GetEPGPRG[10] = (tU8)((sid >> 0) & 0xFF);

		retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetEPGPRG, sizeof(DABMWcmd_GetEPGPRG), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	else
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_StopEPGPRG, hReceiver);
		retval = ETAL_sendCommandTo_MDR(DABMWcmd_StopEPGPRG, sizeof(DABMWcmd_StopEPGPRG), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetEPGLogo_MDR
 *
 **************************/
tSInt ETAL_cmdGetEPGLogo_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd, tU8 ecc, tU16 eid, tU32 sid, tU8 logoType)
{
	tU8 DABMWcmd_GetEPGLogo[]  = {0x96, 0x01, 0x14, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU8 DABMWcmd_StopEPGLogo[]  = {0x96, 0x01, 0x11, 0x01, 0x08};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	if (cmd == cmdActionStart)
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetEPGLogo, hReceiver);

		DABMWcmd_GetEPGLogo[4]  = ecc;
		DABMWcmd_GetEPGLogo[5]  = (tU8)((eid >> 8) & 0xFF);
		DABMWcmd_GetEPGLogo[6]  = (tU8)((eid >> 0) & 0xFF);
		DABMWcmd_GetEPGLogo[7]  = (tU8)((sid >> 24) & 0xFF);
		DABMWcmd_GetEPGLogo[8]  = (tU8)((sid >> 16) & 0xFF);
		DABMWcmd_GetEPGLogo[9]  = (tU8)((sid >> 8) & 0xFF);
		DABMWcmd_GetEPGLogo[10] = (tU8)((sid >> 0) & 0xFF);
		DABMWcmd_GetEPGLogo[11] = logoType;

		retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetEPGLogo, sizeof(DABMWcmd_GetEPGLogo), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	else
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_StopEPGLogo, hReceiver);
		retval = ETAL_sendCommandTo_MDR(DABMWcmd_StopEPGLogo, sizeof(DABMWcmd_StopEPGLogo), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetJMLOBJ_MDR
 *
 **************************/
tSInt ETAL_cmdGetJMLOBJ_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd, tU16 objectId)
{
	tU8 DABMWcmd_GetJMLOBJ[]  = {0x96, 0x01, 0x20, 0x02, 0x00, 0x00}; // last two bytes is JML Object ID, set below
	tU8 DABMWcmd_StopJMLUpdate[]  = {0x94, 0x01, 0x21};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	if (cmd == cmdActionStart)
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_GetJMLOBJ, hReceiver);

		DABMWcmd_GetJMLOBJ[4] = (tU8)((objectId >> 8) & 0xFF);
		DABMWcmd_GetJMLOBJ[5] = (tU8)((objectId >> 0) & 0xFF);

		retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetJMLOBJ, sizeof(DABMWcmd_GetJMLOBJ), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	else
	{
		ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_StopJMLUpdate, hReceiver);
		retval = ETAL_sendCommandTo_MDR(DABMWcmd_StopJMLUpdate, sizeof(DABMWcmd_StopJMLUpdate), &cstatus, FALSE, &resp, &resplen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}


/***************************
 *
 * ETAL_cmdLearn_MDR
 *
 **************************/
tSInt ETAL_cmdLearn_MDR(ETAL_HANDLE hReceiver, EtalFrequencyBand bandIndex, etalCmdActionTy cmd)
{
	tU8 DABMWcmd_Learn[]  = {0x92, 0x00, 0x64, 0x01, 0x01};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;
	tBool has_response = FALSE;

	ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_Learn, hReceiver);
	ETAL_paramSetLearnBand_MDR(DABMWcmd_Learn, bandIndex);
	if (cmd == cmdActionStop)
	{
		DABMWcmd_Learn[4] = 0x80;  // set the stop bit in the payload
		DABMWcmd_Learn[1] |= 0xC0; // unused but MDR returns an error if not set to some valid value
		has_response = TRUE;
	}

	/*
	 * The 'learn start' command in reality does have a response but it normally arrives
	 * much after the ETAL communication timeout so we have to force the
	 * communication layer to ignore it and treat it specially.
	 * On the contrary the 'learn stop' has an immediate response
	 * and we want to process it here to close the learn FSM
	 */
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_Learn, sizeof(DABMWcmd_Learn), &cstatus, has_response, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if (has_response)
	{
		if ((resplen > 0) && (resp[1] != (tU8)0))
		{
			retval = OSAL_ERROR;
		}
		/*
		 * Additional processing for the stop command
		 */
		ETAL_invokeEventCallback_MDR(resp,  resplen);
		ETAL_receiverSetSpecial(hReceiver, cmdSpecialLearn, cmdActionStop);
	}
	return retval;
}

/***************************
 *
 * ETAL_cmdSeamlessEstimation_MDR
 *
 **************************/
tSInt ETAL_cmdSeamlessEstimation_MDR(etalSeamlessEstimationConfigTy *seamlessEstimationConfig)
{
	tU8 DABMWcmd_SeamlessEstimation[]  = {0x92, 0x00, 0x6F, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;
	tBool has_response = FALSE;

	ETAL_setSeamlessEstimationConfig_MDR(DABMWcmd_SeamlessEstimation, seamlessEstimationConfig);

	/*
	 * The 'seamless estimation' command in reality has a response but it normally arrives
	 * much after the ETAL communication timeout so we have to force the
	 * communication layer to ignore it and treat it specially.
	 */
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_SeamlessEstimation, sizeof(DABMWcmd_SeamlessEstimation), &cstatus, has_response, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		retval = OSAL_ERROR;
	}
	return retval;
}

/***************************
 *
 * ETAL_cmdSeamlessSwitching_MDR
 *
 **************************/
tSInt ETAL_cmdSeamlessSwitching_MDR(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig)
{
	tU8 DABMWcmd_SeamlessSwitching[]  = {0x92, 0x00,	0x66, 0x1A,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;
	tBool has_response = FALSE;

	ETAL_setSeamlessSwitchingConfig_MDR(DABMWcmd_SeamlessSwitching, seamlessSwitchingConfig);

	/*
	 * The 'seamless estimation' command in reality has a response but it normally arrives
	 * much after the ETAL communication timeout so we have to force the
	 * communication layer to ignore it and treat it specially.
	 */
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_SeamlessSwitching, sizeof(DABMWcmd_SeamlessSwitching), &cstatus, has_response, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		retval = OSAL_ERROR;
	}
	return retval;
}

/***************************
 *
 * ETAL_cmdSelectAudioOutput_MDR
 *
 **************************/
tSInt ETAL_cmdSelectAudioOutput_MDR(ETAL_HANDLE hReceiver)
{
	tU8 DABMWcmd_SelectAudioOutput[]  = {0x90, 0x00, 0x65};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_SelectAudioOutput, hReceiver);
	retval = ETAL_sendCommandTo_MDR(DABMWcmd_SelectAudioOutput, sizeof(DABMWcmd_SelectAudioOutput), &cstatus, TRUE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if ((resplen > 0) && (resp[1] != (tU8)0))
	{
		retval = OSAL_ERROR;
	}
	return retval;
}

#if defined (CONFIG_ETAL_HAVE_AUDIO_CONTROL) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_cmdMute_MDR
 *
 **************************/
tSInt ETAL_cmdMute_MDR(ETAL_HANDLE hReceiver, tBool muteFlag)
{
	tU8 DABMWcmd_Mute[]  = {0x94, 0x00, 0x82};
	tU16 cstatus = 0;
	tSInt retval = OSAL_OK;

	ETAL_paramSetApplicationFromReceiver_MDR(DABMWcmd_Mute, hReceiver);

	DABMWcmd_Mute[1] |= ((muteFlag == TRUE) ? 0x40 : 0x80);

	retval = ETAL_sendCommandTo_MDR(DABMWcmd_Mute, sizeof(DABMWcmd_Mute), &cstatus, FALSE, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, NULL, 0);
		return OSAL_ERROR;
	}
	return retval;
}
#endif // CONFIG_ETAL_HAVE_AUDIO_CONTROL || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_HAVE_DATASERVICES) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_paramSetMOTApplication_MDR
 *
 **************************/
static tVoid ETAL_paramSetMOTApplication_MDR(tU8 *cmd, etalMOTserviceTy service)
{
	tU8 param;

	switch (service)
	{
		case etalMOTServiceEPGraw:
		case etalMOTServiceSLS:
		case etalMOTServiceSLSoverXPAD:
			param = (tU8)service;
			break;

		default:
			ASSERT_ON_DEBUGGING(0);
			param = 0;
			break;
	}
	cmd[7] = param;	
}

/***************************
 *
 * ETAL_cmdStartStopMOT
 *
 **************************/
tSInt ETAL_cmdStartStopMOT(ETAL_HANDLE hReceiver, etalMOTserviceTy service, etalCmdActionTy action)
{
	tU8 DABMWcmd_GetMOT[]  =  {0x96, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
	tU8 DABMWcmd_StopMOT[]  = {0x96, 0x01, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00};
	tU8 *cmd;
	tU32 cmdlen;
	tU16 cstatus = 0;
	tSInt retval = OSAL_OK;

	if (action == cmdActionStart)
	{
		cmd = DABMWcmd_GetMOT;
		cmdlen = sizeof(DABMWcmd_GetMOT);
	}
	else
	{
		cmd = DABMWcmd_StopMOT;
		cmdlen = sizeof(DABMWcmd_StopMOT);
	}
	ETAL_paramSetApplicationFromReceiver_MDR(cmd, hReceiver);
	ETAL_paramSetMOTApplication_MDR(cmd, service);

	retval = ETAL_sendCommandTo_MDR(cmd, cmdlen, &cstatus, FALSE, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, NULL, 0);
		return OSAL_ERROR;
	}
	return retval;
}
#endif // CONFIG_ETAL_HAVE_DATASERVICES || CONFIG_ETAL_HAVE_ALL_API

/***************************
 * ETAL_paramSetEventNoficationBitmap
 *
 **************************/
static tVoid ETAL_paramSetEventNoficationBitmap(tU8 *cmd, tU16 val)
{
	*(cmd + ETAL_MDR_EVENTNOTIFICATION_BITMAP_OFFSET + 0) = (tU8)((val >> 8) & 0xFF);
	*(cmd + ETAL_MDR_EVENTNOTIFICATION_BITMAP_OFFSET + 1) = (tU8)((val >> 0) & 0xFF);
}

/***************************
 *
 * ETAL_cmdSetupEventNotification_MDR
 *
 **************************/
tSInt ETAL_cmdSetupEventNotification_MDR(ETAL_HANDLE hReceiver, tU16 eventBitMap)
{
	tU8 DABMWcmd_SetupEventNotification[]  = {0x96, 0x00, 0x10, 0x02, 0x00, 0x00}; // last byte may be changed below
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt retval = OSAL_OK;

	// DAB status has to be always enabled to keep INFO tune event enabled
	// Only DAB status event can be enabled or disabled.
	eventBitMap |= ETAL_AUTONOTIF_TYPE_DAB_STATUS;

	ETAL_paramSetEventNoficationBitmap(DABMWcmd_SetupEventNotification, eventBitMap);

	retval = ETAL_sendCommandTo_MDR(DABMWcmd_SetupEventNotification, sizeof(DABMWcmd_SetupEventNotification), &cstatus, FALSE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	else if ((resplen > 1) && (resp[1] != (tU8)0))
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_cmdGetFirmwareVersion_MDR
 *
 **************************/
tSInt ETAL_cmdGetFirmwareVersion_MDR(tU32 *year, tU32 *month, tU32 *day, tU32 *major, tU32 *minor, tU32 *internal, tU32 *build)
{
	tU8 DABMWcmd_SetupEventNotification[]  = {0x94, 0x00, 0x02};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tU8 *payload;
	tSInt retval;

	retval = ETAL_sendCommandTo_MDR(DABMWcmd_SetupEventNotification, sizeof(DABMWcmd_SetupEventNotification), &cstatus, FALSE, &resp, &resplen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		return OSAL_ERROR;
	}
	else if ((resplen != ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN + ETAL_DABMW_GET_VERSION_PAYLOAD) || (resp[1] != (tU8)0))
	{
		return OSAL_ERROR;
	}
	else
	{
		payload = ETAL_getGenericPayload_MDR(resp);
		*year = ETAL_DABMW_GET_VERSION_YEAR(payload);
		*month = ETAL_DABMW_GET_VERSION_MONTH(payload);
		*day = ETAL_DABMW_GET_VERSION_DAY(payload);
		*major = ETAL_DABMW_GET_VERSION_MAJOR(payload);
		*minor = ETAL_DABMW_GET_VERSION_MINOR(payload);
		*internal = ETAL_DABMW_GET_VERSION_INTERNAL(payload);
		*build = ETAL_DABMW_GET_VERSION_BUILD(payload);
	}
	return OSAL_OK;

}

/***************************
 *
 * ETAL_cmdGetTime_MDR
 *
 **************************/
tSInt ETAL_cmdGetTime_MDR(tBool getTimeWithLTO, EtalTime *DABTime)
{
	tU8 DABMWcmd_GetTime[]  = {0x94, 0x00, 0x45};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tU8 *payload;
	tSInt retval = OSAL_ERROR;

	if(DABTime != NULL)
	{
		/* Initialize DABTime */
		DABTime->m_isTimeValid  = 0;
		DABTime->m_mjd          = 0;
		DABTime->m_hours        = 0;
		DABTime->m_minutes      = 0;
		DABTime->m_seconds      = 0;
		DABTime->m_milliseconds = 0;
		DABTime->m_isLtoValid   = 0;
		DABTime->m_lto          = 0;

		DABMWcmd_GetTime[1] = (getTimeWithLTO == TRUE) ? 0x80 : 0x00;

		retval = ETAL_sendCommandTo_MDR(DABMWcmd_GetTime, sizeof(DABMWcmd_GetTime), &cstatus, FALSE, &resp, &resplen);
		if (retval == OSAL_OK)
		{
			if (((resplen == ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN + ETAL_DABMW_GET_TIME_WITHOUT_LTO_PAYLOAD) ||
				 (resplen == ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN + ETAL_DABMW_GET_TIME_WITH_LTO_PAYLOAD)) &&
				(resp[1] == (tU8)0))
			{
				payload = resp + 4;

				DABTime->m_isTimeValid = ETAL_utilityGetU8(payload, 0) & 0x01;
				DABTime->m_mjd = ETAL_utilityGetU24(payload, 1);
				DABTime->m_hours = ETAL_utilityGetU8(payload, 4);
				DABTime->m_minutes = ETAL_utilityGetU8(payload, 5);
				DABTime->m_seconds = ETAL_utilityGetU8(payload, 6);
				DABTime->m_milliseconds = ETAL_utilityGetU16(payload, 7);

				if(resplen == ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN + ETAL_DABMW_GET_TIME_WITH_LTO_PAYLOAD)
				{
					DABTime->m_isLtoValid = TRUE;
					DABTime->m_lto = ETAL_utilityGetU8(payload, 9) & 0x3F;
				}
				retval = OSAL_OK;
			}
			else
			{
				ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
			}
		}
		else
		{
			ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, ETAL_commandStatusToEventErrStatus_MDR(cstatus), cstatus, resp, resplen);
		}
	}
	return retval;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

