//!
//!  \file    rif_protocol_router.c
//!  \brief   <i><b> Radio interface protocol router </b></i>
//!  \details This module implements the "Logical Messaging router Layer" of the Radio interface messages.
//!  \author  David Pastor
//!

#include "target_config.h"
#include "target_config_radio_if.h"

#include "osal.h"

#include "defines.h"

#include "etaldefs.h"
#include "ipfcomm.h"
#include "TcpIpProtocol.h"

#include "steci_defines.h"
#include "DAB_Protocol.h"
#include "connection_modes.h"
#include "steci_protocol.h"
#include "steci_uart_protocol.h"
#include "HDRADIO_Protocol.h"
#include "hdr_boot.h"
#include "radio_if.h"

#include "etal_api.h"
#include "etaltml_api.h"

#include "etalinternal.h"
#include "tunerdriver_internal.h" // for CMOST_MAX_RESPONSE_LEN

#include "radio_if_util.h"
#include "rif_tasks.h"
#include "rif_msg_queue.h"
#include "rif_protocol_router.h"
#include "rif_rimw_protocol.h"


/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

///
// This define shall be defined just after inlcusion of target config because 
// it is used to correctly setup printf level
///
#define CONFIG_TRACE_CLASS_MODULE       CONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER

#define RIF_PR_MSG_NB_MAX       10
#define RIF_PR_MSG_QUEUE_NAME   "Q_rifPR"
#define RIF_PR_MSG_QUEUE_NB     5

/*!
 * \def		ETAL_CMOST_COLLISION_ERROR
 * 			Responses to commands may contain a collision indication
 * 			(i.e. the command was not executed because another command
 * 			was being executed). This macro accesses that bit.
 */
#define RIF_CMOST_COLLISION_ERROR(_buf)    (((tU32)(_buf) & 0x20) == 0x20)
/*!
 * \def		ETAL_CMOST_COLLISION_WAIT
 * 			If the CMOST driver reports a collision after ETAL sends a command to CMOST,
 * 			ETAL sleeps for this amount of milliseconds before re-sending the command.
 * 			The command transmission is retried for at most #ETAL_TO_CMOST_CMD_TIMEOUT_IN_MSEC ms.
 */
#define RIF_CMOST_COLLISION_WAIT           10

typedef enum
{
	STATE_RIF_PR_IDLE
} state_rif_pr_t;


/*****************************************************************
| Local types
|----------------------------------------------------------------*/
typedef struct
{
	tU8 lun;
	tU32 num_bytes;
	tU8 *msg_buf_ptr;
} rif_pr_msg_buf;

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
rif_pr_msg_buf rif_pr_message_list[RIF_PR_MSG_NB_MAX];
tU8 rif_pr_message_list_rd = 0, rif_pr_message_list_wr = 0;
rif_msg_queue_t *rif_pr_msg_queue_p = NULL;
tU8 rif_pr_resp[
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
				((CMOST_MAX_RESPONSE_LEN > (HDRADIO_CP_OVERHEAD + LM_PAYLOAD_MAX_LEN))?CMOST_MAX_RESPONSE_LEN:(HDRADIO_CP_OVERHEAD + LM_PAYLOAD_MAX_LEN))
#else
				CMOST_MAX_RESPONSE_LEN
#endif // #ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
				];

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/
tSInt rif_sendCommandTo_CMOST(ETAL_HANDLE hGeneric, tU8 *cmd, tU32 clen, tU16 *cstat, tU8 *resp, tU32 *rlen);

/*****************************************************************
| functions
|----------------------------------------------------------------*/

/***********************************
 *
 * rif_protocol_router_handle
 *
 **********************************/
/*!
 * \brief		rif protocol router handle
 * \details		rif protocol router handle thread entry
 * \return		void
 * \callgraph
 * \callergraph
 */
