//!
//!  \file 		etalcmd_cmost.c
//!  \brief 	<i><b> ETAL command protocol layer for CMOST devices </b></i>
//!  \details   The ETAL command protocol layer implements the command protocol
//!				specific to each device controlled by ETAL (Tuner, DCOP).
//! 
//!				This file implements the command protocol for the CMOST devices
//!				(STAR, DOT).
//! 
//!             None of the functions in this file directly locks the Tuner, rather
//!             they rely on the communication layer (etalcomm_cmost.c) to do it.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"

#ifdef CONFIG_ETAL_SUPPORT_CMOST

#include "etalinternal.h"
#include "tunerdriver_internal.h"
#include <math.h>

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

/*
 * Generic macros
 */

/*!
 * \def		ETAL_CMOST_HEADER_24BIT
 * 			CMOST commands are implemented as byte arrays. The command
 * 			header is 24 bits and each command parameter is also 24 bits.
 * 			This macro defines the byte offset from the beginning of the
 * 			array to access the first header byte.
 */
#define ETAL_CMOST_HEADER_24BIT   	0
/*!
 * \def		ETAL_CMOST_PARAM1_24BIT
 * 			Offset to access the first byte of the first parameter
 *  \see	ETAL_CMOST_HEADER_24BIT
 */
#define ETAL_CMOST_PARAM1_24BIT     3
/*!
 * \def		ETAL_CMOST_PARAM2_24BIT
 * 			Offset to access the first byte of the second parameter
 *  \see	ETAL_CMOST_HEADER_24BIT
 */
#define ETAL_CMOST_PARAM2_24BIT     6
/*!
 * \def		ETAL_CMOST_PARAM3_24BIT
 * 			Offset to access the first byte of the third parameter
 *  \see	ETAL_CMOST_HEADER_24BIT
 */
#define ETAL_CMOST_PARAM3_24BIT     9
/*!
 * \def		ETAL_CMOST_PARAM4_24BIT
 * 			Offset to access the first byte of the fourth parameter
 *  \see	ETAL_CMOST_HEADER_24BIT
 */
#define ETAL_CMOST_PARAM4_24BIT    12
/*!
 * \def		ETAL_CMOST_PARAM5_24BIT
 * 			Offset to access the first byte of the fifth parameter
 *  \see	ETAL_CMOST_HEADER_24BIT
 */
#define ETAL_CMOST_PARAM5_24BIT    15
/*!
 * \def		ETAL_CMOST_PARAM6_24BIT
 * 			Offset to access the first byte of the sixth parameter
 *  \see	ETAL_CMOST_HEADER_24BIT
 */
#define ETAL_CMOST_PARAM6_24BIT    18

/*
 * TUNER_Change_Band (0x0A) macros
 */

/*!
 * \def		ETAL_CMOST_CHANGEBAND_HDFILTER_ENABLE
 * 			Parameter 3 in the TUNER_Change_Band (0x0A) command is optional
 * 			and contains the 'Processing features'.
 * 			This macro accesses the bit that controls the HD filter.
 */
#define ETAL_CMOST_CHANGEBAND_HDFILTER_ENABLE   0x000010
/*!
 * \def		ETAL_CMOST_CHANGEBAND_HDBLENDING_ENABLE
 * 			This macro access the bit that controls the HD blending feature
 * 			that is the ability of the CMOST to switch from internal FM
 * 			signal to external HD signal based on a GPIO level.
 *  \see	ETAL_CMOST_CHANGEBAND_HDFILTER_ENABLE
 */
#define ETAL_CMOST_CHANGEBAND_HDBLENDING_ENABLE 0x000020

/*
 * Control Status Register (CSR) macros
 */

/*!
 * \def		ETAL_CMOST_CSR_ENA
 * 			The Control Status Register (CSR) is the CMOST register
 * 			that controls the CMOST RDS buffer.
 * 			This macro accesses the bit to enable RDS buffer
 */
#define ETAL_CMOST_CSR_ENA                    ((tU8)0x01)
/*!
 * \def		ETAL_CMOST_CSR_RBDS
 *			Accesses the bit that configures the CMOST in RBDS mode to decode E blocks.
 */
#define ETAL_CMOST_CSR_RBDS                   ((tU8)0x02)
/*!
 * \def		ETAL_CMOST_CSR_RDS_IREN
 * 			The Control Status Register (CSR) is the CMOST register
 * 			that controls the CMOST RDS buffer.
 * 			This macro accesses the bit to enable the use of the hardware Interrupt
 * 			line to signal the RDS data is ready.
 */
#define ETAL_CMOST_CSR_RDS_IREN               ((tU8)0x04)
/*!
 * \def		ETAL_CMOST_CSR_ERRTHRESH
 * 			The Control Status Register (CSR) is the CMOST register
 * 			that controls the CMOST RDS buffer.
 * 			This macro accesses the bit to enable Error Threshold above which
 * 			a block is considered uncorrectable and not saved to the RDS buffer.
 * 			0: block with 0 error are stored in the RDS buffer
 * 			1: block with 0 or 1 error corrected are stored in the RDS buffer
 * 			2: block with 0 or 1 or 2 errors corrected are stored in the RDS buffer
 * 			3: block with 0 or 1 or 2 or 3 errors corrected are stored in the RDS buffer
 * 			6: block are stored in the RDS buffer regardless of number of errors
 */
#define ETAL_CMOST_CSR_ERRTHRESH_0            ((tU8)0x00)
#define ETAL_CMOST_CSR_ERRTHRESH_1            ((tU8)0x10)
#define ETAL_CMOST_CSR_ERRTHRESH_2            ((tU8)0x20)
#define ETAL_CMOST_CSR_ERRTHRESH_3            ((tU8)0x30)
#define ETAL_CMOST_CSR_ERRTHRESH_6            ((tU8)0x60)
#define ETAL_CMOST_CSR_ERRTHRESH_MASK         ((tU8)0x70)

/*!
 * \def		ETAL_CMOST_CSR_FORCEFASTPI
 *			Accesses the bit that configures the CMOST to stay in startup fast PI
 *			acquisition mode. 
 *  \see	ETAL_CMOST_CSR_RDS_IREN
 */
#define ETAL_CMOST_CSR_FORCEFASTPI        ((tU8)0x01)
/*!
 * \def		ETAL_CMOST_CSR_RESET
 * 			The Control Status Register (CSR) is the CMOST register
 * 			that do a manual reset the first time  the CMOST RDS buffer is used.
 */
#define ETAL_CMOST_CSR_RESET                  ((tU8)0x02)
/*!
 * \def		ETAL_CMOST_CSR_GROUPOUTEN
 *			Accesses the bit that configures the CMOST in group output mode.
 */
#define ETAL_CMOST_CSR_GROUPOUTEN         ((tU8)0x04)

/*
 * TUNER_Read (0x1E) macros
 */

/*!
 * \def		MAX_NB_OF_WORD_TO_BE_READ
 * 			The TUNER_Read (0x1E) command is used to read CMOST memory locations.
 * 			At most this number of 24-bits locations can be read in one shot.
 *  \remark	This limit comes from the CMOST Firmware implementation thus it must not
 *  		be changed in ETAL.
 */
#define MAX_NB_OF_WORD_TO_BE_READ			30

/*
 * TUNER_Seek_Start (0x26) macros
 */

/*!
 * \def		ETAL_CMDSEEKSTART_CMOST_CONTINUE_NB_PARAM
 * 			Number of parameters used for the Seek Start command
 * 			when used in continue mode.
 */
#define ETAL_CMDSEEKSTART_CMOST_CONTINUE_NB_PARAM      ((tU8)0x01)
/*!
 * \def		ETAL_CMDSEEKSTART_CMOST_START_NB_PARAM_WO_STEP
 * 			Number of parameters used for the Seek Start command
 * 			when the step is present.
 */
#define ETAL_CMDSEEKSTART_CMOST_START_NB_PARAM_WO_STEP ((tU8)0x02)
/*!
 * \def		ETAL_CMDSEEKSTART_CMOST_START_NB_PARAM_W_STEP
 * 			Number of parameters used for the Seek Start command
 * 			when the step is not present.
 */
#define ETAL_CMDSEEKSTART_CMOST_START_NB_PARAM_W_STEP  ((tU8)0x03)
/*!
 * \def		ETAL_CMOST_SEEK_START_MINIMUM_TIME
 * 			After the end of a Seek step (Automatic or Manual) ETAL waits
 * 			this amount of time (in ms) before returning from the
 * 			function call to let the quality measurements settle
 * 			on the STAR.
 */
#define ETAL_CMOST_SEEK_START_MINIMUM_TIME                10

/*
 * TUNER_Seek_End (0x27) macros
 */

/*!
 * \def		ETAL_CMOST_SEEK_STOP_MINIMUM_TIME
 *			Same as #ETAL_CMOST_SEEK_START_MINIMUM_TIME but 
 *			for the TUNER_Seek_End command.
 */
#define ETAL_CMOST_SEEK_STOP_MINIMUM_TIME                 10

/*
 * TUNER_Get_Seek_Status (0x28) macros
 */

/*!
 * \def		ETAL_CMOST_SEEKSTATUS_GOODSIGNAL
 * 			The first parameter of the response to the TUNER_Get_Seek_Status (0x28)
 * 			command contains the 'Current Seek RF Frequency'. The three
 * 			MSBits contain the status.
 * 			This macro accesses the bit that indicates if a station with
 * 			quality above threshold was found.
 */
#define ETAL_CMOST_SEEKSTATUS_GOODSIGNAL      0x800000
/*!
 * \def		ETAL_CMOST_SEEKSTATUS_FULLCYCLE
 * 			Accesses the bit that indicates if during the seek the whole band
 * 			was scanned.
 *  \see	ETAL_CMOST_SEEKSTATUS_GOODSIGNAL
 */
#define ETAL_CMOST_SEEKSTATUS_FULLCYCLE       0x400000
/*!
 * \def		ETAL_CMOST_SEEKSTATUS_WRAPPED
 * 			Accesses the bit that indicates if during the last seek execution 
 * 			the algorithm reached one of the band limits and restarted on the other
 * 			band limit.
 *  \see	ETAL_CMOST_SEEKSTATUS_GOODSIGNAL
 */
#define ETAL_CMOST_SEEKSTATUS_WRAPPED         0x200000  // undocumented on 27 Aug 2014
/*!
 * \def		ETAL_CMOST_SEEKSTATUS_FREQMASK
 * 			Bitmask to access the current RF frequency in the response
 * 			to TUNER_Get_Seek_Status (0x28).
 * \see		ETAL_CMOST_SEEKSTATUS_GOODSIGNAL
 */
#define ETAL_CMOST_SEEKSTATUS_FREQMASK        0x1FFFFF
/*!
 * \def		ETAL_CMD_SEEK_GET_STATUS_ANSWER_QUALITY_INDEX
 * 			Offset from the first byte of the CMOST answer to the command
 *			TUNER_Get_Seek_Status where the quality information is stored
 *			(the first parameter contains the seek status bits).
 */
#define ETAL_CMD_SEEK_GET_STATUS_ANSWER_QUALITY_INDEX	3	

/*!
 * \def     ETAL_CMOST_AF_START_MINIMUM_TIME
 *          After the end of an AF start, ETAL waits
 *          this amount of time (in ms) before returning from the
 *          function call to let the quality measurements settle
 *          on the STAR.
 */
#define ETAL_CMOST_AF_START_MINIMUM_TIME                4

/*!
 * \def     ETAL_CMOST_AF_END_MINIMUM_TIME
 *          After the end of an AF end, ETAL waits
 *          this amount of time (in ms) before returning from the
 *          function call to let the quality measurements settle
 *          on the STAR.
 */
#define ETAL_CMOST_AF_END_MINIMUM_TIME                  15

/*!
 * \def     ETAL_CMOST_AF_CHECK_MINIMUM_TIME
 *          After the end of an AF check, ETAL waits
 *          this amount of time (in ms) before returning from the
 *          function call to let the quality measurements settle
 *          on the STAR.
 */
#define ETAL_CMOST_AF_CHECK_MINIMUM_TIME                6

/*!
 * \def     ETAL_CMOST_AF_SWITCH_MINIMUM_TIME
 *          After the end of an AF switch, ETAL waits
 *          this amount of time (in ms) before returning from the
 *          function call to let the quality measurements settle
 *          on the STAR.
 */
#define ETAL_CMOST_AF_SWITCH_MINIMUM_TIME               15

/*!
 * \def     ETAL_CMD_AF_CHECK_ANSWER_QUALITY_INDEX
 *          Offset from the first byte of the CMOST answer to the command
 *          TUNER_AF_Check where the quality information is stored.
 */
#define ETAL_CMD_AF_CHECK_ANSWER_QUALITY_INDEX          0

/*!
 * \def     ETAL_CMD_AF_START_ANSWER_QUALITY_INDEX
 *          Offset from the first byte of the CMOST answer to the command
 *          TUNER_AF_Start where the quality information is stored.
 */
#define ETAL_CMD_AF_START_ANSWER_QUALITY_INDEX          0

/*!
 * \def     ETAL_CMD_AF_END_ANSWER_QUALITY_INDEX
 *          Offset from the first byte of the CMOST answer to the command
 *          TUNER_AF_End where the quality information is stored.
 */
#define ETAL_CMD_AF_END_ANSWER_QUALITY_INDEX            0

/*!
 * \def     ETAL_CMD_GET_AF_QUALITY_ANSWER_QUALITY_INDEX
 *          Offset from the first byte of the CMOST answer to the command
 *          TUNER_Get_AF_Quality where the quality information is stored.
 */
#define ETAL_CMD_GET_AF_QUALITY_ANSWER_QUALITY_INDEX    0

/*****************************************************************
| local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/

/*****************************************************************
| local functions
|----------------------------------------------------------------*/

/***************************
 *
 * ETAL_paramSetGeneric_CMOST
 *
 **************************/
/*!
 * \brief		Writes a 24-bit parameter to a CMOST command string
 * \param[out]	cmd - pointer to the location of the buffer containing the
 * 				      first byte of the CMOST command parameter to be written
 * \param[in]	p - the parameter to be written; only the 24 LSB are considered
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_paramSetGeneric_CMOST(tU8 *cmd, tU32 p)
{
	tU32 i = 0;

	cmd[i++] = (tU8)((p & 0x00FF0000) >> 16);
	cmd[i++] = (tU8)((p & 0x0000FF00) >>  8);
	cmd[i  ] = (tU8)((p & 0x000000FF) >>  0);
}

/***************************
 *
 * ETAL_commandStatusToEventErrStatus_CMOST
 *
 **************************/
/*!
 * \brief		Convert the CMOST error code to an internal code
 * \details		The function parses the two error codes returned by the CMOST,
 * 				the 'Checksum Error' and the 'Wrong CID' error.
 * \param[in]	cstatus - the command header byte containing the response status
 * \return		The converted status, or #EtalCommStatus_GenericError in case of
 * 				unrecognized error code or illegal *cstatus*
 * \see			CMOST_CHECKSUM_ERROR, CMOST_ILLEGAL_CID_ERROR
 * \callgraph
 * \callergraph
 */
