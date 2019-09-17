//!
//!  \file    rif_etalapi_cnv.c
//!  \brief   <i><b> Radio interface etaltml api conversion </b></i>
//!  \details This module implements the conversion etaltml api with protocol layer command, reponse, notification and datapath.
//!  \author  Alan Le Fol
//!

#include "target_config.h"
#include "target_config_radio_if.h"

#include "osal.h"

#include "etal_api.h"

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
#include "rif_etalapi_cnv.h"
#include "rif_etaltmlapi_cnv.h"
#include "rif_rimw_protocol.h"
#include "rif_protocol_router.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
#define RIF_ETALTML_API_CNV_ETAL_CMDNUM_MAX     0x17
enum
{
	CMDNUM_ETAL_INITIALIZE = 0x01,
	CMDNUM_ETAL_PING
};

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/

tSInt rif_etaltmlapi_cnv_etaltml_get_textinfo(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etaltmlapi_cnv_etaltml_start_textinfo(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
tSInt rif_etaltmlapi_cnv_etaltml_stop_textinfo(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);







/*****************************************************************
| Local types
|----------------------------------------------------------------*/
typedef tSInt (*rifEtalTmlApiCnv_EtalCmdFunc) (tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen); 

typedef struct
{
    rifEtalTmlApiCnv_EtalCmdFunc etaltml_cmd_func;
} rif_etaltmlapi_cnv_etal_cmd_list_Ty;

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
rif_etaltmlapi_cnv_etal_cmd_list_Ty rif_etaltmlapi_cnv_etal_cmd_list[RIF_ETALTML_API_CNV_ETAL_CMDNUM_MAX] =
{
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {rif_etaltmlapi_cnv_etaltml_start_textinfo}, {rif_etaltmlapi_cnv_etaltml_stop_textinfo}, 
	{rif_etaltmlapi_cnv_etaltml_get_textinfo}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, {NULL}, 
	{NULL}, {NULL}, {NULL}, {NULL}, {NULL}
};

/*****************************************************************
| functions
|----------------------------------------------------------------*/

/***********************************
 *
 * rif_etaltmlapi_cnv_get_cmd_func
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
rifEtalTmlApiCnv_EtalCmdFunc rif_etaltmlapi_cnv_get_cmd_func(tU16 cmd_num)
{
	/* get command number */
	if (cmd_num < RIF_ETALTML_API_CNV_ETAL_CMDNUM_MAX)
	{
		return rif_etaltmlapi_cnv_etal_cmd_list[cmd_num].etaltml_cmd_func;
	}
	else
	{
		return (rifEtalTmlApiCnv_EtalCmdFunc)NULL;
	}
}

/***********************************
 *
 * rif_etaltmlapi_cnv_checkCmdNumber
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
tSInt rif_etaltmlapi_cnv_checkCmdNumber(tU8 *cmd, tU32 clen)
{
	tU16 cmd_num;

	/* get command number */
	cmd_num = (RIF_RIMW_GET_CMD_NUM(cmd) & 0x01FF);
	//rif_pr_tracePrintComponent(TR_CLASS_EXTERNAL, "cmd_num = %d  len = %d\n", cmd_num, clen);
	rif_pr_tracePrintComponent(TR_CLASS_EXTERNAL, "EtalTmlApi : cmd_num = %d  len = %d\n", cmd_num, clen); //Debug Alan
	if (rif_etaltmlapi_cnv_get_cmd_func(cmd_num) != NULL)
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
 * rif_etaltmlapi_cnv_cmd
 *
 **********************************/
/*!
 * \brief		convert command in etaltml api
 * \details		convert command in etaltml api calls and return result
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
tSInt rif_etaltmlapi_cnv_cmd(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU16 cmd_num;
	rifEtalTmlApiCnv_EtalCmdFunc etaltml_cmd_func;

	/* initialize response length to 0 */
	*rlen = 0;
	/* get command number */
	cmd_num = (RIF_RIMW_GET_CMD_NUM(cmd) & 0x01FF);
	rif_pr_tracePrintComponent(TR_CLASS_EXTERNAL, "cmd_num = %d %p\n", cmd_num, rif_etaltmlapi_cnv_get_cmd_func(cmd_num));
	/* get command conversion function */
	if ((etaltml_cmd_func = rif_etaltmlapi_cnv_get_cmd_func(cmd_num)) == NULL)
	{
		return OSAL_ERROR_NOT_IMPLEMENTED;
	}
	/* call command conversion function to get response */
	etaltml_cmd_func(cmd, clen, resp, rlen);
	return OSAL_OK;
}

/***********************************
 *
 * rif_etaltmlapi_cnv_resp_add_EtalTextInfo
 *
 **********************************/
/*!
 * \brief		add EtalTextInfo to a payload
 * \details		add a EtalTextInfo to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalTextInfo pointer to value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etaltmlapi_cnv_resp_add_EtalTextInfo(tU8 *resp, tU32 *rlen, EtalTextInfo *ptr)
{
	tSInt ret = OSAL_OK, i;

	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add m_broadcastStandard */
			ret = rif_etalapi_cnv_resp_add_EtalBcastStandard(resp, rlen, ptr->m_broadcastStandard);
		}
		if (ret == OSAL_OK)
		{
			/* add m_serviceNameIsNew */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_serviceNameIsNew);
		}
		
		if (ret == OSAL_OK)
		{
			/* add m_serviceName[i] */
			ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr->m_serviceName);
			if (ptr != NULL)
			{	
				for(i = 0; i < ETAL_DEF_MAX_SERVICENAME; i++)
				{
					ret = rif_etalapi_cnv_resp_add_tChar(resp, rlen, ptr->m_serviceName[i]);
				}
			}
		}
		if (ret == OSAL_OK)
		{
			/* add m_serviceNameCharset */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_serviceNameCharset);
		}
	
		if (ret == OSAL_OK)
		{
			/* add m_currentInfoIsNew */
			ret = rif_etalapi_cnv_resp_add_tBool(resp, rlen, ptr->m_currentInfoIsNew);
		}

		if (ret == OSAL_OK)
		{
			/* add m_currentInfo[i] */
			ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr->m_currentInfo);
			if (ptr != NULL)
			{	
				for(i = 0; i < ETAL_DEF_MAX_INFO; i++)
				{
					ret = rif_etalapi_cnv_resp_add_tChar(resp, rlen, ptr->m_currentInfo[i]);
				}
			}
		}

		if (ret == OSAL_OK)
		{
			/* add m_currentInfoCharset */
			ret = rif_etalapi_cnv_resp_add_tU8(resp, rlen, ptr->m_currentInfoCharset);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etaltmlapi_cnv_resp_add_pEtalTextInfo
 *
 **********************************/
/*!
 * \brief		add EtalTextInfo pointer to a payload
 * \details		add a EtalTextInfo pointer to a response payload. Increase response length and response payload length.
 * \param[in/out] resp  - pointer to buffer containing the response payload. the function
 * 				          does not make any assumption on the content of the buffer
 * \param[in/out] rlen  - size in bytes of the *resp* buffer
 * \param[in]	  ptr   - EtalTextInfo pointer value to add.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - invalid response payload length
 * \callgraph
 * \callergraph
 */
tSInt rif_etaltmlapi_cnv_resp_add_pEtalTextInfo(tU8 *resp, tU32 *rlen, EtalTextInfo *ptr)
{
	tSInt ret = OSAL_OK;

	/* add EtalCFDataContainer * to response payload */
	ret = rif_etalapi_cnv_resp_add_pointer_type(resp, rlen, (tVoid *)ptr);
	if (ptr != NULL)
	{
		if (ret == OSAL_OK)
		{
			/* add EtalCFDataContainer */
			ret = rif_etaltmlapi_cnv_resp_add_EtalTextInfo(resp, rlen, ptr);
		}
	}

	return ret;
}

/***********************************
 *
 * rif_etaltmlapi_cnv_etaltml_get_textinfo
 *
 **********************************/
/*!
 * \brief		handle an etaltml_get_textinfo rimw command
 * \details		call etaltml_get_textinfo api from a rimw message and create rimw response.
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
tSInt rif_etaltmlapi_cnv_etaltml_get_textinfo(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	tU8 *cmd_payload;
	tU32 cmd_len;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hReceiver;
	EtalTextInfo radiotext, *pResp = &radiotext;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	/* get payload address and payload length */
	cmd_payload = rif_rimw_get_cmd_payload_address(cmd, clen);
	cmd_len = rif_rimw_get_cmd_payload_length(cmd, clen);
	
	/* get etal api parameters */
	ret = rif_etalapi_cnv_type_get_ETAL_HANDLE(&cmd_payload, &cmd_len, &hReceiver);
	if (ret == OSAL_OK)
	{
		OSAL_pvMemorySet(&radiotext, 0, sizeof(EtalTextInfo));
		ret = rif_etalapi_cnv_type_get_p(&cmd_payload, &cmd_len, (tPVoid *)&pResp);
	}
		
	if (ret == OSAL_OK)
	{
		/* call etaltml_get_textinfo */
		retval = etaltml_get_textinfo(hReceiver, pResp);
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
		/* add EtalTextInfo * to response payload */
		ret = rif_etaltmlapi_cnv_resp_add_pEtalTextInfo(resp, rlen, pResp);
	}
	
	return ret;
}

/***********************************
 *
 * rif_etaltmlapi_cnv_etaltml_start_textinfo
 *
 **********************************/
/*!
 * \brief		handle an etaltml_start_textinfo rimw command
 * \details		call etaltml_start_textinfo api from a rimw message and create rimw response.
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
tSInt rif_etaltmlapi_cnv_etaltml_start_textinfo(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
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
		/* call etaltml_start_textinfo */
		retval = etaltml_start_textinfo(hReceiver);
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
 * rif_etaltmlapi_cnv_etaltml_stop_textinfo
 *
 **********************************/
/*!
 * \brief		handle an etaltml_stop_textinfo rimw command
 * \details		call etaltml_stop_textinfo api from a rimw message and create rimw response.
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
tSInt rif_etaltmlapi_cnv_etaltml_stop_textinfo (tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
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
		/* call etaltml_stop_textinfo */
		retval = etaltml_stop_textinfo(hReceiver);
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