tVoid rif_protocol_router_handle(tVoid *pParams)
{
	tU8 i;
#if 0
	tU8 txBuf[16];
#endif
	tU16 cstatus;
	tSInt retval;
	tU8 *resp = rif_pr_resp;
#if defined(CONFIG_ETAL_SUPPORT_DCOP_MDR) && defined(CONFIG_COMM_DRIVER_DIRECT)
	tU8 *presp;
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR && CONFIG_COMM_DRIVER_DIRECT
	tU32 rlen;
	ETAL_HANDLE hTuner1, hTuner2;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	tS32 srlen;
	tyHDRADIOInstanceID instIdRes;
#endif // #ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	rif_pr_msg_header_t *rif_pr_msg_p;
	state_rif_pr_t rif_pr_state = STATE_RIF_PR_IDLE;

	/* initialize rif_pr_message_list */
	for(i = 0; i < RIF_PR_MSG_NB_MAX; i++)
	{
		rif_pr_message_list[i].lun = INVALID_LUN;
		rif_pr_message_list[i].num_bytes = 0;
		rif_pr_message_list[i].msg_buf_ptr = NULL;
	}
	rif_pr_message_list_rd = rif_pr_message_list_wr = 0;

	/* create message queue */
	rif_pr_msg_queue_p = rif_msg_create_queue(RIF_PR_MSG_QUEUE_NAME, RIF_PR_MSG_QUEUE_NB);

	/* create CMOST tuner handle*/
	hTuner1 = ETAL_handleMakeTuner(0);
	hTuner2 = ETAL_handleMakeTuner(1);

	while (1)
	{
		/* read msg queue */
		if ((rif_pr_msg_p = (rif_pr_msg_header_t *)rif_msg_receive(rif_pr_msg_queue_p)) != NULL)
		{
			switch (rif_pr_state)
			{
				case STATE_RIF_PR_IDLE:
					switch (rif_pr_msg_p->msg_id)
					{
						case MSG_ID_RIF_PR_TX_RESP_DABMW:
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
							OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "rif protocol router transmit dabmw msg_p %x lun %x len %d",
								rif_pr_msg_p, rif_pr_msg_p->lun, rif_pr_msg_p->num_bytes);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)

							/* send DCOP DAB response */
							if (rif_pr_msg_p->lun == 0x30)
							{
								STECI_UART_SendMessageStart(&rif_SteciUartParams, rif_pr_msg_p->lun, rif_pr_msg_p->msg_buf_ptr,
									0, 0, 0, 0, rif_pr_msg_p->num_bytes, 0, DEV_PROTOCOL_STECI);
							}
							break;

						case MSG_ID_RIF_PR_TX_RESP_RIMW:
							rif_pr_tracePrintSysmin(TR_CLASS_EXTERNAL, "rif protocol router transmit rimw msg_p %x lun %x len %d",
								rif_pr_msg_p, rif_pr_msg_p->lun, rif_pr_msg_p->num_bytes);
							/* send RIMW response */
							STECI_UART_SendMessageStart(&rif_SteciUartParams, rif_pr_msg_p->lun, rif_pr_msg_p->msg_buf_ptr,
								0, 0, 0, 0, rif_pr_msg_p->num_bytes, 0, DEV_PROTOCOL_MCP);
							break;
					}
					break;
			}

			/* release msg from queue */
			if (rif_msg_release(rif_pr_msg_queue_p, rif_pr_msg_p) != OSAL_OK)
			{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
				OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router msg release error");
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
			}
		}

		if (rif_pr_message_list_rd != rif_pr_message_list_wr)
		{
			if (rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr != NULL)
			{
				rif_pr_tracePrintSysmin(TR_CLASS_EXTERNAL, "rif protocol router received lun %x len %d",
					rif_pr_message_list[rif_pr_message_list_rd].lun, rif_pr_message_list[rif_pr_message_list_rd].num_bytes);

				/* send command to DCOP DAB */
				if (rif_pr_message_list[rif_pr_message_list_rd].lun == 0x30)
				{
#if defined(CONFIG_ETAL_SUPPORT_DCOP_MDR) && defined(CONFIG_COMM_DRIVER_DIRECT)
					retval = ETAL_sendDirectCommandTo_MDR(rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr, rif_pr_message_list[rif_pr_message_list_rd].num_bytes, &cstatus, TRUE, &presp, &rlen);
					if (retval != OSAL_OK)
					{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
						OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router DCOP MDR send command error %d", retval);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					}
					else
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR && CONFIG_COMM_DRIVER_DIRECT
					{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
						OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "rif protocol router Tx %d bytes", rlen);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
					}
				}

				/* send command to CMOST1 */
				if (rif_pr_message_list[rif_pr_message_list_rd].lun == (STECI_UART_CMOST_LUN | (BROADCAST_LUN & 0x0F)))
				{
					/* send command to CMOST */
					retval = rif_sendCommandTo_CMOST(hTuner1, rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr, rif_pr_message_list[rif_pr_message_list_rd].num_bytes, &cstatus, resp, &rlen);
					if (retval != OSAL_OK)
					{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
						OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router CMOST1 send cmd error %d  cstatus %d", retval, cstatus);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					}
					else
					{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
						OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "rif protocol router Tx %d bytes", rlen);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)

						STECI_UART_SendMessageStart(&rif_SteciUartParams, BROADCAST_LUN, resp,
							0, 0, STECI_UART_CMOST1_I2C_ADDR, 0, rlen, 0, DEV_PROTOCOL_CMOST);
					}
				}

				/* send command to CMOST2 */
				if (rif_pr_message_list[rif_pr_message_list_rd].lun == (STECI_UART_CMOST2_LUN | (BROADCAST_LUN & 0x0F)))
				{
					/* send command to CMOST2 */
					retval = rif_sendCommandTo_CMOST(hTuner2, rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr, rif_pr_message_list[rif_pr_message_list_rd].num_bytes, &cstatus, resp, &rlen);
					if (retval != OSAL_OK)
					{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
						OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router CMOST2 send cmd error %d  cstatus %d", retval, cstatus);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					}
					else
					{
						STECI_UART_SendMessageStart(&rif_SteciUartParams, BROADCAST_LUN, resp,
							0, 0, STECI_UART_CMOST2_I2C_ADDR, 0, rlen, 0, DEV_PROTOCOL_CMOST);
					}
				}

				/* send command to DCOP HD */
				if (rif_pr_message_list[rif_pr_message_list_rd].lun == (STECI_UART_DCOP_HD_LUN | (BROADCAST_LUN & 0x0F)))
				{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					/* send command to HD */
					if(-1 == HDRADIO_Write_Command(rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr[0], &(rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr[1]), (rif_pr_message_list[rif_pr_message_list_rd].num_bytes - 1)))
					{
#endif // #ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
						OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router DCOP HDR send command error");
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					}
					else
					{
						instIdRes = rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr[0];
						retval = HDRADIO_Read_Response(&instIdRes, resp, &srlen);

						if((srlen >= HDRADIO_CP_OVERHEAD) && (retval == OSAL_OK))
						{
							if ((tU32)srlen > sizeof(rif_pr_resp))
							{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
								OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router DCOP HDR response too long error %d", srlen);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
								ASSERT_ON_DEBUGGING(0);
							}
							STECI_UART_SendMessageStart(&rif_SteciUartParams, BROADCAST_LUN, resp,
								0, instIdRes, 0, 0, srlen, 0, DEV_PROTOCOL_HDR);
						}
						else
						{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
							OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router DCOP HDR response error %d", srlen);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
						}
					}
#endif // #ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
				}

				/* send command to RIMW Etal API */
				if ((rif_pr_message_list[rif_pr_message_list_rd].lun == (STECI_UART_MCP_LUN | (BROADCAST_LUN & 0x0F))) ||
					(rif_pr_message_list[rif_pr_message_list_rd].lun == STECI_UART_MCP_LUN ))
				{
					//retval = rimw_ApiCommandParser(rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr, rif_pr_message_list[rif_pr_message_list_rd].num_bytes, resp, &rlen);
					retval = rif_sendMsgTo_rimw(MSG_ID_RIF_RIMW_RX_CMD_PR, rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr, rif_pr_message_list[rif_pr_message_list_rd].num_bytes);
					if (retval != OSAL_OK)
					{
						rif_pr_tracePrintError(TR_CLASS_EXTERNAL, "rif protocol router RIMW send cmd error %d", retval);
					}
					/*else
					{
						STECI_UART_SendMessageStart(&rif_SteciUartParams, BROADCAST_LUN, resp,
							0, 0, 0, 0, rlen, 0, DEV_PROTOCOL_MCP);
					}*/
				}

				/* free received message */
				OSAL_vMemoryFree(rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr);
				rif_pr_message_list[rif_pr_message_list_rd].msg_buf_ptr = NULL;
				rif_pr_message_list[rif_pr_message_list_rd].num_bytes = 0;
				rif_pr_message_list[rif_pr_message_list_rd].lun = INVALID_LUN;

				/* increment read position */
				rif_pr_message_list_rd++;
				if (rif_pr_message_list_rd >= RIF_PR_MSG_NB_MAX)
				{
					rif_pr_message_list_rd = 0;
				}
			}
		}
		// We will rest if we do not have something to do already
		OSAL_s32ThreadWait(1);
	}

	/* delete message queue */
	rif_msg_delete_queue(&rif_pr_msg_queue_p);
}

/***********************************
 *
 * ForwardRxPacketToProtocolRouter
 *
 **********************************/
/*!
 * \brief		Copy message and send it to rif protocol router
 * \details		Copy message and send it to rif protocol router
 * \return		void
 * \callgraph
 * \callergraph
 */
tVoid ForwardRxPacketToProtocolRouter(tU8 lun, tU8 *inDataBuf, tU32 bytesNumInDataBuf)
{
	/* check write overflow */
	if ((rif_pr_message_list_wr == rif_pr_message_list_rd) &&
		(rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr != NULL))
	{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router rif_pr_message_list_rd full, free lun %x len %d adr %x",
			rif_pr_message_list[rif_pr_message_list_wr].lun, rif_pr_message_list[rif_pr_message_list_wr].num_bytes,
			rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)

		OSAL_vMemoryFree(rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr);
		rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr = NULL;
		rif_pr_message_list[rif_pr_message_list_wr].num_bytes = 0;
		rif_pr_message_list[rif_pr_message_list_wr].lun = INVALID_LUN;
		/* increment read position */
		rif_pr_message_list_rd++;
		if (rif_pr_message_list_rd >= RIF_PR_MSG_NB_MAX)
		{
			rif_pr_message_list_rd = 0;
		}
	}

	/* check write position is empty */
	if (rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr != NULL)
	{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router rif_pr_message_list not empty, free lun %02x len %d adr %x",
			rif_pr_message_list[rif_pr_message_list_wr].lun, rif_pr_message_list[rif_pr_message_list_wr].num_bytes,
			rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)

		OSAL_vMemoryFree(rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr);
		rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr = NULL;
		rif_pr_message_list[rif_pr_message_list_wr].num_bytes = 0;
		rif_pr_message_list[rif_pr_message_list_wr].lun = INVALID_LUN;
	}

	/* allocate memory for message buffer */
	if ((rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr = OSAL_pvMemoryAllocate(bytesNumInDataBuf)) == NULL)
	{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif protocol router malloc error");
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
	}
	else
	{
		/* copy lun, bytesNumInDataBuf and inDataBuf */
		OSAL_pvMemoryCopy(rif_pr_message_list[rif_pr_message_list_wr].msg_buf_ptr, inDataBuf, bytesNumInDataBuf);
		rif_pr_message_list[rif_pr_message_list_wr].num_bytes = bytesNumInDataBuf;
		rif_pr_message_list[rif_pr_message_list_wr].lun = lun;

		/* increment write position */
		rif_pr_message_list_wr++;
		if (rif_pr_message_list_wr >= RIF_PR_MSG_NB_MAX)
		{
			rif_pr_message_list_wr = 0;
		}
	}
}

/***************************
 *
 * rif_sendCommandTo_CMOST
 *
 **************************/
/*!
 * \brief		Sends a command to the CMOST
 * \details		The function locks the device, sends the command to the low level driver
 * 				and reads the response.
 * 				- If the response indicates the command was rejected
 * 				due to collision, it sleeps for #ETAL_CMOST_COLLISION_WAIT milliseconds and
 * 				then re sends it. This loop is repeated for at most #ETAL_TO_CMOST_CMD_TIMEOUT_IN_MSEC
 * 				milliseconds, then the function returns with OSAL_ERROR_TIMEOUT_EXPIRED.
 * 				- If the response indicates no error, the function copies it to the
 * 				*resp* (if provided) and returns.
 * 				The *hGeneric* may be a Tuner or Receiver handle; in the latter case the
 * 				function extracts the Tuner handle from the Receiver status.
 *
 * \remark		This function is not normally invoked directly, except
 * 				during system initialization (when Receivers are not yet defined).
 * 				This function is an adaptation of function ETAL_sendCommandTo_CMOST.
 * \param[in]	hGeneric - the Tuner or Receiver handle
 * \param[in]	cmd   - array of bytes containing the command to send; the function
 * 				        does not make any assumption on the content of the array
 * \param[in]	clen  - size in bytes of the *cmd* buffer
 * \param[out]	cstat - pointer to an integer where the function stores a compact
 * 				        code indicating the operation outcome. This code is the MSB of the
 * 				        CMOST response, that is the byte containing the 'Checksum error',
 * 				        'Wrong CID' and 'Collision' bits. For details see
 * 				        TDA7707X_STAR_API_IF.pdf, version 2.35, section 1.2.1.
 * 				        May be NULL, in this case it is ignored.
 * \param[out]	resp  - pointer to a buffer where the function stores the complete CMOST response,
 * 				        or NULL. In the latter case the parameter is ignored. The response
 * 				        is an array of bytes not including the the Command Parameters and including the
 * 				        Checksum bytes. The caller must provide a buffer large enough to hold
 * 				        the largest CMOST answer (see #CMOST_MAX_RESPONSE_LEN).
 * \param[out]	rlen  - pointer to an integer where the function stores the size in bytes
 * 				        of the buffer written to *resp*. If NULL it is ignored.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication with low level driver failed
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *hTuner*
 * \return		OSAL_ERROR_TIMEOUT_EXPIRED - timeout waiting for command to be sent due to collision
 * \return		OSAL_ERROR_DEVICE_INIT - device initialization error (semaphore not available)
 * \return		OSAL_ERROR_FROM_SLAVE  - the CMOST returned an error, the error code is in *cstat*
 * \callgraph
 * \callergraph
 */
