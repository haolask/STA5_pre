//!
//!  \file 		tunerdriver.c
//!  \brief 	<i><b> CMOST driver API layer </b></i>
//!  \details   External interface implementation for CMOST driver.
//!				Details on these functions can be found also in the TUNER_DRIVER_Specification.pdf
//!				document.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "target_config.h"

	#include "osal.h"

#if defined (CONFIG_ETAL_SUPPORT_CMOST)
#include "tunerdriver.h"
#include "tunerdriver_trace.h"
#include "tunerdriver_internal.h"
#include "cmost_helpers.h"
#include "boot_cmost.h"
#include "common_trace.h"
#include "boot_trace.h"

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	#include "ipfcomm.h"
#endif

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && (defined (CONFIG_BOARD_ACCORDO2) || defined (CONFIG_BOARD_ACCORDO5))
	#include "bsp_sta1095evb.h"
#endif
/*!
 * \def		TUNERDRIVER_READ_BUF_LEN
 * 			Size of a temporary buffer used to read CMOST locations
 * 			Not to be changed.
 */
#define TUNERDRIVER_READ_BUF_LEN   	8

#define MAX_CMOST_DEVICE			2
static tyTunerDriverCMOSTDeviceConfiguration TunerDriverCMOSTDeviceConfiguration[MAX_CMOST_DEVICE];
static tChar TunerDriverExternalDriverIPAddress[IP_ADDRESS_CHARLEN] = DEFAULT_IP_ADDRESS;

/***************************
 *
 * TUNERDRIVER_system_init
 *
 **************************/
/*!
 * \brief       TUNER DRIVER system initialization
 * \details     Performs the initializations of OSAL primitives for TUNERDRIVER, DAB DCOP / HD DCOP libraries
 *
 * \return      OSAL_OK
 * \return      OSAL_ERROR
 *
 * \callgraph
 * \callergraph
 */

tSInt TUNERDRIVER_system_init(tVoid)
{
    tSInt ret = OSAL_OK;

    TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_system_init");
    if (OSAL_INIT() != OSAL_OK)
    {
		ret = OSAL_ERROR;
	    TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_system_init : Return = %d, ERROR in OSAL_INIT()", ret);
    }
#ifdef CONFIG_COMM_DRIVER_EXTERNAL
    if (ret == OSAL_OK)
    {
    	/* Initialize Protocol layer indexes */
    	ipforwardPowerUp();

    	if (OSAL_s32SemaphoreCreate(CMOST_HELPER_SEM_NAME, &cmostHelperSendMsg_sem, 1) != OSAL_OK)
    	{
    		ret = OSAL_ERROR;
    		TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_system_init : Return = %d, ERROR in OSAL_s32SemaphoreCreate()", ret);
    	}
    }
#endif //CONFIG_COMM_DRIVER_EXTERNAL
    else
    {
    	TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_system_init : Return = %d, SUCCESS", ret);
    }
    return ret;
}

/***************************
 *
 * TUNERDRIVER_system_deinit
 *
 **************************/
/*!
 * \brief       TUNER DRIVER system deinitialization
 * \details     Performs the deinitializations of OSAL primitives for TUNERDRIVER, DAB DCOP / HD DCOP libraries
 *
 * \return      OSAL_OK
 * \return      OSAL_ERROR
 *
 * \callgraph
 * \callergraph
 */

tSInt TUNERDRIVER_system_deinit(tVoid)
{
    tSInt ret = OSAL_OK;
    TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_system_deinit");

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	if (OSAL_s32SemaphoreClose(cmostHelperSendMsg_sem) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	    TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_system_deinit : Return = %d, ERROR in OSAL_s32SemaphoreClose()", ret);
	}
	else if (OSAL_s32SemaphoreDelete(CMOST_HELPER_SEM_NAME) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_system_deinit : Return = %d, ERROR in OSAL_s32SemaphoreDelete()", ret);
	}
	else
#endif //CONFIG_COMM_DRIVER_EXTERNAL
	{
	    TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_system_deinit : Return = %d, SUCCESS", ret);
		ret = OSAL_DEINIT();
		/* After OSAL_DEINIT() No trace are printed */
	}
    return ret;
}

/***************************
 *
 * TUNERDRIVER_init
 *
 **************************/
/*!
 * \brief		TUNER DRIVER global initialization	
 * \param[in]	deviceID - the device ID of the CMOST
 * \param[in]	deviceConfiguration - pointer to the device configuration of the device ID
 * \details		Performs the initializations and bus configuration
 *
 * 				TUNER_DRIVER uses only OSAL primitives directly mapped to OS primitives
 * 				so there is nothing to initialize. In particular, TUNER DRIVER doesn't use
 * 				any semaphore nor thread.
 * \remark		Provided only for TUNER_DRIVER library deliveries,
 * 				do not invoke from ETAL!
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */

tSInt TUNERDRIVER_init(tU32 deviceID, tyCMOSTDeviceConfiguration *deviceConfiguration)
{
	tSInt ret = OSAL_OK;

	if ((deviceID < MAX_CMOST_DEVICE) && (deviceConfiguration != NULL) && !TUNERDRIVER_IsDeviceActive(deviceID))
	{
		// values are correct
		if(deviceConfiguration->communicationBusType == BusI2C)
		{
			TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_init : DeviceID = %d, Bus name = %s, device address = 0x%x, GPIO_reset = %d, GPIO_IRQ = %d, IRQ callback = 0x%x",
			        deviceID,
					deviceConfiguration->communicationBus.i2c.busName,
					deviceConfiguration->communicationBus.i2c.deviceAddress,
					deviceConfiguration->GPIO_RESET,
					deviceConfiguration->GPIO_IRQ,
					deviceConfiguration->IRQCallbackFunction);
		}
		else
		{
			TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_init : DeviceID = %d, Bus name = %s, SPI mode = %d, SPI speed = %d, GPIO_CS = %d, GPIO_reset = %d, GPIO_IRQ = %d, IRQ callback = 0x%x",
			        deviceID,
					deviceConfiguration->communicationBus.spi.busName,
					deviceConfiguration->communicationBus.spi.mode,
					deviceConfiguration->communicationBus.spi.speed,
					deviceConfiguration->communicationBus.spi.GPIO_CS,
					deviceConfiguration->GPIO_RESET,
					deviceConfiguration->GPIO_IRQ,
					deviceConfiguration->IRQCallbackFunction);
		}

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
		/* Store CMOST device configuration */
		TunerDriverCMOSTDeviceConfiguration[deviceID].active = TRUE;
		TunerDriverCMOSTDeviceConfiguration[deviceID].CMOSTDeviceConfiguration = *deviceConfiguration;

        if (!TcpIpProtocolIsConnected(ProtocolLayerIndexCMOST))
		{
			ProtocolLayerIndexCMOST = PROTOCOL_LAYER_INDEX_INVALID;
            TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_init : Start TCP/IP thread for EXTERNAL driver operation with CMOST with IP : %s",
            		TunerDriverExternalDriverIPAddress);
			ret = ipforwardInit(TunerDriverExternalDriverIPAddress, TCP_IP_CLIENT_PORT_NUMBER_CMOST, LOG_MASK_CMOST, LOG_FILENAME_CMOST, &ProtocolLayerIndexCMOST);
			if(ret != OSAL_OK)
			{
			    TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_init :  Return = %d, ERROR in ipforwardInit()", ret);
			}
	        else
	        {
	            TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_init : Return = %d, SUCCESS", ret);
	        }
		}
        else
        {
            TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_init : TCP/IP thread for EXTERNAL driver operation with CMOST already started");
        }
#else //CONFIG_COMM_DRIVER_EXTERNAL
		/* Store CMOST device configuration */
		TunerDriverCMOSTDeviceConfiguration[deviceID].active = TRUE;
		TunerDriverCMOSTDeviceConfiguration[deviceID].CMOSTDeviceConfiguration = *deviceConfiguration;

		if (BSP_BusConfig_CMOST(deviceID) != OSAL_OK)
		{
            ret =  OSAL_ERROR;
            TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_init : Return = %d, ERROR in bus Configuration", ret);
		}
        else
        {
            TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_init : Return = %d, SUCCESS", ret);
        }
#endif //CONFIG_COMM_DRIVER_EXTERNAL
	}
	else
	{
        ret = OSAL_ERROR;
		TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_init : Return = %d, ERROR invalid deviceID (%d) or deviceConfiguration is NULL (0x%x)",
		        ret, deviceID, deviceConfiguration);
	}

	return ret;
}

	

/***************************
 *
 * TUNERDRIVER_deinit
 *
 **************************/
/*!
 * \brief		TUNER DRIVER global deinitialization
 * \param[in]	deviceID - the device ID of the CMOST
 * \details		Performs the initializations and bus configuration
 *
 * 				TUNER_DRIVER uses only OSAL primitives directly mapped to OS primitives
 * 				so there is nothing to initialize. In particular, TUNER DRIVER doesn't use
 * 				any semaphore nor thread.
 * \remark		Provided only for TUNER_DRIVER library deliveries,
 * 				do not invoke from ETAL!
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */

tSInt TUNERDRIVER_deinit(tU32 deviceID)
{
	tSInt ret = OSAL_OK;

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	tSInt i;
	tBool noMoreDeviceConfigured = TRUE;
#endif // CONFIG_COMM_DRIVER_EXTERNAL

	TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_deinit : DeviceID = %d", deviceID);

	if ((deviceID < MAX_CMOST_DEVICE) && TUNERDRIVER_IsDeviceActive(deviceID))
	{
#ifdef CONFIG_COMM_DRIVER_EXTERNAL
		TunerDriverCMOSTDeviceConfiguration[deviceID].active = FALSE;

        for(i = 0; i < MAX_CMOST_DEVICE; i++)
      	{
        	if(TUNERDRIVER_IsDeviceActive(i) == TRUE)
        	{
        		noMoreDeviceConfigured = FALSE;
        	}
        }

        if(noMoreDeviceConfigured == TRUE)
        {
            TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_deinit : Stop TCP/IP thread for EXTERNAL driver operation with CMOST");

			if (ipforwardDeinit(ProtocolLayerIndexCMOST) != OSAL_OK)
			{
				TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_deinit :  Return = %d, ERROR in ipforwardDeinit()", ret);
			}
			else
			{
				TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_deinit : Return = %d, SUCCESS", ret);
			}
        }
#else //CONFIG_COMM_DRIVER_EXTERNAL
		TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_deinit : DeviceID = %d", deviceID);

		if (deviceID < MAX_CMOST_DEVICE)
		{
			if (BSP_BusUnconfig_CMOST(deviceID) < 0)
			{
				ret = OSAL_ERROR;
				TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_deinit : Return = %d, ERROR in BSP_BusUnconfig_CMOST()", ret);
			}
			else
			{
				TunerDriverCMOSTDeviceConfiguration[deviceID].active = FALSE;
				TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_deinit : Return = %d, SUCCESS", ret);
			}
		}
#endif //CONFIG_COMM_DRIVER_EXTERNAL
	}
	else
	{
        ret = OSAL_ERROR;
		TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_deinit : Return = %d, ERROR invalid deviceID", ret);
	}

	return ret;
}

/***************************
 *
 * TUNERDRIVER_reset_CMOST
 *
 **************************/
/*!
 * \brief		Issue a hardware reset to the CMOST
 * \param[in]	deviceID - the device ID of the CMOST
 * \return		OSAL_ERROR - parameter error or communication error; in both cases the command
 * 				             was not executed
 * \return		OSAL_OK
 * \see			CMOST_helperSendMessage
 * \callgraph
 * \callergraph
 */
tSInt TUNERDRIVER_reset_CMOST(tU32 deviceID)
{
	tU8 resp[1], cmd[1] = {0x00};
	tU32 resp_len;
	CMOST_paramsTy param;
	tSInt ret = OSAL_OK;

	TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_reset_CMOST : DeviceID = %d", deviceID);

	// deviceID must be < MAX_CMOST_DEVICE
	if (deviceID < MAX_CMOST_DEVICE)
	{
		ret = CMOST_helperSendMessage(&param, cmd, 0, CMOST_CMDMODE_RESET, CMOST_ACCESSSIZE_24BITS, deviceID, resp, &resp_len);
		if (ret != OSAL_OK)
		{
	        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_reset_CMOST : Return = %d, ERROR in CMOST_helperSendMessage()", ret);
		}
		else
		{
#if defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
		// For TKernel
		// add a thread wait, to be sure the CMOST / I2C is all well startup
	(void)OSAL_s32ThreadWait(100);
#endif
	TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_reset_CMOST : Return = %d, SUCCESS", ret);
	}
	}
	else
	{
        ret = OSAL_ERROR;
        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_reset_CMOST : Return = %d, ERROR Invalid deviceID = %d", ret, deviceID);
	}
	
	return ret;
}


