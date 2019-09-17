//!
//!  \file 		etalbootcmd_mdr.c
//!  \brief 	<i><b> ETAL BOOT MDR API layer </b></i>
//!  \details   MDR-specific boot command implementation
//!  \author 	David Pastor
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined(CONFIG_ETAL_SUPPORT_DCOP_MDR) && !defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_NO_DOWNLOAD)

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

/*****************************************************************
| Local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*****************************************************************
| prototypes
|----------------------------------------------------------------*/

/***************************
 *
 * ETAL_cmdSpecialBootMode_MDR
 *
 **************************/
tSInt ETAL_cmdSpecialBootMode_MDR(STECI_cmdModeEnumTy boot_mode)
{
#define ETAL_MDR_BOOT_MODE_INDEX	1

	tU8 bootModeCmd[]  = { STECI_BOOT_CMD, 0x00, 0x00, 0x00, 0x00, 0x00 };
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt ret = OSAL_OK;

	// set boot_mode
	bootModeCmd[ETAL_MDR_BOOT_MODE_INDEX] = boot_mode;

    /* send boot mode */
	ret = ETAL_sendCommandSpecialTo_MDR(bootModeCmd, sizeof(bootModeCmd), &cstatus, TRUE, &resp, &resplen);

    /* check boot mode response is valid */
    if ((ret == OSAL_OK) && (((resplen > 8) && ((resp[8] != STECI_BOOT_CMD) || (resp[3] != 0))) || (resplen < 9)))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialBootMode_MDR boot mode response error 0x%x len %d 0x%x", (resplen > 3)?resp[3]:0, resplen, (resplen > 8)?resp[8]:0);
        ret = OSAL_ERROR;
    }
	return ret;
}

/***************************
 *
 * ETAL_cmdSpecialFlashBootstrap_MDR
 *
 **************************/
tSInt ETAL_cmdSpecialFlashBootstrap_MDR(DCOP_accessModeEnumTy data_mode, tU16 payload_len, tU8 *payload_ptr)
{
#define ETAL_MDR_DATA_MODE_INDEX	3

	tU8 flashBootstrapCmd[]  = { STECI_FLASH_CMD, STECI_FLASH_OP_MODE_IS_BOOTSTRAP, 0x00, 0x00 };
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp, *cmd;
	tSInt ret = OSAL_OK;

	// init the data mode
	flashBootstrapCmd[ETAL_MDR_DATA_MODE_INDEX] = data_mode;
	
    /* check data_mode */
    if ((data_mode != STECI_ACCESS_DATA_MODE) && (data_mode != STECI_ACCESS_FILE_MODE))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashBootstrap_MDR invalid data_mode parameter %d", data_mode);
        return OSAL_ERROR;
    }

    /* send flash bootstrap */
    cmd = (tU8 *)OSAL_pvMemoryAllocate(sizeof(flashBootstrapCmd) + 2 + payload_len);
	if (cmd == NULL)
	{
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, ETAL_INVALID_HANDLE,  sizeof(ETAL_HANDLE));
		return OSAL_ERROR;
	}

	OSAL_pvMemoryCopy((tPVoid)cmd, (tPCVoid)flashBootstrapCmd, sizeof(flashBootstrapCmd));
	cmd[sizeof(flashBootstrapCmd)] = (tU8) ((payload_len >> 8) & 0xFF);
	cmd[sizeof(flashBootstrapCmd) + 1] = (tU8) (payload_len & 0xFF);
	OSAL_pvMemoryCopy((tPVoid)(&cmd[sizeof(flashBootstrapCmd) + 2]), (tPCVoid)payload_ptr, payload_len);

	ret = ETAL_sendCommandSpecialTo_MDR(cmd, (sizeof(flashBootstrapCmd) + 2 + payload_len), &cstatus, TRUE, &resp, &resplen);
	OSAL_vMemoryFree((tVoid *)cmd);

    /* check flash bootstrap response is valid */
    if ((ret == OSAL_OK) && (((resplen > 8) && ((resp[8] != STECI_FLASH_CMD) || (resp[3] != 0))) || (resplen < 9)))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashBootstrap_MDR invalid flash bootstrap response error 0x%x len %d 0x%x", (resplen > 3)?resp[3]:0, resplen, (resplen > 8)?resp[8]:0);
        ret = OSAL_ERROR;
    }
	return ret;
}

/***************************
 *
 * ETAL_cmdSpecialFlashCheckFlasher_MDR
 *
 **************************/
