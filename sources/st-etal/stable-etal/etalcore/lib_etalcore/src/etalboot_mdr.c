//!
//!  \file 		etalboot_mdr.c
//!  \brief 	<i><b> ETAL BOOT MDR high layer </b></i>
//!  \details   MDR-specific boot command high layer implementation
//!  \author 	David Pastor
//!

#include "osal.h"
#include "etalinternal.h"

#if defined(CONFIG_ETAL_SUPPORT_DCOP_MDR) && defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE)


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
static tSInt ETAL_download_MDR(const EtalDCOPAttr *dcop_attr);


/*****************************************************************
| functions
|----------------------------------------------------------------*/

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE_RENAME
/**************************************
 *
 * ETAL_renameBootFileMDR
 *
 *************************************/
/*
 * rename MDR firmware file by changing extension to .used
 *
 * Returns:
 *
 * OSAL_OK
 *  rename of MDR firmware file success
 *
 * OSAL_ERROR
 *  rename of MDR firmware file success
 */
tSInt ETAL_renameBootFileMDR(tChar *mdr_fw_name_found)
{
    tChar mdr_fw_name_used[ETAL_MDR_BOOT_FILENAME_MAX_SIZE], *pos_dot_mdr;

    /* replace filnename extension by .used */
    pos_dot_mdr = OSAL_ps8StringRSearchChar(mdr_fw_name_found, '.');
    if ((pos_dot_mdr == NULL) || (pos_dot_mdr[1] == '/'))
    {
        ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETAL_renameBootFileFoundMDR MDR_firmware_name doesn't have dot: %s", mdr_fw_name_found);
        OSAL_szStringNCopy(mdr_fw_name_used, mdr_fw_name_found, ETAL_MDR_BOOT_FILENAME_MAX_SIZE);
        OSAL_szStringConcat(mdr_fw_name_used, ".used");
    }
    else
    {
        OSAL_szStringNCopy(mdr_fw_name_used, mdr_fw_name_found, (size_t)(pos_dot_mdr - mdr_fw_name_found));
        mdr_fw_name_used[(pos_dot_mdr - mdr_fw_name_found)] = 0;
        OSAL_szStringConcat(mdr_fw_name_used, ".used");
    }

    /* move file only if different name */
    if (OSAL_s32StringCompare(mdr_fw_name_found, mdr_fw_name_used) != 0)
    {
        /* remove file .used */
        OSAL_remove(mdr_fw_name_used);

        /* rename file in .used */
        if (OSAL_rename(mdr_fw_name_found, mdr_fw_name_used) != 0)
        {
            ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETAL_isBootModeMDR ETAL_renameBootFileFoundMDR rename fails %s -> %s", mdr_fw_name_found, mdr_fw_name_used);
            return OSAL_ERROR; 
        }
    }

    return OSAL_OK;
}
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE_RENAME

/**************************************
 *
 * ETAL_isDoFlashOrDownloadOrDumpMDR
 *
 *************************************/
/*
 * check if do flash program or do flash dump is requested
 */
EtalDcopBootType ETAL_isDoFlashOrDownloadOrDumpMDR(tVoid)
{
	const EtalDCOPAttr *dcop_attr = ETAL_statusHardwareAttrGetDCOPAttr();

	if ((dcop_attr->m_isDisabled == FALSE) &&
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
		(((dcop_attr->m_doFlashProgram == TRUE) && (dcop_attr->m_pvGetImageContext != NULL)) ||
		((dcop_attr->m_doFlashDump == TRUE) && (dcop_attr->m_pvPutImageContext != NULL)))
#else
		(((dcop_attr->m_doFlashProgram == TRUE) && (dcop_attr->m_cbGetImage != NULL)) ||
		((dcop_attr->m_doFlashDump == TRUE) && (dcop_attr->m_cbPutImage != NULL)))
#endif
		)
	{
		return ETAL_DCOP_BOOT_FLASH;
	}
    else if ((dcop_attr->m_isDisabled == FALSE) &&
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
	((dcop_attr->m_doDownload== TRUE) && (dcop_attr->m_pvGetImageContext != NULL)))
#else
	((dcop_attr->m_doDownload == TRUE) && (dcop_attr->m_cbGetImage != NULL)))
#endif
    {
        return ETAL_DCOP_BOOT_LOAD;
    }
	else
	{
		return ETAL_DCOP_BOOT_REGULAR;
	}
}

/**************************************
 *
 * ETAL_doFlashOrDownloadOrDumpMDR
 *
 *************************************/
/*
 * download and rename dcop_fw.bin file
 */
