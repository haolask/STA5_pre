//!
//!  \file 		etalapi.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, initialization functions
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
	#include "etalversion.h"
#endif

#if defined (CONFIG_BOARD_ACCORDO5) && defined (CONFIG_ETAL_SUPPORT_EARLY_TUNER) && defined(CONFIG_HOST_OS_LINUX)
#include "bsp_sta1095evb.h"
#endif

/***************************
 *
 * Macros
 *
 **************************/
/*!
 * \def		VERSION_TYPE_STRING_LEN
 * 			Size in bytes of the string used to store
 * 			the CMOST device flavour (STAR or DOT)
 * 			for #ETAL_versionSet_CMOST
 */
#define VERSION_TYPE_STRING_LEN 5
/*!
 * \def		VERSION_CHAN_STRING_LEN
 * 			Size in bytes of the string used to store
 * 			the CMOST channels (T for dual channel or
 * 			S for single channel) for #ETAL_versionSet_CMOST
 */
#define VERSION_CHAN_STRING_LEN 2
/*!
 * \def		ETAL_INACTIVE_DEVICE_STRING
 * 			String used by #etal_get_version to indicate
 * 			a device disabled by run-time configuration
 */
#define ETAL_INACTIVE_DEVICE_STRING  "inactive"

/***************************
 *
 * Local variables
 *
 **************************/
/*!
 * \var		etalCapabilitiesp
 * 			Pointer to the current capabilities, actualized based on the
 * 			parameter passed to #etal_initialize. Points to a memory
 * 			buffer dynamically allocated by #etal_get_capabilities
 * 			only when called. The buffer is freed by #etal_deinitialize.
 * \remark	NULL unless #etal_get_capabilities is called.
 */
static EtalHwCapabilities *etalCapabilitiesp;

/* IP address for external driver */
static tChar IPAddressExternalDriver[IP_ADDRESS_CHARLEN] = DEFAULT_IP_ADDRESS;

/***************************
 *
 * Local function
 *
 **************************/
// Protoype : 
#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
static ETAL_STATUS ETAL_version(EtalVersion *vers);
static tVoid ETAL_versionSetGlobal(EtalComponentVersion *vers);
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST)
static tSInt ETAL_versionSet_CMOST(EtalComponentVersion vers[]);
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
static tSInt ETAL_versionSet_MDR(EtalComponentVersion *vers);
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
static tVoid ETAL_version_substring(tChar *dst, tU32 dst_len, tChar *src, tU32 first, tU32 last);
static tSInt ETAL_versionSet_HDRADIO(EtalComponentVersion *vers);
#endif

static ETAL_STATUS ETAL_version(EtalVersion *vers);
static ETAL_STATUS ETAL_get_time(ETAL_HANDLE hReceiver, EtalTime *ETALTime);
static tVoid ETAL_tracePrintInitParameters(const EtalHardwareAttr *init_params);
static tVoid ETAL_tracePrintTunerInitParameters(const EtalTunerAttr *tuner_init_params);
static tVoid ETAL_tracePrintDcopInitParameters(const EtalDCOPAttr *dcop_init_params);
static ETAL_STATUS ETAL_clear_landscape(EtalMemoryClearConfig MemClearConfig);
static ETAL_STATUS ETAL_save_landscape(EtalNVMSaveConfig MemClearConfig);
static ETAL_STATUS ETAL_backup_context_for_early_audio(const etalCtxBackupEarlyAudioTy *CtxEarlyAudio);

#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_versionReset
 *
 **************************/
/*!
 * \brief		Resets a version string to all 0x00
 * \param[in]	vers - pointer to the version string
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_versionReset(EtalVersion *vers)
{
	(void)OSAL_pvMemorySet((tVoid *)vers, 0x00, sizeof(EtalVersion));
}

/***************************
 *
 * ETAL_versionSetGlobal
 *
 **************************/
/*!
 * \brief		Sets the ETAL version in the #EtalComponentVersion
 * \details		The ETAL version is composed of (major, middle, minor)
 * 				numbers and the SVN version number. The former
 * 				are set manually in a file named #etalversion.h; the
 * 				latter is also defined in the same file but done
 * 				through the shell command 'make version'.
 * \param[in,out] vers - pointer to the version string
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_versionSetGlobal(EtalComponentVersion *vers)
{
	vers->m_major  = ETAL_VERSION_MAJOR;
	vers->m_middle = ETAL_VERSION_MIDDLE;
	vers->m_minor  = ETAL_VERSION_MINOR;
	vers->m_build  = ETAL_VERSION_SVN;
	(void)OSAL_s32NPrintFormat(vers->m_name, ETAL_VERSION_NAME_MAX, "ETAL %d.%d.%d build %d", ETAL_VERSION_MAJOR, ETAL_VERSION_MIDDLE, ETAL_VERSION_MINOR, ETAL_VERSION_SVN);
	vers->m_isValid = TRUE;
}

#if defined (CONFIG_ETAL_SUPPORT_CMOST)
/***************************
 *
 * ETAL_versionSet_CMOST
 *
 **************************/
