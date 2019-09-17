//!
//!  \file 		etalbootcmd_hdr.c
//!  \brief 	<i><b> ETAL BOOT HDR API layer </b></i>
//!  \details   HD Radio-specific boot command implementation
//!  \author 	David Pastor
//!

#include "osal.h"
#include "etalinternal.h"

#if defined(CONFIG_COMM_DRIVER_EMBEDDED) && defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
#include "hdradio_trace.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
#define HDR_BOOT_RESP_MAX_SIZE        16

/*****************************************************************
| Local types
|----------------------------------------------------------------*/
typedef enum
{
    HDR_COMM_STATUS_OK,
    HDR_COMM_STATUS_ERR_UNKNOWN,
    HDR_COMM_STATUS_ERR_NO_BOARD_RSP
} HDR_commStatusEnumTy;

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
tU8 hdr_boot_resp[HDR_BOOT_RESP_MAX_SIZE];

/*****************************************************************
| prototypes
|----------------------------------------------------------------*/
static tVoid ETAL_MessageFeedback_HDR (tVoid *deviceHandle, tU8 *dataPtr, tU32 numBytes, HDR_commStatusEnumTy errMsg, tU8 *resp, tU32 *respLen);
static tVoid ETAL_ExecuteSpecialCommands_HDR(tVoid *devicehandler, tU8 cmd, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr, tU8 *rspBfr, tU32 *rspLen);


/***************************
 *
 * ETAL_cmdSpecialReset_HDR
 *
 **************************/
tSInt ETAL_cmdSpecialReset_HDR(tVoid)
{
	tU8 resetCmd[]  = { HDR_RESET_CMD, 0, 0, 0 };
	tU16 payload_len;
	tU32 resplen = 0;
	tU8 *resp = hdr_boot_resp, *cmd;
	tSInt ret = OSAL_OK;

	payload_len = 0;
	cmd = (tU8 *)OSAL_pvMemoryAllocate(sizeof(resetCmd) + 2 + payload_len);
	if (cmd != NULL)
	{
		/* add command header */
		(void)OSAL_pvMemoryCopy((tPVoid)cmd, (tPCVoid)resetCmd, sizeof(resetCmd));

		/* add payload length */
		cmd[sizeof(resetCmd) + 0] = (tU8) ((payload_len >> 8) & 0xFF);
		cmd[sizeof(resetCmd) + 1] = (tU8) ((payload_len     ) & 0xFF);

		/* execute special command */
		ETAL_ExecuteSpecialCommands_HDR(NULL, HDR_RESET_CMD, &(cmd[0]), payload_len, &(cmd[sizeof(resetCmd) + 2]), resp, &resplen);

		/* check response */
		if ((resplen <= 6) || (resp[6] != HDR_RESET_CMD) || (resp[1] != (tU8)0))
		{
			ret = OSAL_ERROR;
		}
	}

	return ret;
}

/***************************
 *
 * ETAL_cmdSpecialFlash_HDR
 *
 **************************/
