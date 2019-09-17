//!
//!  \file 		etalcmd_hdradio.c
//!  \brief 	<i><b> ETAL command protocol layer for HD Radio devices </b></i>
//!  \details   The ETAL command protocol layer implements the command protocol
//!				specific to each device controlled by ETAL (Tuner, DCOP).
//!
//!				This file implements the command protocol for the HD Radio devices
//!				(STA680).
//!
//!				The reference specification is:
//!				"HD Radio Commercial Receiver Baseband Processor Command and Data Interface Definition",
//!				Rev. 11, December 2013 
//!
//!  $Author$
//!  \author 	(original version) Raffaele Belardi, Roberto Allevi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO


/***************************
 *
 * Defines
 *
 **************************/
/*!
 * \def		HD_SYNCWAIT_CROSSING_DELAY
 * 			The #ETAL_tuneFSM_HDRADIO in state #PollHDSyncWait checks
 * 			if at least #ETAL_HD_DELAY_BEFORE_HD_SYNC msec have passed since
 * 			the #PollInit state entry, if not it breaks out of the loop.
 * 			To ensure that the control immediately passes from #PollInit to
 * 			#PollHDSyncWait in some conditions the state machine waits
 * 			ETAL_HD_DELAY_BEFORE_HD_SYNC + HD_SYNCWAIT_CROSSING_DELAY
 * 			at the end of #PollInit.
 */
#define HD_SYNCWAIT_CROSSING_DELAY  1

/*!
 * \def		HD_CRITICAL_COMMAND_DELAY
 * 			Some commands require a delay before the next command is sent
 * 			to the DCOP (see RX_TN_4068 and RX_IDD_2206 Rev 11 section 5.5.2.1.1)
 * 			Should be at least 46ms per specification
 */
#define HD_CRITICAL_COMMAND_DELAY  50

/*!
 * \def		ETAL_HD_TUNEGETSTATUS_TIMEOUT
 * 			Timeout for Tune_Get_Status command to get instance tuned status ok.
 * 			The timeout is equal to ETAL_HD_TUNEGETSTATUS_TIMEOUT * HD_CRITICAL_COMMAND_DELAY
 */
#define ETAL_HD_TUNEGETSTATUS_TIMEOUT   4

/*!
 * \def		LS_BYTE_MASK
 * 			Least Significant Byte Mask
 */
#define LS_BYTE_MASK                       ((tU32)0xFF)

/***************************
 *
 * Local types
 *
 **************************/
/*!
 * \enum	EtalPollSyncStateTy
 * 			States for the #ETAL_tuneFSM_HDRADIO state machine
 */
typedef enum
{
	/*! Startup state */
	PollInit,
	/*! Tune command sent, wait a fixed amount of time (#ETAL_HD_DELAY_BEFORE_HD_SYNC)
	 * then check for the HD sync */
	PollHDSyncWait,
	/*! Tune command sent, HD sync received, delay #ETAL_HD_DELAY_BEFORE_AUDIO_SYNC
	 * msec before starting to poll the Digital acquisition status for Audio sync */
	PollAudioSyncDelay,
	/*! Wait Audio sync by polling the Digital acquisition status
	 * every #ETAL_HD_AUDIO_SYNC_POLL_TIME msec */
	PollAudioSyncWait,
	/*! Tune command complete */
	PollEnd,
	/*! Parameter or communication error */
	PollError,
#ifdef	HD_MONITORING
	/*! Permanent monitoring of the tune status*/
	PollMonitoring,
#endif
} EtalPollSyncStateTy;

/*!
 * \struct		EtalTuneFSMContextTy
 * 				The #ETAL_tuneFSM_HDRADIO may be invoked from the API thread or from the
 * 				#ETAL_CommunicationLayer_ThreadEntry_HDRADIO thread. Each thread
 * 				must use its own state variable to avoid corruption. Additionally,
 * 				each thread could potentially address #ETAL_MAX_HDINSTANCE instances
 * 				of the DCOP.
 */
typedef struct
{
	/*! The #ETAL_tuneFSM_HDRADIO measures the delay from command
	 *  to command answer; this field stores the absolute
	 *  time when the command was issued */
	OSAL_tMSecond       startTime[ETAL_MAX_HDINSTANCE];
	OSAL_tMSecond       HD_BlendStarTime[ETAL_MAX_HDINSTANCE];	
	/*! The #ETAL_tuneFSM_HDRADIO state */
	EtalPollSyncStateTy state[ETAL_MAX_HDINSTANCE];
#ifdef	HD_MONITORING
	tU8 digiAcqStatusMonitored[ETAL_MAX_HDINSTANCE]; 
#endif
} EtalTuneFSMContextTy;

/***************************
 *
 * Local storage
 *
 **************************/
 /*!
  * \var	etalTuneContext
  * 		The #ETAL_tuneFSM_HDRADIO state machine context
  */
EtalTuneFSMContextTy etalTuneContext[ETAL_HDRADIO_TUNEFSM_MAX_USER];

/*!
 * \var		CP_buf
 * 			Command Protocol buffer. This is the buffer used to assemble the Command
 * 			Layer commands before sending them to the Logical Message Layer.
 * 			It is defined as global to reduce the stack requirements.
 */
static tU8 CP_buf[HDRADIO_CP_FULL_SIZE];

/*!
 * \var		v_alignmentConfiguration
 * 			store the alignement configuration if AAA is enabled or not 
 *			this is needed because specific timing applies depending on configuration
 *			May be overwritten by #ETAL_cmdDisableAAA_HDRADIO during the ETAL initialization
 */
static	EtalAudioAlignmentAttr	v_alignmentConfiguration =
#if defined(CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) && !defined(CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA)
{TRUE, TRUE}; // default is AAA activated for FM & AM
#else
{FALSE, FALSE}; // default is AAA not activated for FM & AM
#endif

/***************************
 *
 * Local prototypes
 *
 **************************/
static tVoid ETAL_sendInternalNotificationForTune_HDRADIO(ETAL_HANDLE hReceiver, tU32 freq, tU32 sync_mask);

/***********************************
 *
 * ETAL_initCPbuffer_HDRADIO
 *
 **********************************/
/*!
 * \brief		Format the Command Packet buffer
 * \details		Function called by all ETAL API functions to initialize
 * 				a "Command Packet" to be sent to the HDRADIO.
 *
 * 				Opcode and FunctionCode are assigned and passed as members of the 
 * 				#etalToHDRADIOCpTy structure, while command parameters -if any- are 
 * 				passed as optional arguments of the function.
 *
 * 				Command parameters must be passed to the function in couples of
 * 				parameter, where the first parameter of the couple contains
 * 				the subsequent parameter length (1..4) and the second parameter
 * 				contains the value, an integer. The list is terminated by
 * 				the reserved value #END_ARG_MARKER.
 * \remark		The function uses the variable number of arguments syntax.
 * \remark		The function changes the *pCPstruct* parameter
 * \param[in]	cmd - pointer to the buffer for the CP; this normally is the
 * 				      global buffer #CP_buf
 * \param[in]	pCPstruct - pointer to the structure describing the Command Packet
 * \return		OSAL_ERROR - parameter error
 * \return		Length of the full CP in bytes in case of no error
 * \callgraph
 * \callergraph
 * \todo		How to manage the abort bit in the command header
 */
static tSInt ETAL_initCPbuffer_HDRADIO(tU8 *cmd, etalToHDRADIOCpTy *pCPstruct, ... )
{
	va_list vl;
	tSInt next_arg;
	tUInt num_arg = 0;
	tU32 indexCP = HDRADIO_CP_PAYLOAD_INDEX;
	tBool subsequentDataFlag = FALSE;
	tSInt retval;

	va_start(vl, pCPstruct); /* Initialize variable arguments */

	pCPstruct->m_DataLength = 0;
	pCPstruct->m_CpData = cmd;
	pCPstruct->m_CpData[0] = pCPstruct->m_Opcode;

	if(pCPstruct->m_CmdType == (tU8)WRITE_TYPE) /* TODO: see function header */
	{
		pCPstruct->m_CmdStatus = (tU8)0;
	}

	if(pCPstruct->m_FunctionCode != (tU8)0)
	{
		pCPstruct->m_CpData[indexCP++] = pCPstruct->m_FunctionCode;
		pCPstruct->m_DataLength++;
		subsequentDataFlag = TRUE;
	}

	while(subsequentDataFlag)
	{
		tU32 subsequentDataLen, subsequentDataValue, i;
		next_arg = va_arg(vl, tSInt); /* Retrieve next arg tSInt type to be a field len */
		num_arg++;
		if(next_arg != END_ARG_MARKER)
		{
			if (next_arg < 0)
			{
				ASSERT_ON_DEBUGGING(0);
				retval = OSAL_ERROR;
				goto exit;
			}
			subsequentDataLen = (tU32)next_arg;

			if(subsequentDataLen == 0 || subsequentDataLen > 4)
			{
				retval = OSAL_ERROR;
				goto exit;
			}

			pCPstruct->m_DataLength += subsequentDataLen;

			next_arg = va_arg(vl, tSInt); /* Retrieve next arg tSInt type to be a field data */
			num_arg++;
			if(next_arg == END_ARG_MARKER)
			{
				retval = OSAL_ERROR;
				goto exit;
			}

			subsequentDataValue = next_arg;

			for(i = 0; i < subsequentDataLen; i++)
			{
				tU8 shift[] = {0, 8, 16, 24};

				pCPstruct->m_CpData[indexCP++] = (tU8)((subsequentDataValue >> shift[i]) & LS_BYTE_MASK);

				if(indexCP > (HDRADIO_CP_FULL_SIZE - HDRADIO_CP_CMDTYPESTATUS_LEN))
				{
					retval = OSAL_ERROR;
					goto exit;
				}
			}
		}
		else
		{
			break;
		}
	}

	va_end(vl); /* Reset variable arguments */

	if(pCPstruct->m_FunctionCode != (tU8)0 && (num_arg % 2) == 0)
	{
		retval = OSAL_ERROR;
		goto exit;
	}

	pCPstruct->m_CpData[1] = (tU8)(pCPstruct->m_DataLength & LS_BYTE_MASK);
	pCPstruct->m_CpData[2] = (tU8)(pCPstruct->m_DataLength >> 8) & LS_BYTE_MASK;
	pCPstruct->m_CpData[3] = (tU8)(pCPstruct->m_DataLength >> 16) & LS_BYTE_MASK;
	pCPstruct->m_CpData[4] = (tU8)(pCPstruct->m_DataLength >> 24) & LS_BYTE_MASK;

	pCPstruct->m_CpData[indexCP++] = pCPstruct->m_CmdType;
	pCPstruct->m_CpData[indexCP++] = (tU8)0x00; /* reserved */
	pCPstruct->m_CpData[indexCP++] = pCPstruct->m_CmdStatus;
	retval = (tSInt)indexCP;

exit:
	return retval;
}

/***************************
 *
 * ETAL_checkParameterError_HDRADIO
 *
 **************************/
/*!
 * \brief		Checks the value returned by #ETAL_initCPbuffer_HDRADIO for errors
 * \param[in]	clen - the value returned by #ETAL_initCPbuffer_HDRADIO
 * \return		OSAL_OK
 * \return		OSAL_ERROR - *clen* indicates an error
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_checkParameterError_HDRADIO(tSInt clen)
{
	tSInt retval = OSAL_OK;
	if (clen == OSAL_ERROR)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "Function parameters incorrect");
		retval = OSAL_ERROR;
	}
	return retval;
}

/***************************
 *
 * ETAL_sendCommunicationErrorEvent_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends a communication error to the API user
 * \details		The function may generate the following error codes:
 * 				- #EtalCommStatus_ProtocolHeaderError
 * 				- #EtalCommStatus_GenericError
 * 				- #EtalCommStatus_TimeoutError (not yet used)
 *
 * 				The raw error code for #ETAL_sendCommunicationErrorEvent
 * 				is not available so it is set to 0
 * \param[in]	hReceiver - handle of the Receiver that triggered the error
 * \param[in]	retval - the OSAL error returned by the communication function
 * \param[in]	buf - pointer to the complete response
 * \param[in]	buf_len - size in bytes of the *buf* buffer.
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_sendCommunicationErrorEvent_HDRADIO(ETAL_HANDLE hReceiver, tSInt retval, tU8 *buf, tU32 buf_len)
{
	if (retval == OSAL_ERROR_TIMEOUT_EXPIRED) /* not yet generated by the HDRadio communication layer */
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_TimeoutError, 0, buf, buf_len);
	}
	else if (retval == OSAL_ERROR_FROM_SLAVE)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_ProtocolHeaderError, 0, buf, buf_len);
	}
	else
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_GenericError, 0, buf, buf_len);
	}
}

/***************************
 *
 * ETAL_parseSIS_HDRADIO
 *
 **************************/
/*!
 * \brief		Extracts the Station Name from the Get_Ext_SIS (0x47) response
 * \details		The response to the Get_Ext_SIS may contain a variable number
 * 				of arguments depending on the broadcast (e.g. Station ID, Station
 * 				Name short form, Station Name long form, ...). This function searches
 * 				and extracts only the 'Station Name (Short form)' field.
 * \param[in]	resp - pointer to buffer containing the response to the Get_Ext_SIS command
 * \param[in]	len - length of the response contained in *resp*
 * \param[out]	name - pointer to buffer where the function stores the parsed null-terminated string
 * \param[in]	max_len - size in bytes of the *name* buffer; if the parsed string is longer
 * 				          than this limit it is silently truncated to *max_len*
 * \param[in]	is_new - pointer to variable where the function stores TRUE if the
 * 				         parsed string is different from the last one sent by the
 * 				         DCOP (note: the function only copies this field from 
 * 				         the one received from the DCOP, it does not validate
 * 				         the indication)
 * \return		OSAL_OK
 * \return		OSAL_ERROR - the Station Name (short form) is not present in the *resp* buffer
 * \callgraph
 * \callergraph
 */
tSInt ETAL_parseSIS_HDRADIO(tU8 *resp, tU32 len, tChar *name, const tU32 max_len, tBool *is_new)
{
	tU32 offset_to_type_desc, bytes_to_copy;
	tS32 Num_Types;
	tU8 Type_ID;
	tU32 Status, Length; /* contain tU8 but store in tU32 to reduce casts */
	tBool found = FALSE;
	tSInt retval = OSAL_ERROR;

	Num_Types = (tS32)resp[ETAL_HDRADIO_GETBASICSIS_NUMTYPES_OFFSET];
	offset_to_type_desc = ETAL_HDRADIO_GETBASICSIS_NUMTYPES_OFFSET + 1;
	while (offset_to_type_desc < len)
	{
		Num_Types--;
		Type_ID = resp[offset_to_type_desc + ETAL_HDRADIO_GETBASICSIS_TYPEID_REL_OFFSET];
		Status  = (tU32)resp[offset_to_type_desc + ETAL_HDRADIO_GETBASICSIS_STATUS_REL_OFFSET];
		Length  = (tU32)resp[offset_to_type_desc + ETAL_HDRADIO_GETBASICSIS_LENGTH_REL_OFFSET];

		if ((Type_ID != (tU8)STATION_SHORT_NAME) ||
			(Status == NO_DATA_AVAILABLE) ||
			(Length == 0))
		{
			/*
			 * Ignore the field, no data or data for a different Type_ID
			 *
			 * Consider also the Type_ID, Status and Length bytes for length calculation
			 */
			offset_to_type_desc += Length + 3;
			continue;
		}
		/*
		 * Station Name (short form) found
		 */
		if (Length <= max_len)
		{
			bytes_to_copy = Length;
		}
		else
		{
			bytes_to_copy = max_len;
		}
		(void)OSAL_pvMemoryCopy((tVoid *)name, (tPCVoid)(resp + offset_to_type_desc + ETAL_HDRADIO_GETBASICSIS_DATA_REL_OFFSET), bytes_to_copy);
		name[bytes_to_copy] = '\0';
		if (Status == NEW_DATA_DISPLAYABLE)
		{
			*is_new = TRUE;
		}
		else
		{
			*is_new = FALSE;
		}
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "Found Station Name %s, is_new = %d", name, *is_new);
		found = TRUE;
		break;
	}

	/*
	 * sanity checks
	 */
	if ((offset_to_type_desc > len) ||
		(Num_Types < 0))
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "Incoherent length (calculated %d, expected %d) or Num_Types (%d) in GetSIS response", offset_to_type_desc, len, Num_Types);
		retval = OSAL_ERROR;
		goto exit;
	}
	if (found)
	{
		retval = OSAL_OK;
	}

exit:
	return retval;
}

/***************************
 *
 * ETAL_parsePSDDecode_HDRADIO
 *
 **************************/
/*!
 * \brief		Extracts the Title and Artist from the PSD_Decode (0x93) response
 * \remark		The function does not notify the caller if there are no Title or Artist
 * 				fields in the *resp*. The output parameters are not changed in this
 * 				case so the caller should provide pointers to zero-initialized buffers
 * 				to detect this condition.
 * \remark		Parameter *audioPrgs* is provided for future enhancements, currently
 * 				the only allowed value is 1. The function does not validate this value
 * 				so it is up to the caller to enforce this limitation.
 * \param[in]	resp - pointer to buffer containing the response to the PSD_Decode command
 * \param[in]	len - length of the response contained in *resp*
 * \param[in]	audioPrgs - the number of audio programs for which to extract the PSD data.
 * 				            See "Remarks" section for limitations.
 * \param[out]	title - pointer to buffer where the function stores the parsed null-terminated Title string
 * \param[in]	max_title - size of the *title* buffer including the null-termination: if the parsed Title string
 * 				            exceeds this limit it is silently truncated to (*max_title* - 1) characters
 * \param[out]	artist - pointer to buffer where the function stores the parsed null-terminated Artist string
 * \param[in]	max_artist - size of the *artist* buffer including the null-termination: if the parsed Artist string
 * 				             exceeds this limit it is silently truncated to (*max_artist* -1) characters
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 * \todo		If *audioPrgs* is greater than 1 the function returns only
 * 				the data for the last program parsed. This is probably
 * 				**not** what the caller intended.
 */
tSInt ETAL_parsePSDDecode_HDRADIO(tU8 *resp, tU32 len, tU8 audioPrgs, tChar *title, tU32 max_title, tChar *artist, tU32 max_artist)
{
#if 0
	tU8 Program; /* commented out because currently not used */
#endif
	tU32 Field_Type; /* contains tU16 but use tU32 to avoid casts */
	tU32 Length;
	tU32 offset_from_field_start;
	tU32 local_len;
	tU32 i;

	offset_from_field_start = ETAL_HDRADIO_GETPSD_FIRST_FIELD_OFFSET;

	/*
	 * Note: if there is data for more than one program in the STA680 response
	 * this loop returns the data for the last program only, which is probably
	 * not the best thing to do.
	 * Maybe <audioPrgs> should become an identifier of the requested program
	 * but this implies changes on upper layers also.
	 */
	for(i = 0; i < (tU32)audioPrgs; i++)
	{
		while (offset_from_field_start < len - HDRADIO_CP_CMDTYPESTATUS_LEN)
		{
#if 0
			Program    = resp[offset_from_field_start + ETAL_HDRADIO_GETPSD_PROGRAM_REL_OFFSET];
#endif
			Field_Type = (tU32)resp[offset_from_field_start + ETAL_HDRADIO_GETPSD_FIELD_REL_OFFSET];
			Length     = (tU32)(resp[offset_from_field_start + ETAL_HDRADIO_GETPSD_LENGTH_REL_OFFSET] & 0xFF);

			if (Field_Type == TITLE)
			{
				if (Length <= max_title - 1)
				{
					local_len = Length;
				}
				else
				{
					local_len = max_title - 1;
				}
				(void)OSAL_pvMemoryCopy((tVoid *)title, (tPCVoid)(resp + offset_from_field_start + ETAL_HDRADIO_GETPSD_DATA_REL_OFFSET), local_len);
				title[local_len] = '\0';
			}
			else if (Field_Type == ARTIST)
			{
				if (Length <= max_artist - 1)
				{
					local_len = Length;
				}
				else
				{
					local_len = max_artist - 1;
				}
				(void)OSAL_pvMemoryCopy((tVoid *)artist, (tPCVoid)(resp + offset_from_field_start + ETAL_HDRADIO_GETPSD_DATA_REL_OFFSET), local_len);
				artist[local_len] = '\0';
			}
			else
			{
				/* Nothing to do */
			}

			offset_from_field_start += (ETAL_HDRADIO_GETPSD_MIN_INFO_LEN + Length);
#if 0
		if (offset_from_field_start > len - HDRADIO_CP_CMDTYPESTATUS_LEN)
		{
			return OSAL_ERROR;
		}
#endif
		}
	}

	return OSAL_OK;
}

/***************************
 *
 * ETAL_configRadiotext_HDRADIO
 *
 **************************/
/*!
 * \brief		Initializes the HD Radio DCOP for TextInfo processing
 * \details		The function sends to the HD Radio DCOP the commands required to:
 * 				- enable Title and Artist PSD decoding
 * 				- configure the max length for the Title and Artist
 *
 * 				This function is normally called only at ETAL system start-up.
 * \return		OSAL_ERROR - communication error with the HD Radio device
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 * \todo		The function assumes only one HD Radio device in the system,
 * 				extend to support multiple devices
 */
tSInt ETAL_configRadiotext_HDRADIO(tVoid)
{
	ETAL_HANDLE hReceiver = (ETAL_HANDLE)HDRADIO_FAKE_RECEIVER;
	tSInt retval = OSAL_OK;

	/* TODO: see function header */

	if (ETAL_cmdPSDCnfgFieldsEn_HDRADIO(hReceiver, NULL) != OSAL_OK)
	{
		retval = OSAL_ERROR;
	}
	else if (ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_TITLE_LENGTH, (tU8)ETAL_DEF_MAX_INFO_TITLE) != OSAL_OK)
	{
		retval = OSAL_ERROR;
	}
	else if (ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_ARTIST_LENGTH, (tU8)ETAL_DEF_MAX_INFO_ARTIST) != OSAL_OK)
	{
		retval = OSAL_ERROR;
	}
	else
	{
		retval = OSAL_OK;
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdGetStatusDigital_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the Sys_Tune (0x82) command, Tune_Get_Status (0x06) function and parses the response
 * \details		This function reads the Digital acquisition status only.
 * \details		If Byte 1 of the response indicates 'Tune operation ongoing'
 * 				the function only returns the #TUNE_OPERATION_PENDING code in the
 * 				#m_TuneOperationType field, other fields of *ptDigiAcqStatus* are
 * 				left unchanged.
 * \details		The function locks the HD Radio device for the duration of the operation.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[out]	ptDigiAcqStatus - pointer to a structure where the function stores
 * 				                  the Digital status received from the HD Radio device.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response. In case of
 * 				             error the *ptDigiAcqStatus* buffer is unchanged.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetStatusDigital_HDRADIO(ETAL_HANDLE hReceiver, etalToHDRADIODigiAcqStatusTy *ptDigiAcqStatus)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;

	etalToHDRADIOCpTy cmdTuneGetStatus;
	(void)OSAL_pvMemorySet((tVoid *)&cmdTuneGetStatus, 0, sizeof(etalToHDRADIOCpTy));

	cmdTuneGetStatus.m_Opcode = (tU8)HDRADIO_SYS_TUNE_CMD;
	cmdTuneGetStatus.m_FunctionCode = (tU8)TUNE_GET_STATUS;
	cmdTuneGetStatus.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneGetStatus, ARG_1B_LEN, TUNE_DIGITAL_ONLY, \
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_TUNESTATUS_PENDING_DIG_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)TUNE_GET_STATUS)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_TUNE_OPERATION_OFFSET] == (tU8)TUNE_OPERATION_PENDING)
		{
			ptDigiAcqStatus->m_TuneOperationType = (tU8)TUNE_OPERATION_PENDING;
		}
		else if (rlen < ETAL_HDRADIO_TUNESTATUS_DIG_SPECIALCASE_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_TUNE_OPERATION_GETSTATUS_BAND_OFFSET] == (tU8)IDLE_MODE)
		{
			/*
			 * If the DCOP instace has not been tuned operation
			 * the DCOP will respond to the SYS_TUNE(TUNE_GET_STATUS) with an undocumented
			 * response format:
			 *  0x06 0x02 0x63 0x00 0x00
			 * where
			 *  0x06 = function code, TUNE_GET_STATUS
			 *  0x02 = only digital status requested
			 *  0x63 = Band byte signaling 'idle state'
			 *         (according to iBiquity spec this byte should only be 0x00 for AM or 0x01 for FM)
			 *  0x00 = frequency LSB
			 *  0x00 = frequency MSB
			 *
			 * This is not an error
			 */
			(void)OSAL_pvMemorySet((tVoid *)ptDigiAcqStatus, 0, sizeof(etalToHDRADIODigiAcqStatusTy));
			ptDigiAcqStatus->m_TuneOperationType = resp[ETAL_HDRADIO_TUNE_OPERATION_OFFSET];
			ptDigiAcqStatus->m_Band = resp[ETAL_HDRADIO_TUNE_OPERATION_GETSTATUS_BAND_OFFSET];
			ptDigiAcqStatus->m_LSBRfChFreq = resp[ETAL_HDRADIO_TUNE_OPERATION_GETSTATUS_FREQUENCY_OFFSET];
			ptDigiAcqStatus->m_MSBRfChFreq = resp[ETAL_HDRADIO_TUNE_OPERATION_GETSTATUS_FREQUENCY_OFFSET + 1];
		}
		else if (rlen < ETAL_HDRADIO_TUNESTATUS_DIG_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			(void)OSAL_pvMemoryCopy((tVoid *)ptDigiAcqStatus, (tPCVoid)&resp[ETAL_HDRADIO_TUNE_OPERATION_OFFSET], sizeof(etalToHDRADIODigiAcqStatusTy));
		}
	}

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdGetStatusDSQM_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the Sys_Tune (0x82) command, Get_DSQM_Status (0x0C) function and parses the response
 * \details		DSQM status is used to quickly determine the HD Radio signal quality.
 * \details		The function locks the HD Radio device for the duration of the operation.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[in]	ptDSQMint - pointer to a structure where the function stores
 * 				            the status received from the device
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response. In case of
 * 				             error the *ptDSQMint* buffer is unchanged.
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_cmdGetStatusDSQM_HDRADIO(ETAL_HANDLE hReceiver, tU32 *ptDSQMint)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;

	etalToHDRADIOCpTy cmdTuneGetStatus;
	(void)OSAL_pvMemorySet((tVoid *)&cmdTuneGetStatus, 0, sizeof(etalToHDRADIOCpTy));

	cmdTuneGetStatus.m_Opcode = (tU8)HDRADIO_SYS_TUNE_CMD;
	cmdTuneGetStatus.m_FunctionCode = (tU8)GET_DSQM_STATUS;
	cmdTuneGetStatus.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneGetStatus, 
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_TUNESTATUS_DSQM_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)GET_DSQM_STATUS)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			*ptDSQMint = ((tU32)resp[ETAL_HDRADIO_DSQM_OFFSET + 1] << 8) | (tU32)resp[ETAL_HDRADIO_DSQM_OFFSET];
		}
	}

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdGetStatusDigitalAsync_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the Sys_Tune (0x82) command, Tune_Get_Status (0x06) function
 * \details		The function sends the command to read the Digital status only and
 * 				does not wait for the answer to arrive from the device.
 *
 * 				The asynchronous behavior is obtained by pushing the command to the
 * 				#ETAL_CommunicationLayer_ThreadEntry_HDRADIO thread which, operating
 * 				in the background, sends it to the device and reads the response after
 * 				a fixed amount of time.
 *
 * 				The response, when available, will be communicated over the *hDatapath*
 * 				which is assumed to be of type #ETAL_DATA_TYPE_TEXTINFO.
 *
 * 				The function locks the HD Radio device for the duration of the operation.
 *
 * 				Unlike the non-async functions, the duration of the operation in this case
 * 				depends only on the time needed to send the command.
 * \param[in]	hDatapath - handle of the #ETAL_DATA_TYPE_TEXTINFO Datapath on which to
 * 				            provide the decoded information once available.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error communicating with the device, or HD Radio command FIFO full
 * \see			ETAL_cmdGetStatusDigital_HDRADIO for details on the command format.
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_cmdGetStatusDigitalAsync_HDRADIO(ETAL_HANDLE hDatapath)
{
	tSInt retval;
	tU8 *cmd = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tSInt clen;

	etalToHDRADIOCpTy cmdTuneGetStatus;
	(void)OSAL_pvMemorySet((tVoid *)&cmdTuneGetStatus, 0, sizeof(etalToHDRADIOCpTy));

	cmdTuneGetStatus.m_Opcode = (tU8)HDRADIO_SYS_TUNE_CMD;
	cmdTuneGetStatus.m_FunctionCode = (tU8)TUNE_GET_STATUS;
	cmdTuneGetStatus.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneGetStatus, ARG_1B_LEN, TUNE_DIGITAL_ONLY, \
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_queueCommand_HDRADIO(hDatapath, cmd, (tU32)clen);

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

#if defined (CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) && !defined (CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA)
/***************************
 *
 * ETAL_cmdGetAlignStatus_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the Sys_Tune (0x82) command, Tune_Get_Align_Status (0x0D) function and parses the response
 * \details		This function reads the Audio Alignement status.
 * \details		If Byte 1 of the response indicates 'Tune operation ongoing'
 * 				the function only returns the #TUNE_OPERATION_PENDING code in the
 * 				#m_TuneOperationType field, other fields of *pO_AudioAlignmentStatus* are
 * 				left unchanged.
 * \details		The function locks the HD Radio device for the duration of the operation.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[out]	ptDigiAcqStatus - pointer to a structure where the function stores
 * 				                  the Digital status received from the HD Radio device.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response. In case of
 * 				             error the *pO_AudioAlignmentStatus* buffer is unchanged.
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_cmdGetAlignStatus_HDRADIO(ETAL_HANDLE hReceiver, etalToHDRADIOAlignStatusTy *pO_AudioAlignmentStatus)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;
	tSInt vl_index;

	etalToHDRADIOCpTy cmdTuneGetAlignStatus;
	OSAL_pvMemorySet((tVoid *)&cmdTuneGetAlignStatus, 0, sizeof(etalToHDRADIOCpTy));

	cmdTuneGetAlignStatus.m_Opcode = (tU8)HDRADIO_SYS_TUNE_CMD;
	cmdTuneGetAlignStatus.m_FunctionCode = (tU8)GET_ALIGN_STATUS;
	cmdTuneGetAlignStatus.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneGetAlignStatus, \
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		return OSAL_ERROR;
	}

	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_TUNEALIGNSTATUS_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)GET_ALIGN_STATUS)
		{
			retval = OSAL_ERROR;
		}
		else
		{
//			OSAL_pvMemoryCopy((tVoid *)pO_AudioAlignmentStatus, (tPCVoid)&resp[ETAL_HDRADIO_TUNE_OPERATION_OFFSET], sizeof(etalToHDRADIOAlignStatusTy));
			// fill the structure
			// 	AudioIndicatorTy m_AudioIndicator;

			vl_index = ETAL_HDRADIO_TUNE_OPERATION_OFFSET;
			pO_AudioAlignmentStatus->m_AudioIndicator.value = resp[vl_index];
			vl_index++;

			// Alingment offset is the 4 next bytes from LSB to MSB
			pO_AudioAlignmentStatus->m_AlignmentOffset = (tU32) (((tU32)resp[vl_index+0] << 0) 
														| (((tU32) resp[vl_index+1]) << 8) 
														| (((tU32) resp[vl_index+2]) << 16)
														| (((tU32) resp[vl_index+3]) << 24));
			vl_index+=4;

			// level
			pO_AudioAlignmentStatus->m_LevelAdjustment = resp[vl_index];

#if 0
			printf("ETAL_cmdGetAlignStatus_HDRADIO : pO_AudioAlignmentStatus m_AudioIndicator.value %d, m_AlignmentOffset %d, m_LevelAdjustment %d\n", 
			pO_AudioAlignmentStatus->m_AudioIndicator.value, 
			pO_AudioAlignmentStatus->m_AlignmentOffset, 
			pO_AudioAlignmentStatus->m_LevelAdjustment);
#endif


		}
	}

#if 0
	printf("ETAL_cmdGetAlignStatus_HDRADIO : retval %d, rlen %d\n", 
			retval, rlen);
#endif

	ETAL_releaseHDRADIODevLock();
	return retval;
}
#endif // CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA && !CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA


/***************************
 *
 * ETAL_cmdTuneSelect_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the Sys_Tune (0x82) command, Tune_Select (0x01) function and parses the response
 * \details		This command is used to select a particular service inside a
 * 				HD Radio program.
 * 				The function locks the HD Radio device for the duration of the operation.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[in]	u32KHzFreq - the frequency to tune to
 * \param[in]	prg - the HD Radio program to select (MPS or one of the SPS)
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdTuneSelect_HDRADIO(ETAL_HANDLE hReceiver, tU32 u32KHzFreq, tyHDRADIOAudioPrgID prg)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen = OSAL_ERROR;
	EtalBcastStandard standard = ETAL_receiverGetStandard(hReceiver);

	etalToHDRADIOCpTy cmdTuneSelect;
	(void)OSAL_pvMemorySet((tVoid *)&cmdTuneSelect, 0, sizeof(etalToHDRADIOCpTy));

	cmdTuneSelect.m_Opcode = (tU8)HDRADIO_SYS_TUNE_CMD;
	cmdTuneSelect.m_FunctionCode = (tU8)TUNE_SELECT;
	cmdTuneSelect.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	/* HDRADIO dev takes two-byte frequency value as the hex equivalent of the decimal frequency in tens of KHz (KHz/10 or MHz*100 ; e.g. 87700KHz/10=0x2242) 
	 * and in kHz for AM_BAND */
	if ((standard == ETAL_BCAST_STD_HD_AM) || (standard == ETAL_BCAST_STD_HD_FM))
	{
		if(standard == ETAL_BCAST_STD_HD_AM)
		{
			clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneSelect, ARG_1B_LEN, AM_BAND, ARG_2B_LEN, u32KHzFreq, ARG_1B_LEN, prg, END_ARG_MARKER);
		}
		else
		{
			clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneSelect, ARG_1B_LEN, FM_BAND, ARG_2B_LEN, u32KHzFreq/10, ARG_1B_LEN, prg, END_ARG_MARKER);
		}
	
		if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
		{
			ETAL_releaseHDRADIODevLock();
			retval = OSAL_ERROR;
			goto exit;
		}

		// new service selected : force the flag as a new station
		ETAL_forceResendRadioInfo();
		
		retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, rlen);
		}
		else
		{
			if (rlen < ETAL_HDRADIO_TUNESEL_MIN_FIELD_LEN)
			{
				retval = OSAL_ERROR;
			}
			else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)TUNE_SELECT || 
						resp[ETAL_HDRADIO_VALID_INDICATION_OFFSET] != (tU8)VALID)
			{
				retval = OSAL_ERROR;
			}
			else
			{
				/* Nothing to do */
			}
		}
	}
	else
	{
		retval = OSAL_ERROR;
	}

	/* Sys_Tune is one of those commands that requires a delay
	 * before the next command, see #HD_CRITICAL_COMMAND_DELAY */
	(void)OSAL_s32ThreadWait(HD_CRITICAL_COMMAND_DELAY);

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

#if 0
/* currently unused */
/***************************
 *
 * ETAL_cmdTuneSelectAsync_HDRADIO
 *
 **************************/
static tSInt ETAL_cmdTuneSelectAsync_HDRADIO(ETAL_HANDLE hDatapath, tU32 u32KHzFreq, tyHDRADIOAudioPrgID prg)
{
	tSInt retval;
	tU8 *cmd = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tSInt clen;

	etalToHDRADIOCpTy cmdTuneSelect;
	OSAL_pvMemorySet((tVoid *)&cmdTuneSelect, 0, sizeof(etalToHDRADIOCpTy));

	cmdTuneSelect.m_Opcode = (tU8)HDRADIO_SYS_TUNE_CMD;
	cmdTuneSelect.m_FunctionCode = (tU8)TUNE_SELECT;
	cmdTuneSelect.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	/* HDRADIO dev takes two-byte frequency value as the hex equivalent of the decimal frequency in tens of KHz (KHz/10 or MHz*100 ; e.g. 87700KHz/10=0x2242) */
	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneSelect, ARG_1B_LEN, FM_BAND, ARG_2B_LEN, u32KHzFreq/10, ARG_1B_LEN, prg, \
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		return OSAL_ERROR;
	}

	retval = ETAL_queueCommand_HDRADIO(hDatapath, cmd, (tU32)clen);

	ETAL_releaseHDRADIODevLock();
	return retval;
}
#endif

/***************************
 *
 * ETAL_cmdGetSIS_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the Get_Ext_SIS_Data (0x47) command, Get_Basic_SIS_Data (0x05) function and parses the response
 * \details		This function is used to read the current Station name from the HD Radio device.
 * 				The function locks the HD Radio device for the duration of the operation.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[out]	name - pointer to buffer where the function stores the parsed null-terminated string
 * \param[in]	max_name - size in bytes of the *name* buffer; if the parsed string is longer
 * 				           than this limit it is silently truncated to *max_name*
 * \param[in]	is_new - pointer to variable where the function stores TRUE if the
 * 				         parsed string is different from the last one sent by the
 * 				         DCOP (note: the function only copies this field from 
 * 				         the one received from the DCOP, it does not validate
 * 				         the indication)
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \see			ETAL_parseSIS_HDRADIO for details on the output parameter format
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetSIS_HDRADIO(ETAL_HANDLE hReceiver, tChar *name, tU16 max_name, tBool *is_new)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;

	etalToHDRADIOCpTy cmdGetBasicSISData;
	(void)OSAL_pvMemorySet((tVoid *)&cmdGetBasicSISData, 0, sizeof(etalToHDRADIOCpTy));

	cmdGetBasicSISData.m_Opcode = (tU8)HDRADIO_GET_EXT_SIS_DATA_CMD;
	cmdGetBasicSISData.m_FunctionCode = (tU8)GET_BASIC_SIS_DATA;
	cmdGetBasicSISData.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdGetBasicSISData, \
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_GETBASICSIS_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)GET_BASIC_SIS_DATA || 
					ETAL_parseSIS_HDRADIO(resp, (tU16)rlen, name, max_name - 1, is_new) != OSAL_OK)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}


	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdGetSISAsync_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the Get_Ext_SIS_Data (0x47) command, Get_Basic_SIS_Data (0x05) function 
 * \details		The function sends the command to read the SIS and
 * 				does not wait for the answer to arrive from the device.
 *
 * 				The asynchronous behavior is obtained by pushing the command to the
 * 				#ETAL_CommunicationLayer_ThreadEntry_HDRADIO thread which, operating
 * 				in the background, sends it to the device and reads the response after
 * 				a fixed amount of time.
 *
 * 				The response, when available, will be communicated over the *hDatapath*
 * 				which is assumed to be of type #ETAL_DATA_TYPE_TEXTINFO.
 *
 * 				The function locks the HD Radio device for the duration of the operation.
 * 				Unlike the non-async functions, the duration of the operation in this case
 * 				depends only on the time needed to send the command.
 * \param[in]	hDatapath - handle of the #ETAL_DATA_TYPE_TEXTINFO Datapath on which to
 * 				            provide the decoded information once available.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error communicating with the device, or HD Radio command FIFO full
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetSISAsync_HDRADIO(ETAL_HANDLE hDatapath)
{
	tSInt retval;
	tU8 *cmd = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tSInt clen;

	etalToHDRADIOCpTy cmdGetBasicSISData;
	(void)OSAL_pvMemorySet((tVoid *)&cmdGetBasicSISData, 0, sizeof(etalToHDRADIOCpTy));

	cmdGetBasicSISData.m_Opcode = (tU8)HDRADIO_GET_EXT_SIS_DATA_CMD;
	cmdGetBasicSISData.m_FunctionCode = (tU8)GET_BASIC_SIS_DATA;
	cmdGetBasicSISData.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdGetBasicSISData, \
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_queueCommand_HDRADIO(hDatapath, cmd, (tU32)clen);

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdPSDCnfgFieldsEn_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the PSD_Decode (0x93) command, Set_PSD_Cfg_Param (0x03) function and parses the response
 * \details		This function configures which PSD fields the HD Radio device should
 * 				decode.
 *				Fields are selected in PSDFieldsEnableBitmap.
 *				In case, this pointer is NULL,	the selected fields are those needed for the ETAL TextInfo operations
 * 				i.e. Title and Artist. The companion functions #ETAL_cmdPSDCnfgTitleLen_HDRADIO
 * 				and #ETAL_cmdPSDCnfgArtistLen_HDRADIO set the max field length.
 *
 * 				The function locks the HD Radio device for the duration of the operation.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[in]	PSDFieldsEnableBitmap - pointer to Fields Enable bitmap
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdPSDCnfgFieldsEn_HDRADIO(ETAL_HANDLE hReceiver, tU16 *PSDFieldsEnableBitmap)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;

	etalToHDRADIOCpTy cmdSetPsdCnfgParam;
	(void)OSAL_pvMemorySet((tVoid *)&cmdSetPsdCnfgParam, 0, sizeof(etalToHDRADIOCpTy));

	cmdSetPsdCnfgParam.m_Opcode = (tU8)HDRADIO_PSD_DECODE_CMD;
	cmdSetPsdCnfgParam.m_FunctionCode = (tU8)SET_PSD_CNFG_PARAM;
	cmdSetPsdCnfgParam.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	if (PSDFieldsEnableBitmap == NULL)
	{
		clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdSetPsdCnfgParam, ARG_1B_LEN, SET_CONF_PARAM, ARG_1B_LEN, PSD_FIELDS_ENABLED, \
				ARG_2B_LEN, TITLE | ARTIST, END_ARG_MARKER);
	}
	else
	{
		clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdSetPsdCnfgParam, ARG_1B_LEN, SET_CONF_PARAM, ARG_1B_LEN, PSD_FIELDS_ENABLED, \
				ARG_2B_LEN, *PSDFieldsEnableBitmap, END_ARG_MARKER);
	}
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_SETPSD_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)SET_PSD_CNFG_PARAM || 
					resp[ETAL_HDRADIO_CONF_STATUS_OFFSET] != (tU8)PARAMETER_SUCCESSFULLY_SET)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdPSDCnfgLen_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the PSD_Decode (0x93) command, Set_PSD_Cfg_Param (0x03) function and parses the response
 * \details		This function sets the max length of a PSD field that
 * 				will be returned by the HD Radio device
 *
 * 				The function locks the HD Radio device for the duration of the operation.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[in]	field - the PSD field to configure
 * \param[in]	fieldLen - max length of the PSD field
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdPSDCnfgLen_HDRADIO(ETAL_HANDLE hReceiver, tyHDRADIOPSDConfigParam field, tU8 fieldLen)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;
	tU32 maxLen;

	etalToHDRADIOCpTy cmdSetPsdCnfgParam;

	switch (field)
	{
		case PSD_TITLE_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_TITLE;
			break;
		case PSD_ARTIST_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_ARTIST;
			break;
		case PSD_ALBUM_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_ALBUM;
			break;
		case PSD_GENRE_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_GENRE;
			break;
		case PSD_COMMENT_SHORT_CONTENT_DESCRIPTION_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_COMMENT_SHORT;
			break;
		case PSD_COMMENT_ACTUAL_TEXT_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_COMMENT;
			break;
		case PSD_UFID_OWNER_IDENTIFIER_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_UFID;
			break;
		case PSD_COMMERCIAL_PRICE_STRING_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_COMMERCIAL_PRICE;
			break;
		case PSD_COMMERCIAL_CONTACT_URL_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_COMMERCIAL_CONTACT;
			break;
		case PSD_COMMERCIAL_NAME_OF_SELLER_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_COMMERCIAL_SELLER;
			break;
		case PSD_COMMERCIAL_DESCRIPTION_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_COMMERCIAL_DESC;
			break;
		case PSD_XHDR_LENGTH:
			maxLen = ETAL_DEF_MAX_INFO_XHDR;
			break;
		default:
			retval = OSAL_ERROR_INVALID_PARAM;
			goto exit;
	}

	if ((maxLen < (tU32)fieldLen) || (fieldLen < (tU8)0x10))
	{
		retval = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}

	(void)OSAL_pvMemorySet((tVoid *)&cmdSetPsdCnfgParam, 0, sizeof(etalToHDRADIOCpTy));

	cmdSetPsdCnfgParam.m_Opcode = (tU8)HDRADIO_PSD_DECODE_CMD;
	cmdSetPsdCnfgParam.m_FunctionCode = (tU8)SET_PSD_CNFG_PARAM;
	cmdSetPsdCnfgParam.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdSetPsdCnfgParam, ARG_1B_LEN, SET_CONF_PARAM, ARG_1B_LEN, field, \
				ARG_1B_LEN, (tU8)fieldLen, \
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_SETPSD_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)SET_PSD_CNFG_PARAM || 
					resp[ETAL_HDRADIO_CONF_STATUS_OFFSET] != (tU8)PARAMETER_SUCCESSFULLY_SET)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdPSDCnfgLenGet_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the PSD_Decode (0x93) command, Get_PSD_Cfg_Param (0x04) function and parses the response
 * \details		This function gets the length of a PSD field
 *
 * 				The function locks the HD Radio device for the duration of the operation.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[in] field - the PSD field to configure
 * \param[out] fieldLen - current configured max length of the PSD field
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdPSDCnfgLenGet_HDRADIO(ETAL_HANDLE hReceiver, tyHDRADIOPSDConfigParam field, tU8 *fieldLen)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;

	etalToHDRADIOCpTy cmdGetPsdCnfgParam;

	switch (field)
	{
		case PSD_TITLE_LENGTH:
		case PSD_ARTIST_LENGTH:
		case PSD_ALBUM_LENGTH:
		case PSD_GENRE_LENGTH:
		case PSD_COMMENT_SHORT_CONTENT_DESCRIPTION_LENGTH:
		case PSD_COMMENT_ACTUAL_TEXT_LENGTH:
		case PSD_UFID_OWNER_IDENTIFIER_LENGTH:
		case PSD_COMMERCIAL_PRICE_STRING_LENGTH:
		case PSD_COMMERCIAL_CONTACT_URL_LENGTH:
		case PSD_COMMERCIAL_NAME_OF_SELLER_LENGTH:
		case PSD_COMMERCIAL_DESCRIPTION_LENGTH:
		case PSD_XHDR_LENGTH:
			break;
		default :
			retval = OSAL_ERROR_INVALID_PARAM;
			goto exit;
	}

	(void)OSAL_pvMemorySet((tVoid *)&cmdGetPsdCnfgParam, 0, sizeof(etalToHDRADIOCpTy));

	cmdGetPsdCnfgParam.m_Opcode = (tU8)HDRADIO_PSD_DECODE_CMD;
	cmdGetPsdCnfgParam.m_FunctionCode = (tU8)GET_PSD_CNFG_PARAM;
	cmdGetPsdCnfgParam.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdGetPsdCnfgParam, ARG_1B_LEN, field, END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_GETPSD_CFG_MIN_RESP_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)GET_PSD_CNFG_PARAM)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			*fieldLen = resp[ETAL_HDRADIO_GETPSD_CFG_LEN_OFFSET];
	}
	}

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
/***************************
 *
 * ETAL_stringToRadiotext_HDRADIO
 *
 **************************/
/*!
 * \brief		Fills the current info field of the TextInfo
 * \details		The function creates a single string from the *title* and *artist* strings
 * 				with the following format:
 * 				- "title / artist" if both fields are non-empty
 * 				- "title / " if only *title* is non-empty
 * 				- "artist" if only *artist* is non-empty
 *
 * 				The function also sets the 'is new' fields appropriately.
 * \param[out]	pRadiotext - pointer to struct where the function stores the values
 * \param[in]	title - pointer to null-terminated string containing the song title
 * \param[in]	artist - pointer to null-terminated string containing the song artist
 * \callgraph
 * \callergraph
 */
tVoid ETAL_stringToRadiotext_HDRADIO(EtalTextInfo *pRadiotext, tChar *title, tChar *artist)
{
	tU32 length_title = OSAL_u32StringLength(title);

	if (length_title != 0)
	{
		if(length_title < ETAL_DEF_MAX_INFO_TITLE)
		{
			OSAL_szStringNCopy(pRadiotext->m_currentInfo, title, length_title);
		}
		else
		{
			OSAL_szStringNCopy(pRadiotext->m_currentInfo, title, ETAL_DEF_MAX_INFO_TITLE);
			pRadiotext->m_currentInfo[ETAL_DEF_MAX_INFO_TITLE - 1] = '\0';
		}
		OSAL_szStringConcat(pRadiotext->m_currentInfo, " / ");
		pRadiotext->m_currentInfoIsNew = TRUE;
	}

	if (OSAL_u32StringLength(artist) != 0)
	{
		OSAL_szStringNConcat(pRadiotext->m_currentInfo, artist, ETAL_DEF_MAX_INFO_ARTIST);
		pRadiotext->m_currentInfoIsNew = TRUE;
	}
}
#endif // #if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)

/***************************
 *
 * ETAL_cmdPSDDecodeGet_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the PSD_Decode (0x93) command, Get_PSD_Decode (0x02) function and parses the response
 * \details		This function reads the current Title and Artist from the HD Radio device.
 * \details		The function locks the HD Radio device for the duration of the operation.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
  * \param[in]	vI_Service -Service number for which to get the PSD * range 0 to MAX_SERVICE = 7 *
 * \param[out]	title - pointer to null-terminated string containing the song title
 * \param[in]	max_title - max size of the *title* array
 * \param[out]	artist - pointer to null-terminated string containing the song artist
 * \param[in]	max_artist - max size of the *artist* array
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdPSDDecodeGet_HDRADIO(ETAL_HANDLE hReceiver, tS8 vI_Service, tChar *title, tU16 max_title, tChar *artist, tU16 max_artist)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;
	tU8 audioPrgs = (tU8)0;
	tU8 u8PrgBitMask;
	tU32 i;

	etalToHDRADIOCpTy cmdGetPSDDecode;
	(void)OSAL_pvMemorySet((tVoid *)&cmdGetPSDDecode, 0, sizeof(etalToHDRADIOCpTy));

	cmdGetPSDDecode.m_Opcode = (tU8)HDRADIO_PSD_DECODE_CMD;
	cmdGetPSDDecode.m_FunctionCode = (tU8)GET_PSD_DECODE;
	cmdGetPSDDecode.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	/*
	 * u8PrgBitMask could be initialized to more than one Program
	 * (e.g. MPS_AUDIO_MASK | SPS2_AUDIO_MASK)
	 * to request PSD for multiple programs.
	 * Currently we the upper layers support the PSD from the MPS only.
	 */
	//  codex #398347
	// We should get the information from the current selected service, not the MSP
	// i.e. if SPS is selected, get the PSD from the SPS
	//
	// before  :
	// u8PrgBitMask = (tU8)MPS_AUDIO_MASK;
	// 
	// After
	u8PrgBitMask = (0x01 << vI_Service);
		
	// END codex #398347
	
	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdGetPSDDecode, ARG_1B_LEN, u8PrgBitMask, ARG_3B_LEN, RESERVED, ARG_2B_LEN, TITLE | ARTIST, \
				ARG_1B_LEN, NONE_COMMENT, ARG_1B_LEN, NONE_UFID, ARG_1B_LEN, NONE_COMMERCIAL, ARG_2B_LEN, RESERVED, \
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, rlen);
	}
	else
	{
		/*
		 * This loop counts the number of Programs for which we want to get the PSD
		 * The function is prepared to cope with multiple programs but the upper
		 * layers only handle the MSP, thus this loop is redundant but future-proof
		 */
		for(i = 0; i <= (tU32)SPS7_AUDIO_HD_8; i++)
		{
			audioPrgs += ((u8PrgBitMask >> i) & 0x01);
		}
 
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_GETPSD_MIN_FIELD_LEN + (tSInt)audioPrgs * ETAL_HDRADIO_GETPSD_MIN_INFO_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)GET_PSD_DECODE || 
					ETAL_parsePSDDecode_HDRADIO(resp, (tU32)rlen, audioPrgs, title, max_title, artist, max_artist) != OSAL_OK)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}


	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdPSDDecodeGetAsync_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the PSD_Decode (0x93) command, Get_PSD_Decode (0x02) function
 * \details		The function sends the command to read the PSD information and
 * 				does not wait for the answer to arrive from the device.
 *
 * 				The asynchronous behavior is obtained by pushing the command to the
 * 				#ETAL_CommunicationLayer_ThreadEntry_HDRADIO thread which, operating
 * 				in the background, sends it to the device and reads the response after
 * 				a fixed amount of time.
 *
 * 				The response, when available, will be communicated over the *hDatapath*
 *
 * 				The function locks the HD Radio device for the duration of the operation.
 *
 * 				Unlike the non-async functions, the duration of the operation in this case
 * 				depends only on the time needed to send the command.
 * \param[in]	hDatapath - handle of the Datapath on which to
 * 				            provide the decoded information once available.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error communicating with the device, or HD Radio command FIFO full
 * \see			ETAL_cmdPSDDecodeGet_HDRADIO for details on the command format.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdPSDDecodeGetAsync_HDRADIO(ETAL_HANDLE hDatapath)
{
	tSInt retval;
	tU8 *cmd = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tSInt clen;

	EtalBcastDataType dataType;

	etalToHDRADIOCpTy cmdGetPSDDecode;
	(void)OSAL_pvMemorySet((tVoid *)&cmdGetPSDDecode, 0, sizeof(etalToHDRADIOCpTy));

	cmdGetPSDDecode.m_Opcode = (tU8)HDRADIO_PSD_DECODE_CMD;
	cmdGetPSDDecode.m_FunctionCode = (tU8)GET_PSD_DECODE;
	cmdGetPSDDecode.m_CmdType = (tU8)WRITE_TYPE;

	dataType = ETAL_receiverGetDataTypeForDatapath(hDatapath);

	ETAL_getHDRADIODevLock();

	if (ETAL_DATA_TYPE_TEXTINFO == dataType)
	{
		clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdGetPSDDecode, ARG_1B_LEN, MPS_AUDIO_MASK, ARG_3B_LEN, RESERVED, \
					ARG_2B_LEN, TITLE | ARTIST, ARG_1B_LEN, NONE_COMMENT, ARG_1B_LEN, NONE_UFID, \
					ARG_1B_LEN, NONE_COMMERCIAL, ARG_2B_LEN, RESERVED, END_ARG_MARKER);
	}
	else if (ETAL_DATA_TYPE_DATA_SERVICE == dataType)
	{
		/* Ask everything by default since only Fields that are enabled will be returned */
		clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdGetPSDDecode, ARG_1B_LEN, MPS_AUDIO_MASK, ARG_3B_LEN, RESERVED, \
					ARG_2B_LEN, TITLE|ARTIST|ALBUM|GENRE|COMMENT|UFID|COMMERCIAL|XHDR, \
					ARG_1B_LEN, LANGUAGE|SHORT_CONTENT_DESCRIPTION|THE_ACTUAL_TEXT, \
					ARG_1B_LEN, OWNER_IDENTIFIER|IDENTIFIER, \
					ARG_1B_LEN, PRICE_STRING|VALID_UNTIL|CONTACT_URL|RECEIVED_AS|NAME_OF_SELLER|DESCRIPTION, \
					ARG_2B_LEN, RESERVED, END_ARG_MARKER);
	}
	else
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_queueCommand_HDRADIO(hDatapath, cmd, (tU32)clen);

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_SetMRCCnfg_HDRADIO
 *
 **************************/
/*!
 * \brief		Set enable or disable the HD FM MRC
 * \details		The function is used to enable or disable the HD FM MRC configuration
 * 				This function only applies if the MRC function is both supported by the software configuration and it is activated
 * 				This function return no error without doing anything in case MRC is not supported by DCOP.
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[in]freq - frequency to tune on instance #2 (only used if setMrcEnable is true), should be same frequency than instance #1
 * \param[in]	setMrcEnable - boolean indicating if HD FM MRC has to be enabled or disabled
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_SetMRCCnfg_HDRADIO(ETAL_HANDLE hReceiver, tU32 freq, tBool set_mrc_enable)
{
	tSInt retval = OSAL_OK;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen = OSAL_ERROR;
	ETAL_HANDLE hReceiver_instace1, hReceiver_instace2, hReceiver_other_instance;
	tS8 curr_prog;
	HDRADIOToEtalMRCCnfgStatusTy hd_mrc_cnfg_status;
	EtalBcastStandard standard = ETAL_receiverGetStandard(hReceiver);
	etalToHDRADIOCpTy cmdTuneSelect;
	etalToHDRADIODigiAcqStatusTy DigiAcqStatus;
	tyHDRADIOInstanceID InstId;
	tU32 freq_dcop, timeout_cnt;

	/* check if DCOP doesn't support MRC */
	if (ETAL_statusIsHDMRCSupportedServices() == FALSE)
	{
		/* DCOP doesn't support MRC: nothing to do, return no error */
		retval = OSAL_OK;
		goto exit;
	}

	/* get dcop instance Id */
	retval = ETAL_receiverGetHdInstance(hReceiver, &InstId);

	if (retval == OSAL_OK)
	{
		hReceiver_instace1 = hReceiver;
		if (InstId == INSTANCE_2)
		{
			/* Set_MRC_Cnfg and Get_MRC_Cnfg functions only applies to Instance #1: 
			any attempt to send this command to Instance #2 or #3 will not be executed and 
			the status byte of the return LM will indicate an unsupported instance*/
			hReceiver_instace1 = HDRADIO_FAKE_RECEIVER;
			hReceiver_instace2 = hReceiver;
			
		}
		else if (InstId == INSTANCE_1)
		{
			hReceiver_instace1 = hReceiver;
			hReceiver_instace2 = HDRADIO_FAKE_RECEIVER2;
		}
		else
		{
			/* Nothing to do */
		}

		/* get current program */
		ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, NULL, &curr_prog, NULL, NULL);
		if (curr_prog == ETAL_INVALID_PROG)
		{
			curr_prog = (tS8)MPS_AUDIO_HD_1;
		}

		/* check frequency */
		if ((freq == ETAL_INVALID_FREQUENCY) || (freq == 0))
		{
			retval = OSAL_ERROR;
		}
	}

	if (retval == OSAL_OK)
	{
		/* get MRC status */
		if (ETAL_cmdGetMRCCnfg_HDRADIO(hReceiver_instace1, &hd_mrc_cnfg_status) != OSAL_OK)
		{
			retval = OSAL_ERROR;
		}
	}

	if (retval == OSAL_OK)
	{
		if ((set_mrc_enable == TRUE) && (hd_mrc_cnfg_status.BitField.m_MRCEnableStatus == (tU8)0))
		{
			if (standard == ETAL_BCAST_STD_HD_FM)
			{
				/* get Sys_Tune status of current instance */
				timeout_cnt = 0;
				while (((retval = ETAL_cmdGetStatusDigital_HDRADIO(hReceiver, &DigiAcqStatus)) == OSAL_OK) &&
						(DigiAcqStatus.m_TuneOperationType == (tU8)TUNE_OPERATION_PENDING) && (timeout_cnt++ < ETAL_HD_TUNEGETSTATUS_TIMEOUT))
				{
					/* Sys_Tune is one of those commands that requires a delay
					 * before the next command, see #HD_CRITICAL_COMMAND_DELAY */
					(void)OSAL_s32ThreadWait(HD_CRITICAL_COMMAND_DELAY);
				}
				if ((retval == OSAL_OK) && (timeout_cnt >= ETAL_HD_TUNEGETSTATUS_TIMEOUT))
				{
					retval = OSAL_ERROR_TIMEOUT_EXPIRED;
				}

				if (retval == OSAL_OK)
				{
					/* check if same band and frequency */
					freq_dcop = ((tU32)DigiAcqStatus.m_LSBRfChFreq + (256 * (tU32)DigiAcqStatus.m_MSBRfChFreq)) * 10;
					if ((DigiAcqStatus.m_Band != (tU8)FM_BAND) || (freq != freq_dcop))
					{
						/* construct same Tune_Select message and send it to current instance */
						(void)OSAL_pvMemorySet((tVoid *)&cmdTuneSelect, 0, sizeof(etalToHDRADIOCpTy));

						cmdTuneSelect.m_Opcode = (tU8)HDRADIO_SYS_TUNE_CMD;
						cmdTuneSelect.m_FunctionCode = (tU8)TUNE_SELECT;
						cmdTuneSelect.m_CmdType = (tU8)WRITE_TYPE;

						ETAL_getHDRADIODevLock();
						clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneSelect, ARG_1B_LEN, FM_BAND, ARG_2B_LEN, freq/10, ARG_1B_LEN, curr_prog, END_ARG_MARKER);
						if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
						{
							retval = OSAL_ERROR;
						}

						if (retval == OSAL_OK)
						{
							retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
							if (retval != OSAL_OK)
							{
								ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, (tU32)rlen);
							}
							else
							{
								if (rlen < ETAL_HDRADIO_TUNESEL_MIN_FIELD_LEN)
								{
									retval = OSAL_ERROR;
								}
								else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)TUNE_SELECT || 
											resp[ETAL_HDRADIO_VALID_INDICATION_OFFSET] != (tU8)VALID)
								{
									retval = OSAL_ERROR;
								}
								else
								{
									/* Nothing to do */
								}
							}
						}
						ETAL_releaseHDRADIODevLock();
						if (retval == OSAL_OK)
						{
							/* Sys_Tune is one of those commands that requires a delay
							 * before the next MRC Enable command  */
							(void)OSAL_s32ThreadWait(3 * HD_CRITICAL_COMMAND_DELAY);

							/* get Sys_Tune status of current instace */
							timeout_cnt = 0;
							while (((retval = ETAL_cmdGetStatusDigital_HDRADIO(hReceiver, &DigiAcqStatus)) == OSAL_OK) &&
									(DigiAcqStatus.m_TuneOperationType == (tU8)TUNE_OPERATION_PENDING) && (timeout_cnt++ < ETAL_HD_TUNEGETSTATUS_TIMEOUT))
							{
								/* Sys_Tune is one of those commands that requires a delay
								 * before the next command, see #HD_CRITICAL_COMMAND_DELAY */
								(void)OSAL_s32ThreadWait(HD_CRITICAL_COMMAND_DELAY);
							}
							if ((retval == OSAL_OK) && (timeout_cnt >= ETAL_HD_TUNEGETSTATUS_TIMEOUT))
							{
								retval = OSAL_ERROR_TIMEOUT_EXPIRED;
							}
							if (DigiAcqStatus.m_Band == (tU8)IDLE_MODE)
							{
								retval = OSAL_ERROR;
							}
						}
					}
				}

				if (retval == OSAL_OK)
				{
					/* get hReceiver of other instance */
					if (InstId == INSTANCE_1)
					{
						hReceiver_other_instance = HDRADIO_FAKE_RECEIVER2;
						
					}
					else if (InstId == INSTANCE_2)
					{
						hReceiver_other_instance = HDRADIO_FAKE_RECEIVER;
						
					}
					else
					{
						retval = OSAL_ERROR;
					}
				}

				/* get Sys_Tune status of other instance */
				if (retval == OSAL_OK)
				{
					timeout_cnt = 0;
					while (((retval = ETAL_cmdGetStatusDigital_HDRADIO(hReceiver_other_instance, &DigiAcqStatus)) == OSAL_OK) &&
							(DigiAcqStatus.m_TuneOperationType == (tU8)TUNE_OPERATION_PENDING) && (timeout_cnt++ < ETAL_HD_TUNEGETSTATUS_TIMEOUT))
					{
						/* Sys_Tune is one of those commands that requires a delay
						 * before the next command, see #HD_CRITICAL_COMMAND_DELAY */
						(void)OSAL_s32ThreadWait(HD_CRITICAL_COMMAND_DELAY);
					}
					if ((retval == OSAL_OK) && (timeout_cnt >= ETAL_HD_TUNEGETSTATUS_TIMEOUT))
					{
						retval = OSAL_ERROR_TIMEOUT_EXPIRED;
					}
				}

				if (retval == OSAL_OK)
				{
					/* check if same band and frequency */
					freq_dcop = ((tU32)DigiAcqStatus.m_LSBRfChFreq + (256 * (tU32)DigiAcqStatus.m_MSBRfChFreq)) * 10;
					if ((DigiAcqStatus.m_Band != (tU8)FM_BAND) || (freq != freq_dcop))
					{
						/* construct same Tune_Select message and send it to other instance */
						(void)OSAL_pvMemorySet((tVoid *)&cmdTuneSelect, 0, sizeof(etalToHDRADIOCpTy));

						cmdTuneSelect.m_Opcode = (tU8)HDRADIO_SYS_TUNE_CMD;
						cmdTuneSelect.m_FunctionCode = (tU8)TUNE_SELECT;
						cmdTuneSelect.m_CmdType = (tU8)WRITE_TYPE;

						ETAL_getHDRADIODevLock();
						clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneSelect, ARG_1B_LEN, FM_BAND, ARG_2B_LEN, freq/10, ARG_1B_LEN, curr_prog, END_ARG_MARKER);
						if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
						{
							retval = OSAL_ERROR;
						}

						if (retval == OSAL_OK)
						{
							retval = ETAL_sendCommandTo_HDRADIO(hReceiver_other_instance, cmd, (tU32)clen, &resp, &rlen);
							if (retval != OSAL_OK)
							{
								ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, (tU32)rlen);
							}
							else
							{
								if (rlen < ETAL_HDRADIO_TUNESEL_MIN_FIELD_LEN)
								{
									retval = OSAL_ERROR;
								}
								else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)TUNE_SELECT || 
											resp[ETAL_HDRADIO_VALID_INDICATION_OFFSET] != (tU8)VALID)
								{
									retval = OSAL_ERROR;
								}
								else
								{
									/* Nothing to do */
								}
							}
						}
						ETAL_releaseHDRADIODevLock();
						if (retval == OSAL_OK)
						{
							/* Sys_Tune is one of those commands that requires a delay
							 * before the next MRC Enable command  */
							(void)OSAL_s32ThreadWait(3 * HD_CRITICAL_COMMAND_DELAY);

							/* get Sys_Tune status of other instace */
							timeout_cnt = 0;
							while (((retval = ETAL_cmdGetStatusDigital_HDRADIO(hReceiver_other_instance, &DigiAcqStatus)) == OSAL_OK) &&
									(DigiAcqStatus.m_TuneOperationType == (tU8)TUNE_OPERATION_PENDING) && (timeout_cnt++ < ETAL_HD_TUNEGETSTATUS_TIMEOUT))
							{
								/* Sys_Tune is one of those commands that requires a delay
								 * before the next command, see #HD_CRITICAL_COMMAND_DELAY */
								(void)OSAL_s32ThreadWait(HD_CRITICAL_COMMAND_DELAY);
							}
							if ((retval == OSAL_OK) && (timeout_cnt >= ETAL_HD_TUNEGETSTATUS_TIMEOUT))
							{
								retval = OSAL_ERROR_TIMEOUT_EXPIRED;
							}
							if (DigiAcqStatus.m_Band == (tU8)IDLE_MODE)
							{
								retval = OSAL_ERROR;
							}
						}
					}
				}

				if (retval == OSAL_OK)
				{
					/* HD FM MRC Enable */
					if ((ETAL_cmdSetMRCCnfg_HDRADIO(hReceiver_instace1, TRUE, &hd_mrc_cnfg_status) != OSAL_OK) ||
						((hd_mrc_cnfg_status.BitField.m_MRCEnableStatus == (tU8)0) || 
						(hd_mrc_cnfg_status.BitField.m_FrequencyMismatchStatus != (tU8)0) ||
						(hd_mrc_cnfg_status.BitField.m_BandMismatchStatus != (tU8)0)))
					{
						retval = OSAL_ERROR;
					}
				}
			}
			else
			{
				retval = OSAL_ERROR;
			}
		}
		else if ((set_mrc_enable == FALSE) && (hd_mrc_cnfg_status.BitField.m_MRCEnableStatus != (tU8)0))
		{
			/* HD FM MRC Disable */
			if ((ETAL_cmdSetMRCCnfg_HDRADIO(hReceiver_instace1, FALSE, &hd_mrc_cnfg_status) != OSAL_OK) ||
				(hd_mrc_cnfg_status.BitField.m_MRCEnableStatus != (tU8)0) ||
				(hd_mrc_cnfg_status.BitField.m_FrequencyMismatchStatus != (tU8)0) ||
				(hd_mrc_cnfg_status.BitField.m_BandMismatchStatus != (tU8)0))
			{
				retval = OSAL_ERROR;
			}

			/* Set_MRC_Cnfg is one of those commands that require a delay
			 * before the next Sys_Tune command */
			(void)OSAL_s32ThreadWait(HD_CRITICAL_COMMAND_DELAY);

			/* construct same Tune_Select message and send it to instance #2 */
			(void)OSAL_pvMemorySet((tVoid *)&cmdTuneSelect, 0, sizeof(etalToHDRADIOCpTy));

			cmdTuneSelect.m_Opcode = (tU8)HDRADIO_SYS_TUNE_CMD;
			cmdTuneSelect.m_FunctionCode = (tU8)TUNE_SELECT;
			cmdTuneSelect.m_CmdType = (tU8)WRITE_TYPE;

			ETAL_getHDRADIODevLock();
			clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdTuneSelect, ARG_1B_LEN, FM_BAND, ARG_2B_LEN, freq/10, ARG_1B_LEN, curr_prog, END_ARG_MARKER);
			if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
			{
				retval = OSAL_ERROR;
			}

			if (retval == OSAL_OK)
			{
				retval = ETAL_sendCommandTo_HDRADIO(hReceiver_instace2, cmd, (tU32)clen, &resp, &rlen);
				if (retval != OSAL_OK)
				{
					ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, (tU32)rlen);
				}
				else
				{
					if (rlen < ETAL_HDRADIO_TUNESEL_MIN_FIELD_LEN)
					{
						retval = OSAL_ERROR;
					}
					else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)TUNE_SELECT || 
								resp[ETAL_HDRADIO_VALID_INDICATION_OFFSET] != (tU8)VALID)
					{
						retval = OSAL_ERROR;
					}
					else
					{
						/* Nothing to do */
					}
				}
			}
			ETAL_releaseHDRADIODevLock();
			if (retval == OSAL_OK)
			{
				/* Sys_Tune is one of those commands that requires a delay
				 * before the next MRC Enable command */
				(void)OSAL_s32ThreadWait(HD_CRITICAL_COMMAND_DELAY);

				/* get Sys_Tune status of other instace */
				timeout_cnt =0;
				while (((retval = ETAL_cmdGetStatusDigital_HDRADIO(hReceiver_instace2, &DigiAcqStatus)) == OSAL_OK) &&
						(DigiAcqStatus.m_TuneOperationType == (tU8)TUNE_OPERATION_PENDING) && (timeout_cnt++ < ETAL_HD_TUNEGETSTATUS_TIMEOUT))
				{
					/* Sys_Tune is one of those commands that requires a delay
					 * before the next command, see #HD_CRITICAL_COMMAND_DELAY */
					(void)OSAL_s32ThreadWait(HD_CRITICAL_COMMAND_DELAY);
				}
				if ((retval == OSAL_OK) && (timeout_cnt >= ETAL_HD_TUNEGETSTATUS_TIMEOUT))
				{
					retval = OSAL_ERROR_TIMEOUT_EXPIRED;
				}
				if (DigiAcqStatus.m_Band == (tU8)IDLE_MODE)
				{
					retval = OSAL_ERROR;
				}
			}
		}
		else
		{
			/* Nothing to do */
		}
	}

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdSetMRCCnfg_HDRADIO
 *
 **************************/