/*!
 * \brief		Sets the version string for the CMOST
 * \details		Reads from the CMOST register the firmware version currently
 * 				loaded (register *IDX_CMT_mainY_st_version_info__0__*). Builds
 * 				the m_name field of the #EtalComponentVersion with the firmware
 * 				version and the device name, taken from the #etalTuner array
 * 				(STAR/DOT, single/dual).
 *
 * 				Sets the version only for CMOST devices which are enabled
 * 				by run-time configuration; for the other devices it
 * 				sets the version string to #ETAL_INACTIVE_DEVICE_STRING
 * 				because the version is read from the device so it must
 * 				be initialized
 * \param[in,out] vers - array of version strings, one per #ETAL_CAPA_MAX_TUNER tuner
 * \return		OSAL_ERROR - Error accessing the device, *vers* is zero-filled
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_versionSet_CMOST(EtalComponentVersion vers[])
{
	ETAL_STATUS ret;
	tU32 i;
	ETAL_HANDLE hTuner;
	tU32 param[1] = {IDX_CMT_mainY_st_version_info__0__};
	tU32 response[1];
	tU16 length;
	tU8 major, middle, minor;
	tChar cmost_flavour[VERSION_TYPE_STRING_LEN];
	tChar cmost_channels[VERSION_CHAN_STRING_LEN];
	EtalDeviceType device;

	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		(void)OSAL_pvMemorySet((tVoid *)&vers[i], 0x00, sizeof(EtalComponentVersion));
		hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)i);
		if ((ETAL_tunerGetType(hTuner) != deviceUnknown) &&
			ETAL_statusHardwareAttrIsTunerActive(hTuner))
		{
			ret = ETAL_read_parameter_nolock(hTuner, fromIndex, param, 1, response, &length);
			if (ret != ETAL_RET_SUCCESS)
			{
				return OSAL_ERROR;
			}
			else if (length != 1)
			{
				return OSAL_ERROR;
			}
			else
			{
				/* Nothing to do */
			}
			major =  (tU8)((response[0] & 0x00FF0000) >> 16);
			middle = (tU8)((response[0] & 0x0000FF00) >>  8);
			minor =  (tU8)((response[0] & 0x000000FF) >>  0);
			vers[i].m_major =  major;
			vers[i].m_middle = middle;
			vers[i].m_minor =  minor;
			device = ETAL_tunerGetType(hTuner);
			if ((device & deviceSTAR) == deviceSTAR)
			{
				(void)OSAL_s32NPrintFormat(cmost_flavour, VERSION_TYPE_STRING_LEN, "STAR");
			}
			else if ((device & deviceDOT) == deviceDOT)
			{
				(void)OSAL_s32NPrintFormat(cmost_flavour, VERSION_TYPE_STRING_LEN, "DOT");
			}
			else
			{
				(void)OSAL_s32NPrintFormat(cmost_flavour, VERSION_TYPE_STRING_LEN, "????");
			}
			if ((device & deviceSingleFE) == deviceSingleFE)
			{
				(void)OSAL_s32NPrintFormat(cmost_channels, VERSION_CHAN_STRING_LEN, "S");
			}
			else if ((device & deviceTwinFE) == deviceTwinFE)
			{
				(void)OSAL_s32NPrintFormat(cmost_channels, VERSION_CHAN_STRING_LEN, "T");
			}
			else
			{
				(void)OSAL_s32NPrintFormat(cmost_channels, VERSION_CHAN_STRING_LEN, "?");
			}
			(void)OSAL_s32NPrintFormat(vers[i].m_name, ETAL_VERSION_NAME_MAX, "CMOST[%u] %s-%s %d.%d.%d", i+1, cmost_flavour, cmost_channels, major, middle, minor);
			vers[i].m_isValid = TRUE;
		}
		else
		{
			(void)OSAL_s32NPrintFormat(vers[i].m_name, ETAL_VERSION_NAME_MAX, ETAL_INACTIVE_DEVICE_STRING);
			vers[i].m_isValid = TRUE;
		}
	}
	return OSAL_OK;
}
#endif // CONFIG_ETAL_SUPPORT_CMOST

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/***************************
 *
 * ETAL_versionSet_MDR
 *
 **************************/
/*!
 * \brief		Sets the version string for the DAB DCOP
 * \details		Parses the values returned by the 'Get Firmware Version'
 * 				DAB DCOP command.
 *
 * 				If the device is disabled from ETAL configuration, only
 * 				the m_name field of #EtalComponentVersion is filled with
 * 				the string #ETAL_INACTIVE_DEVICE_STRING and m_isValid set to
 * 				TRUE, all other	fields are zero-filled.
 * \param[in]	vers - pointer to variable allocated by the caller and filled
 * 				       by the function
 * \return		OSAL_ERROR - Error accessing the device, *vers* is zero-filled
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_versionSet_MDR(EtalComponentVersion *vers)
{
	tSInt retval;
	tU32 year, month, day, major, minor, internal, build, boardType;

	OSAL_pvMemorySet((tVoid *)vers, 0x00, sizeof(EtalComponentVersion));
	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
		retval = ETAL_cmdGetFirmwareVersion_MDR(&year, &month, &day, &major, &minor, &internal, &build, &boardType);
		if (retval != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		vers->m_major = major;
		vers->m_minor = minor;
		vers->m_build = build;
		OSAL_s32NPrintFormat(vers->m_name, ETAL_VERSION_NAME_MAX, "MDR %u.0.%u build %u %u-%u-%u %s %s", 
				major, minor, build, year, month, day, 
				(((boardType&ETAL_VERSION_DCOP_RAMLESS_FLAG)==ETAL_VERSION_DCOP_RAMLESS_FLAG)?"- RAMless":""), 
				(((boardType&ETAL_VERSION_DCOP_NVMLESS_FLAG)==ETAL_VERSION_DCOP_NVMLESS_FLAG)?"- NVMless":""));
		vers->m_isValid = TRUE;
	}
	else
	{
		OSAL_s32NPrintFormat(vers->m_name, ETAL_VERSION_NAME_MAX, ETAL_INACTIVE_DEVICE_STRING);
		vers->m_isValid = TRUE;
	}
	return OSAL_OK;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
/***************************
 *
 * ETAL_version_substring
 *
 **************************/
/*!
 * \brief		Returns a NULL-terminated substring of a given string
 * \remark		Custom implementation for the ETAL_versionSet_HDRADIO,
 * 				almost no error checking
 * \param[in]	dst - pointer to the buffer where the function stores the substring
 * \param[in]	dst_line - max length of the returned string. If the *first*, *last*
 * 				           parameters would result in longer string, the function
 * 				           exits returning null length string
 * \param[in]	src - pointer to the original string
 * \param[in]	first - first character in the *str* to copy
 * \param[in]	last - last character in the *str* to copy
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_version_substring(tChar *dst, tU32 dst_len, tChar *src, tU32 first, tU32 last)
{
	tU32 i;
	tChar *loc_src;

	if (last - first + 1 > dst_len)
	{
		ASSERT_ON_DEBUGGING(0);
		*dst = '\0';
		return;
	}
	loc_src = src + first;
	for (i = first; i < last; i++)
	{
		*dst++ = *loc_src++;
	}
	*dst = '\0';
}

/***************************
 *
 * ETAL_versionSet_HDRADIO
 *
 **************************/
/*!
 * \brief		Sets the version string for the HDRadio DCOP
 * \details		The version string returned by the DCOP has the following
 * 				format:
 * 				"CCCCCCCC-HHHHHHHH-SSSSSSSS-RVVVV.BBB", e.g.
 * 				"  STA680-51001569-0D000033-C0006.300"
 * 				The function extracts the following info:
 * 				- VVVV goes into m_major field of #EtalComponentVersion
 * 				- BBB  goes into the m_build field of #EtalComponentVersion
 * 				The whole string is copied as-is into the m_name field.
 *
 * 				If the device is disabled from ETAL configuration, only
 * 				the m_name field of #EtalComponentVersion is filled with
 * 				the string #ETAL_INACTIVE_DEVICE_STRING and m_isValid set to
 * 				TRUE, all other	fields are zero-filled.
 * \param[in]	vers - pointer to variable allocated by the caller and filled
 * 				       by the function
 * \return		OSAL_ERROR - Error accessing the device, *vers* is zero-filled
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_versionSet_HDRADIO(EtalComponentVersion *vers)
{
#define LOC_STR_LEN 6
	tSInt retval;
	tChar str[ETAL_HDRADIO_SWVER_MAX_STRING_LEN];
	tChar str_loc[LOC_STR_LEN];
	tU32 version, build;

	(void)OSAL_pvMemorySet((tVoid *)vers, 0x00, sizeof(EtalComponentVersion));
	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
		retval = ETAL_cmdSysVersion_HDRADIO(str);
		if (retval != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		ETAL_version_substring(str_loc, LOC_STR_LEN, str, 28, 32);
		version = (tU32)OSAL_s32AsciiToS32(str_loc);

		ETAL_version_substring(str_loc, LOC_STR_LEN, str, 33, 36);
		build = (tU32)OSAL_s32AsciiToS32(str_loc);

		vers->m_major = (tU8) (version & 0xFF);
		vers->m_build = build;
		OSAL_szStringCopy(vers->m_name, str);
		vers->m_isValid = TRUE;
	}
	else
	{
		(void)OSAL_s32NPrintFormat(vers->m_name, ETAL_VERSION_NAME_MAX, ETAL_INACTIVE_DEVICE_STRING);
		vers->m_isValid = TRUE;
	}
	return OSAL_OK;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

/***************************
 *
 * ETAL_clear_landscape
 *
 **************************/
/*!
 * \brief		Clear databases
 * \param[in]	MemClearConfig - Memory clear configuration
 * \return		ETAL_RET_INVALID_HANDLE - illegal Receiver handle
 * \return		ETAL_RET_INVALID_RECEIVER - hReceiver is not configured
 * \return		ETAL_RET_ERROR - the device does not respond
 * \return		ETAL_RET_SUCCESS - devices respond
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_clear_landscape(EtalMemoryClearConfig MemClearConfig)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
    tU8 clearMemory;

    clearMemory = (MemClearConfig.m_clear_DAB_non_volatile_memory == TRUE) ? ETAL_DAB_NVM_CLEAR_DAB_DATABASE : 0;
    clearMemory |= (MemClearConfig.m_clear_DAB_volatile_memory == TRUE) ? ETAL_DAB_NVM_CLEAR_VOLATILE_DATABASE : 0;

    if (ETAL_cmdClearDatabase_MDR(clearMemory) != OSAL_OK)
    {
    	ret = ETAL_RET_ERROR;
    }
#endif

    if ((MemClearConfig.m_clear_AMFM_volatile_memory == TRUE) ||
    	(MemClearConfig.m_clear_AMFM_non_volatile_memory == TRUE) ||
    	(MemClearConfig.m_clear_HD_volatile_memory == TRUE) ||
    	(MemClearConfig.m_clear_HD_non_volatile_memory == TRUE))
    {
    	ret = ETAL_RET_NOT_IMPLEMENTED;
    }
    return ret;
}

/***************************
 *
 * ETAL_save_landscape
 *
 **************************/
/*!
 * \brief		Save databases
 * \param[in]	NVMSaveConfig - Memory clear configuration
 * \return		ETAL_RET_INVALID_HANDLE - illegal Receiver handle
 * \return		ETAL_RET_INVALID_RECEIVER - hReceiver is not configured
 * \return		ETAL_RET_ERROR - the device does not respond
 * \return		ETAL_RET_SUCCESS - devices respond
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_save_landscape(EtalNVMSaveConfig NVMSaveConfig)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
    if (NVMSaveConfig.m_save_DAB_landscape == TRUE)
    {
    	if (ETAL_cmdPowerDown_MDR(ETAL_DAB_NVM_SAVE_DAB_LANDSCAPE) != OSAL_OK)
    	{
    		ret = ETAL_RET_ERROR;
    	}
    }
#endif

    if ((NVMSaveConfig.m_save_AMFM_landscape == TRUE) ||
    	(NVMSaveConfig.m_save_HD_landscape == TRUE))
    {
		ret = ETAL_RET_NOT_IMPLEMENTED;
    }
    return ret;
}


/***************************
 *
 * ETAL_backup_context_for_early_audio
 *
 **************************/
/*!
 * \brief		Save the Tuner context for early audio feature
 * \param[in]	CtxEarlyAudio - pointer to strucure containing parameters to save
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_PARAMETER_ERR - NULL *status* parameter
 * \return		ETAL_RET_ERROR - the parameters is not saved due to a FS error
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_backup_context_for_early_audio(const etalCtxBackupEarlyAudioTy *CtxEarlyAudio)
{
#if defined (CONFIG_BOARD_ACCORDO5) && defined (CONFIG_ETAL_SUPPORT_EARLY_TUNER) && defined(CONFIG_HOST_OS_LINUX)
	tS32 ret;
	BSPCtxBackupEarlyAudioTy BSPCtxEarlyAudio;

	BSPCtxEarlyAudio.Freq = CtxEarlyAudio->Freq;
	BSPCtxEarlyAudio.Band = (tU32)(CtxEarlyAudio->Band);

	ret = BSP_backup_context_for_early_audio(&BSPCtxEarlyAudio);
	if(ret < 0)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Save ctx in file succeed. Freq = %d, Band = %x", CtxEarlyAudio->Freq, CtxEarlyAudio->Band);
		return ETAL_RET_ERROR;
	}

#endif
	return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_version
 *
 **************************/
/*!
 * \brief		Fills the #EtalVersion for all devices in the system
 * \param[in]	vers - pointer to variable allocated by the caller and filled
 * 				       by the function
 * \return		ETAL_RET_PARAMETER_ERR - NULL function parameter
 * \return		ETAL_RET_ERROR - Error accessing one or more of the devices, *vers*
 * 				                 might be incomplete
 * \return		ETAL_RET_SUCCESS
 * \see			ETAL_versionSetGlobal, ETAL_versionSet_CMOST, ETAL_versionSet_MDR, ETAL_versionSet_HDRADIO
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_version(EtalVersion *vers)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (vers == NULL)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		ETAL_versionReset(vers);
		ETAL_versionSetGlobal(&vers->m_ETAL);
#if defined (CONFIG_ETAL_SUPPORT_CMOST)
		if (ETAL_versionSet_CMOST(vers->m_CMOST) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
		if (ETAL_versionSet_MDR(&vers->m_MDR) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
		if (ETAL_versionSet_HDRADIO(&vers->m_HDRadio) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
#endif
	}

	return ret;
}
#endif // CONFIG_ETAL_HAVE_GET_VERSION || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_tracePrintInitParameters
 *
 **************************/
/*!
 * \brief		Print the #etal_initialize parameter
 * \param[in]	init_params - pointer to the structure to be printed (NULL is accepted)
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_tracePrintInitParameters(const EtalHardwareAttr *init_params)
{
	tU32 i;

	if (init_params != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "init_params (CouVar: %d, disHea: %d, disHeaUse: %d, defLev: %d, defLevUse: %d, NVMLoaConDAB: %d, NVMLoaConAMFM: %d, NVMLoaConHD: %d)",
			init_params->m_CountryVariant,
			init_params->m_traceConfig.m_disableHeader,
			init_params->m_traceConfig.m_disableHeaderUsed,
			init_params->m_traceConfig.m_defaultLevel,
			init_params->m_traceConfig.m_defaultLevelUsed,
			init_params->m_NVMLoadConfig.m_load_DAB_landscape,
			init_params->m_NVMLoadConfig.m_load_AMFM_landscape,
			init_params->m_NVMLoadConfig.m_load_HD_landscape);

		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "init_params (m_DCOPAttr: (m_isDisabled: %d, m_doFlashDump: %d, m_doFlashProgram: %d, m_doDownload: %d, m_cbGetImage: 0x%x, m_pvGetImageContext: %p, m_cbPutImage: 0x%x, m_pvPutImageContext: %p, m_sectDescrFilename %s))",
			init_params->m_DCOPAttr.m_isDisabled, init_params->m_DCOPAttr.m_doFlashDump, init_params->m_DCOPAttr.m_doFlashProgram,
			init_params->m_DCOPAttr.m_doDownload, init_params->m_DCOPAttr.m_cbGetImage, init_params->m_DCOPAttr.m_pvGetImageContext,
			init_params->m_DCOPAttr.m_cbPutImage, init_params->m_DCOPAttr.m_pvPutImageContext, init_params->m_DCOPAttr.m_sectDescrFilename);

		for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "init_params (TunerAttr: (TunerId = %d, m_isDisable %d, m_useDownloadImage %d, m_DownloadImageSize %d, m_useCustomParam %d, m_CustomParamSize %d, XTALAli: %d, XTALAli: %d))",
				i,
				init_params->m_tunerAttr[i].m_isDisabled,
				init_params->m_tunerAttr[i].m_useDownloadImage, init_params->m_tunerAttr[i].m_DownloadImageSize,
				init_params->m_tunerAttr[i].m_CustomParam, init_params->m_tunerAttr[i].m_CustomParamSize,
				init_params->m_tunerAttr[i].m_useXTALalignment, init_params->m_tunerAttr[i].m_XTALalignment);
		}
	}
}

/***************************
 *
 * ETAL_tracePrintTunerInitParameters
 *
 **************************/
/*!
 * \brief		Print the #etal_tuner_initialize parameter
 * \param[in]	init_params - pointer to the structure to be printed (NULL is accepted)
 * \callgraph
 * \callergraph
 */

static tVoid ETAL_tracePrintTunerInitParameters(const EtalTunerAttr *tuner_init_params)
{
	if (tuner_init_params != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "init_params (TunerAttr: (m_isDisable %d, m_useDownloadImageuse %d, m_DownloadImageSize %d, m_useCustomParam %d, m_CustomParamSize %d, XTALAli: %d, XTALAli: %d)",
			tuner_init_params->m_isDisabled,
			tuner_init_params->m_useDownloadImage, tuner_init_params->m_DownloadImageSize,
			tuner_init_params->m_CustomParam, tuner_init_params->m_CustomParamSize,
			tuner_init_params->m_useXTALalignment, tuner_init_params->m_XTALalignment);

	}
}

/***************************
 *
 * ETAL_tracePrintDcopInitParameters
 *
 **************************/
/*!
 * \brief		Print the #etal_tuner_initialize parameter
 * \param[in]	init_params - pointer to the structure to be printed (NULL is accepted)
 * \callgraph
 * \callergraph
 */

static tVoid ETAL_tracePrintDcopInitParameters(const EtalDCOPAttr *dcop_init_params)
{
	if (dcop_init_params != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "init_params (m_DCOPAttr: (m_isDisabled: %d, m_doFlashDump: %d, m_doFlashProgram: %d, m_doDownload: %d, m_cbGetImage: 0x%x, m_pvGetImageContext: %p, m_cbPutImage: 0x%x, m_pvPutImageContext: %p))",
			dcop_init_params->m_isDisabled, dcop_init_params->m_doFlashDump, dcop_init_params->m_doFlashProgram,
			dcop_init_params->m_doDownload, dcop_init_params->m_cbGetImage, dcop_init_params->m_pvGetImageContext,
			dcop_init_params->m_cbPutImage, dcop_init_params->m_pvPutImageContext);

	}
}



/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * etal_initialize
 *
 **************************/
/*!
 * \brief		Initialize ETAL, OSAL and the hardware devices controlled by ETAL
 * \details		Only if initialization was successful prints the ETAL version
 * 				at the end of the procedure (provided the right trace level is
 * 				enabled).
 *
 * 				In case of error undoes all the initialization done to leave
 * 				the system in the same state as it was before the call to 
 * 				etal_intialize.
 *
 * 				In case of error ETAL is not available; details on the
 * 				reason for failure are available calling #etal_get_init_status
 * \remark		Prints done through the ETAL trace system (which relies
 * 				on OSAL's #OSALUTIL_s32TracePrintf) are not available
 * 				until the successful return of this function, if 
 * 				ETAL is configured with CONFIG_TRACE_ASYNC.
 * \param[in]	init_params - pointer to initialization parameters
 * \return		ETAL_RET_ALREADY_INITIALIZED - the function was already successfully
 * 				                               invoked
 * \return		ETAL_RET_NOT_INITIALIZED - Memory allocation/thread allocation/semaphore creation error
 * \return		ETAL_RET_NO_HW_MODULE - Device communication error
 * \return		ETAL_RET_PARAMETER_ERR - Error parsing the function parameter
 * \return		ETA_RET_SUCCESS - ETAL is available
 * \see			ETAL_init
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_initialize(const EtalHardwareAttr *init_params)
{
	tSInt retosal;
#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
	EtalVersion vl_EtalVersion;
#endif
	ETAL_STATUS ret;

	/*************************
	 * state_initStart
	 *
	 * initial check
	 *************************/
	 
	if (ETAL_statusIsInitialized())
	{
		ret = ETAL_RET_ALREADY_INITIALIZED;
		goto exit;
	}
	
	ETAL_initStatusSetState(state_initStart);

	/* init state and device status are managed from ETAL_init */
#if defined(CONFIG_HOST_OS_FREERTOS)
	retosal = ETAL_init_Light(init_params, TRUE);
#else	/* CONFIG_HOST_OS_FREERTOS */
	retosal = ETAL_init(init_params, TRUE);
