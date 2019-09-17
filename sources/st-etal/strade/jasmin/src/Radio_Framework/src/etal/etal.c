/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file etal.c																				    		*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains etal startup API definitions									*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/
#include "etal.h"
#include <stddef.h>
#include "AMFM_Tuner_Ctrl_Types.h"
#include "DAB_Tuner_Ctrl_Request.h"
#include "IRadio.h"

/*-----------------------------------------------------------------------------
defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
type definitions
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
variables (global)
-----------------------------------------------------------------------------*/
EtalHardwareAttr Etal_Initparams;
EtalHwCapabilities *Etal_Hwcapabilites;
EtalVersion Etal_Curr_Version;
Ts_Sys_Msg st_etal_sendmsg;

/*-----------------------------------------------------------------------------
variables (static)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
variables (extern)
-----------------------------------------------------------------------------*/
extern ETAL_HANDLE AMFM_FG_recvhdl;
extern ETAL_HANDLE AMFM_BG_recvhdl;
extern ETAL_HANDLE dabFG_recvhdl;

/*-----------------------------------------------------------------------------
private function declarations
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void AMFM_Tuner_ctrl_Seek_CB_Notify - send msg to handler                */
/*===========================================================================*/
void AMFM_Tuner_ctrl_Seek_CB_Notify(void *pvContext)
{
	Ts_Sys_Msg* msg = NULL;
	//memset(&(msg), 0x00, sizeof(msg));
	msg = ETAL_SYS_MSG_HANDLE(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_INFO_RESID);
	ETAL_UpdateParameterIntoMessage((Tchar *)(msg->data), pvContext, (Tu8)(sizeof(EtalSeekStatus)), &(msg->msg_length));
	SYS_SEND_MSG(msg);
}
void AMFM_Tuner_ctrl_Quality_Monitor_CB_Notify(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	UNUSED(vpContext);
	Ts_Sys_Msg* msg = NULL;
	msg = ETAL_SYS_MSG_HANDLE(RADIO_AM_FM_TUNER_CTRL, QUALITY_NOTIFICATION_MSGID);
	ETAL_UpdateParameterIntoMessage((Tchar *)(msg->data), pQuality, (Tu8)(sizeof(EtalBcastQualityContainer)), &(msg->msg_length));
	SYS_SEND_MSG(msg);
	
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  ETAL_SYS_MSG_HANDLE										 */
/*===========================================================================*/
Ts_Sys_Msg* ETAL_SYS_MSG_HANDLE(Tu16 cid, Tu16 msgid)
{
	memset(&st_etal_sendmsg, 0x00, sizeof(st_etal_sendmsg));
	st_etal_sendmsg.dest_id = cid;
	st_etal_sendmsg.msg_id = msgid;
	return (&st_etal_sendmsg);
}

/*===========================================================================*/
/* void ETAL_UpdateParameterIntoMessage                      */
/*===========================================================================*/
void ETAL_UpdateParameterIntoMessage(Tchar *pu8_data, const void *vp_parameter, Tu8 u8_ParamLength, Tu16 *pu16_Datalength)
{
	/* copying parameter to the data slots in Ts_Sys_Msg structure */
	SYS_RADIO_MEMCPY((pu8_data + *pu16_Datalength), vp_parameter, (size_t)(u8_ParamLength));

	/*  Updating msg_length for each parameter which represents length of data in  Ts_Sys_Msg structure   */
	*pu16_Datalength = (Tu16)(*pu16_Datalength + u8_ParamLength);
}

/*===========================================================================*/
/*  void ETAL_Event_Callback                                                 */
/*===========================================================================*/
void ETAL_Event_Callback(void *pvContext, ETAL_EVENTS dwEvent, void *pvParams)
{
	UNUSED(pvContext);
	switch (dwEvent)
	{
		case ETAL_INFO_SEEK:
		{

		   EtalSeekStatus* st_seek_status = (EtalSeekStatus *)pvParams;
		   if ((st_seek_status->m_receiverHandle == AMFM_FG_recvhdl) || (st_seek_status->m_receiverHandle == AMFM_BG_recvhdl))
		   {
			   AMFM_Tuner_ctrl_Seek_CB_Notify(pvParams);
		   }			
		   else if (st_seek_status->m_receiverHandle == dabFG_recvhdl)
		   {
			   Ts_Sys_Msg* p_msg = &st_etal_sendmsg;
			   memset(p_msg, 0, sizeof(Ts_Sys_Msg));
			   ETAL_UpdateParameterIntoMessage((Tchar *)(p_msg->data), (const void *)st_seek_status, sizeof(EtalSeekStatus), &(p_msg->msg_length));
			   p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
			   p_msg->msg_id = DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION;
			   SYS_SEND_MSG(p_msg);
		   }
		   else
		   {
			   //do nothing
		   }
		}
		break;

		case ETAL_INFO_TUNE:
		{
			EtalTuneStatus *Tunestatus = (EtalTuneStatus*)pvParams;
			if (Tunestatus->m_receiverHandle == dabFG_recvhdl)
			{
				Ts_Sys_Msg* p_msg = &st_etal_sendmsg;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] ETAL_INFO_TUNE - SF %d - SY %d - MS %d - SID %d ", Tunestatus->m_stopFrequency, Tunestatus->m_sync, Tunestatus->m_muteStatus, Tunestatus->m_serviceId);
				memset(p_msg, 0, sizeof(Ts_Sys_Msg));
				ETAL_UpdateParameterIntoMessage((Tchar *)(p_msg->data), (const void *)Tunestatus, sizeof(EtalTuneStatus), &(p_msg->msg_length));
				p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
				p_msg->msg_id = DAB_TUNER_CTRL_TUNE_STATUS_NOTIFICATION;
				SYS_SEND_MSG(p_msg);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] ETAL_INFO_TUNE - AMFM ");
			}
		}
		break;
	    default:
		{
			//do nothing
		}
		break;
	}
}