/*!
 * \brief		Set enable or disable the HD FM MRC
 * \details		The function is used to enable or disable the HD FM MRC configuration
 				This function only applies if the MRC function is both supported by the software configuration and it is activated
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[in]	setMrcEnable - boolean indicating if HD FM MRC has to be enabled or disabled
 * \param[out]	hd_mrc_cnfg_status - Returned HD FM MRC configuration status
 *            	                     Bit 0: MRC Enable Status
 *            	                       0: MRC Disabled
 *            	                       1: MRC Enabled
 *            	                     Bit 1: Frequency Mismatch Status
 *            	                       0: No Frequency Mismatch
 *            	                       1: Tuner 1 and Tuner 2 frequency mismatch. Tuner 2 demod output is disabled.
 *            	                     Bit 2: Band Mismatch Status
 *            	                       0: No Band Mismatch
 *            	                       1: Tuner 1 and Tuner 2 are not both tuned to the FM band
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSetMRCCnfg_HDRADIO(ETAL_HANDLE hReceiver, tBool set_mrc_enable, HDRADIOToEtalMRCCnfgStatusTy *hd_mrc_cnfg_status)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;
	etalToHDRADIOCpTy cmdSetMRCCnfg;

	/* Check parameter */
	if ((set_mrc_enable != FALSE) && (set_mrc_enable != TRUE))
	{
		retval = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}

	/* Build message */
	(void)OSAL_pvMemorySet((tVoid *)&cmdSetMRCCnfg, 0, sizeof(etalToHDRADIOCpTy));

	cmdSetMRCCnfg.m_Opcode = (tU8)HDRADIO_IBOC_CNTRL_CNFG_CMD;
	cmdSetMRCCnfg.m_FunctionCode = (tU8)SET_MRC_CNFG;
	cmdSetMRCCnfg.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdSetMRCCnfg, 
			ARG_1B_LEN, set_mrc_enable, 
			END_ARG_MARKER);

	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	/* Send message */
	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Error Sending the HDRADIO Set_MRC_Cnfg command");
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, (tU32)rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_IBOC_CNTRL_CNFG_SET_MRC_CNFG_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)SET_MRC_CNFG)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			hd_mrc_cnfg_status->mrc_cnfg_status = resp[ETAL_HDRADIO_IBOC_CNTRL_CNFG_MRC_CNFG_STATUS_OFFSET];
		}
	}

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdGetMRCCnfg_HDRADIO
 *
 **************************/
/*!
 * \brief		Get the HD FM MRC state
 * \details		The function is used to get the HD FM MRC configuration
 				This function only applies if the MRC function is both supported by the software configuration and it is activated
 				This function only applies to Instance #1: any attempt to send this command to Instance #2 or #3 will not be executed
 * \param[in]	hReceiver - handle of the HD Radio Receiver
 * \param[out]	hd_mrc_cnfg_status - Returned HD FM MRC configuration status
 *            	                     Bit 0: MRC Enable Status
 *            	                       0: MRC Disabled
 *            	                       1: MRC Enabled
 *            	                     Bit 1: Frequency Mismatch Status
 *            	                       0: No Frequency Mismatch
 *            	                       1: Tuner 1 and Tuner 2 frequency mismatch. Tuner 2 demod output is disabled.
 *            	                     Bit 2: Band Mismatch Status
 *            	                       0: No Band Mismatch
 *            	                       1: Tuner 1 and Tuner 2 are not both tuned to the FM band
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetMRCCnfg_HDRADIO(ETAL_HANDLE hReceiver, HDRADIOToEtalMRCCnfgStatusTy *hd_mrc_cnfg_status)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;
	etalToHDRADIOCpTy cmdGetMRCCnfg;

	/* Build message */
	(void)OSAL_pvMemorySet((tVoid *)&cmdGetMRCCnfg, 0, sizeof(etalToHDRADIOCpTy));

	cmdGetMRCCnfg.m_Opcode = (tU8)HDRADIO_IBOC_CNTRL_CNFG_CMD;
	cmdGetMRCCnfg.m_FunctionCode = (tU8)GET_MRC_CNFG;
	cmdGetMRCCnfg.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdGetMRCCnfg, 
			END_ARG_MARKER);

	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	/* Send message */
	retval = ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Error Sending the HDRADIO Set_MRC_Cnfg command");
		ETAL_sendCommunicationErrorEvent_HDRADIO(hReceiver, retval, resp, (tU32)rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_IBOC_CNTRL_CNFG_GET_MRC_CNFG_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)GET_MRC_CNFG)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			hd_mrc_cnfg_status->mrc_cnfg_status = resp[ETAL_HDRADIO_IBOC_CNTRL_CNFG_MRC_CNFG_STATUS_OFFSET];
		}
	}

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdGetSupportedServices_HDRADIO
 *
 **************************/
/*!
 * \brief		Get the HD Supported Services
 * \details		The function is used to get the HD Supported Services
 * \param[out]	rx_sw_cnfg - Returned HD Supported Services
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetSupportedServices_HDRADIO(etalHDRXSWCnfgTy *rx_sw_cnfg)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;
	etalToHDRADIOCpTy cmdGetMRCCnfg;

	/* Build message */
	(void)OSAL_pvMemorySet((tVoid *)&cmdGetMRCCnfg, 0, sizeof(etalToHDRADIOCpTy));

	cmdGetMRCCnfg.m_Opcode = (tU8)HDRADIO_SYS_CNTRL_CNFG_CMD;
	cmdGetMRCCnfg.m_FunctionCode = (tU8)GET_SUPPORTED_SERVICES;
	cmdGetMRCCnfg.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdGetMRCCnfg, 
			END_ARG_MARKER);

	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	/* Send message */
	retval = ETAL_sendCommandTo_HDRADIO((ETAL_HANDLE)HDRADIO_FAKE_RECEIVER, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Error Sending the HDRADIO Get_Supported_Services command");
		ETAL_sendCommunicationErrorEvent_HDRADIO((ETAL_HANDLE)HDRADIO_FAKE_RECEIVER, retval, resp, (tU32)rlen);
	}
	else
	{
		/* Checking the Return Data */
		if (rlen < ETAL_HDRADIO_SYSCNTRL_GET_SUPPORTED_SERVICES_MIN_FIELD_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)GET_SUPPORTED_SERVICES)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			(void)OSAL_pvMemoryCopy((tPVoid)(rx_sw_cnfg->RX_SW_Cnfg), (tPVoid) &(resp[ETAL_HDRADIO_SYSCNTRL_GET_SUPPORTED_SERVICES_RX_SW_CNFG_OFFSET]), sizeof(etalHDRXSWCnfgTy));
		}
	}

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdGetQuality_HDRADIO
 *
 **************************/
/*!
 * \brief		Reads the quality information from the HD Radio device
 * \details		The function uses the #ETAL_cmdGetStatusDigital_HDRADIO and
 * 				#ETAL_cmdGetStatusDSQM_HDRADIO functions to read the current
 * 				HD Radio quality from the device.
 * \param[in]	hReceiver - handle of the HD Radio receiver from which to read the quality
 * \param[out]	p - pointer to a structure where the function stores the quality.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetQuality_HDRADIO(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p)
{
	etalToHDRADIODigiAcqStatusTy digi_acq_status;
#if defined (CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) && !defined (CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA)
	etalToHDRADIOAlignStatusTy vl_AudioAlignmentStatus;
#endif
	OSAL_tMSecond timestamp;
	tSInt retval = OSAL_OK;
	tU32 local_dsqm;

	retval = ETAL_cmdGetStatusDigital_HDRADIO(hReceiver, &digi_acq_status);
	if (retval == OSAL_OK)
	{
		retval = ETAL_cmdGetStatusDSQM_HDRADIO(hReceiver, &local_dsqm);
	}
#if (defined(CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) && !defined (CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA))
	if (retval == OSAL_OK)
	{
		retval = ETAL_cmdGetAlignStatus_HDRADIO(hReceiver, &vl_AudioAlignmentStatus);
	}
#endif
	if (retval == OSAL_OK)
	{
		timestamp = OSAL_ClockGetElapsedTime();
		p->m_TimeStamp = timestamp;
		p->m_standard = ETAL_receiverGetStandard(hReceiver);

		if(((digi_acq_status.m_DigiAcquisitionStatus & (tU8)HD_ACQUIRED) == (tU8)HD_ACQUIRED) ||
		   ((digi_acq_status.m_DigiAcquisitionStatus & (tU8)SIS_ACQUIRED) == (tU8)SIS_ACQUIRED) ||
		   ((digi_acq_status.m_DigiAcquisitionStatus & (tU8)DIGITAL_AUDIO_ACQUIRED) == (tU8)DIGITAL_AUDIO_ACQUIRED))
		{
			p->EtalQualityEntries.hd.m_isValidDigital = TRUE;
		}
		else
		{
			p->EtalQualityEntries.hd.m_isValidDigital = FALSE;
		}

		p->EtalQualityEntries.hd.m_QI = (tU32)digi_acq_status.m_AudioQI;
		p->EtalQualityEntries.hd.m_CdToNo = (tU32)digi_acq_status.m_CdNo;
		p->EtalQualityEntries.hd.m_DSQM = local_dsqm;
#if defined (CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) && !defined (CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA)
		p->EtalQualityEntries.hd.m_AudioAlignment = (vl_AudioAlignmentStatus.m_AudioIndicator.BitField.m_AlignmentIndicator == (tU8)1)?true:false;
#else
		p->EtalQualityEntries.hd.m_AudioAlignment = false;
#endif
	}
	else
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_GenericError, 0, NULL, 0);
	}

	return retval;
}


/***************************
 *
 * ETAL_cmdSetAlignParam_HDRADIO
 *
 **************************/
/*!
 * \brief		Set the alignement algorithm (AAA) parameter the HD Radio device
 * \details		The function is used to set the  AAA configuration
 				it controls 
 				- which type of alignment is active (for AM, for FM) 
 				- action when the alignment is not found (remain in FM, play digital without alignment)
 				- action on the level (gain) alignment (for AM, for FM)
 * \param[in]	vI_alignmentActiveForAM - boolean indicating if alignment is active for HD - AM case
 * \param[in]	vI_alignmentActiveForFM -  boolean indicating if alignment is active for HD - FM case
 * \param[in]	vI_enableAMGain         -  boolean indicating if  AM gain level alignement is applied
 * \param[in]	vI_enableFMGain         -  boolean indicating if  FM gain level alignement is applied 
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSetAlignParam_HDRADIO(tBool  vI_alignmentActiveForAM, tBool vI_alignmentActiveForFM, tBool vI_enableAMGain, tBool vI_enableFMGain)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;
	tU8 vl_autoAlignByte = (tU8)0;
	tU8	vl_alignmentNotFoundAction = (tU8)0;
	tU8 vl_txGain = (tU8)0;

	etalToHDRADIOCpTy cmdSetAlignParam;
	(void)OSAL_pvMemorySet((tVoid *)&cmdSetAlignParam, 0, sizeof(etalToHDRADIOCpTy));

	cmdSetAlignParam.m_Opcode = (tU8)HDRADIO_IBOC_CNTRL_CNFG_CMD;
	cmdSetAlignParam.m_FunctionCode = (tU8)SET_ALIGN_PARAM;
	cmdSetAlignParam.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	// set the auto alignment action byte
	// bit 0 = 
	/* Bit[0]:
	1 = Enable AM Auto Alignment function
	0 = Disable AM Auto Alignment function
	Bit[1]:
	1 = Enable FM Auto Alignment function
	0 = Disable FM Auto Alignment function
	*/

	vl_autoAlignByte = (tU8)((TRUE == vI_alignmentActiveForAM)?1:0) << 0;
	vl_autoAlignByte |= (tU8)((TRUE == vI_alignmentActiveForFM)?1:0) << 1;

	
	// set the action on alignment not found
	/*
	Byte 2: was Alignment Not Found Action, is now reserved
	Specifies baseband action when alignment cannot be determined
	Bit[0]
	0 = Never blend to digital. Alignment algorithm will continue to be run at least every five seconds.
	1 = Blend immediately to digital. No alignment correction will be applied.
	*/
	vl_alignmentNotFoundAction = (tU8)0;

	// set the Tx Gain action
	/*Bit[0]:
	1 = Enable AM TX_GAIN update with Level Alignment value
	0 = Disable AM TX_GAIN update with Level Alignment value
	Bit[1]:
	1 = Enable FM TX_GAIN update with Level Alignment value
	0 = Disable FM TX_GAIN update with Level Alignment value
	*/
	vl_txGain = (tU8)((TRUE == vI_enableAMGain)?1:0) << 0;
	vl_txGain |= (tU8)((TRUE == vI_enableFMGain)?1:0) << 1;
	
	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdSetAlignParam, 
			ARG_1B_LEN, vl_autoAlignByte, 
			ARG_1B_LEN, vl_alignmentNotFoundAction, 
			ARG_1B_LEN, vl_txGain, 
			END_ARG_MARKER);
	
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
		goto exit;
	}

	retval = ETAL_sendCommandTo_HDRADIO((ETAL_HANDLE)HDRADIO_FAKE_RECEIVER, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_HDRADIO(ETAL_INVALID_HANDLE, retval, resp, (tU32)rlen);
	}
	else
	{
		// store the new status of the configuration 
		v_alignmentConfiguration.m_enableAutoAlignmentForAM = vI_alignmentActiveForAM;
		v_alignmentConfiguration.m_enableAutoAlignmentForFM = vI_alignmentActiveForFM;
	}

	ETAL_releaseHDRADIODevLock();

exit:
	return retval;
}


#if defined(CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA)
/***************************
 *
 * ETAL_cmdDisableAAA_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the Sys_Cntrl_Cnfg (0x83) command to disable AAA
 * \details		Reads the currently activated services using function
 * 				Get_Activated_Services (0x01), masks out the AAA and rewrites
 * 				the configuration using the Set_Activated_Services (0x02)
 * \remark		Should be called only immediately after DCOP boot
 * \remark		ets the #v_alignmentConfiguration to the actual value
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdDisableAAA_HDRADIO(tVoid)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;
	tU8 bitmap[4];
	tBool isAAAenabled;

	etalToHDRADIOCpTy cmdSysCtrlConfig;

	ETAL_getHDRADIODevLock();

	/* The HDRADIO_SYS_CNTRL_CNFG_CMD/SET_ACTIVATED_SERVICES must be issued only
	 * from idle state, otherwise it will be rejected.
	 *
	 * Send change band to put DCOP in Idle mode (see Table 5.1 of DCOP spec */

	OSAL_pvMemorySet((tVoid *)&cmdSysCtrlConfig, 0, sizeof(etalToHDRADIOCpTy));

	cmdSysCtrlConfig.m_Opcode = (tU8)HDRADIO_BAND_SELECT_CMD;
	cmdSysCtrlConfig.m_FunctionCode = (tU8)IDLE_MODE;
	cmdSysCtrlConfig.m_CmdType = (tU8)WRITE_TYPE;

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdSysCtrlConfig, \
				END_ARG_MARKER);

	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		return OSAL_ERROR;
	}

	/* the DCOP InstanceId is not used for this command but needs to be set to avoid errors */
	retval = ETAL_sendCommandToInstance_HDRADIO(INSTANCE_1, cmd, (tU32)clen, &resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent((ETAL_HANDLE)0, EtalCommStatus_GenericError, 0, NULL, 0);
	}
	else
	{
		/* Band_Select is one of those commands that require a delay
		 * before the next command, see #HD_CRITICAL_COMMAND_DELAY */
		OSAL_s32ThreadWait(HD_CRITICAL_COMMAND_DELAY);

		/* Get activated services */

		OSAL_pvMemorySet((tVoid *)&cmdSysCtrlConfig, 0, sizeof(etalToHDRADIOCpTy));

		cmdSysCtrlConfig.m_Opcode = (tU8)HDRADIO_SYS_CNTRL_CNFG_CMD;
		cmdSysCtrlConfig.m_FunctionCode = (tU8)GET_ACTIVATED_SERVICES;
		cmdSysCtrlConfig.m_CmdType = (tU8)WRITE_TYPE;

		clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdSysCtrlConfig, \
					END_ARG_MARKER);

		if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
		{
			ETAL_releaseHDRADIODevLock();
			return OSAL_ERROR;
		}

		/* the DCOP InstanceId is not used for this command but needs to be set to avoid errors */
		retval = ETAL_sendCommandToInstance_HDRADIO(INSTANCE_1, cmd, (tU32)clen, &resp, &rlen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent((ETAL_HANDLE)0, EtalCommStatus_GenericError, 0, NULL, 0);
		}
		else
		{
			if (rlen < ETAL_HDRADIO_SYSCNTRL_GET_MIN_FIELD_LEN)
			{
				retval = OSAL_ERROR;
			}
			else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)GET_ACTIVATED_SERVICES)
			{
				retval = OSAL_ERROR;
			}
			else
			{
				bitmap[0] = resp[ETAL_HDRADIO_SW_CONFIG_OFFSET + 0];
				bitmap[1] = resp[ETAL_HDRADIO_SW_CONFIG_OFFSET + 1];
				bitmap[2] = resp[ETAL_HDRADIO_SW_CONFIG_OFFSET + 2];
				bitmap[3] = resp[ETAL_HDRADIO_SW_CONFIG_OFFSET + 3];
			}

			/* check if AAA is enbled, needed to update global status below */

			if (((bitmap[3] & (tU8)0x20) == (tU8)0x20) && // Automatic Audio Alignment bit
				((bitmap[1] & (tU8)0x01) == (tU8)0x01))   // Aux1 Audio Input
			{
				isAAAenabled = TRUE;
			}
			else
			{
				isAAAenabled = FALSE;
			}

			/* save the global status in case of premature exit from errors below */

			v_alignmentConfiguration.m_enableAutoAlignmentForAM = isAAAenabled;
			v_alignmentConfiguration.m_enableAutoAlignmentForFM = isAAAenabled;

			/* mask out the AAA */

			bitmap[3] &= ~(0x20);

			/* resend the configuration */

			OSAL_pvMemorySet((tVoid *)&cmdSysCtrlConfig, 0, sizeof(etalToHDRADIOCpTy));

			cmdSysCtrlConfig.m_Opcode = (tU8)HDRADIO_SYS_CNTRL_CNFG_CMD;
			cmdSysCtrlConfig.m_FunctionCode = (tU8)SET_ACTIVATED_SERVICES;
			cmdSysCtrlConfig.m_CmdType = (tU8)WRITE_TYPE;

			clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdSysCtrlConfig, \
						ARG_1B_LEN, bitmap[0], \
						ARG_1B_LEN, bitmap[1], \
						ARG_1B_LEN, bitmap[2], \
						ARG_1B_LEN, bitmap[3], \
						END_ARG_MARKER);
			if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
			{
				ETAL_releaseHDRADIODevLock();
				return OSAL_ERROR;
			}

			/* the DCOP InstanceId is not used for this command but needs to be set to avoid errors */
			retval = ETAL_sendCommandToInstance_HDRADIO(INSTANCE_1, cmd, (tU32)clen, &resp, &rlen);
			if (retval != OSAL_OK)
			{
				ETAL_sendCommunicationErrorEvent((ETAL_HANDLE)0, EtalCommStatus_GenericError, 0, NULL, 0);
			}
			else
			{
				if (rlen < ETAL_HDRADIO_SYSCNTRL_SET_MIN_FIELD_LEN)
				{
					retval = OSAL_ERROR;
				}
				else if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)SET_ACTIVATED_SERVICES)
				{
					retval = OSAL_ERROR;
				}
				else if (resp[ETAL_HDRADIO_SW_CONFIG_STATUS_OFFSET] != (tU8)0x01)
				{
					retval = OSAL_ERROR;
				} 
				else
				{
					isAAAenabled = FALSE;
				}

				/* update the global status */
				v_alignmentConfiguration.m_enableAutoAlignmentForAM = isAAAenabled;
				v_alignmentConfiguration.m_enableAutoAlignmentForFM = isAAAenabled;
			}
		}
	}

	ETAL_releaseHDRADIODevLock();
	return retval;
}
#endif // CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA


#if defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
/***************************
 *
 * ETAL_PSDChangePeriodicFunc
 *
 **************************/
/*!
 * \brief		Periodically reads the PSD status from the HD Radio DCOP
 * \details		This function is invoked periodically by the ETAL internal
 * 				callback mechanism (#ETAL_intCbSchedulePeriodicCallbacks).
 * 				It pushes commands to the #ETAL_CommunicationLayer_ThreadEntry_HDRADIO
 * 				to read the Digital Status and the PSD.
 *
 * 				The function is normally started when the user invokes the
 * 				#etaltml_start_textinfo function and stopped either
 * 				by explicit call to #etaltml_stop_textinfo or implicitly
 * 				by destroying the Receiver. The latter case is managed
 * 				by registering a dedicated callback (see #ETAL_PSDChangeStopPeriodicFunc).
 * \param[in]	hGeneric - handle of the Datapath to be used for the TextInfo processing
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_PSDChangePeriodicFunc(ETAL_HANDLE hGeneric)
{
	ETAL_HANDLE hDatapath;
#ifdef HD_MONITORING
	tU8 vl_acq_status;
	tS8 vl_curr_prog;
	tU32 vl_avail_prog_num;
#endif

	if (!ETAL_handleIsReceiver(hGeneric))
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}

	hDatapath = ETAL_receiverGetDatapathFromDataType(hGeneric, ETAL_DATA_TYPE_TEXTINFO);
    if (hDatapath != ETAL_INVALID_HANDLE)
    {
#ifdef HD_MONITORING
		// Get the HD sync status,
		// do not request PSD if status is not acquired

		/*
		 * If digital audio is aquired the current program may be the MPS or
		 * one of the SPS (if present)
		 */
		ETAL_receiverGetRadioInfo_HDRADIO(hGeneric, &vl_acq_status, &vl_curr_prog, &vl_avail_prog_num, NULL);
		if ((vl_acq_status & (tU8)HD_ACQUIRED) != (tU8)HD_ACQUIRED)
		{
			// HD not present
			// nothing to do
			goto exit;		
		}

#else
		/*
		 * this call is required because the PSD processing code in 
		 * ETAL_CommunicationLayer_ThreadEntry_HDRADIO assumes the digital
		 * status is known
		 */
		if (ETAL_cmdGetStatusDigitalAsync_HDRADIO(hDatapath) != OSAL_OK)
		{
			goto exit;
		}
#endif
			
		if (ETAL_cmdPSDDecodeGetAsync_HDRADIO(hDatapath) != OSAL_OK)
		{
			goto exit;
		}
    }

	// TMP change : get the SIS
	(void)ETAL_pollSISChangeAsync_HDRADIO(hDatapath);

exit:
	return;	
}

/***************************
 *
 * ETAL_PSDChangeStopPeriodicFunc
 *
 **************************/
/*!
 * \brief		Stops the periodic PSD polling function
 * \details		This function is only invoked at Receiver destruction
 * 				to stop the background PSD polling function. It is
 * 				registered as an internal callback of type #callAtReceiverDestroy.
 * \param[in]	hGeneric - handle of the Receiver involved in the seek operation
 * \param[in]	param - unused, ignored
 * \param[in]	param_len - unused, ignored
 * \param[in]	context - unused, ignored
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_PSDChangeStopPeriodicFunc(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
	ETAL_HANDLE hReceiver;

	if (!ETAL_handleIsReceiver(hGeneric))
	{
		ASSERT_ON_DEBUGGING(0);
	}
	else
	{
		hReceiver = hGeneric;
		/*
		 * if we were invoked after the etaltml_stop_radiotext
		 * the ETAL_PSDChangePeriodicFunc was already de-registered
		 * so an error would not indicate abnormal situation
		 */
		(LINT_IGNORE_RET) ETAL_intCbDeregisterPeriodic(&ETAL_PSDChangePeriodicFunc, hReceiver);
	}
	return;
}

/***************************
 *
 * ETAL_cmdStartPSD_HDRADIO
 *
 **************************/
/*!
 * \brief		Starts the background periodic PSD polling
 * \details		This function registers the function that periodically
 * 				checks the PSD for changes (#ETAL_PSDChangePeriodicFunc) and
 * 				the function that de-registers it at Receiver destruction
 * 				(#ETAL_PSDChangeStopPeriodicFunc).
 * \param[in]	hReceiver - handle of the HD Radio Receiver on which to read the PSD
 * \return		ETAL_RET_SUCCESS
 * \return		the error returned by #ETAL_intCbRegisterPeriodic or #ETAL_intCbRegister
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_cmdStartPSD_HDRADIO(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;

	if (FALSE == ETAL_intCbIsRegisteredPeriodic(&ETAL_PSDChangePeriodicFunc, hReceiver))
	{
		ret = ETAL_intCbRegisterPeriodic(&ETAL_PSDChangePeriodicFunc, hReceiver, ETAL_HDRADIOPSD_INTCB_DELAY);
		if (ret == ETAL_RET_SUCCESS)
		{
			ret = ETAL_intCbRegister(callAtReceiverDestroy, &ETAL_PSDChangeStopPeriodicFunc, hReceiver, ETAL_INTCB_CONTEXT_UNUSED);
		}
	}
	else
	{
		// already registered
		// return ok and nothing to do
		ret = ETAL_RET_SUCCESS;
	}
	
	return ret;
}


/***************************
 *
 * ETAL_cmdStopPSD_HDRADIO
 *
 **************************/
/*!
 * \brief		Stops all the internal callback functions used to poll PSD changes
 * \param[in]	hReceiver - handle of the HD Radio Receiver used to read the PSD
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_ERROR
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_cmdStopPSD_HDRADIO(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS, ret2 = ETAL_RET_SUCCESS;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	if (TRUE == ETAL_intCbIsRegisteredPeriodic(&ETAL_PSDChangePeriodicFunc, hReceiver))
	{
		ret = ETAL_intCbDeregisterPeriodic(&ETAL_PSDChangePeriodicFunc, hReceiver);
		ret2 = ETAL_intCbDeregister(callAtReceiverDestroy, &ETAL_PSDChangeStopPeriodicFunc, hReceiver);
	}
	else
	{
		// no registered callback, so text info already stopped !
		// all is fine
	}

	if ((ret != ETAL_RET_SUCCESS) || (ret2 != ETAL_RET_SUCCESS))
	{
		retval = ETAL_RET_ERROR;
	}
	
	return retval;
}
#endif // CONFIG_ETALTML_HAVE_RADIOTEXT

/***************************
 *
 * ETAL_PSDChangeDataServPeriodicFunc
 *
 **************************/
/*!
 * \brief		Periodically reads the PSD status from the HD Radio DCOP
 * \details		This function is invoked periodically by the ETAL internal
 * 				callback mechanism (#ETAL_intCbSchedulePeriodicCallbacks).
 * 				It pushes commands to the #ETAL_CommunicationLayer_ThreadEntry_HDRADIO
 * 				to read the Digital Status and the PSD.
 *
 * 				The function is normally started when the user enables PSD data service
 * 				and stopped either
 * 				by disabling PSD data service, or implicitly
 * 				by destroying the Receiver. The latter case is managed by registering
 * 				a dedicated callback (see #ETAL_PSDChangeStopDataServPeriodicFunc).
 * \param[in]	hGeneric - handle of the Datapath to be used for the PSD processing
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_PSDChangeDataServPeriodicFunc(ETAL_HANDLE hGeneric)
{
	ETAL_HANDLE hDatapath;

	if (!ETAL_handleIsReceiver(hGeneric))
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}

	hDatapath = ETAL_receiverGetDatapathFromDataType(hGeneric, ETAL_DATA_TYPE_DATA_SERVICE);
	if (hDatapath != ETAL_INVALID_HANDLE)
  {
		/*
		 * this call is required because the PSD processing code in 
		 * ETAL_CommunicationLayer_ThreadEntry_HDRADIO assumes the digital
		 * status is known
		 */
		if (ETAL_cmdGetStatusDigitalAsync_HDRADIO(hDatapath) != OSAL_OK)
		{
			goto exit;
		}
		if (ETAL_cmdPSDDecodeGetAsync_HDRADIO(hDatapath) != OSAL_OK)
		{
			goto exit;
		}
  }

exit:
	return;
}

/***************************
 *
 * ETAL_PSDChangeStopDataServPeriodicFunc
 *
 **************************/
/*!
 * \brief		Stops the periodic PSD polling function
 * \details		This function is only invoked at Receiver destruction
 * 				to stop the background PSD polling function. It is
 * 				registered as an internal callback of type #callAtReceiverDestroy.
 * \param[in]	hGeneric - handle of the Receiver involved in the seek operation
 * \param[in]	param - unused, ignored
 * \param[in]	param_len - unused, ignored
 * \param[in]	context - unused, ignored
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_PSDChangeStopDataServPeriodicFunc(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
	ETAL_HANDLE hReceiver;

	if (!ETAL_handleIsReceiver(hGeneric))
	{
		ASSERT_ON_DEBUGGING(0);
	}
	else
	{
		hReceiver = hGeneric;
		/*
		 * if we were invoked after the etaltml_stop_radiotext
		 * the ETAL_PSDChangePeriodicFunc was already de-registered
		 * so an error would not indicate abnormal situation
		 */
		(LINT_IGNORE_RET) ETAL_intCbDeregisterPeriodic(&ETAL_PSDChangeDataServPeriodicFunc, hReceiver);
	}
	
	return;
}

/***************************
 *
 * ETAL_cmdStartPSDDataServ_HDRADIO
 *
 **************************/
/*!
 * \brief		Starts the background periodic PSD polling
 * \details		This function registers the function that periodically
 * 				checks the PSD for changes (#ETAL_PSDChangeDataServPeriodicFunc) and
 * 				the function that de-registers it at Receiver destruction
 * 				(#ETAL_PSDChangeStopDataServPeriodicFunc).
 * \param[in]	hReceiver - handle of the HD Radio Receiver on which to read the PSD
 * \return		ETAL_RET_SUCCESS
 * \return		the error returned by #ETAL_intCbRegisterPeriodic or #ETAL_intCbRegister
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_cmdStartPSDDataServ_HDRADIO(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;

	ret = ETAL_intCbRegisterPeriodic(&ETAL_PSDChangeDataServPeriodicFunc, hReceiver, ETAL_HDRADIOPSD_INTCB_DELAY);
	if (ret == ETAL_RET_SUCCESS)
	{
		ret = ETAL_intCbRegister(callAtReceiverDestroy, &ETAL_PSDChangeStopDataServPeriodicFunc, hReceiver, ETAL_INTCB_CONTEXT_UNUSED);
	}
	return ret;
}

/***************************
 *
 * ETAL_cmdStopPSDDataServ_HDRADIO
 *
 **************************/
/*!
 * \brief		Stops all the internal callback functions used to poll PSD changes
 * \param[in]	hReceiver - handle of the HD Radio Receiver used to read the PSD
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_ERROR
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_cmdStopPSDDataServ_HDRADIO(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret, ret2;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	ret = ETAL_intCbDeregisterPeriodic(&ETAL_PSDChangeDataServPeriodicFunc, hReceiver);
	ret2 = ETAL_intCbDeregister(callAtReceiverDestroy, &ETAL_PSDChangeStopDataServPeriodicFunc, hReceiver);
	if ((ret != ETAL_RET_SUCCESS) || (ret2 != ETAL_RET_SUCCESS))
	{
		retval = ETAL_RET_ERROR;
	}
	return retval;
}

/***************************
 *
 * ETAL_sendTuneEvent_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends an #ETAL_INFO_TUNE event to the API user
 * \param[in]	hReceiver - handle of the Receiver involved in the tune operation
 * \param[in]	freq - the frequency to which the Receiver is tuned, in Hz
 * \param[in]	sync_mask - the sync status. For HD Radio the sync status is a
 * 				            combination of the values:
 * 				            - #ETAL_TUNESTATUS_SYNCMASK_FOUND
 * 				            - #ETAL_TUNESTATUS_SYNCMASK_HD_SYNC
 * 				            - #ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC
 *
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_sendTuneEvent_HDRADIO(ETAL_HANDLE hReceiver, tU32 freq, tU32 sync_mask)
{
	EtalTuneStatus tune_status;
	tU8 vl_acq_status; // current HD tune acquisition status
	tS8 vl_curr_prog;	// current HD program
	tU32 vl_avail_prog_num;	// total number of HD program 

	// retrieve current program information				
	ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, &vl_acq_status, &vl_curr_prog, &vl_avail_prog_num, NULL);

	(void)OSAL_pvMemorySet((tVoid *)&tune_status, 0x00, sizeof(tune_status));
	tune_status.m_receiverHandle = hReceiver;
	tune_status.m_stopFrequency = freq;
	tune_status.m_sync = sync_mask;
	tune_status.m_serviceId = vl_curr_prog;
    tune_status.m_muteStatus = ETAL_INVALID_MUTE_STATUS;

	/* Internal notification */
    if(tune_status.m_stopFrequency != 0)
    {
    	ETAL_sendInternalNotificationForTune_HDRADIO(hReceiver, freq, sync_mask);
    	ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_TUNE, (tVoid *)&tune_status, sizeof(tune_status));

        if (true == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress))
        {
            ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalTuneRequestInProgress, cmdActionStop);
        }
    }
}

/***************************
 *
 * ETAL_sendInternalNotificationForTune_HDRADIO
 *
 **************************/
/*!
 * \brief       Sends an internal notification to ETAL subscriber when a status
 *              after HD tune request is available.
 * \param[in]   hReceiver - handle of the Receiver involved in the tune operation
 * \param[in]   freq - the frequency to which the Receiver is tuned, in Hz
 * \param[in]   sync_mask - the sync status. This is a combination of the values:
 *                          - #ETAL_TUNESTATUS_SYNCMASK_xxxxxxx
 *
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_sendInternalNotificationForTune_HDRADIO(ETAL_HANDLE hReceiver, tU32 freq, tU32 sync_mask)
{
    EtalTuneStatus tune_status;

    ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "HD Tune internal notification on frequency %d. Sync status %d", freq, sync_mask);

    (void)OSAL_pvMemorySet((tVoid *)&tune_status, 0x00, sizeof(tune_status));
    tune_status.m_receiverHandle = hReceiver;
    tune_status.m_stopFrequency = freq;
    tune_status.m_sync = sync_mask;
    tune_status.m_serviceId = ETAL_INVALID_PROG;
    tune_status.m_muteStatus = ETAL_INVALID_MUTE_STATUS;
    ETAL_intCbScheduleCallbacks(hReceiver, callAtHDTuneFrequency, (tVoid *)&tune_status, sizeof(tune_status));
}

/***************************
 *
 * ETAL_tuneFSM_HDRADIO
 *
 **************************/
