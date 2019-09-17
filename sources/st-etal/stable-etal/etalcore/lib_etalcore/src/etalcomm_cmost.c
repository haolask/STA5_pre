//!
//!  \file 		etalcomm_cmost.c
//!  \brief 	<i><b> ETAL communication layer for CMOST devices </b></i>
//!  \details   Functions that interface ETAL to the low level CMOST driver
//!				(a.k.a. TUNER DRIVER).
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"
#include "tunerdriver.h"
#include "tunerdriver_internal.h"
#include "cmost_dump.h"

#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	#include "ipfcomm.h"
#endif

#ifdef CONFIG_ETAL_SUPPORT_CMOST

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*!
 * \def		ETAL_CMOST_COLLISION_WAIT
 * 			If the CMOST driver reports a collision after ETAL sends a command to CMOST,
 * 			ETAL sleeps for this amount of milliseconds before re-sending the command.
 * 			The command transmission is retried for at most #ETAL_TO_CMOST_CMD_TIMEOUT_IN_MSEC ms.
 */
#define ETAL_CMOST_COLLISION_WAIT           10
/*!
 * \def		ETAL_CMOST_COLLISION_ERROR
 * 			Responses to commands may contain a collision indication
 * 			(i.e. the command was not executed because another command
 * 			was being executed). This macro accesses that bit.
 */
#define ETAL_CMOST_COLLISION_ERROR(_buf)    (((tU32)(_buf) & 0x20) == 0x20)
/*!
 * \def		ETAL_CMOST_BUSY_CHA
 * 			The SCSR0 contains indicates if the CMOST device is busy
 * 			completing a command. This macro accesses the bit which
 * 			gives this indication for the CMOST A channel.
 */
#define ETAL_CMOST_BUSY_CHA(_buf)           ((_buf) & 0x0001)
/*!
 * \def		ETAL_CMOST_BUSY_CHB
 * 			The SCSR0 contains indicates if the CMOST device is busy
 * 			completing a command. This macro accesses the bit which
 * 			gives this indication for the CMOST B channel.
 */
#define ETAL_CMOST_BUSY_CHB(_buf)           ((_buf) & 0x1000)

/*****************************************************************
| local types
|----------------------------------------------------------------*/

/*!
 * \enum	EtalBusConfigModeTy
 * 			Specifies the operational mode of the #ETAL_BusConfig_CMOST function
 */
typedef enum
{
	/*! Create and open the file descriptors */
	BusConfigCreate,
	/*! Close and release the file descriptors */
	BusConfigDestroy
} EtalBusConfigModeTy;

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/

/**************************************
 *
 * ETAL_tunerTypeToBootType
 *
 *************************************/
static BootTunerTy ETAL_tunerTypeToBootType(EtalDeviceType tuner_type)
{
	BootTunerTy ret;
	
	switch (tuner_type)
	{
		case deviceSTARS:
			ret = BOOT_TUNER_STAR_S;
			break;

		case deviceSTART:
			ret = BOOT_TUNER_STAR_T;
			break;

		case deviceDOTS:
			ret = BOOT_TUNER_DOT_S;
			break;

		case deviceDOTT:
			ret = BOOT_TUNER_DOT_T;
			break;

		default:
			ret = BOOT_TUNER_UNKNOWN;
			break;
	}
	
	return ret;
}

#if defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETAL_HAVE_ALL_API)
/**************************************
 *
 * ETAL_programXTALalignment
 *
 *************************************/
static tSInt ETAL_programXTALalignment(ETAL_HANDLE hTuner, tU32 value)
{
	tU32 paramValueArray[2];

	paramValueArray[0] = IDX_CMT_systemConfig_tuneDetCompCoeff;
	paramValueArray[1] = value;

	return ETAL_write_parameter_internal(hTuner, fromIndex, paramValueArray, 1);
}
#endif

/**************************************
 *
 * ETAL_busConfig_CMOST
 *
 *************************************/