/*===========================================================================*/
/*  Tbool ETAL_Init                                                          */
/*===========================================================================*/
Tbool ETAL_Init(Radio_EtalHardwareAttr attr)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;

	memset(&Etal_Initparams, 0, sizeof(EtalHardwareAttr));

	/* Country Variant Configuration */
	Etal_Initparams.m_CountryVariant = ETAL_COUNTRY_VARIANT_EU;			/* The country in which ETAL is operating				*/

	/* DCOP Attribute Configuration */
	SYS_RADIO_MEMCPY(&(Etal_Initparams.m_DCOPAttr), &(attr.m_DCOPAttr), sizeof(attr.m_DCOPAttr));

	/* Tuner Attribute Configuration */
	SYS_RADIO_MEMCPY(&(Etal_Initparams.m_tunerAttr[0]), &(attr.m_tunerAttr), (sizeof(attr.m_tunerAttr) * ETAL_CAPA_MAX_TUNER));

	/* Callback Configuration */
	Etal_Initparams.m_cbNotify = ETAL_Event_Callback;				/* Callback for event notification						*/
	Etal_Initparams.m_context = NULL;								/* First parameter passed to the m_cbNotify function	*/

	/* Trace Level Configuration */
	Etal_Initparams.m_traceConfig.m_disableHeaderUsed = FALSE;					/* m_disableHeader field is not parsed */
	Etal_Initparams.m_traceConfig.m_disableHeader = FALSE;					/* this field is unused */
	Etal_Initparams.m_traceConfig.m_defaultLevelUsed = FALSE;					/* m_defaultLevel field is not parsed */
	Etal_Initparams.m_traceConfig.m_defaultLevel = ETAL_TR_LEVEL_FATAL;		/* this field is unused */
	
	/* Initialize ETAL Component  */
	e_RetStatus = ETAL_Initialize(&Etal_Initparams);

	if (e_RetStatus == ETAL_RET_SUCCESS)
	{
		return FALSE;
	}
	else
	{
		/* etal_initialize Failed */
		return TRUE;
	}
}

/*===========================================================================*/
/*  Tbool ETAL_Config                                                          */
/*===========================================================================*/
Tbool ETAL_Config(void)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;

	/* Config audio Path */
	e_RetStatus = ETAL_Config_AudioPath();

	if (e_RetStatus == ETAL_RET_SUCCESS)
	{
		/*Get the ETAL Version*/
		e_RetStatus = ETAL_Get_Version(&Etal_Curr_Version);

		if (e_RetStatus == ETAL_RET_SUCCESS)
		{
			/* Get the capabilities of hardware devices attached with ETAL */
			e_RetStatus = ETAL_Get_Capabilities(&Etal_Hwcapabilites);

			if (e_RetStatus == ETAL_RET_SUCCESS)
			{
				/* ETAL_Config() SUCCESS. Return success */
				return FALSE;
			}
			else
			{
				/* etal_get_capabilities Failed */
				return TRUE;
			}
		}
		else
		{
			/* etal_get_version Failed */
			return TRUE;
		}
	}
	else
	{
		/* etal_config_audio_path Failed */
		return TRUE;
	}
	
}


/*===========================================================================*/
/*  ETAL_STATUS ETAL_Initialize                                              */
/*===========================================================================*/
ETAL_STATUS ETAL_Initialize(EtalHardwareAttr *pETAL_InitParams)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;

	e_RetStatus = etal_initialize(pETAL_InitParams);

	if (e_RetStatus == ETAL_RET_NOT_INITIALIZED)
	{
		e_RetStatus = etal_initialize(pETAL_InitParams);

		if (e_RetStatus != ETAL_RET_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] : etal initialize error due to:%d", e_RetStatus);
		}
		else
		{
			/* ETAL Init success */
		}
	}
	else if (e_RetStatus != ETAL_RET_SUCCESS)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] : etal initialize error due to:%d", e_RetStatus);
	}

	return e_RetStatus;
}

/*===========================================================================*/
/*  ETAL_STATUS ETAL_Get_Capabilities                                        */
/*===========================================================================*/
ETAL_STATUS ETAL_Get_Capabilities(EtalHwCapabilities  **pCapabilities)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_SUCCESS;

	e_RetStatus = etal_get_capabilities(pCapabilities);

	if (e_RetStatus != ETAL_RET_SUCCESS)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] : etal_get_capabilities error due to:%d", e_RetStatus);
	}
	else
	{
		/* MISRA C */
	}

	return e_RetStatus;
}

/*===========================================================================*/
/*  ETAL_STATUS ETAL_Get_Version                                             */
/*===========================================================================*/
ETAL_STATUS ETAL_Get_Version(EtalVersion   *vers)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;

	e_RetStatus = etal_get_version(vers);

	if (e_RetStatus != ETAL_RET_SUCCESS)
	{
		e_RetStatus = etal_get_version(vers);

		if (e_RetStatus != ETAL_RET_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] : etal_get_version error due to:%d", e_RetStatus);
		}
		else
		{
			/* print version in log */
			ETAL_print_version(vers);
		}
	}
	else
	{
		/* print version in log */
		ETAL_print_version(vers);
	}

	return e_RetStatus;
}

/*===========================================================================*/
/*  ETAL_STATUS ETAL_Config_AudioPath		                                 */
/*===========================================================================*/
ETAL_STATUS ETAL_Config_AudioPath(void)
{
	EtalAudioInterfTy Etal_Audio_Intf_Config;
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;

	/* Configure Tuner 0 */
	Etal_Audio_Intf_Config.m_dac			= 1;			/* Enable the audio DAC								*/
	Etal_Audio_Intf_Config.m_sai_out		= 1;			/* Enable Serial Audio Interface Output				*/
	Etal_Audio_Intf_Config.m_sai_in			= 1;			/* Enable Serial Audio Interface Intput				*/
	Etal_Audio_Intf_Config.m_sai_slave_mode = 0;			/* Serial Audio Interface in Master Mode			*/
	Etal_Audio_Intf_Config.unused			= 0;			/* Assigned as Zero to avoid ETAL_PARAMETER_ERROR	*/

	/* audio interface configuration for a STAR device */
	e_RetStatus = etal_config_audio_path(TUNER_0, Etal_Audio_Intf_Config); 

	if (e_RetStatus != ETAL_RET_SUCCESS)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] : Tuner 0 etal_config_audio_path error due to:%d", e_RetStatus);
	}
	else
	{
		/* Configure Tuner 1 */
		Etal_Audio_Intf_Config.m_dac			= 1;			/* Enable the audio DAC								*/
		Etal_Audio_Intf_Config.m_sai_out		= 0;			/* Disable Serial Audio Interface Output			*/
		Etal_Audio_Intf_Config.m_sai_in			= 1;			/* Enable Serial Audio Interface Intput				*/
		Etal_Audio_Intf_Config.m_sai_slave_mode	= 1;			/* Serial Audio Interface in Slave Mode				*/
		Etal_Audio_Intf_Config.unused			= 0;			/* Assigned as Zero to avoid ETAL_PARAMETER_ERROR	*/

		/* audio interface configuration for a STAR device */
		e_RetStatus = etal_config_audio_path(TUNER_1, Etal_Audio_Intf_Config);

		if (e_RetStatus != ETAL_RET_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] : Tuner 1 etal_config_audio_path error due to:%d", e_RetStatus);
		}
		else
		{
			/* For MISRA C */
		}
	}

	return e_RetStatus;
}

/*===========================================================================*/
/*  Tbool ETAL_Deinit                                                        */
/*===========================================================================*/
Tbool ETAL_Deinit(void)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;

	e_RetStatus = etal_deinitialize();

	if (e_RetStatus == ETAL_RET_SUCCESS)
	{
		/* SUCCESS */
		return FALSE;
	}
	else
	{
		/* FAILURE */
		return TRUE;
	}
}

/*===========================================================================*/
/*  void ETAL_print_version                                                  */
/*===========================================================================*/
void ETAL_print_version(EtalVersion   *vers)
{
	/* print ETAL version */
	if (vers->m_ETAL.m_isValid == TRUE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] : ETAL Version : %s", vers->m_ETAL.m_name);
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] : ETAL not present ");
	}

	/* print m_CMOST TUNER_0 firmware version */
	if (vers->m_CMOST[TUNER_0].m_isValid == TRUE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] : CMOST TUNER_0 firmware Version : %s", vers->m_CMOST[TUNER_0].m_name);
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] : CMOST TUNER_0 not present ");
	}

	/* print m_CMOST TUNER_1 firmware version */
	if (vers->m_CMOST[TUNER_1].m_isValid == TRUE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] : CMOST TUNER_1 firmware Version : %s", vers->m_CMOST[TUNER_1].m_name);
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] : CMOST TUNER_1 not present ");
	}

	/* print DCOP firmware version */
	if (vers->m_MDR.m_isValid == TRUE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] : DCOP firmware Version : %s", vers->m_MDR.m_name);
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] : DCOP not present ");
	}
}

/*=============================================================================
end of file
=============================================================================*/