/*!
 * \brief		Manages the HD Radio tune command asynchronously
 * \details		The function sends an HD tune command and then waits #ETAL_HD_DELAY_BEFORE_HD_SYNC
 * 				before checking the digital acquisition status for HD flag.
 * 				If the *waitAudio* parameter is FALSE, the function stops here, otherwise
 * 				it waits another #ETAL_HD_DELAY_BEFORE_AUDIO_SYNC before starting to poll
 * 				the digital acquisition status for AUDIO flag and keeps polling it every
 * 				#ETAL_HD_AUDIO_SYNC_POLL_TIME until either AUDIO is acquired or #ETAL_HD_TUNE_TIMEOUT
 * 				from the start of the tune command have passed.
 *
 * 				The function also sends an #ETAL_INFO_TUNE event when/if the HD sync is found,
 * 				and another one when the HD audio is acquired (only if *waitAudio* is TRUE).
 *
 * 				This function takes the #ETAL_TuneSem_HDRADIO which is used only in this function and
 * 				in the #etal_destroy_receiver to avoid the receiver being deleted while the function
 * 				is in progress; the caller must take also the receiver lock because the function must
 * 				change the receiver state (see calls to ETAL_receiverXXX functions).
 * \param[in]	hReceiver - handle of the HD Radio receiver to tune
 * \param[in]	freq - the frequency to tune to, in Hz
 * \param[in]	actionType - defines the operational mode of the function, see #etalTuneFSMHDActionTy
 * \param[in]	waitAudio - if TRUE the function waits for HD audio sync before completion;
 * 				            if FALSE the function completes after HD signal is acquired (or timeout)
 * \param[in]	index - identifies the user of the function and is used to access the correct set of
 * 				        state variables. Each concurrent user must have a unique index assigned to
 * 				        avoid corruption of the state variables.
 * \remark		*index* is not currently used and should be set to 0
 * \return		ETAL_RET_SUCCESS - tune complete, HD signal acquired (for *waitAudio*==FALSE) or 
 * 				                   digital audio acquired (for *waitAudio*==TRUE), or stop command complete.
 * 				                   The FSM can be disabled
 * \return		ETAL_RET_IN_PROGRESS - only for #cmdTuneImmediateResponse, the FSM is still
 * 				                       operational and should be invoked again
 * \return		ETAL_RET_NO_DATA - no HD signal present on the selected frequency or
 * 				                   digital audio not acquired after a timeout.
 * 				                   The FSM can be disabled
 * \return		ETAL_RET_ERROR - communication error with the device
 * 				                 the FSM can be disabled
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_tuneFSM_HDRADIO(ETAL_HANDLE hReceiver, tU32 freq, etalTuneFSMHDActionTy actionType, tBool waitAudio, etalTuneFSMIndexTy index)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	OSAL_tMSecond *start_time;
	OSAL_tMSecond *hd_start_time;
	EtalPollSyncStateTy *state;
	etalToHDRADIODigiAcqStatusTy tDigiAcqStatus;
#if defined (CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) && !defined(CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA)
	etalToHDRADIOAlignStatusTy vl_AudioAlignmentStatus;
#endif
#ifdef	HD_MONITORING
	tU8 *pl_DigiAcqStatusMonitored;
#endif
	tyHDRADIOInstanceID instanceId;
	EtalTuneFSMContextTy *context;
	tS8 vl_dBGain = (tS8)0;
	tSInt vl_res;
	tBool configVpaMode;

	// FSM is being used.
	// we need to lock the FSM for the receiver
	//
	
	if ((ETAL_receiverGetHdInstance(hReceiver, &instanceId) != OSAL_OK) ||
		(index >= ETAL_HDRADIO_TUNEFSM_MAX_USER))
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : invalid Instance, receiver = %d, instanceId = %d, index = %d\n", 
				hReceiver, instanceId,  index);

		// this may happen if the FSM is checked but a STOP is pending
		// receiver may be out of order
		
		//			ASSERT_ON_DEBUGGING(0);

		// just return an error which will remove the receiver from the list of fsm on-going...
		
		ret = ETAL_RET_ERROR;
		goto exit;
	}

	// EPR Changes : 
	// we should keep only one context for any HD FSM/action
	// else we come to unsync in some situation :
	// example 
	//	=> HD activity on going on 'ETAL_HDRADIO_TUNEFSM_INTERNAL_USER' to get HD status after seek
	// 	=> new Tune request seek request for HD on 'ETAL_HDRADIO_TUNEFSM_API_USER'
	// the FSM is handling the activity on HD instance, we should not have risk of // activity on same HD instance.
	//
	// for now use only the ETAL_HDRADIO_TUNEFSM_API_USER FSM = 0
	//

#ifdef	HD_MONITORING
	// lock the FSM
	// lock is now corresponding to the function processing
	//
	
	if (ETAL_getTuneFSMLock_HDRADIO(instanceId) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND));
		ret = ETAL_RET_ERROR;
		goto exit;
	}
#endif

	context = &etalTuneContext[0]; // here was parameter index used
	state = &context->state[instanceId - 1];
	start_time = &context->startTime[instanceId - 1];
	hd_start_time = &context->HD_BlendStarTime[instanceId - 1];

#ifdef	HD_MONITORING
	pl_DigiAcqStatusMonitored = &context->digiAcqStatusMonitored[instanceId - 1];
#endif

	/*
	 * state PollEnd expects tDigiAcqStatus to be initialized, but depending on OS scheduling
	 * the while loop in PollAudioSyncWait may exit before initializing it
	 */
	(void)OSAL_pvMemorySet((tPVoid)&tDigiAcqStatus, 0x00, sizeof(etalToHDRADIODigiAcqStatusTy));

	if (actionType == tuneFSMHDRestart)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollInit forced");

		ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, (tU8)0x00, ETAL_INVALID_PROG, (tU8)0x00, (tU8) 0x00);

#ifndef HD_MONITORING
		if (*state != PollInit)
		{
			ETAL_releaseTuneFSMLock_HDRADIO(instanceId);
		}

#endif

		*state = PollInit;

#ifdef 	HD_MONITORING
		// release the lock before the return
		ETAL_releaseTuneFSMLock_HDRADIO(instanceId);
#endif 

		goto exit;
	}
#ifdef HD_MONITORING
	else if (actionType == tuneFSMHDNewService)
	{
		// only update the current acquisition status to nothing
		// so that it force to resend one later
		//
		*pl_DigiAcqStatusMonitored = (tU8)HD_ACQUIRED;
		
		// release the lock before the return
		ETAL_releaseTuneFSMLock_HDRADIO(instanceId);
		
		ret = ETAL_RET_SUCCESS;
		goto exit;
	}
#endif 
	else if (actionType == tuneFSMHDNormalResponse)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : cmdHDDTuneNormalResponse => PollInit forced");
		*state = PollInit;
	}
	else
	{
		/* Nothing to do */
	}

	ret = ETAL_RET_IN_PROGRESS;
	do
	{
		if (freq == ETAL_INVALID_FREQUENCY)
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : ETAL_INVALID_FREQUENCY");
			*state = PollError;
		}
		switch (*state)
		{
			case PollInit:
#ifndef HD_MONITORING
				//lock for the FSM full execution time
				if (ETAL_getTuneFSMLock_HDRADIO(instanceId) != OSAL_OK)
				{
					ASSERT_ON_DEBUGGING(0);
					ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND));
					return ETAL_RET_ERROR;
				}
#endif
				// we are in init state, so that means at start of HD tune
				// notify an info that HD is no more sync, station found.
				ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_FOUND));

				// Set the station flag as new : this is needed to force the radiotext resend
				ETAL_forceResendRadioInfo();
				
				*start_time = OSAL_ClockGetElapsedTime();
				*hd_start_time = OSAL_ClockGetElapsedTime();
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollInit");
				ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, (tU8)0x00, ETAL_INVALID_PROG, (tU8)0x00, (tU8) 0x00);

				if (ETAL_cmdTuneSelect_HDRADIO(hReceiver, freq, MPS_AUDIO_HD_1) != OSAL_OK)
				{
					 ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : error in ETAL_cmdTuneSelect_HDRADIO");
					*state = PollError;
					break;
				}

				// set HD FM MRC mode if FM VPA mode is active
				if (ETAL_receiverGetConfigVPAMode(hReceiver, &configVpaMode) != ETAL_RET_SUCCESS)
				{
					 ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : error in ETAL_receiverGetConfigVPAMode");
					*state = PollError;
					break;
				}
				else if (ETAL_SetMRCCnfg_HDRADIO(hReceiver, freq, configVpaMode) != OSAL_OK)
				{
					 ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : error in ETAL_SetMRCCnfg_HDRADIO");
					*state = PollError;
					break;
				}
				else
				{
					/* Nothing to do */
				}
	
				if (actionType == tuneFSMHDNormalResponse)
				{
					(void)OSAL_s32ThreadWait(ETAL_HD_DELAY_BEFORE_HD_SYNC + HD_SYNCWAIT_CROSSING_DELAY); /* HD_SYNCWAIT_CROSSING_DELAY to ensure the 'if' in PollHDSyncWait evaluates to false */
				}
				*state = PollHDSyncWait;
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollHDSyncWait");
				/*
				 * fall through is required for cmdTuneNormalResponse
				 * otherwise the while() loop exits
				 */

			case PollHDSyncWait:
				if (OSAL_ClockGetElapsedTime() < *start_time + ETAL_HD_DELAY_BEFORE_HD_SYNC)
				{
					break;
				}
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollHDSyncWait request status");
				if (ETAL_cmdGetStatusDigital_HDRADIO(hReceiver, &tDigiAcqStatus) != OSAL_OK)
				{
					ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : error in ETAL_cmdGetStatusDigital_HDRADIO in PollHDSyncWait");
					/* cannot communicate with HD */
					*state = PollError;
					break;
				}
				else if (((tDigiAcqStatus.m_TuneOperationType == (tU8)ALL_TYPES_TUNED) ||
						(tDigiAcqStatus.m_TuneOperationType == (tU8)ONLY_DIGITAL_TUNED)) &&
						(tDigiAcqStatus.m_Band != (tU8)IDLE_MODE) &&
						((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)HD_ACQUIRED) == (tU8)HD_ACQUIRED))
				{
					/* we have HD signal */
					ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, tDigiAcqStatus.m_DigiAcquisitionStatus, (tS8)tDigiAcqStatus.m_CurrentProg, tDigiAcqStatus.m_AudioProgramsAvailable, (tU8) 0x00);
					ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_HD_SYNC | ETAL_TUNESTATUS_SYNCMASK_FOUND));
					if (waitAudio == TRUE)
					{
						*state = PollAudioSyncDelay;
					}
					else
					{
						*state = PollEnd;
						break;
					}
				}
				else
				{
					// the synchronisation failed : indicate it					
					ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, tDigiAcqStatus.m_DigiAcquisitionStatus, (tS8)tDigiAcqStatus.m_CurrentProg, tDigiAcqStatus.m_AudioProgramsAvailable, (tU8) 0x00);
					ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_SYNC_FAILURE| ETAL_TUNESTATUS_SYNCMASK_FOUND));
			
					ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollHDSyncWait no HD");
					/* no HD contents on this frequency */
					ret = ETAL_RET_NO_DATA;
#ifdef HD_MONITORING
					// keep monitoring in case this is due only to bad reception for sometime
					*state = PollMonitoring;
					ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollMonitoring");
					*pl_DigiAcqStatusMonitored = tDigiAcqStatus.m_DigiAcquisitionStatus;
#else
					*state = PollInit;
#endif
					break;
				}
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollAudioSyncDelay");
				/*
				 * fall through is required for cmdTuneNormalResponse
				 * otherwise the while() loop exits
				 */

			case PollAudioSyncDelay:
				if (actionType == tuneFSMHDNormalResponse)
				{
					(void)OSAL_s32ThreadWait(ETAL_HD_DELAY_BEFORE_AUDIO_SYNC);
				}
				else if (OSAL_ClockGetElapsedTime() < *start_time + ETAL_HD_DELAY_BEFORE_HD_SYNC + ETAL_HD_DELAY_BEFORE_AUDIO_SYNC)
				{
					// before breaking : we need to check the gain and send then to cmost
					// Send the information to CMOST if needed
					// the rationnal to send is : audio is available and polling periodicity
					// 
					// 
					if (OSAL_ClockGetElapsedTime() > *hd_start_time + ETAL_HD_BLEND_STATUS_CMOST_PERIOD)
					{
						if (ETAL_cmdGetStatusDigital_HDRADIO(hReceiver, &tDigiAcqStatus) != OSAL_OK)
						{
							ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : error in ETAL_cmdGetStatusDigital_HDRADIO in PollAudioSyncDelay");
							*state = PollError;
							break;
						}

						// sent the even/status info if either the SIS is acquired
						if (((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)SIS_ACQUIRED) == (tU8)SIS_ACQUIRED)
							& ((*pl_DigiAcqStatusMonitored & (tU8)SIS_ACQUIRED) != (tU8)SIS_ACQUIRED))
						{
							// sent an event for SIS acquired info
							// 
								
							/* we have HD signal but no more audio */
							ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_HD_SYNC | ETAL_TUNESTATUS_SYNCMASK_SIS_ACQUIRED | ETAL_TUNESTATUS_SYNCMASK_FOUND));
						}
						else
						{
							// for now nothing to send.
						}

#ifdef HD_MONITORING
						// keep monitoring in case this is due only to bad reception for sometime
						*pl_DigiAcqStatusMonitored = tDigiAcqStatus.m_DigiAcquisitionStatus;
#endif

#if (defined(CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) && !defined (CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA))

						// get the digital audio status

						if (OSAL_OK == ETAL_cmdGetAlignStatus_HDRADIO(hReceiver, &vl_AudioAlignmentStatus))
						{	
							// update alignment status
							ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, tDigiAcqStatus.m_DigiAcquisitionStatus, (tS8)tDigiAcqStatus.m_CurrentProg, tDigiAcqStatus.m_AudioProgramsAvailable, vl_AudioAlignmentStatus.m_AudioIndicator.value);
						}
						else
						{
							ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : ETAL_cmdGetAlignStatus_HDRADIO error");
						}												
#endif // CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA && !CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA

						if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)DIGITAL_AUDIO_ACQUIRED) == (tU8)DIGITAL_AUDIO_ACQUIRED)
						{
							//ii. Parameter 1: T_AD_gain_step = 500 msec (fractionnal value will be 0x16f)
							//iii.	Parameter 2: T_DA_gain_step = 100 msec (fractionnal value will be 0x72f)
							//iv. Parameter 3: AV-gain =	0 dB  (fractionnal value will be 0x7fffff)
							//v.	Parameter 4: DV-gain = -6 dB is default (value read from DCOP status and transformed in dB)
							//vi. Parameter 5: Cd/No (value read from DCOP status)
							// 
							// DV-gain : The gain needs to be updated : 
							// from the table values 
							//0x00 to 0x07 = positive gain value 
							//  0x00 = 0, 1 = 1 ... 7 = 7....
							// = -8, 7 = -1, others = unvalid
							// 0x18 to 0x1F =) negative values
							// 0x18 = -8, 0x19 = -7.... 0x1F = -1
							// other not valid
							// ie 0x08 to 0x17 reserved
							// 0x20 to 7F = reserved
							// 0x80 = invalid
							//

							// 

							if (tDigiAcqStatus.m_TXDigitalAudioGain <= (tU8)0x07)
							{
								vl_dBGain = (tS8)tDigiAcqStatus.m_TXDigitalAudioGain;
							}
							else if ((tDigiAcqStatus.m_TXDigitalAudioGain >= (tU8)0x18) && (tDigiAcqStatus.m_TXDigitalAudioGain <= (tU8)0x1F))
							{
								vl_dBGain = (tS8)tDigiAcqStatus.m_TXDigitalAudioGain - (tS8)32;
							}
							else
							{
								// invalid values
								ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "Invalid gain value: %d", tDigiAcqStatus.m_TXDigitalAudioGain);
	
							}

							(void)ETAL_CmdTunerConfHDBlend_CMOST(hReceiver, ETAL_HD_T_AD_GAIN_STEP_MS, ETAL_HD_T_DA_GAIN_STEP_MS, (tS8)ETAL_HD_AV_GAIN_DB, vl_dBGain, tDigiAcqStatus.m_CdNo);
							*hd_start_time = OSAL_ClockGetElapsedTime();
														
						}

					}
					break;
				}
				else
				{
					/* Nothing to do */
				}
				*state = PollAudioSyncWait;
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollAudioSyncWait");
				/*
				 * fall through is required for cmdTuneNormalResponse
				 * otherwise the while() loop exits
				 */

			case PollAudioSyncWait:
				do {
					// get the status so that it is update to date even if timeout
					// since it used after
					//
					// define a local variable to identify which tune timeout applies
					// 
					OSAL_tMSecond vl_tuneTimeout;
					
#if !defined(CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) || defined(CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA)
					vl_tuneTimeout = ETAL_HD_TUNE_TIMEOUT_AAA_DISABLED;
#else
					EtalBcastStandard vl_standard_info;

					// get the standard information 			
					vl_standard_info = ETAL_receiverGetStandard(hReceiver);
					
					if(vl_standard_info == ETAL_BCAST_STD_HD_AM)
					{
						// we are in AM
						if (TRUE == v_alignmentConfiguration.m_enableAutoAlignmentForAM)
						{
							vl_tuneTimeout = ETAL_HD_TUNE_TIMEOUT_AAA_ENABLED;
						}
						else
						{
							vl_tuneTimeout = ETAL_HD_TUNE_TIMEOUT_AAA_DISABLED;
						}
					}
					else if (vl_standard_info == ETAL_BCAST_STD_HD_FM)
					{
						if (TRUE == v_alignmentConfiguration.m_enableAutoAlignmentForFM)
						{
							vl_tuneTimeout = ETAL_HD_TUNE_TIMEOUT_AAA_ENABLED;
						}
						else
						{
							vl_tuneTimeout = ETAL_HD_TUNE_TIMEOUT_AAA_DISABLED;
						}
					}	
					else
					{
						// should not happen : 
						// 
						vl_tuneTimeout = ETAL_HD_TUNE_TIMEOUT_AAA_DISABLED;
					}
#endif


	
					if (ETAL_cmdGetStatusDigital_HDRADIO(hReceiver, &tDigiAcqStatus) != OSAL_OK)
					{
						ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : error in ETAL_cmdGetStatusDigital_HDRADIO in PollAudioSyncWait");
						*state = PollError;
						break;
					}
					else if (((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)DIGITAL_AUDIO_ACQUIRED) == (tU8)DIGITAL_AUDIO_ACQUIRED))
					{
						// it may happen sometime that the SIS is not acquired !
						if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)SIS_ACQUIRED) == (tU8)SIS_ACQUIRED)
						{
							// set the right status
							ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC | ETAL_TUNESTATUS_SYNCMASK_HD_SYNC | ETAL_TUNESTATUS_SYNCMASK_FOUND | ETAL_TUNESTATUS_SYNCMASK_SIS_ACQUIRED));
						}
						else
						{
							// set the right status
							ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC | ETAL_TUNESTATUS_SYNCMASK_HD_SYNC | ETAL_TUNESTATUS_SYNCMASK_FOUND ));
					
						}
						
						*state = PollEnd;
						break;
					}
					else if (OSAL_ClockGetElapsedTime() > (*start_time + vl_tuneTimeout))
					{
						ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollAudioSyncWait TIMEOUT");
						*state = PollEnd;

						// send the right status
						//
						if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)SIS_ACQUIRED) == (tU8)SIS_ACQUIRED)
						{
							// so keep monitoring, no event in that case
							// 
								
							/* we have HD signal but no more audio */
							ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED | ETAL_TUNESTATUS_SYNCMASK_HD_SYNC | ETAL_TUNESTATUS_SYNCMASK_SIS_ACQUIRED | ETAL_TUNESTATUS_SYNCMASK_FOUND));

						}
						else if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)HD_ACQUIRED) == (tU8)HD_ACQUIRED)
						{				
							// the audio acquisition failed but we are still sync : indicate it
							ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED| ETAL_TUNESTATUS_SYNCMASK_HD_SYNC| ETAL_TUNESTATUS_SYNCMASK_FOUND));
						}
						else
						{
							// no HD anymore
							ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED| ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND| ETAL_TUNESTATUS_SYNCMASK_FOUND));			
						}
						
						ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollAudioSyncWait returned %d", tDigiAcqStatus.m_DigiAcquisitionStatus);
						break;
					}
					else if (actionType == tuneFSMHDNormalResponse)
					{
						(void)OSAL_s32ThreadWait(ETAL_HD_AUDIO_SYNC_POLL_TIME);
					}
					else
					{
						/* Nothing to do */
					}
				} while (actionType == tuneFSMHDNormalResponse);
				break;

			case PollEnd:
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollEnd");
				ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, tDigiAcqStatus.m_DigiAcquisitionStatus, (tS8)tDigiAcqStatus.m_CurrentProg, tDigiAcqStatus.m_AudioProgramsAvailable, (tU8) 0x00);
				if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)DIGITAL_AUDIO_ACQUIRED) != (tU8)0)
				{
					ret = ETAL_RET_SUCCESS;
				}
				else 
				{
					if (waitAudio == FALSE)
					{
						ret = ETAL_RET_SUCCESS;
					}
					else
					{
						ret = ETAL_RET_NO_DATA;
					}
				}