tSInt ETAL_cmdSpecialFlashCheckFlasher_MDR(DCOP_targetMemEnumTy bootstrap_type)
{
#define ETAL_MDR_BOOTSTRAP_TYPE_INDEX	2

	tU8 flashCheckFlasherCmd[]  = { STECI_FLASH_CMD, STECI_FLASH_OP_MODE_IS_FLASHER_CHK, 0x00, 0x00, 0x00, 0x00 };
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt ret = OSAL_OK;

	/* indicate if we check flasher or SPI loader */
	flashCheckFlasherCmd[ETAL_MDR_BOOTSTRAP_TYPE_INDEX] = bootstrap_type;

    /* send flash check flasher */
    ret = ETAL_sendCommandSpecialTo_MDR(flashCheckFlasherCmd, sizeof(flashCheckFlasherCmd), &cstatus, TRUE, &resp, &resplen);

    /* check flash check flasher response is valid */
    if ((ret == OSAL_OK) && (((resplen > 8) && ((resp[8] != STECI_FLASH_CMD) || (resp[3] != 0))) || (resplen < 9)))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashCheckFlasher_MDR flash check flasher response error 0x%x len %d 0x%x", (resplen > 3)?resp[3]:0, resplen, (resplen > 8)?resp[8]:0);
        ret = OSAL_ERROR;
    }

	return ret;
}

/***************************
 *
 * ETAL_cmdSpecialFlashErase_MDR
 *
 **************************/
tSInt ETAL_cmdSpecialFlashErase_MDR(void)
{
	tU8 flashEraseCmd[]  = { STECI_FLASH_CMD, STECI_FLASH_OP_MODE_IS_ERASE, 0x00, 0x00, 0x00, 0x00 };
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp;
	tSInt ret = OSAL_OK;

    /* send flash erase */
    ret = ETAL_sendCommandSpecialTo_MDR(flashEraseCmd, sizeof(flashEraseCmd), &cstatus, TRUE, &resp, &resplen);

    /* check flash erase response is valid */
    if ((ret == OSAL_OK) && (((resplen > 8) && ((resp[8] != STECI_FLASH_CMD) || (resp[3] != 0))) || (resplen < 9)))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashErase_MDR flash erase response error 0x%x len %d 0x%x", (resplen > 3)?resp[3]:0, resplen, (resplen > 8)?resp[8]:0);
        ret = OSAL_ERROR;
    }

	return ret;
}

/***************************
 *
 * ETAL_cmdSpecialFlashProgramDownload_MDR
 *
 **************************/
tSInt ETAL_cmdSpecialFlashProgramDownload_MDR(DCOP_targetMemEnumTy bootstrap_type, DCOP_accessModeEnumTy data_mode, tU16 payload_len, tU8 *payload_ptr)
{
#define ETAL_MDR_DATA_MODE_INDEX		3
#define ETAL_MDR_BOOTSTRAP_TYPE_INDEX	2


	tU8 flashProgramDownloadCmd[] = { STECI_FLASH_CMD, STECI_FLASH_OP_MODE_IS_PROGRAM, 0x00, 0x00 };
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp, *cmd;
	tSInt ret = OSAL_OK;

	// init data mode and bootstrap_type	
	flashProgramDownloadCmd[ETAL_MDR_DATA_MODE_INDEX] = data_mode;
	flashProgramDownloadCmd[ETAL_MDR_BOOTSTRAP_TYPE_INDEX] = bootstrap_type;

    /* check bootstrap_type */
    if ((bootstrap_type != STECI_TARGET_SPI2FLASH) && (bootstrap_type != STECI_TARGET_SPI2MEM))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashProgramDownload_MDR invalid bootstrap_type parameter %d", bootstrap_type);
        return OSAL_ERROR;
    }

    /* check data_mode */
    if ((data_mode != STECI_ACCESS_DATA_MODE) && (data_mode != STECI_ACCESS_FILE_MODE))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashProgramDownload_MDR invalid data_mode parameter %d", data_mode);
        return OSAL_ERROR;
    }

    /* send flash program download */
    cmd = (tU8 *)OSAL_pvMemoryAllocate(sizeof(flashProgramDownloadCmd) + 2 + payload_len);
	if (cmd == NULL)
	{
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, ETAL_INVALID_HANDLE,  sizeof(ETAL_HANDLE));
		return OSAL_ERROR;
	}

    OSAL_pvMemoryCopy((tPVoid)cmd, (tPCVoid)flashProgramDownloadCmd, sizeof(flashProgramDownloadCmd));
    cmd[sizeof(flashProgramDownloadCmd)] = (tU8)((payload_len >> 8) & 0xFF);
    cmd[sizeof(flashProgramDownloadCmd) + 1] = (tU8)(payload_len & 0xFF);
    if (NULL != payload_ptr)
    {
        OSAL_pvMemoryCopy((tPVoid)(&cmd[sizeof(flashProgramDownloadCmd) + 2]), (tPCVoid)payload_ptr, payload_len);
    }

    ret = ETAL_sendCommandSpecialTo_MDR(cmd, (sizeof(flashProgramDownloadCmd) + 2 + payload_len), &cstatus, TRUE, &resp, &resplen);
    OSAL_vMemoryFree((tVoid *)cmd);

    /* check flash program download response is valid */
    if ((ret == OSAL_OK) && (((resplen > 8) && ((resp[8] != STECI_FLASH_CMD) || (resp[3] != 0))) || (resplen < 9)))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashProgramDownload_MDR invalid flash program response error 0x%x len %d 0x%x", (resplen > 3)?resp[3]:0, resplen, (resplen > 8)?resp[8]:0);
        ret = OSAL_ERROR;
    }
	return ret;
}