tSInt ETAL_doFlashOrDownloadOrDumpMDR(tVoid)
{
	const EtalDCOPAttr *dcop_attr = ETAL_statusHardwareAttrGetDCOPAttr();

	if ((dcop_attr->m_isDisabled == TRUE) || (ETAL_download_MDR(dcop_attr) != ETAL_RET_SUCCESS))
	{
		return OSAL_ERROR;
	}
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE_RENAME
	else
	{
		/* rename MDR firmware to .used */
		if ((dcop_attr->m_doFlashProgram == TRUE) && (ETAL_renameBootFileMDR((tChar *)(((tChar **)(dcop_attr->m_pvGetImageContext))[1])) != OSAL_OK))
		{
			return OSAL_ERROR;
		}
	}
#endif
	return OSAL_OK;
}

/***************************
 *
 * ETAL_download_MDR
 *
 **************************/
static tSInt ETAL_download_MDR(const EtalDCOPAttr *dcop_attr)
{
	tSInt retval = ETAL_RET_SUCCESS, ret = OSAL_OK;
#ifndef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
	tU32 returnedByteNum/*, remainingByteNum*/;
	tU8 *blockp = NULL/*, *returnedBlockp*/;
	int retcgi/*, retcpi*/;
#endif // !CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
	DCOP_targetMemEnumTy target_mem;

	if ((dcop_attr->m_isDisabled == FALSE) && (dcop_attr->m_doFlashProgram == TRUE) && 
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
		(dcop_attr->m_pvGetImageContext != NULL)
#else
		(dcop_attr->m_cbGetImage != NULL)
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
		)
	{
		target_mem = STECI_TARGET_SPI2FLASH;
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "starting DCOP DAB Firmware flash");
	}
	else if ((dcop_attr->m_isDisabled == FALSE) && (dcop_attr->m_doDownload == TRUE) && 
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
			(dcop_attr->m_pvGetImageContext != NULL)
#else
			(dcop_attr->m_cbGetImage != NULL)
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
			)
	{
		target_mem = STECI_TARGET_SPI2MEM;
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "starting DCOP DAB Firmware download");
	}
	else if ((dcop_attr->m_isDisabled == FALSE) && (dcop_attr->m_doFlashDump == TRUE) && 
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
		(dcop_attr->m_pvPutImageContext != NULL)
#else
		(dcop_attr->m_cbPutImage != NULL)
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
		)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "starting DCOP DAB Firmware flash dump");
	}
	else
	{
		ret = OSAL_ERROR;
	}

	if (ret == OSAL_OK)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL, "boot mode ok");
		/* Send boot mode */
		ret = ETAL_cmdSpecialBootMode_MDR(STECI_BOOT_MODE);

		if (ret == OSAL_OK)
		{
			/* Send flash bootstrap */
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
			ret = ETAL_cmdSpecialFlashBootstrap_MDR(STECI_ACCESS_FILE_MODE, (tU16)OSAL_u32StringLength((tChar *)(((tChar **)(dcop_attr->m_pvGetImageContext))[0])) + 1, (tU8 *)(((tChar **)(dcop_attr->m_pvGetImageContext))[0]));
#else
			blockp = OSAL_pvMemoryAllocate(SPI_DEVICE_BUFFER_SIZE/*FLASH_WRITE_PAYLOAD_SIZE*/);
			if (blockp != NULL)
			{
				do
				{
					returnedByteNum = 0;
					retcgi = dcop_attr->m_cbGetImage(dcop_attr->m_pvGetImageContext, SPI_DEVICE_BUFFER_SIZE/*FLASH_WRITE_PAYLOAD_SIZE*/, blockp, &returnedByteNum, NULL, TRUE);
					ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "retcgi %d, returnedByteNum %d\n", retcgi, returnedByteNum);
					if ((retcgi == 0) && (returnedByteNum != 0))
					{
						ret = ETAL_cmdSpecialFlashBootstrap_MDR(STECI_ACCESS_DATA_MODE, returnedByteNum, blockp);
					}
				} while((retcgi == 0) && (returnedByteNum == SPI_DEVICE_BUFFER_SIZE/*FLASH_WRITE_PAYLOAD_SIZE*/) && (ret == OSAL_OK));
				if (retcgi != 0)
				{
					ret = OSAL_ERROR;
				}
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE

				if (ret == OSAL_OK)
				{
					/* Delay */
					OSAL_s32ThreadWait (15); // Delay by 15 ms

					/* Send flash check flasher */
					ret = ETAL_cmdSpecialFlashCheckFlasher_MDR(target_mem);

					if (ret == OSAL_OK)
					{
						ETAL_tracePrintSystem(TR_CLASS_APP_ETAL, "flasher check ok");
						if (TRUE == dcop_attr->m_doDownload)
						{
							/* Download firmware to DCOP */
							ret = ETAL_cmdSpecialFlashProgramDownload_MDR(STECI_TARGET_SPI2MEM, STECI_ACCESS_FILE_MODE, (tU16)OSAL_u32StringLength((tChar *)(((tChar **)(dcop_attr->m_sectDescrFilename)))) + 1, (tU8 *)(((tChar **)(dcop_attr->m_sectDescrFilename))));

							if (ret == OSAL_OK)
							{
								ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "DCOP DAB Firmware download complete!");
							}
							else
							{
								ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "DCOP DAB Firmware download failure (%d)", ret);
							}
						}
						if (dcop_attr->m_doFlashProgram == TRUE)
						{
							/* Send flash erase */
							ret = ETAL_cmdSpecialFlashErase_MDR();
							ETAL_tracePrintSystem(TR_CLASS_APP_ETAL, "flasher erase ok");

							if (ret == OSAL_OK)
							{
								/* Send flash program download */
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
								ret = ETAL_cmdSpecialFlashProgramDownload_MDR(STECI_TARGET_SPI2FLASH, STECI_ACCESS_FILE_MODE, (tU16)OSAL_u32StringLength((tChar *)(((tChar **)(dcop_attr->m_pvGetImageContext))[1])) + 1, (tU8 *)(((tChar **)(dcop_attr->m_pvGetImageContext))[1]));
#else
								do
								{
									returnedByteNum = 0;
									retcgi = dcop_attr->m_cbGetImage(dcop_attr->m_pvGetImageContext, SPI_DEVICE_BUFFER_SIZE/*FLASH_WRITE_PAYLOAD_SIZE*/, blockp, &returnedByteNum, NULL, FALSE);
									ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "retcgi %d, returnedByteNum %d\n", retcgi, returnedByteNum);
									if ((retcgi == 0) && (returnedByteNum != 0))
									{
										ret = ETAL_cmdSpecialFlashProgramDownload_MDR(STECI_TARGET_SPI2FLASH, STECI_ACCESS_DATA_MODE, returnedByteNum, blockp);
									}
								} while((retcgi == 0) && (returnedByteNum == SPI_DEVICE_BUFFER_SIZE/*FLASH_WRITE_PAYLOAD_SIZE*/) && (ret == OSAL_OK));
								if (retcgi != 0)
								{
									ret = OSAL_ERROR;
								}
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE

								if (ret == OSAL_OK)
								{
									ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "DCOP DAB Firmware flash complete!");
								}
								else
								{
									ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "DCOP DAB Firmware flash failure (%d)", ret);
								}
							}
						}

						if (ret == OSAL_OK)
						{
							if (dcop_attr->m_doFlashDump == TRUE)
							{
								/* Send flash dump */
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
								ret = ETAL_cmdSpecialFlashDump_MDR(STECI_TARGET_SPI2FLASH, STECI_ACCESS_FILE_MODE, (tU16)OSAL_u32StringLength((tChar *)(dcop_attr->m_pvPutImageContext)) + 1, (tU8 *)(dcop_attr->m_pvPutImageContext));
#else
								// TO DO: Flash Dump not working because steci_protocol doesn't support Flash Dump with STECI_ACCESS_DATA_MODE
								// new protocol layer interface for data mode need to be defined with perhaps autonotification responses for chunk of fash dump memory
#if 0
								//	ret = ETAL_cmdSpecialFlashDump_MDR(STECI_TARGET_SPI2FLASH, STECI_ACCESS_DATA_MODE, &returnedByteNum, &returnedBlockp);
								//	retcpi = dcop_attr->m_cbPutImage(dcop_attr->m_pvPutImageContext, returnedBlockp, returnedByteNum, 0);
								ret = OSAL_ERROR;
#else
								/* Send flash dump workaround */
								ret = ETAL_cmdSpecialFlashDump_MDR(STECI_TARGET_SPI2FLASH, STECI_ACCESS_FILE_MODE, (tU16)OSAL_u32StringLength(ETAL_MDR_FLASH_DUMP_FILENAME) + 1, (tU8 *)ETAL_MDR_FLASH_DUMP_FILENAME);
#endif
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
								if (ret == OSAL_OK)
								{
									ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "DCOP DAB Firmware flash dump complete!");
								}
								else
								{
									ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "DCOP DAB Firmware flash dump failure (%d)", ret);
								}
							}
						}
					}
				}
#ifndef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
			}
#endif // !CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
		}

		/* Send normal mode */
		if (ret == OSAL_OK)
		{
		    if (TRUE != dcop_attr->m_doDownload)
		    {
			ret = ETAL_cmdSpecialBootMode_MDR(STECI_NORMAL_MODE);
			}
		}
		else
		{
			(LINT_IGNORE_RET) ETAL_cmdSpecialBootMode_MDR(STECI_NORMAL_MODE);
		}

		// should not we wait after reboot ?
		// asume the reset is done and fw loaded
		if (TRUE != dcop_attr->m_doDownload)
		{
			OSAL_s32ThreadWait(DAB_FW_LOADING_TIME);
		}

#ifndef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
		if (ret == OSAL_OK)
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "reboot DCOP DAB in normal mode needed!");
		}
#endif /* !CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO */

	}

#ifndef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
	if (blockp != NULL)
	{
		OSAL_vMemoryFree(blockp);
	}
#endif

	if (ret != OSAL_OK)
	{
		retval = ETAL_RET_ERROR;
	}

	return retval;
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR && CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE

