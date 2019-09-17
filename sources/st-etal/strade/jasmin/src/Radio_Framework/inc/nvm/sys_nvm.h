/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file sys_nvm.h																				    		*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains platform related API's declarations and Macro definitions		*
*																											*
*																											*
*************************************************************************************************************/


#ifndef SYS_NVM_H
#define SYS_NVM_H

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "cfg_types.h"


/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/
#define SYS_NVM_WRITE						Sys_NVM_Write   	/* NVM WRITE WRAPPER MACRO */
#define SYS_NVM_READ						Sys_NVM_Read		/* NVM READ WRAPPER MACRO */
#define SYS_TUNER_LSM_WRITE                 Sys_Tuner_LSM_Write /* TUNER LSM WRITE WRAPPER MACRO */
#define SYS_TUNER_LSM_READ                  Sys_Tuner_LSM_Read  /* TUNER LSM READ WRAPPER MACRO*/
	
#define NUM_OF_NVM_CMPNTS					3					/* NUMBER OF COMPONENTS USING NVM */
#define NVM_ID_TUNER_RADIOMNGR_APP          (Tu8)0x00			/* NVM ID FOR RADIO MANAGER APP */
#define NVM_ID_TUNER_AMFM_APP			    (Tu8)0x01			/* NVM ID FOR AMFM APP */
#define NVM_ID_DAB_TC_LINKING       		(Tu8)0x02 			/* NVM ID FOR DAB TC LINKING INFO */
#define NVM_ID_DAB_TC_LEARN_MEMORY     		(Tu8)0x03 			/* NVM ID FOR DAB TC LEARN MEMORY */
#define RADIO_MNGR_TUNER_LSM			    (Tu8)0x04       	/* TUNER_LSM_ID FOR RADIO_MNGR*/

#define NVM_ID_TUNER_RADIOMNGR_APP_LEN      (Tuint32)0x2DDC     /* FLASH SIZE FOR RM - 11740 */ 
#define NVM_ID_TUNER_AMFM_APP_LEN			(Tuint32)0x550  	/* FLASH SIZE FOR AMFM - 1360 */
#define NVM_ID_DAB_TC_LINKING_LEN   		(Tuint32)0x6B0  	/* FLASH SIZE FOR DAB TC LINKING - 1712 */
#define NVM_ID_DAB_TC_LEARN_MEMORY_LEN   	(Tuint32)0x640  	/* FLASH SIZE FOR DAB TC LEARN MEMORY - 1600 */

#define NVM_ID_TUNER_RADIO_MAGR_APP_OFFSET  (Tuint32)(0x00000000)															/* OFFSET ADDRESS FOR RADIO MANAGER */
#define NVM_ID_TUNER_AMFM_APP_OFFSET		(Tuint32)(NVM_ID_TUNER_RADIO_MAGR_APP_OFFSET + NVM_ID_TUNER_RADIOMNGR_APP_LEN)  /* OFFSET ADDRESS FOR AMFM APP */    
#define NVM_ID_DAB_TC_LINKING_OFFSET		(Tuint32)(NVM_ID_TUNER_AMFM_APP_OFFSET + NVM_ID_TUNER_AMFM_APP_LEN)				/* OFFSET ADDRESS FOR DAB TC LINKING */
#define NVM_ID_DAB_TC_LEARN_MEMORY_OFFSET	(Tuint32)(NVM_ID_DAB_TC_LINKING_OFFSET + NVM_ID_DAB_TC_LINKING_LEN)				/* OFFSET ADDRESS FOR DAB TC LEARN MEMORY */

#define SYS_NVM_SUCCESS						0x00 /* NVM SUCCESS RETURN */ 
#define SYS_NVM_FAILED						0x01 /* NVM FAILURE RETURN */
#define SYS_NVM_ID_NOT_FOUND                0xFF /* INVALID NVM FILE ID */
#define SYS_NVM_SEEK_SUCCESS                0x00 /* FSEEK RETURN */
#define SYS_NVM_WRITE_SUCCESS               0x01 /* FWRITE RETURN */
#define SYS_NVM_READ_SUCCESS                0x01 /* FREAD RETURN */  
#define SYS_LSM_SUCCESS						0x00 /* LSM SUCCESS RETURN */ 
#define SYS_LSM_FAILED						0x01 /* LSM FAILURE RETURN */
#define LSM_OFFSET                          0x00 /*LSM OFFSET POINTS TO THE BEGINNING OF FILE*/ 
#define SYS_LSM_SEEK_SUCCESS                0x00 /* FSEEK RETURN */
#define SYS_LSM_WRITE_SUCCESS               0x01 /* FWRITE RETURN */
#define SYS_LSM_READ_SUCCESS                0x01 /* FREAD RETURN */  
/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/
typedef struct nvm_info
{
	Tu8 	    u8_NVM_fileId; 	    /*NVM_ID_RADIO_MANGR,NVM_ID_AMFM_APP,NVM_ID_DAB_APP */
	Tuint32     u32_NVM_Datalength; /* size of the data */
	Tuint32     u32_NVM_offset;   	/*Each component have offset address for read/write */	
}st_nvm_filedata;

/*-----------------------------------------------------------------------------------------------------
    function declarations extern 
------------------------------------------------------------------------------------------------------*/
/*****************************************************************************************************/
/**	 \brief                 This API function sys_nvm_file_open is to open the nvm_file which is there in the pc.
*   \param[in]				no parameters
*   \param[out]				Tu8
*   \pre-condition			Startup case can be called
*   \details                open  the nvm file with the update mode for the required operation.
*   \post-condition			Based on return value(index)read and write will be done.
*   \ErrorHandling    		Return error code or the index
*
******************************************************************************************************/
Tu8 sys_nvm_file_open(void);
/*****************************************************************************************************/
/*****************************************************************************************************/
/**	 \brief                 This API function sys_nvm_file_close is to close the nvm_file which is there in the pc.
*   \param[in]				no parameters
*   \param[out]				Tu8
*   \pre-condition			Shutdown case can be called
*   \details                close  the nvm file 
*   \post-condition			Based on return value shutdown will be done.
*   \ErrorHandling    		Return error code or the index
*
******************************************************************************************************/
Tu8 sys_nvm_file_close(void);
/*****************************************************************************************************/
/**	 \brief                 This API function sys_lsm_file_open is to open the lsm_file which is there in the pc.
*   \param[in]				no parameters
*   \param[out]				Tu8
*   \pre-condition			Startup case can be called
*   \details                open  the lsm file with the update mode for the required operation.
*   \post-condition			Based on return value(index)read and write will be done.
*   \ErrorHandling    		Return error code or the index
*
******************************************************************************************************/
Tu8 sys_lsm_file_open(void);
/*****************************************************************************************************/
/*****************************************************************************************************/
/**	 \brief                 This API function sys_lsm_file_close is to close the lsm_file which is there in the pc.
*   \param[in]				no parameters
*   \param[out]				Tu8
*   \pre-condition			Shutdown case can be called
*   \details                close  the lsm file
*   \post-condition			Based on return value shutdown will be done.
*   \ErrorHandling    		Return error code or the index
*
******************************************************************************************************/
Tu8 sys_lsm_file_close(void);
/*****************************************************************************************************/
/**	 \brief                 This API function sys_nvm_write is to write data into NVM
*   \param[in]				Tu8 		NVM_file_id,
*							const void* write_buffer,
*							Tu32 		data_size,
*							Tu32* 		written_bytes 
*							NVM_file_id -- NVM_ID_RADIO_MANGR,NVM_ID_AMFM_APP,NVM_ID_DAB_APP
*							data_size -- Length buffer
*							*write_buffer -- write buffer
*							written_bytes -- written bytes
*   \param[out]				Tsint32
*   \pre-condition			Before shutdown this API can be called.
*   \details                This API function is used to write data into NVM 
*   \post-condition			This API written data into NVM
*   \ErrorHandling    		error code
* 
******************************************************************************************************/
Tu8 Sys_NVM_Write(	Tu8 		u8_NVM_file_id,
					const void* write_buffer,
					Tu32 		u32_data_size,
					Tu32* 		pu32_written_bytes );
					
/*****************************************************************************************************/
/**	 \brief                 This API function sys_nvm_read is to read data from NVM 
*   \param[in]				Tu8 		NVM_file_id,
*							void* 		read_buffer,
*							Tu32 		data_size,
*							Tu32* 		read_bytes 
*							NVM_file_id -- NVM_ID_RADIO_MANGR,NVM_ID_AMFM_APP,NVM_ID_DAB_APP
*							data_size -- Length to read from NVM
*							read_bytes -- read bytes
*							*read_buffer -- read buffer
*   \param[out]				Tsint32
*   \pre-condition			After registering data area can be called.
*   \details                This API function is used to read data from NVM
*   \post-condition			This API can be read data from NVM
*   \ErrorHandling    		error code
* 
******************************************************************************************************/
Tu8 Sys_NVM_Read(	Tu8 		u8_NVM_file_id,
					void* 		read_buffer,
					Tu32 		u32_data_size,
					Tu32* 		pu32_read_bytes );
/*****************************************************************************************************/
/**	 \brief                 This API function Sys_NVM_GetFileData is to get details of nvm_info structure.
*   \param[in]				st_nvm_filedata* get_nvm_comp_info,Tu8 component
*   \param[out]				Tu16
*   \pre-condition			For Read/Write will be called
*   \details                To get the offset and len for Read/write 
*   \post-condition			Can be provided details for read/write 
*   \ErrorHandling    		Return error code
* 
******************************************************************************************************/
Tu16 Sys_NVM_GetFileData( st_nvm_filedata* pst_get_nvm_comp_info);
/*****************************************************************************************************/
/**	 \brief                 This API function Sys_Tuner_LSM_Write is to write data to EEPROM for tuner 
*                           last mode.
*   \param[in]				Tu8 u8_NVM_file_id, Tu32 len, void* wbuf
*   \param[out]				Tu8
*   \pre-condition			None
*   \details                To Write data into EEPROM for tuner last mode.
*   \post-condition			Data has written to EEPROM.
*   \ErrorHandling    		Return error code
* 
******************************************************************************************************/
Tu8  Sys_Tuner_LSM_Write(Tu8 u8_compnent_id, Tu32 u32_len, void* pu8_wbuf);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_NVM_SetErrCode Read data from EEPROM for tuner last mode
*   \param[in]				Tu8  u8_NVM_file_id, Tu32 len, void* rbuf
*   \param[out]				Tu8
*   \pre-condition			None
*   \details                Read data from EEPROM for tuner last mode.
*   \post-condition			Data read from EEPROM.
*   \ErrorHandling    		Return error code
* 
******************************************************************************************************/
Tu8  Sys_Tuner_LSM_Read(Tu8  u8_compnent_id, Tu32 u32_len, void* pu8_rbuf);


#endif /* SYS_NVM_H */
/*=============================================================================
    end of file
=============================================================================*/