/*!
 * \brief		Initialize or destroy the Operating System resources needed to access all the CMOST devices
 * \details		The function reads from the etalconfig files the addresses of the
 * 				CMOST devices (with #ETAL_tunerGetAddress) and configure the communication bus
 * 				to do the real work.
 * \param		mode - the operation mode (create or destroy)
 * \return		OSAL_OK  
 * \return		OSAL_ERROR - The function was unable to configure a device, this is a fatal error
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_busConfig_CMOST(EtalBusConfigModeTy mode)
{
	ETAL_HINDEX tuner_index;
	ETAL_HANDLE hTuner;
	tSInt retosal = OSAL_OK;
	tU32 i;
	tyCMOSTDeviceConfiguration deviceConfig;
	EtalDeviceDesc deviceDescription;

	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		tuner_index = (ETAL_HINDEX)i;
		hTuner = ETAL_handleMakeTuner(tuner_index);
		if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
		{
			if (ETAL_tunerGetAddress(hTuner, &deviceDescription) != OSAL_OK)
			{
				ASSERT_ON_DEBUGGING(0);
				retosal = OSAL_ERROR;
			}
			else
			{

#ifndef CONFIG_COMM_DRIVER_EXTERNAL
				ETAL_getDeviceConfig_CMOST(hTuner, &deviceConfig);
#else
				ETAL_getDeviceConfigExternal_CMOST(hTuner, &deviceConfig);
#endif

				if (mode == BusConfigCreate)
				{
					if (TUNERDRIVER_init(tuner_index, &deviceConfig) < 0)
					{
						retosal = OSAL_ERROR;
					}
				}
				else
				{
					if (TUNERDRIVER_deinit(tuner_index) != OSAL_OK)
					{
						retosal = OSAL_ERROR;
					}
				}
			}
		}
		if (retosal != OSAL_OK)
		{
			break;
		}
	}
	return retosal;
}

/**************************************
 *
 * ETAL_load_CMOST
 *
 *************************************/
/*!
 * \brief		Initialize one CMOST device
 * \details		Downloads the firmware or patches and the CMOST parameters, if present.
 *
 * 				The actual firmware/patches/parameter download is done in the
 * 				TUNER_DRIVER.
 *
 * 				The function lets the TUNER DRIVER get the firmware from 
 * 				the appropriate place (file or embedded array) unless the
 * 				etalStatus contains a CMOST image (passed to #etal_initialize);
 * 				in the latter case it passes the image to the TUNER DRIVER
 * 				to use instead of the file or embedded array.
 *
 * 				After downloading the function puts the device in stand-by mode
 * \param[in]	hTuner - the handle of the Tuner to initialize
 * \param[in]	apply_reset - if TRUE the function resets the Tuner before starting
 * 				              the download
 * \return		OSAL_OK
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *hTuner*
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - communication error with the CMOST
 * \see			TUNERDRIVER_download_CMOST
 * \callgraph
 * \callergraph
 */
