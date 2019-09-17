//!
//!  \file 		etaltmlapi_radiotext.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, Radio text access
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)

#include "dabmw_import.h"
#include "rds_data.h"
#include "rds_strategy.h"
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
	#include "rds_landscape.h"
#endif
#include "common_trace.h"

/***************************
 *
 * Defines
 *
 **************************/
#define ETAL_GET_DECODED_RDS_PTY(_buf_) ((_buf_) & 0x1F)   
#define ETAL_GET_DECODED_RDS_TP(_buf_) (((_buf_) >> 5) & 0x01)   
#define ETAL_GET_DECODED_RDS_TA(_buf_) (((_buf_) >> 6) & 0x01)   
#define ETAL_GET_DECODED_RDS_MS(_buf_) (((_buf_) >> 7) & 0x01)   

/***************************
 *
 * Variables
 *
 **************************/
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
static EtalRDSData          RDSdataStorage;
#endif

static EtalTextInfo         RadioTextStorage;
static tU32                 RDSServiceListTable[ETAL_MAX_RECEIVERS];

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
static ETAL_HANDLE radioTextReceiver[ETAL_MAX_RECEIVERS];
#endif
/***************************
 *
 * ETALTML_statusGetRadiotextStorage
 *
 **************************/
static tVoid ETALTML_statusGetRadiotextStorage(EtalRDSData **rds_storage, EtalTextInfo **radio_text_storage)
{
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
	*rds_storage = &RDSdataStorage;
#else
	*rds_storage = NULL;
#endif
	*radio_text_storage = &RadioTextStorage;
}

/***************************
 *
 * ETALTML_isValidGenericRadiotextReceiver
 *
 **************************/
static tBool ETALTML_isValidGenericRadiotextReceiver(ETAL_HANDLE hReceiver, EtalBcastStandard *std)
{
	*std = ETAL_receiverGetStandard(hReceiver);

	switch (*std)
	{
		case ETAL_BCAST_STD_DAB:
		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:	
			return TRUE;

		default:
			break;
	}
	return FALSE;
}

/***************************
 *
 * ETALTML_RDSresetData
 *
 **************************/
/*
 * RDS decoder maintains memory of which data it transferred
 * to the caller and does not transfer twice the same data.
 * This is a problem in case of reconfiguration of the
 * receiver because even after the new config the RDS may not
 * be sent. To avoid, reset RDS data before destroying
 * the receiver
 */
tVoid ETALTML_RDSresetData(ETAL_HANDLE hReceiver)
{
	tSInt rds_slot;

	if (!ETAL_receiverIsValidRDSHandle(hReceiver))
	{
		return;
	}

	rds_slot = ETAL_receiverGetRDSSlot(hReceiver);
	if (rds_slot >= 0)
	{
		/*
		 * not really a new frequency, but this is the way to reset all data
		 */
		DABMW_RdsDataEvent((DABMW_storageSourceTy)hReceiver, DABMW_EVENT_FREQUENCY_TUNED);
	}
}


/***************************
 *
 * ETALTML_RDSInit
 *
 **************************/
/*
 * Init the parameters and configuration of the RDS
 */
tVoid ETALTML_RDSInit()
{
	DABMW_RdsDataInit();

}

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
/***************************
 *
 * ETALTML_RDSgetDecodedBlock
 *
 **************************/
