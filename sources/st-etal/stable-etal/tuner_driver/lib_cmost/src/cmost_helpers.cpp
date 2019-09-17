//!
//!  \file		cmost_helpers.cpp
//!  \brief		<i><b> CMOST driver </b></i>
//!  \details	CMOST helper functionalities. This is mainly one function
//!				defined to ease access to the CMOST protocol function.
//!  $Author$
//!  \author	(original version) Raffaele Belardi, Roberto Allevi, Alberto Saviotti
//!  $Revision$
//!  $Date$
//!


#include "target_config.h"

#include "osal.h"

#if defined (CONFIG_ETAL_SUPPORT_CMOST)

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	#include "etaldefs.h" // required by ipfcomm.h
	#include "ipfcomm.h"
	#include "TcpIpProtocol.h"
#endif

#include "tunerdriver_internal.h"
#include "tunerdriver.h"
#include "cmost_protocol.h"
#include "cmost_helpers.h"
#include "common_trace.h"
#include "cmost_trace.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef CONFIG_COMM_DRIVER_EXTERNAL
/*!
 * \var		ETAL_toIpfBufCMOST
 * 			Local buffer used to build the command to be sent
 * 			to the external driver.
 * \todo	The buffer is unique across the system, it should be made
 * 			tuner-dependent to avoid race conditions in multiple-tuner
 * 			environment
 */
static tU8 ETAL_toIpfBufCMOST[PROTOCOL_LAYER_TO_DEVICE_BUFFER_LEN]; /* TODO see description */

OSAL_tSemHandle cmostHelperSendMsg_sem = 0;
#endif

/***************************
 *
 * Macros
 *
 **************************/
/*!
 * \def		CMOST_COMMAND_ID
 * 			Extracts the CMOST Command Id from a buffer
 * 			containing the 3-bytes header of a CMOST command
 * 			or response
 */
#define CMOST_COMMAND_ID(_buf)         (((_buf)[1] >> 8) & 0xFF)
/*!
 * 			CMOST_RESPONSE_STATUS
 * 			Extracts the CMOST Status byte from a buffer
 * 			containing the 3-bytes header of a CMOST command
 * 			or response
 */
#define CMOST_RESPONSE_STATUS(_buf)     ((_buf)[0] & 0xC0)

#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_SYSTEM_MIN)
/***************************
 *
 * CMOST_statusToString
 *
 **************************/
/*!
 * \brief		Converts a CMOST status to a printable string
 * \param[in]	status - the CMOST status byte, i.e. the one obtained with #CMOST_RESPONSE_STATUS
 * \return		pointer to a string
 * \callgraph
 * \callergraph
 */
static tCString CMOST_statusToString(tU8 status)
{
	static tCString vl_string = NULL;
	
	if (status == (tU8)0)
	{
		vl_string =  "No error";
	}
	else if (CMOST_CHECKSUM_ERROR(status) && !CMOST_ILLEGAL_CID_ERROR(status))
	{
		vl_string = "Checksum error";
	}
	else if (!CMOST_CHECKSUM_ERROR(status) && CMOST_ILLEGAL_CID_ERROR(status))
	{
		vl_string = "Illegal Command ID";
	}
	else if (CMOST_CHECKSUM_ERROR(status) && CMOST_ILLEGAL_CID_ERROR(status))
	{
		vl_string =  "Checksum error and illegal Command ID";
	}
	else
	{
		vl_string = "Unknown status code";
	}
	
	return vl_string;
}
#endif

/***************************
 *
 * CMOST_helperSendMessage
 *
 **************************/
/*!
 * \brief		Main entry point for the CMOST driver
 * \details		For CONFIG_COMM_DRIVER_EMBEDDED the function is an almost empty wrapper
 * 				around the #CMOST_ProtocolHandle which implements the real CMOST protocol.
 *
 * 				For CONFIG_COMM_DRIVER_EXTERNAL the function pushes the command
 * 				to the external CMOST driver over the TCP/IP, then reads the response
 * 				over the same link after a fixed amount of time.
 * \param[in]	param - the work area used by the function. It must be allocated by the caller.
 * 				        The function overwrites the content of this area.
 * \param[in]	buf - the command or firmware chunk to be sent to the CMOST.
 * 				      This is a pointer to a data buffer initialized by the caller.
 * 				      The content of the buffer depends on the *mode*, for details 
 * 				      see #CMOST_cmdModeEnumTy.
 * \param[in]	buf_len - size in bytes of *buf*
 * \param[in]	mode - requested mode of operation; see #CMOST_cmdModeEnumTy for details.
 * \param[in]	access_size - indicates if 24-bit or 32-bit access mode is required.
 * 				              The access_size actually depends on the operational mode, not all
 * 				              combinations are legal. See #CMOST_cmdModeEnumTy for details
 * \param[in]	deviceID - the device ID of the CMOST
 * \param[out]	resp - the address of a buffer allocated by the caller of the function
 * 				       where the function stores the response to the command (if any).
 * 				       Depending on *mode* this buffer must be capable of holding 
 * 				       #CMOST_MAX_RESPONSE_LEN bytes.
 * \param[out]	resp_lenp - the address of an integer variable where the function
 * 				            stores the amount of bytes written to *resp*
 * \return		OSAL_OK
 * \return		OSAL_ERROR
 * \return		OSAL_ERROR_TIMEOUT_EXPIRED
 * \callgraph
 * \callergraph
 */
