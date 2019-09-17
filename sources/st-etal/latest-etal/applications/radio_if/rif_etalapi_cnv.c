//!
//!  \file    rif_etalapi_cnv.c
//!  \brief   <i><b> Radio interface etal api conversion </b></i>
//!  \details This module implements the conversion etal api with protocol layer command, reponse, notification and datapath.
//!  \author  David Pastor
//!

#include "target_config.h"
#include "target_config_radio_if.h"

#include "osal.h"

#include "etal_api.h"
#include "etaltml_api.h"


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

#include "radio_if.h"
#include "radio_if_util.h"
#include "rif_tasks.h"
#include "rif_msg_queue.h"
#include "rif_etalapi_cnv.h"
#include "rif_rimw_protocol.h"
#include "rif_protocol_router.h"
#include "rif_etaltmlapi_cnv.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
#define RIF_ETALAPI_CNV_ETAL_CMDNUM_MAX     0x4E
enum
{
	CMDNUM_ETAL_INITIALIZE = 0x01,
	CMDNUM_ETAL_PING
};

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/
void rif_etalapi_cnv_CbProcessBlock(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
void rif_etalapi_cnv_CbTextInfo_DataPath(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
void rif_etalapi_cnv_CbEtalRDSData_DataPath(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);

void rif_etalapi_cnv_CbBcastQualityProcess(EtalBcastQualityContainer* pQuality, void* vpContext);

tSInt rif_etalapi_cnv_etal_initialize(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_deinitialize(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_capabilities(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_config_receiver(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_destroy_receiver(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_config_datapath(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_destroy_datapath(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_config_audio_path(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_audio_select(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_mute(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_force_mono(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_reinitialize(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_xtal_alignment(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_version(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_debug_config_audio_alignment(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_init_status(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_tune_receiver(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_receiver_frequency(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_seek_start_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_seek_continue_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_seek_stop_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_seek_get_status_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_change_band_receiver(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_tune_receiver_async(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_current_ensemble(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_service_select_audio(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_service_select_data(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_event_FM_stereo_start(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_event_FM_stereo_stop(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_AF_start(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_AF_end(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_AF_switch(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_AF_check(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_AF_search_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_start_RDS(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_stop_RDS(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_ensemble_list(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_ensemble_data(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_service_list(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_specific_service_data_DAB(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_fic(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_enable_data_service(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_disable_data_service(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_setup_PSD(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_reception_quality(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_channel_quality(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_config_reception_quality_monitor(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_destroy_reception_quality_monitor(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_receiver_alive(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_get_CF_data(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_seamless_estimation_start (tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_seamless_estimation_stop (tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etalapi_cnv_etal_seamless_switching(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);


/*****************************************************************
| Local types
|----------------------------------------------------------------*/
typedef tSInt (*rifEtalApiCnv_EtalCmdFunc) (tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen); 

typedef struct
{
    rifEtalApiCnv_EtalCmdFunc etal_cmd_func;
} rif_etalapi_cnv_etal_cmd_list_Ty;

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
rif_etalapi_cnv_etal_cmd_list_Ty rif_etalapi_cnv_etal_cmd_list[RIF_ETALAPI_CNV_ETAL_CMDNUM_MAX] =
{
	{NULL}, {rif_etalapi_cnv_etal_initialize}, {NULL}, {NULL}, {rif_etalapi_cnv_etal_deinitialize},
	{NULL}, {NULL}, {rif_etalapi_cnv_etal_get_capabilities}, {rif_etalapi_cnv_etal_config_receiver},
	{rif_etalapi_cnv_etal_destroy_receiver}, {rif_etalapi_cnv_etal_config_datapath}, {rif_etalapi_cnv_etal_destroy_datapath}, {rif_etalapi_cnv_etal_config_audio_path},
	{rif_etalapi_cnv_etal_audio_select}, {rif_etalapi_cnv_etal_mute}, {rif_etalapi_cnv_etal_force_mono}, {NULL},
	{rif_etalapi_cnv_etal_reinitialize}, {rif_etalapi_cnv_etal_xtal_alignment}, {rif_etalapi_cnv_etal_get_version}, {rif_etalapi_cnv_etal_get_init_status},
	{rif_etalapi_cnv_etal_tune_receiver}, {rif_etalapi_cnv_etal_get_receiver_frequency}, {rif_etalapi_cnv_etal_seek_start_manual}, {rif_etalapi_cnv_etal_seek_continue_manual},
	{rif_etalapi_cnv_etal_seek_stop_manual}, {NULL}, {NULL}, {NULL},
	{NULL}, {rif_etalapi_cnv_etal_seek_get_status_manual}, {rif_etalapi_cnv_etal_change_band_receiver}, {rif_etalapi_cnv_etal_tune_receiver_async},
	{rif_etalapi_cnv_etal_get_current_ensemble}, {rif_etalapi_cnv_etal_service_select_audio}, {rif_etalapi_cnv_etal_service_select_data}, {NULL},
	{NULL}, {rif_etalapi_cnv_etal_event_FM_stereo_start}, {rif_etalapi_cnv_etal_event_FM_stereo_stop}, {rif_etalapi_cnv_etal_AF_start},
	{rif_etalapi_cnv_etal_AF_end}, {rif_etalapi_cnv_etal_AF_switch}, {rif_etalapi_cnv_etal_AF_search_manual}, {rif_etalapi_cnv_etal_start_RDS},
	{rif_etalapi_cnv_etal_stop_RDS}, {NULL}, {NULL}, {NULL},
	{NULL}, {NULL}, {rif_etalapi_cnv_etal_get_ensemble_list}, {rif_etalapi_cnv_etal_get_ensemble_data},
	{rif_etalapi_cnv_etal_get_service_list}, {rif_etalapi_cnv_etal_get_specific_service_data_DAB}, {NULL}, {NULL},
	{rif_etalapi_cnv_etal_get_fic}, {NULL}, {rif_etalapi_cnv_etal_enable_data_service}, {rif_etalapi_cnv_etal_disable_data_service},
	{NULL}, {rif_etalapi_cnv_etal_setup_PSD}, {rif_etalapi_cnv_etal_get_reception_quality},
	{rif_etalapi_cnv_etal_get_channel_quality}, {rif_etalapi_cnv_etal_config_reception_quality_monitor}, {rif_etalapi_cnv_etal_destroy_reception_quality_monitor}, 
	{rif_etalapi_cnv_etal_get_CF_data},	{rif_etalapi_cnv_etal_seamless_estimation_start}, {rif_etalapi_cnv_etal_seamless_estimation_stop}, {rif_etalapi_cnv_etal_seamless_switching}, 
	{NULL}, {NULL}, {rif_etalapi_cnv_etal_receiver_alive}, {NULL}, {NULL}, {NULL}, {NULL}
};

tU32 *rif_m_CustomParam = NULL;
tU8 *rif_m_DownloadImage =  NULL;

/*****************************************************************
| functions
|----------------------------------------------------------------*/

/***********************************
 *
 * rif_etalapi_cnv_get_cmd_func
 *
 **********************************/
/*!
 * \brief		get command function
 * \details		get command conversion function pointer
 * \param[in]	cmd_num - command number
 * \return		command conversion function pointer
 * \return		NULL - cmmand function not implemented
 * \callgraph
 * \callergraph
 */
rifEtalApiCnv_EtalCmdFunc rif_etalapi_cnv_get_cmd_func(tU16 cmd_num)
{
	/* get command number */
	if (cmd_num < RIF_ETALAPI_CNV_ETAL_CMDNUM_MAX)
	{
		return rif_etalapi_cnv_etal_cmd_list[cmd_num].etal_cmd_func;
	}
	else
	{
		return (rifEtalApiCnv_EtalCmdFunc)NULL;
	}
}

/***********************************
 *
 * rif_etalapi_cnv_checkCmdNumber
 *
 **********************************/
/*!
 * \brief		check command number
 * \details		check that command number is implemented
 * \param[in]	cmd   - buffer of tU8 containing the command to send; the function
 * 				        does not make any assumption on the content of the buffer
 * \param[in]	clen  - size in bytes of the *cmd* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_NOT_IMPLEMENTED - etalapi cmmand not implemented
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_checkCmdNumber(tU8 *cmd, tU32 clen)
{
	tU16 cmd_num;

	/* get command number */
	cmd_num = RIF_RIMW_GET_CMD_NUM(cmd);
	rif_pr_tracePrintComponent(TR_CLASS_EXTERNAL, "cmd_num = %d  len = %d\n", cmd_num, clen);
	if (rif_etalapi_cnv_get_cmd_func(cmd_num) != NULL)
	{
		return OSAL_OK;
	}
	else
	{
		return OSAL_ERROR_NOT_IMPLEMENTED;
	}
}

/***********************************
 *
 * rif_etalapi_cnv_cmd
 *
 **********************************/
/*!
 * \brief		convert command in etal api
 * \details		convert command in etal api calls and return result
 * \param[in]	cmd   - buffer of tU8 containing the command to send; the function
 * 				        does not make any assumption on the content of the buffer
 * \param[in]	clen  - size in bytes of the *cmd* buffer
 * \param[out]	resp  - pointer to a buffer where the function stores the complete RIMW response.
 * \param[out]	rlen  - pointer to an integer where the function stores the size in bytes
 * 				        of the buffer written to *resp*.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_NOT_IMPLEMENTED - command number conversion not implemented
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_cmd(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU16 cmd_num;
	rifEtalApiCnv_EtalCmdFunc etal_cmd_func;

	/* initialize response length to 0 */
	*rlen = 0;
	/* get command number */
	cmd_num = RIF_RIMW_GET_CMD_NUM(cmd);
	rif_pr_tracePrintComponent(TR_CLASS_EXTERNAL, "cmd_num = %d %p\n", cmd_num, rif_etalapi_cnv_get_cmd_func(cmd_num));
	/* get command conversion function */
	if ((etal_cmd_func = rif_etalapi_cnv_get_cmd_func(cmd_num)) == NULL)
	{
		return OSAL_ERROR_NOT_IMPLEMENTED;
	}
	/* call command conversion function to get response */
	etal_cmd_func(cmd, clen, resp, rlen);
	return OSAL_OK;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_payload_size
 *
 **********************************/
/*!
 * \brief		increase rimw payload length
 * \details		increase rimw payload length field and rlen parameter accordingly
 * \param[in/out] resp  - buffer of tU8 containing the response to send; the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - pointer to size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR   - invalid payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_payload_size(tU8 *resp, tU32 *rlen, tU32 size)
{
	rimw_apiCmdTy *resp_p = (rimw_apiCmdTy *)resp;
	tU32 resp_len;

	if ((*rlen <= sizeof(rimw_apiHeaderTy)) && (size >= 1))
	{
		/* add payload length field */
		resp_p->header.header_0.fields.Data  = 1;
		resp_p->header.header_0.fields.Len   = 1;
		resp_p->payload[0] = 0x00;
		resp_p->payload[1] = 0x00;
		*rlen += 2;
	}

	/* check response length is higer or equal to rimw_apiHeaderTy size + 2 bytes of payload length */
	if (*rlen < (sizeof(rimw_apiHeaderTy) + 2))
	{
		return OSAL_ERROR;
	}

	/* get reponse payload length */
	resp_len = (((tU32)resp_p->payload[0] << 8) & 0xFF00) + ((tU32)resp_p->payload[1] & 0xFF);

	/* increase response payload length */
	resp_len += size;

	/* check response payload length is lower than 65536 bytes */
	if ((resp_len > 0xFFFF) || (size > 0xFFFF))
	{
		return OSAL_ERROR;
	}

	/* fill response payload length */
	resp_p->payload[0] = (tU8)((resp_len >> 8) & 0xFF);
	resp_p->payload[1] = (tU8)(resp_len & 0xFF);

	/* icnrease response length */
	*rlen += size;

	return OSAL_OK;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tU8
 *
 **********************************/
/*!
 * \brief		get tU8 from payload
 * \details		get a tU8 from payload and return it. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tU8 returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tU8(tU8 **cmd, tU32 *clen, tU8 *rvalue)
{
	/* check command payload length */
	if (*clen < 1)
	{
		return OSAL_ERROR_INVALID_PARAM;
	}
	*rvalue = ((tU8)(cmd[0][0]) & 0xFF);
	cmd[0] += 1;
	*clen -= 1;
	return OSAL_OK;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tU16
 *
 **********************************/
/*!
 * \brief		get tU16 from payload
 * \details		get a tU16 from payload and return it. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tU16 returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tU16(tU8 **cmd, tU32 *clen, tU16 *rvalue)
{
	/* check command payload length */
	if (*clen < 2)
	{
		return OSAL_ERROR_INVALID_PARAM;
	}
	*rvalue = (((tU16)(cmd[0][0]) & 0xFF) << 8) | ((tU16)(cmd[0][1]) & 0xFF);
	cmd[0] += 2;
	*clen -= 2;
	return OSAL_OK;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tU32
 *
 **********************************/
/*!
 * \brief		get tU32 from payload
 * \details		get a tU32 from command payload and return it. Increase command pointer position and
 * 				decrease command length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tU32 returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tU32(tU8 **cmd, tU32 *clen, tU32 *rvalue)
{
	/* check command payload length */
	if (*clen < 4)
	{
		return OSAL_ERROR_INVALID_PARAM;
	}
	*rvalue = (((tU32)(cmd[0][0]) & 0xFF) << 24) | (((tU32)(cmd[0][1]) & 0xFF) << 16) | (((tU32)(cmd[0][2]) & 0xFF) << 8) | ((tU32)(cmd[0][3]) & 0xFF);
	cmd[0] += 4;
	*clen -= 4;
	return OSAL_OK;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tU8_size
 *
 **********************************/
/*!
 * \brief		get tU8 buffer from payload
 * \details		get a tU8 buffer from command payload and return it. Increase command pointer position and
 * 				decrease command length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tU8 buffer returned.
 * \param[in]	  size   - size in bytes of the returned *rvalue* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter size
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tU8_size(tU8 **cmd, tU32 *clen, tU8 *rvalue, tUInt size)
{
	/* get command bytes */
	while ((*clen > 0) && (size != 0))
	{
		/* get tU8 */
		rvalue[0] = ((tU8)(cmd[0][0]) & 0xFF);
		cmd[0] += 1;
		*clen -= 1;
		size--;
		rvalue++;
	}
	if (size != 0)
	{
		rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_etalapi_cnv_type_get_tU8_size size missing %d", size);
		return OSAL_ERROR_INVALID_PARAM;
	}

	return OSAL_OK;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_param_tU8
 *
 **********************************/
/*!
 * \brief		add tU8 to a payload
 * \details		add a tU8 to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer of tU8 containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - pointer to tU8 value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_param_tU8(tU8 *resp, tU32 *rlen, tU8 *value)
{
	tSInt ret;

	/* icnrease response length and response payload length */
	ret = rif_etalapi_cnv_resp_add_payload_size(resp, rlen, 1);

	if (ret == OSAL_OK)
	{
		/* add tU8 value */
		resp[*rlen - 1] = (tU8)(*value & 0xFF);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_param_tU16
 *
 **********************************/
/*!
 * \brief		add tU16 to a payload
 * \details		add a tU16 to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer of tU8 containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - pointer to tU16 value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_param_tU16(tU8 *resp, tU32 *rlen, tU16 *value)
{
	tSInt ret;

	/* icnrease response length and response payload length */
	ret = rif_etalapi_cnv_resp_add_payload_size(resp, rlen, 2);

	if (ret == OSAL_OK)
	{
		/* add tU32 value to response payload */
		resp[*rlen - 2] = (tU8)((*value >> 8) & 0xFF);
		resp[*rlen - 1] = (tU8)(*value & 0xFF);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_param_tU32
 *
 **********************************/
/*!
 * \brief		add tU32 to a payload
 * \details		add a tU32 to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer of tU8 containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - pointer to tU32 value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_param_tU32(tU8 *resp, tU32 *rlen, tU32 *value)
{
	tSInt ret;

	/* icnrease response length and response payload length */
	ret = rif_etalapi_cnv_resp_add_payload_size(resp, rlen, 4);

	if (ret == OSAL_OK)
	{
		/* add tU32 value to response payload */
		resp[*rlen - 4] = (tU8)((*value >> 24) & 0xFF);
		resp[*rlen - 3] = (tU8)((*value >> 16) & 0xFF);
		resp[*rlen - 2] = (tU8)((*value >> 8) & 0xFF);
		resp[*rlen - 1] = (tU8)(*value & 0xFF);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_param_tU8_size
 *
 **********************************/
/*!
 * \brief		add tU8 buffer to a payload
 * \details		add a tU32 to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer of tU8 containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - pointer to tU32 value to add.
 * \param[in]	  size  - size in bytes of the *value* buffer to add
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_param_tU8_size(tU8 *resp, tU32 *rlen, tU8 *value, tUInt size)
{
	tSInt ret = OSAL_OK;

	/* add response payload bytes */
	while ((size != 0) && (ret == OSAL_OK))
	{
		/* icnrease response length and response payload length */
		ret = rif_etalapi_cnv_resp_add_payload_size(resp, rlen, 1);

		/* add tU8 value to response payload */
		resp[*rlen - 1] = (tU8)(*value & 0xFF);

		/* increase value address and decrease value size */
		value++;
		size--;
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_size
 *
 **********************************/
/*!
 * \brief		get a buffer from a command payload
 * \details		get a buffer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to buffer returned value.
 * \param[in]	  size   - size in bytes of the returned *rvalue* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter size or clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_size(tU8 **cmd, tU32 *clen, tVoid *rvalue, tUInt size)
{
	switch (size)
	{
		case 1:
			return rif_etalapi_cnv_type_get_tU8(cmd, clen, (tU8 *)rvalue);
			break;
		case 2:
			return rif_etalapi_cnv_type_get_tU16(cmd, clen, (tU16 *)rvalue);
			break;
		case 4:
			return rif_etalapi_cnv_type_get_tU32(cmd, clen, (tU32 *)rvalue);
			break;
		default:
			return rif_etalapi_cnv_type_get_tU8_size(cmd, clen, rvalue, size);
			break;
	}
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_param_size
 *
 **********************************/
/*!
 * \brief		add buffer to a payload
 * \details		add a buffer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - pointer to buffer value to add.
 * \param[in]	  size  - size in bytes of the *value* buffer to add
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_param_size(tU8 *resp, tU32 *rlen, tVoid *value, tUInt size)
{
	switch (size)
	{
		case 1:
			return rif_etalapi_cnv_resp_add_param_tU8(resp, rlen, (tU8 *)value);
			break;
		case 2:
			return rif_etalapi_cnv_resp_add_param_tU16(resp, rlen, (tU16 *)value);
			break;
		case 4:
			return rif_etalapi_cnv_resp_add_param_tU32(resp, rlen, (tU32 *)value);
			break;
		default:
			return rif_etalapi_cnv_resp_add_param_tU8_size(resp, rlen, value, size);
			break;
	}
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pointer_type
 *
 **********************************/
/*!
 * \brief		add pointer type to a payload
 * \details		add pointer type to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr - pointer for checking type (NULL / not NULL).
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pointer_type(tU8 *resp, tU32 *rlen, tVoid *ptr)
{
	tU8 value;

	/* add pointer type to response payload */
	if (ptr == NULL)
	{
		value = RIF_ETALAPI_CNV_POINTER_NULL;
	}
	else
	{
		value = RIF_ETALAPI_CNV_POINTER_VALID;
	}

	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(tU8));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_tChar
 *
 **********************************/
/*!
 * \brief		add tChar to a payload
 * \details		add a tChar to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - tChar value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_tChar(tU8 *resp, tU32 *rlen, tChar value)
{
	/* add tChar to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(tChar));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_tBool
 *
 **********************************/
/*!
 * \brief		add tBool to a payload
 * \details		add a tBool to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - tBool value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_tBool(tU8 *resp, tU32 *rlen, tBool value)
{
	/* add tBool to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(tBool));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_tU8
 *
 **********************************/
/*!
 * \brief		add tU8 to a payload
 * \details		add a tU8 to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - tU8 value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_tU8(tU8 *resp, tU32 *rlen, tU8 value)
{
	/* add tU8 to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(tU8));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_tS8
 *
 **********************************/
/*!
 * \brief		add tS8 to a payload
 * \details		add a tS8 to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - tS8 value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_tS8(tU8 *resp, tU32 *rlen, tS8 value)
{
	/* add tS8 to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(tS8));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_tU16
 *
 **********************************/
/*!
 * \brief		add tU16 to a payload
 * \details		add a tU16 to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - tU16 value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_tU16(tU8 *resp, tU32 *rlen, tU16 value)
{
	/* add tU32 to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(tU16));
}


/***********************************
 *
 * rif_etalapi_cnv_resp_add_tU32
 *
 **********************************/
/*!
 * \brief		add tU32 to a payload
 * \details		add a tU32 to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - tU32 value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_tU32(tU8 *resp, tU32 *rlen, tU32 value)
{
	/* add tU32 to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(tU32));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_tS32
 *
 **********************************/
/*!
 * \brief		add tS32 to a payload
 * \details		add a tS32 to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - tS32 value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_tS32(tU8 *resp, tU32 *rlen, tS32 value)
{
	/* add tS32 to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(tS32));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_ptU8
 *
 **********************************/
/*!
 * \brief		add tU8 pointer to a payload
 * \details		add a tU8 pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - tU8 pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_ptU8(tU8 *resp, tU32 *rlen, tU8 *ptr)
{
	tSInt ret = OSAL_OK;

	/* add *tU32 to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if ((ret == OSAL_OK) && (ptr != NULL))
	{
		ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, *ptr);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_ptU16
 *
 **********************************/
/*!
 * \brief		add tU16 pointer to a payload
 * \details		add a tU16 pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - tU16 pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_ptU16(tU8 *resp, tU32 *rlen, tU16 *ptr)
{
	tSInt ret = OSAL_OK;

	/* add *tU32 to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if ((ret == OSAL_OK) && (ptr != NULL))
	{
		ret = rif_etalapi_cnv_resp_add_tU16(resp, rlen, *ptr);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_ptU32
 *
 **********************************/
/*!
 * \brief		add tU32 pointer to a payload
 * \details		add a tU32 pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - tU32 pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_ptU32(tU8 *resp, tU32 *rlen, tU32 *ptr)
{
	tSInt ret = OSAL_OK;

	/* add *tU32 to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if ((ret == OSAL_OK) && (ptr != NULL))
	{
		ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, *ptr);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_ETAL_STATUS
 *
 **********************************/
/*!
 * \brief		add ETAL_STATUS to a payload
 * \details		add a ETAL_STATUS to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - ETAL_STATUS value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_ETAL_STATUS(tU8 *resp, tU32 *rlen, ETAL_STATUS value)
{
	/* add ETAL_STATUS to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(ETAL_STATUS));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_ETAL_HANDLE
 *
 **********************************/
/*!
 * \brief		add ETAL_HANDLE to a payload
 * \details		add a ETAL_HANDLE to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - ETAL_HANDLE value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_ETAL_HANDLE(tU8 *resp, tU32 *rlen, ETAL_HANDLE value)
{
	/* add ETAL_HANDLE to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(ETAL_HANDLE));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalBcastStandard
 *
 **********************************/
/*!
 * \brief		add EtalBcastStandard to a payload
 * \details		add a EtalBcastStandard to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - EtalBcastStandard value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalBcastStandard(tU8 *resp, tU32 *rlen, EtalBcastStandard value)
{
	/* add EtalBcastStandard to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(EtalBcastStandard));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalFrequencyBand
 *
 **********************************/
/*!
 * \brief		add EtalFrequencyBand to a payload
 * \details		add a EtalFrequencyBand to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - EtalFrequencyBand value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalFrequencyBand(tU8 *resp, tU32 *rlen, EtalFrequencyBand value)
{
	/* add EtalBcastStandard to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(EtalFrequencyBand));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalDeviceType
 *
 **********************************/
/*!
 * \brief		add EtalDeviceType to a payload
 * \details		add a EtalDeviceType to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - EtalDeviceType value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalDeviceType(tU8 *resp, tU32 *rlen, EtalDeviceType value)
{
	/* add ETAL_HANDLE to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(EtalDeviceType));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalDeviceBus
 *
 **********************************/
/*!
 * \brief		add EtalDeviceBus to a payload
 * \details		add a EtalDeviceBus to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - EtalDeviceBus value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalDeviceBus(tU8 *resp, tU32 *rlen, EtalDeviceBus value)
{
	/* add ETAL_HANDLE to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(EtalDeviceBus));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalDeviceDesc
 *
 **********************************/
/*!
 * \brief		add EtalDeviceDesc to a payload
 * \details		add a EtalDeviceDesc to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalDeviceDesc pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalDeviceDesc(tU8 *resp, tU32 *rlen, EtalDeviceDesc *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_deviceType */
			ret = rif_etalapi_cnv_resp_add_EtalDeviceType(resp, rlen, ptr->m_deviceType);
		}

		if (ret == OSAL_OK)
		{
			/* add m_busType */
			ret = rif_etalapi_cnv_resp_add_EtalDeviceBus(resp, rlen, ptr->m_busType);
		}

		if (ret == OSAL_OK)
		{
			/* add m_busAddress */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_busAddress);
		}

		if (ret == OSAL_OK)
		{
			/* add m_channels */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_channels);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalTuner
 *
 **********************************/
/*!
 * \brief		add EtalTuner to a payload
 * \details		add a EtalTuner to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalTuner pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalTuner(tU8 *resp, tU32 *rlen, EtalTuner *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_TunerDevice */
			ret = rif_etalapi_cnv_resp_add_EtalDeviceDesc(resp, rlen, &(ptr->m_TunerDevice));
		}

		for(i = 0; i < ETAL_CAPA_MAX_FRONTEND_PER_TUNER; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add m_standards[i] */
				ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_standards[i]);
			}
		}

		for(i = 0; i < ETAL_CAPA_MAX_FRONTEND_PER_TUNER; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add m_dataType[i] */
				ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_dataType[i]);
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalComponentVersion
 *
 **********************************/
/*!
 * \brief		add EtalComponentVersion to a payload
 * \details		add a EtalComponentVersion to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalComponentVersion pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalComponentVersion(tU8 *resp, tU32 *rlen, EtalComponentVersion *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_isValid */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_isValid);
		}

		if (ret == OSAL_OK)
		{
			/* add m_major */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_major);
		}

		if (ret == OSAL_OK)
		{
			/* add m_middle */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_middle);
		}

		if (ret == OSAL_OK)
		{
			/* add m_minor */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_minor);
		}

		if (ret == OSAL_OK)
		{
			/* add m_build */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_build);
		}

		for(i = 0; i < ETAL_VERSION_NAME_MAX; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add m_name[i] */
				ret = rif_etalapi_cnv_resp_add_tChar(resp, rlen, ptr->m_name[i]);
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalInitState
 *
 **********************************/
/*!
 * \brief		add EtalInitState to a payload
 * \details		add a EtalInitState to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - EtalInitState value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalInitState(tU8 *resp, tU32 *rlen, EtalInitState value)
{
	/* add EtalInitState to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(EtalInitState));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalDeviceStatus
 *
 **********************************/
/*!
 * \brief		add EtalDeviceStatus to a payload
 * \details		add a EtalDeviceStatus to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - EtalDeviceStatus value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalDeviceStatus(tU8 *resp, tU32 *rlen, EtalDeviceStatus value)
{
	/* add EtalDeviceStatus to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(EtalDeviceStatus));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalDeviceInitStatus
 *
 **********************************/
/*!
 * \brief		add EtalDeviceInitStatus to a payload
 * \details		add a EtalDeviceInitStatus to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalDeviceInitStatus pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalDeviceInitStatus(tU8 *resp, tU32 *rlen, EtalDeviceInitStatus *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_deviceStatus */
			ret = rif_etalapi_cnv_resp_add_EtalDeviceStatus(resp, rlen, ptr->m_deviceStatus);
		}

		for(i = 0; i < ETAL_SILICON_VERSION_MAX; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add m_expectedSilicon[i] */
				ret = rif_etalapi_cnv_resp_add_tChar(resp, rlen, ptr->m_expectedSilicon[i]);
			}
		}

		for(i = 0; i < ETAL_SILICON_VERSION_MAX; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add m_detectedSilicon[i] */
				ret = rif_etalapi_cnv_resp_add_tChar(resp, rlen, ptr->m_detectedSilicon[i]);
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_etalSeekStatusTy
 *
 **********************************/
/*!
 * \brief		add etalSeekStatusTy to a payload
 * \details		add a etalSeekStatusTy to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - etalSeekStatusTy value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_etalSeekStatusTy(tU8 *resp, tU32 *rlen, etalSeekStatusTy value)
{
	/* add etalSeekStatusTy to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(etalSeekStatusTy));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalDabQualityEntries
 *
 **********************************/
/*!
 * \brief		add EtalDabQualityEntries to a payload
 * \details		add a EtalDabQualityEntries to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalDabQualityEntries pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalDabQualityEntries(tU8 *resp, tU32 *rlen, EtalDabQualityEntries *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_RFFieldStrength */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_RFFieldStrength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_BBFieldStrength */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_BBFieldStrength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_FicBitErrorRatio */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_FicBitErrorRatio);
		}
		if (ret == OSAL_OK)
		{
			/* add m_MscBitErrorRatio */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_MscBitErrorRatio);
		}
		if (ret == OSAL_OK)
		{
			/* add m_isValidMscBitErrorRatio */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_isValidMscBitErrorRatio);
		}
		if (ret == OSAL_OK)
		{
			/* add m_audioSubChBitErrorRatio */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_audioSubChBitErrorRatio);
		}
		if (ret == OSAL_OK)
		{
			/* add m_audioBitErrorRatioLevel */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_audioBitErrorRatioLevel);
		}
		if (ret == OSAL_OK)
		{
			/* add m_reedSolomonInformation */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_reedSolomonInformation);
		}
		if (ret == OSAL_OK)
		{
			/* add m_syncStatus */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_syncStatus);
		}
		if (ret == OSAL_OK)
		{
			/* add m_muteFlag */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_muteFlag);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalFmQualityEntries
 *
 **********************************/
/*!
 * \brief		add EtalFmQualityEntries to a payload
 * \details		add a EtalFmQualityEntries to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalFmQualityEntries pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalFmQualityEntries(tU8 *resp, tU32 *rlen, EtalFmQualityEntries *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_RFFieldStrength */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_RFFieldStrength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_BBFieldStrength */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_BBFieldStrength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_FrequencyOffset */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_FrequencyOffset);
		}
		if (ret == OSAL_OK)
		{
			/* add m_ModulationDetector */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_ModulationDetector);
		}
		if (ret == OSAL_OK)
		{
			/* add m_Multipath */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_Multipath);
		}
		if (ret == OSAL_OK)
		{
			/* add m_UltrasonicNoise */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_UltrasonicNoise);
		}
		if (ret == OSAL_OK)
		{
			/* add m_AdjacentChannel */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_AdjacentChannel);
		}
		if (ret == OSAL_OK)
		{
			/* add m_SNR */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_SNR);
		}
		if (ret == OSAL_OK)
		{
			/* add m_coChannel */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_coChannel);
		}
		if (ret == OSAL_OK)
		{
			/* add m_StereoMonoReception */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_StereoMonoReception);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalHdQualityEntries
 *
 **********************************/
/*!
 * \brief		add EtalHdQualityEntries to a payload
 * \details		add a EtalHdQualityEntries to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalHdQualityEntries pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalHdQualityEntries(tU8 *resp, tU32 *rlen, EtalHdQualityEntries *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_isValidDigital */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_isValidDigital);
		}
		if (ret == OSAL_OK)
		{
			/* add m_QI */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_QI);
		}
		if (ret == OSAL_OK)
		{
			/* add m_CdToNo */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_CdToNo);
		}
		if (ret == OSAL_OK)
		{
			/* add m_DSQM */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_DSQM);
		}
		if (ret == OSAL_OK)
		{
			/* add m_AudioAlignment */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_AudioAlignment);
		}
		if (ret == OSAL_OK)
		{
			/* add m_analogQualityEntries */
			ret = rif_etalapi_cnv_resp_add_EtalFmQualityEntries(resp, rlen, &(ptr->m_analogQualityEntries));
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalBcastQualityContainer
 *
 **********************************/
/*!
 * \brief		add EtalBcastQualityContainer to a payload
 * \details		add a EtalBcastQualityContainer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalBcastQualityContainer pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalBcastQualityContainer(tU8 *resp, tU32 *rlen, EtalBcastQualityContainer *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_TimeStamp */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_TimeStamp);
		}
		if (ret == OSAL_OK)
		{
			/* add m_standard */
			ret = rif_etalapi_cnv_resp_add_EtalBcastStandard(resp, rlen, ptr->m_standard);
		}
		if (ret == OSAL_OK)
		{
			/* add m_Context */
			ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, (tU32 *)ptr->m_Context);
		}
		if (ret == OSAL_OK)
		{
			if (ptr->m_standard == ETAL_BCAST_STD_DAB)
			{
				/* add EtalQualityEntries.dab */
				ret = rif_etalapi_cnv_resp_add_EtalDabQualityEntries(resp, rlen, &(ptr->EtalQualityEntries.dab));
			}
			else if ((ptr->m_standard == ETAL_BCAST_STD_FM) || (ptr->m_standard == ETAL_BCAST_STD_AM))
			{
				/* add EtalQualityEntries.amfm */
				ret = rif_etalapi_cnv_resp_add_EtalFmQualityEntries(resp, rlen, &(ptr->EtalQualityEntries.amfm));
			}
			else if ((ptr->m_standard == ETAL_BCAST_STD_HD_FM) || (ptr->m_standard == ETAL_BCAST_STD_HD) ||
				(ptr->m_standard == ETAL_BCAST_STD_HD_AM))
			{
				/* add EtalQualityEntries.hd */
				ret = rif_etalapi_cnv_resp_add_EtalHdQualityEntries(resp, rlen, &(ptr->EtalQualityEntries.hd));
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalCFDataContainer
 *
 **********************************/
/*!
 * \brief		add EtalCFDataContainer to a payload
 * \details		add a EtalCFDataContainer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalCFDataContainer pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalCFDataContainer(tU8 *resp, tU32 *rlen, EtalCFDataContainer *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_CurrentFrequency */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_CurrentFrequency);
		}
		if (ret == OSAL_OK)
		{
			/* add m_CurrentBand */
			ret = rif_etalapi_cnv_resp_add_EtalFrequencyBand(resp, rlen, ptr->m_CurrentBand);
		}
		if (ret == OSAL_OK)
		{
			/* add m_QualityContainer */
			//ret = rif_etalapi_cnv_resp_add_pEtalBcastQualityContainer(resp, rlen, ptr->m_QualityContainer);
			ret = rif_etalapi_cnv_resp_add_EtalBcastQualityContainer(resp, rlen, &(ptr->m_QualityContainer));
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalBcastQaIndicators
 *
 **********************************/
/*!
 * \brief		add EtalBcastQaIndicators to a payload
 * \details		add a EtalBcastQaIndicators to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  value - EtalBcastQaIndicators value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalBcastQaIndicators(tU8 *resp, tU32 *rlen, EtalBcastQaIndicators value)
{
	/* add etalSeekStatusTy to response payload */
	return rif_etalapi_cnv_resp_add_param_size(resp, rlen, (tVoid *)(&value), sizeof(EtalBcastQaIndicators));
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalQaMonitoredEntryAttr
 *
 **********************************/
/*!
 * \brief		add EtalQaMonitoredEntryAttr to a payload
 * \details		add a EtalQaMonitoredEntryAttr to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalQaMonitoredEntryAttr pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalQaMonitoredEntryAttr(tU8 *resp, tU32 *rlen, EtalQaMonitoredEntryAttr *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_MonitoredIndicator */
			ret = rif_etalapi_cnv_resp_add_EtalBcastQaIndicators(resp, rlen, ptr->m_MonitoredIndicator);
		}
		if (ret == OSAL_OK)
		{
			/* add m_InferiorValue */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_InferiorValue);
		}
		if (ret == OSAL_OK)
		{
			/* add m_SuperiorValue */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_SuperiorValue);
		}
		if (ret == OSAL_OK)
		{
			/* add m_UpdateFrequency */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_UpdateFrequency);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalBcastQualityMonitorAttr
 *
 **********************************/
/*!
 * \brief		add EtalBcastQualityMonitorAttr to a payload
 * \details		add a EtalBcastQualityMonitorAttr to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalBcastQualityMonitorAttr pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalBcastQualityMonitorAttr(tU8 *resp, tU32 *rlen, EtalBcastQualityMonitorAttr *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_receiverHandle */
			ret = rif_etalapi_cnv_resp_add_ETAL_HANDLE(resp, rlen, ptr->m_receiverHandle);
		}
		if (ret == OSAL_OK)
		{
			for(i = 0; i < ETAL_MAX_QUALITY_PER_MONITOR; i++)
			{
				if (ret == OSAL_OK)
				{
					/* add m_monitoredIndicators */
					ret = rif_etalapi_cnv_resp_add_EtalQaMonitoredEntryAttr(resp, rlen, &(ptr->m_monitoredIndicators[i]));
				}
			}
		}
		if (ret == OSAL_OK)
		{
			/* add m_Context */
			ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, (tU32 *)ptr->m_Context);
		}
		if (ret == OSAL_OK)
		{
			/* add m_CbBcastQualityProcess */
			ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, (tU32 *)ptr->m_CbBcastQualityProcess);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalEnsembleDesc
 *
 **********************************/
/*!
 * \brief		add EtalEnsembleDesc to a payload
 * \details		add a EtalEnsembleDesc to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalEnsembleDesc pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalEnsembleDesc(tU8 *resp, tU32 *rlen, EtalEnsembleDesc *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_ECC */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_ECC);
		}
		if (ret == OSAL_OK)
		{
			/* add m_ensembleId */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_ensembleId);
		}
		if (ret == OSAL_OK)
		{
			/* add m_frequency */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_frequency);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalEnsembleList
 *
 **********************************/
/*!
 * \brief		add EtalEnsembleList to a payload
 * \details		add a EtalEnsembleList to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalEnsembleList pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalEnsembleList(tU8 *resp, tU32 *rlen, EtalEnsembleList *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_ensembleCount */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_ensembleCount);
		}
		if (ret == OSAL_OK)
		{
			for(i = 0; i < ETAL_DEF_MAX_ENSEMBLE; i++)
			{
				if (ret == OSAL_OK)
				{
					/* add m_ensemble[i] */
					ret = rif_etalapi_cnv_resp_add_EtalEnsembleDesc(resp, rlen, &(ptr->m_ensemble[i]));
				}
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalServiceList
 *
 **********************************/
/*!
 * \brief		add EtalServiceList to a payload
 * \details		add a EtalServiceList to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalServiceList pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalServiceList(tU8 *resp, tU32 *rlen, EtalServiceList *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_serviceCount */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_serviceCount);
		}
		if (ret == OSAL_OK)
		{
			for(i = 0; i < ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE; i++)
			{
				if (ret == OSAL_OK)
				{
					/* add m_service[i] */
					ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_service[i]);
				}
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalServiceInfo
 *
 **********************************/
/*!
 * \brief		add EtalServiceInfo to a payload
 * \details		add a EtalServiceInfo to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalServiceInfo pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalServiceInfo(tU8 *resp, tU32 *rlen, EtalServiceInfo *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_serviceBitrate */
			ret = rif_etalapi_cnv_resp_add_tU16(resp, rlen, ptr->m_serviceBitrate);
		}
		if (ret == OSAL_OK)
		{
			/* add m_subchId */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_subchId);
		}
		if (ret == OSAL_OK)
		{
			/* add m_packetAddress */
			ret = rif_etalapi_cnv_resp_add_tU16(resp, rlen, ptr->m_packetAddress);
		}
		if (ret == OSAL_OK)
		{
			/* add m_serviceLanguage */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_serviceLanguage);
		}
		if (ret == OSAL_OK)
		{
			/* add m_componentType */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_componentType);
		}
		if (ret == OSAL_OK)
		{
			/* add m_streamType */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_streamType);
		}
		if (ret == OSAL_OK)
		{
			/* add m_scCount */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_scCount);
		}
		if (ret == OSAL_OK)
		{
			/* add m_serviceLabelCharset */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_serviceLabelCharset);
		}
		if (ret == OSAL_OK)
		{
			for(i = 0; i < ETAL_DEF_MAX_LABEL_LEN; i++)
			{
				if (ret == OSAL_OK)
				{
					/* add m_serviceLabel[i] */
					ret = rif_etalapi_cnv_resp_add_tChar(resp, rlen, ptr->m_serviceLabel[i]);
				}
			}
		}
		if (ret == OSAL_OK)
		{
			/* add m_serviceLabelCharflag */
			ret = rif_etalapi_cnv_resp_add_tU16(resp, rlen, ptr->m_serviceLabelCharflag);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalSCInfo
 *
 **********************************/
/*!
 * \brief		add EtalSCInfo to a payload
 * \details		add a EtalSCInfo to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalSCInfo pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalSCInfo(tU8 *resp, tU32 *rlen, EtalSCInfo *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_scIndex */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_scIndex);
		}
		if (ret == OSAL_OK)
		{
			/* add m_dataServiceType */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_dataServiceType);
		}
		if (ret == OSAL_OK)
		{
			/* add m_scType */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_scType);
		}
		if (ret == OSAL_OK)
		{
			/* add m_scLabelCharset */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_scLabelCharset);
		}
		if (ret == OSAL_OK)
		{
			for(i = 0; i < ETAL_DEF_MAX_LABEL_LEN; i++)
			{
				if (ret == OSAL_OK)
				{
					/* add m_scLabel[i] */
					ret = rif_etalapi_cnv_resp_add_tChar(resp, rlen, ptr->m_scLabel[i]);
				}
			}
		}
		if (ret == OSAL_OK)
		{
			/* add m_scLabelCharflag */
			ret = rif_etalapi_cnv_resp_add_tU16(resp, rlen, ptr->m_scLabelCharflag);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalServiceComponentList
 *
 **********************************/
/*!
 * \brief		add EtalServiceComponentList to a payload
 * \details		add a EtalServiceComponentList to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalServiceComponentList pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalServiceComponentList(tU8 *resp, tU32 *rlen, EtalServiceComponentList *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_scCount */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_scCount);
		}
		if (ret == OSAL_OK)
		{
			for(i = 0; i < ETAL_DEF_MAX_SC_PER_SERVICE; i++)
			{
				if (ret == OSAL_OK)
				{
					/* add m_scInfo[i] */
					ret = rif_etalapi_cnv_resp_add_EtalSCInfo(resp, rlen, &(ptr->m_scInfo[i]));
				}
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalPSDLength
 *
 **********************************/
/*!
 * \brief		add EtalPSDLength to a payload
 * \details		add a EtalPSDLength to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalPSDLength pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalPSDLength(tU8 *resp, tU32 *rlen, EtalPSDLength *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_PSDTitleLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDTitleLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDArtistLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDArtistLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDAlbumLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDAlbumLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDGenreLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDGenreLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDCommentShortLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDCommentShortLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDCommentLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDCommentLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDUFIDLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDUFIDLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDCommercialPriceLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDCommercialPriceLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDCommercialContactLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDCommercialContactLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDCommercialSellerLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDCommercialSellerLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDCommercialDescriptionLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDCommercialDescriptionLength);
		}
		if (ret == OSAL_OK)
		{
			/* add m_PSDXHDRLength */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_PSDXHDRLength);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_EtalDataBlockStatus
 *
 **********************************/
/*!
 * \brief		add EtalDataBlockStatus to a payload
 * \details		add a EtalDataBlockStatus to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalDataBlockStatus pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_EtalDataBlockStatus(tU8 *resp, tU32 *rlen, EtalDataBlockStatusTy *ptr)
{
	tSInt ret = OSAL_OK;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_isValid */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_isValid);
		}
		if (ret == OSAL_OK)
		{
			/* add m_validData */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_validData);
		}
		if (ret == OSAL_OK)
		{
			/* add m_continuityError */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_continuityError);
		}
	}

	return ret;
}


/***********************************
 *
 * rif_etalapi_cnv_resp_add_pETAL_HANDLE
 *
 **********************************/
/*!
 * \brief		add ETAL_HANDLE pointer to a payload
 * \details		add a ETAL_HANDLE pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - ETAL_HANDLE pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pETAL_HANDLE(tU8 *resp, tU32 *rlen, ETAL_HANDLE *ptr)
{
	tSInt ret = OSAL_OK;

	/* add *ETAL_HANDLE to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if ((ret == OSAL_OK) && (ptr != NULL))
	{
		ret = rif_etalapi_cnv_resp_add_ETAL_HANDLE(resp, rlen, *ptr);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalReceiverAttr
 *
 **********************************/
/*!
 * \brief		add EtalReceiverAttr pointer to a payload
 * \details		add a EtalReceiverAttr pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalReceiverAttr pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalReceiverAttr(tU8 *resp, tU32 *rlen, EtalReceiverAttr *ptr)
{
	tSInt ret = OSAL_OK, i;

	/* add EtalReceiverAttr * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_Standard */
			ret = rif_etalapi_cnv_resp_add_EtalBcastStandard(resp, rlen, ptr->m_Standard);
		}
		if (ret == OSAL_OK)
		{
			/* add m_FrontEndsSize */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_FrontEndsSize);
		}
		for(i = 0; i < ETAL_CAPA_MAX_FRONTEND; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add m_FrontEnds[i] */
				ret = rif_etalapi_cnv_resp_add_ETAL_HANDLE(resp, rlen, ptr->m_FrontEnds[i]);
			}
		}
		if (ret == OSAL_OK)
		{
			/* add processingFeatures.u.m_processing_features */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->processingFeatures.u.m_processing_features);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalHwCapabilities
 *
 **********************************/
/*!
 * \brief		add EtalHwCapabilities pointer to a payload
 * \details		add a EtalHwCapabilities pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalHwCapabilities pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalHwCapabilities(tU8 *resp, tU32 *rlen, EtalHwCapabilities *ptr)
{
	tSInt ret = OSAL_OK, i;

	/* add EtalHwCapabilities * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_DCOP */
			ret = rif_etalapi_cnv_resp_add_EtalDeviceDesc(resp, rlen, &(ptr->m_DCOP));
		}

		for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add m_Tuner[i] */
				ret = rif_etalapi_cnv_resp_add_EtalTuner(resp, rlen, &(ptr->m_Tuner[i]));
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_ppEtalHwCapabilities
 *
 **********************************/
/*!
 * \brief		add EtalHwCapabilities pointer to pointer to a payload
 * \details		add a EtalHwCapabilities pointer to pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalHwCapabilities pointer to pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_ppEtalHwCapabilities(tU8 *resp, tU32 *rlen, EtalHwCapabilities **ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalHwCapabilities ** to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if ((ret == OSAL_OK) && (ptr != NULL))
	{
		ret = rif_etalapi_cnv_resp_add_pEtalHwCapabilities(resp, rlen, *ptr);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalVersion
 *
 **********************************/
/*!
 * \brief		add EtalVersion pointer to a payload
 * \details		add a EtalVersion pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalVersion pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalVersion(tU8 *resp, tU32 *rlen, EtalVersion *ptr)
{
	tSInt ret = OSAL_OK, i;

	/* add EtalVersion * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_ETAL */
			ret = rif_etalapi_cnv_resp_add_EtalComponentVersion(resp, rlen, &(ptr->m_ETAL));
		}

		if (ret == OSAL_OK)
		{
			/* add m_MDR */
			ret = rif_etalapi_cnv_resp_add_EtalComponentVersion(resp, rlen, &(ptr->m_MDR));
		}

		if (ret == OSAL_OK)
		{
			/* add m_HDRadio */
			ret = rif_etalapi_cnv_resp_add_EtalComponentVersion(resp, rlen, &(ptr->m_HDRadio));
		}

		for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add m_CMOST[i] */
				ret = rif_etalapi_cnv_resp_add_EtalComponentVersion(resp, rlen, &(ptr->m_CMOST[i]));
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalAudioAlignmentAttr
 *
 **********************************/
/*!
 * \brief		add EtalAudioAlignmentAttr pointer to a payload
 * \details		add a EtalAudioAlignmentAttr pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalAudioAlignmentAttr pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalAudioAlignmentAttr(tU8 *resp, tU32 *rlen, EtalAudioAlignmentAttr *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalAudioAlignmentAttr * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_enableAutoAlignmentForFM */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_enableAutoAlignmentForFM);
		}

		if (ret == OSAL_OK)
		{
			/* add m_enableAutoAlignmentForAM */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_enableAutoAlignmentForAM);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalInitStatus
 *
 **********************************/
/*!
 * \brief		add EtalInitStatus pointer to a payload
 * \details		add a EtalInitStatus pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalInitStatus pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalInitStatus(tU8 *resp, tU32 *rlen, EtalInitStatus *ptr)
{
	tSInt ret = OSAL_OK, i;

	/* add EtalInitStatus * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_lastInitState */
			ret = rif_etalapi_cnv_resp_add_EtalInitState(resp, rlen, ptr->m_lastInitState);
		}

		if (ret == OSAL_OK)
		{
			/* add m_warningStatus */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_warningStatus);
		}

		for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add m_tunerStatus[i] */
				ret = rif_etalapi_cnv_resp_add_EtalDeviceInitStatus(resp, rlen, &(ptr->m_tunerStatus[i]));
			}
		}

		if (ret == OSAL_OK)
		{
			/* add m_DCOPStatus */
			ret = rif_etalapi_cnv_resp_add_EtalDeviceInitStatus(resp, rlen, &(ptr->m_DCOPStatus));
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalTuneStatus
 *
 **********************************/
/*!
 * \brief		add EtalTuneStatus pointer to a payload
 * \details		add a EtalTuneStatus pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalTuneStatus pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalTuneStatus(tU8 *resp, tU32 *rlen, EtalTuneStatus *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalTuneStatus * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_receiverHandle */
			ret = rif_etalapi_cnv_resp_add_ETAL_HANDLE(resp, rlen, ptr->m_receiverHandle);
		}
		if (ret == OSAL_OK)
		{
			/* add m_stopFrequency */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_stopFrequency);
		}
		if (ret == OSAL_OK)
		{
			/* add m_sync */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_sync);
		}
		if (ret == OSAL_OK)
		{
			/* add m_serviceId */
			ret = rif_etalapi_cnv_resp_add_tS8(resp, rlen, ptr->m_serviceId);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalSeamlessEstimationStatus
 *
 **********************************/
/*!
 * \brief		add EtalSeamlessEstimationStatus pointer to a payload
 * \details 	add a EtalSeamlessEstimationStatus pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp	- pointer to buffer containing the response payload. the function
 *						  does not make any assumption on the content of the buffer
 * \param[in/out] rlen	- size in bytes of the *resp* buffer
 * \param[in]	  ptr	- EtalSeamlessEstimationStatus pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalSeamlessEstimationStatus(tU8 *resp, tU32 *rlen, EtalSeamlessEstimationStatus *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalSeamlessEstimationStatus * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_receiverHandle */
			ret = rif_etalapi_cnv_resp_add_ETAL_HANDLE(resp, rlen, ptr->m_receiverHandle);
		}
		if (ret == OSAL_OK)
		{
			/* add m_status */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_status);
		}
		if (ret == OSAL_OK)
		{
			/* add m_providerType */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_providerType);
		}
		if (ret == OSAL_OK)
		{
			/* add m_absoluteDelayEstimate */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_absoluteDelayEstimate);
		}
		if (ret == OSAL_OK)
		{
			/* add m_delayEstimate */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_delayEstimate);
		}
		if (ret == OSAL_OK)
		{
			/* add m_timestamp_FAS */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_timestamp_FAS);
		}
		if (ret == OSAL_OK)
		{
			/* add m_timestamp_SAS */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_timestamp_SAS);
		}
		if (ret == OSAL_OK)
		{
			/* add m_RMS2_FAS */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_RMS2_FAS);
		}
		if (ret == OSAL_OK)
		{
			/* add m_RMS2_SAS */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_RMS2_SAS);
		}
		if (ret == OSAL_OK)
		{
			/* add m_confidenceLevel */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_confidenceLevel);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalSeamlessSwitchingStatus
 *
 **********************************/
/*!
 * \brief		add EtalSeamlessSwitchingStatus pointer to a payload
 * \details 	add a EtalSeamlessSwitchingStatus pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp	- pointer to buffer containing the response payload. the function
 *						  does not make any assumption on the content of the buffer
 * \param[in/out] rlen	- size in bytes of the *resp* buffer
 * \param[in]	  ptr	- EtalSeamlessSwitchingStatus pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalSeamlessSwitchingStatus(tU8 *resp, tU32 *rlen, EtalSeamlessSwitchingStatus *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalSeamlessSwitchingStatus * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_receiverHandle */
			ret = rif_etalapi_cnv_resp_add_ETAL_HANDLE(resp, rlen, ptr->m_receiverHandle);
		}
		if (ret == OSAL_OK)
		{
			/* add m_status */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_status);
		}
		if (ret == OSAL_OK)
		{
			/* add m_absoluteDelayEstimate */
			ret = rif_etalapi_cnv_resp_add_tS32(resp, rlen, ptr->m_absoluteDelayEstimate);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalSeekStatus
 *
 **********************************/
/*!
 * \brief		add EtalSeekStatus pointer to a payload
 * \details		add a EtalSeekStatus pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalSeekStatus pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalSeekStatus(tU8 *resp, tU32 *rlen, EtalSeekStatus *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalSeekStatus * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_receiverHandle */
			ret = rif_etalapi_cnv_resp_add_ETAL_HANDLE(resp, rlen, ptr->m_receiverHandle);
		}
		if (ret == OSAL_OK)
		{
			/* add m_status */
			ret = rif_etalapi_cnv_resp_add_etalSeekStatusTy(resp, rlen, ptr->m_status);
		}
		if (ret == OSAL_OK)
		{
			/* add m_frequency */
			ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, ptr->m_frequency);
		}
		if (ret == OSAL_OK)
		{
			/* add m_frequencyFound */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_frequencyFound);
		}
		if (ret == OSAL_OK)
		{
			/* add m_HDProgramFound */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_HDProgramFound);
		}
		if (ret == OSAL_OK)
		{
			/* add m_fullCycleReached */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_fullCycleReached);
		}
		if (ret == OSAL_OK)
		{
			/* add m_serviceId */
			ret = rif_etalapi_cnv_resp_add_tS8(resp, rlen, ptr->m_serviceId);
		}
		if (ret == OSAL_OK)
		{
			/* add m_quality */
			ret = rif_etalapi_cnv_resp_add_EtalBcastQualityContainer(resp, rlen, &(ptr->m_quality));
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalBcastQualityContainer
 *
 **********************************/
/*!
 * \brief		add EtalBcastQualityContainer pointer to a payload
 * \details		add a EtalBcastQualityContainer pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalBcastQualityContainer pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalBcastQualityContainer(tU8 *resp, tU32 *rlen, EtalBcastQualityContainer *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalBcastQualityContainer * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalBcastQualityContainer */
			ret = rif_etalapi_cnv_resp_add_EtalBcastQualityContainer(resp, rlen, ptr);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalCFDataContainer
 *
 **********************************/
/*!
 * \brief		add EtalCFDataContainer pointer to a payload
 * \details		add a EtalCFDataContainer pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalCFDataContainer pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalCFDataContainer(tU8 *resp, tU32 *rlen, EtalCFDataContainer *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalCFDataContainer * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalCFDataContainer */
			ret = rif_etalapi_cnv_resp_add_EtalCFDataContainer(resp, rlen, ptr);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalBcastQualityMonitorAttr
 *
 **********************************/
/*!
 * \brief		add EtalBcastQualityMonitorAttr pointer to a payload
 * \details		add a EtalBcastQualityMonitorAttr pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalBcastQualityMonitorAttr pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalBcastQualityMonitorAttr(tU8 *resp, tU32 *rlen, EtalBcastQualityMonitorAttr *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalBcastQualityMonitorAttr * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalBcastQualityMonitorAttr */
			ret = rif_etalapi_cnv_resp_add_EtalBcastQualityMonitorAttr(resp, rlen, ptr);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalEnsembleList
 *
 **********************************/
/*!
 * \brief		add EtalEnsembleList pointer to a payload
 * \details		add a EtalEnsembleList pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalEnsembleList pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalEnsembleList(tU8 *resp, tU32 *rlen, EtalEnsembleList *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalEnsembleList * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalEnsembleList */
			ret = rif_etalapi_cnv_resp_add_EtalEnsembleList(resp, rlen, ptr);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalServiceList
 *
 **********************************/
/*!
 * \brief		add EtalServiceList pointer to a payload
 * \details		add a EtalServiceList pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalServiceList pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalServiceList(tU8 *resp, tU32 *rlen, EtalServiceList *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalServiceList * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalServiceList */
			ret = rif_etalapi_cnv_resp_add_EtalServiceList(resp, rlen, ptr);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalServiceInfo
 *
 **********************************/
/*!
 * \brief		add EtalServiceInfo pointer to a payload
 * \details		add a EtalServiceInfo pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalServiceInfo pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalServiceInfo(tU8 *resp, tU32 *rlen, EtalServiceInfo *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalServiceInfo * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalServiceInfo */
			ret = rif_etalapi_cnv_resp_add_EtalServiceInfo(resp, rlen, ptr);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalServiceComponentList
 *
 **********************************/
/*!
 * \brief		add EtalServiceComponentList pointer to a payload
 * \details		add a EtalServiceComponentList pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalServiceComponentList pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalServiceComponentList(tU8 *resp, tU32 *rlen, EtalServiceComponentList *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalServiceComponentList * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalServiceComponentList */
			ret = rif_etalapi_cnv_resp_add_EtalServiceComponentList(resp, rlen, ptr);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalPSDLength
 *
 **********************************/
/*!
 * \brief		add EtalPSDLength pointer to a payload
 * \details		add a EtalPSDLength pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalPSDLength pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalPSDLength(tU8 *resp, tU32 *rlen, EtalPSDLength *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalPSDLength * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalPSDLength */
			ret = rif_etalapi_cnv_resp_add_EtalPSDLength(resp, rlen, ptr);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_resp_add_pEtalDataBlockStatus
 *
 **********************************/
/*!
 * \brief		add EtalDataBlockStatus pointer to a payload
 * \details		add a EtalDataBlockStatus pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalDataBlockStatus pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_resp_add_pEtalDataBlockStatus(tU8 *resp, tU32 *rlen, EtalDataBlockStatusTy *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalDataBlockStatus * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalDataBlockStatus */
			ret = rif_etalapi_cnv_resp_add_EtalDataBlockStatus(resp, rlen, ptr);
		}
	}

	return ret;
}


/***********************************
 *
 * rif_etalapi_cnv_create_resp
 *
 **********************************/
/*!
 * \brief		create response header
 * \details		create a reponse header with no payload. Increase response length.
 * \param[in]	  cmd   - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  status- rimw response status.
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_create_resp(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen, tU8 status)
{
	rimw_apiCmdTy *resp_p = (rimw_apiCmdTy *)resp;
	tU16 cmdNum;
	tSInt ret = OSAL_OK;

	/* fill response header */
	resp_p->header.header_0.fields.Stop  = 0;
	resp_p->header.header_0.fields.Data  = 0;
	resp_p->header.header_0.fields.Fast  = 0;
	resp_p->header.header_0.fields.Len   = 0;
	resp_p->header.header_0.fields.Sto   = 1;
	resp_p->header.header_0.fields.Auto  = 0;
	resp_p->header.header_0.fields.Reply = 1;
	resp_p->header.header_0.fields.Host  = 0;

	/* set response status code */
	resp_p->header.header_1.fields.specific_status = (status & RIF_RIMW_STATUS_MASK);

	/* set command response number */
	cmdNum = RIF_RIMW_GET_CMD_NUM(cmd);
	resp_p->header.header_1.fields.cmdNumM = (tU8)((cmdNum >> 8) & RIF_RIMW_H1_CMDNUMM_MASK);
	resp_p->header.header_2.fields.cmdNumL = (tU8)(cmdNum & RIF_RIMW_H2_CMDNUML_MASK);

	/* set response length to rimw_apiHeaderTy size */
	*rlen = sizeof(rimw_apiHeaderTy);

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_create_auto_resp
 *
 **********************************/
/*!
 * \brief		create auto response header
 * \details		create an auto reponse header with no payload. Increase response length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  status- rimw response status.
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_create_auto_resp(tU8 *resp, tU32 *rlen, tU8 notificationNum)
{
	rimw_apiCmdTy *resp_p = (rimw_apiCmdTy *)resp;
	tSInt ret = OSAL_OK;

	/* fill auto notification response header */
	resp_p->header.header_0.fields.Stop  = 0;
	resp_p->header.header_0.fields.Data  = 0;
	resp_p->header.header_0.fields.Fast  = 0;
	resp_p->header.header_0.fields.Len   = 0;
	resp_p->header.header_0.fields.Sto   = 1;
	resp_p->header.header_0.fields.Auto  = 1;
	resp_p->header.header_0.fields.Reply = 1;
	resp_p->header.header_0.fields.Host  = 0;

	/* set header 1 */
	resp_p->header.header_1.value = 0;

	/* set auto notification response number */
	resp_p->header.header_2.value = notificationNum;

	/* set response length to rimw_apiHeaderTy size */
	*rlen = sizeof(rimw_apiHeaderTy);

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_create_auto_notif
 *
 **********************************/
/*!
 * \brief		create auto notification header
 * \details		create an auto notification header with no payload. Increase response length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  status- rimw response status.
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_create_auto_notif(tU8 *resp, tU32 *rlen, tU8 notifEvent, tU8 notificationNum)
{
	rimw_apiCmdTy *resp_p = (rimw_apiCmdTy *)resp;
	tSInt ret = OSAL_OK;

	/* fill auto notification response header */
	resp_p->header.header_0.fields.Stop  = 0;
	resp_p->header.header_0.fields.Data  = 1;
	resp_p->header.header_0.fields.Fast  = 0;
	resp_p->header.header_0.fields.Len	 = 0;
	resp_p->header.header_0.fields.Sto	 = 1;
	resp_p->header.header_0.fields.Auto  = 1;
	resp_p->header.header_0.fields.Reply = 1;
	resp_p->header.header_0.fields.Host  = 1;

	/* set header 1 */
	resp_p->header.header_1.value = notifEvent;

	/* set auto notification response number */
	resp_p->header.header_2.value = notificationNum;

	/* set response length to rimw_apiHeaderTy size */
	*rlen = sizeof(rimw_apiHeaderTy);

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_create_data_path
 *
 **********************************/
/*!
 * \brief		create data path header
 * \details		create an auto reponse header with no payload. Increase response length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  status- rimw response status.
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_create_data_path(tU8 *resp, tU32 *rlen, tU8 notifEvent, tU8 notificationNum, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	rimw_apiCmdTy *resp_p = (rimw_apiCmdTy *)resp;
	tSInt ret = OSAL_OK;

	/* fill auto notification response header */
	resp_p->header.header_0.fields.Stop  = 0;
	resp_p->header.header_0.fields.Data  = 1;
	resp_p->header.header_0.fields.Fast  = 0;
	resp_p->header.header_0.fields.Len	 = 0;
	resp_p->header.header_0.fields.Sto	 = 1;
	resp_p->header.header_0.fields.Auto  = 1;
	resp_p->header.header_0.fields.Reply = 1;
	resp_p->header.header_0.fields.Host  = 1;

	/* set header 1 */
	resp_p->header.header_1.value = notifEvent;

	/* set auto notification response number */
	resp_p->header.header_2.value = notificationNum;

	/* set response length to rimw_apiHeaderTy size */
	*rlen = sizeof(rimw_apiHeaderTy);

	/* add dwActualBufferSize */
	ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, dwActualBufferSize);	

	if (ret == OSAL_OK)
	{
		/* add status to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalDataBlockStatus(resp, rlen, status);
	}

	if (ret == OSAL_OK)
	{
		/* add pvContext to response payload */
		ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)pvContext);
		if (pvContext != NULL)
		{
			if (ret == OSAL_OK)
			{
				/* add pvContext */
				ret = rif_etalapi_cnv_resp_add_tU32(resp, rlen, (tU32)pvContext);
			}
		}
	}
	return ret;
}


/***********************************
 *
 * rif_etalapi_cnv_type_get_pointer_type
 *
 **********************************/
/*!
 * \brief		get pointer type from a command payload
 * \details		get pointer type from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer type returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pointer_type(tU8 **cmd, tU32 *clen, rif_etalapi_cnv_pointer_type_enum *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, 1);
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalCountryVariant
 *
 **********************************/
/*!
 * \brief		get EtalCountryVariant from a command payload
 * \details		get EtalCountryVariant from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalCountryVariant returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalCountryVariant(tU8 **cmd, tU32 *clen, EtalCountryVariant *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalCountryVariant));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tBool
 *
 **********************************/
/*!
 * \brief		get tBool from a command payload
 * \details		get tBool from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tBool returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tBool(tU8 **cmd, tU32 *clen, tBool *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(tBool));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tS8
 *
 **********************************/
/*!
 * \brief		get tS8 from a command payload
 * \details		get tS8 from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tBool returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tS8(tU8 **cmd, tU32 *clen, tS8 *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(tS8));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tChar
 *
 **********************************/
/*!
 * \brief		get tChar from a command payload
 * \details		get tChar from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tChar returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tChar(tU8 **cmd, tU32 *clen, tChar *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(tChar));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tS32
 *
 **********************************/
/*!
 * \brief		get tS32 from a command payload
 * \details		get tS32 from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tBool returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tS32(tU8 **cmd, tU32 *clen, tS32 *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(tS32));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tSInt
 *
 **********************************/