tSInt ETAL_cmdSpecialFlash_HDR(HDR_flashModeEnumTy op_mode, HDR_targetMemOrInstanceIDTy trg_mem_or_instId, HDR_accessModeEnumTy access_mode, const EtalDCOPAttr *dcop_attr)
{
	tU8 flashCmd[]  = { HDR_FLASH_CMD, 0x00, 0x00, 0x00 };
	tU32 payload_len;
	tU32 resplen = 0;
	tU8 *resp = hdr_boot_resp, *cmd = NULL;
	tSInt ret = OSAL_OK;
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE
	tChar *filename =  NULL;
#endif // #ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE

	flashCmd[1] = op_mode;
	flashCmd[2] = trg_mem_or_instId.trg_mem;
	flashCmd[3] = access_mode;

	/* set specific 2 header byte */
	if (op_mode == HDR_FLASH_OP_MODE_IS_PH2CMD)
	{
		flashCmd[2] = (tU8)trg_mem_or_instId.instId;
	}

	if (access_mode == HDR_ACCESS_DATA_MODE)
	{
		/* allocate cmd message */
		payload_len = sizeof(EtalCbGetImage) + sizeof(tPVoid);
		cmd = (tU8 *)OSAL_pvMemoryAllocate(sizeof(flashCmd) + 2 + payload_len);
		if (cmd == NULL)
		{
			ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, (tVoid *)ETAL_INVALID_HANDLE, sizeof(tVoid *));
			ret = OSAL_ERROR;
		}
		else if ((cmd != NULL) && (payload_len >= 8))
		{
			/* add command header */
			(void)OSAL_pvMemoryCopy((tPVoid)cmd, (tPCVoid)flashCmd, sizeof(flashCmd));

			/* add payload length */
			cmd[sizeof(flashCmd) + 0] = (tU8) ((payload_len >> 8) & 0xFF);
			cmd[sizeof(flashCmd) + 1] = (tU8) ((payload_len     ) & 0xFF);

			/* add m_cbGetImage in payload */
			cmd[sizeof(flashCmd) + 2] = (tU8) (((tU32)(dcop_attr->m_cbGetImage) >> 24) & 0xFF);
			cmd[sizeof(flashCmd) + 3] = (tU8) (((tU32)(dcop_attr->m_cbGetImage) >> 16) & 0xFF);
			cmd[sizeof(flashCmd) + 4] = (tU8) (((tU32)(dcop_attr->m_cbGetImage) >>  8) & 0xFF);
			cmd[sizeof(flashCmd) + 5] = (tU8) (((tU32)(dcop_attr->m_cbGetImage)      ) & 0xFF);

			/* add m_pvGetImageContext in payload */
			cmd[sizeof(flashCmd) + 6] = (tU8) (((tU32)(dcop_attr->m_pvGetImageContext) >> 24) & 0xFF);
			cmd[sizeof(flashCmd) + 7] = (tU8) (((tU32)(dcop_attr->m_pvGetImageContext) >> 16) & 0xFF);
			cmd[sizeof(flashCmd) + 8] = (tU8) (((tU32)(dcop_attr->m_pvGetImageContext) >>  8) & 0xFF);
			cmd[sizeof(flashCmd) + 9] = (tU8) (((tU32)(dcop_attr->m_pvGetImageContext)      ) & 0xFF);

			/* execute special command */
			ETAL_ExecuteSpecialCommands_HDR(NULL, HDR_FLASH_CMD, &(cmd[0]), (tU16)payload_len, &(cmd[sizeof(flashCmd) + 2]), resp, &resplen);

			/* check response */
			if ((resplen <= 6) || (resp[6] != HDR_FLASH_CMD) || (resp[1] != (tU8)0))
			{
				ret = OSAL_ERROR;
			}
		}
		else
		{
			ret = OSAL_ERROR;
		}
	}
	else if (access_mode == HDR_ACCESS_FILE_MODE)
	{
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE
		if (dcop_attr->m_pvGetImageContext == NULL)
		{
			ret = OSAL_ERROR;
		}
		if (ret == OSAL_OK)
		{
			/* get filename */
			if (op_mode == HDR_FLASH_OP_MODE_IS_PHASE1)
			{
				filename = (tChar *)(((tChar **)(dcop_attr->m_pvGetImageContext))[0]);
			}
			else if ((op_mode == HDR_FLASH_OP_MODE_IS_PROGRAM) || (op_mode == HDR_FLASH_OP_MODE_IS_PH2CMD))
			{
				filename = (tChar *)(((tChar **)(dcop_attr->m_pvGetImageContext))[1]);
			}
			else
			{
				ret = OSAL_ERROR;
			}
		}
		if (ret == OSAL_OK)
		{
			/* allocate cmd message */
			payload_len = (tU32)OSAL_u32StringLength(filename) + 1;
			cmd = (tU8 *)OSAL_pvMemoryAllocate(sizeof(flashCmd) + 2 + payload_len);
			if (cmd == NULL)
			{
				ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, (tVoid *)ETAL_INVALID_HANDLE,  sizeof(ETAL_HANDLE));
				ret = OSAL_ERROR;
			}
		}
		if (ret == OSAL_OK)
		{
			/* add command header */
			OSAL_pvMemoryCopy((tPVoid)cmd, (tPCVoid)flashCmd, sizeof(flashCmd));

			/* add payload length */
			cmd[sizeof(flashCmd) + 0] = (tU8) ((payload_len >> 8) & 0xFF);
			cmd[sizeof(flashCmd) + 1] = (tU8) ((payload_len     ) & 0xFF);

			/* add filename string in payload */
			OSAL_pvMemoryCopy((tPVoid)&(cmd[sizeof(flashCmd) + 2]), (tPCVoid)filename, payload_len);

			/* execute special command */
			ETAL_ExecuteSpecialCommands_HDR(NULL, HDR_FLASH_CMD, &(cmd[0]), (tU16)payload_len, &(cmd[sizeof(flashCmd) + 2]), resp, &resplen);

			/* check response */
			if ((resplen <= 6) || (resp[6] != HDR_FLASH_CMD) || (resp[1] != (tU8)0))
			{
				ret = OSAL_ERROR;
			}
		}