tSInt CMOST_helperSendMessage(CMOST_paramsTy *param, tU8 *buf, tU32 buf_len, CMOST_cmdModeEnumTy mode, CMOST_accessSizeEnumTy access_size, tU32 deviceID, tU8 *resp, tU32 *resp_lenp)
{
#ifdef CONFIG_COMM_DRIVER_EMBEDDED
	tS32 s32rlen = 0;
#elif defined (CONFIG_COMM_DRIVER_EXTERNAL)
	CtrlAppMessageHeaderType head;
	tU32 rlen;
	OSAL_tMSecond end_time;
#endif
	tSInt ret = OSAL_OK;
	tU32 device_address;

	(void)OSAL_pvMemorySet((tVoid *)param, 0x00, sizeof(*param));
	param->deviceID = deviceID;
	param->useI2c = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ? TRUE : FALSE;
	device_address = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ?
			((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress :
			deviceID; // not used in SPI case

	CMOST_tracePrintComponent(TR_CLASS_CMOST, "Sending command to CMOST device_address 0x%x, useI2C=%d",
			device_address, param->useI2c);

#ifdef CONFIG_COMM_DRIVER_EMBEDDED
	if (CMOST_SendMessageStart((tVoid *)param, buf, (tU8)mode, (tU8)access_size, (tU8)device_address, (tU8)0, buf_len) == 0)
	{
		if (CMOST_ProtocolHandle((tVoid *)param, resp, &s32rlen) != 0)
		{
			// error may be due to parameter error or device error (OSAL_ERROR_DEVICE_INIT).
			// Currently they are not differentiated from the low level driver
			// so we must return a generic error code
			ret = OSAL_ERROR;
		}
	}
	else
	{
		ret = OSAL_ERROR;
	}
	if (s32rlen < 0)
	{
		*resp_lenp = 0;
		ret = OSAL_ERROR;
	}
	else
	{
		*resp_lenp = (tU32)s32rlen;
	}
#elif defined (CONFIG_COMM_DRIVER_EXTERNAL)
	if (ProtocolLayerIndexCMOST == PROTOCOL_LAYER_INDEX_INVALID)
	{
		return OSAL_ERROR;
	}
	if (buf_len > PROTOCOL_LAYER_TO_DEVICE_BUFFER_LEN - sizeof(head))
	{
		CMOST_tracePrintError(TR_CLASS_CMOST, "Illegal CMOST message size %d", buf_len);
		return OSAL_ERROR;
	}

	if (OSAL_s32SemaphoreWait(cmostHelperSendMsg_sem, OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}
	head.CtrlAppSync =    (tU8)0x1D;
	head.CtrlAppLunId =   (tU8)0xFF; // unused
	head.CtrlAppSpare_0 = (tU8)mode;
	head.CtrlAppSpare_1 = (tU8)access_size;
	head.CtrlAppSpare_2 = (tU8)device_address;
	head.CtrlAppSpare_3 = (tU8)0; // unused
	/*
	 * this ugly code is to switch MSB and LSB, because this is the
	 * way the MDR_protocol layer expects them
	 */
	head.CtrlAppCmdDataLen = (tU16)(((buf_len & 0xFF) << 8) | ((buf_len >> 8) & 0xFF));
	OSAL_pvMemoryCopy((tVoid *)ETAL_toIpfBufCMOST, (tPCVoid)&head, sizeof(head));
	OSAL_pvMemoryCopy((tVoid *)(ETAL_toIpfBufCMOST + sizeof(head)), (tPCVoid)buf, buf_len);

	/*
	 * send the data to the EXTERNAL driver
	 */
	TcpIpProtocolSendData(ProtocolLayerIndexCMOST, ETAL_toIpfBufCMOST, buf_len + sizeof(head));

	end_time = OSAL_ClockGetElapsedTime() + ETAL_TO_CMOST_CMD_TIMEOUT_EXT_IN_MSEC;
	ret = OSAL_ERROR_TIMEOUT_EXPIRED;
	while (OSAL_ClockGetElapsedTime() < end_time)
	{
		OSAL_s32ThreadWait(TUNER_DRIVER_CMOST_THREAD_SCHEDULING);

		/*
		 * Assume the caller allocated the right amount of data for resp (CMOST_MAX_RESPONSE_LEN)
		 */			
		if (ProtocolLayer_FifoPop(ProtocolLayerIndexCMOST, resp, CMOST_MAX_RESPONSE_LEN, &rlen, &head) == OSAL_OK)
		{
			if (CheckCtrlAppMessage_CMOST((tU8)mode, ProtocolLayerIndexCMOST, &head) != OSAL_OK)
			{
				CMOST_tracePrintSysmin(TR_CLASS_CMOST, "Received unrecognized message from CMOST LUN 0x%x %d bytes", head.CtrlAppLunId, rlen);
				ret = OSAL_ERROR;
			}
			else
			{
				CMOST_tracePrintComponent(TR_CLASS_CMOST, "Received message from CMOST LUN 0x%x %d bytes", head.CtrlAppLunId, rlen);
#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_COMPONENT)
				COMMON_tracePrintBufComponent(TR_CLASS_CMOST, resp, rlen, NULL);
#endif
				ret = OSAL_OK;
			}
			break;
		}
	}

	if (ret == OSAL_OK)
	{
		*resp_lenp = (tU32)rlen;
	}
	else
	{
		// timeout or format error
		*resp_lenp = 0;
	}
#endif // CONFIG_COMM_DRIVER_EXTERNAL

#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_SYSTEM_MIN)
	/*
	 * CMOST_ACCESSSIZE_32BITS denotes the pre-firmware access (direct access),
	 * which does not generate an answer with the RESPONSE_STATUS format
	 */
	if ((access_size != CMOST_ACCESSSIZE_32BITS) && (*resp_lenp > 0) && (CMOST_RESPONSE_STATUS(resp) != (tU8)0))
	{
		CMOST_tracePrintSysmin(TR_CLASS_CMOST, "CMOST Response status 0x%x (%s)", (tU32)CMOST_RESPONSE_STATUS(resp), CMOST_statusToString(CMOST_RESPONSE_STATUS(resp)));
	}
#endif
#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	OSAL_s32SemaphorePost(cmostHelperSendMsg_sem);
#endif
	return ret;
}



#ifdef __cplusplus
}
#endif

#endif //  CONFIG_ETAL_SUPPORT_CMOST

// End of file