/*!
 * \brief		get tSInt from a command payload
 * \details		get tSInt from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tBool returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tSInt(tU8 **cmd, tU32 *clen, tSInt *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(tSInt));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_tPVoid
 *
 **********************************/
/*!
 * \brief		get void * from a command payload
 * \details		get void * from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to void * returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_tPVoid(tU8 **cmd, tU32 *clen, tPVoid *rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get tPVoid */
			ret = rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, /* sizeof(tPVoid) doesn't work on 64bits CPU use 4 instead */ 4);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalCbGetImage
 *
 **********************************/
/*!
 * \brief		get EtalCbGetImage from a command payload
 * \details		get EtalCbGetImage from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalCbGetImage returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalCbGetImage(tU8 **cmd, tU32 *clen, EtalCbGetImage *rvalue)
{
	return rif_etalapi_cnv_type_get_tPVoid(cmd, clen, (tPVoid *)rvalue);
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalCbPutImage
 *
 **********************************/
/*!
 * \brief		get EtalCbPutImage from a command payload
 * \details		get EtalCbPutImage from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalCbPutImage returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalCbPutImage(tU8 **cmd, tU32 *clen, EtalCbPutImage *rvalue)
{
	return rif_etalapi_cnv_type_get_tPVoid(cmd, clen, (tPVoid *)rvalue);
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalCbNotify
 *
 **********************************/
/*!
 * \brief		get EtalCbNotify from a command payload
 * \details		get EtalCbNotify from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalCbNotify returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalCbNotify(tU8 **cmd, tU32 *clen, EtalCbNotify *rvalue)
{
	return rif_etalapi_cnv_type_get_tPVoid(cmd, clen, (tPVoid *)rvalue);
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalCbProcessBlock
 *
 **********************************/
/*!
 * \brief		get EtalCbProcessBlock from a command payload
 * \details		get EtalCbProcessBlock from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalCbProcessBlock returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalCbProcessBlock(tU8 **cmd, tU32 *clen, EtalCbProcessBlock *rvalue, EtalBcastDataType cmdType)
{
	tSInt ret;

	ret = rif_etalapi_cnv_type_get_tPVoid(cmd, clen, (tPVoid *)rvalue);
	/* set m_CbProcessBloc */
	if (rvalue != NULL)
	{	
		switch(cmdType){
			case ETAL_DATA_TYPE_AUDIO:
				// TODO
				break;
			case ETAL_DATA_TYPE_DCOP_AUDIO:
				// TODO
				break;
			case ETAL_DATA_TYPE_DATA_SERVICE:
				// TODO
				break;
			case ETAL_DATA_TYPE_DAB_DATA_RAW:
				// TODO
				break;
			case ETAL_DATA_TYPE_DAB_AUDIO_RAW:
				// TODO
				break;
			case ETAL_DATA_TYPE_DAB_FIC:
				// TODO
				break;
			case ETAL_DATA_TYPE_TEXTINFO:
				*rvalue = rif_etalapi_cnv_CbTextInfo_DataPath;
				break;
			case ETAL_DATA_TYPE_FM_RDS:
				*rvalue = rif_etalapi_cnv_CbEtalRDSData_DataPath;
				break;
			case ETAL_DATA_TYPE_FM_RDS_RAW:
				//TODO
				break;
			default:
				*rvalue = rif_etalapi_cnv_CbProcessBlock;
				break;
		}
	}
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalDCOPAttr
 *
 **********************************/
/*!
 * \brief		get EtalDCOPAttr from a command payload
 * \details		get EtalDCOPAttr from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalDCOPAttr returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalDCOPAttr(tU8 **cmd, tU32 *clen, EtalDCOPAttr *rvalue)
{
	tSInt ret;

	/* get m_isDisabled */
	ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_isDisabled));
	if (ret == OSAL_OK)
	{
		/* get m_doFlashDump */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_doFlashDump));
	}
	if (ret == OSAL_OK)
	{
		/* get m_doFlashProgram */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_doFlashProgram));
	}
	if (ret == OSAL_OK)
	{
		/* get m_doDownload */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_doDownload));
	}
	if (ret == OSAL_OK)
	{
		/* get m_cbGetImage */
		ret = rif_etalapi_cnv_type_get_EtalCbGetImage(cmd, clen, &(rvalue->m_cbGetImage));
	}
	if (ret == OSAL_OK)
	{
		/* get m_pvGetImageContext */
		ret = rif_etalapi_cnv_type_get_tPVoid(cmd, clen, &(rvalue->m_pvGetImageContext));
	}
	if (ret == OSAL_OK)
	{
		/* get m_cbPutImage */
		ret = rif_etalapi_cnv_type_get_EtalCbPutImage(cmd, clen, &(rvalue->m_cbPutImage));
	}
	if (ret == OSAL_OK)
	{
		/* get m_pvPutImageContext */
		ret = rif_etalapi_cnv_type_get_tPVoid(cmd, clen, &(rvalue->m_pvPutImageContext));
	}
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
	if (ret == OSAL_OK)
	{
		/* get m_sectDescrFilename */
		ret = rif_etalapi_cnv_type_get_ptU8(cmd, clen, &(rvalue->m_sectDescrFilename));
	}
#endif
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalTunerAttr
 *
 **********************************/
/*!
 * \brief		get EtalTunerAttr from a command payload
 * \details		get EtalTunerAttr from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalTunerAttr returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalTunerAttr(tU8 **cmd, tU32 *clen, EtalTunerAttr *rvalue)
{
	tSInt ret;
	tU32 i;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get m_isDisabled */
	ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_isDisabled));
	if (ret == OSAL_OK)
	{
		/* get m_useXTALalignment */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_useXTALalignment));
	}
	if (ret == OSAL_OK)
	{
		/* get m_XTALalignment */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_XTALalignment));
	}
	if (ret == OSAL_OK)
	{
		/* get m_useCustomParam */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_useCustomParam));
	}
	if (ret == OSAL_OK)
	{
		/* get m_CustomParamSize */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_CustomParamSize));
	}
	if (ret == OSAL_OK)
	{
		/* get pointer type */
		ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	}
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			rvalue->m_CustomParam = NULL;
			if (rif_m_CustomParam != NULL)
			{
				OSAL_vMemoryFree(rif_m_CustomParam);
				rif_m_CustomParam = NULL;
			}
		}
		else if ((pointer_type == RIF_ETALAPI_CNV_POINTER_VALID) && (rvalue->m_CustomParamSize != 0))
		{
			/* alloc m_CustomParam */
			if (rif_m_CustomParam != NULL)
			{
				OSAL_vMemoryFree(rif_m_CustomParam);
				rif_m_CustomParam = NULL;
			}
			rif_m_CustomParam = OSAL_pvMemoryAllocate(rvalue->m_CustomParamSize * sizeof(tU32) * 2);
			if (rif_m_CustomParam == NULL)
			{
				ret = OSAL_ERROR_UNEXPECTED;
			}
			else
			{
				rvalue->m_CustomParam = rif_m_CustomParam;
			}
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}
	if ((ret == OSAL_OK) && (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID))
	{
		for(i = 0; i < rvalue->m_CustomParamSize; i++)
		{
			if (ret == OSAL_OK)
			{
				/* get m_CustomParam address, value */
				ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_CustomParam[(2 * i)]));
				ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_CustomParam[(2 * i) + 1]));
			}
		}
	}
	if (ret == OSAL_OK)
	{
		/* get m_useDownloadImage */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_useDownloadImage));
	}
	if (ret == OSAL_OK)
	{
		/* get m_DownloadImageSize */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_DownloadImageSize));
	}
	if (ret == OSAL_OK)
	{
		/* get pointer type */
		ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	}
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			rvalue->m_DownloadImage = NULL;
			if (rif_m_DownloadImage != NULL)
			{
				OSAL_vMemoryFree(rif_m_DownloadImage);
				rif_m_DownloadImage = NULL;
			}
		}
		else if ((pointer_type == RIF_ETALAPI_CNV_POINTER_VALID) && (rvalue->m_DownloadImageSize != 0))
		{
			/* alloc m_DownloadImage */
			if (rif_m_DownloadImage != NULL)
			{
				OSAL_vMemoryFree(rif_m_DownloadImage);
				rif_m_DownloadImage = NULL;
			}
			rif_m_DownloadImage = OSAL_pvMemoryAllocate(rvalue->m_DownloadImageSize * sizeof(tU8));
			if (rif_m_DownloadImage == NULL)
			{
				ret = OSAL_ERROR_UNEXPECTED;
			}
			else
			{
				rvalue->m_DownloadImage = rif_m_DownloadImage;
			}
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}
	if ((ret == OSAL_OK) && (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID))
	{
		for(i = 0; i < rvalue->m_DownloadImageSize; i++)
		{
			if (ret == OSAL_OK)
			{
				/* get m_DownloadImage */
				ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_DownloadImage[i]));
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalTraceLevel
 *
 **********************************/
/*!
 * \brief		get EtalTraceLevel from a command payload
 * \details		get EtalTraceLevel from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalTraceLevel returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalTraceLevel(tU8 **cmd, tU32 *clen, EtalTraceLevel *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalTraceLevel));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalTraceConfig
 *
 **********************************/
/*!
 * \brief		get EtalTraceConfig from a command payload
 * \details		get EtalTraceConfig from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalTraceConfig returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalTraceConfig(tU8 **cmd, tU32 *clen, EtalTraceConfig *rvalue)
{
	tSInt ret;

	/* get m_disableHeader */
	ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_disableHeader));
	if (ret == OSAL_OK)
	{
		/* get m_disableHeaderUsed */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_disableHeaderUsed));
	}
	if (ret == OSAL_OK)
	{
		/* get m_defaultLevel */
		ret = rif_etalapi_cnv_type_get_EtalTraceLevel(cmd, clen, &(rvalue->m_defaultLevel));
	}
	if (ret == OSAL_OK)
	{
		/* get m_defaultLevelUsed */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_defaultLevelUsed));
	}
	if (ret == OSAL_OK)
	{
		/* get m_reserved */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_reserved));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalNVMLoadConfig
 *
 **********************************/
/*!
 * \brief       get EtalNVMLoadConfig from a command payload
 * \details     get EtalNVMLoadConfig from a command payload. Increase command payload pointer position and
 *              decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 *                         does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]    rvalue - pointer to EtalSeamlessSwitchingConfigTy returned value.
 * \return      OSAL_OK
 * \return      OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalNVMLoadConfig(tU8 **cmd, tU32 *clen, EtalNVMLoadConfig *rvalue)
{
    return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalNVMLoadConfig));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalHardwareAttr
 *
 **********************************/
/*!
 * \brief		get EtalHardwareAttr from a command payload
 * \details		get EtalHardwareAttr from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalHardwareAttr returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalHardwareAttr(tU8 **cmd, tU32 *clen, EtalHardwareAttr *rvalue)
{
	tSInt ret, i;

	/* get m_CountryVariant */
	ret = rif_etalapi_cnv_type_get_EtalCountryVariant(cmd, clen, &(rvalue->m_CountryVariant));
	if (ret == OSAL_OK)
	{
		/* get m_DCOPAttr */
		ret = rif_etalapi_cnv_type_get_EtalDCOPAttr(cmd, clen, &(rvalue->m_DCOPAttr));
	}
	for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if (ret == OSAL_OK)
		{
			/* get m_tunerAttr */
			ret = rif_etalapi_cnv_type_get_EtalTunerAttr(cmd, clen, &(rvalue->m_tunerAttr[i]));
		}
	}
	if (ret == OSAL_OK)
	{
		/* get m_cbNotify */
		ret = rif_etalapi_cnv_type_get_EtalCbNotify(cmd, clen, &(rvalue->m_cbNotify));
	}
	if (ret == OSAL_OK)
	{
		/* get m_context */
		ret = rif_etalapi_cnv_type_get_tPVoid(cmd, clen, &(rvalue->m_context));
	}
	if (ret == OSAL_OK)
	{
		/* get m_traceConfig */
		ret = rif_etalapi_cnv_type_get_EtalTraceConfig(cmd, clen, &(rvalue->m_traceConfig));
	}
	if (ret == OSAL_OK)
	{
		/* get m_NVMLoadConfig */
		ret = rif_etalapi_cnv_type_get_EtalNVMLoadConfig(cmd, clen, &(rvalue->m_NVMLoadConfig));
	}
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_ETAL_HANDLE
 *
 **********************************/
/*!
 * \brief		get ETAL_HANDLE from a command payload
 * \details		get ETAL_HANDLE from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to ETAL_HANDLE returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_ETAL_HANDLE(tU8 **cmd, tU32 *clen, ETAL_HANDLE *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(ETAL_HANDLE));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalBcastStandard
 *
 **********************************/
/*!
 * \brief		get EtalBcastStandard from a command payload
 * \details		get EtalBcastStandard from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalBcastStandard returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalBcastStandard(tU8 **cmd, tU32 *clen, EtalBcastStandard *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalBcastStandard));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalReceiverAttr
 *
 **********************************/
/*!
 * \brief		get EtalReceiverAttr from a command payload
 * \details		get EtalReceiverAttr from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalReceiverAttr returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalReceiverAttr(tU8 **cmd, tU32 *clen, EtalReceiverAttr *rvalue)
{
	tSInt ret, i;

	/* get m_Standard */
	ret = rif_etalapi_cnv_type_get_EtalBcastStandard(cmd, clen, &(rvalue->m_Standard));
	if (ret == OSAL_OK)
	{
		/* get m_FrontEndsSize */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_FrontEndsSize));
	}
	for(i = 0; i < ETAL_CAPA_MAX_FRONTEND; i++)
	{
		if (ret == OSAL_OK)
		{
			/* get m_FrontEnds */
			ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(cmd, clen, &(rvalue->m_FrontEnds[i]));
		}
	}
	if (ret == OSAL_OK)
	{
		/* get processingFeatures.u.m_processing_features */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->processingFeatures.u.m_processing_features));
	}
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalBcastDataType
 *
 **********************************/
/*!
 * \brief		get EtalBcastDataType from a command payload
 * \details		get EtalBcastDataType from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalBcastDataType returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalBcastDataType(tU8 **cmd, tU32 *clen, EtalBcastDataType *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalBcastDataType));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalSink
 *
 **********************************/
/*!
 * \brief		get EtalSink from a command payload
 * \details		get EtalSink from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalSink returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalSink(tU8 **cmd, tU32 *clen, EtalSink *rvalue, EtalBcastDataType cmdType)
{
	tSInt ret;

	/* get m_context */
	ret = rif_etalapi_cnv_type_get_tPVoid(cmd, clen, &(rvalue->m_context));
	if (ret == OSAL_OK)
	{
		/* get m_BufferSize */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_BufferSize));
	}
	if (ret == OSAL_OK)
	{
		/* m_CbProcessBlock */
		ret = rif_etalapi_cnv_type_get_EtalCbProcessBlock(cmd, clen, &(rvalue->m_CbProcessBlock), cmdType);
	}
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalDataPathAttr
 *
 **********************************/
/*!
 * \brief		get EtalDataPathAttr from a command payload
 * \details		get EtalDataPathAttr from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalDataPathAttr returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalDataPathAttr(tU8 **cmd, tU32 *clen, EtalDataPathAttr *rvalue)
{
	tSInt ret;

	/* get m_receiverHandle */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(cmd, clen, &(rvalue->m_receiverHandle));
	if (ret == OSAL_OK)
	{
		/* get m_dataType */
		ret = rif_etalapi_cnv_type_get_EtalBcastDataType(cmd, clen, &(rvalue->m_dataType));
	}
	if (ret == OSAL_OK)
	{
		/* get m_sink */
		ret = rif_etalapi_cnv_type_get_EtalSink(cmd, clen, &(rvalue->m_sink), rvalue->m_dataType);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalAudioInterfTy
 *
 **********************************/
/*!
 * \brief		get EtalAudioInterfTy from a command payload
 * \details		get EtalAudioInterfTy from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalAudioInterfTy returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalAudioInterfTy(tU8 **cmd, tU32 *clen, EtalAudioInterfTy *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalAudioInterfTy));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalAudioSourceTy
 *
 **********************************/
/*!
 * \brief		get EtalAudioSourceTy from a command payload
 * \details		get EtalAudioSourceTy from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalAudioSourceTy returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalAudioSourceTy(tU8 **cmd, tU32 *clen, EtalAudioSourceTy *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalAudioSourceTy));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalComponentVersion
 *
 **********************************/
/*!
 * \brief		get EtalComponentVersion from a command payload
 * \details		get EtalComponentVersion from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalComponentVersion returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalComponentVersion(tU8 **cmd, tU32 *clen, EtalComponentVersion *rvalue)
{
	tSInt ret, i;

	/* get m_isValid */
	ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_isValid));
	if (ret == OSAL_OK)
	{
		/* get m_major */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_major));
	}
	if (ret == OSAL_OK)
	{
		/* get m_middle */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_middle));
	}
	if (ret == OSAL_OK)
	{
		/* get m_minor */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_minor));
	}
	if (ret == OSAL_OK)
	{
		/* get m_build */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_build));
	}
	for(i = 0; i < ETAL_VERSION_NAME_MAX; i++)
	{
		if (ret == OSAL_OK)
		{
			/* get m_name */
			ret = rif_etalapi_cnv_type_get_tChar(cmd, clen, &(rvalue->m_name[i]));
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalVersion
 *
 **********************************/
/*!
 * \brief		get EtalVersion from a command payload
 * \details		get EtalVersion from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalVersion returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalVersion(tU8 **cmd, tU32 *clen, EtalVersion *rvalue)
{
	tSInt ret, i;

	/* get m_ETAL */
	ret = rif_etalapi_cnv_type_get_EtalComponentVersion(cmd, clen, &(rvalue->m_ETAL));
	if (ret == OSAL_OK)
	{
		/* get m_MDR */
		ret = rif_etalapi_cnv_type_get_EtalComponentVersion(cmd, clen, &(rvalue->m_MDR));
	}
	if (ret == OSAL_OK)
	{
		/* get m_HDRadio */
		ret = rif_etalapi_cnv_type_get_EtalComponentVersion(cmd, clen, &(rvalue->m_HDRadio));
	}
	for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if (ret == OSAL_OK)
		{
			/* get m_CMOST */
			ret = rif_etalapi_cnv_type_get_EtalComponentVersion(cmd, clen, &(rvalue->m_CMOST[i]));
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalAudioAlignmentAttr
 *
 **********************************/
/*!
 * \brief		get EtalAudioAlignmentAttr from a command payload
 * \details		get EtalAudioAlignmentAttr from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalAudioAlignmentAttr returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalAudioAlignmentAttr(tU8 **cmd, tU32 *clen, EtalAudioAlignmentAttr *rvalue)
{
	tSInt ret;

	/* get m_enableAutoAlignmentForFM */
	ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_enableAutoAlignmentForFM));
	if (ret == OSAL_OK)
	{
		/* get m_enableAutoAlignmentForAM */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_enableAutoAlignmentForAM));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalInitState
 *
 **********************************/
/*!
 * \brief		get EtalInitState from a command payload
 * \details		get EtalInitState from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalInitState returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalInitState(tU8 **cmd, tU32 *clen, EtalInitState *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalInitState));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalDeviceStatus
 *
 **********************************/
/*!
 * \brief		get EtalDeviceStatus from a command payload
 * \details		get EtalDeviceStatus from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalDeviceStatus returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalDeviceStatus(tU8 **cmd, tU32 *clen, EtalDeviceStatus *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalDeviceStatus));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalDeviceInitStatus
 *
 **********************************/