#else
		ret = OSAL_ERROR_NOT_IMPLEMENTED;
#endif
	}
	else
	{
		/* Nothing to do */
	}

	/* free cmd allocation */
	if (cmd != NULL)
	{
		OSAL_vMemoryFree((tPVoid)cmd);
	}

	return ret;
}

/**************************************
 *
 * ETAL_ExecuteSpecialCommands_HDR
 *
 *************************************/
/*
 * Execute special command
 *
 * Parameters:
 *  devicehandler - pointer to device handle
 *  cmd           - command Id
 *  paramPtr      - pointer to header parameters
 *  numBytes      - number of bytes in dataPtr payload
 *  dataPtr       - pointer to payload
 *  rspBfr        - pointer to response buffer
 *  rspLen        - response byte length written in response buffer
 *
 * Returns:
 *  None
 *
 */
tVoid ETAL_ExecuteSpecialCommands_HDR(tVoid *devicehandler, tU8 cmd, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr, tU8 *rspBfr, tU32 *rspLen)
{
	tU16 res = HDR_COMM_STATUS_OK;
	tU8 dataBfr = (tU8)0;
	tU8 cmd_mode;
	HDR_flashModeEnumTy flashOperation;
	tyHDRADIODeviceConfiguration HDDeviceConfig;

	// Only INSTANCE1 for Flash programming
	//HDR_instanceIdEnumTy instanceNb = HDR_INSTANCE_1;

	if (HDR_RESET_CMD == cmd)
	{
		cmd_mode = (tU8)HDR_RESET_MODE;
	}
	else if (HDR_BOOT_CMD == cmd)
	{
		cmd_mode = (tU8)HDR_BOOT_MODE;
	}
	else if (HDR_FLASH_CMD == cmd)
	{
		cmd_mode = (tU8)HDR_FLASH_MODE;
	}
	else if (HDR_NORMAL_CMD == cmd)
	{
		cmd_mode = (tU8)HDR_NORMAL_MODE;
	}
	else
	{
		// invalid command requested
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "invalid command request : %d", cmd);
		return;
	}

	if ((tU8)HDR_RESET_MODE == cmd_mode)
	{
		// Reset DCOP device
#ifdef CONFIG_COMM_DRIVER_EMBEDDED
			HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "Reset HD Radio device");
			if (HDRADIO_reset() != OSAL_OK)
			{
				res = HDR_COMM_STATUS_ERR_UNKNOWN;
			}
			else
			{
				(void)OSAL_s32ThreadWait(2);
			}