static EtalCommErr ETAL_commandStatusToEventErrStatus_CMOST(tU16 cstatus)
{
	EtalCommErr retval;
	if (cstatus > ETAL_MAX_TU8_VALUE)
	{
		/* we expect the CMOST status to fit in a single byte only */
		ASSERT_ON_DEBUGGING(0);
		retval = EtalCommStatus_GenericError;
	}
	else if (CMOST_CHECKSUM_ERROR((tU8)cstatus))
	{
		retval = EtalCommStatus_ChecksumError;
	}
	else if (CMOST_ILLEGAL_CID_ERROR((tU8)cstatus))
	{
		retval = EtalCommStatus_MessageFormatError;
	}
	else
	{
		retval = EtalCommStatus_GenericError;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_sendCommunicationErrorEvent_CMOST
 *
 **************************/
/*!
 * \brief		Sends a communication error to the API user
 * \details		The function may generate the following error codes:
 * 				- #EtalCommStatus_TimeoutError
 * 				- #EtalCommStatus_ChecksumError
 * 				- #EtalCommStatus_MessageFormatError
 * 				- #EtalCommStatus_GenericError
 *
 * 				Other error codes are used by DCOP.
 * \param[in]	hReceiver - handle of the Receiver that triggered the error
 * \param[in]	retval - the OSAL error returned by the communication function
 * \param[in]	err_raw - the CMOST command header byte containing the response status
 * \param[in]	buf - pointer to the complete response, excluded the CMOST command header
 * 				      (i.e. the first #CMOST_HEADER_LEN bytes)
 * \param[in]	buf_len - size in bytes of the *buf* buffer.
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_sendCommunicationErrorEvent_CMOST(ETAL_HANDLE hReceiver, tSInt retval, tU16 err_raw, tU8 *buf, tU32 buf_len)
{
	if (retval == OSAL_ERROR_TIMEOUT_EXPIRED)
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_TimeoutError, err_raw, buf, buf_len);
	}
	else
	{
		ETAL_sendCommunicationErrorEvent(hReceiver, ETAL_commandStatusToEventErrStatus_CMOST(err_raw), err_raw, buf, buf_len);
	}
}

/**************************************
 *
 * ETAL_sendCmdCheckResult_CMOST
 *
 *************************************/
/*!
 * \brief		Same as #ETAL_sendCommandTo_CMOST but also processes the error condition
 * \param[in]	hTuner - handle of the Tuner to which the command is addressed
 * \param[in]	buf - array of bytes containing the command to send; the function
 * 				      does not make any assumption on the content of the array
 * \param[in]	len  - size in bytes of the *buf* buffer
 * \param[out]	resp  - pointer to a buffer where the function stores the complete CMOST response,
 * 				        or NULL. In the latter case the parameter is ignored. The response
 * 				        is an array of bytes including the the Command Parameters and 
 * 				        Checksum bytes. The caller must provide a buffer large enough to hold
 * 				        the largest CMOST answer (see #CMOST_MAX_RESPONSE_LEN).
 * \param[out]	rlen  - pointer to an integer where the function stores the size in bytes
 * 				        of the buffer written to *resp*. If NULL it is ignored.

 * \return		OSAL_OK
 * \return		OSAL_ERROR - write failure or system not configured
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_sendCmdCheckResult_CMOST(ETAL_HANDLE hTuner, tU8 *buf, tU32 len, tU8 *resp, tU32 *rlen)
{
	tU16 cstatus;
	tSInt retval;
	tU32 local_len;

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
	COMMON_tracePrintBufComponent(TR_CLASS_APP_ETAL_CMD, buf, len, "Command from Host:");
#endif

	retval = ETAL_sendCommandTo_CMOST(hTuner, buf, len, &cstatus, resp, rlen);
	if (retval != OSAL_OK)
	{
		/* avoid dereferencing a NULL pointer in the function call below */
		if (rlen == NULL)
		{
			local_len = 0;
		}
		else
		{
			local_len = *rlen;
		}
		ETAL_sendCommunicationErrorEvent_CMOST(ETAL_INVALID_HANDLE, retval, cstatus, resp, local_len);
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
 * ETAL_paramSetFrequency_CMOST
 *
 **************************/
/*!
 * \brief		Writes the 'Frequency' parameter of the TUNER_Tune (0x08) command
 * \details		The function assumes that *cmd* points to a Tune command and writes the
 * 				frequency parameter in the correct location.
 * \param[in]	f - the frequency
 * \param[in,out] cmd - pointer to the buffer containing the first byte of the CMOST command
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_paramSetFrequency_CMOST(tU8 *cmd, tU32 f)
{
	ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM2_24BIT], f);
}

/***************************
 *
 * ETAL_paramSetTunerChannel_CMOST
 *
 **************************/
/*!
 * \brief		Writes the 'Tuner channel' parameter to a CMOST command buffer
 * \details		Many CMOST commands require the channel specifier in the first
 * 				parameter. This function writes it deriving it from the *hReceiver*.
 * \param[in,out] cmd - pointer to a buffer containing the command to be sent to
 * 				        the CMOST
 * \param[in]	hReceiver - the handle of the Receiver, used to extract the Tuner
 * 				            channel
 * \return		The identifier of the channel just written to the CMOST command
 * \callgraph
 * \callergraph
 */
static tU32 /*@alt void@*/ ETAL_paramSetTunerChannel_CMOST(tU8 *cmd, ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
	tU32 retval;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp == NULL)
	{
		retval = ETAL_CHN_UNDEF;
	}
	else if (recvp->CMOSTConfig.channel == ETAL_CHN_UNDEF)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD,"ETAL_paramSetTunerChannel_CMOST invalid channel : hReceiver = %d \n", hReceiver);
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_CHN_UNDEF;
	}
	else if (((recvp->CMOSTConfig.processingFeatures.u.m_processing_features & (tU8)ETAL_PROCESSING_FEATURE_FM_VPA) != (tU8)0) &&
		((recvp->CMOSTConfig.m_DebugVPAMode == ETAL_DEBUG_VPA_MODE_ON) || (recvp->CMOSTConfig.m_DebugVPAMode == ETAL_DEBUG_VPA_MODE_NONE)))
	{
		/* in FM VPA Mode send  command on foreground tuner only */
		ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM1_24BIT], ETAL_CHN_FOREGROUND);
		retval = (tU32)ETAL_CHN_FOREGROUND;
	}
	else
	{
		ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM1_24BIT], recvp->CMOSTConfig.channel);
		retval = (tU32)recvp->CMOSTConfig.channel;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_paramSetBand_CMOST
 *
 **************************/
/*!
 * \brief		Writes the 'Tuner Mode' parameter of the TUNER_Change_Band (0x0A) command
 * \details		Converts *band* from ETAL internal code to CMOST code and
 * 				writes it to the *cmd* CMOST command buffer. Also
 * 				writes the *bandMin* and *bandMax* parameters.
 * \param[in,out] cmd - pointer to a buffer containing the command to be sent to
 * 				        the CMOST
 * \param[in]	band - the band identifier, in ETAL format
 * \param[in]	bandMin - the min band limit in Hz
 * \param[in]	bandMax - the max band limit in Hz
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_paramSetBand_CMOST(tU8 *cmd, EtalFrequencyBand band, tU32 bandMin, tU32 bandMax)
{
	tU32 mode = ETAL_CMOST_TUNER_MODE_STANDBY; // standby

	if (band == ETAL_BAND_WB) // must come before FM-catch-all comparison
	{
		mode = ETAL_CMOST_TUNER_MODE_WB;
	}
	else if ((band & ETAL_BAND_FM_BIT) == ETAL_BAND_FM_BIT)
	{
		mode = ETAL_CMOST_TUNER_MODE_FM;
	}
	else if (band == ETAL_BAND_DAB3)
	{
		mode = ETAL_CMOST_TUNER_MODE_DAB3;
	}
	else if (band == ETAL_BAND_DABL)
	{
		mode = ETAL_CMOST_TUNER_MODE_DABL;
	}
	else if (band == ETAL_BAND_MWUS) // must come before AM-catch-all comparison
	{
		mode = ETAL_CMOST_TUNER_MODE_AMUS;
	}
	else if ((band & ETAL_BAND_AM_BIT) == ETAL_BAND_AM_BIT)
	{
		mode = ETAL_CMOST_TUNER_MODE_AMEU;
	}
	else if (band == ETAL_BAND_DRM30)
	{
		mode = ETAL_CMOST_TUNER_MODE_DRM30;
	}
	else if (band == ETAL_BAND_DRMP)
	{
		mode = ETAL_CMOST_TUNER_MODE_DRMP;
	}
	else
	{
		/* Nothing to do */
	}

	ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM2_24BIT], mode);
	ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM4_24BIT], bandMin);
	ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM5_24BIT], bandMax);
}

/***************************
 *
 * ETAL_EtalStandard_To_CMOST_Band
 *
 **************************/
/*!
 * \brief		Convert the ETAL standard to CMOST band  
 * \details		Converts *band* from ETAL internal code to CMOST code and
 
 * \param[in,out] cmd - pointer to a buffer containing the command to be sent to
 * 				        the CMOST
 * \param[in]	band - the band identifier, in ETAL format
 *
 * \return		The etalCMOST mode
 * \callergraph
 */
etalCMOSTTunerMode ETAL_EtalStandard_To_CMOST_Band(EtalBcastStandard std)
{
	etalCMOSTTunerMode mode = ETAL_CMOST_TUNER_MODE_STANDBY; // standby
	EtalFrequencyBand band;

	band = ETAL_BAND_UNDEF;
	switch (std)
	{
		case ETAL_BCAST_STD_FM:
			band = ETAL_BAND_FM;
			break;

		case ETAL_BCAST_STD_AM:
			band = ETAL_BAND_AM;
			break;

		case ETAL_BCAST_STD_DAB:
			band = ETAL_BAND_DAB3;
			break;

		case ETAL_BCAST_STD_DRM:
			band = ETAL_BAND_DRM30;
			break;

		case ETAL_BCAST_STD_HD_FM:
			band = ETAL_BAND_HD;
			break;

		case ETAL_BCAST_STD_HD_AM:
			band = ETAL_BAND_MWUS;
			break;

		default:
			break;
	}
	


	if (band == ETAL_BAND_WB) // must come before FM-catch-all comparison
	{
		mode = ETAL_CMOST_TUNER_MODE_WB;
	}
	else if ((band & ETAL_BAND_FM_BIT) == ETAL_BAND_FM_BIT)
	{
		mode = ETAL_CMOST_TUNER_MODE_FM;
	}
	else if (band == ETAL_BAND_DAB3)
	{
		mode = ETAL_CMOST_TUNER_MODE_DAB3;
	}
	else if (band == ETAL_BAND_DABL)
	{
		mode = ETAL_CMOST_TUNER_MODE_DABL;
	}
	else if (band == ETAL_BAND_MWUS) // must come before AM-catch-all comparison
	{
		mode = ETAL_CMOST_TUNER_MODE_AMUS;
	}
	else if ((band & ETAL_BAND_AM_BIT) == ETAL_BAND_AM_BIT)
	{
		mode = ETAL_CMOST_TUNER_MODE_AMEU;
	}
	else if (band == ETAL_BAND_DRM30)
	{
		mode = ETAL_CMOST_TUNER_MODE_DRM30;
	}
	else if (band == ETAL_BAND_DRMP)
	{
		mode = ETAL_CMOST_TUNER_MODE_DRMP;
	}
	else
	{
		/* Nothing to do */
	}

	return mode;
}


#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
/***************************
 *
 * ETAL_paramSeekSetMode_CMOST
 *
 **************************/
/*!
 * \brief		Writes the 'configuration' parameter of the TUNER_Seek_Start (0x26) command
 * \details		The function writes only the bits 0-2 of the configuration parameter
 * \param[in,out] cmd - pointer to a buffer containing the command to be sent to
 * 				        the CMOST
 * \param[in]	dir - seek direction (up/down)
 * \param[in]	exitState - seek exit status (mute/unmute)
 * \param[in]	seekMode - seek mode (use predefined step or specific step)
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_paramSeekSetMode_CMOST(tU8 *cmd, etalSeekDirectionTy dir, etalSeekAudioTy exitState, etalSeekModeTy seekMode)
{
	tU32 param;
	tU32 mode =      (tU32)((seekMode == cmdAutomaticModeStart) ? 1 : 0);
	tU32 direction = (tU32)((dir == cmdDirectionDown) ? 1 : 0);
	tU32 mute =      (tU32)((exitState == cmdAudioMuted) ? 1 : 0);

	/* auto mode, direction from parameter, unmute */

	param = (mode << 0) | (direction << 1) | (mute << 2);
	ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM2_24BIT], param);
}

/***************************
 *
 * ETAL_paramSeekSetLength_CMOST
 *
 **************************/
/*!
 * \brief		Writes the 'length' byte of the CMOST Command Header
 * \param[in,out] cmd - pointer to a buffer containing the command to be sent to
 * 				        the CMOST
 * \param[in]	size - the value to be written
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_paramSeekSetLength_CMOST(tU8 *cmd, tU8 size)
{
	cmd[2] = size;
}

/***************************
 *
 * ETAL_extractAmFmReceptionQualityFromNotification_CMOST
 *
 **************************/
/*!
 * \brief		Parse a CMOST response and extracts the quality parameters
 * \details		The function supports two types of quality parameters, depending on
 * 				the *type* parameter.
 * \remark		
 * \param[in]	type - the type of quality parameters to extract
 * \param[out]	d - pointer to the quality container to write to
 * \param[in]	standard - the broadcast standard
 * \param[in]	payload - pointer to the CMOST response
 * \callgraph
 * \callergraph
 */
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC) && (defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL) || \
	defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BF) || defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BG) || \
	defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_CA) || defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_DA) || \
	defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_EB))
/* The STAR-T BC uses a FW version with the old (3-parameter) quality response,
 * newer cuts use the 4-parameter quality response.
 * To make the code more readable there are two versions of the same function.
 * To avoid complicating the code, the STAR-T BC cannot be build with other silicon types.
 */
	#error "Configuration with CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC and other silicons not supported"
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC)
/*
 * OLD CODE TO BE REMOVED!
 *
 * see comment above.
 */
static tVoid ETAL_extractAmFmQualityFromNotification_CMOST(EtalQualityReportingType type, EtalBcastQualityContainer *d, EtalBcastStandard standard, tU8 *payload)
{
	EtalFmQualityEntries *pQualityentries = NULL;

	if (ETAL_IS_HDRADIO_STANDARD(standard))
	{
		pQualityentries = &(d->EtalQualityEntries.hd.m_analogQualityEntries);
	}
	else
	{
		pQualityentries = &(d->EtalQualityEntries.amfm);
	}

	switch(type)
	{
		case receptionQuality:
			if ((standard == ETAL_BCAST_STD_HD_FM) || (standard == ETAL_BCAST_STD_FM))
			{
				pQualityentries->m_RFFieldStrength 					=   ETAL_CMOST_GET_AMFMMON_FM_RF_FIELD_STRENGTH(payload[0]);
				pQualityentries->m_BBFieldStrength 					=   ETAL_CMOST_GET_AMFMMON_FM_BB_FIELD_STRENGTH(payload[1]);
				pQualityentries->m_FrequencyOffset 					=   ETAL_CMOST_GET_AMFMMON_FM_DETUNING(payload[2]);
				pQualityentries->m_Multipath 						=   ETAL_CMOST_GET_AMFMMON_FM_MULTIPATH(payload[3]);
				pQualityentries->m_UltrasonicNoise 					=   ETAL_CMOST_GET_AMFMMON_FM_MPX_NOISE(payload[4]);
				pQualityentries->m_AdjacentChannel 					=   ETAL_VALUE_NOT_AVAILABLE(tS32);
				pQualityentries->m_ModulationDetector 				=	ETAL_CMOST_GET_AMFMMON_FM_DEVIATION(payload[5]);
				pQualityentries->m_SNR								=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_coChannel						=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_StereoMonoReception 				= 	ETAL_CMOST_GET_AMFMMON_FM_STEREO_MONO_RECEPTION(payload[5]);
			}
			else if ((standard == ETAL_BCAST_STD_HD_AM) || (standard == ETAL_BCAST_STD_AM))
			{
				pQualityentries->m_RFFieldStrength 					=   ETAL_CMOST_GET_AMFMMON_AM_RF_FIELD_STRENGTH(payload[0]);
				pQualityentries->m_BBFieldStrength 					=   ETAL_CMOST_GET_AMFMMON_AM_BB_FIELD_STRENGTH(payload[1]);
				pQualityentries->m_FrequencyOffset 					=   ETAL_CMOST_GET_AMFMMON_AM_DETUNING(payload[2]);
				pQualityentries->m_Multipath 						=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_UltrasonicNoise 					=  	ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_AdjacentChannel 					=   ETAL_CMOST_GET_AMFMMON_AM_ADJACENT_CHANNEL(payload[4]);
				pQualityentries->m_ModulationDetector 				=   ETAL_CMOST_GET_AMFMMON_AM_MODULATION(payload[5]);
				pQualityentries->m_SNR								=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_coChannel						=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_StereoMonoReception 				=   ETAL_VALUE_NOT_AVAILABLE(tU32);
			}
			break;

		case channelQuality:
		case channelQualityForAF:
			if ((standard == ETAL_BCAST_STD_HD_FM) || (standard == ETAL_BCAST_STD_FM))
			{
				pQualityentries->m_RFFieldStrength 					=   ETAL_CMOST_GET_AMFMMON_FM_RF_FIELD_STRENGTH(payload[0]);
				pQualityentries->m_BBFieldStrength 					=   ETAL_CMOST_GET_AMFMMON_FM_BB_FIELD_STRENGTH(payload[1]);
				pQualityentries->m_FrequencyOffset 					=   ETAL_CMOST_GET_AMFMMON_FM_DETUNING(payload[2]);
				pQualityentries->m_Multipath 						=   ETAL_CMOST_GET_AMFMMON_FM_MULTIPATH(payload[3]);
				pQualityentries->m_UltrasonicNoise 					=   ETAL_CMOST_GET_AMFMMON_FM_MPX_NOISE(payload[4]);
				pQualityentries->m_AdjacentChannel 					=   ETAL_CMOST_GET_AMFMMON_FM_ADJACENT_CHANNEL(payload[5]);
				pQualityentries->m_ModulationDetector 				=	ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_SNR								=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_coChannel						=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_StereoMonoReception 				= 	ETAL_VALUE_NOT_AVAILABLE(tU32);
			}
			else if ((standard == ETAL_BCAST_STD_HD_AM) || (standard == ETAL_BCAST_STD_AM))
			{
				if (type == channelQualityForAF)
				{
					/* AF not available in AM */
					ETAL_resetQualityContainer(standard, d);
				}
				else
				{
				pQualityentries->m_RFFieldStrength 					=   ETAL_CMOST_GET_AMFMMON_AM_RF_FIELD_STRENGTH(payload[0]);
				pQualityentries->m_BBFieldStrength 					=   ETAL_CMOST_GET_AMFMMON_AM_BB_FIELD_STRENGTH(payload[1]);
				pQualityentries->m_FrequencyOffset 					=   ETAL_CMOST_GET_AMFMMON_AM_DETUNING(payload[2]);
				pQualityentries->m_Multipath 						=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_UltrasonicNoise					=  	ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_AdjacentChannel 					=   ETAL_CMOST_GET_AMFMMON_AM_ADJACENT_CHANNEL(payload[4]);
				pQualityentries->m_ModulationDetector 				=   ETAL_CMOST_GET_AMFMMON_AM_MODULATION(payload[5]);
				pQualityentries->m_SNR								=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_coChannel						=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_StereoMonoReception 				=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				}
			}
			break;

		default:
			/* Undefined type */
			ETAL_resetQualityContainer(standard, d);
			break;
	}
	return;
}
#else // !CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
static tVoid ETAL_extractAmFmQualityFromNotification_CMOST(EtalQualityReportingType type, EtalBcastQualityContainer *d, EtalBcastStandard standard, tU8 *payload)
{
	EtalFmQualityEntries *pQualityentries = NULL;
	EtalDabQualityEntries *pDABQualityentries = NULL;
	switch(standard)
	{
		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_FM:
			if(standard == ETAL_BCAST_STD_HD_FM)
			{
				pQualityentries = &(d->EtalQualityEntries.hd.m_analogQualityEntries);
			}
			else
			{
				pQualityentries = &(d->EtalQualityEntries.amfm);
			}

			if ((type == receptionQuality) || (type == channelQuality) || (type == channelQualityForAF))
			{
				pQualityentries->m_RFFieldStrength 			=   ETAL_CMOST_GET_AMFMMON_FM_RF_FIELD_STRENGTH(payload[0]);
				pQualityentries->m_BBFieldStrength 			=   ETAL_CMOST_GET_AMFMMON_FM_BB_FIELD_STRENGTH(payload[1]);
				pQualityentries->m_FrequencyOffset 			=   ETAL_CMOST_GET_AMFMMON_FM_DETUNING(payload[2]);
				pQualityentries->m_Multipath 				=   ETAL_CMOST_GET_AMFMMON_FM_MULTIPATH(payload[3]);
				pQualityentries->m_UltrasonicNoise 			=   ETAL_CMOST_GET_AMFMMON_FM_MPX_NOISE(payload[4]);
				pQualityentries->m_AdjacentChannel 			=   ETAL_CMOST_GET_AMFMMON_FM_ADJACENT_CHANNEL(payload[6]);
				pQualityentries->m_SNR						=   ETAL_CMOST_GET_AMFMMON_FM_SNR(payload[5]);
				pQualityentries->m_ModulationDetector 		=	ETAL_CMOST_GET_AMFMMON_FM_DEVIATION(payload[8]);
				pQualityentries->m_coChannel				=   ETAL_CMOST_GET_AMFMMON_FM_COCHANNEL(payload[7]);
				pQualityentries->m_StereoMonoReception 		= 	ETAL_CMOST_GET_AMFMMON_FM_STEREO_MONO_RECEPTION(payload[8]);

				if(type == channelQualityForAF)
				{
					pQualityentries->m_ModulationDetector 	=	ETAL_VALUE_NOT_AVAILABLE(tU32);
					pQualityentries->m_coChannel			=   ETAL_VALUE_NOT_AVAILABLE(tU32);
					pQualityentries->m_StereoMonoReception 	= 	ETAL_VALUE_NOT_AVAILABLE(tU32);
				}

				if (type != receptionQuality)
				{
					pQualityentries->m_StereoMonoReception 	= 	ETAL_VALUE_NOT_AVAILABLE(tU32);
				}
			}
			else
			{
				/* Undefined type */
				ETAL_resetQualityContainer(standard, d);
			}
			break;

		case ETAL_BCAST_STD_HD_AM:
		case ETAL_BCAST_STD_AM:
			if(standard == ETAL_BCAST_STD_HD_FM)
			{
				pQualityentries = &(d->EtalQualityEntries.hd.m_analogQualityEntries);
			}
			else
			{
				pQualityentries = &(d->EtalQualityEntries.amfm);
			}

			if ((type == receptionQuality) || (type == channelQuality)) /* AF not available in AM */
			{
				pQualityentries->m_RFFieldStrength 			=   ETAL_CMOST_GET_AMFMMON_AM_RF_FIELD_STRENGTH(payload[0]);
				pQualityentries->m_BBFieldStrength 			=   ETAL_CMOST_GET_AMFMMON_AM_BB_FIELD_STRENGTH(payload[1]);
				pQualityentries->m_FrequencyOffset 			=   ETAL_CMOST_GET_AMFMMON_AM_DETUNING(payload[2]);
				pQualityentries->m_Multipath 				=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_UltrasonicNoise 			=  	ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_AdjacentChannel 			=   ETAL_CMOST_GET_AMFMMON_AM_ADJACENT_CHANNEL(payload[6]);
				pQualityentries->m_ModulationDetector 		=   ETAL_CMOST_GET_AMFMMON_AM_MODULATION(payload[8]);
				pQualityentries->m_SNR						=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_coChannel				=   ETAL_VALUE_NOT_AVAILABLE(tU32);
				pQualityentries->m_StereoMonoReception 		=   ETAL_VALUE_NOT_AVAILABLE(tU32);
			}
			else
			{
				/* Undefined type */
				ETAL_resetQualityContainer(standard, d);
			}
			break;

		case ETAL_BCAST_STD_DAB:
			pDABQualityentries = &(d->EtalQualityEntries.dab);

			if (type == channelQuality) /* AF and reception quality not available in DAB */
			{
				pDABQualityentries->m_RFFieldStrength       = ETAL_CMOST_GET_AMFMMON_DAB_RF_FIELD_STRENGTH(payload[0]);
				/* BB field strength is provided by the DCOP */
			}
			else
			{
				/* Undefined type */
				ETAL_resetQualityContainer(standard, d);
			}
			break;

		default:
			break;
	}
	return;
}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_BA || CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_BC
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
/***************************
 *
 * ETAL_paramSetSeekThresholds_CMOST
 *
 **************************/