tSInt ETAL_load_CMOST(ETAL_HANDLE hTuner)
{
	tSInt retosal;
	tSInt ret = OSAL_OK;
	EtalDeviceType tuner_type;
	BootTunerTy boot_type;
	tU8 *firmware;
	tU32 firmware_size;
	tBool load_default_params;
	EtalDeviceDesc deviceDescription;

#if defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETAL_HAVE_ALL_API)
	tU32 xtal_align;
#endif

	if (ETAL_tunerGetAddress(hTuner, &deviceDescription) != OSAL_OK)
	{
		ret = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}

	/* Download the firmware or patches */
	tuner_type = ETAL_tunerGetType(hTuner);
	boot_type = ETAL_tunerTypeToBootType(tuner_type);
	firmware = NULL;
	firmware_size = 0;

	if (ETAL_statusHardwareAttrUseTunerImage(hTuner))
	{
		/* etalStatus contains some CMOST firmware image for this device, use it */
		ETAL_statusHardwareAttrGetTunerImage(hTuner, &firmware, &firmware_size);
	}
	if (ETAL_statusHardwareAttrUseDefaultParams(hTuner))
	{
		load_default_params = TRUE;
	}
	else
	{
		load_default_params = FALSE;
	}

	retosal = TUNERDRIVER_download_CMOST(ETAL_handleTunerGetIndex(hTuner), boot_type, firmware, firmware_size, load_default_params);
	if ((retosal == OSAL_ERROR) || (retosal == OSAL_ERROR_DEVICE_NOT_OPEN))
	{
		ret = OSAL_ERROR_DEVICE_NOT_OPEN;
		goto exit;
	}
	else if (retosal != OSAL_OK)
	{
		ret = retosal;
		goto exit;
	}
	else
	{
		/* Nothing to do */
	}
#if defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETAL_HAVE_ALL_API)
	/*
	 * Apply the XTAL alignment value if required
	 */
	if (ETAL_statusHardwareAttrUseXTALAlignment(hTuner))
	{
		xtal_align = ETAL_statusHardwareAttrGetXTALAlignment(hTuner);
		if (ETAL_programXTALalignment(hTuner, xtal_align) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Unable to set XTAL alignment for tuner %d", hTuner);
		}
	}
#endif

	(LINT_IGNORE_RET) ETAL_cmdSetTunerStandby_CMOST(hTuner);

exit:
	return ret;
}

/***********************************
 *
 * ETAL_initCommunication_CMOST
 *
 **********************************/
/*!
 * \brief		initializes the CMOST communication
 * \details		Initializes the Operating System driver (I2C or SPI) and resets the CMOST devices
 * \return		OSAL_ERROR_DEVICE_INIT - error accessing the Operating System driver interface
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - communication error with the CMOST
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt ETAL_initCommunication_CMOST(tVoid)
{
	tSInt ret = OSAL_OK;
	tU32 i;
	ETAL_HANDLE hTuner;
	ETAL_HINDEX tuner_index;

	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		tuner_index = (ETAL_HINDEX)i;
		hTuner = ETAL_handleMakeTuner(tuner_index);

		// early audio enhancement
		// START
		// Check if tuner is active
		if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
		{
			ret = ETAL_initCommunication_SingleCMOST(tuner_index, TRUE);
			if (ret != OSAL_OK)
			{
				break;
			}
		}
	}
	return ret;
}


/***********************************
 *
 * ETAL_initCommunication_CMOST
 *
 **********************************/
/*!
 * \brief		initializes the CMOST communication
 * \details		Initializes the Operating System driver (I2C or SPI) and resets the CMOST devices for identified CMOST
 * \param[in]	deviceID - the device ID of the CMOST
  * \param[in]	vI_manageReset - indicate if the CMOST Reset should be managed
  *			In case it is set to FALSE, the CMOST reset is not handled. This is usefull in start-up case when CMOST already boot
 * \return		OSAL_ERROR_DEVICE_INIT - error accessing the Operating System driver interface
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - communication error with the CMOST
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt ETAL_initCommunication_SingleCMOST(tU32 deviceID, tBool vI_manageReset)
{
	tSInt retosal;
	tSInt ret = OSAL_OK;
	ETAL_HANDLE hTuner;
	ETAL_HINDEX tuner_index;
	EtalDeviceDesc deviceDescription;
	tyCMOSTDeviceConfiguration deviceConfig;


		tuner_index = (ETAL_HINDEX)deviceID;
		hTuner = ETAL_handleMakeTuner(tuner_index);
		if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
		{
			if (ETAL_tunerGetAddress(hTuner, &deviceDescription) != OSAL_OK)
			{
				ASSERT_ON_DEBUGGING(0);
				ret = OSAL_ERROR_DEVICE_INIT;
				goto exit;
			}
			else
			{
#ifndef CONFIG_COMM_DRIVER_EXTERNAL
				ETAL_getDeviceConfig_CMOST(hTuner, &deviceConfig);
#else
				TUNERDRIVER_setTcpIpAddress(ETAL_getIPAddressForExternalDriver());
				ETAL_getDeviceConfigExternal_CMOST(hTuner, &deviceConfig);
#endif

				if ((retosal = TUNERDRIVER_init(tuner_index, &deviceConfig)) != OSAL_OK)
				{
					ret = retosal;
					goto exit;
				}
			}
		}

		// If Reset management is requested handle it.
		if (true == vI_manageReset)
		{
			// early audio enhancement
			// START
			// Check if tuner is active
			if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
			{
				/*
				 * On the MTD board there are two CMOST connected to the
				 * same reset line, the reset_issued
				 * parameter ensures that the reset is generated only once
				 * TODO insert a configuration item
				 */
#if defined(CONFIG_BOARD_ACCORDO5 ) && defined (CONFIG_COMM_CMOST_SPI) 
				// nothing to do
				// on A5 & SPI the reset should be managed on the M3
				//
#else // defined(CONFIG_BOARD_ACCORDO5 ) && defined (CONFIG_COMM_CMOST_SPI)

#ifdef CONFIG_MODULE_INTEGRATED
#ifndef CONFIG_COMM_CMOST_HAVE_DEDICATED_RESET_LINE
				// the reset line is shared, reset only if 1st tuner
				if (hTuner == ETAL_tunerGetFirstTuner_CMOST())
#endif
#endif
				{
					if (TUNERDRIVER_reset_CMOST(tuner_index) != OSAL_OK)
					{
						ret = OSAL_ERROR_DEVICE_NOT_OPEN;
						goto exit;
					}
				}
#endif // defined(CONFIG_BOARD_ACCORDO5 ) && defined (CONFIG_COMM_CMOST_SPI) 
			}
		}

exit:
	return ret;
}


/***********************************
 *
 * ETAL_deinitCommunication_CMOST
 *
 **********************************/
/*!
 * \brief		De-initializes the CMOST communication
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error accessing the OS resource
 * \callgraph
 * \callergraph
 */
tSInt ETAL_deinitCommunication_CMOST(tVoid)
{
	tSInt retosal = OSAL_OK;

	if (ETAL_busConfig_CMOST(BusConfigDestroy) != OSAL_OK)
	{
		retosal = OSAL_ERROR;
	}
	return retosal;
}

/***********************************
 *
 * ETAL_readStatusRegister_CMOST
 *
 **********************************/
/*!
 * \brief		Reads the SCSR0, SCSR1 and SCSR2 registers of the CMOST
 * \details		SCSR0 is at address 0x20100. SCSR1 is used to read the
 * 				channel A frequency, SCSR2 for the channel B frequency.
 *
 * 				If any of *scsr*, *freq0*, *freq1* is NULL the corresponding
 * 				register is not read.
 * \remark		The SCSR0 may be accessed though the CMOST command interpreter
 * 				(using the 0x1E commands); this function uses direct access instead
 * 				because it is faster.
 * \param[in]	hTuner - the Tuner handle
 * \param[out]	scsr   - pointer to an integer where the function stores the SCSR0, or NULL
 * \param[out]	freq0  - pointer to an integer where the function stores the current frequency for channel A, or NULL
 * \param[out]	freq1  - pointer to an integer where the function stores the current frequency for channel B, or NULL
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *hTuner*
 * \callgraph
 * \callergraph
 */
tSInt ETAL_readStatusRegister_CMOST(ETAL_HANDLE hTuner, tU32 *scsr, tU32 *freq0, tU32 *freq1)
{
	tSInt retval = OSAL_OK;
	tU8 bufr[4];
	EtalDeviceDesc deviceDescription;

	if (ETAL_tunerGetLock(hTuner) != ETAL_RET_SUCCESS)
	{
		retval = OSAL_ERROR;
		goto exit;
	}
	if (ETAL_tunerGetAddress(hTuner, &deviceDescription) != OSAL_OK)
	{
		retval = OSAL_ERROR_INVALID_PARAM;
	}

	if ((retval == OSAL_OK) && (scsr != NULL))
	{
		if (TUNERDRIVER_readRaw32_CMOST(ETAL_handleTunerGetIndex(hTuner), 0x020100, bufr) != OSAL_OK)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			*scsr  = ((tU32)bufr[0] << 16) | ((tU32)bufr[1] << 8) | bufr[2];
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "address=0x%.2x useI2C=%d SCSR0=0x%.6x",
					deviceDescription.m_busAddress, (deviceDescription.m_busType == ETAL_BusI2C), *scsr);
		}
	}
	if ((retval == OSAL_OK) && (freq0 != NULL))
	{
		if (TUNERDRIVER_readRaw32_CMOST(ETAL_handleTunerGetIndex(hTuner), 0x020101, bufr) != OSAL_OK)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			*freq0 = ((tU32)bufr[0] << 16) | ((tU32)bufr[1] << 8) | bufr[2];
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "address=0x%.2x useI2C=%d freq0=%d",
					deviceDescription.m_busAddress, (deviceDescription.m_busType == ETAL_BusI2C), *freq0);
		}
	}
	if ((retval == OSAL_OK) && (freq1 != NULL))
	{
		if (TUNERDRIVER_readRaw32_CMOST(ETAL_handleTunerGetIndex(hTuner), 0x020102, bufr) != OSAL_OK)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			*freq1 = ((tU32)bufr[0] << 16) | ((tU32)bufr[1] << 8) | bufr[2];
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "address=0x%.2x useI2C=%d freq1=%d",
					deviceDescription.m_busAddress, (deviceDescription.m_busType == ETAL_BusI2C), *freq1);
		}
	}
	ETAL_tunerReleaseLock(hTuner);

exit:
	return retval;
}

/***********************************
 *
 * ETAL_waitBusyBit_CMOST
 *
 **********************************/
/*!
 * \brief		Reads the SCSR0 register until both channels' Busy bits are 0
 * \details		The function polls the SCSR0 registers for at most
 * 				*max_delay_msec* milliseconds; between polls it goes to sleep for
 * 				#ETAL_CMOST_BUSYBIT_WAIT_SCHEDULING milliseconds. Thus the
 * 				total delay may be larger than the requested *max_delay_msec*.
 * \param[in]	hTuner - the Tuner handle
 * \param[in]	tuner_mask - the channels to consider: if 0x01 consider only channel A,
 * 				             if 0x02 consider only channel B, if 0x03 consider both channels.
 * 				             In the latter case the function does not return until both
 * 				             channel's busy bits are clear.
 * \param[in]	max_delay_msec - the amount of time to wait, in msec, for the busy bits
 * 				                 to clear; this is a lower limit on the total delay,
 * 				                 the actual delay may be larger than this.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_waitBusyBit_CMOST(ETAL_HANDLE hTuner, tU32 tuner_mask, tU32 max_delay_msec)
{

	tSInt retval = OSAL_ERROR;

	OSAL_tMSecond end_time;
	tU32 scsr;

 	end_time = OSAL_ClockGetElapsedTime() + max_delay_msec;

	while (OSAL_ClockGetElapsedTime() < end_time)
	{
		if (ETAL_readStatusRegister_CMOST(hTuner, &scsr, NULL, NULL) != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_waitBusyBit_CMOST : error reading scrs, tuner %d", hTuner);
			break;
		}

		// Check CHA busy bit if tuner_mask 1 (0x01) requested
		// if tuner_mask is not set to 0x01, then no need to check CHA
		// the condition is handled by : 
		// TRUE if :
		// * tuner_mask not set to 0x01 (which means CHA check is not needed)
		// * OR CHA is free (which means tuner_mask is set to 0x01, CHA is checked)
		// result in True either if no check on channel 1, or channel 1 free
		//
		// Check CHB busy bit if tuner_mask 2 (0x02) requested
		// if tuner_mask is not set to 0x02, then no need to check CHB
		// 
		
		if ((((tuner_mask & 0x01) == 0x00) || (ETAL_CMOST_BUSY_CHA(scsr) == 0)) && 
			(((tuner_mask & 0x02) == 0x00) || (ETAL_CMOST_BUSY_CHB(scsr) == 0)))
		{
			retval = OSAL_OK;

			break;
		}
		(void)OSAL_s32ThreadWait(ETAL_CMOST_BUSYBIT_WAIT_SCHEDULING);
	}

	if (retval != OSAL_OK)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Timeout or read error waiting for CMOST busy bit clear");
	}

	return retval;
}

#endif // CONFIG_ETAL_SUPPORT_CMOST

/***************************
 *
 * ETAL_sendCommandTo_CMOST
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
 * \return		OSAL_ERROR - communication with low level driver failed; in this case the 
 * 				             output parameters are unchanged
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *hTuner*
 * \return		OSAL_ERROR_TIMEOUT_EXPIRED - timeout waiting for command to be sent due to collision
 * \return		OSAL_ERROR_DEVICE_INIT - device initialization error (semaphore not available)
 * \return		OSAL_ERROR_FROM_SLAVE  - the CMOST returned an error, the error code is in *cstat*
 * \callgraph
 * \callergraph
 */
tSInt ETAL_sendCommandTo_CMOST(ETAL_HANDLE hGeneric, tU8 *cmd, tU32 clen, tU16 *cstat, tU8 *resp, tU32 *rlen)
{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	tSInt retval = OSAL_OK;
	OSAL_tMSecond start_time, end_time;
	tU32 len = 0;
	tU8 resp_buf[CMOST_MAX_RESPONSE_LEN]; /* avoid static to protect against task switch corruption */
	ETAL_HANDLE hTuner;
	EtalDeviceDesc deviceDescription;


	// init the CSTAT for further use 
	// to an allowed value (TU8) but in theory not possible sent by CMOST
	// bit 0-> 4 = reserved : put to 1 ie 1F
	// bit 5 / 6  / 7 = error code : simulate no error
	if (cstat != NULL)
	{
		*cstat = (tU16)0x00;
	}
			
	/* we need a Tuner handle; if we've been passed a Receiver handle
	 * we need to extract the Tuner handle from there
	 */
	if (ETAL_handleIsReceiver(hGeneric))
	{
		if (ETAL_receiverGetTunerId(hGeneric, &hTuner) != OSAL_OK)
		{
			retval = OSAL_ERROR_INVALID_PARAM;
			goto exit;
		}
	}
	else
	{
		hTuner = hGeneric;
	}

	/* this also performs validity check on hTuner */
	if (ETAL_tunerGetAddress(hTuner, &deviceDescription) != OSAL_OK)
	{
		retval = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}

	if (ETAL_tunerGetLock(hTuner) != ETAL_RET_SUCCESS)
	{
		retval = OSAL_ERROR_DEVICE_INIT;
		goto exit;
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
		
		if (ETAL_CMOST_COLLISION_ERROR(resp_buf[0]))
		{
			retval = OSAL_ERROR_TIMEOUT_EXPIRED;
			(void)OSAL_s32ThreadWait(ETAL_CMOST_COLLISION_WAIT);
			continue;
		}
		else if (resp_buf[0] != (tU8)0)
		{
			retval = OSAL_ERROR_FROM_SLAVE;
		}
		else
		{
			/* Nothing to do */
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
		else if (len > CMOST_HEADER_LEN)
		{
			if (resp != NULL)
			{
				(void)OSAL_pvMemoryCopy((tVoid *)resp, (tPCVoid)(resp_buf + CMOST_HEADER_LEN), len - CMOST_HEADER_LEN);
			}
			if (rlen != NULL)
			{
				*rlen = len - CMOST_HEADER_LEN;
			}
		}
		else
		{
			/* Nothing to do */
		}
	}

exit:
	return retval;
#else
	return OSAL_OK;
#endif
}