tVoid ETALTML_RDSgetDecodedBlock(ETAL_HANDLE hReceiver, EtalRDSData *pRDSdata, tBool forcedGet)
{
	tSInt slot_s;
	tVoid *ptr;
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
    tU32 piVal = DABMW_INVALID_RDS_PI_VALUE;
    tU32 baseFreq = DABMW_INVALID_FREQUENCY;
    tBool mode = DABMW_GET_AF_AUTODETECTMODE;
	tS32 af_temp;
#endif

	if (pRDSdata == NULL)
	{
		// may happen if STAR is not compiled in
		return;
	}

	OSAL_pvMemorySet((tVoid *)pRDSdata, 0x00, sizeof(EtalRDSData));

	slot_s = ETAL_receiverGetRDSSlot(hReceiver);
	if (slot_s == -1)
	{
		return;
	}

	// PI
	if (DABMW_RdsGetPi(slot_s, &ptr, forcedGet) != 0)
	{
		pRDSdata->m_PI = (tU16)(*((tU32 *)ptr)); 
		pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_PI;
	}

	// PS
	if (DABMW_RdsGetPs(slot_s, &ptr, forcedGet) != 0)
	{
		OSAL_pvMemoryCopy((tVoid *)pRDSdata->m_PS, (tPCVoid)ptr, ETAL_DEF_MAX_PS_LEN);
		pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_PS;
	}

	// DI
	if (DABMW_RdsGetDi(slot_s, &pRDSdata->m_DI, forcedGet) != 0)
	{
		pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_DI;
	}

	// TIME, offset, MJD
	if (DABMW_RdsGetTime(slot_s, &ptr, forcedGet) != 0)
	{
		pRDSdata->m_timeHour =     *((tU8 *)ptr + 0);
		pRDSdata->m_timeMinutes =  *((tU8 *)ptr + 1);
		pRDSdata->m_offset =       *((tU8 *)ptr + 2);
		pRDSdata->m_MJD =          *((tU32 *)ptr + 3);
		pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_TOM;
	}

	// RT
	if (DABMW_RdsGetRt(slot_s, (tU8 *)pRDSdata->m_RT, forcedGet) != 0)
	{
		pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_RT;
	}

	// PTY, TP, TA, MS
	if (DABMW_RdsGetPtyTpTaMs(slot_s, &ptr, forcedGet) != 0)
	{
		pRDSdata->m_PTY = ETAL_GET_DECODED_RDS_PTY(*((tU8*)ptr + 0));
		pRDSdata->m_TP =  ETAL_GET_DECODED_RDS_TP(*((tU8*)ptr + 0));
		pRDSdata->m_TA =  ETAL_GET_DECODED_RDS_TA(*((tU8*)ptr + 0));
		pRDSdata->m_MS =  ETAL_GET_DECODED_RDS_MS(*((tU8*)ptr + 0));
        if (ETAL_GET_DECODED_RDS_PTY(*((tU8*)ptr + 1)) == (tU8)0x1F)
        {
		    pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_PTY;
        }
        if (ETAL_GET_DECODED_RDS_TP(*((tU8*)ptr + 1)) == (tU8)0x01)
        {
		    pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_TP;
        }
        if (ETAL_GET_DECODED_RDS_TA(*((tU8*)ptr + 1)) == (tU8)0x01)
        {
		    pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_TA;
        }
        if (ETAL_GET_DECODED_RDS_MS(*((tU8*)ptr + 1)) == (tU8)0x01)
        {
		    pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_MS;
        }
	}

	// AFList, AFListPI, AFListLen
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
	if (DABMW_RdsCheckPi(slot_s, (tPU32)&piVal) != 0)
	{
		af_temp = DABMW_RdsGetAfList(hReceiver, (tU8*)&pRDSdata->m_AFList, forcedGet, piVal, baseFreq, mode, DABMW_AF_LIST_BFR_LEN + 4);
		if (af_temp > 0)
		{
			pRDSdata->m_AFListLen = (tU32)af_temp;
			pRDSdata->m_AFListPI = piVal;
			pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_AF;
		}
		else
		{
			pRDSdata->m_AFListLen = 0;
		}
	}
#endif

}

/***************************
 *
 * ETALTML_RDSdecodeBlock
 *
 **************************/
