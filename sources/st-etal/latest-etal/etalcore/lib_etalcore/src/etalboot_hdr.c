//!
//!  \file 		etalboot_mdr.c
//!  \brief 	<i><b> ETAL BOOT HD Radio high layer </b></i>
//!  \details   HD Radio-specific boot command high layer implementation
//!  \author 	David Pastor
//!

#include "osal.h"
#include "etalinternal.h"

#if defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO) && defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD)


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
static tSInt ETAL_download_HDR(const EtalDCOPAttr *dcop_attr);


/*****************************************************************
| functions
|----------------------------------------------------------------*/

/**************************************
 *
 * ETAL_isDoFlashOrDownloadOrDumpHDR
 *
 *************************************/
/*
 * check if do flash program or do flash dump is requested
 */
tBool ETAL_isDoFlashOrDownloadOrDumpHDR(tVoid)
{
	const EtalDCOPAttr *dcop_attr = ETAL_statusHardwareAttrGetDCOPAttr();

	if ((dcop_attr->m_isDisabled == FALSE) &&
		((((dcop_attr->m_doFlashProgram == TRUE) || (dcop_attr->m_doDownload == TRUE)) && 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		(dcop_attr->m_pvGetImageContext != NULL)
#else
		(dcop_attr->m_cbGetImage != NULL)
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		) ||
		((dcop_attr->m_doFlashDump == TRUE) && 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		(dcop_attr->m_pvPutImageContext != NULL)
#else
		(dcop_attr->m_cbPutImage != NULL)
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		))
		)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**************************************
 *
 * ETAL_isDoDownloadHDR
 *
 *************************************/
/*
 * check if do flash program or do flash dump is requested
 */
tBool ETAL_isDoDownloadHDR(tVoid)
{
	const EtalDCOPAttr *dcop_attr = ETAL_statusHardwareAttrGetDCOPAttr();

	if ((dcop_attr->m_isDisabled == FALSE) &&
		(dcop_attr->m_doFlashProgram == FALSE) && (dcop_attr->m_doDownload == TRUE) && 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		(dcop_attr->m_pvGetImageContext != NULL)
#else
		(dcop_attr->m_cbGetImage != NULL)
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/**************************************
 *
 * ETAL_doFlashOrDownloadOrDumpHDR
 *
 *************************************/
/*
 * download and rename dcop_fw.bin file
 */
tSInt ETAL_doFlashOrDownloadOrDumpHDR(tVoid)
{
	const EtalDCOPAttr *dcop_attr = ETAL_statusHardwareAttrGetDCOPAttr();

	if ((dcop_attr->m_isDisabled == TRUE) ||
		(ETAL_download_HDR(dcop_attr) != ETAL_RET_SUCCESS)
		)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/***************************
 *
 * ETAL_download_HDR
 *
 **************************/
static tSInt ETAL_download_HDR(const EtalDCOPAttr *dcop_attr)
{
	tSInt retval = ETAL_RET_SUCCESS, ret = OSAL_OK;
	HDR_targetMemOrInstanceIDTy trg_mem_or_instId;

	if ((dcop_attr->m_isDisabled == FALSE) && 
		(dcop_attr->m_doFlashProgram == TRUE) && (dcop_attr->m_doDownload == FALSE) && 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		(dcop_attr->m_pvGetImageContext != NULL)
#else
		(dcop_attr->m_cbGetImage != NULL)
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "starting DCOP HDR Firmware flashing");
	}
	else if ((dcop_attr->m_isDisabled == FALSE) && 
		(dcop_attr->m_doFlashProgram == FALSE) && (dcop_attr->m_doDownload == TRUE) && 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		(dcop_attr->m_pvGetImageContext != NULL)
#else
		(dcop_attr->m_cbGetImage != NULL)
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "starting DCOP HDR Firmware downloading");
	}
	else if ((dcop_attr->m_isDisabled == FALSE) &&
		(dcop_attr->m_doFlashDump == TRUE) && 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		(dcop_attr->m_pvPutImageContext != NULL)
#else
		(dcop_attr->m_cbPutImage != NULL)
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
		)
	{
		ret = OSAL_OK;
	}
	else
	{
		ret = OSAL_ERROR;
	}

	if ((ret == OSAL_OK)
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_VIA_CMD
		&& (dcop_attr->m_doDownload == TRUE)
#endif // CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_VIA_CMD
		)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL, "phase1 start");
		/* Reset DCOP and Load phase1 */
		/* phase 1 or phase 2&3, file mode of data mode, flash or download */
		trg_mem_or_instId.trg_mem = HDR_TARGET_SPI2FLASH;
		ret = ETAL_cmdSpecialFlash_HDR(HDR_FLASH_OP_MODE_IS_PHASE1, trg_mem_or_instId, 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
			HDR_ACCESS_FILE_MODE, 
#else
			HDR_ACCESS_DATA_MODE, 
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
			dcop_attr);
	}

	if (ret == OSAL_OK)
	{
#ifndef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_VIA_CMD
		/* wait at least 100 ms before starting phase2 */
		(void)OSAL_s32ThreadWait(100);
#endif // #ifndef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_VIA_CMD

		if (dcop_attr->m_doFlashProgram == TRUE)
		{
#ifndef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_VIA_CMD

			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL, "phase2&3 start");
			/* Load Phase 2 & 3 to flash DCOP */
			trg_mem_or_instId.trg_mem = HDR_TARGET_SPI2FLASH;
			ret = ETAL_cmdSpecialFlash_HDR(HDR_FLASH_OP_MODE_IS_PROGRAM, trg_mem_or_instId, 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
				HDR_ACCESS_FILE_MODE, 
#else
				HDR_ACCESS_DATA_MODE, 
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
				dcop_attr);

#else   // #ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_VIA_CMD

			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL, "phase2 via cmd start");
			/* Load Phase 2 via cmd to flash DCOP instance 1 */
			trg_mem_or_instId.instId = INSTANCE_1;
			ret = ETAL_cmdSpecialFlash_HDR(HDR_FLASH_OP_MODE_IS_PH2CMD, trg_mem_or_instId, 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
				HDR_ACCESS_FILE_MODE, 
#else
				HDR_ACCESS_DATA_MODE, 
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
				dcop_attr);

#endif // #ifndef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_VIA_CMD

			if (ret == OSAL_OK)
			{
				/* Reset DCOP */
				ret = ETAL_cmdSpecialReset_HDR();
			}
			else
			{
				/* Reset DCOP in any case */
				(LINT_IGNORE_RET) ETAL_cmdSpecialReset_HDR();
			}

			if (ret == OSAL_OK)
			{
				ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "DCOP HDR Firmware flash complete!");
			}
			else
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "DCOP HDR Firmware flash failure (%d)", ret);
			}
		}
		else if (dcop_attr->m_doDownload == TRUE)
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL, "phase2 start");
			/* Load Phase 2 to DCOP volatil memory */
			trg_mem_or_instId.trg_mem = HDR_TARGET_SPI2MEM;
			ret = ETAL_cmdSpecialFlash_HDR(HDR_FLASH_OP_MODE_IS_PROGRAM, trg_mem_or_instId, 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
				HDR_ACCESS_FILE_MODE, 
#else
				HDR_ACCESS_DATA_MODE, 
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
				dcop_attr);

			if (ret == OSAL_OK)
			{
				ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "DCOP HDR Firmware download complete!");
			}
			else
			{
				/* Reset DCOP */
				(LINT_IGNORE_RET) ETAL_cmdSpecialReset_HDR();
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "DCOP HDR Firmware download failure (%d)", ret);
			}
		}
		else
		{
			/* Nothing to do */
		}
	}

	if (ret == OSAL_OK)
	{
		if ((dcop_attr->m_isDisabled == FALSE) &&
			(dcop_attr->m_doFlashDump == TRUE) && 
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
			(dcop_attr->m_pvPutImageContext != NULL)
#else
			(dcop_attr->m_cbPutImage != NULL)
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
			)
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "starting DCOP HDR Firmware flash dump");
			// TODO
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "DCOP HDR Firmware flash dump failure");
			ret = OSAL_ERROR_NOT_IMPLEMENTED;
		}
	}

	if (ret != OSAL_OK)
	{
		retval = ETAL_RET_ERROR;
	}

	return retval;
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO && CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD

