/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file sys_nvm.c																				    		*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains nvm API definitions 											*
*																											*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "sys_nvm.h"
#include "debug_log.h"
#include <stdio.h>
#include <memory.h>
#include <Windows.h>
#ifdef UITRON
#include "sw_mem_manage_api.h"
#endif
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
   variables (global)
-----------------------------------------------------------------------------*/
FILE *nvm_file_ptr;
FILE *lsm_file_ptr;
/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/
/*  NVM Data Info for AMFM Application */
static st_nvm_filedata NVM_filedata_AMFMApp[] =
{
	{ NVM_ID_TUNER_AMFM_APP, NVM_ID_TUNER_AMFM_APP_LEN, NVM_ID_TUNER_AMFM_APP_OFFSET }
};

/*  NVM Data Info for DAB TC LINKING */
static st_nvm_filedata NVM_filedata_DAB_TC_Linking[] =
{
	{ NVM_ID_DAB_TC_LINKING, NVM_ID_DAB_TC_LINKING_LEN, NVM_ID_DAB_TC_LINKING_OFFSET }
};

/*  NVM Data Info for DAB TC LEARN MEMORY */
static st_nvm_filedata NVM_filedata_DAB_TC_LearnMemory[] =
{
	{ NVM_ID_DAB_TC_LEARN_MEMORY, NVM_ID_DAB_TC_LEARN_MEMORY_LEN, NVM_ID_DAB_TC_LEARN_MEMORY_OFFSET }
};

/*  NVM Data Info for Radio Manager */
static st_nvm_filedata NVM_filedata_RadioMngr[] =
{
	{ NVM_ID_TUNER_RADIOMNGR_APP, NVM_ID_TUNER_RADIOMNGR_APP_LEN, NVM_ID_TUNER_RADIO_MAGR_APP_OFFSET }
};

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/
/*===========================================================================
Tu8 sys_nvm_file_open(void)
============================================================================*/
Tu8 sys_nvm_file_open(void)
{
	FILE *nvmcheck = NULL;
	Tu8 u8_ret_value = SYS_NVM_FAILED;

	/* Create the NVM file during startup if it doesn't exist already */
    nvmcheck = fopen("D:\\ST_NVM.bin", "ab");
	if(nvmcheck != NULL)
	{
		fclose(nvmcheck);
        lsm_file_ptr = fopen("D:\\ST_LSM.bin", "rb+");

		if(nvm_file_ptr != NULL)
		{
			u8_ret_value = SYS_NVM_SUCCESS;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] NVM_FILE Open Success");

		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] NVM_FILE Open Failure");
		}
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] NVM_FILE does not exist");
	}
	return u8_ret_value;
}
/*===========================================================================
Tu8 sys_nvm_file_close(void)
============================================================================*/
Tu8 sys_nvm_file_close(void)
{
	Tu8 u8_ret_value = SYS_NVM_FAILED;
	u8_ret_value = fclose(nvm_file_ptr);
	if(u8_ret_value == SYS_NVM_SUCCESS)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] NVM_FILE close Success");
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] NVM_FILE close Failure");
	}
	return u8_ret_value;
}
/*===========================================================================
 Tu8 Sys_NVM_GetFileData( st_nvm_filedata* pst_get_nvm_comp_info)		    	 
============================================================================*/
Tu16 Sys_NVM_GetFileData( st_nvm_filedata* pst_get_nvm_comp_info)
{
	Tu16 u16_res     	= SYS_NVM_FAILED; 
	Tu8  u8_component	= pst_get_nvm_comp_info->u8_NVM_fileId;
	
	if (u8_component == NVM_ID_TUNER_AMFM_APP)
	{
		pst_get_nvm_comp_info->u32_NVM_Datalength = NVM_filedata_AMFMApp[0].u32_NVM_Datalength;
		pst_get_nvm_comp_info->u32_NVM_offset = NVM_filedata_AMFMApp[0].u32_NVM_offset;
		u16_res = SYS_NVM_SUCCESS;
	}
	else if (u8_component == NVM_ID_DAB_TC_LINKING)
	{
		pst_get_nvm_comp_info->u32_NVM_Datalength = NVM_filedata_DAB_TC_Linking[0].u32_NVM_Datalength;
		pst_get_nvm_comp_info->u32_NVM_offset = NVM_filedata_DAB_TC_Linking[0].u32_NVM_offset;
		u16_res = SYS_NVM_SUCCESS;
	}
	else if (u8_component == NVM_ID_DAB_TC_LEARN_MEMORY)
	{
		pst_get_nvm_comp_info->u32_NVM_Datalength = NVM_filedata_DAB_TC_LearnMemory[0].u32_NVM_Datalength;
		pst_get_nvm_comp_info->u32_NVM_offset = NVM_filedata_DAB_TC_LearnMemory[0].u32_NVM_offset;
		u16_res = SYS_NVM_SUCCESS;
	}
	else if (u8_component == NVM_ID_TUNER_RADIOMNGR_APP)
	{
		pst_get_nvm_comp_info->u32_NVM_Datalength = NVM_filedata_RadioMngr[0].u32_NVM_Datalength;
		pst_get_nvm_comp_info->u32_NVM_offset = NVM_filedata_RadioMngr[0].u32_NVM_offset;
		u16_res = SYS_NVM_SUCCESS;
	}
	else
	{
	  u16_res = SYS_NVM_ID_NOT_FOUND;
	}	
	return u16_res;
}
/*===========================================================================
 Tu8 Sys_NVM_Write(	Tu8 		u8_NVM_file_id,
					const void* write_buffer,
					Tu32 		u32_data_size,
					Tu32* 		pu32_written_bytes )             			 
===========================================================================*/
Tu8 Sys_NVM_Write(	Tu8 		u8_NVM_file_id,
					const void* write_buffer,
					Tu32 		u32_data_size,
					Tu32* 		pu32_written_bytes )
{
	st_nvm_filedata st_Comp_info ;
	Tu32            u32_num_item = 1; //fwrite count is always set to 1
	Tu32            u32_ret_status = SYS_NVM_FAILED;
	Tu8 			u8_res = SYS_NVM_FAILED;
	Tu16 			u16_Error_check = SYS_NVM_FAILED;
	
	if(nvm_file_ptr != NULL)
	{
		if((write_buffer != NULL) && (u32_data_size > 0))
		{
            /* Clearing to avoid junk data */
			(void)memset((void*)&st_Comp_info, 0, sizeof(st_nvm_filedata));

			st_Comp_info.u8_NVM_fileId	= u8_NVM_file_id;	
			u16_Error_check 			= Sys_NVM_GetFileData(&st_Comp_info);

            /* Checking component find status*/
			if((u16_Error_check == SYS_NVM_SUCCESS))
			{   
				u32_ret_status = fseek(nvm_file_ptr, st_Comp_info.u32_NVM_offset, SEEK_SET);
				if (u32_ret_status == SYS_NVM_SEEK_SUCCESS)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] File Seek Success");
					u32_ret_status = fwrite(write_buffer, u32_data_size, u32_num_item, nvm_file_ptr);
					if (u32_ret_status == SYS_NVM_WRITE_SUCCESS)
					{
						u8_res = SYS_NVM_SUCCESS;
						*pu32_written_bytes = u32_data_size;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] NVM_Write Success");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] NVM_Write Failure");
					}
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] File Seek Failure");
				}
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] Invalid NVM id in SYS_NVM_WRITE");
			}
		}
		else
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] Invalid write buffer or data size is less than zero bytes in SYS_NVM_WRITE");
		}
	}
	else
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_WARNING, "[RADIO][FW] Error in opening NVM file");
	}
	return u8_res;
}

/*===========================================================================
Tu8 Sys_NVM_Read(	Tu8 		u8_NVM_file_id,
					void* 		read_buffer,
					Tu32 		u32_data_size,
					Tu32* 		pu32_read_bytes )				             	 
===========================================================================*/
Tu8 Sys_NVM_Read(	Tu8 		u8_NVM_file_id,
					void* 		read_buffer,
					Tu32 		u32_data_size,
					Tu32* 		pu32_read_bytes )
{
	st_nvm_filedata	st_Comp_info ;
	Tu32            u32_num_item = 1; //fread count is always set to 1
	Tu32            u32_ret_status = SYS_NVM_FAILED;
	Tu16 			u16_Error_check = SYS_NVM_FAILED;	
	Tu8 			u8_res 			= SYS_NVM_FAILED;
	
	if (nvm_file_ptr != NULL)
	{
		if(u32_data_size > 0)
		{
            /* Clearing to avoid junk data */
			memset(&st_Comp_info, 0, sizeof(st_nvm_filedata));

			st_Comp_info.u8_NVM_fileId	= u8_NVM_file_id;	
			u16_Error_check 			= Sys_NVM_GetFileData(&st_Comp_info);

            /* Checking component find status */
			if(u16_Error_check == SYS_NVM_SUCCESS)
			{
				u32_ret_status = fseek(nvm_file_ptr, st_Comp_info.u32_NVM_offset, SEEK_SET);
				if (u32_ret_status == SYS_NVM_SEEK_SUCCESS)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] File Seek Success");
					u32_ret_status = fread(read_buffer, u32_data_size, u32_num_item, nvm_file_ptr);
					if (u32_ret_status == SYS_NVM_READ_SUCCESS)
					{
						u8_res = SYS_NVM_SUCCESS;
						*pu32_read_bytes = u32_data_size;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] NVM_Read Success");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] NVM_Read Failure");
					}
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] File Seek Failure");
				}

			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] Invalid NVM id in SYS_NVM_READ");
			}
		}
		else
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] data size is less than zero bytes in SYS_NVM_READ");
		}
	}
	else
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_WARNING, "[RADIO][FW] Error in opening NVM file");
	}
	
	return u8_res;
}
/*===========================================================================
Tu8 sys_lsm_file_open(void)
============================================================================*/
Tu8 sys_lsm_file_open(void)
{
	FILE *lsmcheck = NULL;
	Tu8 u8_ret_value = SYS_LSM_FAILED;
	
	/* Create the LSM file during startup if it doesn't exist already */
    lsmcheck = fopen("D:\\ST_LSM.bin", "ab");
	if(lsmcheck != NULL)
	{
		fclose(lsmcheck);
        lsm_file_ptr = fopen("D:\\ST_LSM.bin", "rb+");

		if (lsm_file_ptr != NULL)
		{
			u8_ret_value = SYS_LSM_SUCCESS;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] LSM_FILE Open Success");

		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] LSM_FILE Open Failure");
		}
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] LSM_FILE doesnot exist");
	}
	return u8_ret_value;
}
/*===========================================================================
Tu8 sys_lsm_file_close(void)
============================================================================*/
Tu8 sys_lsm_file_close(void)
{
	Tu8 u8_ret_value = SYS_LSM_FAILED;
	u8_ret_value = fclose(lsm_file_ptr);
	if (u8_ret_value == SYS_LSM_SUCCESS)
	{
		u8_ret_value = SYS_LSM_SUCCESS;
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] LSM_FILE close Success");
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] LSM_FILE close Failure");
	}
	return u8_ret_value;
}
/*===========================================================================
 Tu8 Sys_Tuner_LSM_Write(Tu32 u32_len, Tu8* u8_wbuf)          							         
===========================================================================*/
Tu8 Sys_Tuner_LSM_Write(Tu8 u8_compnent_id, Tu32 u32_len, void* pu8_wbuf)
{
	Tu32    u32_num_item = 1; //fwrite count is always set to 1
	Tu32    u32_ret_status = SYS_LSM_FAILED;
	Tu8 	u8_res = SYS_LSM_FAILED;
	if (lsm_file_ptr != NULL)
	{
		if(u8_compnent_id == RADIO_MNGR_TUNER_LSM)
		{
			if(pu8_wbuf != NULL && u32_len > 0)
			{
				u32_ret_status = fseek(lsm_file_ptr, LSM_OFFSET, SEEK_SET);
				if(u32_ret_status == SYS_LSM_SEEK_SUCCESS)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] File Seek Success");
					u32_ret_status = fwrite(pu8_wbuf, u32_len, u32_num_item, lsm_file_ptr);
					if(u32_ret_status == SYS_LSM_WRITE_SUCCESS)
					{
						u8_res = SYS_LSM_SUCCESS;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] LSM_Write Success");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] LSM_Write Failure");
					}
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] File Seek Failure");
				}
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] Write buffer is NULL/Write data size is less than zero in SYS_TUNER_LSM_WRITE");
			}
		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] INVALID COMPONENT ID in SYS_TUNER_LSM_WRITE");
		}
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_WARNING, "[RADIO][FW] Error in opening LSM file");
	}
	return u8_res;
}
/*===========================================================================
 Tu8  Sys_Tuner_LSM_Read(Tu32 len, Tu8* rbuf)           							         
===========================================================================*/
Tu8  Sys_Tuner_LSM_Read(Tu8 u8_compnent_id, Tu32 u32_len, void* pu8_rbuf)
{
	Tu32    u32_num_item = 1; //fread count is always set to 1
	Tu32    u32_ret_status = SYS_LSM_FAILED;
	Tu8 	u8_res = SYS_LSM_FAILED;
	if (lsm_file_ptr != NULL)
	{
		if (u8_compnent_id == RADIO_MNGR_TUNER_LSM)
		{
			if (u32_len > 0)
			{
				u32_ret_status = fseek(lsm_file_ptr, LSM_OFFSET, SEEK_SET);
				if (u32_ret_status == SYS_LSM_SEEK_SUCCESS)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] File Seek Success");
					u32_ret_status = fread(pu8_rbuf, u32_len, u32_num_item, lsm_file_ptr);
					if (u32_ret_status == SYS_LSM_READ_SUCCESS)
					{
						u8_res = SYS_LSM_SUCCESS;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] LSM_Read Success");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] LSM_Read Failure");
					}
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] File Seek Failure");
				}
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] Length of Read data size is less than zero in SYS_TUNER_LSM_READ");
			}
		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] INVALID COMPONENT ID in SYS_TUNER_LSM_READ");
		}
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_WARNING, "[RADIO][FW] Error in opening LSM file");
	}
	return 	u8_res;
}
/*=======================================================================================================
    end of file
========================================================================================================*/