/*!
 * \brief		get EtalDeviceInitStatus from a command payload
 * \details		get EtalDeviceInitStatus from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalDeviceInitStatus returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalDeviceInitStatus(tU8 **cmd, tU32 *clen, EtalDeviceInitStatus *rvalue)
{
	tSInt ret, i;

	/* get m_deviceStatus */
	ret = rif_etalapi_cnv_type_get_EtalDeviceStatus(cmd, clen, &(rvalue->m_deviceStatus));
	for(i = 0; i < ETAL_SILICON_VERSION_MAX; i++)
	{
		if (ret == OSAL_OK)
		{
			/* get m_expectedSilicon */
			ret = rif_etalapi_cnv_type_get_tChar(cmd, clen, &(rvalue->m_expectedSilicon[i]));
		}
	}
	for(i = 0; i < ETAL_SILICON_VERSION_MAX; i++)
	{
		if (ret == OSAL_OK)
		{
			/* get m_detectedSilicon */
			ret = rif_etalapi_cnv_type_get_tChar(cmd, clen, &(rvalue->m_detectedSilicon[i]));
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalInitStatus
 *
 **********************************/
/*!
 * \brief		get EtalInitStatus from a command payload
 * \details		get EtalInitStatus from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalInitStatus returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalInitStatus(tU8 **cmd, tU32 *clen, EtalInitStatus *rvalue)
{
	tSInt ret, i;

	/* get m_lastInitState */
	ret = rif_etalapi_cnv_type_get_EtalInitState(cmd, clen, &(rvalue->m_lastInitState));
	if (ret == OSAL_OK)
	{
		/* get m_warningStatus */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_warningStatus));
	}
	for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if (ret == OSAL_OK)
		{
			/* get m_tunerStatus */
			ret = rif_etalapi_cnv_type_get_EtalDeviceInitStatus(cmd, clen, &(rvalue->m_tunerStatus[i]));
		}
	}
	if (ret == OSAL_OK)
	{
		/* get m_DCOPStatus */
		ret = rif_etalapi_cnv_type_get_EtalDeviceInitStatus(cmd, clen, &(rvalue->m_DCOPStatus));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_etalSeekDirectionTy
 *
 **********************************/
/*!
 * \brief		get etalSeekDirectionTy from a command payload
 * \details		get etalSeekDirectionTy from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to etalSeekDirectionTy returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_etalSeekDirectionTy(tU8 **cmd, tU32 *clen, etalSeekDirectionTy *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(etalSeekDirectionTy));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_etalSeekAudioTy
 *
 **********************************/
/*!
 * \brief		get etalSeekAudioTy from a command payload
 * \details		get etalSeekAudioTy from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to etalSeekAudioTy returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_etalSeekAudioTy(tU8 **cmd, tU32 *clen, etalSeekAudioTy *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(etalSeekAudioTy));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_etalSeekStatusTy
 *
 **********************************/
/*!
 * \brief		get etalSeekStatusTy from a command payload
 * \details		get etalSeekStatusTy from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to etalSeekAudioTy returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_etalSeekStatusTy(tU8 **cmd, tU32 *clen, etalSeekStatusTy *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(etalSeekStatusTy));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalDabQualityEntries
 *
 **********************************/
/*!
 * \brief		get EtalDabQualityEntries from a command payload
 * \details		get EtalDabQualityEntries from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalInitStatus returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalDabQualityEntries(tU8 **cmd, tU32 *clen, EtalDabQualityEntries *rvalue)
{
	tSInt ret;

	/* get m_RFFieldStrength */
	ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->m_RFFieldStrength));
	if (ret == OSAL_OK)
	{
		/* get m_BBFieldStrength */
		ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->m_BBFieldStrength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_FicBitErrorRatio */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_FicBitErrorRatio));
	}
	if (ret == OSAL_OK)
	{
		/* get m_isValidFicBitErrorRatio */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_isValidFicBitErrorRatio));
	}
	if (ret == OSAL_OK)
	{
		/* get m_MscBitErrorRatio */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_MscBitErrorRatio));
	}
	if (ret == OSAL_OK)
	{
		/* get m_isValidMscBitErrorRatio */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_isValidMscBitErrorRatio));
	}
	if (ret == OSAL_OK)
	{
		/* get m_dataSubChBitErrorRatio */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_dataSubChBitErrorRatio));
	}
	if (ret == OSAL_OK)
	{
		/* get m_isValidDataSubChBitErrorRatio */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_isValidDataSubChBitErrorRatio));
	}
	if (ret == OSAL_OK)
	{
		/* get m_audioSubChBitErrorRatio */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_audioSubChBitErrorRatio));
	}
	if (ret == OSAL_OK)
	{
		/* get m_audioBitErrorRatioLevel */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_audioBitErrorRatioLevel));
	}
	if (ret == OSAL_OK)
	{
		/* get m_reedSolomonInformation */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_reedSolomonInformation));
	}
	if (ret == OSAL_OK)
	{
		/* get m_syncStatus */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_syncStatus));
	}
	if (ret == OSAL_OK)
	{
		/* get m_muteFlag */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_muteFlag));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalFmQualityEntries
 *
 **********************************/
/*!
 * \brief		get EtalFmQualityEntries from a command payload
 * \details		get EtalFmQualityEntries from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalInitStatus returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalFmQualityEntries(tU8 **cmd, tU32 *clen, EtalFmQualityEntries *rvalue)
{
	tSInt ret;

	/* get m_RFFieldStrength */
	ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->m_RFFieldStrength));
	if (ret == OSAL_OK)
	{
		/* get m_BBFieldStrength */
		ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->m_BBFieldStrength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_FrequencyOffset */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_FrequencyOffset));
	}
	if (ret == OSAL_OK)
	{
		/* get m_ModulationDetector */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_ModulationDetector));
	}
	if (ret == OSAL_OK)
	{
		/* get m_Multipath */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_Multipath));
	}
	if (ret == OSAL_OK)
	{
		/* get m_UltrasonicNoise */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_UltrasonicNoise));
	}
	if (ret == OSAL_OK)
	{
		/* get m_AdjacentChannel */
		ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->m_AdjacentChannel));
	}
	if (ret == OSAL_OK)
	{
		/* get m_SNR */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_SNR));
	}
	if (ret == OSAL_OK)
	{
		/* get m_coChannel */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_coChannel));
	}
	if (ret == OSAL_OK)
	{
		/* get m_StereoMonoReception */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_StereoMonoReception));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalHdQualityEntries
 *
 **********************************/
/*!
 * \brief		get EtalHdQualityEntries from a command payload
 * \details		get EtalHdQualityEntries from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalInitStatus returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalHdQualityEntries(tU8 **cmd, tU32 *clen, EtalHdQualityEntries *rvalue)
{
	tSInt ret;

	/* get m_isValidDigital */
	ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_isValidDigital));
	if (ret == OSAL_OK)
	{
		/* get m_QI */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_QI));
	}
	if (ret == OSAL_OK)
	{
		/* get m_CdToNo */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_CdToNo));
	}
	if (ret == OSAL_OK)
	{
		/* get m_DSQM */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_DSQM));
	}
	if (ret == OSAL_OK)
	{
		/* get m_AudioAlignment */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_AudioAlignment));
	}
	if (ret == OSAL_OK)
	{
		/* get m_analogQualityEntries */
		ret = rif_etalapi_cnv_type_get_EtalFmQualityEntries(cmd, clen, &(rvalue->m_analogQualityEntries));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalBcastQualityContainer
 *
 **********************************/
/*!
 * \brief		get EtalBcastQualityContainer from a command payload
 * \details		get EtalBcastQualityContainer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalBcastQualityContainer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalBcastQualityContainer(tU8 **cmd, tU32 *clen, EtalBcastQualityContainer *rvalue)
{
	tSInt ret = OSAL_OK;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_TimeStamp */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_TimeStamp));
	}
	if (ret == OSAL_OK)
	{
		/* get m_standard */
		ret = rif_etalapi_cnv_type_get_EtalBcastStandard(cmd, clen, &(rvalue->m_standard));
	}
	if (ret == OSAL_OK)
	{
		/* get m_Context */
		ret = rif_etalapi_cnv_type_get_tPVoid(cmd, clen, &(rvalue->m_Context));
	}
	if (ret == OSAL_OK)
	{
		if (rvalue->m_standard == ETAL_BCAST_STD_DAB)
		{
			/* get EtalQualityEntries.dab */
			ret = rif_etalapi_cnv_type_get_EtalDabQualityEntries(cmd, clen, &(rvalue->EtalQualityEntries.dab));
		}
		else if ((rvalue->m_standard == ETAL_BCAST_STD_FM) || (rvalue->m_standard == ETAL_BCAST_STD_AM))
		{
			/* get EtalQualityEntries.dab */
			ret = rif_etalapi_cnv_type_get_EtalFmQualityEntries(cmd, clen, &(rvalue->EtalQualityEntries.amfm));
		}
		else if ((rvalue->m_standard == ETAL_BCAST_STD_HD_FM) || (rvalue->m_standard == ETAL_BCAST_STD_HD) ||
			(rvalue->m_standard == ETAL_BCAST_STD_HD_AM))
		{
			/* get EtalQualityEntries.hd */
			ret = rif_etalapi_cnv_type_get_EtalHdQualityEntries(cmd, clen, &(rvalue->EtalQualityEntries.hd));
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalSeekStatus
 *
 **********************************/
/*!
 * \brief		get EtalSeekStatus from a command payload
 * \details		get EtalSeekStatus from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalInitStatus returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalSeekStatus(tU8 **cmd, tU32 *clen, EtalSeekStatus *rvalue)
{
	tSInt ret;

	/* get m_receiverHandle */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(cmd, clen, &(rvalue->m_receiverHandle));
	if (ret == OSAL_OK)
	{
		/* get m_status */
		ret = rif_etalapi_cnv_type_get_etalSeekStatusTy(cmd, clen, &(rvalue->m_status));
	}
	if (ret == OSAL_OK)
	{
		/* get m_frequency */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_frequency));
	}
	if (ret == OSAL_OK)
	{
		/* get m_frequencyFound */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_frequencyFound));
	}
	if (ret == OSAL_OK)
	{
		/* get m_HDProgramFound */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_HDProgramFound));
	}
	if (ret == OSAL_OK)
	{
		/* get m_fullCycleReached */
		ret = rif_etalapi_cnv_type_get_tBool(cmd, clen, &(rvalue->m_fullCycleReached));
	}
	if (ret == OSAL_OK)
	{
		/* get m_serviceId */
		ret = rif_etalapi_cnv_type_get_tS8(cmd, clen, &(rvalue->m_serviceId));
	}
	if (ret == OSAL_OK)
	{
		/* get m_quality */
		ret = rif_etalapi_cnv_type_get_EtalBcastQualityContainer(cmd, clen, &(rvalue->m_quality));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalFrequencyBand
 *
 **********************************/
/*!
 * \brief		get EtalFrequencyBand from a command payload
 * \details		get EtalFrequencyBand from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalFrequencyBand returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalFrequencyBand(tU8 **cmd, tU32 *clen, EtalFrequencyBand *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalFrequencyBand));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalProcessingFeatures
 *
 **********************************/
/*!
 * \brief		get EtalProcessingFeatures from a command payload
 * \details		get EtalProcessingFeatures from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalProcessingFeatures returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalProcessingFeatures(tU8 **cmd, tU32 *clen, EtalProcessingFeatures *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)&(rvalue->u.m_processing_features), sizeof(rvalue->u.m_processing_features));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalServiceSelectMode
 *
 **********************************/
/*!
 * \brief		get EtalServiceSelectMode from a command payload
 * \details		get EtalServiceSelectMode from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalServiceSelectMode returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalServiceSelectMode(tU8 **cmd, tU32 *clen, EtalServiceSelectMode *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalServiceSelectMode));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalServiceSelectSubFunction
 *
 **********************************/
/*!
 * \brief		get EtalServiceSelectSubFunction from a command payload
 * \details		get EtalServiceSelectSubFunction from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalServiceSelectSubFunction returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalServiceSelectSubFunction(tU8 **cmd, tU32 *clen, EtalServiceSelectSubFunction *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalServiceSelectSubFunction));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_etalAFModeTy
 *
 **********************************/
/*!
 * \brief		get etalAFModeTy from a command payload
 * \details		get etalAFModeTy from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to etalAFModeTy returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_etalAFModeTy(tU8 **cmd, tU32 *clen, etalAFModeTy *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(etalAFModeTy));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalRDSRBDSModeTy
 *
 **********************************/
/*!
 * \brief		get EtalRDSRBDSModeTy from a command payload
 * \details		get EtalRDSRBDSModeTy from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalRDSRBDSModeTy returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalRDSRBDSModeTy(tU8 **cmd, tU32 *clen, EtalRDSRBDSModeTy *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalRDSRBDSModeTy));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalBcastQaIndicators
 *
 **********************************/
/*!
 * \brief		get EtalBcastQaIndicators from a command payload
 * \details		get EtalBcastQaIndicators from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalBcastQaIndicators returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalBcastQaIndicators(tU8 **cmd, tU32 *clen, EtalBcastQaIndicators *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalBcastQaIndicators));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalQaMonitoredEntryAttr
 *
 **********************************/
/*!
 * \brief		get EtalQaMonitoredEntryAttr from a command payload
 * \details		get EtalQaMonitoredEntryAttr from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalQaMonitoredEntryAttr returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalQaMonitoredEntryAttr(tU8 **cmd, tU32 *clen, EtalQaMonitoredEntryAttr *rvalue)
{
	tSInt ret = OSAL_OK;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_MonitoredIndicator */
		ret = rif_etalapi_cnv_type_get_EtalBcastQaIndicators(cmd, clen, &(rvalue->m_MonitoredIndicator));
	}
	if (ret == OSAL_OK)
	{
		/* get m_InferiorValue */
		ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->m_InferiorValue));
	}
	if (ret == OSAL_OK)
	{
		/* get m_SuperiorValue */
		ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->m_SuperiorValue));
	}
	if (ret == OSAL_OK)
	{
		/* get m_UpdateFrequency */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_UpdateFrequency));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalCbBcastQualityProcess
 *
 **********************************/
/*!
 * \brief		get EtalCbBcastQualityProcess from a command payload
 * \details		get EtalCbBcastQualityProcess from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalCbBcastQualityProcess returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalCbBcastQualityProcess(tU8 **cmd, tU32 *clen, tPVoid *rvalue)
{
	tSInt ret;

	ret = rif_etalapi_cnv_type_get_tPVoid(cmd, clen, (tPVoid *)rvalue);
	/* set EtalCbBcastQualityProcess */
	if (rvalue != NULL)
	{
		if (*rvalue == 0)
		{
			*rvalue = rif_etalapi_cnv_CbBcastQualityProcess;
		}
	}
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalBcastQualityMonitorAttr
 *
 **********************************/
/*!
 * \brief		get EtalBcastQualityMonitorAttr from a command payload
 * \details		get EtalBcastQualityMonitorAttr from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalBcastQualityMonitorAttr returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalBcastQualityMonitorAttr(tU8 **cmd, tU32 *clen, EtalBcastQualityMonitorAttr *rvalue)
{
	tSInt ret = OSAL_OK, i;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_receiverHandle */
		ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(cmd, clen, &(rvalue->m_receiverHandle));
	}
	if (ret == OSAL_OK)
	{
		for(i = 0; i < ETAL_MAX_QUALITY_PER_MONITOR; i++)
		{
			if (ret == OSAL_OK)
			{
				/* get m_monitoredIndicators[i] */
				ret = rif_etalapi_cnv_type_get_EtalQaMonitoredEntryAttr(cmd, clen, &(rvalue->m_monitoredIndicators[i]));
			}
		}
	}
	if (ret == OSAL_OK)
	{
		/* get m_Context */
		ret = rif_etalapi_cnv_type_get_tPVoid(cmd, clen, &(rvalue->m_Context));
	}
	if (ret == OSAL_OK)
	{
		/* get m_CbBcastQualityProcess */
		ret = rif_etalapi_cnv_type_get_EtalCbBcastQualityProcess(cmd, clen, (tPVoid *)&(rvalue->m_CbBcastQualityProcess));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalEnsembleDesc
 *
 **********************************/
/*!
 * \brief		get EtalEnsembleDesc from a command payload
 * \details		get EtalEnsembleDesc from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalEnsembleDesc returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalEnsembleDesc(tU8 **cmd, tU32 *clen, EtalEnsembleDesc *rvalue)
{
	tSInt ret = OSAL_OK;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_ECC */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_ECC));
	}
	if (ret == OSAL_OK)
	{
		/* get m_ensembleId */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_ensembleId));
	}
	if (ret == OSAL_OK)
	{
		/* get m_frequency */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_frequency));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalEnsembleList
 *
 **********************************/
/*!
 * \brief		get EtalEnsembleList from a command payload
 * \details		get EtalEnsembleList from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalEnsembleList returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalEnsembleList(tU8 **cmd, tU32 *clen, EtalEnsembleList *rvalue)
{
	tSInt ret = OSAL_OK, i;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_ensembleCount */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_ensembleCount));
	}
	if (ret == OSAL_OK)
	{
		for(i = 0; i < ETAL_DEF_MAX_ENSEMBLE; i++)
		{
			if (ret == OSAL_OK)
			{
				/* get m_ensemble[i] */
				ret = rif_etalapi_cnv_type_get_EtalEnsembleDesc(cmd, clen, &(rvalue->m_ensemble[i]));
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalServiceList
 *
 **********************************/
/*!
 * \brief		get EtalServiceList from a command payload
 * \details		get EtalServiceList from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalServiceList returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalServiceList(tU8 **cmd, tU32 *clen, EtalServiceList *rvalue)
{
	tSInt ret = OSAL_OK, i;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_serviceCount */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_serviceCount));
	}
	if (ret == OSAL_OK)
	{
		for(i = 0; i < ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE; i++)
		{
			if (ret == OSAL_OK)
			{
				/* get m_service[i] */
				ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_service[i]));
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalServiceInfo
 *
 **********************************/
/*!
 * \brief		get EtalServiceInfo from a command payload
 * \details		get EtalServiceInfo from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalServiceInfo returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalServiceInfo(tU8 **cmd, tU32 *clen, EtalServiceInfo *rvalue)
{
	tSInt ret = OSAL_OK, i;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_serviceBitrate */
		ret = rif_etalapi_cnv_type_get_tU16(cmd, clen, &(rvalue->m_serviceBitrate));
	}
	if (ret == OSAL_OK)
	{
		/* get m_subchId */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_subchId));
	}
	if (ret == OSAL_OK)
	{
		/* get m_packetAddress */
		ret = rif_etalapi_cnv_type_get_tU16(cmd, clen, &(rvalue->m_packetAddress));
	}
	if (ret == OSAL_OK)
	{
		/* get m_serviceLanguage */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_serviceLanguage));
	}
	if (ret == OSAL_OK)
	{
		/* get m_componentType */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_componentType));
	}
	if (ret == OSAL_OK)
	{
		/* get m_streamType */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_streamType));
	}
	if (ret == OSAL_OK)
	{
		/* get m_scCount */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_scCount));
	}
	if (ret == OSAL_OK)
	{
		/* get m_serviceLabelCharset */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_serviceLabelCharset));
	}
	if (ret == OSAL_OK)
	{
		for(i = 0; i < ETAL_DEF_MAX_LABEL_LEN; i++)
		{
			if (ret == OSAL_OK)
			{
				/* get m_serviceLabel[i] */
				ret = rif_etalapi_cnv_type_get_tChar(cmd, clen, &(rvalue->m_serviceLabel[i]));
			}
		}
	}
	if (ret == OSAL_OK)
	{
		/* get m_serviceLabelCharflag */
		ret = rif_etalapi_cnv_type_get_tU16(cmd, clen, &(rvalue->m_serviceLabelCharflag));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalSCInfo
 *
 **********************************/
/*!
 * \brief		get EtalSCInfo from a command payload
 * \details		get EtalSCInfo from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalSCInfo returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalSCInfo(tU8 **cmd, tU32 *clen, EtalSCInfo *rvalue)
{
	tSInt ret = OSAL_OK, i;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_scIndex */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_scIndex));
	}
	if (ret == OSAL_OK)
	{
		/* get m_dataServiceType */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_dataServiceType));
	}
	if (ret == OSAL_OK)
	{
		/* get m_scType */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_scType));
	}
	if (ret == OSAL_OK)
	{
		/* get m_scLabelCharset */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_scLabelCharset));
	}
	if (ret == OSAL_OK)
	{
		for(i = 0; i < ETAL_DEF_MAX_LABEL_LEN; i++)
		{
			if (ret == OSAL_OK)
			{
				/* get m_scLabel[i] */
				ret = rif_etalapi_cnv_type_get_tChar(cmd, clen, &(rvalue->m_scLabel[i]));
			}
		}
	}
	if (ret == OSAL_OK)
	{
		/* get m_scLabelCharflag */
		ret = rif_etalapi_cnv_type_get_tU16(cmd, clen, &(rvalue->m_scLabelCharflag));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalServiceComponentList
 *
 **********************************/
/*!
 * \brief		get EtalServiceComponentList from a command payload
 * \details		get EtalServiceComponentList from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalServiceComponentList returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalServiceComponentList(tU8 **cmd, tU32 *clen, EtalServiceComponentList *rvalue)
{
	tSInt ret = OSAL_OK, i;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_scCount */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_scCount));
	}
	if (ret == OSAL_OK)
	{
		for(i = 0; i < ETAL_DEF_MAX_SC_PER_SERVICE; i++)
		{
			if (ret == OSAL_OK)
			{
				/* get m_scInfo[i] */
				ret = rif_etalapi_cnv_type_get_EtalSCInfo(cmd, clen, &(rvalue->m_scInfo[i]));
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalDataService_EpgLogoType
 *
 **********************************/
/*!
 * \brief		get EtalDataService_EpgLogoType from a command payload
 * \details		get EtalDataService_EpgLogoType from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalDataService_EpgLogoType returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalDataService_EpgLogoType(tU8 **cmd, tU32 *clen, EtalDataService_EpgLogoType *rvalue)
{
	return rif_etalapi_cnv_type_get_size(cmd, clen, (tVoid *)rvalue, sizeof(EtalDataService_EpgLogoType));
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalDataServiceParam
 *
 **********************************/
/*!
 * \brief		get EtalDataServiceParam from a command payload
 * \details		get EtalDataServiceParam from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalDataServiceParam returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalDataServiceParam(tU8 **cmd, tU32 *clen, EtalDataServiceParam *rvalue)
{
	tSInt ret = OSAL_OK;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_ecc */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_ecc));
	}
	if (ret == OSAL_OK)
	{
		/* get m_eid */
		ret = rif_etalapi_cnv_type_get_tU16(cmd, clen, &(rvalue->m_eid));
	}
	if (ret == OSAL_OK)
	{
		/* get m_sid */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->m_sid));
	}
	if (ret == OSAL_OK)
	{
		/* get m_logoType */
		ret = rif_etalapi_cnv_type_get_EtalDataService_EpgLogoType(cmd, clen, &(rvalue->m_logoType));
	}
	if (ret == OSAL_OK)
	{
		/* get m_JMLObjectId */
		ret = rif_etalapi_cnv_type_get_tU16(cmd, clen, &(rvalue->m_JMLObjectId));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalPSDLength
 *
 **********************************/
/*!
 * \brief		get EtalPSDLength from a command payload
 * \details		get EtalPSDLength from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalPSDLength returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalPSDLength(tU8 **cmd, tU32 *clen, EtalPSDLength *rvalue)
{
	tSInt ret = OSAL_OK;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDTitleLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDTitleLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDArtistLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDArtistLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDAlbumLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDAlbumLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDGenreLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDGenreLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDCommentShortLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDCommentShortLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDCommentLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDCommentLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDUFIDLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDUFIDLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDCommercialPriceLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDCommercialPriceLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDCommercialContactLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDCommercialContactLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDCommercialSellerLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDCommercialSellerLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDCommercialDescriptionLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDCommercialDescriptionLength));
	}
	if (ret == OSAL_OK)
	{
		/* get m_PSDXHDRLength */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->m_PSDXHDRLength));
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalSeamlessEstimationConfigTy
 *
 **********************************/
/*!
 * \brief		get EtalSeamlessEstimationConfigTy from a command payload
 * \details		get EtalSeamlessEstimationConfigTy from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalSeamlessEstimationConfigTy returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalSeamlessEstimationConfigTy(tU8 **cmd, tU32 *clen, etalSeamlessEstimationConfigTy *rvalue)
{
	tSInt ret = OSAL_OK;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get mode */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->mode));
	}
	if (ret == OSAL_OK)
	{
		/* get startPosition */
		ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->startPosition));
	}
	if (ret == OSAL_OK)
	{
		/* get stopPosition */
		ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->stopPosition));
	}
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_EtalSeamlessSwitchingConfigTy
 *
 **********************************/
/*!
 * \brief		get EtalSeamlessSwitchingConfigTy from a command payload
 * \details		get EtalSeamlessSwitchingConfigTy from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to EtalSeamlessSwitchingConfigTy returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_EtalSeamlessSwitchingConfigTy(tU8 **cmd, tU32 *clen, etalSeamlessSwitchingConfigTy *rvalue)
{
	tSInt ret = OSAL_OK;

	if (rvalue == NULL)
	{
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		/* get systemToSwitch */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->systemToSwitch));
	}
	if (ret == OSAL_OK)
	{
		/* get providerType */
		ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, &(rvalue->providerType));
	}
	if (ret == OSAL_OK)
	{
		/* get absoluteDelayEstimate */
		ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->absoluteDelayEstimate));
	}
	if (ret == OSAL_OK)
	{
		/* get delayEstimate */
		ret = rif_etalapi_cnv_type_get_tS32(cmd, clen, &(rvalue->delayEstimate));
	}
	if (ret == OSAL_OK)
	{
		/* get timestampFAS */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->timestampFAS));
	}
	if (ret == OSAL_OK)
	{
		/* get timestampSAS */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->timestampSAS));
	}
	if (ret == OSAL_OK)
	{
		/* get averageRMS2FAS */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->averageRMS2FAS));
	}
	if (ret == OSAL_OK)
	{
		/* get averageRMS2SAS */
		ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, &(rvalue->averageRMS2SAS));
	}
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_p
 *
 **********************************/
/*!
 * \brief		get pointer from a command payload
 * \details		get pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tU32 pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen or pointer type
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_p(tU8 **cmd, tU32 *clen, void **rvalue)
{
	tSInt ret;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* nothing to do */
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalHardwareAttr
 *
 **********************************/