#endif // #ifdef CONFIG_COMM_DRIVER_EMBEDDED

		// Set the payload for the response
		dataBfr = HDR_RESET_CMD;
	}
	else if ((tU8)HDR_BOOT_MODE == cmd_mode)
	{
		// Get the mode
		flashOperation = (HDR_flashModeEnumTy)paramPtr[1];
		// Check if the mode is legal
		if (HDR_NORMAL_MODE == (HDR_cmdModeEnumTy)flashOperation)
		{
			// Restart DCOP device in NORMAL mode
#ifdef CONFIG_COMM_DRIVER_EMBEDDED
			ETAL_getDeviceConfig_HDRADIO(&HDDeviceConfig);
			if (HDRADIO_init(&HDDeviceConfig) != OSAL_OK)
			{
				res = HDR_COMM_STATUS_ERR_UNKNOWN;
			}

			if (HDRADIO_reset() != OSAL_OK)
			{
				res = HDR_COMM_STATUS_ERR_UNKNOWN;
			}

			/*
			 * Time to load the FW from flash and execute it
			 */
			(void)OSAL_s32ThreadWait(HDRADIO_FW_LOADING_TIME);

#endif // #ifdef CONFIG_COMM_DRIVER_EMBEDDED
		}
		else if (HDR_BOOT_MODE == (HDR_cmdModeEnumTy)flashOperation)
		{
			// Restart DCOP device in BOOT mode
#ifdef CONFIG_COMM_DRIVER_EMBEDDED
			HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "Reset HD Radio device");
			if (HDRADIO_reset() != OSAL_OK)
			{
				res = HDR_COMM_STATUS_ERR_UNKNOWN;
			}
#endif // #ifdef CONFIG_COMM_DRIVER_EMBEDDED
		}
		else
		{
			/* Nothing to do */
		}

		// Set the payload for the response
		dataBfr = HDR_BOOT_CMD;
	}
	else if ((tU8)HDR_FLASH_MODE == cmd_mode)
	{
		// Get the mode
		flashOperation = (HDR_flashModeEnumTy)paramPtr[1];

		res = HDR_ExecuteFlashOperations (devicehandler, flashOperation, paramPtr, numBytes, dataPtr);

		if (HDR_COMM_STATUS_OK != res)
		{
			res = HDR_COMM_STATUS_ERR_UNKNOWN;
		}

		// Set the payload for the response
		dataBfr = HDR_FLASH_CMD;
	}
	else
	{
		/* Nothing to do */
	}

	// Send information to upper layer of the reset:
	// - Device handler for the port
	// - Data buffer containing a single byte that is the command
	// - Len of the payload (1)
	ETAL_MessageFeedback_HDR (devicehandler, &dataBfr, 1, (HDR_commStatusEnumTy)res, rspBfr, rspLen);
}

static tVoid ETAL_MessageFeedback_HDR (tVoid *deviceHandle, tU8 *dataPtr, tU32 numBytes, HDR_commStatusEnumTy errMsg, tU8 *resp, tU32 *respLen)
{
#if 0
	tU8 spareBytesRsp[4];

	// Fill spare bytes
	spareBytesRsp[0] = 0;
	spareBytesRsp[1] = (tU8)errMsg;
	spareBytesRsp[2] = 0;
	spareBytesRsp[3] = 0;

	// Send to TCP/IP the feedback that the command did not went through
	if (HDR_FEEDBACK_SENDER_IS_PROTOCOL == feedbackSender)
	{
		ForwardPacketToCtrlAppPort (deviceHandle, dataPtr, numBytes, PROTOCOL_LUN, INVALID_PORT_NUMBER, (tU8 *)&spareBytesRsp[0]);
	}
	else
	{
		ForwardPacketToCtrlAppPort (deviceHandle, dataPtr, numBytes, BROADCAST_LUN, INVALID_PORT_NUMBER, (tU8 *)&spareBytesRsp[0]);
	}
#else
	/* add response control header */
	resp[0] = (tU8)0;
	resp[1] = (tU8)errMsg;
	resp[2] = (tU8)0;
	resp[3] = (tU8)0;
	/* add payload length */
	resp[4] = (tU8) ((numBytes >> 8) & 0xFF);
	resp[5] = (tU8) ((numBytes     ) & 0xFF);
	/* add payload */
	(void)OSAL_pvMemoryCopy((tPVoid)&(resp[6]), (tPVoid)dataPtr, numBytes);
	*respLen = 6 + numBytes;
#endif
}
#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