#endif /* CONFIG_HOST_OS_FREERTOS */

	/***************************************************************************/
	/* if using CONFIG_TRACE_ASYNC printfs are not available before this point */
	/***************************************************************************/

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_initialize()");
	ETAL_tracePrintInitParameters(init_params);

	ret = ETAL_RET_SUCCESS;
	if (retosal != OSAL_OK)
	{
		if (retosal == OSAL_ERROR_DEVICE_INIT)
		{
			ret = ETAL_RET_NOT_INITIALIZED;
		}
		else if (retosal == OSAL_ERROR_DEVICE_NOT_OPEN)
		{
			ret = ETAL_RET_NO_HW_MODULE;
		}
		else if (retosal == OSAL_ERROR_INVALID_PARAM)
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else if (retosal != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}

	if (ret == ETAL_RET_SUCCESS)
	{
#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
		if (ETAL_version(&vl_EtalVersion) == ETAL_RET_SUCCESS)
		{
			ETAL_tracePrintVersion(&vl_EtalVersion);
		}
#endif
	}
	else
	{
#if !defined(CONFIG_HOST_OS_FREERTOS)
		ETAL_initRollback(TRUE);
#endif
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_initialize() = %s", ETAL_STATUS_toString(ret));

exit:
	return ret;
}

/***************************
 *
 * etal_setTcpIpAddress
 *
 **************************/
/*!
 * \brief		Set TCP/IP address for external driver
 * \param[in]	address - Pointer on IP address
 * \return		ETAL_RET_SUCCESS - IP address is set.
 * \return		ETAL_RET_ERROR - Error (default IP address returned)
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_setTcpIpAddress(tChar* address)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if(address != NULL)
	{
		strncpy ( IPAddressExternalDriver, address, IP_ADDRESS_CHARLEN );
	}
	else
	{
		strncpy ( IPAddressExternalDriver, DEFAULT_IP_ADDRESS, IP_ADDRESS_CHARLEN );
	}

	return ret;
}

/**************************************
 *
 * ETAL_getIPAddressForExternalDriver
 *
 *************************************/
/*!
 * \brief		Returns the IP address for external driver
 * \return		return IP address used for external driver
 * \callgraph
 * \callergraph
 */
tChar* ETAL_getIPAddressForExternalDriver(tVoid)
{
	return IPAddressExternalDriver;
}

/***************************
 *
 * etal_tuner_initialize
 *
 **************************/
/*!
 * \brief		Initialize tuner hardware devices controlled by ETAL
 * \details		Only if initialization was successful prints the Tuner version
 * 				at the end of the procedure (provided the right trace level is
 * 				enabled).
 *				The ETAL layer should already be initialized 
 *
 * \param[in]	deviceID - the device ID of the CMOST 
 * \param[in]	tuner_init_params - pointer to initialization parameters for the CMOST
 * \param[in]	IsAlreadyStarted - indicated if the tuner is already started : in early audio use case
 *			the tuner may have be started in advance. Setting this parameter avoids to reset/reload the tuner
 *		 	if tuner is not already started, it will be reset/fw reload... as for a normal startup
 *
 * \return		ETAL_RET_ALREADY_INITIALIZED - the function was already successfully
 * 				                               invoked
 * \return		ETAL_RET_NOT_INITIALIZED - Memory allocation/thread allocation/semaphore creation error
 * \return		ETAL_RET_NO_HW_MODULE - Device communication error
 * \return		ETAL_RET_PARAMETER_ERR - Error parsing the function parameter
 * \return		ETA_RET_SUCCESS - ETAL is available
 * \see			ETAL_init
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_tuner_initialize(tU32 deviceID, const EtalTunerAttr *tuner_init_params, tBool IsAlreadyStarted)
{
	tSInt retosal;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
	EtalVersion vl_EtalVersion;
#endif

	/*************************
	 * state_initStart
	 *
	 * initial check
	 *************************/
	if (false == ETAL_statusIsInitialized())
	{
		return ETAL_RET_NOT_INITIALIZED;
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_tuner_initialize(DeviceID = %d, tunerIsAlreadyStarted = %d)", deviceID, IsAlreadyStarted);
	ETAL_tracePrintTunerInitParameters(tuner_init_params);


	/* init state and device status are managed from ETAL_init */

	retosal = ETAL_tuner_init(deviceID, tuner_init_params, IsAlreadyStarted);

	if (retosal != OSAL_OK)
	{
		if (retosal == OSAL_ERROR_DEVICE_INIT)
		{
			ret = ETAL_RET_NOT_INITIALIZED;
		}
		else if (retosal == OSAL_ERROR_DEVICE_NOT_OPEN)
		{
			ret = ETAL_RET_NO_HW_MODULE;
		}
		else if (retosal == OSAL_ERROR_INVALID_PARAM)
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else if (retosal != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}


	if (ret == ETAL_RET_SUCCESS)
	{
#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
		if (ETAL_version(&vl_EtalVersion) == ETAL_RET_SUCCESS)
		{
			ETAL_tracePrintVersion(&vl_EtalVersion);
		}
#endif
	}
	else
	{
		ETAL_initRollback(TRUE);
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_tuner_initialize() = %s", ETAL_STATUS_toString(ret));

	return ret;
}

/***************************
 *
 * etal_dcop_initialize
 *
 **************************/
/*!
 * \brief		Initialize dcop hardware devices controlled by ETAL
 * \details		Only if initialization was successful prints the Tuner version
 * 				at the end of the procedure (provided the right trace level is
 * 				enabled).
 *				The ETAL layer should already be initialized 
 *
 * \param[in] dcop_init_params - pointer to initialization parameters for the CMOST
 * \param[in] InitType - indicated the initialization type f the tuner is already started : in early audio use case
 *			the tuner may have be started in advance. Setting this parameter avoids to reset/reload the tuner
 *		 	if tuner is not already started, it will be reset/fw reload... as for a normal startup
 *
 * \return		ETAL_RET_ALREADY_INITIALIZED - the function was already successfully
 * 				                               invoked
 * \return		ETAL_RET_NOT_INITIALIZED - Memory allocation/thread allocation/semaphore creation error
 * \return		ETAL_RET_NO_HW_MODULE - Device communication error
 * \return		ETAL_RET_PARAMETER_ERR - Error parsing the function parameter
 * \return		ETA_RET_SUCCESS - ETAL is available
 * \see			ETAL_init
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_dcop_initialize(const EtalDCOPAttr *dcop_init_params, EtalDcopInitTypeEnum InitType)
{
	tSInt retosal;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#if !defined (CONFIG_HOST_OS_FREERTOS)
#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
	EtalVersion vl_EtalVersion;
#endif
#endif

	/*************************
	 * state_initStart
	 *
	 * initial check
	 *************************/
	ETAL_initStatusSetState(state_initStart);

	if (false == ETAL_statusIsInitialized())
	{
		return ETAL_RET_NOT_INITIALIZED;
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_dcop_initialize(InitType : %d)", InitType);
	ETAL_tracePrintDcopInitParameters(dcop_init_params);


	/* init state and device status are managed from ETAL_init */

#if defined(CONFIG_HOST_OS_FREERTOS)
	retosal = ETAL_Dcop_init_Light(dcop_init_params, InitType);
	if (retosal != OSAL_OK)
	{
		ret = ETAL_RET_ERROR;
	}

#else
	retosal = ETAL_Dcop_init(dcop_init_params, InitType);

	if (retosal != OSAL_OK)
	{
		if (retosal == OSAL_ERROR_DEVICE_INIT)
		{
			ret = ETAL_RET_NOT_INITIALIZED;
		}
		else if (retosal == OSAL_ERROR_DEVICE_NOT_OPEN)
		{
			ret = ETAL_RET_NO_HW_MODULE;
		}
		else if (retosal == OSAL_ERROR_INVALID_PARAM)
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else if (retosal != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}


	// in case of 'only reset' the DCOP SPI/STECI interface is not active
	// no version read
	//
	if (ETAL_DCOP_INIT_RESET_ONLY != InitType)
	{
		if (ret == ETAL_RET_SUCCESS)
		{
#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
			if (ETAL_version(&vl_EtalVersion) == ETAL_RET_SUCCESS)
			{
				ETAL_tracePrintVersion(&vl_EtalVersion);
			}
#endif
		}
		else
		{
			ETAL_initRollback(TRUE);
		}
	}
	
#endif /* CONFIG_HOST_OS_FREERTOS */

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_dcop_initialize() = %s", ETAL_STATUS_toString(ret));

	return ret;
}


/***************************
 *
 * etal_deinitialize
 *
 **************************/
/*!
 * \brief		Restores the system state as it was before the call to #etal_initialize
 * \remark		After successful return the capabilities structure allocated by
 * 				#etal_get_capabilities is no longer available
 * \return		ETAL_RET_NOT_INITIALIZED - ETAL was not initialized yet
 * \return		ETA_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_deinitialize(tVoid)
{
	ETAL_STATUS ret;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_deinitialize()");

	if (!ETAL_statusIsInitialized())
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
#if defined(CONFIG_HOST_OS_FREERTOS)
		ETAL_deinit_Light();
		ret = ETAL_RET_SUCCESS;
#else
		/* this lock is not released explicitly at the end of this function
		 * because it will be deleted by ETAL_initRollback
		 */
		if (ETAL_statusGetLock() != OSAL_OK)
		{
			ret = ETAL_RET_NOT_INITIALIZED;
		}
		else
		{
			ETAL_initRollback(TRUE);
			ETAL_initStatusSetState(state_initNotInitialized);
			if (etalCapabilitiesp != NULL)
			{
				OSAL_vMemoryFree((tPVoid)etalCapabilitiesp);
			}
			ret = ETAL_RET_SUCCESS;
		}
#endif
		ETAL_statusSetInitialized(FALSE);
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_deinitialize() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_reinitialize
 *
 **************************/
/*!
 * \brief		Repeats the ETAL initialization procedure
 * \details		The function uses the same initialization parameters originally
 * 				passed to #etal_initialize. It performs the same actions
 * 				as the #etal_initialize except for OSAL initialization, which
 * 				is not done because the function maintains the ETAL system lock
 * 				for the duration of the operation.
 * 				It gives also the ability to reload landscapes of different standards
 * 				saved in NVM at deinitialization.
 * \remark		This function should be called only in case of communication error
 * 				with one of the hardware devices (i.e. #ETAL_RECEIVER_ALIVE_ERROR
 * 				or #ETAL_ERROR_COMM_FAILED).
 * \param[in]   NVMLoadConfig - NVM load configuration
 *
 * \return		ETAL_RET_NOT_INITIALIZED - ETAL was not initialized yet
 * \return		ETAL_RET_ERROR - Error reinitializing ETAL
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_reinitialize(EtalNVMLoadConfig NVMLoadConfig)
{
	ETAL_STATUS ret;
    EtalHardwareAttr hardware_attr;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_reinitialize(NVMLoaConDAB : %d, NVMLoaConAMFM : %d, NVMLoaConHD : %d)",
    		NVMLoadConfig.m_load_DAB_landscape,
    		NVMLoadConfig.m_load_AMFM_landscape,
    		NVMLoadConfig.m_load_HD_landscape);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ETAL_statusHardwareAttrBackup(&hardware_attr);

		ETAL_initRollback(FALSE);
		ETAL_initStatusSetState(state_initNotInitialized);

		hardware_attr.m_NVMLoadConfig = NVMLoadConfig;

		if (ETAL_init(&hardware_attr, FALSE) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			ret = ETAL_RET_SUCCESS;
		}
		ETAL_statusReleaseLock();
	}
	
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_reinitialize() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_get_capabilities
 *
 **************************/
/*!
 * \brief		Returns the ETAL capabilities
 * \details		ETAL capabilities describe the hardware known to ETAL.
 * 				The function uses dynamic memory allocation to store
 * 				the capabilities. The storage remains allocated and
 * 				valid until etal_deinitialize
 * \remark		
 * \param[out]	pCapabilities - pointer to location where the function stores
 * 				                a pointer to the newly allocated capabilities
 * \return		ETAL_RET_NOT_INITIALIZED - ETAL was not initialized yet
 * \return		ETAL_RET_PARAMETER_ERR - NULL function parameter
 * \return		ETAL_RET_ERROR - memory allocation error (also sends the
 * 				                 ETAL_WARNING_OUT_OF_MEMORY event)
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_get_capabilities(EtalHwCapabilities** pCapabilities)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU16 i, j;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_capabilities()");

	if (!ETAL_statusIsInitialized())
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else if (pCapabilities == NULL)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		etalCapabilitiesp = (EtalHwCapabilities *)OSAL_pvMemoryAllocate(sizeof(EtalHwCapabilities));
		if (etalCapabilitiesp != NULL)
		{
			ETAL_statusFillCapabilities(etalCapabilitiesp);
			*pCapabilities = etalCapabilitiesp;
		}
		else
		{
			*pCapabilities = NULL;
			ret = ETAL_RET_ERROR;
			ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, (tVoid *)((tU32)ETAL_INVALID_HANDLE), sizeof(ETAL_HANDLE));
		}
	}
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_capabilities() = %s", ETAL_STATUS_toString(ret));
	if (pCapabilities == NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "cap: (nil)");
	}
	else if (*pCapabilities == NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "*cap: (nil)");
	}
	else
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "**cap: dcop(devTyp: 0x%x busTyp: %d busAdd: 0x%x ch: %d)", 
			(*pCapabilities)->m_DCOP.m_deviceType, (*pCapabilities)->m_DCOP.m_busType, (*pCapabilities)->m_DCOP.m_busAddress,
			(*pCapabilities)->m_DCOP.m_channels);
		for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "       Tun%d(devTyp: 0x%x busTyp: %d busAdd: 0x%x ch: %d", i,
			(*pCapabilities)->m_Tuner[i].m_TunerDevice.m_deviceType, (*pCapabilities)->m_Tuner[i].m_TunerDevice.m_busType, 
			(*pCapabilities)->m_Tuner[i].m_TunerDevice.m_busAddress, (*pCapabilities)->m_Tuner[i].m_TunerDevice.m_channels);
			for(j = 0; j < ETAL_CAPA_MAX_FRONTEND_PER_TUNER; j++)
			{
				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "            std%d: 0x%x", 
				j, (*pCapabilities)->m_Tuner[i].m_standards[j]);
			}
			for(j = 0; j < ETAL_CAPA_MAX_FRONTEND_PER_TUNER; j++)
			{
				ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "            datTyp%d: 0x%x", 
				j, (*pCapabilities)->m_Tuner[i].m_dataType[j]);
			}
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "           )");
		}
	}
	return ret;
}


ETAL_STATUS etal_get_time(ETAL_HANDLE hReceiver, EtalTime *ETALTime)
{
	ETAL_STATUS ret;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_time(rec: %d, *tim: 0x%x)", hReceiver, ETALTime);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_get_time(hReceiver, ETALTime);

		ETAL_statusReleaseLock();
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_time() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

#if defined (CONFIG_ETAL_HAVE_RECEIVER_ALIVE) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_receiver_alive
 *
 **************************/
/*!
 * \brief		Pings the hardware devices attached to the Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \return		ETAL_RET_INVALID_HANDLE - illegal Receiver handle
 * \return		ETAL_RET_INVALID_RECEIVER - hReceiver is not configured
 * \return		ETAL_RET_ERROR - the device does not respond
 * \return		ETAL_RET_SUCCESS - devices respond
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_receiver_alive(ETAL_HANDLE hReceiver)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_receiver_alive(rec: %d)", hReceiver);

    if (!ETAL_handleIsValid(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (!ETAL_receiverIsValidHandle(hReceiver))
    {
        ret = ETAL_RET_INVALID_RECEIVER;
    }
    else
	{
		ret = ETAL_receiverGetLock(hReceiver);
		if (ret == ETAL_RET_SUCCESS)
		{
			switch (ETAL_receiverGetStandard(hReceiver))
			{
				case ETAL_BCAST_STD_DRM:
				case ETAL_BCAST_STD_DAB:
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
					if (ETAL_cmdPing_MDR() != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
#ifdef CONFIG_ETAL_SUPPORT_CMOST
					/* check also the CMOST */
					else if (ETAL_cmdPing_CMOST(hReceiver) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
#endif
#endif
					break;

				case ETAL_BCAST_STD_HD_FM:
				case ETAL_BCAST_STD_HD_AM:
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					if (ETAL_cmdPing_HDRADIO() != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
#ifdef CONFIG_ETAL_SUPPORT_CMOST
					/* check also the CMOST */
					else if (ETAL_cmdPing_CMOST(hReceiver) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					else
					{
						/* Nothing to do */
					}
#endif
#endif
					break;

				case ETAL_BCAST_STD_FM:
				case ETAL_BCAST_STD_AM:
#ifdef CONFIG_ETAL_SUPPORT_CMOST
					if (ETAL_cmdPing_CMOST(hReceiver) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
#endif
					break;

				default:
					ASSERT_ON_DEBUGGING(0);
					ret = ETAL_RET_NOT_IMPLEMENTED;
					break;
			}
			ETAL_receiverReleaseLock(hReceiver);
		}
	}
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_receiver_alive() = %s", ETAL_STATUS_toString(ret));
    return ret;
}
#endif // CONFIG_ETAL_HAVE_RECEIVER_ALIVE || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_HAVE_GET_VERSION) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_get_version
 *
 **************************/
/*!
 * \brief		Returns the software/firmware versions of ETAL and the attached devices
 * \param[in]	vers - pointer to an externally allocated variable filled by the function
 * \return		ETAL_RET_NOT_INITIALIZED - ETAL was not initialized yet
 * \return		ETAL_RET_PARAMETER_ERR - NULL *vers*
 * \return		ETAL_RET_ERROR - Error accessing one or more of the devices, *vers*
 * 				                 might be incomplete
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_get_version(EtalVersion *vers)
{
	ETAL_STATUS ret;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_version()");

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_version(vers);

		ETAL_statusReleaseLock();
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_version() = %s", ETAL_STATUS_toString(ret));
	return ret;
}
#endif // CONFIG_ETAL_HAVE_GET_VERSION || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * etal_clear_landscape
 *
 **************************/
/*!
 * \brief		Clear landscapes.
 * \param[in]	MemClearConfig - Memory clear configuration
 * \return		ETAL_RET_INVALID_HANDLE - illegal Receiver handle
 * \return		ETAL_RET_INVALID_RECEIVER - hReceiver is not configured
 * \return		ETAL_RET_ERROR - the device does not respond
 * \return		ETAL_RET_SUCCESS - devices respond
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_clear_landscape(EtalMemoryClearConfig MemClearConfig)
{
	ETAL_STATUS ret;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_clear_landscape(memCleDabVm: %d, memCleDabNvm: %d, memCleAmfmVm: %d, memCleAmfmNvm: %d, memCleHdVm: %d, memCleHdNvm: %d)",
    		MemClearConfig.m_clear_DAB_volatile_memory,
    		MemClearConfig.m_clear_DAB_non_volatile_memory,
    		MemClearConfig.m_clear_AMFM_volatile_memory,
    		MemClearConfig.m_clear_AMFM_non_volatile_memory,
    		MemClearConfig.m_clear_HD_volatile_memory,
    		MemClearConfig.m_clear_HD_non_volatile_memory);

    ret = ETAL_clear_landscape(MemClearConfig);

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_clear_landscape() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_save_landscape
 *
 **************************/
/*!
 * \brief		Save landscapes.
 * \param[in]	MemClearConfig - Memory clear configuration
 * \return		ETAL_RET_INVALID_HANDLE - illegal Receiver handle
 * \return		ETAL_RET_INVALID_RECEIVER - hReceiver is not configured
 * \return		ETAL_RET_ERROR - the device does not respond
 * \return		ETAL_RET_SUCCESS - devices respond
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_save_landscape(EtalNVMSaveConfig NVMSaveConfig)
{
	ETAL_STATUS ret;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_save_landscape(nvmSavDab: %d, nvmSavAmfm: %d, nvmSavHd: %d)",
    		NVMSaveConfig.m_save_DAB_landscape,
    		NVMSaveConfig.m_save_AMFM_landscape,
    		NVMSaveConfig.m_save_HD_landscape);

    ret = ETAL_save_landscape(NVMSaveConfig);

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_save_landscape() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_get_init_status
 *
 **************************/
/*!
 * \brief		Returns the details of the ETAL initialization status
 * \remark		Should be called only after #etal_initialize returns with an error
 * \param[in]	status - pointer to externally allocated variable filled by the function
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_PARAMETER_ERR - NULL *status* parameter
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_get_init_status(EtalInitStatus *status)
{
	/*
	 * This function is typically called after an etal_initialize() failure
	 * thus the global lock is not available.
	 */
	if (status == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	ETAL_initStatusGet(status);

	return ETAL_RET_SUCCESS;
}

/***************************
 *
 * etal_backup_context_for_early_audio
 *
 **************************/
/*!
 * \brief		Save the Tuner context for early audio feature
 * \param[in]	CtxEarlyAudio - pointer to strucure containing parameters to save
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_PARAMETER_ERR - NULL *status* parameter
 * \return		ETAL_RET_ERROR - the parameters is not saved due to a FS error
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_backup_context_for_early_audio(const etalCtxBackupEarlyAudioTy *CtxEarlyAudio)
{
	ETAL_STATUS ret;
	if(CtxEarlyAudio == NULL)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		ret = ETAL_backup_context_for_early_audio(CtxEarlyAudio);
	}

	return ret;
}

/***************************
 *
 * ETAL_get_time
 *
 **************************/
/*!
 * \brief		Get DAB time
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	time - pointer to time parameter
 * \return		ETAL_RET_PARAMETER_ERR - NULL time parameter
 * \return		ETAL_RET_ERROR - request only valid on a DAB receiver
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_get_time(ETAL_HANDLE hReceiver, EtalTime *ETALTime)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (ETALTime == NULL)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		switch (ETAL_receiverGetStandard(hReceiver))
		{
			case ETAL_BCAST_STD_DRM:
			case ETAL_BCAST_STD_DAB:
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
				if (ETAL_cmdGetTime_MDR(TRUE, ETALTime) != OSAL_OK)
				{
					ret = ETAL_RET_ERROR;
				}
#endif
				break;
			default:
				memset(ETALTime, 0, sizeof(ETALTime));
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	return ret;
}