#ifdef	HD_MONITORING
				*pl_DigiAcqStatusMonitored = tDigiAcqStatus.m_DigiAcquisitionStatus;

				// keep in monitonting state now
				// *state = PollInit;
				*state = PollMonitoring;
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollMonitoring");
				// change the start time to start the 'monitoring time'
				*start_time = OSAL_ClockGetElapsedTime();

#else
				*state = PollInit;
#endif
				break;

			case PollError:
				ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : PollError");
				ret = ETAL_RET_ERROR;
				ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND));
				*state = PollInit;
				break;

#ifdef	HD_MONITORING
			case PollMonitoring:								
				// time to monitor ? 
				if (OSAL_ClockGetElapsedTime() > *start_time + ETAL_HD_TUNE_STATUS_MONITORING)
				{
					// Time to get the status
					// 	
					vl_res = ETAL_cmdGetStatusDigital_HDRADIO(hReceiver, &tDigiAcqStatus);
					if (vl_res == OSAL_OK)
					{								
						if (tDigiAcqStatus.m_DigiAcquisitionStatus != *pl_DigiAcqStatusMonitored)
						{
							// there is a change in audio acquisition status : 
							// send the tune event info, 
							// and move to appropriate state

#if (defined(CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) && !defined (CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA))
							// get the align status information
							if (OSAL_OK == ETAL_cmdGetAlignStatus_HDRADIO(hReceiver, &vl_AudioAlignmentStatus))
							{	
								// update alignment status
								ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, tDigiAcqStatus.m_DigiAcquisitionStatus, (tS8)tDigiAcqStatus.m_CurrentProg, tDigiAcqStatus.m_AudioProgramsAvailable, vl_AudioAlignmentStatus.m_AudioIndicator.value);
							}
							else
							{
								ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : ETAL_cmdGetAlignStatus_HDRADIO error");
								ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, tDigiAcqStatus.m_DigiAcquisitionStatus, (tS8)tDigiAcqStatus.m_CurrentProg, tDigiAcqStatus.m_AudioProgramsAvailable, (tU8) 0x00);
							}
#else 
							ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, tDigiAcqStatus.m_DigiAcquisitionStatus, (tS8)tDigiAcqStatus.m_CurrentProg, tDigiAcqStatus.m_AudioProgramsAvailable, (tU8) 0x00);
#endif // CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA && !CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA					

							// HD AUDIO is acquired now : send info and all is fine ;)
							if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)DIGITAL_AUDIO_ACQUIRED) != (tU8)0)
							{
								// ret status keep to be 'in progress' so that semaphore is kept for the state.
								ret = ETAL_RET_SUCCESS;
								// it may happen sometime that the SIS is not acquired ! despite audio is acquired
								if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)SIS_ACQUIRED) == (tU8)SIS_ACQUIRED)
								{
									// set the right status
									ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC | ETAL_TUNESTATUS_SYNCMASK_HD_SYNC | ETAL_TUNESTATUS_SYNCMASK_FOUND | ETAL_TUNESTATUS_SYNCMASK_SIS_ACQUIRED));
								}
								else
								{
									// set the right status
									ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC | ETAL_TUNESTATUS_SYNCMASK_HD_SYNC | ETAL_TUNESTATUS_SYNCMASK_FOUND ));
							
								}
								// keep same state 
								*state = PollMonitoring;
							}
							else if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)SIS_ACQUIRED) == (tU8)SIS_ACQUIRED)
							{
								// so keep monitoring, no event in that case
								// 
									
								/* we have HD signal but no more audio */
								ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_HD_SYNC | ETAL_TUNESTATUS_SYNCMASK_SIS_ACQUIRED | ETAL_TUNESTATUS_SYNCMASK_FOUND));

								// keep same state
								*state = PollMonitoring;
							}
							else if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)HD_ACQUIRED) == (tU8)HD_ACQUIRED)
							{
		
								// for now we do not report it...
								// 								
								/* we have HD signal but no more audio */
								ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_HD_SYNC | ETAL_TUNESTATUS_SYNCMASK_FOUND));
				
								// nothing change : we state this is ok
								// ret status keep to be 'in progress' so that semaphore is kept for the state.
									
								*state = PollAudioSyncWait;
			
							}
							else
							{
								// we lost the sygnal hd : back to audio sync
								/* we have HD signal but no more audio */
								ETAL_sendTuneEvent_HDRADIO(hReceiver, freq, (tU32)(ETAL_TUNESTATUS_SYNCMASK_FOUND));

								// nothing change : we state this is ok
								// ret status keep to be 'in progress' so that semaphore is kept for the state.

								*state = PollHDSyncWait;
									
							}						
						}
						else
						{
							// nothing change : we state this is ok
						ret = ETAL_RET_SUCCESS;
						}

						// update last monitored status
						*pl_DigiAcqStatusMonitored = tDigiAcqStatus.m_DigiAcquisitionStatus ;

						// update the startime which is now the time since last monitoring status
						context->startTime[instanceId - 1] = OSAL_ClockGetElapsedTime();


						// Send the information to CMOST if needed
						// the rationnal to send is : audio is available and polling periodicity
						// 
						// 
						if (OSAL_ClockGetElapsedTime() > *hd_start_time + ETAL_HD_BLEND_STATUS_CMOST_PERIOD)
						{
#if (defined(CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA) && !defined (CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA))

							// get the digital audio status
							if (OSAL_OK == ETAL_cmdGetAlignStatus_HDRADIO(hReceiver, &vl_AudioAlignmentStatus))
							{	
								// update alignment status
								ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, tDigiAcqStatus.m_DigiAcquisitionStatus, (tS8)tDigiAcqStatus.m_CurrentProg, tDigiAcqStatus.m_AudioProgramsAvailable, vl_AudioAlignmentStatus.m_AudioIndicator.value);
							}
							else
							{
								ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "ETAL_tuneFSM_HDRADIO : ETAL_cmdGetAlignStatus_HDRADIO error");
							}	
#endif // CONFIG_MODULE_DCOP_HDRADIO_SUPPORT_AAA && !CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA
							if ((tDigiAcqStatus.m_DigiAcquisitionStatus & (tU8)DIGITAL_AUDIO_ACQUIRED) == (tU8)DIGITAL_AUDIO_ACQUIRED)
							{

								//ii. Parameter 1: T_AD_gain_step = 500 msec (fractionnal value will be 0x16f)
								//iii.	Parameter 2: T_DA_gain_step = 100 msec (fractionnal value will be 0x72f)
								//iv. Parameter 3: AV-gain =	0 dB  (fractionnal value will be 0x7fffff)
								//v.	Parameter 4: DV-gain = -6 dB is default (value read from DCOP status and transformed in dB)
								//vi. Parameter 5: Cd/No (value read from DCOP status)
								// 
								// DV-gain : The gain needs to be updated : 
								// from the table values 
								//0x00 to 0x07 = positive gain value 
								//  0x00 = 0, 1 = 1 ... 7 = 7....
								// = -8, 7 = -1, others = unvalid
								// 0x18 to 0x1F =) negative values
								// 0x18 = -8, 0x19 = -7.... 0x1F = -1
								// other not valid
								// ie 0x08 to 0x17 reserved
								// 0x20 to 7F = reserved
								// 0x80 = invalid
								//
								

								if (tDigiAcqStatus.m_TXDigitalAudioGain <= (tU8)0x07)
								{
									vl_dBGain = (tS8)tDigiAcqStatus.m_TXDigitalAudioGain;
								}
								else if ((tDigiAcqStatus.m_TXDigitalAudioGain >= (tU8)0x18) && (tDigiAcqStatus.m_TXDigitalAudioGain <= (tU8)0x1F))
								{
									vl_dBGain = (tS8)tDigiAcqStatus.m_TXDigitalAudioGain - (tS8)32;
								}
								else
								{
									// invalid values
									ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "Invalid gain value: %d", tDigiAcqStatus.m_TXDigitalAudioGain);
		
								}
		
								(void)ETAL_CmdTunerConfHDBlend_CMOST(hReceiver, ETAL_HD_T_AD_GAIN_STEP_MS, ETAL_HD_T_DA_GAIN_STEP_MS, (tS8)ETAL_HD_AV_GAIN_DB, vl_dBGain, tDigiAcqStatus.m_CdNo);
								*hd_start_time = OSAL_ClockGetElapsedTime();
										

							}
						}
					}
					else
					{
						// it means the status acquisition command to DCOP was not good.
						// nothing to do
						// 
						// simulate all is ok we will retry next time
						ret = ETAL_RET_ERROR;
					}
				}
				else
				{
					// nothing change : we state this is ok
					// this will release the semaphore.
					ret = ETAL_RET_SUCCESS;
				}
				
				break;
#endif
		}
	/*
	 * exit for PollInit, PollHDSyncWait and PollAudioSyncDelay, PollAudioSyncWait
	 */
#ifdef	HD_MONITORING
	/*
	* exit for PollInit, PollHDSyncWait and PollAudioSyncDelay, PollAudioSyncWait, PollMonitoring
	 */

	} while ((*state != PollInit) && (*state != PollHDSyncWait) && (*state != PollAudioSyncDelay) && (*state != PollAudioSyncWait) && (*state != PollMonitoring));
#else
	/*
	* exit for PollInit, PollHDSyncWait and PollAudioSyncDelay, PollAudioSyncWait
	 */

	} while ((*state != PollInit) && (*state != PollHDSyncWait) && (*state != PollAudioSyncDelay) && (*state != PollAudioSyncWait));
#endif

	if ((ret == ETAL_RET_NO_DATA) || (ret == ETAL_RET_ERROR))
	{
		ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, (tU8)0x00, ETAL_INVALID_PROG, (tU8)0x00, (tU8)0x00);
	}

#ifndef HD_MONITORING
	if  ((ret == ETAL_RET_NO_DATA) || (ret == ETAL_RET_ERROR) || (ret == ETAL_RET_SUCCESS))
	{
		ETAL_releaseTuneFSMLock_HDRADIO(instanceId);
	}
#else
	//procedure is ended : release the FSM lock
	ETAL_releaseTuneFSMLock_HDRADIO(instanceId);
#endif

exit:
	return ret;
}

/***************************
 *
 * ETAL_cmdSysVersion_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends the Sys_Version (0x80) command, Get_SW_Version (0x01) function and parses the response
 * \details		This function read the software version string from the HD Radio device
 * 				and returns it to the caller as null-terminated string.
 * 				The function locks the HD Radio device for the duration of the operation.
 * \param[in]	str - pointer to a buffer where the function writes the software version.
 * 				      The buffer must be at least #ETAL_HDRADIO_SWVER_MAX_STRING_LEN bytes long.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or unrecognized response.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSysVersion_HDRADIO(tChar *str)
{
	tSInt retval;
	tU8 *resp = CP_buf; /* The buffer is statically allocated being defined as global. Although paying in ".bss" size, this way saves function's stack */
	tU8 *cmd = CP_buf;
	tSInt rlen;
	tSInt clen;

	etalToHDRADIOCpTy cmdGetSwVersion;
	(void)OSAL_pvMemorySet((tVoid *)&cmdGetSwVersion, 0, sizeof(etalToHDRADIOCpTy));

	cmdGetSwVersion.m_Opcode = (tU8)HDRADIO_SYS_VERSION_CMD;
	cmdGetSwVersion.m_FunctionCode = (tU8)GET_SW_VERSION;
	cmdGetSwVersion.m_CmdType = (tU8)WRITE_TYPE;

	ETAL_getHDRADIODevLock();

	clen = ETAL_initCPbuffer_HDRADIO(cmd, &cmdGetSwVersion, \
				END_ARG_MARKER);
	if (ETAL_checkParameterError_HDRADIO(clen) == OSAL_ERROR)
	{
		ETAL_releaseHDRADIODevLock();
		retval = OSAL_ERROR;
	}
	else
	{
		retval = ETAL_sendCommandTo_HDRADIO((ETAL_HANDLE)HDRADIO_FAKE_RECEIVER, cmd, (tU32)clen, &resp, &rlen);
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent_HDRADIO(ETAL_INVALID_HANDLE, retval, resp, rlen);
		}
		else
		{
			if (rlen < ETAL_HDRADIO_SWVER_MIN_FIELD_LEN)
			{
				retval = OSAL_ERROR;
			}
			else
			{
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Complete HDRADIO version: %s", &resp[ETAL_HDRADIO_SWVER_STRING_OFFSET]);
				resp[ETAL_HDRADIO_SWVER_STRING_OFFSET + ETAL_HDRADIO_SWVER_MAX_STRING_LEN] = (tU8)'\0';
				if (resp[ETAL_HDRADIO_CMD_FUNCTION_OFFSET] != (tU8)GET_SW_VERSION) 
				{
					retval = OSAL_ERROR;
				}
				else
				{
					OSAL_szStringCopy(str, (const tChar *)&resp[ETAL_HDRADIO_SWVER_STRING_OFFSET]);
				}
			}
		}
	
		ETAL_releaseHDRADIODevLock();
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdPing_HDRADIO
 *
 **************************/
/*!
 * \brief		Checks if the HD Radio device is alive
 * \details		The function sends a command to the HD Radio device and reads the answer
 * 				to verify if the device is alive. The command sent is the Sys_Version
 * 				(see #ETAL_cmdSysVersion_HDRADIO) which returns the software version
 * 				in the following format (in parentheses the number of characters of
 * 				each sub-string):
 * 				- HW Chip Name(8)-Hardware Configuration(8)-Software Configuration(8)-Release Type(1)Base Version Number(4).Build Number(3)
 *
 * 				The function checks if the response contains at least the
 * 				#ETAL_HDRADIO_STA680_SWVER_STRING sub-string.
 * \return		OSAL_OK - the HD Radio device is alive
 * \return		OSAL_ERROR - the HD Radio device does not respond or responds with unrecognized string
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdPing_HDRADIO(void)
{
	tSInt retval;
	tChar str[ETAL_HDRADIO_SWVER_MAX_STRING_LEN];

	retval = ETAL_cmdSysVersion_HDRADIO(str);
	if (retval == OSAL_OK)
	{
		/* The complete string is: */
		/* SW VER STRING: HW Chip Name(8)-Hardware Configuration(8)-Software Configuration(8)-Release Type(1)Base Version Number(4).Build Number(3) */
		/* We only consider a sub-string here */
		str[ETAL_HDRADIO_SWVER_STRING_LEN] = '\0';
		if (OSAL_s32StringCompare ((const tChar *)str, ETAL_HDRADIO_STA680_SWVER_STRING) != 0)
		{
			retval = OSAL_ERROR;
		}
	}

	return retval;
}

/***************************
 *
 * ETAL_pollSISChangeAsync_HDRADIO
 *
 **************************/
tSInt ETAL_pollSISChangeAsync_HDRADIO(ETAL_HANDLE hDatapath)
{
	tSInt vl_res = OSAL_OK;

	if (ETAL_cmdGetStatusDigitalAsync_HDRADIO(hDatapath) != OSAL_OK)
	{
		vl_res = OSAL_ERROR;
	}
	else if (ETAL_cmdGetSISAsync_HDRADIO(hDatapath) != OSAL_OK)
	{
		vl_res = OSAL_ERROR;
	}
	else
	{
		/* Nothing to do */
	}

	return vl_res;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