/*!
 * \brief		get EtalHardwareAttr pointer from a command payload
 * \details		get EtalHardwareAttr pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalHardwareAttr pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalHardwareAttr(tU8 **cmd, tU32 *clen, EtalHardwareAttr **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalReceiverAttr */
			ret = rif_etalapi_cnv_type_get_EtalHardwareAttr(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pETAL_HANDLE
 *
 **********************************/
/*!
 * \brief		get ETAL_HANDLE pointer from a command payload
 * \details		get ETAL_HANDLE pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to ETAL_HANDLE pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen or pointer type
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pETAL_HANDLE(tU8 **cmd, tU32 *clen, ETAL_HANDLE **rvalue)
{
	tSInt ret;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get ETAL_HANDLE */
			ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_ptU8
 *
 **********************************/
/*!
 * \brief		get tU8 pointer from a command payload
 * \details		get tU8 pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tU8 pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen or pointer type
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_ptU8(tU8 **cmd, tU32 *clen, tU8 **rvalue)
{
	tSInt ret;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get tU8 */
			ret = rif_etalapi_cnv_type_get_tU8(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_ptU16
 *
 **********************************/
/*!
 * \brief		get tU16 pointer from a command payload
 * \details		get tU16 pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tU16 pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen or pointer type
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_ptU16(tU8 **cmd, tU32 *clen, tU16 **rvalue)
{
	tSInt ret;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get tU16 */
			ret = rif_etalapi_cnv_type_get_tU16(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_ptU32
 *
 **********************************/
/*!
 * \brief		get tU32 pointer from a command payload
 * \details		get tU32 pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to tU32 pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen or pointer type
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_ptU32(tU8 **cmd, tU32 *clen, tU32 **rvalue)
{
	tSInt ret;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get tU32 */
			ret = rif_etalapi_cnv_type_get_tU32(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalReceiverAttr
 *
 **********************************/
/*!
 * \brief		get EtalReceiverAttr pointer from a command payload
 * \details		get EtalReceiverAttr pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalReceiverAttr pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalReceiverAttr(tU8 **cmd, tU32 *clen, EtalReceiverAttr **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalReceiverAttr */
			ret = rif_etalapi_cnv_type_get_EtalReceiverAttr(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_ppEtalHwCapabilities
 *
 **********************************/
/*!
 * \brief		get EtalHwCapabilities pointer to pointer from a command payload
 * \details		get EtalHwCapabilities pointer to pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to pointer to EtalHwCapabilities pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_ppEtalHwCapabilities(tU8 **cmd, tU32 *clen, EtalHwCapabilities ***rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* nothing to do here because **EtalHwCapabilities is only output */
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;

}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalDataPathAttr
 *
 **********************************/
/*!
 * \brief		get EtalDataPathAttr pointer from a command payload
 * \details		get EtalDataPathAttr pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalDataPathAttr pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalDataPathAttr(tU8 **cmd, tU32 *clen, EtalDataPathAttr **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalDataPathAttr */
			ret = rif_etalapi_cnv_type_get_EtalDataPathAttr(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalVersion
 *
 **********************************/
/*!
 * \brief		get EtalVersion pointer from a command payload
 * \details		get EtalVersion pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalVersion pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalVersion(tU8 **cmd, tU32 *clen, EtalVersion **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalVersion */
			ret = rif_etalapi_cnv_type_get_EtalVersion(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalAudioAlignmentAttr
 *
 **********************************/
/*!
 * \brief		get EtalAudioAlignmentAttr pointer from a command payload
 * \details		get EtalAudioAlignmentAttr pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalAudioAlignmentAttr pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalAudioAlignmentAttr(tU8 **cmd, tU32 *clen, EtalAudioAlignmentAttr **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalAudioAlignmentAttr */
			ret = rif_etalapi_cnv_type_get_EtalAudioAlignmentAttr(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalInitStatus
 *
 **********************************/
/*!
 * \brief		get EtalInitStatus pointer from a command payload
 * \details		get EtalInitStatus pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalInitStatus pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalInitStatus(tU8 **cmd, tU32 *clen, EtalInitStatus **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalInitStatus */
			ret = rif_etalapi_cnv_type_get_EtalInitStatus(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalSeekStatus
 *
 **********************************/
/*!
 * \brief		get EtalSeekStatus pointer from a command payload
 * \details		get EtalSeekStatus pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalSeekStatus pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalSeekStatus(tU8 **cmd, tU32 *clen, EtalSeekStatus **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalSeekStatus */
			ret = rif_etalapi_cnv_type_get_EtalSeekStatus(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalBcastQualityContainer
 *
 **********************************/
/*!
 * \brief		get EtalBcastQualityContainer pointer from a command payload
 * \details		get EtalBcastQualityContainer pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalBcastQualityContainer pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalBcastQualityContainer(tU8 **cmd, tU32 *clen, EtalBcastQualityContainer **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalBcastQualityContainer */
			ret = rif_etalapi_cnv_type_get_EtalBcastQualityContainer(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalBcastQualityMonitorAttr
 *
 **********************************/
/*!
 * \brief		get EtalBcastQualityMonitorAttr pointer from a command payload
 * \details		get EtalBcastQualityMonitorAttr pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalBcastQualityMonitorAttr pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalBcastQualityMonitorAttr(tU8 **cmd, tU32 *clen, EtalBcastQualityMonitorAttr **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalBcastQualityMonitorAttr */
			ret = rif_etalapi_cnv_type_get_EtalBcastQualityMonitorAttr(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalEnsembleList
 *
 **********************************/
/*!
 * \brief		get EtalEnsembleList pointer from a command payload
 * \details		get EtalEnsembleList pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalEnsembleList pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalEnsembleList(tU8 **cmd, tU32 *clen, EtalEnsembleList **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalEnsembleList */
			ret = rif_etalapi_cnv_type_get_EtalEnsembleList(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalServiceList
 *
 **********************************/
/*!
 * \brief		get EtalServiceList pointer from a command payload
 * \details		get EtalServiceList pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalServiceList pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalServiceList(tU8 **cmd, tU32 *clen, EtalServiceList **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalServiceList */
			ret = rif_etalapi_cnv_type_get_EtalServiceList(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalServiceInfo
 *
 **********************************/
/*!
 * \brief		get EtalServiceInfo pointer from a command payload
 * \details		get EtalServiceInfo pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalServiceInfo pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalServiceInfo(tU8 **cmd, tU32 *clen, EtalServiceInfo **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalServiceInfo */
			ret = rif_etalapi_cnv_type_get_EtalServiceInfo(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalServiceComponentList
 *
 **********************************/
/*!
 * \brief		get EtalServiceComponentList pointer from a command payload
 * \details		get EtalServiceComponentList pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalServiceComponentList pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalServiceComponentList(tU8 **cmd, tU32 *clen, EtalServiceComponentList **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalServiceComponentList */
			ret = rif_etalapi_cnv_type_get_EtalServiceComponentList(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalPSDLength
 *
 **********************************/
/*!
 * \brief		get EtalPSDLength pointer from a command payload
 * \details		get EtalPSDLength pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalPSDLength pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalPSDLength(tU8 **cmd, tU32 *clen, EtalPSDLength **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalPSDLength */
			ret = rif_etalapi_cnv_type_get_EtalPSDLength(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalSeamlessEstimationConfigTy
 *
 **********************************/
/*!
 * \brief		get EtalSeamlessEstimationConfigTy pointer from a command payload
 * \details		get EtalSeamlessEstimationConfigTy pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalSeamlessEstimationConfigTy pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalSeamlessEstimationConfigTy(tU8 **cmd, tU32 *clen, etalSeamlessEstimationConfigTy **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalSeamlessEstimationConfigTy */
			ret = rif_etalapi_cnv_type_get_EtalSeamlessEstimationConfigTy(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_type_get_pEtalSeamlessSwitchingConfigTy
 *
 **********************************/
/*!
 * \brief		get EtalSeamlessSwitchingConfigTy pointer from a command payload
 * \details		get EtalSeamlessSwitchingConfigTy pointer from a command payload. Increase command payload pointer position and
 * 				decrease payload length accordingly.
 * \param[in/out] cmd    - pointer to pointer to buffer of tU8 containing the command payload. the function
 * 				           does not make any assumption on the content of the buffer
 * \param[in/out] clen   - pointer to size in bytes of the *cmd* buffer
 * \param[out]	  rvalue - pointer to pointer to EtalSeamlessSwitchingConfigTy pointer returned value.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter clen
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_type_get_pEtalSeamlessSwitchingConfigTy(tU8 **cmd, tU32 *clen, etalSeamlessSwitchingConfigTy **rvalue)
{
	tSInt ret = OSAL_OK;
	rif_etalapi_cnv_pointer_type_enum pointer_type = RIF_ETALAPI_CNV_POINTER_NULL;

	/* get pointer type */
	ret = rif_etalapi_cnv_type_get_pointer_type(cmd, clen, &pointer_type);
	if (ret == OSAL_OK)
	{
		if (pointer_type == RIF_ETALAPI_CNV_POINTER_NULL)
		{
			/* set null pointer */
			*rvalue = NULL;
		}
		else if (pointer_type == RIF_ETALAPI_CNV_POINTER_VALID)
		{
			/* get EtalSeamlessEstimationConfigTy */
			ret = rif_etalapi_cnv_type_get_EtalSeamlessSwitchingConfigTy(cmd, clen, *rvalue);
		}
		else
		{
			/* invalid pointer type */
			ret = OSAL_ERROR_INVALID_PARAM;
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_CbNotify
 *
 **********************************/
/*!
 * \brief		handle etal events notification to rimw auto-notifications
 * \details		convert etal events notification to a rimw message auto-notification.
 * \param[in]	  pvContext - pointer to pvContext
 * \param[in]	  dwEvent   - etal event Id
 * \param[in]	  pvParams  - pointer to buffer pvParams, the size depend on etal event.
 * \return		void
  * \callgraph
 * \callergraph
 */
void rif_etalapi_cnv_CbNotify(void *pvContext, ETAL_EVENTS dwEvent, void *pvParams)
{
	tSInt ret = OSAL_OK;
	tU32 rlen = 0;

	switch (dwEvent) {
		case ETAL_INFO_TUNE:
			/* create auto response */
			//ret = rif_etalapi_cnv_create_auto_resp(rif_rimw_auto_notif, &rlen, (tU8)dwEvent);
			ret = rif_etalapi_cnv_create_auto_notif(rif_rimw_auto_notif, &rlen, ETAL_EVENT, (tU8)dwEvent);
			if (ret == OSAL_OK)
			{
				/* add tU32 *pvContext to auto notification response payload */
				ret = rif_etalapi_cnv_resp_add_ptU32(rif_rimw_auto_notif, &rlen, (tU32 *)pvContext);
			}
			if (ret == OSAL_OK)
			{
				/* add EtalTuneStatus *pvParams to auto notification response payload */
				ret = rif_etalapi_cnv_resp_add_pEtalTuneStatus(rif_rimw_auto_notif, &rlen, pvParams);
			}
			if (ret == OSAL_OK)
			{
				/* send auto notification response */
				if ((ret = rif_sendMsgTo_pr(MSG_ID_RIF_PR_TX_RESP_RIMW, STECI_UART_MCP_LUN, rif_rimw_auto_notif, rlen)) != OSAL_OK)
				{
					rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_sendMsgTo_pr error (%d)", ret);
				}
			}
			break;
		case ETAL_INFO_SEAMLESS_ESTIMATION_END:
			/* create auto notif */
			ret = rif_etalapi_cnv_create_auto_notif(rif_rimw_auto_notif, &rlen, ETAL_EVENT, (tU8)dwEvent);
			if (ret == OSAL_OK)
			{
				/* add tU32 *pvContext to auto notification response payload */
				ret = rif_etalapi_cnv_resp_add_ptU32(rif_rimw_auto_notif, &rlen, (tU32 *)pvContext);
			}
			if (ret == OSAL_OK)
			{
				/* add EtalSeamlessEstimationStatus *pvParams to auto notification response payload */
				ret = rif_etalapi_cnv_resp_add_pEtalSeamlessEstimationStatus(rif_rimw_auto_notif, &rlen, pvParams);
			}
			if (ret == OSAL_OK)
			{
				/* send auto notification response */
				if ((ret = rif_sendMsgTo_pr(MSG_ID_RIF_PR_TX_RESP_RIMW, STECI_UART_MCP_LUN, rif_rimw_auto_notif, rlen)) != OSAL_OK)
				{
					rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_sendMsgTo_pr error (%d)", ret);
				}
			}
			break;
		case ETAL_INFO_SEAMLESS_SWITCHING_END:
			/* create auto notif */
			ret = rif_etalapi_cnv_create_auto_notif(rif_rimw_auto_notif, &rlen, ETAL_EVENT, (tU8)dwEvent);
			if (ret == OSAL_OK)
			{
				/* add tU32 *pvContext to auto notification response payload */
				ret = rif_etalapi_cnv_resp_add_ptU32(rif_rimw_auto_notif, &rlen, (tU32 *)pvContext);
			}
			if (ret == OSAL_OK)
			{
				/* add EtalSeamlessSwitchingStatus *pvParams to auto notification response payload */
				ret = rif_etalapi_cnv_resp_add_pEtalSeamlessSwitchingStatus(rif_rimw_auto_notif, &rlen, pvParams);
			}
			if (ret == OSAL_OK)
			{
				/* send auto notification response */
				if ((ret = rif_sendMsgTo_pr(MSG_ID_RIF_PR_TX_RESP_RIMW, STECI_UART_MCP_LUN, rif_rimw_auto_notif, rlen)) != OSAL_OK)
				{
					rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_sendMsgTo_pr error (%d)", ret);
				}
			}
			break;
		case ETAL_INFO_LEARN:
//			break;
		case ETAL_INFO_SCAN:
//			break;
		case ETAL_ERROR_COMM_FAILED:
//			break;
		default:
			rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "Unexpected event from ETAL notification handler (%d)", dwEvent);
			break;
	}
}

/***********************************
 *
 * rif_etalapi_cnv_CbRDS_DataPath
 *
 **********************************/
/*!
 * \brief		Callback to RDS DataPath
 * \details		convert RDS DataPath to a rimw message auto-notification.
 * \param[in]	  pBuffer   - pointer to pBuffer
 * \param[in]	  dwActualBufferSize  - size in bytes of the *pBuffer* buffer
 * \param[in]	  status    - pointer to buffer an EtalDataBlockStatusTy.
 * \param[in]	  pvContext - pointer to pvContext
 * \return		void
 * \callgraph
 * \callergraph
 */
void rif_etalapi_cnv_CbEtalRDSData_DataPath(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	tSInt ret = OSAL_OK;
	tU32 rlen = 0;

	/* create auto notif */
	ret = rif_etalapi_cnv_create_data_path(rif_rimw_auto_notif, &rlen, ETAL_EVENT, ETAL_DATA_PATH_TYPE_FM_RDS, dwActualBufferSize, status, pvContext);
	if (ret == OSAL_OK)
	{
		ret = rif_etaltmlapi_cnv_resp_add_pEtalRDSData(rif_rimw_auto_notif, &rlen, (EtalRDSData *)pBuffer);
	}
	if (ret == OSAL_OK)
	{
		/* send auto notification response */
		if ((ret = rif_sendMsgTo_pr(MSG_ID_RIF_PR_TX_RESP_RIMW, STECI_UART_MCP_LUN, rif_rimw_auto_notif, rlen)) != OSAL_OK)
		{
			rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_sendMsgTo_pr error (%d)", ret);
		}
	}
}


/***********************************
 *
 * rif_etalapi_cnv_CbTextInfo_DataPath
 *
 **********************************/
/*!
 * \brief		Callback to TEXT_INFO DataPath
 * \details		convert TextInfo DataPath to a rimw message auto-notification.
 * \param[in]	  pBuffer   - pointer to pBuffer
 * \param[in]	  dwActualBufferSize  - size in bytes of the *pBuffer* buffer
 * \param[in]	  status    - pointer to buffer an EtalDataBlockStatusTy.
 * \param[in]	  pvContext - pointer to pvContext
 * \return		void
 * \callgraph
 * \callergraph
 */
void rif_etalapi_cnv_CbTextInfo_DataPath(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	tSInt ret = OSAL_OK;
	tU32 rlen = 0;

	/* create auto notif */
	ret = rif_etalapi_cnv_create_data_path(rif_rimw_auto_notif, &rlen, ETAL_EVENT, ETAL_DATA_PATH_TYPE_TEXTINFO, dwActualBufferSize, status, pvContext);
	if (ret == OSAL_OK)
	{
		ret = rif_etaltmlapi_cnv_resp_add_pEtalTextInfo(rif_rimw_auto_notif, &rlen, (EtalTextInfo *)pBuffer);
	}
	if (ret == OSAL_OK)
	{
		/* send auto notification response */
		if ((ret = rif_sendMsgTo_pr(MSG_ID_RIF_PR_TX_RESP_RIMW, STECI_UART_MCP_LUN, rif_rimw_auto_notif, rlen)) != OSAL_OK)
		{
			rif_rimw_tracePrintError(TR_CLASS_EXTERNAL, "rif_sendMsgTo_pr error (%d)", ret);
		}
	}
}

/***********************************
 *
 * rif_etalapi_cnv_CbProcessBlock
 *
 **********************************/
/*!
 * \brief		handle etal dataPath callback notification to rimw data channel
 * \details		convert etal dataPath callback notification to a rimw message data channel.
 * \param[in]	  pBuffer   - pointer to pBuffer
 * \param[in]	  dwActualBufferSize  - size in bytes of the *pBuffer* buffer
 * \param[in]	  status    - pointer to buffer an EtalDataBlockStatusTy.
 * \param[in]	  pvContext - pointer to pvContext
 * \return		void
 * \callgraph
 * \callergraph
 */
void rif_etalapi_cnv_CbProcessBlock(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	// TODO
}

/***********************************
 *
 * rif_etalapi_cnv_CbBcastQualityProcess
 *
 **********************************/
/*!
 * \brief		handle etal broadcast quality callback notification to rimw data channel
 * \details		convert etal boradcast quality callback notification to a rimw message data channel.
 * \param[in]	  pBuffer   - pointer to pBuffer
 * \param[in]	  dwActualBufferSize  - size in bytes of the *pBuffer* buffer
 * \param[in]	  status    - pointer to buffer an EtalDataBlockStatusTy.
 * \param[in]	  pvContext - pointer to pvContext
 * \return		void
 * \callgraph
 * \callergraph
 */
void rif_etalapi_cnv_CbBcastQualityProcess(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	// TODO
}

/***********************************
 *
 * rif_etalapi_cnv_etal_initialize
 *
 **********************************/
/*!
 * \brief		handle an etal_initialize rimw command
 * \details		call etal_initialize api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_initialize(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	EtalHardwareAttr HWInit, *pHWInit = &HWInit;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	OSAL_pvMemorySet(&HWInit, 0, sizeof(HWInit));
	ret = rif_etalapi_cnv_type_get_pEtalHardwareAttr(&cmd_payload, &cmd_len, &pHWInit);

	if (ret == OSAL_OK)
	{
		/* call etal_initialize */
		retval = etal_initialize(pHWInit);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_deinitialize
 *
 **********************************/
/*!
 * \brief		handle an etal_deinitialize rimw command
 * \details		call etal_deinitialize api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_deinitialize(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	/*tU8 *cmd_payload;*/
	tSInt ret = OSAL_OK;
#if 0
	ETAL_STATUS retval = ETAL_RET_SUCCESS;
	tU32 cmd_len;

/* get payload address and payload length */
	/*cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);*/
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	if (cmd_len != 0)
	{
		ret = OSAL_ERROR_INVALID_PARAM;
	}

	if (ret == OSAL_OK)
	{
		/* call etal_deinitialize */
		retval = etal_deinitialize();
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	/* free memory allocated on etal_initialize */
	if (rif_m_CustomParam != NULL)
	{
		OSAL_vMemoryFree(rif_m_CustomParam);
		rif_m_CustomParam = NULL;
	}
	if (rif_m_DownloadImage != NULL)
	{
		OSAL_vMemoryFree(rif_m_DownloadImage);
		rif_m_DownloadImage = NULL;
	}
	/* Not used with radio_if because we need to create another OSAL */
#endif


	OSAL_s32ThreadWait(10000); 	
	
	/* It's solution before OSAL will create */
	rif_stop_rif(); 
	
	OSAL_s32ThreadWait(10000);
	/****************/

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_capabilities
 *
 **********************************/
/*!
 * \brief		handle an etal_get_capabilities rimw command
 * \details		call etal_get_capabilities api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_capabilities(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	EtalHwCapabilities *pCapabilities = NULL, **ppCapabilities = &pCapabilities;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ppEtalHwCapabilities(&cmd_payload, &cmd_len, &ppCapabilities);

	if (ret == OSAL_OK)
	{
		/* call etal_get_capabilities */
		retval = etal_get_capabilities(ppCapabilities);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add EtalHwCapabilities** to response payload */
		ret = rif_etalapi_cnv_resp_add_ppEtalHwCapabilities(resp, rlen, ppCapabilities);
	}

	/* free pCapabilities memory */
	if (pCapabilities != NULL)
	{
		OSAL_vMemoryFree(pCapabilities);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_config_receiver
 *
 **********************************/
/*!
 * \brief		handle an etal_config_receiver rimw command
 * \details		call etal_config_receiver api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_config_receiver(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver, *hReceiver_p = &hReceiver;
	EtalReceiverAttr receiverAttr, *receiverAttr_p = &receiverAttr;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_pETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver_p);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_pEtalReceiverAttr(&cmd_payload, &cmd_len, &receiverAttr_p);
	}

	if (ret == OSAL_OK)
	{
		/* call etal_config_receiver */
		retval = etal_config_receiver(hReceiver_p, receiverAttr_p);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add *ETAL_HANDLE to response payload */
		ret = rif_etalapi_cnv_resp_add_pETAL_HANDLE(resp, rlen, hReceiver_p);
	}

	if (ret == OSAL_OK)
	{
		/* add *EtalReceiverAttr to response payload */
		/* ret = rif_etalapi_cnv_resp_add_pEtalReceiverAttr(resp, rlen, receiverAttr_p); */
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_destroy_receiver
 *
 **********************************/
/*!
 * \brief		handle an etal_destroy_receiver rimw command
 * \details		call etal_destroy_receiver api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_destroy_receiver(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver, *hReceiver_p = &hReceiver;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_pETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver_p);

	if (ret == OSAL_OK)
	{
		/* call etal_destroy_receiver */
		retval = etal_destroy_receiver(hReceiver_p);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add *ETAL_HANDLE to response payload */
		ret = rif_etalapi_cnv_resp_add_pETAL_HANDLE(resp, rlen, hReceiver_p);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_config_datapath
 *
 **********************************/
/*!
 * \brief		handle an etal_config_datapath rimw command
 * \details		call etal_config_datapath api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_config_datapath(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE pDatapath, *pDatapath_p = &pDatapath;
	EtalDataPathAttr pDatapathAttr, *pDatapathAttr_p = &pDatapathAttr;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_pETAL_HANDLE(&cmd_payload, &cmd_len, &pDatapath_p);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_pEtalDataPathAttr(&cmd_payload, &cmd_len, &pDatapathAttr_p);
	}

	if (ret == OSAL_OK)
	{
		/* call etal_config_datapath */
		retval = etal_config_datapath(pDatapath_p, pDatapathAttr_p);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add *ETAL_HANDLE to response payload */
		ret = rif_etalapi_cnv_resp_add_pETAL_HANDLE(resp, rlen, pDatapath_p);
	}

	if (ret == OSAL_OK)
	{
		/* add *EtalDataPathAttr to response payload */
		/* ret = rif_etalapi_cnv_resp_add_pEtalDataPathAttr(resp, rlen, pDatapathAttr_p); */
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_destroy_datapath
 *
 **********************************/
/*!
 * \brief		handle an etal_destroy_datapath rimw command
 * \details		call etal_destroy_datapath api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_destroy_datapath(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE pDatapath, *pDatapath_p = &pDatapath;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_pETAL_HANDLE(&cmd_payload, &cmd_len, &pDatapath_p);

	if (ret == OSAL_OK)
	{
		/* call etal_destroy_datapath */
		retval = etal_destroy_datapath(pDatapath_p);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add *ETAL_HANDLE to response payload */
		ret = rif_etalapi_cnv_resp_add_pETAL_HANDLE(resp, rlen, pDatapath_p);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_config_audio_path
 *
 **********************************/
/*!
 * \brief		handle an etal_config_audio_path rimw command
 * \details		call etal_config_audio_path api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_config_audio_path(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	tU32 tunerIndex = 0;
	EtalAudioInterfTy intf;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &tunerIndex);
	ret = rif_etalapi_cnv_type_get_EtalAudioInterfTy(&cmd_payload, &cmd_len, &intf);

	if (ret == OSAL_OK)
	{
		/* call etal_config_audio_path */
		retval = etal_config_audio_path(tunerIndex, intf);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_audio_select
 *
 **********************************/
/*!
 * \brief		handle an etal_audio_select rimw command
 * \details		call etal_audio_select api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_audio_select(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	EtalAudioSourceTy src;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	ret = rif_etalapi_cnv_type_get_EtalAudioSourceTy(&cmd_payload, &cmd_len, &src);

	if (ret == OSAL_OK)
	{
		/* call etal_audio_select */
		retval = etal_audio_select(hReceiver, src);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_mute
 *
 **********************************/
/*!
 * \brief		handle an etal_mute rimw command
 * \details		call etal_mute api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_mute(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tBool muteFlag;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	ret = rif_etalapi_cnv_type_get_tBool(&cmd_payload, &cmd_len, &muteFlag);

	if (ret == OSAL_OK)
	{
		/* call etal_mute */
		retval = etal_mute(hReceiver, muteFlag);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_force_mono
 *
 **********************************/
/*!
 * \brief		handle an etal_force_mono rimw command
 * \details		call etal_force_mono api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_force_mono(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tBool forceMonoFlag;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	ret = rif_etalapi_cnv_type_get_tBool(&cmd_payload, &cmd_len, &forceMonoFlag);

	if (ret == OSAL_OK)
	{
		/* call etal_force_mono */
		retval = etal_force_mono(hReceiver, forceMonoFlag);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_reinitialize
 *
 **********************************/
/*!
 * \brief		handle an etal_reinitialize rimw command
 * \details		call etal_reinitialize api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_reinitialize(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;
	EtalNVMLoadConfig etalNVMLoadConfig;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);
	
	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_EtalNVMLoadConfig(&cmd_payload, &cmd_len, &etalNVMLoadConfig);
	if (ret == OSAL_OK)
	{
		/* call etal_reinitialize */
		retval = etal_reinitialize(etalNVMLoadConfig);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_xtal_alignment
 *
 **********************************/
/*!
 * \brief		handle an etal_xtal_alignment rimw command
 * \details		call etal_xtal_alignment api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_xtal_alignment(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 calculatedAlignment_v = 0, *calculatedAlignment = &calculatedAlignment_v;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&calculatedAlignment);
		/*ret = rif_etalapi_cnv_type_get_ptU32(&cmd_payload, &cmd_len, &calculatedAlignment);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_xtal_alignment */
		retval = etal_xtal_alignment(hReceiver, calculatedAlignment);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add tU32 *calculatedAlignment to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, calculatedAlignment);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_version
 *
 **********************************/
/*!
 * \brief		handle an etal_get_version rimw command
 * \details		call etal_get_version api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_version(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	EtalVersion version, *ver = &version;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	OSAL_pvMemorySet(ver, 0, sizeof(EtalVersion));
	ret = rif_etalapi_cnv_type_get_pEtalVersion(&cmd_payload, &cmd_len, &ver);

	if (ret == OSAL_OK)
	{
		/* call etal_get_version */
		retval = etal_get_version(ver);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add EtalVersion *ver to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalVersion(resp, rlen, ver);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_debug_config_audio_alignment
 *
 **********************************/
/*!
 * \brief		handle an etal_debug_config_audio_alignment rimw command
 * \details		call etal_debug_config_audio_alignment api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_debug_config_audio_alignment(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	EtalAudioAlignmentAttr alignmentParamsAttr, *alignmentParams = &alignmentParamsAttr;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_pEtalAudioAlignmentAttr(&cmd_payload, &cmd_len, &alignmentParams);

	if (ret == OSAL_OK)
	{
		/* call etal_debug_config_audio_alignment */
		retval = etal_debug_config_audio_alignment(alignmentParams);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add EtalAudioAlignmentAttr *alignmentParams to response payload */
		/*ret = rif_etalapi_cnv_resp_add_pEtalAudioAlignmentAttr(resp, rlen, alignmentParams);*/
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_init_status
 *
 **********************************/
/*!
 * \brief		handle an etal_get_init_status rimw command
 * \details		call etal_get_init_status api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_init_status(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	EtalInitStatus InitStatus, *status = &InitStatus;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	OSAL_pvMemorySet(status, 0, sizeof(EtalInitStatus));
	ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&status);
	/*ret = rif_etalapi_cnv_type_get_pEtalInitStatus(&cmd_payload, &cmd_len, &status);*/

	if (ret == OSAL_OK)
	{
		/* call etal_get_init_status */
		retval = etal_get_init_status(status);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add EtalInitStatus *status to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalInitStatus(resp, rlen, status);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_tune_receiver
 *
 **********************************/
/*!
 * \brief		handle an etal_tune_receiver rimw command
 * \details		call etal_tune_receiver api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_tune_receiver(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 Frequency;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;
	tU8 cpt;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &Frequency);
	}

	if (ret == OSAL_OK)
	{
		/* call etal_tune_receiver */
		/* do while for limited ETAL_RET_NO_DATA */
		cpt = 0;
		do{
			OSAL_s32ThreadWait(1000); 
			retval = etal_tune_receiver(hReceiver, Frequency);
			cpt++;
		}while(retval == ETAL_RET_NO_DATA && cpt != 3);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));		
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_receiver_frequency
 *
 **********************************/
/*!
 * \brief		handle an etal_get_receiver_frequency rimw command
 * \details		call etal_get_receiver_frequency api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_receiver_frequency(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 freq = ETAL_INVALID_FREQUENCY, *pFrequency = &freq;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&pFrequency);
		/*ret = rif_etalapi_cnv_type_get_ptU32(&cmd_payload, &cmd_len, &pFrequency);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_get_receiver_frequency */
		retval = etal_get_receiver_frequency(hReceiver, pFrequency);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add tU32 * to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, pFrequency);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_seek_start_manual
 *
 **********************************/
/*!
 * \brief		handle an etal_seek_start_manual rimw command
 * \details		call etal_seek_start_manual api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_seek_start_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	etalSeekDirectionTy direction;
	tU32 Frequency = ETAL_INVALID_FREQUENCY, step, *freq = &Frequency;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_etalSeekDirectionTy(&cmd_payload, &cmd_len, &direction);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &step);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&freq);
		/*ret = rif_etalapi_cnv_type_get_ptU32(&cmd_payload, &cmd_len, &freq);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_seek_start_manual */
		retval = etal_seek_start_manual(hReceiver, direction, step, freq);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add tU32 * to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, freq);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_seek_continue_manual
 *
 **********************************/
/*!
 * \brief		handle an etal_seek_continue_manual rimw command
 * \details		call etal_seek_continue_manual api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_seek_continue_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 frequency = ETAL_INVALID_FREQUENCY, *freq = &frequency;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&freq);
		/*ret = rif_etalapi_cnv_type_get_ptU32(&cmd_payload, &cmd_len, &Frequency);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_seek_continue_manual */
		retval = etal_seek_continue_manual(hReceiver, freq);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add tU32 * to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, freq);
	}

	return ret;
}

/***********************************
 *
 * etal_seek_stop_manual
 *
 **********************************/
/*!
 * \brief		handle an etal_seek_stop_manual rimw command
 * \details		call etal_seek_stop_manual api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_seek_stop_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	etalSeekAudioTy exitSeekAction;
	tU32 Frequency = ETAL_INVALID_FREQUENCY, *freq = &Frequency;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_etalSeekAudioTy(&cmd_payload, &cmd_len, &exitSeekAction);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&freq);
		/*ret = rif_etalapi_cnv_type_get_ptU32(&cmd_payload, &cmd_len, &freq);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_seek_stop_manual */
		retval = etal_seek_stop_manual(hReceiver, exitSeekAction, freq);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add tU32 * to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, freq);
	}

	return ret;
}

/***********************************
 *
 * etal_seek_get_status_manual
 *
 **********************************/
/*!
 * \brief		handle an etal_seek_get_status_manual rimw command
 * \details		call etal_seek_get_status_manual api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_seek_get_status_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	EtalSeekStatus seekStatusV, *seekStatus = &seekStatusV;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(seekStatus, 0, sizeof(EtalSeekStatus));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&seekStatus);
		/*ret = rif_etalapi_cnv_type_get_pEtalSeekStatus(&cmd_payload, &cmd_len, &seekStatus);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_seek_get_status_manual */
		retval = etal_seek_get_status_manual(hReceiver, seekStatus);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalSeekStatus * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalSeekStatus(resp, rlen, seekStatus);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_change_band_receiver
 *
 **********************************/
/*!
 * \brief		handle an etal_change_band_receiver rimw command
 * \details		call etal_change_band_receiver api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_change_band_receiver(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	EtalFrequencyBand band;
	tU32 fmin, fmax;
	EtalProcessingFeatures processingFeatures;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_EtalFrequencyBand(&cmd_payload, &cmd_len, &band);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &fmin);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &fmax);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_EtalProcessingFeatures(&cmd_payload, &cmd_len, &processingFeatures);
	}

	if (ret == OSAL_OK)
	{
		/* call etal_change_band_receiver */
		retval = etal_change_band_receiver(hReceiver, band, fmin, fmax, processingFeatures);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_tune_receiver_async
 *
 **********************************/
/*!
 * \brief		handle an etal_tune_receiver_async rimw command
 * \details		call etal_tune_receiver_async api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_tune_receiver_async(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 Frequency;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &Frequency);
	}

	if (ret == OSAL_OK)
	{
#if defined (CONFIG_ETAL_HAVE_TUNE_ASYNC) || defined (CONFIG_ETAL_HAVE_TUNE_BOTH) || defined (CONFIG_ETAL_HAVE_ALL_API)
		/* call etal_tune_receiver_async */
		retval = etal_tune_receiver_async(hReceiver, Frequency);
#else
		ret = OSAL_ERROR_NOT_IMPLEMENTED;
#endif // defined (CONFIG_ETAL_HAVE_TUNE_ASYNC) || defined (CONFIG_ETAL_HAVE_TUNE_BOTH) || defined (CONFIG_ETAL_HAVE_ALL_API)
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_current_ensemble
 *
 **********************************/
/*!
 * \brief		handle an etal_get_current_ensemble rimw command
 * \details		call etal_get_current_ensemble api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_current_ensemble(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 UEId = 0, *pUEId = &UEId;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&pUEId);
		/*ret = rif_etalapi_cnv_type_get_ptU32(&cmd_payload, &cmd_len, &pUEId);*/
	}

	if (ret == OSAL_OK)
	{
#if (defined (CONFIG_ETAL_HAVE_ADVTUNE) || defined (CONFIG_ETAL_HAVE_ALL_API)) && (defined(CONFIG_ETAL_SUPPORT_DCOP_MDR))
		/* call etal_get_current_ensemble */
		retval = etal_get_current_ensemble(hReceiver, pUEId);
#else
		ret = OSAL_ERROR_NOT_IMPLEMENTED;
#endif // (CONFIG_ETAL_HAVE_ADVTUNE || CONFIG_ETAL_HAVE_ALL_API) && CONFIG_ETAL_SUPPORT_DCOP_MDR
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add tU32 * to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, pUEId);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_service_select_audio
 *
 **********************************/
/*!
 * \brief		handle an etal_service_select_audio rimw command
 * \details		call etal_service_select_audio api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_service_select_audio(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	EtalServiceSelectMode mode;
	tU32 UEId, service;
	tSInt sc, subch;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_EtalServiceSelectMode(&cmd_payload, &cmd_len, &mode);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &UEId);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &service);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tSInt(&cmd_payload, &cmd_len, &sc);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tSInt(&cmd_payload, &cmd_len, &subch);
	}

	if (ret == OSAL_OK)
	{
#if (defined (CONFIG_ETAL_HAVE_ADVTUNE) || defined (CONFIG_ETAL_HAVE_ALL_API)) && (defined(CONFIG_ETAL_SUPPORT_DCOP))
		/* call etal_service_select_audio */
		retval = etal_service_select_audio(hReceiver, mode, UEId, service, sc, subch);
#else
		ret = OSAL_ERROR_NOT_IMPLEMENTED;
#endif // (CONFIG_ETAL_HAVE_ADVTUNE || CONFIG_ETAL_HAVE_ALL_API) && CONFIG_ETAL_SUPPORT_DCOP
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_service_select_data
 *
 **********************************/
/*!
 * \brief		handle an etal_service_select_data rimw command
 * \details		call etal_service_select_data api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_service_select_data(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hDatapath;
	EtalServiceSelectMode mode;
	EtalServiceSelectSubFunction type;
	tU32 UEId, service;
	tSInt sc, subch;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hDatapath);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_EtalServiceSelectMode(&cmd_payload, &cmd_len, &mode);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_EtalServiceSelectSubFunction(&cmd_payload, &cmd_len, &type);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &UEId);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &service);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tSInt(&cmd_payload, &cmd_len, &sc);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tSInt(&cmd_payload, &cmd_len, &subch);
	}

	if (ret == OSAL_OK)
	{
#if (defined (CONFIG_ETAL_HAVE_ADVTUNE) || defined (CONFIG_ETAL_HAVE_ALL_API)) && (defined(CONFIG_ETAL_SUPPORT_DCOP))
		/* call etal_service_select_data */
		retval = etal_service_select_data(hDatapath, mode, type, UEId, service, sc, subch);
#else
		ret = OSAL_ERROR_NOT_IMPLEMENTED;
#endif // (CONFIG_ETAL_HAVE_ADVTUNE || CONFIG_ETAL_HAVE_ALL_API) && CONFIG_ETAL_SUPPORT_DCOP
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_event_FM_stereo_start
 *
 **********************************/
/*!
 * \brief		handle an etal_event_FM_stereo_start rimw command
 * \details		call etal_event_FM_stereo_start api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_event_FM_stereo_start(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);

	if (ret == OSAL_OK)
	{
		/* call etal_event_FM_stereo_start */
		retval = etal_event_FM_stereo_start(hReceiver);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_etal_event_FM_stereo_stop
 *
 **********************************/
/*!
 * \brief		handle an etal_event_FM_stereo_stop rimw command
 * \details		call etal_event_FM_stereo_stop api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_event_FM_stereo_stop(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);

	if (ret == OSAL_OK)
	{
		/* call etal_event_FM_stereo_stop */
		retval = etal_event_FM_stereo_stop(hReceiver);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_AF_start
 *
 **********************************/
/*!
 * \brief		handle an etal_AF_start rimw command
 * \details		call etal_AF_start api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_AF_start(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	etalAFModeTy AFMode;
	tU32 alternateFrequency = ETAL_INVALID_FREQUENCY, antennaSelection = 0;
	EtalBcastQualityContainer bcastqual, *result = &bcastqual;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_etalAFModeTy(&cmd_payload, &cmd_len, &AFMode);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &alternateFrequency);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &antennaSelection);
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&bcastqual, 0, sizeof(EtalBcastQualityContainer));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&result);
		/*ret = rif_etalapi_cnv_type_get_pEtalBcastQualityContainer(&cmd_payload, &cmd_len, &result);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_AF_start */
		retval = etal_AF_start(hReceiver, AFMode, alternateFrequency, antennaSelection, result);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalBcastQualityContainer * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalBcastQualityContainer(resp, rlen, result);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_AF_end
 *
 **********************************/
/*!
 * \brief		handle an etal_AF_end rimw command
 * \details		call etal_AF_end api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_AF_end(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 frequency = ETAL_INVALID_FREQUENCY;
	EtalBcastQualityContainer bcastqual, *result = &bcastqual;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &frequency);
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&bcastqual, 0, sizeof(EtalBcastQualityContainer));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&result);
		/*ret = rif_etalapi_cnv_type_get_pEtalBcastQualityContainer(&cmd_payload, &cmd_len, &result);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_AF_end */
		retval = etal_AF_end(hReceiver, frequency, result);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalBcastQualityContainer * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalBcastQualityContainer(resp, rlen, result);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_AF_switch
 *
 **********************************/
/*!
 * \brief		handle an etal_AF_switch rimw command
 * \details		call etal_AF_switch api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_AF_switch(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 alternateFrequency = ETAL_INVALID_FREQUENCY;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &alternateFrequency);
	}

	if (ret == OSAL_OK)
	{
		/* call etal_AF_switch */
		retval = etal_AF_switch(hReceiver, alternateFrequency);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_AF_check
 *
 **********************************/
/*!
 * \brief		handle an etal_AF_check rimw command
 * \details		call etal_AF_check api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_AF_check(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 alternateFrequency = ETAL_INVALID_FREQUENCY, antennaSelection = 0;
	EtalBcastQualityContainer bcastqual, *p = &bcastqual;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &alternateFrequency);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &antennaSelection);
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&bcastqual, 0, sizeof(EtalBcastQualityContainer));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&p);
		/*ret = rif_etalapi_cnv_type_get_pEtalBcastQualityContainer(&cmd_payload, &cmd_len, &p);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_AF_check */
		retval = etal_AF_check(hReceiver, alternateFrequency, antennaSelection, p);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalBcastQualityContainer * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalBcastQualityContainer(resp, rlen, p);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_AF_search_manual
 *
 **********************************/
/*!
 * \brief		handle an etal_AF_search_manual rimw command
 * \details		call etal_AF_search_manual api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_AF_search_manual(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	#define RIF_ETALAPI_CNV_AFLIST_MAX  96  /* should not be too big else will create stack size issue */
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 i, nbOfAF = 0, antennaSelection = 0, af_list[RIF_ETALAPI_CNV_AFLIST_MAX], *AFList = af_list;
	EtalBcastQualityContainer *bcastqual = NULL, *AFQualityList = NULL;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &antennaSelection);
	}
	if (ret == OSAL_OK)
	{
		/* get nbOfAF before AFList to know the size of AFList */
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &nbOfAF);
	}
	if (ret == OSAL_OK)
	{
		if (nbOfAF > RIF_ETALAPI_CNV_AFLIST_MAX)
		{
			ret = OSAL_ERROR_INVALID_PARAM;
		}
		else
		{
			if (nbOfAF != 0)
			{
				bcastqual = (EtalBcastQualityContainer *)OSAL_pvMemoryAllocate(nbOfAF * sizeof(EtalBcastQualityContainer));
			}
			else
			{
				/* allocation at least one EtalBcastQualityContainer size to avoid any memory corruption */
				bcastqual = (EtalBcastQualityContainer *)OSAL_pvMemoryAllocate(     1 * sizeof(EtalBcastQualityContainer));
			}
			AFQualityList = bcastqual;
			if (bcastqual == NULL)
			{
				ret = OSAL_ERROR_UNEXPECTED;
			}
		}
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&AFList);
	}
	if ((ret == OSAL_OK) && (AFList != NULL))
	{
		for(i = 0; i < nbOfAF; i++)
		{
			if (ret == OSAL_OK)
			{
				ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &AFList[i]);
			}
		}
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(bcastqual, 0, nbOfAF * sizeof(EtalBcastQualityContainer));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&AFQualityList);
	}
	/*if ((ret == OSAL_OK) && (AFQualityList != NULL))
	{
		for(i = 0; i < nbOfAF; i++)
		{
			if (ret == OSAL_OK)
			{
				ret = rif_etalapi_cnv_type_get_EtalBcastQualityContainer(&cmd_payload, &cmd_len, &(AFQualityList[i]));
			}
		}
	}*/

	if (ret == OSAL_OK)
	{
		/* call etal_AF_search_manual */
		retval = etal_AF_search_manual(hReceiver, antennaSelection, AFList, nbOfAF, AFQualityList);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalBcastQualityContainer * to response payload */
		ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)AFQualityList);
	}
	if ((ret == OSAL_OK) && (AFQualityList != NULL))
	{
		for(i = 0; i < nbOfAF; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add EtalBcastQualityContainer to response payload */
				ret = rif_etalapi_cnv_resp_add_EtalBcastQualityContainer(resp, rlen, &(AFQualityList[i]));
			}
		}
	}

	/* free allocated buffer */
	if (bcastqual == NULL)
	{
		OSAL_vMemoryFree(bcastqual);
	}
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_start_RDS
 *
 **********************************/