/***************************
 *
 * TUNERDRIVER_setTcpIpAddress
 *
 **************************/
/*!
 * \brief		Set TCP/IP address for externaldriver
 * \param[in]	IPAddress - IP address
 * \return		OSAL_OK
 * \see			CMOST_helperSendMessage
 * \callgraph
 * \callergraph
 */
tSInt TUNERDRIVER_setTcpIpAddress(tChar *IPAddress)
{
	tSInt ret = OSAL_OK;

	if(IPAddress != NULL)
	{
		strncpy ( TunerDriverExternalDriverIPAddress, IPAddress, IP_ADDRESS_CHARLEN );
	}
	else
	{
		strncpy ( TunerDriverExternalDriverIPAddress, DEFAULT_IP_ADDRESS, IP_ADDRESS_CHARLEN );
		ret = OSAL_ERROR;
	}
	return ret;
}

/***************************
 *
 * TUNERDRIVER_download_CMOST
 *
 **************************/
/*!
 * \brief		Send Firmware or Patches to the CMOST
 * \details		If *image* is NULL the function reads the Firmware or Patches from a file 
 * 				(CONFIG_COMM_CMOST_FIRMWARE_FILE) or from an array stored in read-only
 * 				memory (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED) and sends it to the
 * 				CMOST device identified by deviceID.
 *
 * 				The file name or the array contents it defined at ETAL or TUNER DRIVER
 * 				build time.
 *
 * 				If *image* is non-NULL the function assumes it points to an array containing
 * 				the correct firmware for this CMOST device and uses it for download. The
 * 				format is documented in #BOOT_ReadLineFromArray_CMOST.
 *
 * 				After the download is complete the function reads a reference
 * 				location of the CMOST to check if the device was correctly
 * 				loaded.
 * \param[in]	deviceID - the device ID of the CMOST
 * \param[in]	boot_type - the type of device (STAR-S/STAR-T/DOT-S/DOT-T). For the function
 * 				            to work correctly it is necessary to build ETAL or TUNER DRIVER
 * 				            with support for the requested device(s). Failure to do so will
 * 				            result in OSAL_ERROR
 * \param[in]	image - if NULL it is ignored (and also *image_size*)
 *                      if non-NULL it points to an array containing a CMOST firmware that the
 *                      function uses to initialize the CMOST instead of the .boot file or the
 *                      built-in array. The format of the array is the same as the one
 *                      documented for #BOOT_ReadLineFromArray_CMOST
 * \param[in]	image_size - size in bytes of the *image* array; if *image* is NULL it is ignored
 * \param[in]	load_default_params - if TRUE the driver loads the parameters embedded in ETAL
 * 				                      to the Tuner, after firmware download an start
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication or configuration error, the device is not loaded
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - Device communication error
 * \return		OSAL_ERROR_INVALID_PARAM - Invalid parameter
 * \see			BOOT_Download_CMOST
 * \callgraph
 * \callergraph
 */
tSInt TUNERDRIVER_download_CMOST(tU32 deviceID, BootTunerTy boot_type, tU8 *image, tU32 image_size, tBool load_default_params)
{
	tSInt ret = OSAL_OK;

    TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_download_CMOST : DeviceID = %d, boot_type = %s, image = 0x%x, image_size = %lu, load_default_params = %d", deviceID, BOOT_tunerTypeToString(boot_type), image, image_size, load_default_params);

	if (deviceID < MAX_CMOST_DEVICE)
	{
		ret = BOOT_Download_CMOST(boot_type, deviceID, image, image_size, load_default_params);
		if(ret != OSAL_OK)
		{
	        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_download_CMOST : Return = %d, ERROR in BOOT_Download_CMOST()", ret);
		}
		else
		{
				TUNERDRIVER_tracePrintSystem(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_download_CMOST : Return = %d, SUCCESS", ret);
		}
	}	
	else
	{
        ret = OSAL_ERROR;
        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_download_CMOST : Return = %d, ERROR invalid deviceID", ret);
	}
	
	return ret;
}

/***************************
 *
 * TUNERDRIVER_testRead_CMOST
 *
 **************************/
/*!
 * \brief		Reads two predefined CMOST memory locations
 * \details		This function may be used in the early stages of hardware platform
 * 				bring up to ensure the I2C or SPI communication with the device
 * 				is operational. It reads two memory mapped locations and
 * 				prints the result. The value printed depends on the CMOST
 * 				silicon version, contact STM for details.
 * \remark		The function may be used before the Firmware or Patches are downloaded
 * 				to the CMOST.
 * \param[in]	deviceID - the device ID of the CMOST
 * \return		OSAL_OK - no errors reading the locations; this does not mean the 
 * 				          location content is valid, the function does not check it
 * \return		OSAL_ERROR - communication error with the device
 * \callgraph
 * \callergraph
 */
tSInt TUNERDRIVER_testRead_CMOST(tU32 deviceID)
{
	tSInt ret = OSAL_OK;
	tU8 bufr[TUNERDRIVER_READ_BUF_LEN];
	tU32 mem_address;

    TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_testRead_CMOST : DeviceID = %d", deviceID);

	if (deviceID < MAX_CMOST_DEVICE)
	{
		(void)OSAL_pvMemorySet((tVoid *)bufr, 0x00, TUNERDRIVER_READ_BUF_LEN);
		mem_address = 0x1401E;
		ret = TUNERDRIVER_readRaw32_CMOST(deviceID, mem_address, bufr);

		if (ret != OSAL_OK)
		{
	        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_testRead_CMOST : Return = %d, ERROR in TUNERDRIVER_readRaw32_CMOST()", ret);
		}
		else
		{
			(void)OSAL_pvMemorySet((tVoid *)bufr, 0x00, TUNERDRIVER_READ_BUF_LEN);
			mem_address = 0x1401F;
			ret = TUNERDRIVER_readRaw32_CMOST(deviceID, mem_address, bufr);
			if (ret  != OSAL_OK)
			{
	            TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_testRead_CMOST : Return = %d, ERROR in TUNERDRIVER_readRaw32_CMOST()", ret);
			}
			else
			{
			    TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_testRead_CMOST : Return = %d, SUCCESS", ret);
#if defined(CONFIG_TRACE_CLASS_TUNERDRIVER) && (CONFIG_TRACE_CLASS_TUNERDRIVER >= TR_LEVEL_COMPONENT)
			    TUNERDRIVER_tracePrintComponent(TR_CLASS_BOOT, "read from 0x%x returned: ", mem_address);
                 COMMON_tracePrintBufComponent(TR_CLASS_TUNERDRIVER, bufr, 4, NULL);
#endif
			}
		}
	}
	else
	{
        ret = OSAL_ERROR;
        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_testRead_CMOST : Return = %d, ERROR invalid deviceID", ret);
	}
	return ret;
}

/***************************
 *
 * TUNERDRIVER_readRaw32_CMOST
 *
 **************************/
/*!
 * \brief		Reads a 32-bit value from the CMOST registers or memory
 * \param[in]	deviceID - the device ID of the CMOST
 * \param[in]	address - the memory or register address in the CMOST memory space starting from
 * 				          which the data is to be written. Only the 20 LSB of the parameter
 * 				          are considered valid, other bits are ignored.
 * \param[out]	resp - Pointer to integer where the function stores the value read.
 * 				       For reads to 24-bit locations the value is LSB-aligned.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error with the device
 * \see			CMOST_helperSendMessage
 * \callgraph
 * \callergraph
 */
tSInt TUNERDRIVER_readRaw32_CMOST(tU32 deviceID, tU32 address, tU8 *resp)
{
	tSInt ret = OSAL_OK;
	CMOST_paramsTy param;
	tU32 len;
	tU8 buf[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}; // format is <address*3, sizeinwords*3>

	TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_readRaw32_CMOST : DeviceID = %d", deviceID);

	if (deviceID < MAX_CMOST_DEVICE)
	{
		buf[0] = (tU8)((address >> 16) & 0x0F);
		buf[1] = (tU8)((address >>  8) & 0xFF);
		buf[2] = (tU8)((address >>  0) & 0xFF);

#if defined(CONFIG_TRACE_CLASS_TUNERDRIVER) && (CONFIG_TRACE_CLASS_TUNERDRIVER >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_TUNERDRIVER, buf, 3, NULL);
#endif

		ret = CMOST_helperSendMessage(&param, buf, sizeof(buf), CMOST_CMDMODE_RD, CMOST_ACCESSSIZE_32BITS, deviceID, resp, &len);

		if(ret != OSAL_OK)
		{
            TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_readRaw32_CMOST : Return = %d, ERROR in CMOST_helperSendMessage()", ret);
		}
		else
		{
		    if(len < 4)
		    {
	            TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_readRaw32_CMOST : Return = %d, ERROR response length < 4 (%d)", ret, len);
		    }
		    else
		    {
		         TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_readRaw32_CMOST : Return = %d, SUCCESS Response %d bytes", ret, len);
#if defined(CONFIG_TRACE_CLASS_TUNERDRIVER) && (CONFIG_TRACE_CLASS_TUNERDRIVER >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_TUNERDRIVER, resp, len, NULL);
#endif
		}
	}
	}
	else
	{
		ret = OSAL_ERROR;
        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_readRaw32_CMOST : Return = %d, ERROR invalid deviceID", ret);
	}
	return ret;
}

/***************************
 *
 * TUNERDRIVER_writeRaw32_CMOST
 *
 **************************/
/*!
 * \brief		Writes a 32-bit value to the CMOST register or memory
 * \param[in]	deviceID - the device ID of the CMOST
 * \param[in]	address - the memory or register address in the CMOST memory space starting from
 * 				          which the data is to be written. Only the 20 LSB of the parameter
 * 				          are considered valid, other bits are ignored.
 * \param[in]	value - the 32-bit value to be written; if writing to a 24-bit register
 * 				        the value must be LSB-aligned, that is the MSB will be ignored.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error with the device
 * \see			CMOST_helperSendMessage
 * \callgraph
 * \callergraph
 */
tSInt TUNERDRIVER_writeRaw32_CMOST(tU32 deviceID, tU32 address, tU32 value)
{
	tU8 response[1];
	tU32 resp_len;
	CMOST_paramsTy param;
	tU8 buf[7];
	tSInt ret = OSAL_OK;

	TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_writeRaw32_CMOST : deviceID = %d", deviceID);

	if (deviceID < MAX_CMOST_DEVICE)
	{
		// format is <address*3, payload*4*n>
		buf[0] = (tU8)(address >> 16) & 0xFF;
		buf[1] = (tU8)(address >>  8) & 0xFF;
		buf[2] = (tU8)(address >>  0) & 0xFF;
		buf[3] = (tU8)(value   >> 24) & 0xFF;
		buf[4] = (tU8)(value   >> 16) & 0xFF;
		buf[5] = (tU8)(value   >>  8) & 0xFF;
		buf[6] = (tU8)(value   >>  0) & 0xFF;

#if defined(CONFIG_TRACE_CLASS_TUNERDRIVER) && (CONFIG_TRACE_CLASS_TUNERDRIVER >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_TUNERDRIVER, buf, 7, NULL);
#endif
		ret = CMOST_helperSendMessage(&param, buf, sizeof(buf), CMOST_CMDMODE_W_BOOT, CMOST_ACCESSSIZE_32BITS, deviceID, response, &resp_len);

		if(ret != OSAL_OK)
		{
	        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_writeRaw32_CMOST : Return = %d, ERROR in CMOST_helperSendMessage()", ret);
		}
		else
		{
		    TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_writeRaw32_CMOST : Return = %d, SUCCESS Response %d bytes", ret, resp_len);
#if defined(CONFIG_TRACE_CLASS_TUNERDRIVER) && (CONFIG_TRACE_CLASS_TUNERDRIVER >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_TUNERDRIVER, response, resp_len, NULL);
#endif
	}
	}
	else
	{
		ret = OSAL_ERROR;
        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_writeRaw32_CMOST : Return = %d, ERROR invalid deviceID (%d)", ret, deviceID);
	}
	return ret;
}

/***************************
 *
 * TUNERDRIVER_writeRawBlock_CMOST
 *
 **************************/
/*!
 * \brief		Writes a block of size bytes to the CMOST
 * \details		The data to be written is contained in a buffer pointed to by *buf*. In order to
 * 				optimize the memory transfer, the first #TUNER_DRIVER_HEADER_RESERVED_CMOST bytes
 * 				of the *buf* must be left unused: data to be written must begin at byte 
 * 				#TUNER_DRIVER_HEADER_RESERVED_CMOST from the start of *buf*. The function
 * 				uses the reserved bytes for an internal communication header. Parameter *size*
 * 				must contain the actual data size, not including the bytes reserved for the header.
 * \remark		The function has no way to detect if the caller reserved the requested
 * 				space at the beginning of the buffer: it will simply overwrite whatever
 * 				data is found in those locations.
 * \param[in]	deviceID - the device ID of the CMOST
 * \param[in]	address - the memory or register address in the CMOST memory space starting from
 * 				          which the data is to be written. Only the 20 LSB of the parameter
 * 				          are considered valid, other bits are ignored.
 * \param[in]	buf - address of a buffer of size #TUNER_DRIVER_HEADER_RESERVED_CMOST + *size*,
 * 				      containing the data to be written starting from byte #TUNER_DRIVER_HEADER_RESERVED_CMOST.
 * 				      When the function returns the buffer can be discarded.
 * \param[in]	size - the size in bytes of the data contained in *buf*.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error with the device
 * \see			CMOST_helperSendMessage
 * \callgraph
 * \callergraph
 */
tSInt TUNERDRIVER_writeRawBlock_CMOST(tU32 deviceID, tU32 address, tU8 *buf, tU32 size)
{
	tSInt ret = OSAL_OK;

	tU8 response[1];
	tU32 resp_len;
	CMOST_paramsTy param;

    TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_writeRawBlock_CMOST : DeviceID = %d", deviceID);

	if (deviceID < MAX_CMOST_DEVICE)
	{
		// format is <address*3,payload*4*n>
		buf[0] = (tU8)(address >> 16) & 0x0F;
		buf[1] = (tU8)(address >>  8) & 0xFF;
		buf[2] = (tU8)(address >>  0) & 0xFF;

		/* this function is used to download the FW, so no tracing here! */
		ret = CMOST_helperSendMessage(&param, buf, size + TUNER_DRIVER_HEADER_RESERVED_CMOST, CMOST_CMDMODE_W_BOOT, CMOST_ACCESSSIZE_32BITS, deviceID, response, &resp_len);
		if(ret != OSAL_OK)
		{
            TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_writeRawBlock_CMOST : Return = %d, ERROR in CMOST_helperSendMessage()", ret);
	}
	else
	{
		    TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_writeRawBlock_CMOST : Return = %d, SUCCESS Response %d bytes", ret, resp_len);
	}	
	}
	else
	{
		ret = OSAL_ERROR;
        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_writeRawBlock_CMOST : Return = %d, ERROR invalid deviceID (%d)", ret, deviceID);
	}	

	return ret;
}

/***************************
 *
 * TUNERDRIVER_sendCommand_CMOST
 *
 **************************/
/*!
 * \brief		Sends a command to the CMOST
 * \details		The function waits for the command completion and returns the CMOST response in
 * 				*resp*. Most of the CMOST commands are executed in a fixed amount of time;
 * 				the more complex commands (Tuner_Change_Band, Tuner_Tune, Tuner_AFCheck)
 * 				require polling the Busy bit in the CMOST's SCSR0 register to ensure the
 * 				command is completed. The function copes with both types of commands and when
 * 				it returns it is guaranteed that the command is completed (possibly with an error).
 *
 * 				The function does not cope with collision errors: in case such error is returned
 * 				(as detected by the first byte of the pResp) the caller should retry sending the command.
 * \param[in]	deviceID - the device ID of the CMOST
 * \param[in]	cmd - pointer to a buffer containing the CMOST command in the format defined in
 * 				     CMOST specification. Only 'Command number' and the 'Parameter 1 to N' must
 * 				     be included; the function calculates and appends the checksum to the command.
 * 				     The function uses its own local storage to build the command so the buffer
 * 				     pointed by *cmd* does not need to provide space for the checksum.
 * \param[in]	clen - size in bytes of the command contained in *cmd*. Due to the CMOST command
 * 				       format this number must always be a multiple of 3.
 * \param[out]	resp - pointer to a buffer where the function stores the CMOST response in the
 * 				       format defined in the CMOST specification. The response includes also the
 * 				       3-bytes checksum generated by the CMOST, if the present.
 * 				       The buffer must be large enough to hold the largest CMOST response,
 * 				       as defined in #CMOST_MAX_RESPONSE_LEN.
 * \param[out]	rlen - pointer to an integer where the function writes the number of valid
 * 				       bytes in *resp*
 * \return		OSAL_OK
 * \return		OSAL_ERROR - communication error with the device
 * \see			CMOST_helperSendMessage
 * \callgraph
 * \callergraph
 */
tSInt TUNERDRIVER_sendCommand_CMOST(tU32 deviceID, tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen)
{
	CMOST_paramsTy param;
	tSInt ret;

    TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_sendCommand_CMOST : DeviceID = %d", deviceID);
#if defined(CONFIG_TRACE_CLASS_TUNERDRIVER) && (CONFIG_TRACE_CLASS_TUNERDRIVER >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_TUNERDRIVER, cmd, clen, NULL);
#endif

	if (deviceID < MAX_CMOST_DEVICE)
	{
		ret = CMOST_helperSendMessage(&param, cmd, clen, CMOST_CMDMODE_CMD_GEN, CMOST_ACCESSSIZE_24BITS, deviceID, resp, rlen);
		if(ret != OSAL_OK)
		{
            TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_sendCommand_CMOST : Return = %d, ERROR in CMOST_helperSendMessage()", ret);
		}
		else
		{
            TUNERDRIVER_tracePrintComponent(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_sendCommand_CMOST : Return = %d, SUCCESS Response %d bytes", ret, *rlen);
#if defined(CONFIG_TRACE_CLASS_TUNERDRIVER) && (CONFIG_TRACE_CLASS_TUNERDRIVER >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_TUNERDRIVER, resp, *rlen, NULL);
#endif
	}
	}
	else
	{
		ret = OSAL_ERROR;
        TUNERDRIVER_tracePrintError(TR_CLASS_TUNERDRIVER, "TUNERDRIVER_sendCommand_CMOST : Return = %d, ERROR invalid deviceID (%d)", ret, deviceID);
	}
	return ret;
}

// note : below function we assume the deviceID check as been done successfully
// as they are called internally
// no check on Device ID
//

tBool TUNERDRIVER_IsDeviceActive(tU32 deviceID)
{
	return TunerDriverCMOSTDeviceConfiguration[deviceID].active;
}

tyCommunicationBusType TUNERDRIVER_GetBusType(tU32 deviceID)
{
	return TunerDriverCMOSTDeviceConfiguration[deviceID].CMOSTDeviceConfiguration.communicationBusType;
}

tU32 TUNERDRIVER_GetGPIOReset(tU32 deviceID)
{
	return TunerDriverCMOSTDeviceConfiguration[deviceID].CMOSTDeviceConfiguration.GPIO_RESET;
}

tU32 TUNERDRIVER_GetGPIOIrq(tU32 deviceID)
{
	return TunerDriverCMOSTDeviceConfiguration[deviceID].CMOSTDeviceConfiguration.GPIO_IRQ;
}

tVoid *TUNERDRIVER_GetIRQCallbackFunction(tU32 deviceID)
{
	return TunerDriverCMOSTDeviceConfiguration[deviceID].CMOSTDeviceConfiguration.IRQCallbackFunction;
}

tVoid *TUNERDRIVER_GetCommunicationBusConfig(tU32 deviceID)
{
	tVoid* ret_val = NULL;
	
	if(TunerDriverCMOSTDeviceConfiguration[deviceID].CMOSTDeviceConfiguration.communicationBusType == BusSPI)
	{
		ret_val = (tVoid*)&(TunerDriverCMOSTDeviceConfiguration[deviceID].CMOSTDeviceConfiguration.communicationBus.spi);
	}
	else
	{
		ret_val = (tVoid*)&(TunerDriverCMOSTDeviceConfiguration[deviceID].CMOSTDeviceConfiguration.communicationBus.i2c);
	}

	return ret_val;

}
#endif // CONFIG_ETAL_SUPPORT_CMOST