static tVoid ETALTML_RDSdecodeBlock(ETAL_HANDLE hReceiver, EtalRDSData *pRDSdata, tU8 *buf, tU32 len)
{
	tSInt slot_s;
	tVoid *ptr;
	tU32 data;
	tU32 errc;
	tU32 count;
	tU32 vl_frequency;
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
    tU32 piVal = DABMW_INVALID_RDS_PI_VALUE;
    tU32 baseFreq = DABMW_INVALID_FREQUENCY;
    tBool mode = DABMW_GET_AF_AUTODETECTMODE;
	tBool forcedGet = FALSE;
	tS32 af_temp;
#endif

	if (pRDSdata == NULL)
	{
		// may happen if STAR is not compiled in
		return;
	}

	OSAL_pvMemorySet((tVoid *)pRDSdata, 0x00, sizeof(EtalRDSData));

	slot_s = ETAL_receiverGetRDSSlot(hReceiver);
	if (slot_s == -1)
	{
		return;
	}

	if (len >= 9)
    {   
        // skip first 3 bytes, it's the Read Notification Register
    	count = 3;
    	while (count < len - 3) // last three bytes are the checksum, skip
    	{
    		data  = (tU32)buf[count + 0] << 16;
    		data |= (tU32)buf[count + 1] <<  8;
    		data |= (tU32)buf[count + 2] <<  0;
    		errc = ((tU32)buf[count + 0] >> 4) & 0x07;

			/*
			Error counter. This field contains the number of corrected errors in the block.
			000: no error detected
			001: single error detected and corrected
			010: double error detected and corrected
			110: uncorrectable
			111: unavailable
			*/
			if (errc <= 2)
			{
				DABMW_RdsMain((DABMW_storageSourceTy)hReceiver, data, errc);
			}
			else
			{
				// error detected 
				
				ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "ETALTML_RDSdecodeBlock: hReceiver %x, errc >2  = %d", hReceiver, errc);
			}
			count += 3;
		}
	}

	// If the frequency is not valid, 
	// Just bypass
	// because the values might be temporary values...
	// Freq is not yet updated
	vl_frequency = ETAL_receiverGetFrequency(hReceiver);

	if (ETAL_INVALID_FREQUENCY != vl_frequency)
	{
		if (DABMW_RdsGetPi(slot_s, &ptr, FALSE) != 0)
		{
			pRDSdata->m_PI = (tU16)(*((tU32 *)ptr)); 
			pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_PI;
			pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_PI;
		}
		else
		{
			// update also the stored information bitmap
			// to provide back the info if available
			if (DABMW_RdsGetPi(slot_s, &ptr, TRUE) != 0)
			{
				pRDSdata->m_PI = (tU16)(*((tU32 *)ptr)); 
				pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_PI;
			}
		}

		// PS
		if (DABMW_RdsGetPs(slot_s, &ptr, FALSE) != 0)
		{
			OSAL_pvMemoryCopy((tVoid *)pRDSdata->m_PS, (tPCVoid)ptr, ETAL_DEF_MAX_PS_LEN);
			pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_PS;
			pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_PS;
		}
		else
		{
			// update also the stored information bitmap
			// to provide back the info if available
			if (DABMW_RdsGetPs(slot_s, &ptr, TRUE) != 0)
			{
				OSAL_pvMemoryCopy((tVoid *)pRDSdata->m_PS, (tPCVoid)ptr, ETAL_DEF_MAX_PS_LEN);
				pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_PS;
			}
		}
		
		// DI
		if (DABMW_RdsGetDi(slot_s, &pRDSdata->m_DI, FALSE) != 0)
		{
			pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_DI;
			pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_DI;
		}
		else
		{
			if (DABMW_RdsGetDi(slot_s, &pRDSdata->m_DI, TRUE) != 0)
			{
				pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_DI;
			}
		}
		
		// TIME, offset, MJD
		if (DABMW_RdsGetTime(slot_s, &ptr, FALSE) != 0)
		{
			pRDSdata->m_timeHour =     *((tU8 *)ptr + 0);
			pRDSdata->m_timeMinutes =  *((tU8 *)ptr + 1);
			pRDSdata->m_offset =       *((tU8 *)ptr + 2);
			pRDSdata->m_MJD =          *((tU32 *)ptr + 3);
			pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_TOM;
			pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_TOM;
		}
		else
		{
			if (DABMW_RdsGetTime(slot_s, &ptr, TRUE) != 0)
			{
				pRDSdata->m_timeHour =     *((tU8 *)ptr + 0);
				pRDSdata->m_timeMinutes =  *((tU8 *)ptr + 1);
				pRDSdata->m_offset =       *((tU8 *)ptr + 2);
				pRDSdata->m_MJD =          *((tU32 *)ptr + 3);
				pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_TOM;
			}
		}
		
		// RT
		if (DABMW_RdsGetRt(slot_s, (tU8 *)pRDSdata->m_RT, FALSE) != 0)
		{
			pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_RT;
			pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_RT;
		}
		else
		{
			if (DABMW_RdsGetRt(slot_s, (tU8 *)pRDSdata->m_RT, TRUE) != 0)
			{
				pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_RT;
			}
		}
		
		// PTY, TP, TA, MS
		if (DABMW_RdsGetPtyTpTaMs(slot_s, &ptr, FALSE) != 0)
		{
			pRDSdata->m_PTY = ETAL_GET_DECODED_RDS_PTY(*((tU8*)ptr + 0));
			pRDSdata->m_TP =  ETAL_GET_DECODED_RDS_TP(*((tU8*)ptr + 0));
			pRDSdata->m_TA =  ETAL_GET_DECODED_RDS_TA(*((tU8*)ptr + 0));
			pRDSdata->m_MS =  ETAL_GET_DECODED_RDS_MS(*((tU8*)ptr + 0));
	        if (ETAL_GET_DECODED_RDS_PTY(*((tU8*)ptr + 1)) == (tU8)0x1F)
	        {
			    pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_PTY;
	        }
	        if (ETAL_GET_DECODED_RDS_TP(*((tU8*)ptr + 1)) == (tU8)0x01)
	        {
			    pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_TP;
	        }
	        if (ETAL_GET_DECODED_RDS_TA(*((tU8*)ptr + 1)) == (tU8)0x01)
	        {
			    pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_TA;
	        }
	        if (ETAL_GET_DECODED_RDS_MS(*((tU8*)ptr + 1)) == (tU8)0x01)
	        {
			    pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_MS;
	        }
			pRDSdata->m_storedInfoBitmap = pRDSdata->m_validityBitmap;
		}
		else
		{
		if (DABMW_RdsGetPtyTpTaMs(slot_s, &ptr, TRUE) != 0)
			{
				pRDSdata->m_PTY = ETAL_GET_DECODED_RDS_PTY(*((tU8*)ptr + 0));
				pRDSdata->m_TP =  ETAL_GET_DECODED_RDS_TP(*((tU8*)ptr + 0));
				pRDSdata->m_TA =  ETAL_GET_DECODED_RDS_TA(*((tU8*)ptr + 0));
				pRDSdata->m_MS =  ETAL_GET_DECODED_RDS_MS(*((tU8*)ptr + 0));
		        if (ETAL_GET_DECODED_RDS_PTY(*((tU8*)ptr + 1)) == (tU8)0x1F)
		        {
				    pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_PTY;
		        }
		        if (ETAL_GET_DECODED_RDS_TP(*((tU8*)ptr + 1)) == (tU8)0x01)
		        {
				    pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_TP;
		        }
		        if (ETAL_GET_DECODED_RDS_TA(*((tU8*)ptr + 1)) == (tU8)0x01)
		        {
				    pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_TA;
		        }
		        if (ETAL_GET_DECODED_RDS_MS(*((tU8*)ptr + 1)) == (tU8)0x01)
		        {
				    pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_MS;
		        }
			}			
		}

		// AFList, AFListPI, AFListLen

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
		if (DABMW_RdsCheckPi(slot_s, (tPU32)&piVal) != 0)
		{
	        if ((af_temp = DABMW_RdsGetAf(slot_s, hReceiver, (tU8*)&pRDSdata->m_AFList, forcedGet, piVal, baseFreq, mode, DABMW_AF_LIST_BFR_LEN + 4)) != 0)
	        {
	            pRDSdata->m_AFListLen = (tU32)af_temp;
	            pRDSdata->m_AFListPI = piVal;
	            pRDSdata->m_validityBitmap |= ETAL_DECODED_RDS_VALID_AF;
				pRDSdata->m_storedInfoBitmap |= ETAL_DECODED_RDS_VALID_AF;
	        }
	        else
	        {
	            pRDSdata->m_AFListLen = 0;
	        }
		}
#else
	    pRDSdata->m_AFListLen = 0;
#endif // CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
	}
	// 
	else
	{
		// Frequency is not valid
		// avoid sending the RDS data
		// it means we are in a transition state
		// 
		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "ETALTML_RDSdecodeBlock: hReceiver %x, not valid freq, skip data", hReceiver);
	}
}

/***************************
 *
 * ETALTML_RDScheckDecoded
 *
 **************************/
tVoid ETALTML_RDScheckDecoded(ETAL_HANDLE hReceiver, tU8 *buf, tU32 len)
{
	ETAL_HANDLE hDatapath;
	EtalRDSData *rds_data;
	EtalTextInfo *radio_text;

	if (len == 0)
	{
		return;
	}
	ETALTML_statusGetRadiotextStorage(&rds_data, &radio_text);
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_2)
	COMMON_tracePrintBufUser2(TR_CLASS_APP_ETAL, buf, len, NULL);
#endif
	ETALTML_RDSdecodeBlock(hReceiver, rds_data, buf, len);
	if ((ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialTextInfo) == true) &&
		((hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_TEXTINFO)) != ETAL_INVALID_HANDLE))
	{
		OSAL_pvMemorySet((tVoid *)radio_text, 0x00, sizeof(EtalTextInfo));
		//etalTestPrintRDS(rds_data); // DEBUG - [RB] this function is no longer included in ETAL, it now part of ETAL Test application

		radio_text->m_broadcastStandard = ETAL_BCAST_STD_FM;
		if ((rds_data->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
		{
			OSAL_szStringNCopy(radio_text->m_serviceName, rds_data->m_PS, (size_t)OSAL_MIN_FUNCTION(ETAL_DEF_MAX_SERVICENAME,ETAL_DEF_MAX_PS_LEN)); 
			radio_text->m_serviceNameIsNew = TRUE;
			radio_text->m_serviceNameIsStored = TRUE;
		}
		else if ((rds_data->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
		{
			OSAL_szStringNCopy(radio_text->m_serviceName, rds_data->m_PS, (size_t)OSAL_MIN_FUNCTION(ETAL_DEF_MAX_SERVICENAME,ETAL_DEF_MAX_PS_LEN)); 
			radio_text->m_serviceNameIsStored = TRUE;
		}
			
		if ((rds_data->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
		{
			OSAL_szStringNCopy(radio_text->m_currentInfo, rds_data->m_RT, (size_t)OSAL_MIN_FUNCTION(ETAL_DEF_MAX_INFO,ETAL_DEF_MAX_RT_LEN)); 
			radio_text->m_currentInfoIsNew = TRUE;
			radio_text->m_currentInfoIsStored= TRUE;
		}
		else if ((rds_data->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
		{
			OSAL_szStringNCopy(radio_text->m_currentInfo, rds_data->m_RT, (size_t)OSAL_MIN_FUNCTION(ETAL_DEF_MAX_INFO,ETAL_DEF_MAX_RT_LEN)); 
			radio_text->m_currentInfoIsStored = TRUE;
		}

		// send the text update only if new.
		if ((TRUE == radio_text->m_currentInfoIsNew) || (TRUE == radio_text->m_serviceNameIsNew))
		{
			ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)radio_text, sizeof(EtalTextInfo), NULL);
		}
	}

	if ((ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialDecodedRDS) == true) &&
		((hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_FM_RDS)) != ETAL_INVALID_HANDLE))
	{
		if (((rds_data->m_validityBitmap & RDSServiceListTable[ETAL_handleReceiverGetIndex(hReceiver)]) != 0) && 
			(rds_data->m_validityBitmap != 0))
		{
			/*
			 * some RDS data changed since last call, invoke the callback
			 */
			ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)rds_data, sizeof(EtalRDSData), NULL);
		}
		else
		{
			// tmp print
			if (rds_data->m_validityBitmap != 0)
			{
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "ETALTML_RDScheckDecoded: receiver = %d no data sent out, m_validityBitmap %d, RDSServiceListTable %d", 
					hReceiver, rds_data->m_validityBitmap, RDSServiceListTable[ETAL_handleReceiverGetIndex(hReceiver)]);
			}
		}
	}
}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
/***************************
 *
 * ETALTML_getServiceLabel
 *
 **************************/