/*!
 * \brief		Writes the 'Seek threshold fstBB' and 'Seek threshold mp_noise_snr' parameters of the TUNER_Set_Seek_Thresholds (0x29) command
 * \param[in,out] cmd - pointer to a buffer containing the command to be sent to
 * 				        the CMOST
 * \param[in]	threshold - Pointer to the thresholds to be written
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_paramSetSeekThresholds_CMOST(tU8 *cmd, EtalSeekThreshold *threshold)
{
	tU32 param2 = 0, param3 = 0, param4 = 0;

	param2 = ((tU32)(threshold->SeekThresholdBBFieldStrength) <<  8) & 0x0000FF00;
	param2 |= ((tU32)threshold->SeekThresholdDetune <<  0) & 0x000000FF;

	param3 = ((tU32)threshold->SeekThresholdMultipath << 16) & 0x00FF0000;
	param3 |= ((tU32)(threshold->SeekThresholdMpxNoise) <<  8) & 0x0000FF00;
	param3 |= ((tU32)threshold->SeekThresholdSignalNoiseRatio <<  0) & 0x000000FF;
	
	param4 = ((tU32)threshold->SeekThresholdAdjacentChannel << 16) & 0x00FF0000;
	param4 |= ((tU32)threshold->SeekThresholdCoChannel << 8) & 0x0000FF00;

	ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM2_24BIT], param2);
	ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM3_24BIT], param3);
	ETAL_paramSetGeneric_CMOST(&cmd[ETAL_CMOST_PARAM4_24BIT], param4);
}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

/**************************************
 *
 * ETAL_directCmdPing_CMOST
 *
 *************************************/
/*!
 * \brief		Sends a TUNER_Ping (0x00) command to the CMOST
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner  - the Tuner handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_FROM_SLAVE    - unexpected response from the CMOST
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *hReceiver*
 * \callgraph
 * \callergraph
 * \todo		Redefine last_word to be of size ETAL_CAPA_MAX_TUNER
 */
tSInt ETAL_directCmdPing_CMOST(ETAL_HANDLE hTuner)
{
	                      //            CID              PAR1
	tU8 CMOSTcmd_Ping[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
	static tU8 last_word[4] = {0x00, 0x00, 0x00, 0x00};
#if (defined(ETAL_CAPA_MAX_TUNER) > 4)
	#error "Unsupported number of tuner for last_word[ETAL_CAPA_MAX_TUNER]"
#endif
	tSInt retval;
	tU8 resp[CMOST_MAX_RESPONSE_LEN] = { 0, 0, 0 };
	tU32 rlen;
	tU32 tuner_index;
	tU16 cstatus;

	tuner_index = (tU32)ETAL_handleTunerGetIndex(hTuner);
	if (tuner_index >= 4)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = OSAL_ERROR;
		goto exit;
	}

	/* Change the word for consecutive call */
	CMOSTcmd_Ping[4] = (tU8)((last_word[tuner_index] != (tU8)0x01) ? 0x01 : 0x02);
	last_word[tuner_index] = CMOSTcmd_Ping[4];

	/* Tag the word with Tuner ID */
	CMOSTcmd_Ping[5] = (tU8)tuner_index;

	retval = ETAL_sendCommandTo_CMOST(hTuner, CMOSTcmd_Ping, sizeof(CMOSTcmd_Ping), &cstatus, resp, &rlen);

	if (retval != OSAL_OK)
	{
		// notification is handled by the event
		ETAL_sendCommunicationErrorEvent_CMOST(ETAL_INVALID_HANDLE , retval, cstatus, NULL, 0);
		last_word[tuner_index] = (tU8)0;
	}
	else
	{
		/* check response is different from previous response */
		if(((CMOSTcmd_Ping[3] ^ resp[0]) != (tU8)0xFF) ||
		   ((CMOSTcmd_Ping[4] ^ resp[1]) != (tU8)0xFF) ||
		   ((CMOSTcmd_Ping[5] ^ resp[2]) != (tU8)0xFF))
		{
			last_word[tuner_index] = (tU8)0;
			// OSAL_ERROR is already used for the communication error and also in this case we don't
			// send any event so we should use a different return value
			retval = OSAL_ERROR_FROM_SLAVE;

			ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_directCmdPing_CMOST : uncorrect PING response posted received => OSAL_ERROR_FROM_SLAVE\n");	
		}
	}

exit:
	return retval;
}

/**************************************
 *
 * ETAL_directCmdWrite_CMOST
 *
 *************************************/
/*!
 * \brief		Writes a 24-bit value to a CMOST memory location
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner  - the Tuner handle
 * \param[in]	address - the CMOST memory address
 * \param[in]	value   - the value to be written; only the 3 LSBytes are considered
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdWrite_CMOST(ETAL_HANDLE hTuner, tU32 address, tU32 value)
{
                           //            CID              PAR1              PAR2
	tU8 CMOSTcmd_Write[] = {0x00, 0x1F, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_Write[ETAL_CMOST_PARAM1_24BIT], address);
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_Write[ETAL_CMOST_PARAM2_24BIT], value);
	return ETAL_sendCmdCheckResult_CMOST(hTuner, CMOSTcmd_Write, sizeof(CMOSTcmd_Write), NULL, NULL);
}

/**************************************
 *
 * ETAL_directCmdRead_CMOST
 *
 *************************************/
/*!
 * \brief		Reads one or more 24-bit values from a CMOST memory location
 * \remark		CMOST supports reading at most #MAX_NB_OF_WORD_TO_BE_READ 24-bit words
 * 				in each Read command. This function **does not** validate the *value*
 * 				parameter against this limit.
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner  - the Tuner handle
 * \param[in]	address - the CMOST memory address
 * \param[in]	value   - the number of 24-bit locations to read; should be less than
 * 				          #MAX_NB_OF_WORD_TO_BE_READ
 * \param[out]	resp    - the address of a buffer where the function stores the CMOST
 * 				          response (parameters only, not including the Command Header).
 * 				          The buffer should be large enough to store the largest CMOST
 * 				          answer, that is (#MAX_NB_OF_WORD_TO_BE_READ x 3).
 * \param[out]	rlen    - the number of bytes in the *resp* buffer written by the function.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdRead_CMOST(ETAL_HANDLE hTuner, tU32 address, tU32 value, tU8 *resp, tU32 *rlen)
{
                          //            CID              PAR1              PAR2
	tU8 CMOSTcmd_Read[] = {0x00, 0x1E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_Read[ETAL_CMOST_PARAM1_24BIT], address);
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_Read[ETAL_CMOST_PARAM2_24BIT], value);
	return ETAL_sendCmdCheckResult_CMOST(hTuner, CMOSTcmd_Read, sizeof(CMOSTcmd_Read), resp, rlen);
}


/**************************************
 *
 * ETAL_directCmdSetBBIf_CMOST
 *
 *************************************/
/*!
 * \brief		Configures the Baseband and Audio interfaces
 * \details		This command configures both the Baseband Interface used to exchange
 * 				digital data with the DCOP, and the Audio Interface. In particular the
 * 				latter selects how audio signals are routed to the CMOST GPIOs.
 * 				This command implements command Tuner_Set_BB_IF (0x04) of the
 * 				CMOST API Specification version 2.51 or later. The 'mode' parameters
 * 				accept the format described in the above document, with 
 * 				*bbi_mode* corresponding to bits 3..0 of Parameter1 and
 * 				*ai_mode* corresponding to bits 19..16 of Parameter1.
 * \remark		The function does not check if the *bb_mode*, *ai_mode* combination
 * 				is valid.
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner - the Tuner handle
 * \param[in]	bbi_mode - Baseband Interface mode, see CMOST API doc for details
 * \param[in]	ai_mode  - Audio Interface mode, see CMOST API doc for details
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdSetBBIf_CMOST(ETAL_HANDLE hTuner, EtalBBIntfModeTy bb_mode, EtalAudioIntfModeTy ai_mode)
{
                              //            CID              PAR1 
	tU8 CMOSTcmd_SetBBIF[] =  {0x00, 0x04, 0x01, 0x00, 0x00, 0x00}; // PAR1 modified below
	tU32 par1 = 0;

#if defined(CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_BA)
	/* Audio interface Mode is only supported on STAR-S BC FW 3.1.2 or later
	 * and not on STAR-T FW 4.3.3 or earlier 
	 * but now STAR-T is 4.3.4 */
	ai_mode = 0;
#endif

	par1 = ((tU32)ai_mode << 16) | (tU32)bb_mode;
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_SetBBIF[ETAL_CMOST_PARAM1_24BIT], par1);
	return ETAL_sendCmdCheckResult_CMOST(hTuner, CMOSTcmd_SetBBIF, sizeof(CMOSTcmd_SetBBIF), NULL, NULL);
}

/**************************************
 *
 * ETAL_directCmdAudioMute_CMOST
 *
 *************************************/
/*!
 * \brief		Mutes the CMOST
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner  - the Tuner handle
 * \param[in]	muteAction - 0 to mute Audio, 1 to unmute, 3 to unmute all (audio and tuner)
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdAudioMute_CMOST(ETAL_HANDLE hTuner, tU8 muteAction)
{ 
                             //            CID              PAR1 
	tU8 CMOSTcmd_AudioMute[] = {0x00, 0x16, 0x01, 0x00, 0x00, 0x00};

	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AudioMute[ETAL_CMOST_PARAM1_24BIT], (tU32)muteAction);

	return ETAL_sendCmdCheckResult_CMOST(hTuner, CMOSTcmd_AudioMute, sizeof(CMOSTcmd_AudioMute), NULL, NULL);
}

#if defined (CONFIG_ETAL_HAVE_AUDIO_CONTROL) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_directCmdFMStereoMode_CMOST
 *
 **************************/
/*!
 * \brief		Sends a Tuner_FM_Stereo_Mode (0x17) to the CMOST
 * \remark		The function does not validate the *fmStereoMode* parameter.
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner       - the Tuner handle
 * \param[in]	fmStereoMode - 0 for automatic selection, 1 to force mono, 2 to force stereo.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdFMStereoMode_CMOST(ETAL_HANDLE hTuner, tU8 fmStereoMode)
{
                                  //            CID              PAR1
	tU8 CMOSTcmd_FMStereoMode[] = {0x00, 0x17, 0x01, 0x00, 0x00, 0x00};

	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_FMStereoMode[ETAL_CMOST_PARAM1_24BIT], (tU32)fmStereoMode);

	return ETAL_sendCmdCheckResult_CMOST(hTuner, CMOSTcmd_FMStereoMode, sizeof(CMOSTcmd_FMStereoMode), NULL, NULL);
}
#endif // CONFIG_ETAL_HAVE_AUDIO_CONTROL || CONFIG_ETAL_HAVE_ALL_API

/**************************************
 *
 * ETAL_directCmdSetAudioIf_CMOST
 *
 *************************************/
/*!
 * \brief		Configure correctly  the audio DAC and the SAI input
 * \details		The CMOST SAI input may be used (e.g. on the Accordo2 EVB) to route
 * 				the digital audio from the DCOP to the CMOST.
 * \remark		This function is used only at ETAL startup to set the initial
 * 				audio interface status to DAC enabled, SAI in enabled
 * 				For regular run-time operation use the #ETAL_cmdSelectAudioSource_CMOST
 * 				instead.
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner  - the Tuner handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdSetAudioIf_CMOST(ETAL_HANDLE hTuner, etalAudioIntfStatusTy intf)
{ 
                             //            CID              PAR1 
	tU8 CMOSTcmd_AudioIF[] = {0x00, 0x0D, 0x01, 0x00, 0x00, (tU8)(0x01 | 0x04)};

	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AudioIF[ETAL_CMOST_PARAM1_24BIT], (tU32)intf.all);

	return ETAL_sendCmdCheckResult_CMOST(hTuner, CMOSTcmd_AudioIF, sizeof(CMOSTcmd_AudioIF), NULL, NULL);
}

#if (defined CONFIG_ETAL_SUPPORT_DCOP)
/**************************************
 *
 * ETAL_directCmdSetHDBlender_CMOST
 *
 *************************************/
/*!
 * \brief		Select the audio input
 * \details		This command is used to route the audio signal from either the
 * 				internally decoded AM/FM analogue audio or from an external
 * 				digital audio signal (e.g. the decoded audio provided by a DCOP).
 * 				Another possibility is to dynamically switch the audio routing
 * 				from internal analogue to external digital based on the value of
 * 				a CMOST hardware pin; this is referred as auto mode and is used
 * 				in presence of an HDRadio DCOP that can measure the quality
 * 				of the FM signal, the quality of the HDRadio signal and indicate
 * 				to the STAR which input to use for better quality.
 * \remark		This function is used only at ETAL startup to set the initial
 * 				HDBlender operational mode. During regular operation use the
 * 				#ETAL_cmdSelectAudioSource_CMOST instead.
 * \remark		The mode of operation selected by *mode* is valid only for the
 * 				ETAL startup; later ETAL uses a different method to select
 * 				the audio input (see #ETAL_cmdSelectAudioSource_CMOST).
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner - the Tuner handle
 * \param[in]	mode   - the audio mode; this should be a combination of
 * 				         #ETAL_SETHDBLENDER_DIGITAL_CMOST, #ETAL_SETHDBLENDER_AUTO_CMOST and
 * 				         #ETAL_SETHDBLENDER_AUTO_CMOST values
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdSetHDBlender_CMOST(ETAL_HANDLE hTuner, tU8 mode)
{
                               //            CID              PAR1 
	tU8 CMOSTcmd_HDBlender[] = {0x00, 0x14, 0x01, 0x00, 0x00, 0x00};

	CMOSTcmd_HDBlender[5] = mode;
	return ETAL_sendCmdCheckResult_CMOST(hTuner, CMOSTcmd_HDBlender, sizeof(CMOSTcmd_HDBlender), NULL, NULL);
}

#if defined (CONFIG_MODULE_INDEPENDENT)
/**************************************
 *
 * ETAL_directCmdSetGPIOforAudioInput_CMOST
 *
 *************************************/
/*!
 * \brief		GPIO settings for audio input
 * \details		This special configuration defines a CMOST GPIO to be used for PCM
 * 				interface to DAB; when configured this way the RDS IRQ does not work
 * 				anymore. This limitation is only for older MDR modules which routed
 * 				the audio on a 'wrong' connector pin. The MTD does not have this
 * 				problem so this fix (starting from FW 3.7.2) is not necessary and
 * 				both audio and IRQ are available. Also the fix will no longer be necessary
 * 				for DAB once the MDR modules will be modified by hand or a new
 * 				spin of the board will be available (June '15).
 *
 * 				For older HW the only solution for non-MTD hardware is to choose
 * 				whether to get the IRQ or the DAB audio.
 * \remark		This function is included only for support of old hardware; its use
 * 				on current hardware is **DEPRECATED**.
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner  - the Tuner handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdSetGPIOforAudioInput_CMOST(ETAL_HANDLE hTuner)
{
	tSInt retval = OSAL_OK;

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR) && defined (CONFIG_MODULE_DCOP_MDR_SPECIAL_AUDIO_GPIO)
	/*
	 * Use the generic Tuner_Write command (0x1F)
	 * the GUI command is
	 * WD 14016 3F3F3F1D 1D1C003F 0000001D
	 */
	tU8 CMOST_SetGPIOforAudioInput_MDR[] = {0x00, 0x1f, 0x07, 0x01, 0x40, 0x16, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x1d, 0x1d, 0x1c, 0x00, 0x1c, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1d};

	retval = ETAL_sendCmdCheckResult_CMOST(hTuner, CMOST_SetGPIOforAudioInput_MDR, sizeof(CMOST_SetGPIOforAudioInput_MDR), NULL, NULL);

#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO) && defined (CONFIG_MODULE_DCOP_HDRADIO_SPECIAL_AUDIO_GPIO)
	/*
	 * Use the generic Tuner_Write command (0x1F)
	 * the GUI command is
	 * WD 14016 1818001D 00001818 00000000 00001D1D
	 */
	tU8 CMOST_SetGPIOforAudioInput_HD[] = {0x00, 0x1f, 0x09, 0x01, 0x40, 0x16, 0x18, 0x18, 0x00, 0x18, 0x00, 0x1d, 0x00, 0x00, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x1d, 0x1d};

	retval = ETAL_sendCmdCheckResult_CMOST(hTuner, CMOST_SetGPIOforAudioInput_HD, sizeof(CMOST_SetGPIOforAudioInput_HD), NULL, NULL);
#endif 

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS)
	if (retval != OSAL_OK)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "CMOST GPIO configuration for audio");
	}
#endif

	return retval;
}
#endif // CONFIG_MODULE_INDEPENDENT

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/**************************************
 *
 * ETAL_directCmdSetSAI_BB_CMOST
 *
 *************************************/
/*!
 * \brief		Configure the CMOST SAI BB for Master 912 kHz
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner  - the Tuner handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdSetSAI_BB_CMOST(ETAL_HANDLE hTuner)
{
                               //            CID              PAR1              PAR2
	tU8 CMOSTcmd_SetSAI_BB[] = {0x00, 0x05, 0x02, 0x00, 0x00, 0x04, 0x00, 0x00, 0x01};

	return ETAL_sendCmdCheckResult_CMOST(hTuner, CMOSTcmd_SetSAI_BB, sizeof(CMOSTcmd_SetSAI_BB), NULL, NULL);
}

/**************************************
 *
 * ETAL_directCmdChangeBandTune_CMOST
 *
 *************************************/
/*!
 * \brief		Change band and Tune support for HDRadio startup
 * \details		When CMOST is connected to the STA680 it is necessary to issue
 * 				a tune and a change band command very early in the initialization
 * 				procedure otherwise the HW interface does not work properly.
 *
 * 				This function issues a change band to FM followed by a tune to
 * 				an arbitrarily chosen frequency (87500, the lower limit of the FM band).
 * \remark		This function is provided only to support the ETAL startup;
 * 				its use in any other scenario is **DEPRECATED**.
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner  - the Tuner handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_directCmdChangeBandTune_CMOST(ETAL_HANDLE hTuner)
{
                             //            CID              PAR1              PAR2              PAR3              PAR4              PAR5              PAR6
	tU8 CMOST_cmdSetBand[] = {0x00, 0x0a, 0x06, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x55, 0xcc, 0x01, 0xa5, 0xe0, 0x00, 0x00, 0x64};
	tU8 CMOST_cmdTune[] =    {0x00, 0x08, 0x03, 0x00, 0x00, 0x01, 0x01, 0x55, 0xcc, 0x00, 0x00, 0x00}; // 87500, lower frequency of the FM band
	tSInt retval;

	if (ETAL_sendCmdCheckResult_CMOST(hTuner, CMOST_cmdSetBand, sizeof(CMOST_cmdSetBand), NULL, NULL) != OSAL_OK)
	{
		retval = OSAL_ERROR;
	}
	else
	{
		if (ETAL_waitBusyBit_CMOST(hTuner, 0x01, ETAL_CHANGE_BAND_DELAY_CMOST) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_directCmdChangeBandTune_CMOST CMOST_cmdSetBand busy bit error hTuner %d",
				hTuner);
		}
	
		if (ETAL_sendCmdCheckResult_CMOST(hTuner, CMOST_cmdTune, sizeof(CMOST_cmdTune), NULL, NULL) != OSAL_OK)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			if (ETAL_waitBusyBit_CMOST(hTuner, 0x01, ETAL_CHANGE_BAND_DELAY_CMOST) != OSAL_OK)
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_directCmdChangeBandTune_CMOST CMOST_cmdTune busy bit error hTuner %d",
					hTuner);
			}
			
			retval = OSAL_OK;
		}
	}

	return retval;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#endif // CONFIG_ETAL_SUPPORT_DCOP

/**************************************
 *
 * ETAL_CmdTunerConfHDBlend_CMOST
 *
 *************************************/
/*!
 * \brief		Command to configures HD Blend info in CMOST
 * \details		API command Tuner_Conf_HD_Blend (0x15) is used to
 *				configure the HD blender times, analog level, digital level and provide Cd/No information. 
 *				This is required for a correct alignemnt in case of transition HD-FM
 * 				This function issues a change band to FM followed by a tune to
 * 				an arbitrarily chosen frequency (87500, the lower limit of the FM band).
 * \remark		This function is provided only to support the ETAL startup;
 * 				its use in any other scenario is **DEPRECATED**.
 * \remark		All the ETAL_directCmd* functions accept a Tuner handle
 * 				instead of a Receiver; these functions are intended to be
 * 				used only at ETAL system startup or in particular conditions
 * 				when a Receiver handle is not available.
 * \param[in]	hTuner  - the Tuner handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hTuner*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
 
tSInt ETAL_CmdTunerConfHDBlend_CMOST(ETAL_HANDLE hGeneric, tU32 vI_AD_GainStep, tU32 vI_DA_GainStep, tS8 vI_AV_Gain, tS8 vI_DV_Gain, tU8 vI_CdNo)
{
#define ETAL_CMD_CMOST_MIN(a, b) ((((a)<(b))?(a):(b)))
// fractionnal operation (round(2^23 * min(10^(level/20),1-(2^-23)))); 
// dec2hex(floor((1/(45600*0.5))*2^23))
//
// fractionnal value is value reported on 24 bits : 
// (2^23 * min(val,1-(2^-23)))); 
// 
#define ETAL_CMD_CMOST_FS_REF 			45600

#define ETAL_CMD_CMOST_GAIN_STEP_MS_TO_VALUE(x)	 ((tF64)((1/((tF64)ETAL_CMD_CMOST_FS_REF))/(((tF64)(x))/1000)))

#define ETAL_CMD_CMOST_DB_TO_VALUE(x)	(pow(10,(((tF64)(x))/20)))

#define ETAL_CMD_CMOST_MIN_REF			(1-pow(2,(-23)))

#define ETAL_CMD_CMOST_FRACTIONAL_CONVERSION(x)		(pow(2,23) * (x))


                                //            CID               			PAR1             		 PAR2              	PAR3             		 PAR4              		PAR5
	tU8 CMOSTcmdTunerConfHDBlend[] = {0x00, 0x15, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval = OSAL_OK;
    etalReceiverStatusTy *recvp;
	tS32 vI_AD_GainStepFractional, vI_DA_GainStepFractional, vl_AV_GainFractional, vl_DV_GainFractional;
	tF64 vl_AD_GainStepValue, vl_DA_GainStepValue, vl_AV_GainValue, vl_DV_GainValue;
	tF64 vl_tmpMin;
	ETAL_HANDLE hReceiver = ETAL_INVALID_HANDLE;

	/* derive from hReceiver the Tuner channel for PAR1 */
	if (ETAL_handleIsReceiver(hGeneric))
	{
		hReceiver = hGeneric;

	    recvp = ETAL_receiverGet(hReceiver);
		if (recvp == NULL)
		{
			retval = OSAL_ERROR_INVALID_PARAM;
			goto exit;
		}
	}

	
	// Get the AD gain from the step in MS
	vl_AD_GainStepValue = ETAL_CMD_CMOST_GAIN_STEP_MS_TO_VALUE(vI_AD_GainStep);
	vl_tmpMin = ETAL_CMD_CMOST_MIN(vl_AD_GainStepValue, ETAL_CMD_CMOST_MIN_REF);
	vI_AD_GainStepFractional = (tS32) (ETAL_CMD_CMOST_FRACTIONAL_CONVERSION(vl_tmpMin));

	// Get the DA gain from the step in MS	
	vl_DA_GainStepValue = ETAL_CMD_CMOST_GAIN_STEP_MS_TO_VALUE(vI_DA_GainStep);
	vl_tmpMin = ETAL_CMD_CMOST_MIN(vl_DA_GainStepValue, ETAL_CMD_CMOST_MIN_REF);
	vI_DA_GainStepFractional = (tS32) (ETAL_CMD_CMOST_FRACTIONAL_CONVERSION(vl_tmpMin));

	// Get the AV gain from the value in dB
	vl_AV_GainValue = ETAL_CMD_CMOST_DB_TO_VALUE(vI_AV_Gain);
	vl_tmpMin = ETAL_CMD_CMOST_MIN(vl_AV_GainValue, ETAL_CMD_CMOST_MIN_REF);
	vl_AV_GainFractional = (tS32) (ETAL_CMD_CMOST_FRACTIONAL_CONVERSION(vl_tmpMin));

	// Get the DV gain from the value in dB	
	// change the DV GAIN : new adjustement is now :
	// The API command, Tuner_Conf_HD_Blend (0x15) will have the DV_gain parameter redefined to have allow for a gain of +6 dB.
	// DV_gain currently has a decimal range of 0.0 - 1.0 (0x000000 - 0x7FFFFF) and corresponds to a applied gain of -? to 0.0dB.  
	// The new range will be from -? to +6.0dB.  This aligns with the AAA requirements from DTS
	// so the real DV Gain should be minor by 6 as an offset for parsing the data.
	
	vl_DV_GainValue = ETAL_CMD_CMOST_DB_TO_VALUE(vI_DV_Gain - 6);
	vl_tmpMin = ETAL_CMD_CMOST_MIN(vl_DV_GainValue, ETAL_CMD_CMOST_MIN_REF);
	
	vl_DV_GainFractional = (tS32) (ETAL_CMD_CMOST_FRACTIONAL_CONVERSION(vl_tmpMin));

	
	// set the different parameters
    ETAL_paramSetGeneric_CMOST(&CMOSTcmdTunerConfHDBlend[ETAL_CMOST_PARAM1_24BIT], vI_AD_GainStepFractional);

	ETAL_paramSetGeneric_CMOST(&CMOSTcmdTunerConfHDBlend[ETAL_CMOST_PARAM2_24BIT], vI_DA_GainStepFractional); 

	ETAL_paramSetGeneric_CMOST(&CMOSTcmdTunerConfHDBlend[ETAL_CMOST_PARAM3_24BIT], vl_AV_GainFractional);
	
	ETAL_paramSetGeneric_CMOST(&CMOSTcmdTunerConfHDBlend[ETAL_CMOST_PARAM4_24BIT], vl_DV_GainFractional); 

	ETAL_paramSetGeneric_CMOST(&CMOSTcmdTunerConfHDBlend[ETAL_CMOST_PARAM5_24BIT], vI_CdNo); 
	
	retval = ETAL_sendCommandTo_CMOST(hGeneric, CMOSTcmdTunerConfHDBlend, sizeof(CMOSTcmdTunerConfHDBlend), &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
		goto exit;
	}	

exit:	
	return retval; 
}


#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
/***************************
 *
 * ETAL_getParameter_CMOST
 *
 **************************/
/*!
 * \brief		Converts a 3-byte array to a 24-bit integer
 * \details		This function is used to convert a three-byte parameter returned by the CMOST into an integer.
 * \param[in]	cmd - pointer to the first location of a 3-byte array
 * \param[out]	p   - pointer to an integer where the function stores the converted value
 * \callgraph
 * \callergraph
 */
tVoid ETAL_getParameter_CMOST(tU8 *cmd, tU32 *p)
{
	tU32 i = 0;

	*p  = ((tU32)cmd[i++] << 16) & 0x00FF0000;
	*p |= ((tU32)cmd[i++] <<  8) & 0x0000FF00;
	*p |= ((tU32)cmd[i  ] <<  0) & 0x000000FF;
}
#endif

/***************************
 *
 * ETAL_cmdPing_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Ping (0x00) to the CMOST
 * \details		The CMOST API specifies that this command must be sent with
 * 				two different values in PAR1 on consecutive calls. If the
 * 				CMOST is alive it answers with the inverse of the PAR1.
 * 				The function builds a unique PAR1 for each Tuner in the system by
 * 				concatenating the Tuner handle and an alternating value.
 * \param[in]	hReceiver - the Receiver handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_FROM_SLAVE    - unexpected response from the CMOST
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *hReceiver*
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdPing_CMOST(ETAL_HANDLE hReceiver)
{
	tSInt retval;
    ETAL_HANDLE hTuner;

    if (ETAL_receiverGetTunerId(hReceiver, &hTuner) == OSAL_OK)
    {
		retval = ETAL_directCmdPing_CMOST(hTuner);
		
    }
    else
    {
        retval = OSAL_ERROR_INVALID_PARAM;
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdPing_CMOST : error receiver %d\n", hReceiver);	
    }

	return retval;
}

/***************************
 *
 * ETAL_cmdChangeBand_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Change_Band (0x0A) to the CMOST
 * \details		The band limits and step parameters are passed as-is to the CMOST.
 * 				If the selected band is HD the function additionally enables the
 * 				HD filtering and HD blending (bits 4 and 5 of PAR3 of the CMOST command).
 * 
 * 				This command is one of those that involves state machines on the CMOST,
 * 				thus the function checks that the operation is completed by polling (with
 * 				a timeout of #ETAL_CHANGE_BAND_DELAY_CMOST) the Busy bit of the SCSR0
 * 				before returning.
 *
 * 				For FM and HD the function also resets the RDS (see #ETAL_RDSreset_CMOST).
 *
 * 				If all operations complete successfully, the function updates the ETAL
 * 				Receiver internal status with the new band and step limits.
 * \remark		The function does not validate its parameters except for the *hReceiver*.
 * \remark		The function always sends all the parameters, even the optional ones
 * 				(band limits, step) so they should be set to valid values by the caller.
 * \param[in]	hGeneric - the Receiver or Tuner Fontend handle
 * \param[in]	band      - the new band to be applied
 * \param[in]	bandMin   - lower frequency band limit in kHz
 * \param[in]	bandMax   - upper frequency band limit in kHz
 * \param[in]	step      - step in kHz to be used for automatic seek, or #ETAL_SEEK_STEP_UNDEFINED;
 * 				            in the latter case the default step defined for the band is used. 
 * 				            **See Todo section for limitations on the *step* parameter.**
 * \param[in]	processingFeatures - processing features to enable or disable FM VPA,
 * 				                     Antenna diversity, HD filter and HD blending
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *processingFeatures* or *hGeneric*
 * \see			ETAL_waitBusyBit_CMOST
 * \callgraph
 * \callergraph
 * \todo		The *step* parameter is not processed correctly: it is used for the ETAL internal
 * 				status but **not** sent to the STAR, which thus uses always the default step for the band.
 */
tSInt ETAL_cmdChangeBand_CMOST(ETAL_HANDLE hGeneric, EtalFrequencyBand band, tU32 bandMin, tU32 bandMax, tU32 step, EtalProcessingFeatures processingFeatures)
{
	                            //            CID              PAR1              PAR2              PAR3              PAR4              PAR5
	tU8 CMOSTcmd_ChangeBand[] = {(tU8)0x00, (tU8)0x0A, (tU8)0x05, (tU8)0x00, (tU8)0x00, (tU8)0x00, 
		(tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, 
		(tU8)0x00, (tU8)0x00, (tU8)0x00};
	tU16 cstatus;
	tSInt retval;
	ETAL_HANDLE hTuner, hReceiver;
	tU32 tuner_mask;
	etalReceiverStatusTy *recvp;

	/* In some VPA use case a Receiver handle is not available 
	 * then a Fontend handle is used	 */
	if (ETAL_handleIsReceiver(hGeneric))
	{
		hReceiver = hGeneric;
		if (ETAL_receiverGetTunerId(hReceiver, &hTuner) != OSAL_OK)
		{
			retval = OSAL_ERROR_INVALID_PARAM;
			goto exit;
		}
		/* derive from hReceiver the Tuner channel for PAR1 */
		recvp = ETAL_receiverGet(hReceiver);
		if (recvp == NULL)
		{
			retval = OSAL_ERROR_INVALID_PARAM;
			goto exit;
		}
		if (((processingFeatures.u.m_processing_features & (tU8)ETAL_PROCESSING_FEATURE_FM_VPA) != (tU8)0) &&
		    ((recvp->CMOSTConfig.m_DebugVPAMode == ETAL_DEBUG_VPA_MODE_ON) || (recvp->CMOSTConfig.m_DebugVPAMode == ETAL_DEBUG_VPA_MODE_NONE)))
		{
			/* In case VPA as processing flag is selected, the band change needs to be performed on foreground tuner only */
			ETAL_paramSetGeneric_CMOST(&CMOSTcmd_ChangeBand[ETAL_CMOST_PARAM1_24BIT], ETAL_CHN_FOREGROUND);
			tuner_mask = ETAL_CHN_FOREGROUND;
		}
		else
		{
			tuner_mask = ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_ChangeBand, hReceiver);
		}

		// Log that a change band on the receiver is required.
		//
		
		recvp->isTunedRequiredAfterChangeBand = TRUE;
	}
	else if (ETAL_handleIsFrontend(hGeneric))
	{
		hTuner = ETAL_handleFrontendGetTuner(hGeneric);
		hReceiver = hTuner;
		tuner_mask = ETAL_FEHandleToChannel_CMOST(hGeneric);
		ETAL_paramSetGeneric_CMOST(&CMOSTcmd_ChangeBand[ETAL_CMOST_PARAM1_24BIT], tuner_mask);
	}
	else
	{
		retval = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}

	ETAL_paramSetBand_CMOST(CMOSTcmd_ChangeBand, band, bandMin, bandMax);

	/* check processingFeatures parameter */
	if ((processingFeatures.u.m_processing_features & (~(tU8)ETAL_PROCESSING_FEATURE_ALL)) != (tU8)0)
	{
		retval = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}

	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_ChangeBand[ETAL_CMOST_PARAM3_24BIT], (tU32)(processingFeatures.u.m_processing_features));

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_ChangeBand, sizeof(CMOSTcmd_ChangeBand), &cstatus, NULL, NULL);
	if ((retval != OSAL_OK) && (ETAL_handleIsReceiver(hReceiver)))
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
		goto exit;
	}


	if (ETAL_waitBusyBit_CMOST(hTuner, tuner_mask, ETAL_CHANGE_BAND_DELAY_CMOST) != OSAL_OK)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_cmdChangeBand_CMOST busy bit error, hTuner %d, tuner_mask = %d", 
							hTuner, tuner_mask);
	}


	if (ETAL_handleIsReceiver(hReceiver))
	{
		if (ETAL_receiverSetProcessingFeatures(hReceiver, processingFeatures) != ETAL_RET_SUCCESS)
		{
			retval = OSAL_ERROR;
			goto exit;
		}

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
		if (ETAL_receiverSupportsRDS(hReceiver))
		{
			if (ETAL_RDSreset_CMOST(hReceiver, (etalRDSAttr *)NULL) != OSAL_OK)
			{
				retval = OSAL_ERROR;
				goto exit;
			}
		}
#endif

		ETAL_receiverSetBandInfo(hReceiver, band, bandMin, bandMax, step);
	}

		
exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdTune_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Tune (0x08) to the CMOST
 * \details		The function sends the command tune; the 'Injection side' CMOST parameter
 * 				is always set to 'Auto'.
 * 
 * 				This command is one of those that involves state machines on the CMOST,
 * 				thus the function checks that the operation is completed by polling (with
 * 				a timeout of #ETAL_CHANGE_BAND_DELAY_CMOST) the Busy bit of the SCSR0
 * 				before returning.
 * \remark		A Tune should be normally preceded by a Change band to initialize the CMOST band.
 * \remark		**This function does not check if the #ETAL_cmdChangeBand_CMOST was previously called;
 * 				if not the operation will probably fail with an error from the CMOST.**
 * \remark		The function does not validate its parameters except for the *hReceiver*.
 * \remark		Frequency Offset CMOST parameter is not supported.
 * \param[in]	hGeneric   - the Receiver or Frontend handle
 * \param[in]	dwFrequency - the frequency to tune to, in kHz. Must be greater
 * 				              than 0 otherwise the function silently fails
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *hGeneric*
 * \see			ETAL_waitBusyBit_CMOST
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdTune_CMOST(ETAL_HANDLE hGeneric, tU32 dwFrequency)
{
                          //            CID              PAR1              PAR2              PAR3
	tU8 CMOSTcmd_Tune[] = {(tU8)0x00, (tU8)0x08, (tU8)0x03, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00, (tU8)0x00};
	tU16 cstatus;
	tSInt retval;
	ETAL_HANDLE hTuner;
	tU32 tuner_mask;
	etalReceiverStatusTy *recvp;

	if (dwFrequency == 0)
	{
		/*
		 * In at least one case it was seen that tune to frequency 0
		 * while in DAB BAND3 caused the CMOST to respond busy
		 * (20 08 01 00 03 01) for more than the ETAL allowed
		 * timeout (500ms).
		 */
		ASSERT_ON_DEBUGGING(0);
		retval = OSAL_OK;
		goto exit;
	}

	if (ETAL_handleIsReceiver(hGeneric))
	{
		tuner_mask = ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_Tune, hGeneric);

		recvp = ETAL_receiverGet(hGeneric);
		if (recvp == NULL)
		{
			retval = OSAL_ERROR_INVALID_PARAM;
			goto exit;
		}

		// mark that the tune has been done
		// Log that a change band on the receiver is required.
		//
		
		recvp->isTunedRequiredAfterChangeBand = FALSE;
	}
	else if (ETAL_handleIsFrontend(hGeneric))
	{
		hTuner = ETAL_handleFrontendGetTuner(hGeneric);
		tuner_mask = (tU32)ETAL_handleFrontendGetChannel(hGeneric);
		ETAL_paramSetGeneric_CMOST(&CMOSTcmd_Tune[ETAL_CMOST_PARAM1_24BIT], tuner_mask);
	}
	else
	{
		retval = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}
	
	ETAL_paramSetFrequency_CMOST(CMOSTcmd_Tune, dwFrequency);

	/* Injection (parameter 3) set to auto */

	retval = ETAL_sendCommandTo_CMOST(hGeneric, CMOSTcmd_Tune, sizeof(CMOSTcmd_Tune), &cstatus, NULL, NULL);
	if (ETAL_handleIsReceiver(hGeneric))
	{
		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent_CMOST(hGeneric, retval, cstatus, NULL, 0);
			retval = OSAL_ERROR;
			goto exit;
		}

		if (ETAL_receiverGetTunerId(hGeneric, &hTuner) != OSAL_OK)
		{
			// will never hit here, the same condition was checked in ETAL_sendCommandTo_CMOST
			retval = OSAL_ERROR;
			goto exit;
		}
	}
	if (ETAL_waitBusyBit_CMOST(hTuner, tuner_mask, ETAL_CHANGE_BAND_DELAY_CMOST) != OSAL_OK)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_cmdTune_CMOST busy bit error, hTuner %d, tuner_mask = %d", 
							hTuner, tuner_mask);
	}

exit:
	return retval;
}

#if defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_cmdTuneXTAL_CMOST
 *
 **************************/
/*!
 * \brief		Sends a specialized TUNER_Tune (0x08) for use within the #etal_xtal_alignment
 * \details		CMOST tune command used only by the XTAL alignment procedure:
 * 				forces Injection to High side, tunes to frequency 83.9MHz
 * 				and does not wait for the busy bit.
 * \remark		**Do not use outside of the #etal_xtal_alignment function.**
 * \param[in]	hReceiver - the Receiver handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdTuneXTAL_CMOST(ETAL_HANDLE hReceiver)
{
                              //            CID              PAR1              PAR2              PAR3
	tU8 CMOSTcmd_TuneXTAL[] = {0x00, 0x08, 0x03, 0x00, 0x00, 0x00, 0x01, 0x5C, 0xD4, 0x00, 0x00, 0x02};
	tU16 cstatus;
	tSInt retval;

	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_TuneXTAL, hReceiver);
	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_TuneXTAL, sizeof(CMOSTcmd_TuneXTAL), &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdDebugSetBBProc_CMOST
 *
 **************************/
/*!
 * \brief		Sends a DEVICE_SET_BB_PROC (0x30) to the CMOST
 * \remark		This CMOST command is for debug only. It is implemented only to support 
 * 				the #etal_xtal_alignment function.
 * \remark		**Do not use outside of the #etal_xtal_alignment function.**
 * \param[in]	hReceiver - the Receiver handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdDebugSetBBProc_CMOST(ETAL_HANDLE hReceiver)
{
                                    //            CID              PAR1              PAR2
	tU8 CMOSTcmd_DebugSetBBProc[] = {0x00, 0x30, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10};
	tU16 cstatus;
	tSInt retval;

	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_DebugSetBBProc, hReceiver);

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_DebugSetBBProc, sizeof(CMOSTcmd_DebugSetBBProc), &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
	}

	return retval;
}
#endif // CONFIG_ETAL_HAVE_XTAL_ALIGNMENT || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
/**************************************
 *
 * ETAL_cmdSelectAudioSource_CMOST
 *
 *************************************/
/*!
 * \brief		Sends a Tuner_Set_Blend (0x14) to the CMOST
 * \details		Selects the audio input for a STAR device.
 * \param[in]	hTuner - the Tuner handle
 * \param[in]	src    - the audio source selection mode
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *src* parameter
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSelectAudioSource_CMOST(ETAL_HANDLE hTuner, etalStarBlendingModeEnumTy src)
{
                               //            CID              PAR1
	tU8 CMOSTcmd_HDBlender[] = {0x00, 0x14, 0x01, 0x00, 0x00, 0x00}; // PAR1 changed below
	ETAL_HANDLE hReceiver;
	tU16 cstatus;
	tSInt retval = OSAL_OK;

	switch (src)
	{
		case ETAL_STAR_BLENDING_AUTO_HD:
		case ETAL_STAR_BLENDING_STAR_ANALOG_AMFM:
#ifndef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
        case ETAL_STAR_BLENDING_STA680_DIGITAL_AMFMHD:
        case ETAL_STAR_BLENDING_HD_ALIGN:
#endif // !CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
#if !defined(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC) && !defined(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BF) && \
    !defined(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BG) && !defined(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_CA) && \
    !defined(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_DA)
        case ETAL_STAR_BLENDING_STAR_ANALOG_AMFM_EARLY:
#endif
        case ETAL_STAR_BLENDING_STA660_DIGITAL_DABDRM:
			break;
		default:
			ASSERT_ON_DEBUGGING(0);
			// OSAL_ERROR already used for the communication error in which case we also send an event
			// so we need to differentiate
			retval = OSAL_ERROR_INVALID_PARAM;
			goto exit;
	}
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_HDBlender[ETAL_CMOST_PARAM1_24BIT], src);

#if 0
	/*
	 * This check is also performed in ETAL_sendCommandTo_CMOST, it is commented out here
	 * to have an uniform function behavior with the OSAL_ERROR, that is to always send
	 * the error event
	 */
	if (ETAL_tunerIsValidHandle(hTuner) != TRUE)
	{
		retval = OSAL_ERROR;
		goto exit;
	}
#endif
	retval = ETAL_sendCommandTo_CMOST(hTuner, CMOSTcmd_HDBlender, sizeof(CMOSTcmd_HDBlender), &cstatus, NULL, NULL);

	if (retval != OSAL_OK)
	{
		hReceiver = ETAL_INVALID_HANDLE;
		// The error event requires the hReceiver so we derive it from the hTuner
		if ((hReceiver = ETAL_receiverSearchFromTunerId(hTuner, hReceiver)) != ETAL_INVALID_HANDLE)
		{
			ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		}
		retval = OSAL_ERROR;
	}

exit:
	return retval;
}

/**************************************
 *
 * ETAL_cmdSelectAudioInterface_CMOST
 *
 *************************************/
/*!
 * \brief		Sends a Tuner_Set_Audio_IF (0xD) to the CMOST
 * \details		Configures the CMOST audio interface.
 * \remark		The function does not validate the *intf* parameter.
 *        		This static command has to be send before the tuner change band.
 * \param[in]	hTuner - the Tuner handle
 * \param[in]	intf - the requested audio interface operational mode
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *hTuner*
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSelectAudioInterface_CMOST(ETAL_HANDLE hTuner, etalAudioIntfStatusTy intf)
{
                             //            CID              PAR1
	tU8 CMOSTcmd_AudioIF[] = {0x00, 0x0D, 0x01, 0x00, 0x00, 0x00}; // PAR1 changed below
	ETAL_HANDLE hReceiver;
	etalAudioIntfStatusTy current_audio_status;
	tU16 cstatus;
	tSInt retval = OSAL_OK;

	if (ETAL_tunerIsValidHandle(hTuner) == FALSE)
	{
		retval = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}

	current_audio_status = ETAL_statusGetTunerAudioStatus(hTuner);
	
   if (current_audio_status.all == intf.all)
	{
		/*
		 * the interface already has the requested state
		 */
		retval = OSAL_OK;
		goto exit;
	}

	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AudioIF[ETAL_CMOST_PARAM1_24BIT], (tU32)intf.all);

	retval = ETAL_sendCommandTo_CMOST(hTuner, CMOSTcmd_AudioIF, sizeof(CMOSTcmd_AudioIF), &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		hReceiver = ETAL_INVALID_HANDLE;
		// The error event requires the hReceiver so we derive it from the hTuner
		if ((hReceiver = ETAL_receiverSearchFromTunerId(hTuner, hReceiver)) != ETAL_INVALID_HANDLE)
		{
			ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		}
		retval = OSAL_ERROR;
		goto exit;
	}
	ETAL_statusSetTunerAudioStatus(hTuner, intf);

exit:
	return retval;
}

/***************************
 *
 * ETAL_RDSreset_CMOST
 *
 **************************/
/*!
 * \brief		Sends a Tuner_Set_RDSBuffer (0x1A) to the CMOST
 * \details		The function resets and configures the RDS buffering functionality
 * 				of the STAR. *RDSAttr* may be used to specify special modes:
 * 				- rdsMode sets the Permanent Fast PI mode
 * 				- nbRdsBlockforInteruptFastPI sets NumPI
 * 				- nbRdsBlockforInteruptNormalMode sets NQRST
 * 				
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	RDSAttr   - optional pointer to RDS attributes to apply, or NULL to use the default
 * 				            if non-NULL, uses the specified values to configure the CMOST and does not
 * 				            update the Receiver status;
 * 				            if NULL, updates the Receiver status with the values contained in 
 * 				            CMOSTcmd_RDSBufferSet and uses it to configure the CMOST.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 * \todo		The behavior with regards to the *RDSAttr* and the ETAL Receiver status is not very consistent,
 * 				it will reviewed in future release.
 */
tSInt ETAL_RDSreset_CMOST(ETAL_HANDLE hReceiver, etalRDSAttr *RDSAttr)
{
#define ETAL_RDS_DEFAULT_NB_BLOCKS_FAST_PI	((tU8)1)
#define ETAL_RDS_DEFAULT_NB_BLOCKS_NORMAL	((tU8)16)

	                               //            CID              PAR1              PAR2              PAR3
	tU8 CMOSTcmd_RDSBufferSet[] =  {(tU8)0x00, (tU8)0x1A, (tU8)0x03, 
		(tU8)0x00, (tU8)0x00, (tU8)0x00, 
		(tU8)0x00, (tU8)(ETAL_CMOST_CSR_RESET), (tU8)(ETAL_CMOST_CSR_ERRTHRESH_0 | ETAL_CMOST_CSR_ENA), 
		(tU8)0x00, (tU8)0x00, (tU8)0x00}; // all PARs modified below
	tU16 cstatus;
	tSInt retval;
	etalRDSAttrInternal *RDSAttrInt;

	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_RDSBufferSet, hReceiver);



	if (RDSAttr != NULL)
	{
		// modify CSR (PAR2)
		// normal mode only for now
		// TMP : in FAST PI mode & IRQ the TDA7707CA is not working correctly 
		// so do not put fast pi in IRQ
#ifndef CONFIG_COMM_ENABLE_RDS_IRQ		
		if (RDSAttr->rdsMode == ETAL_RDS_MODE_PERMANENT_FAST_PI)
		{
			CMOSTcmd_RDSBufferSet[7] |= ETAL_CMOST_CSR_FORCEFASTPI;
		}
#endif
		if (RDSAttr->groupouten)
		{
			/* set group mode */
			CMOSTcmd_RDSBufferSet[7] |= ETAL_CMOST_CSR_GROUPOUTEN;
		}

		if ((RDSAttr->rdsRbdsMode & ETAL_RBDS_MODE) == ETAL_RBDS_MODE)
		{
			CMOSTcmd_RDSBufferSet[8] |= ETAL_CMOST_CSR_RBDS;
		}

#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
		CMOSTcmd_RDSBufferSet[8] |= ETAL_CMOST_CSR_RDS_IREN;
#endif
		if (RDSAttr->errthresh != ETAL_CMOST_CSR_ERRTHRESH_0)
		{
			/* set errthresh */
			CMOSTcmd_RDSBufferSet[8] &= ~ETAL_CMOST_CSR_ERRTHRESH_MASK;
			CMOSTcmd_RDSBufferSet[8] |= ((RDSAttr->errthresh << 4) & ETAL_CMOST_CSR_ERRTHRESH_MASK);
		}

		// modify NBR (PAR3)
		CMOSTcmd_RDSBufferSet[10] |= (RDSAttr->nbRdsBlockforInteruptFastPI & 0x0F);
		CMOSTcmd_RDSBufferSet[11] = RDSAttr->nbRdsBlockforInteruptNormalMode;
	}
	else
	{
		if (ETAL_receiverGetRDSAttrInt(hReceiver, &RDSAttrInt) == ETAL_RET_SUCCESS)
		{
			/*
			 * Update the Receiver's status:
			 * RDSAttrInt returned by ETAL_receiverGetRDSAttrInt points to an ETAL internal structure
			 */
			RDSAttrInt->rdsAttr.nbRdsBlockforInteruptFastPI = (CMOSTcmd_RDSBufferSet[10] & 0x0F);
			RDSAttrInt->rdsAttr.nbRdsBlockforInteruptNormalMode = CMOSTcmd_RDSBufferSet[11];
			if (((CMOSTcmd_RDSBufferSet[7] & ETAL_CMOST_CSR_FORCEFASTPI) == ETAL_CMOST_CSR_FORCEFASTPI) &&
				(RDSAttrInt->rdsAttr.nbRdsBlockforInteruptFastPI != (tU8)0))
			{
				RDSAttrInt->rdsAttr.rdsMode = ETAL_RDS_MODE_PERMANENT_FAST_PI;
			}
			else if (((CMOSTcmd_RDSBufferSet[7] & ETAL_CMOST_CSR_FORCEFASTPI) == (tU8)0) &&
				(RDSAttrInt->rdsAttr.nbRdsBlockforInteruptFastPI != (tU8)0))
			{
				RDSAttrInt->rdsAttr.rdsMode = ETAL_RDS_MODE_TEMPORARY_FAST_PI;
			}
			else
			{
				RDSAttrInt->rdsAttr.rdsMode = ETAL_RDS_MODE_NORMAL;
			}

#ifndef CONFIG_COMM_ENABLE_RDS_IRQ
			RDSAttrInt->numPICnt = (tU8)0;
#endif
		}

		
		// we assume this is a stop if no attributes
		//
		// disable RDS
		//
		CMOSTcmd_RDSBufferSet[8] = (tU8)0;
		//
		// set some valid default values to nb blocks awake
		// modify NBR (PAR3)
		CMOSTcmd_RDSBufferSet[10] |= (ETAL_RDS_DEFAULT_NB_BLOCKS_FAST_PI & (tU8)0x0F);
		CMOSTcmd_RDSBufferSet[11] = ETAL_RDS_DEFAULT_NB_BLOCKS_NORMAL;
		
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
		CMOSTcmd_RDSBufferSet[8] |= ETAL_CMOST_CSR_RDS_IREN;
#endif

		// set num block as a valid values.
		
	}

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_RDSBufferSet, sizeof(CMOSTcmd_RDSBufferSet), &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
	}
	return retval;
}