/*!
 * \brief		handle an etal_start_RDS rimw command
 * \details		call etal_start_RDS api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_start_RDS(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tBool forceFastPi;
	tU8 numPi;
	EtalRDSRBDSModeTy mode;
	tU8 errThresh;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tBool(&cmd_payload, &cmd_len, &forceFastPi);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU8(&cmd_payload, &cmd_len, &numPi);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_EtalRDSRBDSModeTy(&cmd_payload, &cmd_len, &mode);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU8(&cmd_payload, &cmd_len, &errThresh);
	}

	if (ret == OSAL_OK)
	{
		/* call etal_start_RDS */
		retval = etal_start_RDS(hReceiver, forceFastPi, numPi, mode, errThresh);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_stop_RDS
 *
 **********************************/
/*!
 * \brief		handle an etal_stop_RDS rimw command
 * \details		call etal_stop_RDS api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_stop_RDS(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);

	if (ret == OSAL_OK)
	{
		/* call etal_stop_RDS */
		retval = etal_stop_RDS(hReceiver);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_ensemble_list
 *
 **********************************/
/*!
 * \brief		handle an etal_get_ensemble_list rimw command
 * \details		call etal_get_ensemble_list api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_ensemble_list(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	EtalEnsembleList vEnsembleList, *pEnsembleList = &vEnsembleList;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	OSAL_pvMemorySet(&vEnsembleList, 0, sizeof(EtalEnsembleList));
	ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&pEnsembleList);
	/*ret = rif_etalapi_cnv_type_get_pEtalEnsembleList(&cmd_payload, &cmd_len, &pEnsembleList);*/

	if (ret == OSAL_OK)
	{
#if (defined(CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined(CONFIG_ETAL_HAVE_ALL_API)) && defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
		/* call etal_get_ensemble_list */
		retval = etal_get_ensemble_list(pEnsembleList);
#else
		ret = OSAL_ERROR_NOT_IMPLEMENTED;
#endif // (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB || CONFIG_ETAL_HAVE_ALL_API) && CONFIG_ETAL_SUPPORT_DCOP_MDR
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalBcastQualityContainer * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalEnsembleList(resp, rlen, pEnsembleList);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_ensemble_data
 *
 **********************************/
/*!
 * \brief		handle an etal_get_ensemble_data rimw command
 * \details		call etal_get_ensemble_data api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_ensemble_data(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	tU32 eid = 0, i;
	tU16 vbitmap = 0, *bitmap = &vbitmap;
	tU8 vcharset = 0, *charset = &vcharset;
	tChar vlabel[ETAL_DEF_MAX_LABEL_LEN + 1], *label = vlabel;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &eid);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&charset);
		/*ret = rif_etalapi_cnv_type_get_ptU8(&cmd_payload, &cmd_len, &charset);*/
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&vlabel, 0, ETAL_DEF_MAX_LABEL_LEN + 1);
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&label);
		/*ret = rif_etalapi_cnv_type_get_ptChar(&cmd_payload, &cmd_len, &label);*/
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&bitmap);
		/*ret = rif_etalapi_cnv_type_get_ptU16(&cmd_payload, &cmd_len, &bitmap);*/
	}

	if (ret == OSAL_OK)
	{
#if (defined(CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined(CONFIG_ETAL_HAVE_ALL_API)) && defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
		/* call etal_get_ensemble_data */
		retval = etal_get_ensemble_data(eid, charset, label, bitmap);
#else
		ret = OSAL_ERROR_NOT_IMPLEMENTED;
#endif // (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB || CONFIG_ETAL_HAVE_ALL_API) && CONFIG_ETAL_SUPPORT_DCOP_MDR
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add tU8 * to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU8(resp, rlen, charset);
	}
	if (ret == OSAL_OK)
	{
		/* add tUChar * to response payload */
		ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)label);
		if (label != NULL)
		{
			for(i = 0; i < ETAL_DEF_MAX_LABEL_LEN; i++)
			{
				ret = rif_etalapi_cnv_resp_add_tChar(resp, rlen, label[i]);
			}
		}
	}
	if (ret == OSAL_OK)
	{
		/* add tU8 * to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU16(resp, rlen, bitmap);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_service_list
 *
 **********************************/
/*!
 * \brief		handle an etal_get_service_list rimw command
 * \details		call etal_get_service_list api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_service_list(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 eid = 0;
	tBool bGetAudioServices = 0, bGetDataServices = 0;
	EtalServiceList vServiceList, *pServiceList = &vServiceList;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &eid);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tBool(&cmd_payload, &cmd_len, &bGetAudioServices);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tBool(&cmd_payload, &cmd_len, &bGetDataServices);
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&vServiceList, 0, sizeof(EtalServiceList));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&pServiceList);
		/*ret = rif_etalapi_cnv_type_get_pEtalServiceList(&cmd_payload, &cmd_len, &pServiceList);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_get_service_list */
		retval = etal_get_service_list(hReceiver, eid, bGetAudioServices, bGetDataServices, pServiceList);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalServiceList * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalServiceList(resp, rlen, pServiceList);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_specific_service_data_DAB
 *
 **********************************/
/*!
 * \brief		handle an etal_get_specific_service_data_DAB rimw command
 * \details		call etal_get_specific_service_data_DAB api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_specific_service_data_DAB(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	tU32 eid = 0, sid = 0;
	EtalServiceInfo vserv_info, *serv_info = &vserv_info;
	EtalServiceComponentList vsclist, *sclist = &vsclist;
	tVoid *dummy = NULL;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &eid);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &sid);
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&vserv_info, 0, sizeof(EtalServiceInfo));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&serv_info);
		/*ret = rif_etalapi_cnv_type_get_pEtalServiceInfo(&cmd_payload, &cmd_len, &serv_info);*/
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&vsclist, 0, sizeof(EtalServiceInfo));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&sclist);
		/*ret = rif_etalapi_cnv_type_get_pEtalServiceComponentList(&cmd_payload, &cmd_len, &sclist);*/
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&dummy);
	}

	if (ret == OSAL_OK)
	{
#if (defined(CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined(CONFIG_ETAL_HAVE_ALL_API)) && defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
		/* call etal_get_specific_service_data_DAB */
		retval = etal_get_specific_service_data_DAB(eid, sid, serv_info, sclist, dummy);
#else
		ret = OSAL_ERROR_NOT_IMPLEMENTED;
#endif // (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB || CONFIG_ETAL_HAVE_ALL_API) && CONFIG_ETAL_SUPPORT_DCOP_MDR
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalServiceInfo * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalServiceInfo(resp, rlen, serv_info);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalServiceComponentList * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalServiceComponentList(resp, rlen, sclist);
	}
	if (ret == OSAL_OK)
	{
		/* add void * to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, dummy);
	}
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_fic
 *
 **********************************/
/*!
 * \brief		handle an etal_get_fic rimw command
 * \details		call etal_get_fic api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_fic(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);

	if (ret == OSAL_OK)
	{
#if (defined(CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined(CONFIG_ETAL_HAVE_ALL_API)) && defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
		/* call etal_get_fic */
		retval = etal_get_fic(hReceiver);
#else
		ret = OSAL_ERROR_NOT_IMPLEMENTED;
#endif // (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB || CONFIG_ETAL_HAVE_ALL_API) && CONFIG_ETAL_SUPPORT_DCOP_MDR
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_enable_data_service
 *
 **********************************/
/*!
 * \brief		handle an etal_enable_data_service rimw command
 * \details		call etal_enable_data_service api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_enable_data_service(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 ServiceBitmap = 0, vEnabledServiceBitmap = 0, *EnabledServiceBitmap = &vEnabledServiceBitmap;
	EtalDataServiceParam ServiceParameters;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &ServiceBitmap);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tVoid *)&EnabledServiceBitmap);
		/*ret = rif_etalapi_cnv_type_get_ptU32(&cmd_payload, &cmd_len, &EnabledServiceBitmap);*/
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_EtalDataServiceParam(&cmd_payload, &cmd_len, &ServiceParameters);
	}

	if (ret == OSAL_OK)
	{
		/* call etal_enable_data_service */
		retval = etal_enable_data_service(hReceiver, ServiceBitmap, EnabledServiceBitmap, ServiceParameters);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add tU32 * to response payload */
		ret = rif_etalapi_cnv_resp_add_ptU32(resp, rlen, EnabledServiceBitmap);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_disable_data_service
 *
 **********************************/
/*!
 * \brief		handle an etal_disable_data_service rimw command
 * \details		call etal_disable_data_service api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_disable_data_service(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU32 ServiceBitmap;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &ServiceBitmap);
	}

	if (ret == OSAL_OK)
	{
		/* call etal_disable_data_service */
		retval = etal_disable_data_service(hReceiver, ServiceBitmap);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_setup_PSD
 *
 **********************************/
/*!
 * \brief		handle an etal_setup_PSD rimw command
 * \details		call etal_setup_PSD api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_setup_PSD(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	tU16 PSDServiceEnableBitmap = 0;
	EtalPSDLength vConfigLenSet, vConfigLenGet, *pConfigLenSet = &vConfigLenSet, *pConfigLenGet = &vConfigLenGet;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU16(&cmd_payload, &cmd_len, &PSDServiceEnableBitmap);
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&vConfigLenSet, 0, sizeof(EtalPSDLength));
		/*ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tVoid *)&pConfigLenSet);*/
		ret = rif_etalapi_cnv_type_get_pEtalPSDLength(&cmd_payload, &cmd_len, &pConfigLenSet);
	}
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&vConfigLenGet, 0, sizeof(EtalPSDLength));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tVoid *)&pConfigLenGet);
		/*ret = rif_etalapi_cnv_type_get_pEtalPSDLength(&cmd_payload, &cmd_len, &pConfigLenGet);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_setup_PSD */
		retval = etal_setup_PSD(hReceiver, PSDServiceEnableBitmap, pConfigLenSet, pConfigLenGet);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalPSDLength * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalPSDLength(resp, rlen, pConfigLenGet);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_reception_quality
 *
 **********************************/
/*!
 * \brief		handle an etal_get_reception_quality rimw command
 * \details		call etal_get_reception_quality api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_reception_quality(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	EtalBcastQualityContainer bcastqual, *pBcastQuality = &bcastqual;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&bcastqual, 0, sizeof(EtalBcastQualityContainer));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&pBcastQuality);
		/*ret = rif_etalapi_cnv_type_get_pEtalBcastQualityContainer(&cmd_payload, &cmd_len, &pBcastQuality);*/
	}

	if (ret == OSAL_OK)
	{
		/* call etal_get_reception_quality */
		retval = etal_get_reception_quality(hReceiver, pBcastQuality);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalBcastQualityContainer * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalBcastQualityContainer(resp, rlen, pBcastQuality);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_channel_quality
 *
 **********************************/
/*!
 * \brief		handle an etal_get_channel_quality rimw command
 * \details		call etal_get_channel_quality api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_channel_quality(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	#define RIF_ETALAPI_CNV_NB_BCASTQUALITY  2
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	EtalBcastQualityContainer bcastqual[RIF_ETALAPI_CNV_NB_BCASTQUALITY], *pBcastQuality = bcastqual;
	tU32 i;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(bcastqual, 0, RIF_ETALAPI_CNV_NB_BCASTQUALITY * sizeof(EtalBcastQualityContainer));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&pBcastQuality);
	}
	/*if ((ret == OSAL_OK) && (pBcastQuality != NULL))
	{
		for(i = 0; i < RIF_ETALAPI_CNV_NB_BCASTQUALITY; i++)
		{
			if (ret == OSAL_OK)
			{
				ret = rif_etalapi_cnv_type_get_EtalBcastQualityContainer(&cmd_payload, &cmd_len, &(pBcastQuality[i]));
			}
		}
	}*/

	if (ret == OSAL_OK)
	{
		/* call etal_get_channel_quality */
		retval = etal_get_channel_quality(hReceiver, pBcastQuality);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalBcastQualityContainer * to response payload */
		ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)pBcastQuality);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalBcastQualityContainer * to response payload */
		ret = rif_etalapi_cnv_resp_add_EtalBcastQualityContainer(resp, rlen, &(pBcastQuality[0]));
	}
	if ((ret == OSAL_OK) && (pBcastQuality != NULL))
	{
		for(i = 0; i < RIF_ETALAPI_CNV_NB_BCASTQUALITY; i++)
		{
			if (ret == OSAL_OK)
			{
				/* add EtalBcastQualityContainer to response payload */
				ret = rif_etalapi_cnv_resp_add_EtalBcastQualityContainer(resp, rlen, &(pBcastQuality[1]));
			}
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_config_reception_quality_monitor
 *
 **********************************/
/*!
 * \brief		handle an etal_config_reception_quality_monitor rimw command
 * \details		call etal_config_reception_quality_monitor api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_config_reception_quality_monitor(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE MonitorHandle = ETAL_INVALID_HANDLE, *pMonitorHandle = &MonitorHandle;
	EtalBcastQualityMonitorAttr MonitorAttr, *pMonitor = &MonitorAttr;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_pETAL_HANDLE(&cmd_payload, &cmd_len, &pMonitorHandle);
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&MonitorAttr, 0, sizeof(EtalBcastQualityMonitorAttr));
		ret = rif_etalapi_cnv_type_get_pEtalBcastQualityMonitorAttr(&cmd_payload, &cmd_len, &pMonitor);
	}

	if (ret == OSAL_OK)
	{
		/* call etal_config_reception_quality_monitor */
		retval = etal_config_reception_quality_monitor(pMonitorHandle, pMonitor);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add *ETAL_HANDLE to response payload */
		ret = rif_etalapi_cnv_resp_add_pETAL_HANDLE(resp, rlen, pMonitorHandle);
	}

	if (ret == OSAL_OK)
	{
		/* add *EtalBcastQualityMonitorAttr to response payload */
		/*ret = rif_etalapi_cnv_resp_add_pEtalBcastQualityMonitorAttr(resp, rlen, pMonitor);*/
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_destroy_reception_quality_monitor
 *
 **********************************/
/*!
 * \brief		handle an etal_destroy_reception_quality_monitor rimw command
 * \details		call etal_destroy_reception_quality_monitor api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_destroy_reception_quality_monitor(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE MonitorHandle = ETAL_INVALID_HANDLE, *pMonitorHandle = &MonitorHandle;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_pETAL_HANDLE(&cmd_payload, &cmd_len, &pMonitorHandle);

	if (ret == OSAL_OK)
	{
		/* call etal_destroy_reception_quality_monitor */
		retval = etal_destroy_reception_quality_monitor(pMonitorHandle);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	if (ret == OSAL_OK)
	{
		/* add *ETAL_HANDLE to response payload */
		ret = rif_etalapi_cnv_resp_add_pETAL_HANDLE(resp, rlen, pMonitorHandle);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_receiver_alive
 *
 **********************************/
/*!
 * \brief		handle an etal_receiver_alive rimw command
 * \details		call etal_receiver_alive api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_receiver_alive(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);

	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);

	if (ret == OSAL_OK)
	{
		/* call etal_receiver_alive */
		retval = etal_receiver_alive(hReceiver);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}

	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}

	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_get_CF_data
 *
 **********************************/
/*!
 * \brief		handle an etal_get_CF_data rimw command
 * \details		call etal_get_CF_data api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_get_CF_data(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len, nbOfAvarage, period;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	EtalCFDataContainer CFdataCont, *pResp = &CFdataCont;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);
	
	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&CFdataCont, 0, sizeof(EtalCFDataContainer));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&pResp);
	}
	
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &nbOfAvarage);
	}
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_tU32(&cmd_payload, &cmd_len, &period);
	}
	
	if (ret == OSAL_OK)
	{
		/* call etal_get_CF_data */
		retval = etal_get_CF_data(hReceiver, pResp, nbOfAvarage, period);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	} 
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	
	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	if (ret == OSAL_OK)
	{
		/* add EtalCFDataContainer * to response payload */
		ret = rif_etalapi_cnv_resp_add_pEtalCFDataContainer(resp, rlen, pResp);
	}
	
	return OSAL_OK;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_seamless_estimation_start
 *
 **********************************/
/*!
 * \brief		handle an etal_seamless_estimation_start rimw command
 * \details		call etal_seamless_estimation_start api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_seamless_estimation_start (tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiverFAS , hReceiverSAS;
	etalSeamlessEstimationConfigTy seamlessEstimationConfig_ptr, *pResp = &seamlessEstimationConfig_ptr;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);
	
	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiverFAS);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiverSAS);	
	}
	
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&seamlessEstimationConfig_ptr, 0, sizeof(etalSeamlessEstimationConfigTy));
		ret = rif_etalapi_cnv_type_get_pEtalSeamlessEstimationConfigTy(&cmd_payload, &cmd_len, &pResp);
	}
		
	if (ret == OSAL_OK)
	{
		/* call etal_seamless_estimation_start */
		retval = etal_seamless_estimation_start(hReceiverFAS, hReceiverSAS, pResp);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	} 
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	
	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_seamless_estimation_stop
 *
 **********************************/
/*!
 * \brief		handle an etal_seamless_estimation_stop rimw command
 * \details		call etal_seamless_estimation_stop api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_seamless_estimation_stop (tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiverFAS , hReceiverSAS;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);
	
	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiverFAS);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiverSAS);	
	}
		
	if (ret == OSAL_OK)
	{
		/* call etal_seamless_estimation_stop */
		retval = etal_seamless_estimation_stop(hReceiverFAS, hReceiverSAS);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	} 
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	
	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	
	return ret;
}

/***********************************
 *
 * rif_etalapi_cnv_etal_seamless_switching
 *
 **********************************/
/*!
 * \brief		handle an seamless_switching rimw command
 * \details		call seamless_switching api from a rimw message and create rimw response.
 * \param[in]	  cmd   - pointer to buffer of tU8 containing the command. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in]	  clen  - size in bytes of the *cmd* buffer
 * \param[in/out] resp  - pointer to buffer containing the response. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt rif_etalapi_cnv_etal_seamless_switching (tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiverFAS , hReceiverSAS;
	etalSeamlessSwitchingConfigTy seamlessSwitchingConfig_ptr, *pResp = &seamlessSwitchingConfig_ptr;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;
	
	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);
	
	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiverFAS);
	if (ret == OSAL_OK)
	{
		ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiverSAS);	
	}
		
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&seamlessSwitchingConfig_ptr, 0, sizeof(etalSeamlessSwitchingConfigTy));
		ret = rif_etalapi_cnv_type_get_pEtalSeamlessSwitchingConfigTy(&cmd_payload, &cmd_len, &pResp);
	}
			
	if (ret == OSAL_OK)
	{
		/* call etal_seamless_switching */
		retval = etal_seamless_switching(hReceiverFAS, hReceiverSAS, pResp);
		/* create response */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	} 
	else
	{
		/* create response error */
		ret = rif_etalapi_cnv_create_resp(cmd, clen, resp, rlen, rif_rimw_OSAL_ERROR_to_rimw_error(ret));
	}
	
	if (ret == OSAL_OK)
	{
		/* add ETAL_STATUS to response payload */
		ret = rif_etalapi_cnv_resp_add_ETAL_STATUS(resp, rlen, retval);
	}
	
	return ret;
}