static ETAL_STATUS ETALTML_getServiceLabel(tChar *label, tU8 *charSet, tU16 max_len)
{
	tU32 service, UEId;
	static EtalServiceInfo serv_info;  // static for size, not persistency
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tBool have_data;

	ETAL_statusGetDABService(&UEId, &service, NULL);
	if ((UEId != ETAL_INVALID_UEID) && (service != ETAL_INVALID_SID))
	{
		if (ETAL_cmdGetSpecificServiceData_MDR(UEId, service, &serv_info, NULL, &have_data) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			if (have_data)
			{
				OSAL_szStringNCopy(label, serv_info.m_serviceLabel, max_len);
				label[max_len - 1] = '\0';
				*charSet = serv_info.m_serviceLabelCharset;
			}
			else
			{
				ret = ETAL_RET_NO_DATA;
			}
		}
	}
	else
	{
		/*
		 * No tune or service select issued yet
		 */
		ret = ETAL_RET_ERROR;
	}
	return ret;
}

/***************************
 *
 * ETALTML_PADRadiotextFunc
 *
 **************************/
static void ETALTML_PADRadiotextFunc(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalPADDLSTy *ppad;
	static EtalTextInfo radiotext; // static for size, not persistency
	ETAL_HANDLE hDatapath, hReceiver;

	if (pBuffer != NULL)
	{
		ppad = (etalPADDLSTy *)pBuffer;
		hReceiver = *(ETAL_HANDLE*)pvContext;

		OSAL_pvMemorySet((tVoid *)&radiotext, 0x00, sizeof(EtalTextInfo));
		radiotext.m_broadcastStandard = ETAL_BCAST_STD_DAB;
		radiotext.m_currentInfoIsNew = TRUE;
		OSAL_szStringNCopy(radiotext.m_currentInfo, ppad->m_PAD_DLS, ETAL_DEF_MAX_INFO);
		radiotext.m_currentInfo[ETAL_DEF_MAX_INFO - 1] = '\0';
		radiotext.m_currentInfoCharset = ppad->m_charset;

		/* get hDatapath from hReceiver and invoke call back */
	    if ((hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_TEXTINFO)) != ETAL_INVALID_HANDLE)
	    {
	    	if (radiotext.m_currentInfoIsNew)
	    	{
	    		/*
	    		 * invoke call back only if at least one of service name or current info changed
	    		 */
	    		ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)&radiotext, sizeof(EtalTextInfo), NULL);
	    	}
	    }
	}
	else
	{
		ASSERT_ON_DEBUGGING(0);
		return;
	}
}

/***************************
 *
 * ETALTML_PADRadiotextPeriodicFunc
 *
 **************************/
static tVoid ETALTML_PADRadiotextPeriodicFunc(ETAL_HANDLE hGeneric)
{
	static EtalTextInfo radiotext; // static for size, not persistency
	tBool is_new;
	ETAL_HANDLE hDatapath, hReceiver = hGeneric;
	tBool vl_releaseLock = FALSE;
	
	if (!ETAL_handleIsReceiver(hGeneric))
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}

	/* get receiver lock */
	if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}
	
	vl_releaseLock = TRUE;
	
	//check receiver is still valid and has not been destroyed !
	if (false == ETAL_receiverIsValidHandle(hReceiver))
	{
		goto exit;
	}

	hReceiver = hGeneric;
	if (ETAL_receiverGetFrequency(hReceiver) == ETAL_INVALID_FREQUENCY)
	{
		/*
		 * Probably a tune command was not yet completed, avoid
		 * sending commands to MDR which would result in 'unexpected response'
		 */
		goto exit;
	}
	OSAL_pvMemorySet((tVoid *)&radiotext, 0x00, sizeof(EtalTextInfo));
	radiotext.m_broadcastStandard = ETAL_BCAST_STD_DAB;
	/*
	 * service label
	 */
	if (ETAL_statusGetDABServiceLabel(radiotext.m_serviceName, &(radiotext.m_serviceNameCharset), ETAL_DEF_MAX_SERVICENAME, &is_new) == OSAL_OK)
	{
		radiotext.m_serviceNameIsNew = is_new;
	}
	else
	{
		/*
		 * No service label yet, try to fetch one from the MDR
		 */
		if (ETALTML_getServiceLabel(radiotext.m_serviceName, &(radiotext.m_serviceNameCharset), ETAL_DEF_MAX_SERVICENAME) == ETAL_RET_SUCCESS)
		{
			ETAL_statusSetDABServiceLabel(radiotext.m_serviceName, radiotext.m_serviceNameCharset);
			/*
			 * re-read immediately to reset the isNew flag
			 */
			ETAL_statusGetDABServiceLabel(radiotext.m_serviceName, &(radiotext.m_serviceNameCharset), ETAL_DEF_MAX_SERVICENAME, NULL);
			radiotext.m_serviceNameIsNew = TRUE;
		}
	}

    /* get hDatapath from hReceiver and invoke call back */
    if ((hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_TEXTINFO)) != ETAL_INVALID_HANDLE)
    {
    	if (radiotext.m_serviceNameIsNew)
    	{
    		/*
    		 * invoke call back only if at least one of service name or current info changed
    		 */
    		ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)&radiotext, sizeof(EtalTextInfo), NULL);
    	}
    }
	