/***************************
 *
 * ETAL_cmdSetSeekThreshold_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Set_Seek_Threshold (0x29) to the CMOST
 * \details		This command sets the thresholds to be used for the STAR's internal (or firmware)
 * 				automatic seek algorithm. It is not needed for the external (or software)
 * 				automatic seek algorithm.
 * \remark		The function does not validate its parameters except for the *hReceiver*.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	threshold - Pointer to the thresholds to apply
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSetSeekThreshold_CMOST(ETAL_HANDLE hReceiver, EtalSeekThreshold *threshold)
{
                             //            CID              PAR1              		PAR2              	PAR3				PAR4
	tU8 CMOSTcmd_SeekThr[] = {0x00, 0x29, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval;

	/* set PAR1 */
	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_SeekThr, hReceiver);

	/* set PAR2 and PAR3 and PAR4 */
	ETAL_paramSetSeekThresholds_CMOST(CMOSTcmd_SeekThr, threshold);

	// assumption : no specific handling for old fw versions

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_SeekThr, sizeof(CMOSTcmd_SeekThr), &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdSeekStart_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Seek_Start (0x26) to the CMOST
 * \details		Initiates or continues an automatic or manual seek operation.
 * 				Automatic mode
 *				--------------
 * 				In Automatic mode the STAR jumps to the next frequency in the current band,
 * 				measures the signal quality and if found good stops there, otherwise
 * 				performs another jump. The jump size depends on the *dwAMFMstep* parameter:
 * 				if set to #ETAL_SEEK_STEP_UNDEFINED	the algorithm uses the default step size defined
 * 				in the STAR firmware for the current frequency band, or the custom step defined with the
 * 				#ETAL_cmdChangeBand_CMOST (but see the Todo for limitations).
 * 				If set to a valid value, the function uses it as a seek step.
 * 				Manual mode
 * 				-----------	
 * 				In Manual mode the STAR jumps to the next frequency in the current band
 * 				and stops there, without performing any quality measure nor taking
 * 				any decision on the next step to take. The step size is specified as in the
 * 				Automatic mode.
 * 				Continue mode
 * 				-------------
 *				The STAR API specifies that for Seek commands successive to the first
 *				the command format is different. For this reason the function accepts a cmdContinue
 *				value for the *seekMode* parameter. The caller should use the cmdManualModeStart or
 *				cmdAutomaticModeStart only for the first invocation of this function,
 *				then he should use cmdContinue value.
 *				Algorithm termination
 *				---------------------
 *				To definitively stop the Automatic seek algorithm call the #ETAL_cmdSeekEnd_CMOST
 *				function.
 * 				Notes
 * 				-----
 * 				During the seek operation audio output is muted; when the seek operation
 * 				completes the audio status is defined by the *stayMuted* parameter.
 *
 * 				This command is one of those that involves state machines on the CMOST,
 * 				thus the function checks that the operation is completed by polling (with
 * 				a timeout of #ETAL_CHANGE_BAND_DELAY_CMOST) the Busy bit of the SCSR0
 * 				before returning.
 *
 * 				At the end of the seek, regardless of Automatic vs Manual mode, the function
 * 				inserts a 10ms delay before returning to let the quality parameters inside
 * 				the STAR settle; thus when the function returns it is guaranteed that the quality
 * 				measures are stable.
 * \remark		The STAR command supports specifying an absolute jump: this feature is not
 * 				available in the current implementation of this function.
 * \remark		The STAR command supports defining the number of quality measures to perform
 * 				at the end of the Automatic seek step; this function uses a fixed value
 * 				(1 measure only).
 * \param[in]	hReceiver  - the Receiver handle
 * \param[in]	dir        - defines the seek direction
 * \param[in]	dwAMFMstepOrDiscreteFreq - the seek step; if set to #ETAL_SEEK_STEP_UNDEFINED the algorithm
 * 				             			   uses the default seek step for the current frequency band or the
 * 				            			   custom one defined with the call to #ETAL_cmdChangeBand_CMOST
 * 				            			   (but see the Todo section).
 * 				            			 - Or seek on a discrete frequency (if param seekOnDiscreteFreq set to TRUE)
 * \param[in]	seekMode   - the type of Seek command to use
 * \param[in]	stayMuted  - defines the audio status at the end of the seek step
 * \param[in]	updateStopFrequency  - defines the seek stop frequency
 * \param[in]	seekOnDiscreteFreq  - defines the discrete frequency
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 * \todo		There is no API to modify the default seek step (see Todo in #ETAL_cmdChangeBand_CMOST)
 */
tSInt ETAL_cmdSeekStart_CMOST(ETAL_HANDLE hReceiver, etalSeekDirectionTy dir, tU32 dwAMFMstepOrDiscreteFreq, etalSeekModeTy seekMode, etalSeekAudioTy stayMuted, tBool updateStopFrequency, tBool seekOnDiscreteFreq)
{

                                 //            CID              PAR1              PAR2              PAR3
	tU8 CMOSTcmd_SeekStart[] =   {0x00, 0x26, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU32 extra_len = 0;
	tU16 cstatus;
	tSInt retval;
	ETAL_HANDLE hTuner;
	tU32 tuner_mask;
#ifndef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
	tU32 param;
#endif

	/* start seek */
	// put the channel (from receiver FG or BG)
	tuner_mask = ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_SeekStart, hReceiver);


	// This is used either for a START seek procedure, either to continue the seek procedure (if the STAR is already in seek mode)
	switch(seekMode)
	{
		case cmdContinue:
			// this is the request to continue the seek.
			// PAR2 and PAR3 have to be sent only for the first seeks step of a new seek cycle. Consecutive seek steps are launched by sending Seek_Start command only with PAR1 using the previous sent configuration.
			// so only PAR1 is set
			// len is one parameter
			// extra len is the number of parameter to be removed from max possible len
			// 2 params to be removed, 3 bytes each ==> 6
			extra_len = 6;
			ETAL_paramSeekSetLength_CMOST(CMOSTcmd_SeekStart, ETAL_CMDSEEKSTART_CMOST_CONTINUE_NB_PARAM);
			
			break;

		case cmdManualModeStart:
		case cmdAutomaticModeStart:
			// for seek start (auto or manual) the PAR2 must be set, PAR3 may also be set as an option
		
			// put the PAR2
			ETAL_paramSeekSetMode_CMOST(CMOSTcmd_SeekStart, dir, stayMuted, seekMode);

			// check if PAR3 (step or discrete frequency) is required)
			if ((dwAMFMstepOrDiscreteFreq != ETAL_SEEK_STEP_UNDEFINED) || (dwAMFMstepOrDiscreteFreq != ETAL_INVALID_FREQUENCY))
			{
				extra_len = 0;
				ETAL_paramSetGeneric_CMOST(&CMOSTcmd_SeekStart[ETAL_CMOST_PARAM3_24BIT], dwAMFMstepOrDiscreteFreq);
#ifndef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
				ETAL_getParameter_CMOST(&CMOSTcmd_SeekStart[ETAL_CMOST_PARAM2_24BIT], &param);

                /* Seek step is applied */
                param &= 0xFFF7;

                if (seekOnDiscreteFreq == TRUE)
                {
                    /* Seek on a discrete frequency */
                    param |= 0x0008;
                }

                /* One measurement for seek start */
                param |= 0x0010;

                /* Seek stop frequency is not updated */
                if(updateStopFrequency == FALSE)
                {
                    param |= 0x0100;
                }

                ETAL_paramSetGeneric_CMOST(&CMOSTcmd_SeekStart[ETAL_CMOST_PARAM2_24BIT], param);
#endif
			}
			else
			{
				/* don't send PAR3 if step is not specified */
				extra_len = 3;
				ETAL_paramSeekSetLength_CMOST(CMOSTcmd_SeekStart, ETAL_CMDSEEKSTART_CMOST_START_NB_PARAM_WO_STEP);
			}

			
			break;
		default:
			// no default
			break;
	}	

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_SeekStart, sizeof(CMOSTcmd_SeekStart) - extra_len, &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
		goto exit;
	}

	if (ETAL_receiverGetTunerId(hReceiver, &hTuner) != OSAL_OK)
	{
		retval = OSAL_ERROR;
		goto exit;
	}

	// in automatic mode, the busy bit should not be checked
	// as per CMOST/API :  TUNER_Seek_Start (0x26)
	// In automatic mode the corresponding 'Busy' and 'seek mode' flag in SCSR0 remains set 
	// until a 'good station' is found and the unmute and WSP convergence has finished (either triggered by automatic unmuting or sending a Seek_End command). 
	// See also section: 3.1 System Status Controller.
	// in our case : the seek ending is monitor by polling
	// so do not monitor the busy in that case.
	// 
	// so check busy only for manual case
	// ie if mode is manual or for continue case, if manual seek is in progress.
	//
	
	if ((cmdManualModeStart == seekMode) || (TRUE == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialManualSeek)))
	{
		if (ETAL_waitBusyBit_CMOST(hTuner, tuner_mask, ETAL_SEEK_START_DELAY_CMOST) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_cmdSeekStart_CMOST busy bit error, hTuner %d, tuner_mask = %d", 
							hTuner, tuner_mask);
		}
	}
	
	/* Time to wait to be sure the CMOST command has been handled by CMOST */
	(void)OSAL_s32ThreadWait(ETAL_CMOST_SEEK_START_MINIMUM_TIME);

	retval = OSAL_OK;

exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdSeekEnd_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Seek_End (0x27) to the CMOST
 * \details		This function should be called to terminate an Automatic Seek procedure
 * 				started with the #ETAL_cmdSeekStart_CMOST for the internal (or firmware)
 * 				implementation. After the call to this function it is allowed to call again
 * 				the #ETAL_cmdSeekStart_CMOST function with *seekMode* set to
 * 				cmdManualModeStart or cmdAutomaticModeStart.
 *
 * 				This function inserts a 10ms delay before returning to let the quality parameters inside
 * 				the STAR settle; thus when the function returns it is guaranteed that the quality
 * 				measures are stable.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	audio     - defines the audio status at the end of the seek
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSeekEnd_CMOST(ETAL_HANDLE hReceiver, etalSeekAudioTy audio)
{
                             //            CID              PAR1              PAR2
	tU8 CMOSTcmd_SeekEnd[] = {0x00, 0x27, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval;

	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_SeekEnd, hReceiver);
	
	if (audio == cmdAudioMuted)
	{
		ETAL_paramSetGeneric_CMOST(&CMOSTcmd_SeekEnd[ETAL_CMOST_PARAM2_24BIT], (tU32)1);
	}
	
	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_SeekEnd, sizeof(CMOSTcmd_SeekEnd), &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
		goto exit;
	}

    /* Time to wait to be sure the CMOST command has been handled by CMOST */
	(void)OSAL_s32ThreadWait(ETAL_CMOST_SEEK_STOP_MINIMUM_TIME);

	retval = OSAL_OK;
	
exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdSeekGetStatus_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Get_Seek_Status (0x28) to the CMOST
 * \details		Requests the current Seek status including the Seek quality indicators.
 * \remark		for DOT devices *pO_seekStoppedOnGoodFrequency* and *quality* information
 * 				is not available thus these parameters should be set to NULL
 * 				or the returned value ignored.
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	pO_seekStoppedOnGoodFrequency - pointer to a location where the function stores the
 * 				                                indication if seek cycle is ended.
 * 				                                True if seek ended on good station, False if seek did not stopped
 * 				                                If NULL the parameter is ignored.
 * \param[out]	pO_fullCycleReached  - pointer to a location where the function stores the 
 * 				                       indication if a full cycle has been reached:
 * 				                       True if full cycle reached, False if not.
 *	                                   If NULL the parameter is ignored.
 * \param[out]	pO_bandBorderCrossed - pointer to a location where the function stores the
 * 				                       indication if a band border has been crossed:
 * 				                       True if crossed, False if not.
 *	                                   If NULL the parameter is ignored.
 * \param[out]	freq - pointer to a location where the function stores the current seek frequency, in Hz.
 *                     If NULL the parameter is ignored.
 * \param[out]	quality - pointer to a #EtalBcastQualityContainer where the function stores the decompressed quality info.
 *                        If NULL the parameter is ignored.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSeekGetStatus_CMOST(ETAL_HANDLE hReceiver, tBool *pO_seekStoppedOnGoodFrequency, tBool *pO_fullCycleReached, tBool *pO_bandBorderCrossed, tU32 *freq, EtalBcastQualityContainer *quality)
{
	// from STAR interface, the response buffer contains 
	/*
		Parameter 1:	Name	Current Seek RF frequency
						Bit[23] 0x0: No seek stop
								0x1: Station found with quality above thresholds (good station)
						Bit[22] 0x0: Full seek cycle not reached
								0x1: Full seek cycle reached
						Bit[21] 0x0: Band border not crossed in last seek cycle
								0x1: Band border crossed in last seek cycle
						Bit[20:0]	Current Seek RF frequency
		Parameter 2:	Name	Compressed quality register  fstRF_fstBB_det
		Parameter 3:	Name	Compressed quality register mp_adj_dev
	*/
                                   //            CID              PAR1
	tU8 CMOSTcmd_SeekGetStatus[] = {0x00, 0x28, 0x01, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval;
	tU8 resp[CMOST_MAX_RESPONSE_LEN];
	tU32 resp_len;
	tU32  status;
	EtalBcastStandard standard;
	tBool good_freq, full_cycle, band_cross;
	tU32 f;

#ifdef DEBUG_SEEK
	EtalFmQualityEntries *fmQualityEntries;
#endif //DEBUG_SEEK

	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_SeekGetStatus, hReceiver);

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_SeekGetStatus, sizeof(CMOSTcmd_SeekGetStatus), &cstatus, resp, &resp_len);

	if ((retval != OSAL_OK) || (resp_len < 1))
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, resp, resp_len);
		retval = OSAL_ERROR;
		goto exit;
	}

	/* Get results */
	ETAL_getParameter_CMOST(&resp[0], &status);

	standard = ETAL_receiverGetStandard(hReceiver);

	if (quality != NULL)
	{
		ETAL_resetQualityContainer(ETAL_receiverGetStandard(hReceiver), quality);

		quality->m_standard = standard;
		quality->m_TimeStamp = OSAL_ClockGetElapsedTime();
		/* the following is not available on DOT */
		ETAL_extractAmFmQualityFromNotification_CMOST(channelQuality, quality, standard, &resp[ETAL_CMD_SEEK_GET_STATUS_ANSWER_QUALITY_INDEX]);
	}

	// now expand the result to the response
	good_freq = (((status & ETAL_CMOST_SEEKSTATUS_GOODSIGNAL) != 0) ? true : false); /* not available for DOT */
	if (pO_seekStoppedOnGoodFrequency != NULL)
	{
		*pO_seekStoppedOnGoodFrequency = good_freq;
	}

	full_cycle = (((status & ETAL_CMOST_SEEKSTATUS_FULLCYCLE) != 0) ? true : false);
	if (pO_fullCycleReached != NULL)
	{
		*pO_fullCycleReached = full_cycle;
	}

	band_cross = (((status & ETAL_CMOST_SEEKSTATUS_WRAPPED) != 0) ? true : false);
	if (pO_bandBorderCrossed != NULL)
	{
		*pO_bandBorderCrossed = band_cross;
	}

	f = status & ETAL_CMOST_SEEKSTATUS_FREQMASK;
	if (freq != NULL)
	{
		*freq = f;

	}
#ifdef DEBUG_SEEK
	if (quality != NULL)
	{
		if (ETAL_IS_HDRADIO_STANDARD(standard))
		{
			fmQualityEntries = &(quality->EtalQualityEntries.hd.m_analogQualityEntries);
		}
		else
		{
			fmQualityEntries = &(quality->EtalQualityEntries.amfm);
		}

		(tVoid)fmQualityEntries;
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_CMD, "GetSeekStatus: rec: %d, RFFS %ld, BBFS %ld, Det %lu, ADJ %ld, MPX %lu, MP %lu, SNR %lu, coch %lu, Good freq %d, Cycle reached %d, Band crossed %d, freq %d",
		        hReceiver,
		        fmQualityEntries->m_RFFieldStrength,
				fmQualityEntries->m_BBFieldStrength,
				fmQualityEntries->m_FrequencyOffset,
				fmQualityEntries->m_AdjacentChannel,
				fmQualityEntries->m_UltrasonicNoise,
				fmQualityEntries->m_Multipath,
				fmQualityEntries->m_SNR,
				fmQualityEntries->m_coChannel,
				good_freq,
				full_cycle,
				band_cross,
				f);
	}
#endif //DEBUG_SEEK

	retval = OSAL_OK;
	
exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdGetReceptionQuality_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Get_Reception_Quality (0x12) to the CMOST
 * \details		Requests the current Reception Quality indicators.
 * \remark		Quality information is not available immediately after a Tune command:
 * 				the caller of this function should ensure at least 5ms delay after the
 * 				#ETAL_cmdTune_CMOST returns before invoking this function.
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	p - pointer to a #EtalBcastQualityContainer variable where the function
 * 				    stores the quality.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetReceptionQuality_CMOST(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p)
{
                                         //            CID              PAR1
	tU8 CMOSTcmd_getReceptionQuality[] = {0x00, 0x12, 0x01, 0x00, 0x00, 0x00};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 resp[CMOST_MAX_RESPONSE_LEN];
	EtalBcastStandard std = ETAL_receiverGetStandard(hReceiver);
	OSAL_tMSecond timestamp;
	tSInt retval = OSAL_OK;

	if(std == ETAL_BCAST_STD_DAB)
	{
		retval = OSAL_OK;
		goto exit;
	}

	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_getReceptionQuality, hReceiver);
	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_getReceptionQuality, sizeof(CMOSTcmd_getReceptionQuality), &cstatus, resp, &resplen);
	
	timestamp = OSAL_ClockGetElapsedTime();
	if ((retval == OSAL_OK) && (resplen >= ETAL_CMOST_GET_AMFMMON_RESP_LEN))
	{
		p->m_TimeStamp = timestamp;
		p->m_standard = std;

		ETAL_extractAmFmQualityFromNotification_CMOST(receptionQuality, p, std, resp);
	}
	else
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, resp, resplen);
		retval = OSAL_ERROR;
	}

exit:
	return retval;
}


/***************************
 *
 * ETAL_cmdGetChannelQuality_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Get_Channel_Quality (0x13) to the CMOST
 * \details		Requests the current field quality information (derived directly after DISS) of foreground and background.
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	p - pointer to a #EtalBcastQualityContainer variable where the function
 * 				    stores the quality (In VPA the quality of both channels will be provided.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetChannelQuality_CMOST(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p)
{
	                                     //            CID              PAR1
	tU8 CMOSTcmd_getReceptionQuality[] = {0x00, 0x13, 0x01, 0x00, 0x00, 0x00};
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 resp[CMOST_MAX_RESPONSE_LEN];
	EtalBcastStandard std = ETAL_receiverGetStandard(hReceiver);
	OSAL_tMSecond timestamp;
	tSInt retval = OSAL_OK;
	etalReceiverStatusTy *recvp;

	if (std == ETAL_BCAST_STD_DRM)
	{
		retval = OSAL_ERROR_NOT_IMPLEMENTED;
		goto exit;
	}

	recvp = ETAL_receiverGet(hReceiver);
	if ((recvp != NULL) && ((recvp->CMOSTConfig.processingFeatures.u.m_processing_features & (tU8)ETAL_PROCESSING_FEATURE_FM_VPA) != (tU8)0) &&
		((recvp->CMOSTConfig.m_DebugVPAMode == ETAL_DEBUG_VPA_MODE_ON) || (recvp->CMOSTConfig.m_DebugVPAMode == ETAL_DEBUG_VPA_MODE_NONE)))
	{
		/* In FM VPA mode, the quality of both channel will be provided (no parameter is needed) */
		CMOSTcmd_getReceptionQuality[ETAL_CMOST_HEADER_24BIT + 2] = (tU8)0;
	}
	else
	{
		/* set parameter1 in single tuner mode only */
		(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_getReceptionQuality, hReceiver);
	}
	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_getReceptionQuality, ((tU32)CMOSTcmd_getReceptionQuality[ETAL_CMOST_HEADER_24BIT + 2] * 3) + 3, &cstatus, resp, &resplen);

	timestamp = OSAL_ClockGetElapsedTime();
	if ((retval == OSAL_OK) && (resplen >= ETAL_CMOST_GET_AMFMMON_RESP_LEN))
	{
		p[0].m_TimeStamp = timestamp;
		p[0].m_standard = std;
		switch (std)
		{
			case ETAL_BCAST_STD_FM:
			case ETAL_BCAST_STD_AM:
			case ETAL_BCAST_STD_HD_FM:
			case ETAL_BCAST_STD_HD_AM:
			case ETAL_BCAST_STD_DAB:
				ETAL_extractAmFmQualityFromNotification_CMOST(channelQuality, &(p[0]), std, resp);
				break;

			default:
				retval = OSAL_ERROR;
				goto exit;
		}

		if (resplen >= ETAL_CMOST_GET_AMFMMON_VPA_ON_RESP_LEN)
		{
			p[1].m_TimeStamp = timestamp;
			p[1].m_standard = std;
			switch (std)
			{
				case ETAL_BCAST_STD_FM:
				case ETAL_BCAST_STD_AM:
				case ETAL_BCAST_STD_HD_FM:
				case ETAL_BCAST_STD_HD_AM:
				case ETAL_BCAST_STD_DAB:
					ETAL_extractAmFmQualityFromNotification_CMOST(channelQuality, &(p[1]), std, &(resp[ETAL_CMOST_GET_AMFMMON_RESP_LEN - 3]));
					break;

				default:
					retval = OSAL_ERROR;
					goto exit;
			}
		}
	}
	else
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, resp, resplen);
		retval = OSAL_ERROR;
	}

exit:
	return retval;
}


/***************************
 *
 * ETAL_cmdGetRDS_CMOST
 *
 **************************/
/*!
 * \brief		Sends a Tuner_Read_RDSBuffer (0x1B) to the CMOST
 * \details		This function reads and resets the current RDS buffer.
 * 				The data stored to the *resp* buffer is composed of:
 * 				- first three bytes are the Read Notification Register
 * 				- subsequent bytes, in multiple of three, are the RDS data words.
 *
 * 				Each RDS data word is composed of three bytes: the MSB is the
 * 				status and contains the ERRCOUNT, CTYPE, BLOCKID indicators;
 * 				the remaining two bytes are the RDS data word. For details see
 * 				TDA770X_STAR_API_IF.pdf, v2.35, section 7.3.
 * \remark		The buffer pointed by *resp* should be capable of holding the
 * 				maximum amount of data that can be returned by the STAR
 * 				(#CMOST_PARAMETER_LEN bytes). The function does not check if this limit
 * 				is respected and just writes the bytes it receives starting from *resp*.
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	resp      - pointer to a buffer where the function stores the STAR response
 * \param[out]	buflen    - pointer to an integer where the function stores the number of
 * 				            bytes written to *resp*.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_INVALID_PARAM - one of the [out] parameters is NULL, or wrong number
 * 				                           of channels in the *hReceiver*.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetRDS_CMOST(ETAL_HANDLE hReceiver, tU8 *resp, tU32 *buflen)
{
                                   //            CID              PAR1
#if defined (CONFIG_COMM_ENABLE_RDS_IRQ)
    tU8 CMOSTcmd_RDSBufferRead[] = {(tU8)0x00, (tU8)0x1B, (tU8)0x00};
#else
	tU8 CMOSTcmd_RDSBufferRead[] = {(tU8)0x00, (tU8)0x1B, (tU8)0x01, (tU8)0x00, (tU8)0x00, (tU8)0x01}; // PAR1 changed below
#endif
	tU16 cstatus;
	tSInt retval;
	tU32 rlen;

	if ((resp == NULL) || (buflen == NULL))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}

#if !defined (CONFIG_COMM_ENABLE_RDS_IRQ)
	ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_RDSBufferRead, hReceiver);
	/* both channels not supported by this command */
	if (CMOSTcmd_RDSBufferRead[ETAL_CMOST_PARAM1_24BIT + 2] == (tU8)ETAL_CHN_BOTH)
	{
		retval = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}
#endif

	/* Read the current data */
	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_RDSBufferRead, (tU32)CMOSTcmd_RDSBufferRead[ETAL_CMOST_HEADER_24BIT + 2] * 3 + 3, &cstatus, resp, &rlen);
	if (retval != OSAL_OK)
	{
#if !defined (CONFIG_COMM_ENABLE_RDS_IRQ)
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, resp, rlen);
#endif
		retval = OSAL_ERROR;
		goto exit;
	}
	*buflen = rlen;
	retval = OSAL_OK;

exit:
	return retval;
}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

/**************************************
 *
 * ETAL_cmdSetTunerStandby_CMOST
 *
 *************************************/
/*!
 * \brief		Sends a TUNER_Change_Band (0x0A) to the CMOST to put the CMOST in standby
 * \details		Configures the CMOST in standby
 * \param[in]	vI_hTuner - the Tuner handle
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *hTuner*
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSetTunerStandby_CMOST(ETAL_HANDLE vI_hTuner)
{
	tU8 CMOSTcmd_ChangeBand[] = {0x00, 0x0A, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval;
	ETAL_HANDLE vl_hReceiver;

	// both channels
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_ChangeBand[ETAL_CMOST_PARAM1_24BIT], 0x03);

	// set to Standby
	// set min-max-step to 0
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_ChangeBand[ETAL_CMOST_PARAM2_24BIT], 0x00);

	retval = ETAL_sendCommandTo_CMOST(vI_hTuner, CMOSTcmd_ChangeBand, sizeof(CMOSTcmd_ChangeBand), &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		vl_hReceiver = ETAL_INVALID_HANDLE;
		// The error event requires the hReceiver so we derive it from the hTuner
		if ((vl_hReceiver = ETAL_receiverSearchFromTunerId(vI_hTuner, vl_hReceiver)) != ETAL_INVALID_HANDLE)
		{
			ETAL_sendCommunicationErrorEvent_CMOST(vI_hTuner, retval, cstatus, NULL, 0);
		}
		retval = OSAL_ERROR;
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdWrite_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Write (0x1F) to the CMOST
 * \details		Writes to the CMOST's memory, starting from *address*, the *length*
 * 				24-bit words of data passed in *value*.
 * \remark		This functions performs a dynamic memory allocation.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	address   - the first CMOST memory address location to write to.
 * \param[out]	value     - address of the buffer containing *length* 32-bit words. For each 
 * 				            word only the 3 Least Significant Bytes are used.
 * \param[in]	length    - the number of 24-bit words to write to the CMOST memory;
 * 				            must be greater than 0 and smaller or equal to #MAX_NB_OF_WORD_TO_BE_READ.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error, also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR - dynamic allocation error, also sends an #ETAL_WARNING_OUT_OF_MEMORY event
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *length*.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdWrite_CMOST(ETAL_HANDLE hReceiver, tU32 address, tU32 *value, tU8 length)
{
	tU16 cstatus;
	tSInt retval = OSAL_OK;
	tU8 *CMOSTcmd_WriteMem1Loc;
	tU32 i, cmdLength;
	tU32 l_length = (tU32)length; /* avoid some casts */

	if((l_length == 0) || (l_length > MAX_NB_OF_WORD_TO_BE_READ))
	{
		retval = OSAL_ERROR_INVALID_PARAM;
	}
	else
	{
		cmdLength = 3 * (1 + 1 + l_length); // header + address + length values

		CMOSTcmd_WriteMem1Loc = (tU8 *)OSAL_pvMemoryCAllocate(cmdLength, sizeof(tU8));
		if (CMOSTcmd_WriteMem1Loc == NULL)
		{
			ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, (tVoid *)((tULong)hReceiver),  sizeof(ETAL_HANDLE));
			retval = OSAL_ERROR;
		}
		else
		{
			ETAL_paramSetGeneric_CMOST(&CMOSTcmd_WriteMem1Loc[ETAL_CMOST_HEADER_24BIT], 0x00001F00 + (1 + l_length));
			ETAL_paramSetGeneric_CMOST(&CMOSTcmd_WriteMem1Loc[ETAL_CMOST_PARAM1_24BIT], address);

			for(i = 0; i < l_length; i++)
			{
				ETAL_paramSetGeneric_CMOST(&CMOSTcmd_WriteMem1Loc[ETAL_CMOST_PARAM2_24BIT + 3*i], value[i]);
			}

			retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_WriteMem1Loc, cmdLength, &cstatus, NULL, NULL);

			if (retval != OSAL_OK)
			{
				ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
				retval = OSAL_ERROR;
			}
			OSAL_vMemoryFree((tVoid *)CMOSTcmd_WriteMem1Loc);
		}
	}
	return retval;
}

/***************************
 *
 * ETAL_cmdRead_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Read (0x1E) to the CMOST
 * \details		Reads from the CMOST's memory space *length* 24-bit words, starting
 * 				from *address*.
 * \remark		*response* must be large enough to store the number of requested 24-bit words.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	address   - the first address of the CMOST memory space to read from
 * \param[in]	length    - number of 24-bit words to read
 * \param[out]	response  - address of a buffer where the function stores the read values
 * \param[out]	responseLength - address of an integer where the function stores the number of bytes
 * 				                 written to *response*.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *length*.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdRead_CMOST(ETAL_HANDLE hReceiver, tU32 address, tU8 length, tU8 *response, tU32 *responseLength)
{
                                 //            CID              PAR1              PAR2
	tU8 CMOSTcmd_ReadMem1Loc[] = {0x00, 0x1E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval = OSAL_OK;

	if((length == (tU8)0) || (length > (tU8)MAX_NB_OF_WORD_TO_BE_READ))
	{
		retval = OSAL_ERROR_INVALID_PARAM;
	}
	else
	{
		ETAL_paramSetGeneric_CMOST(&CMOSTcmd_ReadMem1Loc[ETAL_CMOST_PARAM1_24BIT], address);
		ETAL_paramSetGeneric_CMOST(&CMOSTcmd_ReadMem1Loc[ETAL_CMOST_PARAM2_24BIT], (tU32)length);

		retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_ReadMem1Loc, sizeof(CMOSTcmd_ReadMem1Loc), &cstatus, response, responseLength);

		if (retval != OSAL_OK)
		{
			ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, response, *responseLength);
			retval = OSAL_ERROR;
		}
	}
	return retval;
}


/***************************
 *
 * ETAL_cmdAudioMute_CMOST
 *
 **************************/
/*!
 * \brief		Sends a Tuner_Audio_Mute (0x16) to the CMOST
 * \remark		The function does not validate the *muteAction* parameter.
 * \param[in]	hReceiver  - the Receiver handle
 * \param[in]	muteAction - 0 to mute Audio, 1 to unmute, 3 to unmute all (audio and tuner)
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdAudioMute_CMOST(ETAL_HANDLE hReceiver, tU8 muteAction)
{
                               //            CID              PAR1
	tU8 CMOSTcmd_AudioMute[] = {0x00, 0x16, 0x01, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval = OSAL_OK;

	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AudioMute[ETAL_CMOST_PARAM1_24BIT], (tU32)muteAction);

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_AudioMute, sizeof(CMOSTcmd_AudioMute), &cstatus, NULL, NULL);

	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
	}
	return retval;
}

#if defined (CONFIG_ETAL_HAVE_AUDIO_CONTROL) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_cmdFMStereoMode_CMOST
 *
 **************************/
/*!
 * \brief		Sends a Tuner_FM_Stereo_Mode (0x17) to the CMOST
 * \remark		The function does not validate the *fmStereoMode* parameter.
 * \param[in]	hReceiver    - the Receiver handle
 * \param[in]	fmStereoMode - 0 for automatic selection, 1 to force mono, 2 to force stereo.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdFMStereoMode_CMOST(ETAL_HANDLE hReceiver, tU8 fmStereoMode)
{
                                  //            CID              PAR1
	tU8 CMOSTcmd_FMStereoMode[] = {0x00, 0x17, 0x01, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval = OSAL_OK;

	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_FMStereoMode[ETAL_CMOST_PARAM1_24BIT], (tU32)fmStereoMode);

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_FMStereoMode, sizeof(CMOSTcmd_FMStereoMode), &cstatus, NULL, NULL);

	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
	}
	return retval;
}
#endif // CONFIG_ETAL_HAVE_AUDIO_CONTROL || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_cmdAFCheck_CMOST
 *
 **************************/
/*!
 * \brief		Sends a Tuner_AF_Check (0x22) to the CMOST
 * \details		Performs a fast quality check measurement on the *alternateFrequency*
 * 				by briefly tuning to that frequency and then returning to the original one.
 * 				
 * 				The function returns the quality measured on the *alternateFrequency*
 * 				in the **previous** call to this function, thus the quality returned
 * 				after the first call to this function should be discarded.
 *
 * 				This command is one of those that involves state machines on the CMOST
 * 				thus the function checks that the operation is completed by polling (with
 * 				a timeout of #ETAL_AF_CHECK_DELAY_CMOST) the Busy bit of the SCSR0.
 *
 * 				Additionally the function waits #ETAL_CMOST_AF_CHECK_MINIMUM_TIME
 * 				before returning to ensure the quality measure for the next measurement
 * 				will be stable.
 * \remark		The function does not validate its parameters except the *hReceiver*.
 * \param[in]	hReceiver          - the Receiver handle
 * \param[in]	alternateFrequency - the frequency to tune to, in kHz
 * \param[in]	antennaSelection   - antenna selection: 0 for automatic selection,
 * 				                     1 for FM1 antenna, 2 for FM2 antenna
 * \param[out]	p - address of a location containing an EtalBcastQualityContainer where
 * 				    the function stores the measured quality. If NULL the quality is not
 * 				    returned.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_NOT_SUPPORTED - *hReceiver* is not configured for FM nor DAB
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdAFCheck_CMOST(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p)
{
                             //            CID              PAR1              PAR2              PAR3
	tU8 CMOSTcmd_AFCheck[] = {0x00, 0x22, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus = 0;
	tSInt retval = OSAL_OK;
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE hTuner;
    tU8 resp[CMOST_MAX_RESPONSE_LEN];
    tU32 resp_len;
    EtalBcastStandard standard;

	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_AFCheck, hReceiver);
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AFCheck[ETAL_CMOST_PARAM2_24BIT], alternateFrequency);
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AFCheck[ETAL_CMOST_PARAM3_24BIT], antennaSelection);

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_AFCheck, sizeof(CMOSTcmd_AFCheck), &cstatus, resp, &resp_len);

	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		retval = OSAL_ERROR;
		goto exit;
	}

	if ((p != NULL) && (resp_len >= ETAL_CMOST_GET_AFQUALITY_RESP_LEN))
	{
    	standard = ETAL_receiverGetStandard(hReceiver);
		(void)OSAL_pvMemorySet((tVoid *)p, 0x00, sizeof(EtalBcastQualityContainer));

		switch(standard)
		{
			case ETAL_BCAST_STD_FM:
			case ETAL_BCAST_STD_HD_FM:
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
				p->m_TimeStamp = OSAL_ClockGetElapsedTime();
				p->m_standard = standard;
				ETAL_extractAmFmQualityFromNotification_CMOST(channelQualityForAF, p, standard, &resp[ETAL_CMD_AF_CHECK_ANSWER_QUALITY_INDEX]);
#else
				retval = OSAL_ERROR_NOT_SUPPORTED;
#endif
				break;

			case ETAL_BCAST_STD_DAB:
				// For DAB band, quality register contains all 0, so no need to extract results.
				break;

			default:
				retval = OSAL_ERROR_NOT_SUPPORTED;
				break;
		}
	}

	/* An error returned by ETAL_receiverGetTunerId is not a communication error
	 * so don't send an event in that case */
	if (ETAL_receiverGetTunerId(hReceiver, &hTuner) == OSAL_OK)
	{
		recvp = ETAL_receiverGet(hReceiver);
		if (recvp == NULL)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			if (ETAL_waitBusyBit_CMOST(hTuner, recvp->CMOSTConfig.channel, ETAL_AF_CHECK_DELAY_CMOST) != OSAL_OK)
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_cmdAFCheck_CMOST busy bit error, hTuner %d, tuner_mask = %d", 
									hTuner, recvp->CMOSTConfig.channel);
			}

		    /* Time to wait to be sure the quality measure on the CMOST stabilizes for the next
			 * invocation of this function */
		    (void)OSAL_s32ThreadWait(ETAL_CMOST_AF_CHECK_MINIMUM_TIME);
		}
	}