/***************************
 *
 * ETAL_cmdSpecialFlashDump_MDR
 *
 **************************/
tSInt ETAL_cmdSpecialFlashDump_MDR(DCOP_targetMemEnumTy bootstrap_type, DCOP_accessModeEnumTy data_mode, tU16 payload_len, tU8 *payload_ptr)
{
#define ETAL_MDR_DATA_MODE_INDEX		3
#define ETAL_MDR_BOOTSTRAP_TYPE_INDEX	2

	tU8 flashDumpCmd[] = { STECI_FLASH_CMD, STECI_FLASH_OP_MODE_IS_DUMP, 0x00, 0x00 };
	tU16 cstatus = 0;
	tU32 resplen = 0;
	tU8 *resp, *cmd;
	tSInt ret = OSAL_OK;

	// init data mode and bootstrap_type	
	flashDumpCmd[ETAL_MDR_DATA_MODE_INDEX] = data_mode;
	flashDumpCmd[ETAL_MDR_BOOTSTRAP_TYPE_INDEX] = bootstrap_type;


    /* check bootstrap_type */
    if ((bootstrap_type != STECI_TARGET_SPI2FLASH) && (bootstrap_type != STECI_TARGET_SPI2MEM))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashDump_MDR invalid bootstrap_type parameter %d", bootstrap_type);
        return OSAL_ERROR;
    }

    // TODO: handle STECI_ACCESS_DATA_MODE
    /* check data_mode */
    if ((data_mode == STECI_ACCESS_DATA_MODE) || (data_mode != STECI_ACCESS_FILE_MODE))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashDump_MDR invalid data_mode parameter %d", data_mode);
        return OSAL_ERROR;
    }

    /* send flash dump */
    cmd = (tU8 *)OSAL_pvMemoryAllocate(sizeof(flashDumpCmd) + 2 + payload_len);
	if (cmd == NULL)
	{
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, ETAL_INVALID_HANDLE,  sizeof(ETAL_HANDLE));
		return OSAL_ERROR;
	}

    OSAL_pvMemoryCopy((tPVoid)cmd, (tPCVoid)flashDumpCmd, sizeof(flashDumpCmd));
    cmd[sizeof(flashDumpCmd)] = (tU8)((payload_len >> 8) & 0xFF);
    cmd[sizeof(flashDumpCmd) + 1] = (tU8)(payload_len & 0xFF);
    OSAL_pvMemoryCopy((tPVoid)(&cmd[sizeof(flashDumpCmd) + 2]), (tPCVoid)payload_ptr, payload_len);

    ret = ETAL_sendCommandSpecialTo_MDR(cmd, (sizeof(flashDumpCmd) + 2 + payload_len), &cstatus, TRUE, &resp, &resplen);
    OSAL_vMemoryFree((tVoid *)cmd);

    /* check flash dump response is valid */
    if ((ret == OSAL_OK) && (((resplen > 8) && ((resp[8] != STECI_FLASH_CMD) || (resp[3] != 0))) || (resplen < 9)))
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ETAL_cmdSpecialFlashDump_MDR invalid flash program response error 0x%x len %d 0x%x", (resplen > 3)?resp[3]:0, resplen, (resplen > 8)?resp[8]:0);
        ret = OSAL_ERROR;
    }
	return ret;
}

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_ETAL_SUPPORT_DCOP_MDR && !CONFIG_COMM_DCOP_MDR_FIRMWARE_NO_DOWNLOAD