tSInt rif_sendCommandTo_CMOST(ETAL_HANDLE hGeneric, tU8 *cmd, tU32 clen, tU16 *cstat, tU8 *resp, tU32 *rlen)
{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	tSInt retval = OSAL_OK;
	OSAL_tMSecond start_time, end_time;
	tU32 len = 0;
	tU8 resp_buf[CMOST_MAX_RESPONSE_LEN]; /* avoid static to protect against task switch corruption */
	ETAL_HANDLE hTuner;
	EtalDeviceDesc deviceDescription;

#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
		OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "%s(0x%x, %p, %d, %p, %p, %p):", __func__, hGeneric, cmd, clen, cstat, resp, rlen);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
		UTIL_TRACE_BUF_COMPONENT(TR_CLASS_EXTERNAL, cmd, clen, "cmd: ");
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

	/* we need a Tuner handle; if we've been passed a Receiver handle
	 * we need to extract the Tuner handle from there
	 */
	if (ETAL_handleIsReceiver(hGeneric))
	{
		if (ETAL_receiverGetTunerId(hGeneric, &hTuner) != OSAL_OK)
		{
			return OSAL_ERROR_INVALID_PARAM;
		}
	}
	else
	{
		hTuner = hGeneric;
	}

	/* this also performs validity check on hTuner */
	if (ETAL_tunerGetAddress(hTuner, &deviceDescription) != OSAL_OK)
	{
		return OSAL_ERROR_INVALID_PARAM;
	}

	if (ETAL_tunerGetLock(hTuner) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR_DEVICE_INIT;
	}

    memset(&resp_buf, 0x00, sizeof(resp_buf));

	start_time = OSAL_ClockGetElapsedTime();
#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
    end_time = start_time + ETAL_TO_CMOST_CMD_TIMEOUT_EXT_IN_MSEC;
#else
	end_time = start_time + ETAL_TO_CMOST_CMD_TIMEOUT_IN_MSEC;
#endif
	while (OSAL_ClockGetElapsedTime() < end_time)
	{
		retval = OSAL_OK;

		if (TUNERDRIVER_sendCommand_CMOST(ETAL_handleTunerGetIndex(hTuner), cmd, clen, resp_buf, &len) != OSAL_OK)
		{
			retval = OSAL_ERROR;
			break;
		}
		
		if (RIF_CMOST_COLLISION_ERROR(resp_buf[0]))
		{
			retval = OSAL_ERROR_TIMEOUT_EXPIRED;
			OSAL_s32ThreadWait(RIF_CMOST_COLLISION_WAIT);
			continue;
		}
		else if (resp_buf[0] != (tU8)0)
		{
			retval = OSAL_ERROR_FROM_SLAVE;
		}
		break;
	}

	ETAL_tunerReleaseLock(hTuner);

	/* resp_buf and len are not initialized in case of
	 * communication error from TUNERDRIVER_sendCommand_CMOST */
	if (retval != OSAL_ERROR)
	{
		if (rlen != NULL)
		{
			*rlen = 0;
		}
		if (cstat != NULL)
		{
			*cstat = (tU16)resp_buf[0];
		}
		if (len > CMOST_MAX_RESPONSE_LEN)
		{
			retval = OSAL_ERROR;
		}
		else if (len >= CMOST_HEADER_LEN)
		{
			if (resp != NULL)
			{
				/*if (len > CMOST_HEADER_LEN + CMOST_CRC_LEN)
				{
					OSAL_pvMemoryCopy((tVoid *)resp, (tPCVoid)(resp_buf), len - CMOST_CRC_LEN);
				}
				else*/
				{
					OSAL_pvMemoryCopy((tVoid *)resp, (tPCVoid)(resp_buf), len);
				}
			}
			if (rlen != NULL)
			{
				/*if (len > CMOST_HEADER_LEN + CMOST_CRC_LEN)
				{
					*rlen = len - CMOST_CRC_LEN;
				}
				else*/
				{
					*rlen = len;
				}
			}
		}
	}

	return retval;
#else
	return OSAL_OK;
#endif
}

/***********************************
 *
 * rif_sendMsgTo_pr
 *
 **********************************/
/*!
 * \brief		send message to pr
 * \details		send message to radio if protocol router
 * \param[in]	msg_id - message ID
 * \param[in]	lun    - Logical Unit Number
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
tSInt rif_sendMsgTo_pr(tU32 msg_id, tU8 lun, tU8 *cmd, tU32 clen)
{
	tU32 msg_header_sz;
	rif_msg_queue_t *msg_queue_p;
	rif_pr_msg_header_t *msg_p;

	rif_rimw_tracePrintComponent(TR_CLASS_EXTERNAL, "%s(%02x, %p, %d)", __FUNCTION__, msg_id, cmd, clen);

	msg_queue_p = rif_pr_msg_queue_p;
	msg_header_sz = sizeof(rif_pr_msg_header_t);

	/* check msg_id */
	if (RIF_GET_MSG_ID_TASK_TO(msg_id) != TASK_ID_RIF_PR)
	{
		rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "%s invalid param error (%02x, %p, %d)", __FUNCTION__, msg_id, cmd, clen);
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR_INVALID_PARAM;
	}

	/* create msg for rif pr */
	msg_p = (rif_pr_msg_header_t *)OSAL_pvMemoryAllocate(msg_header_sz + clen);
	if (msg_p == NULL)
	{
		rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "%s msg alloc error (%02x, %p, %d)", __FUNCTION__, msg_id, cmd, clen);
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR_UNEXPECTED;
	}
	OSAL_pvMemorySet(msg_p, 0, msg_header_sz);
	msg_p->msg_id = msg_id;
	msg_p->lun = lun;

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