exit:	
	return retval;
}

/***************************
 *
 * ETAL_cmdAFSwitch_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_AF_Switch (0x23) to the CMOST
 * \remark		The function does not validate its parameters except the *hReceiver*.
 * \param[in]	hReceiver          - the Receiver handle
 * \param[in]	alternateFrequency - The frequency to tune to, in kHz
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdAFSwitch_CMOST(ETAL_HANDLE hReceiver, tU32 alternateFrequency)
{
                              //            CID              PAR1              PAR2
	tU8 CMOSTcmd_AFSwitch[] = {0x00, 0x23, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus = 0;
	tSInt retval = OSAL_OK;

	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_AFSwitch, hReceiver);
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AFSwitch[ETAL_CMOST_PARAM2_24BIT], alternateFrequency);

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_AFSwitch, sizeof(CMOSTcmd_AFSwitch), &cstatus, NULL, NULL);

	if (retval == OSAL_OK)
	{
	    /* Time to wait to be sure the quality measure on the CMOST stabilizes for the next
		 * invocation of this function */
	    (void)OSAL_s32ThreadWait(ETAL_CMOST_AF_SWITCH_MINIMUM_TIME);
	}
	else
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
        retval = OSAL_ERROR;
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdAFStart_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_AF_Start (0x20) to the CMOST
 * \details		Preliminary command required before an AF-check or AF-search procedure.
 * 				The function issues the CMOST command then waits #ETAL_CMOST_AF_START_MINIMUM_TIME
 * 				before returning to ensure the quality measure for the next measurement will be stable.
 *
 *				The function returns the quality measured on the *alternateFrequency*
 *				in the **previous** call to this function, thus the quality returned
 *				after the first call to this function should be discarded.
 *
 * \remark		The function does not validate its parameters except the *hReceiver*.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	AFMode    - selects if it is a normal mode command or a restart command; in the latter
 * 				            case the CMOST re-uses the parameters send previously.
 * \param[in]	alternateFrequency - the frequency to tune to, in kHz
 * \param[in]	antennaSelection   - antenna selection: 0 for automatic selection,
 * 				                     1 for FM1 antenna, 2 for FM2 antenna
 * \param[out]	p - address of a location containing an EtalBcastQualityContainer where
 * 				    the function stores the measured quality. If NULL the quality is not
 * 				    returned.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_NOT_SUPPORTED - *hReceiver* is not configured for FM nor DAB
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdAFStart_CMOST(ETAL_HANDLE hReceiver, etalAFModeTy AFMode, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p)
{
                             //            CID              PAR1              PAR2              PAR3
	tU8 CMOSTcmd_AFStart[] = {0x00, 0x20, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU32 extra_len = 0;
	tU16 cstatus;
	tSInt retval;
    tU8 resp[CMOST_MAX_RESPONSE_LEN];
    tU32 resp_len;
    EtalBcastStandard standard;
	ETAL_HANDLE hTuner;
	etalReceiverStatusTy *recvp;

	/* start AF */
	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_AFStart, hReceiver);

	if(AFMode == cmdRestartAFMeasurement)
	{
		// send only PAR1
		extra_len = 6;
		ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AFStart[ETAL_CMOST_HEADER_24BIT], 0x01);
	}
	else
	{
		ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AFStart[ETAL_CMOST_PARAM2_24BIT], alternateFrequency);
		ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AFStart[ETAL_CMOST_PARAM3_24BIT], antennaSelection);
	}

	if ((retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_AFStart, sizeof(CMOSTcmd_AFStart) - extra_len, &cstatus, resp, &resp_len)) == OSAL_OK)
	{
	    if ((p != NULL) && (resp_len >= ETAL_CMOST_GET_AFQUALITY_RESP_LEN))
		{
	    	standard = ETAL_receiverGetStandard(hReceiver);
			(void)OSAL_pvMemorySet((tVoid *)p, 0x00, sizeof(EtalBcastQualityContainer));

			switch(standard)
			{
				case ETAL_BCAST_STD_FM:
				case ETAL_BCAST_STD_HD_FM:
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
					p->m_TimeStamp = OSAL_ClockGetElapsedTime();
					p->m_standard = standard;
					ETAL_extractAmFmQualityFromNotification_CMOST(channelQualityForAF, p, standard, &resp[ETAL_CMD_AF_START_ANSWER_QUALITY_INDEX]);
#else
					retval = OSAL_ERROR_NOT_SUPPORTED;
#endif
					break;

				case ETAL_BCAST_STD_DAB:
					// For DAB band, quality register contains all 0, so no need to extract results.
					break;

				default:
					retval = OSAL_ERROR_NOT_SUPPORTED;
					break;
			}
		}

		/* An error returned by ETAL_receiverGetTunerId is not a communication error
		 * so don't send an event in that case */
		if (ETAL_receiverGetTunerId(hReceiver, &hTuner) == OSAL_OK)
		{
			recvp = ETAL_receiverGet(hReceiver);
			if (recvp == NULL)
			{
				retval = OSAL_ERROR;
			}
			else
			{
				if (ETAL_waitBusyBit_CMOST(hTuner, recvp->CMOSTConfig.channel, ETAL_AF_START_DELAY_CMOST) != OSAL_OK)
				{
					ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_cmdAFStart_CMOST busy bit error, hTuner %d, tuner_mask = %d", 
										hTuner, recvp->CMOSTConfig.channel);
				}

			    /* Time to wait to be sure the quality measure on the CMOST stabilizes for the next
				 * invocation of this function */
		        (void)OSAL_s32ThreadWait(ETAL_CMOST_AF_START_MINIMUM_TIME);
			}
		}
	}
	else
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
        retval = OSAL_ERROR;
	}
	return retval;
}

/***************************
 *
 * ETAL_cmdAFEnd_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_AF_End (0x21) to the CMOST
 * \details		The function terminates an AF procedure and reads the quality info
 * 				reported by the STAR.
 * 				The function issues the CMOST command then waits #ETAL_CMOST_AF_END_MINIMUM_TIME
 * 				before returning to ensure the quality measure for the next measurement will be stable.
 *
 * \remark		The function does not validate its parameters except the *hReceiver*.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	frequency - the frequency to tune to at the end of the procedure, in kHz,
 * 				            or 0xFFFFFF to tune to the original frequency.
 * \param[out]	p - address of a location containing an EtalBcastQualityContainer where
 * 				    the function stores the measured quality. If NULL the quality is not
 * 				    returned.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_NOT_SUPPORTED - hReceiver is no configured FM nor DAB
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdAFEnd_CMOST(ETAL_HANDLE hReceiver, tU32 frequency, EtalBcastQualityContainer* p)
{
                           //            CID              PAR1              PAR2
	tU8 CMOSTcmd_AFEnd[] = {0x00, 0x21, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval;
	tU8 resp[CMOST_MAX_RESPONSE_LEN];
	tU32 resp_len;
    EtalBcastStandard standard;

	(void)ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_AFEnd, hReceiver);
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_AFEnd[ETAL_CMOST_PARAM2_24BIT], frequency);

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_AFEnd, sizeof(CMOSTcmd_AFEnd), &cstatus, resp, &resp_len);
	if (retval == OSAL_OK)
	{
		if ((p != NULL) && (resp_len >= ETAL_CMOST_GET_AFQUALITY_RESP_LEN))
		{
			standard = ETAL_receiverGetStandard(hReceiver);
			(void)OSAL_pvMemorySet((tVoid *)p, 0x00, sizeof(EtalBcastQualityContainer));

			switch(standard)
			{
				case ETAL_BCAST_STD_FM:
				case ETAL_BCAST_STD_HD_FM:
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
					p->m_TimeStamp = OSAL_ClockGetElapsedTime();
					p->m_standard = standard;
					ETAL_extractAmFmQualityFromNotification_CMOST(channelQualityForAF, p, standard, &resp[ETAL_CMD_AF_END_ANSWER_QUALITY_INDEX]);
#else
					retval = OSAL_ERROR_NOT_SUPPORTED;
#endif
					break;

				case ETAL_BCAST_STD_DAB:
					// For DAB band, quality register contains all 0, so no need to extract results.
					break;

				default:
					retval = OSAL_ERROR_NOT_SUPPORTED;
					break;
			}
		}

	    /* Time to wait to be sure the quality measure on the CMOST stabilizes for the next
		 * invocation of this function */
        (void)OSAL_s32ThreadWait(ETAL_CMOST_AF_END_MINIMUM_TIME);
	}
	else
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
        retval = OSAL_ERROR;
	}
	return retval;
}

/***************************
 *
 * ETAL_cmdGetAFQuality_CMOST
 *
 **************************/
/*!
 * \brief		Sends a TUNER_Get_AF_Quality (0x24) to the CMOST
 * \remark		The function does not validate its parameters except the *hReceiver*.
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	p - address of a location containing an EtalBcastQualityContainer where
 * 				    the function stores the measured quality.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_NOT_SUPPORTED - ETAL not compiled with STAR support
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdGetAFQuality_CMOST(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p)
{
                                  //            CID              PAR1
	tU8 CMOSTcmd_GetAFQuality[] = {0x00, 0x24, 0x01, 0x00, 0x00, 0x00};
	tU16 cstatus = 0;
	tU32 resplen = 0, tuner_mask;
	tU8 resp[CMOST_MAX_RESPONSE_LEN];
	tSInt retval = OSAL_OK;
	ETAL_HANDLE hTuner;
	EtalBcastStandard standard;

	tuner_mask = ETAL_paramSetTunerChannel_CMOST(CMOSTcmd_GetAFQuality, hReceiver);

	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_GetAFQuality, sizeof(CMOSTcmd_GetAFQuality), &cstatus, resp, &resplen);

	if ((retval != OSAL_OK) || (resplen < ETAL_CMOST_GET_AMFMMON_RESP_LEN))
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, resp, resplen);
		retval = OSAL_ERROR;
		goto exit;
	}

	if (ETAL_receiverGetTunerId(hReceiver, &hTuner) != OSAL_OK)
	{
		// will never reach here because the hReceiver was validated by ETAL_sendCommandTo_CMOST
		retval = OSAL_ERROR;
		goto exit;
	}
	
	if (ETAL_waitBusyBit_CMOST(hTuner, tuner_mask, ETAL_CHANGE_BAND_DELAY_CMOST) != OSAL_OK)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_cmdGetAFQuality_CMOST busy bit error, hTuner %d, tuner_mask = %d", 
							hTuner, tuner_mask);
	}

	/* Get results */
	standard = ETAL_receiverGetStandard(hReceiver);
	(void)OSAL_pvMemorySet((tVoid *)p, 0x00, sizeof(EtalBcastQualityContainer));

	switch(standard)
	{
		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_HD_FM:
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
                p->m_TimeStamp = OSAL_ClockGetElapsedTime();
                p->m_standard = standard;
                ETAL_extractAmFmQualityFromNotification_CMOST(channelQualityForAF, p, standard, &resp[ETAL_CMD_GET_AF_QUALITY_ANSWER_QUALITY_INDEX]);
#else
			retval = OSAL_ERROR_NOT_SUPPORTED;
#endif
			break;

		case ETAL_BCAST_STD_DAB:
			// For DAB band, quality register contains all 0, so no need to extract results.
			break;

		default:
			retval = OSAL_ERROR_NOT_SUPPORTED;
			break;
	}

exit:
	return retval;
}

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
#if defined(CONFIG_ETAL_HAVE_DEBUG_COMMANDS) || defined(CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_cmdSetFMProc_CMOST
 *
 **************************/
/*!
 * \brief		Sends a Tuner_Set_FM_Proc (0x11) to the CMOST
 * \remark		This CMOST command switch the foreground channel among single tuner, phase diversity (VPA). 
 * \remark		Must not be used to drive the tuner DISS in the normal application because it may damage the receiver structure.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	fmMode - configures the mode of DISS (auto, manual, VPA tacking)
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdSetFMProc_CMOST(ETAL_HANDLE hReceiver, EtalFMMode fmMode)
{
						//            CID              PAR1              PAR2
	tU8 CMOSTcmd_SetFMProc[] = {0x00, 0x11, 0x01, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval;

	// FM Mode : parameter 1, on 3 bytes
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_SetFMProc[ETAL_CMOST_PARAM1_24BIT], fmMode);

	/* send command to CMOST */
	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_SetFMProc, 3 + (tU32)CMOSTcmd_SetFMProc[ETAL_CMOST_HEADER_24BIT + 2] * 3, &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		goto exit;
	}

	/* set Config VPA Mode */
	if (fmMode == ETAL_FM_MODE_VPA)
	{
		if (ETAL_receiverSetConfigVPAMode(hReceiver, TRUE) != ETAL_RET_SUCCESS)
		{
			retval = OSAL_ERROR;
			goto exit;
		}
	}
	else if (fmMode == ETAL_FM_MODE_SINGLE_TUNER)
	{
		if (ETAL_receiverSetConfigVPAMode(hReceiver, FALSE) != ETAL_RET_SUCCESS)
		{
			retval = OSAL_ERROR;
			goto exit;
		}
	}
	else
	{
		/* Nothing to do */
	}

	retval = OSAL_OK;
	
exit:
	return retval;
}

/***************************
 *
 * ETAL_cmdDebugSetDISS_CMOST
 *
 **************************/
/*!
 * \brief		Sends a DEVICE_SET_DISS (0x31) to the CMOST
 * \remark		This CMOST command is for debug only to change DISS status. 
 * \remark		Must not be used to drive the tuner DISS in the normal application because it may damage the receiver structure.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	tuner_channel - the Tuner channel to use
 * \param[in]	mode - configures the mode of DISS (auto, manual, VPA tacking)
 * \param[in]	filter_index - optional filter index, used only in manual mode. 
 *                             10 coefficient sets are supported in FM, index 0 is the narrowest filter (cut off frequency 25 kHz), 
 *                             while index 9 is the widest one (cut off frequency 135 kHz).
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_NOT_SUPPORTED - hReceiver broadcast standard not supported
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdDebugSetDISS_CMOST(ETAL_HANDLE hReceiver, etalChannelTy tuner_channel, EtalDISSMode mode, tU8 filter_index)
{
							//            CID              PAR1              PAR2
	tU8 CMOSTcmd_DebugSetDISS[] = {0x00, 0x31, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU16 cstatus;
	tSInt retval;

	if (mode == ETAL_DISS_MODE_AUTO)
	{
		/* adapt command length */
		CMOSTcmd_DebugSetDISS[ETAL_CMOST_HEADER_24BIT + 2] = (tU8)0x02;
	}
	
	// init with input parameters
	// tuner channel : parameter 1, on 3 bytes
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_DebugSetDISS[ETAL_CMOST_PARAM1_24BIT], tuner_channel);
	//mode : parameter 2, on 3 bytes
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_DebugSetDISS[ETAL_CMOST_PARAM2_24BIT], mode);
	//mode : parameter 3, on 3 bytes
	ETAL_paramSetGeneric_CMOST(&CMOSTcmd_DebugSetDISS[ETAL_CMOST_PARAM3_24BIT], (tU32)filter_index);

	/* send command to CMOST */
	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_DebugSetDISS, (tU32)(3 + ((tU32)CMOSTcmd_DebugSetDISS[ETAL_CMOST_HEADER_24BIT + 2] * 3)), &cstatus, NULL, NULL);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
	}

	return retval;
}

/***************************
 *
 * ETAL_cmdDebugGetWSPStatus_CMOST
 *
 **************************/
/*!
 * \brief		Sends a DEVICE_GET_WSP_Status (0x32) to the CMOST
 * \remark		This CMOST command is for debug only to get WSP status. 
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	WSPStatus - response of type EtalWSPStatus of WSP status.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error or invalid *hReceiver*; also sends an #ETAL_ERROR_COMM_FAILED event
 * \return		OSAL_ERROR_NOT_SUPPORTED - hReceiver broadcast standard not supported
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt ETAL_cmdDebugGetWSPStatus_CMOST(ETAL_HANDLE hReceiver, EtalWSPStatus *WSPStatus)
{
							//            CID              PAR1              PAR2
	tU8 CMOSTcmd_DebugSetDISS[] = {0x00, 0x32, 0x00};
	tU16 cstatus;
	tSInt retval;
	tU8 resp[16];
	tU32 rlen;

	/* send command to CMOST */
	retval = ETAL_sendCommandTo_CMOST(hReceiver, CMOSTcmd_DebugSetDISS, sizeof(CMOSTcmd_DebugSetDISS), &cstatus, resp, &rlen);
	if (retval != OSAL_OK)
	{
		ETAL_sendCommunicationErrorEvent_CMOST(hReceiver, retval, cstatus, NULL, 0);
		goto exit;
	}

	/* check length of response */
	if (rlen < 9)
	{
		retval = OSAL_ERROR;
		goto exit;
	}

	WSPStatus->m_filter_index[0] = resp[0] & 0x0F;
	WSPStatus->m_filter_index[1] = (resp[0] >> 4) & 0x0F;
	WSPStatus->m_softmute        = resp[1];
	WSPStatus->m_highcut         = (tS8)resp[2];
	WSPStatus->m_lowcut          = (tS8)resp[3];
	WSPStatus->m_stereoblend     = resp[4];
	WSPStatus->m_highblend       = resp[5];
	WSPStatus->m_rolloff         = (((tS32)(tS8)resp[6]) << 16) |
			                       (((tU32)resp[7]) << 8) |
			                       (tU32)resp[8];

	retval = OSAL_OK;

exit:
	return retval;
}
#endif // defined(CONFIG_ETAL_HAVE_DEBUG_COMMANDS) || defined(CONFIG_ETAL_HAVE_ALL_API)
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

#endif // CONFIG_ETAL_SUPPORT_CMOST
