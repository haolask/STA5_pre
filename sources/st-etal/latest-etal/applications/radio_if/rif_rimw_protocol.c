//!
//!  \file    rif_rimw_protocol.c
//!  \brief   <i><b> Radio interface middleware protocol </b></i>
//!  \details This module implements the "Middleware Messaging protocol Layer" of the Radio interface ETAL API messages.
//!  \author  David Pastor
//!

#include "target_config.h"
#include "target_config_radio_if.h"

#include "osal.h"

#include "defines.h"

#if defined(CONFIG_HOST_OS_LINUX_EMBEDDED) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	#include "etaldefs.h"
	#include "ipfcomm.h"
#endif
#include "TcpIpProtocol.h"
#include "steci_defines.h"
#include "DAB_Protocol.h"
#include "connection_modes.h"
#include "steci_protocol.h"
#include "steci_uart_protocol.h"

#include "radio_if_util.h"
#include "rif_tasks.h"
#include "rif_msg_queue.h"
#include "rif_protocol_router.h"
#include "rif_rimw_protocol.h"
#include "etal_api.h"
#include "etaltml_api.h"

#include "rif_etalapi_cnv.h"
#include "rif_etaltmlapi_cnv.h"


/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
///
// This define shall be defined just after inlcusion of target config because 
// it is used to correctly setup printf level
///
#define CONFIG_TRACE_CLASS_MODULE       CONFIG_TRACE_CLASS_RIF_RIMW

#define RIF_RIMW_MSG_QUEUE_NAME             "Q_rimw"
#define RIF_RIMW_MSG_QUEUE_NB               5
#define RIF_RIMW_MSG_RESP_MAX_SIZE          1024


/*****************************************************************
| Local types
|----------------------------------------------------------------*/
typedef struct
{
	tU32 msg_id;
	tU32 num_bytes;
	tU8 *msg_buf_ptr;
	tU8 dummy;
} rif_rimw_msg_header_t;


/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
rif_msg_queue_t *rif_rimw_msg_queue_p = NULL;
tU8 rif_rimw_resp[RIF_RIMW_MSG_RESP_MAX_SIZE];
tU8 rif_rimw_auto_notif[RIF_RIMW_MSG_AUTO_NOTIF_MAX_SIZE];


/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/

/*****************************************************************
| functions
|----------------------------------------------------------------*/

/***********************************
 *
 * rif_rimw_OSAL_ERROR_to_rimw_error
 *
 **********************************/
/*!
 * \brief		convert OSAL_ERROR code to rimw error code
 * \details		convert OSAL_ERROR code to rimw error code to send in notification or response
 * \param[in]	osal_error_code - osal_error
 * \return		rimw error code
 * \callgraph
 * \callergraph
 */
tU8 rif_rimw_OSAL_ERROR_to_rimw_error(tSInt osal_error_code)
{
	tU8 rimw_error_code;

	switch (osal_error_code)
	{
		case OSAL_OK:
			rimw_error_code = RIF_RIMW_CMD_STATUS_OK;
			break;
		case OSAL_ERROR_INVALID_PARAM:
			rimw_error_code = RIF_RIMW_CMD_STATUS_ERR_PARAM_WRONG;
			break;
		case OSAL_ERROR_ACTION_FORBIDDEN:
		case OSAL_ERROR_NOT_SUPPORTED:
		case OSAL_ERROR_NOT_IMPLEMENTED:
			rimw_error_code = RIF_RIMW_CMD_STATUS_ERR_UNAVAILABLE_FNCT;
			break;
		case OSAL_ERROR_CHECKSUM:
			rimw_error_code = RIF_RIMW_CMD_STATUS_ERR_CHECKSUM;
			break;
		case OSAL_ERROR_TIMEOUT_EXPIRED:
			rimw_error_code = RIF_RIMW_CMD_STATUS_ERR_TIMEOUT;
			break;
		case OSAL_ERROR:
		case OSAL_ERROR_UNEXPECTED:
		case OSAL_ERROR_DEVICE_INIT:
		case OSAL_ERROR_DEVICE_NOT_OPEN:
		case OSAL_ERROR_CANNOT_OPEN:
		case OSAL_ERROR_FROM_SLAVE:
		case OSAL_ERROR_CALLBACK:
		default:
			rimw_error_code = RIF_RIMW_CMD_STATUS_ERR_GENERIC_FAILURE;
			break;
	}

	return rimw_error_code;
}

/***********************************
 *
 * rif_rimw_get_cmd_payload_address
 *
 **********************************/
/*!
 * \brief		get command payload address
 * \details		get command payload address pointer
 * \param[in]	cmd   - array of bytes containing the command to send; the function
 * 				        does not make any assumption on the content of the array
 * \param[in]	clen  - size in bytes of the *cmd* buffer
 * \return		pointer to command payload
 * \return		NULL if no payload
 * \callgraph
 * \callergraph
 */
tU8 *rif_rimw_get_cmd_payload_address(tU8 *cmd, tU32 clen)
{
	rimw_apiCmdTy *cmd_p = (rimw_apiCmdTy *)cmd;

	/* no payload return NULL */
	if ((cmd_p->header.header_0.fields.Data == 0) || (clen <= 3))
	{
		return NULL;
	}
	/* payload length on 1 byte */
	if (cmd_p->header.header_0.fields.Len == 0)
	{
		return &(cmd_p->payload[1]);
	}
	/* payload length on 2 bytes */
	return &(cmd_p->payload[2]);
}

/***********************************
 *
 * rif_rimw_get_cmd_payload_length
 *
 **********************************/
/*!
 * \brief		get command payload length
 * \details		get command payload length
 * \param[in]	cmd   - array of bytes containing the command to send; the function
 * 				        does not make any assumption on the content of the array
 * \param[in]	clen  - size in bytes of the *cmd* buffer
 * \return		length of command payload
 * \callgraph
 * \callergraph
 */
tU32 rif_rimw_get_cmd_payload_length(tU8 *cmd, tU32 clen)
{
	rimw_apiCmdTy *cmd_p = (rimw_apiCmdTy *)cmd;

	/* no payload return NULL */
	if ((cmd_p->header.header_0.fields.Data == 0) || (clen <= 3))
	{
		return 0;
	}
	/* payload length on 1 byte */
	if (cmd_p->header.header_0.fields.Len == 0)
	{
		return cmd_p->payload[0];
	}
	/* payload length on 2 bytes */
	return ((tU32)(cmd_p->payload[0] & 0x00FF) << 8) | ((tU32)(cmd_p->payload[1] & 0x00FF));
}

/***********************************
 *
 * rif_rimw_ApiCommandParser
 *
 **********************************/
/*!
 * \brief		rimw API command parser
 * \details		rimw API command parser
 * \param[in]	cmd   - array of bytes containing the command to send; the function
 * 				        does not make any assumption on the content of the array
 * \param[in]	clen  - size in bytes of the *cmd* buffer
 * \param[out]	resp  - pointer to a buffer where the function stores the complete RIMW response.
 * \param[out]	rlen  - pointer to an integer where the function stores the size in bytes
 * 				        of the buffer written to *resp*.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM   - invalid parameter
 * \return		OSAL_ERROR_NOT_IMPLEMENTED - command not implemented
 * \callgraph
 * \callergraph
 */
tSInt rif_rimw_ApiCommandParser(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	rimw_apiHeaderTy *cmdhead = (rimw_apiHeaderTy *)cmd, *resphead = (rimw_apiHeaderTy *)resp;
	tU16 payload_len;
	tU8 ret_status = RIF_RIMW_CMD_STATUS_OK;
	tSInt ret = OSAL_OK;

	/* check cmd length */
	if (((cmdhead->header_0.fields.Data == 0) && (((cmdhead->header_0.fields.Len == 1) && (clen <= 3)) || (clen < 3))) ||
		((cmdhead->header_0.fields.Data == 1) && (((cmdhead->header_0.fields.Len == 0) && (clen < 5)) ||
		((cmdhead->header_0.fields.Len == 1) && (clen < 6)))))
	{
		ret_status = RIF_RIMW_CMD_STATUS_ERR_WRONG_CMD_LENGHT;
		ret = OSAL_ERROR_INVALID_PARAM;
	}

	if (ret_status == RIF_RIMW_CMD_STATUS_OK)
	{
		/* check H0 */
		/* STOP bit not supported */
		if ((cmdhead->header_0.fields.Stop == 1) || (cmdhead->header_0.fields.Fast == 1) ||
			(cmdhead->header_0.fields.Sto == 0) || (cmdhead->header_0.fields.Auto == 1) ||
			(cmdhead->header_0.fields.Reply == 1) || (cmdhead->header_0.fields.Host == 0))
		{
			ret_status = RIF_RIMW_CMD_STATUS_ERR_PARAM_WRONG;
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}
	if (ret_status != RIF_RIMW_CMD_STATUS_ERR_WRONG_CMD_LENGHT)
	{
		/* get payload length */
		payload_len = 0;
		if (cmdhead->header_0.fields.Data == 1)
		{
			if (cmdhead->header_0.fields.Len == 0)
			{
				payload_len = cmd[sizeof(rimw_apiHeaderTy)];
				if (clen != (payload_len + 4))
				{
					ret_status = RIF_RIMW_CMD_STATUS_ERR_WRONG_CMD_LENGHT;
					ret = OSAL_ERROR_INVALID_PARAM;
				}
			}
			else
			{
				payload_len = ((tU16)cmd[sizeof(rimw_apiHeaderTy)] << 8) + (tU16)cmd[sizeof(rimw_apiHeaderTy) + 1];
				if (clen != (payload_len + 5))
				{
					ret_status = RIF_RIMW_CMD_STATUS_ERR_WRONG_CMD_LENGHT;
					ret = OSAL_ERROR_INVALID_PARAM;
				}
			}
		}
	}
	/* check message number */
	if((RIF_RIMW_GET_CMD_NUM(cmd) & 0x200) == 0x200){
		if ((clen >= 3) && (rif_etaltmlapi_cnv_checkCmdNumber(cmd, clen) == OSAL_ERROR_NOT_IMPLEMENTED))
		{
			ret_status = RIF_RIMW_CMD_STATUS_ERR_ILLEGAL_CMDNUM;
			ret = OSAL_ERROR_NOT_IMPLEMENTED;
		}	
	}
	else
	{
		if ((clen >= 3) && (rif_etalapi_cnv_checkCmdNumber(cmd, clen) == OSAL_ERROR_NOT_IMPLEMENTED))
		{
			ret_status = RIF_RIMW_CMD_STATUS_ERR_ILLEGAL_CMDNUM;
			ret = OSAL_ERROR_NOT_IMPLEMENTED;
		}		
	}

	/* create notification response */
	/* fill response header */
	resphead->header_0.fields.Stop  = 0;
	resphead->header_0.fields.Data  = 0;
	resphead->header_0.fields.Fast  = 0;
	resphead->header_0.fields.Len   = 0;
	resphead->header_0.fields.Sto   = 1;
	resphead->header_0.fields.Auto  = 0;
	resphead->header_0.fields.Reply = 0;
	resphead->header_0.fields.Host  = 0;
	resphead->header_1.value = cmdhead->header_1.value;
	resphead->header_1.fields.cmdNumM = cmdhead->header_1.fields.cmdNumM;
	resphead->header_1.fields.specific_status = ret_status;
	resphead->header_2.fields.cmdNumL = cmdhead->header_2.fields.cmdNumL;
	*rlen = 3;

	return ret;
}

tVoid rif_rimw_init(tVoid)
{
	/* create message queue */
	rif_rimw_msg_queue_p = rif_msg_create_queue(RIF_RIMW_MSG_QUEUE_NAME, RIF_RIMW_MSG_QUEUE_NB);
}

/***********************************
 *
 * rif_sendMsgTo_rimw
 *
 **********************************/
/*!
 * \brief		send message to rimw
 * \details		send message to radio if middleware
 * \param[in]	msg_id - message ID
 * \param[in]	cmd    - array of bytes containing the command to send; the function
 * 				         does not make any assumption on the content of the array
 * \param[in]	clen   - size in bytes of the *cmd* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication with rimw failed
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \return		OSAL_ERROR_UNEXPECTED - allocation of message failed
 * \callgraph
 * \callergraph
 */
tSInt rif_sendMsgTo_rimw(tU32 msg_id, tU8 *cmd, tU32 clen)
{
	tU32 msg_header_sz;
	rif_msg_queue_t *msg_queue_p;
	rif_rimw_msg_header_t *msg_p;

	rif_rimw_tracePrintComponent(TR_CLASS_EXTERNAL, "%s(%02x, %p, %d)", __FUNCTION__, msg_id, cmd, clen);

	msg_queue_p = rif_rimw_msg_queue_p;
	msg_header_sz = sizeof(rif_rimw_msg_header_t);

	/* check msg_id */
	if (RIF_GET_MSG_ID_TASK_TO(msg_id) != TASK_ID_RIF_RIMW)
	{
		rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "%s invalid param error (%02x, %p, %d)", __FUNCTION__, msg_id, cmd, clen);
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR_INVALID_PARAM;
	}

	/* create msg for rif rimw */
	msg_p = (rif_rimw_msg_header_t *)OSAL_pvMemoryAllocate(msg_header_sz + clen);
	if (msg_p == NULL)
	{
		rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "%s msg alloc error (%02x, %p, %d)", __FUNCTION__, msg_id, cmd, clen);
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR_UNEXPECTED;
	}
	OSAL_pvMemorySet(msg_p, 0, msg_header_sz);
	msg_p->msg_id = msg_id;

	/* copy command in message */
	msg_p->num_bytes = clen;
	if (cmd != NULL)
	{
		OSAL_pvMemoryCopy(((tPVoid)msg_p + msg_header_sz), cmd, clen);
		msg_p->msg_buf_ptr = (tU8 *)((tPVoid)msg_p + msg_header_sz);
	}
	else
	{
		msg_p->msg_buf_ptr = NULL;
	}

	/* send msg to rif rimw */
	if (rif_msg_send(msg_queue_p, msg_p) != OSAL_OK)
	{
		rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "%s send error (%02x, %p, %d)", __FUNCTION__, msg_id, cmd, clen);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

tVoid rif_rimw_protocol_handle(tVoid *pParams)
{
	rif_rimw_msg_header_t *rif_rimw_msg_p;
	tU32 rlen;
	tSInt ret;

	/* init rimw */
	rif_rimw_init();

	while (1)
	{
		/* read msg queue */
		if ((rif_rimw_msg_p = (rif_rimw_msg_header_t *)rif_msg_receive(rif_rimw_msg_queue_p)) != NULL)
		{
			switch (rif_rimw_msg_p->msg_id)
			{
				case MSG_ID_RIF_RIMW_RX_CMD_PR:
					rif_rimw_tracePrintComponent(TR_CLASS_EXTERNAL, "MSG_ID_RIF_RIMW_%02x receive msg_p %p sz %d", rif_rimw_msg_p->msg_id, rif_rimw_msg_p->msg_buf_ptr, rif_rimw_msg_p->num_bytes);
					if ((ret = rif_rimw_ApiCommandParser(rif_rimw_msg_p->msg_buf_ptr, rif_rimw_msg_p->num_bytes, rif_rimw_resp, &rlen)) != OSAL_OK)
					{
						rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_rimw_ApiCommandParser error (%d)", ret);
					}
					/* send notification */
					if ((ret = rif_sendMsgTo_pr(MSG_ID_RIF_PR_TX_RESP_RIMW, STECI_UART_MCP_LUN, rif_rimw_resp, rlen)) != OSAL_OK)
					{
						rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_sendMsgTo_pr error (%d)", ret);
					}
					else
					{
						if((RIF_RIMW_GET_CMD_NUM(rif_rimw_msg_p->msg_buf_ptr) & 0x200) == 0x200)
						{	
							if ((ret = rif_etaltmlapi_cnv_cmd(rif_rimw_msg_p->msg_buf_ptr, rif_rimw_msg_p->num_bytes, rif_rimw_resp, &rlen)) != OSAL_OK)
							{
								rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_etaltmlapi_cnv_cmd error (%d)", ret);
							}
							else if (rlen > RIF_RIMW_MSG_RESP_MAX_SIZE)
							{
								rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_etaltmlapi_cnv_cmd rif_rimw_resp overflow error (%d) %d %d", ret, rlen, RIF_RIMW_MSG_RESP_MAX_SIZE);
								printf("rif_etaltmlapi_cnv_cmd rif_rimw_resp overflow error (%d) %d %d\n", ret, rlen, RIF_RIMW_MSG_RESP_MAX_SIZE);
								ASSERT_ON_DEBUGGING(0);
							}
							else
							{
								/* send response */
								if ((ret = rif_sendMsgTo_pr(MSG_ID_RIF_PR_TX_RESP_RIMW, STECI_UART_MCP_LUN, rif_rimw_resp, rlen)) != OSAL_OK)
								{
									rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_sendMsgTo_pr error (%d)", ret);
								}
							}
						}
						else
						{
							if ((ret = rif_etalapi_cnv_cmd(rif_rimw_msg_p->msg_buf_ptr, rif_rimw_msg_p->num_bytes, rif_rimw_resp, &rlen)) != OSAL_OK)
							{
								rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_etalapi_cnv_cmd error (%d)", ret);
							}
							else if (rlen > RIF_RIMW_MSG_RESP_MAX_SIZE)
							{
								rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_etalapi_cnv_cmd rif_rimw_resp overflow error (%d) %d %d", ret, rlen, RIF_RIMW_MSG_RESP_MAX_SIZE);
								printf("rif_etalapi_cnv_cmd rif_rimw_resp overflow error (%d) %d %d\n", ret, rlen, RIF_RIMW_MSG_RESP_MAX_SIZE);
								ASSERT_ON_DEBUGGING(0);
							}
							else
							{
								/* send response */
								if ((ret = rif_sendMsgTo_pr(MSG_ID_RIF_PR_TX_RESP_RIMW, STECI_UART_MCP_LUN, rif_rimw_resp, rlen)) != OSAL_OK)
								{
									rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_sendMsgTo_pr error (%d)", ret);
								}
							}
						}
					}
					break;
			}

			/* release msg from queue */
			if (rif_msg_release(rif_rimw_msg_queue_p, rif_rimw_msg_p) != OSAL_OK)
			{
				rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif rimw msg release error");
			}
		}
		// We will rest if we do not have something to do already
		OSAL_s32ThreadWait(1);
	}
}