exit:
	
	if (TRUE == vl_releaseLock)
	{
		ETAL_receiverReleaseLock(hReceiver);
	}
	
	return;
	
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * etaltml_get_textinfo
 *
 **************************/
ETAL_STATUS etaltml_get_textinfo(ETAL_HANDLE hReceiver, EtalTextInfo *pRadiotext)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	static etalPADDLSTy pad_data;    // static for size, not persistency
	tBool no_data;
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	tSInt retval;
	static tChar title_hd[ETAL_DEF_MAX_INFO_TITLE];    // static for size, not persistency
	static tChar artist_hd[ETAL_DEF_MAX_INFO_ARTIST];  // static for size, not persistency
	tU8 vl_acq_status; // current HD tune acquisition status
	tS8 vl_curr_prog;	// current HD program
	tU32 vl_avail_prog_num;	// total number of HD program 
#endif
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
	static EtalRDSData RDSdata; // static for size, not persistency
#endif

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_get_textinfo(rec: %d)", hReceiver);

	if (pRadiotext == NULL)
	{
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_get_textinfo() = %s", ETAL_STATUS_toString(ETAL_RET_PARAMETER_ERR));
		return ETAL_RET_PARAMETER_ERR;
	}

	if ((ret = ETAL_receiverGetLock(hReceiver)) != ETAL_RET_SUCCESS)
	{
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_get_textinfo() = %s", ETAL_STATUS_toString(ret));
		return ret;
	}

	if (!ETAL_receiverIsValidHandle(hReceiver) ||
		(!ETALTML_isValidGenericRadiotextReceiver(hReceiver, &std)))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		OSAL_pvMemorySet((tVoid *)pRadiotext, 0x00, sizeof(EtalTextInfo));

		switch (std)
		{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			case ETAL_BCAST_STD_DAB:
				pRadiotext->m_broadcastStandard = ETAL_BCAST_STD_DAB;
				/*
				 * service label
				 */
				if ((ret = ETALTML_getServiceLabel(pRadiotext->m_serviceName, &(pRadiotext->m_serviceNameCharset), ETAL_DEF_MAX_SERVICENAME)) == ETAL_RET_SUCCESS)
				{
					pRadiotext->m_serviceNameIsNew = TRUE;
				}
				else if (ret == ETAL_RET_ERROR)
				{
					break;
				}

				/*
				 * current info (e.g. PAD)
				 */
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
				if (ETAL_cmdGetPAD_MDR(hReceiver, &pad_data, &no_data) == OSAL_ERROR)
#else
				if (ETAL_cmdGetPAD_DABMWoH(hReceiver, &pad_data, &no_data) == OSAL_ERROR)
#endif
				{
					if (!no_data)
					{
						ret = ETAL_RET_ERROR;
					}
					break;
				}
				else
				{
					OSAL_szStringNCopy(pRadiotext->m_currentInfo, pad_data.m_PAD_DLS, ETAL_DEF_MAX_INFO);
					pRadiotext->m_currentInfo[ETAL_DEF_MAX_INFO - 1] = '\0';
					pRadiotext->m_currentInfoCharset = pad_data.m_charset;
					pRadiotext->m_currentInfoIsNew = TRUE;
				}
				break;
#endif

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
				pRadiotext->m_broadcastStandard = ETAL_BCAST_STD_FM;
				ETALTML_RDSgetDecodedBlock(hReceiver, &RDSdata, TRUE);
				if ((RDSdata.m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
				{
					OSAL_szStringNCopy(pRadiotext->m_serviceName, RDSdata.m_PS, OSAL_MIN_FUNCTION(ETAL_DEF_MAX_SERVICENAME,ETAL_DEF_MAX_PS_LEN)); 
					pRadiotext->m_serviceNameIsNew = TRUE;
				}
				if ((RDSdata.m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
				{
					OSAL_szStringNCopy(pRadiotext->m_currentInfo, RDSdata.m_RT, OSAL_MIN_FUNCTION(ETAL_DEF_MAX_INFO,ETAL_DEF_MAX_RT_LEN)); 
					pRadiotext->m_currentInfoIsNew = TRUE;
				}
				break;
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			case ETAL_BCAST_STD_HD_FM:
			case ETAL_BCAST_STD_HD_AM:
				pRadiotext->m_broadcastStandard = std;
				// if HD is not sync, then return 	
				ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, &vl_acq_status, &vl_curr_prog, &vl_avail_prog_num, NULL);

				if ((vl_acq_status & (tU8)HD_ACQUIRED) != (tU8)HD_ACQUIRED)
				{
					// HD not present
					// nothing to do
					ret = ETAL_RET_ERROR;
				}
				else
				{
					retval = ETAL_cmdGetSIS_HDRADIO(hReceiver, pRadiotext->m_serviceName, ETAL_DEF_MAX_SERVICENAME, &(pRadiotext->m_serviceNameIsNew));
					if (retval != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}

					// now request PSD for this program
					retval = ETAL_cmdPSDDecodeGet_HDRADIO(hReceiver, vl_curr_prog, title_hd, ETAL_DEF_MAX_INFO_TITLE, artist_hd, ETAL_DEF_MAX_INFO_ARTIST);

					if (retval != OSAL_OK)
					{

						ret = ETAL_RET_ERROR;
					}
					else
					{
						ETAL_stringToRadiotext_HDRADIO(pRadiotext, title_hd, artist_hd);
						pRadiotext->m_currentInfoIsNew = TRUE;
					}
				}
				break;
#endif

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_receiverReleaseLock(hReceiver);

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_get_textinfo() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etaltml_start_textinfo
 *
 **************************/
ETAL_STATUS etaltml_start_textinfo(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
    EtalDataPathAttr dataPathAttr;
	tU32 enabledServices = 0;
	EtalDataServiceParam param;
#endif //defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_start_textinfo(rec: %d)", hReceiver);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_start_textinfo() = %s", ETAL_STATUS_toString(ETAL_RET_NOT_INITIALIZED));
		return ETAL_RET_NOT_INITIALIZED;
	}

	if (!ETALTML_isValidGenericRadiotextReceiver(hReceiver, &std))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		switch (std)
		{
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
			case ETAL_BCAST_STD_DAB:
				/*
				 * Start the periodic internal callback which invokes the datapath callback
				 * when new data is available.
				 *
				 * The periodic callback reads the service label from MDR
				 * if not already stored in the receiver status. Instead
				 * the PAD information is sent autonomously by MDR
				 * every time there is an update; ETAL_CommunicationLayer_Receive_MDR
				 * saves this info in ETAL status so it is available
				 * to the periodic callback for transfer over hDatapath
				 */
				 // If callback already register : it means that there is already a start pending
				 // do nothing, i.e. only 1 text info simultaneously

				
				if (FALSE == ETAL_intCbIsRegisteredPeriodic(ETALTML_PADRadiotextPeriodicFunc, hReceiver))
				{
					ret = ETAL_intCbRegisterPeriodic(ETALTML_PADRadiotextPeriodicFunc, hReceiver, ETAL_PADRADIOTEXT_INTCB_DELAY);
				
					if (ret == ETAL_RET_SUCCESS)
					{
	                	ETAL_receiverSetSpecial(hReceiver, cmdSpecialTextInfo, cmdActionStart);
	                	radioTextReceiver[ETAL_handleReceiverGetIndex(hReceiver)] = hReceiver;

	                	dataPathAttr.m_receiverHandle = hReceiver;
	                	dataPathAttr.m_dataType = ETAL_DATA_TYPE_DAB_DLS;
	                	dataPathAttr.m_sink.m_context = (tVoid*)&radioTextReceiver[ETAL_handleReceiverGetIndex(hReceiver)];
	                	dataPathAttr.m_sink.m_BufferSize = sizeof(etalPADDLSTy);
	                	dataPathAttr.m_sink.m_CbProcessBlock = ETALTML_PADRadiotextFunc;
	                	if ((ret = ETAL_config_datapath(&hDatapath, &dataPathAttr)) == ETAL_RET_SUCCESS)
	                	{
	                		OSAL_pvMemorySet(&param, 0, sizeof(param));
	                		if ((ret = ETAL_enable_data_service(hReceiver, ETAL_DATASERV_TYPE_DLS, &enabledServices, param)) != ETAL_RET_SUCCESS)
	                		{
	                			(LINT_IGNORE_RET)ETAL_destroy_datapath(&hDatapath);
			                	ETAL_receiverSetSpecial(hReceiver, cmdSpecialTextInfo, cmdActionStop);
	                		}
	                	}
	                	else
	                	{
		                	ETAL_receiverSetSpecial(hReceiver, cmdSpecialTextInfo, cmdActionStop);
	                	}
					}
				}
				else
				{
					// already registered
					// return ok, all is fine
					ret = ETAL_RET_SUCCESS;
				}	
				break;
#endif
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
                ETAL_receiverSetSpecial(hReceiver, cmdSpecialTextInfo, cmdActionStart);
				break;
#endif

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			case ETAL_BCAST_STD_HD_FM:			
			case ETAL_BCAST_STD_HD_AM:
				ret = ETAL_cmdStartPSD_HDRADIO(hReceiver);
				if (ret == ETAL_RET_SUCCESS)
				{
	                ETAL_receiverSetSpecial(hReceiver, cmdSpecialTextInfo, cmdActionStart);
				}
				break;
#endif

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_start_textinfo() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etaltml_stop_textinfo
 *
 **************************/
ETAL_STATUS etaltml_stop_textinfo(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
#endif

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_stop_textinfo(rec: %d)", hReceiver);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
	    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_stop_textinfo() = %s", ETAL_STATUS_toString(ETAL_RET_NOT_INITIALIZED));
		return ETAL_RET_NOT_INITIALIZED;
	}

	if (!ETALTML_isValidGenericRadiotextReceiver(hReceiver, &std))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		switch (std)
		{
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
			case ETAL_BCAST_STD_DAB:
                ETAL_receiverSetSpecial(hReceiver, cmdSpecialTextInfo, cmdActionStop);
				if (TRUE == ETAL_intCbIsRegisteredPeriodic(ETALTML_PADRadiotextPeriodicFunc, hReceiver))
				{
					ret = ETAL_intCbDeregisterPeriodic(ETALTML_PADRadiotextPeriodicFunc, hReceiver);
				}
				else
				{
					// no registered callback, so text info already stopped !
					// all is fine
					ret = ETAL_RET_SUCCESS;
				}

				if ((ret = ETAL_disable_data_service(hReceiver, ETAL_DATASERV_TYPE_DLS)) == ETAL_RET_SUCCESS)
				{
					hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_DLS);
					ret = ETAL_destroy_datapath(&hDatapath);
				}
				else
				{
				
					ETAL_tracePrintError(TR_CLASS_APP_ETAL, "etaltml_stop_textinfo/ ETAL_disable_data_service failed (%d) %s", ret, ETAL_STATUS_toString(ret));
        			(LINT_IGNORE_RET)ETAL_destroy_datapath(&hDatapath);
				}
                ETAL_receiverSetSpecial(hReceiver, cmdSpecialTextInfo, cmdActionStop);
				break;
#endif
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
                ETAL_receiverSetSpecial(hReceiver, cmdSpecialTextInfo, cmdActionStop);
				break;
#endif

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			case ETAL_BCAST_STD_HD_FM:
			case ETAL_BCAST_STD_HD_AM:				
                ETAL_receiverSetSpecial(hReceiver, cmdSpecialTextInfo, cmdActionStop);
				ret = ETAL_cmdStopPSD_HDRADIO(hReceiver);
				break;
#endif

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_stop_textinfo() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etaltml_get_decoded_RDS
 *
 **************************/
ETAL_STATUS etaltml_get_decoded_RDS(ETAL_HANDLE hReceiver, EtalRDSData *pRDSdata)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_get_decoded_RDS(rec: %d)", hReceiver);

	if (pRDSdata == NULL)
	{
	    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_get_decoded_RDS() = %s", ETAL_STATUS_toString(ETAL_RET_PARAMETER_ERR));
		return ETAL_RET_PARAMETER_ERR;
	}

	if ((ret = ETAL_receiverGetLock(hReceiver)) != ETAL_RET_SUCCESS)
	{
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_get_decoded_RDS() = %s", ETAL_STATUS_toString(ret));
		return ret;
	}

	if (!ETAL_receiverIsValidHandle(hReceiver) ||
		!ETAL_receiverIsValidRDSHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
        ETALTML_RDSgetDecodedBlock(hReceiver, pRDSdata, TRUE);
#else
		ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
	}
	ETAL_receiverReleaseLock(hReceiver);

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_get_decoded_RDS() = %s", ETAL_STATUS_toString(ret));
    return ret;
}

/***************************
 *
 * etaltml_start_decoded_RDS
 *
 * enum EtalEnumDecodedRDSBitmap can be used to fill RDSServiceList parameter
 *
 **************************/
ETAL_STATUS etaltml_start_decoded_RDS(ETAL_HANDLE hReceiver, tU32 RDSServiceList)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_start_decoded_RDS(rec: %d, RDSSerLis: 0x%x)", hReceiver, RDSServiceList);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_start_decoded_RDS() = %s", ETAL_STATUS_toString(ETAL_RET_NOT_INITIALIZED));
		return ETAL_RET_NOT_INITIALIZED;
	}

	if (ETAL_receiverIsValidRDSHandle(hReceiver) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
        std = ETAL_receiverGetStandard(hReceiver);
		switch (std)
		{
			// TODO review this ifdef, if DOT it's a programming mess
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
            case ETAL_BCAST_STD_HD_FM:			
#endif
                RDSServiceListTable[ETAL_handleReceiverGetIndex(hReceiver)] |= RDSServiceList;
                ETAL_receiverSetSpecial(hReceiver, cmdSpecialDecodedRDS, cmdActionStart);
				break;

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_start_decoded_RDS() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etaltml_stop_decoded_RDS
 *
 * enum EtalEnumDecodedRDSBitmap can be used to fill RDSServiceList parameter
 *
 **************************/
ETAL_STATUS etaltml_stop_decoded_RDS(ETAL_HANDLE hReceiver, tU32 RDSServiceList)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_stop_decoded_RDS(rec: %d, RDSSerLis: %d)", hReceiver, RDSServiceList);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
	    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_stop_decoded_RDS() = %s", ETAL_STATUS_toString(ETAL_RET_NOT_INITIALIZED));
		return ETAL_RET_NOT_INITIALIZED;
	}

	if (ETAL_receiverIsValidRDSHandle(hReceiver) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
        std = ETAL_receiverGetStandard(hReceiver);
		switch (std)
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
            case ETAL_BCAST_STD_HD_FM:
#endif
                RDSServiceListTable[ETAL_handleReceiverGetIndex(hReceiver)] &= ~RDSServiceList;
                ETAL_receiverSetSpecial(hReceiver, cmdSpecialDecodedRDS, cmdActionStop);
				break;

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_stop_decoded_RDS() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

#endif // CONFIG_ETAL_HAVE_ETALTML && CONFIG_ETALTML_HAVE_RADIOTEXT
