//!
//!  \file 		HDRADIO_Protocol.c
//!  \brief 	<i><b> HDRADIO driver </b></i>
//!  \details   Low level driver for the HDRADIO device
//!  \author 	Roberto Allevi
//!
/*
 * This file contains:
 * - the raw driver to access the HDRADIO over ..... bus from a Linux environment,
 *   using /sys/class/gpio for reset management
 *
 * The functions are customized for usage on the Accordo2 RevB board.
 * Generalization/configurability is TODO.
 *
 * WARNING: The implementation uses the standard C method ("errno")
 * to read the error return from C library functions but currently (09 May 2014)
 * it seems that the uclib for A2 does not set it properly: the only way to read the
 * system call return value is through the 'strace -f' command.
 */

#include "target_config.h"

#if (defined CONFIG_COMM_DRIVER_EMBEDDED) && (defined CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

#if defined (CONFIG_APP_TUNERDRIVER_LIBRARY)
	#include "osal_replace.h"
#else
	#include "osal.h"
#endif

#if defined (CONFIG_HOST_OS_LINUX)
	#include <sys/ioctl.h>
#endif
#include <unistd.h>

#include "bsp_sta1095evb.h"
#include "bsp_trace.h"
#include "HDRADIO_Protocol.h"
#include "hdradio_internal.h"
#include "common_trace.h"
#include "hdradio_trace.h"

/**************************************
 * Defines and macros
 *************************************/
#define LM_FORMAT_FIELDS          12
#define LM_BYTE_HEADER            ((tU8)0xA5)
#define LM_STATUS_SUCCESS         ((tU8)0x01)

#undef HDRADIO_DEBUG_DUMP_TO_FILE
#ifdef HDRADIO_DEBUG_DUMP_TO_FILE
	#define HDRADIO_DEBUG_DUMP_FILENAME "hdradio_data.bin"
#endif

#define HDRADIO_DCOP_RESETING_TIME              2

#define MAX_HDRADIO_DCOP_DEVICE					1

/* SPI frequency */
#define SPI_SPEED_MIN			1076
#define SPI_SPEED_MAX			7056000

/**************************************
 * Local types
 *************************************/

/**************************************
 * Local variables
 *************************************/
#ifdef HDRADIO_DEBUG_DUMP_TO_FILE
	static FILE *HDRADIO_DebugDump_fd;
#endif
static tU8 LM_buf[LM_FULL_SIZE];
// LMcount should be incremented on write.
// start at 255
static tU8 LMCount = (tU8)255;	/* The value of the first LM count is 0x00 and it is incremented each time the HC sends a logical message */
static tyHDRADIODeviceConfiguration HDRADIODeviceConfiguration[MAX_HDRADIO_DCOP_DEVICE];
static tChar HDRADIOExternalDriverIPAddress[IP_ADDRESS_CHARLEN] = DEFAULT_IP_ADDRESS;

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
static tSInt HDRADIO_sendResetWithExternalDriver(tVoid);
#endif //

/**************************************
 * Function prototypes
 *************************************/
static tS32 HDRADIO_LinuxWrite(tU8 *buf, tU32 len);
static tS32 HDRADIO_Write_LM_Packet(tU8 *buf, tU32 len);
static tS32 HDRADIO_LinuxRead(tU8 **pBuf, tS32 *pLen);
static tS32 HDRADIO_Read_LM_Packet(tU8 **pBuf, tS32 *pLen);

/**************************************
 * Function definitions
 *************************************/

/**************************************
 *
 * HDRADIO_LinuxWrite
 *
 *************************************/
/*
 * Writes a buffer to the HDRADIO in Linux environment
 *
 * Must be called after BSP_BusConfig_HDRADIO
 *
 * Parameters:
 *  buf - pointer to the buffer
 *  len - size of the buffer in bytes
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - write failure or system not configured
 *
 */
static tS32 HDRADIO_LinuxWrite(tU8 *buf, tU32 len)
{
	tS32 ret_bsp, ret = OSAL_OK;

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	ret_bsp = BSP_Write_HDRADIO(HDRADIO_DCOP_ID_0, buf, len);
	if (ret_bsp < 0)
	{
		ret = OSAL_ERROR;
	}
	else
	{
		ret = OSAL_OK;
	}

#if (defined HDRADIO_DEBUG_DUMP_TO_FILE)
	HDRADIO_DebugDump_fd = fopen(HDRADIO_DEBUG_DUMP_FILENAME, "a");

	if ((HDRADIO_DebugDump_fd != NULL) && (fwrite(buf, 1, len, HDRADIO_DebugDump_fd) < 0))
	{
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "writing dump to file: %s", strerror(errno));
	}

	fclose(HDRADIO_DebugDump_fd);
	HDRADIO_DebugDump_fd = NULL;
#endif // #if (defined HDRADIO_DEBUG_DUMP_TO_FILE)

#endif // CONFIG_HOST_OS_LINUX
	return ret;
}

/**************************************
 *
 * HDRADIO_LinuxRead
 *
 *************************************/
/*
 * Reads a buffer from the HDRADIO in Linux environment
 *
 * Must be called after BSP_BusConfig_HDRADIO
 *
 * Parameters:
 *  pBuf - pointer of pointer to the buffer
 *  pLen - pointer to the buffer size in bytes
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - read failure or system not configured
 *
 */
static tS32 HDRADIO_LinuxRead(tU8 **pBuf, tS32 *pLen)
{
	tS32 ret = OSAL_OK;
	
#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	*pLen = BSP_Read_HDRADIO(HDRADIO_DCOP_ID_0, pBuf);
	if (*pLen < 0)
	{
		ret = OSAL_ERROR;
		goto exit;
	}

#if (defined HDRADIO_DEBUG_DUMP_TO_FILE)
	HDRADIO_DebugDump_fd = fopen(HDRADIO_DEBUG_DUMP_FILENAME, "a");

	if ((HDRADIO_DebugDump_fd != NULL) && (fwrite(*pBuf, 1, *pLen, HDRADIO_DebugDump_fd) < 0))
	{
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "writing dump to file: %s", strerror(errno));
	}

	fclose(HDRADIO_DebugDump_fd);
	HDRADIO_DebugDump_fd = NULL;
#endif // #if (defined HDRADIO_DEBUG_DUMP_TO_FILE)

#endif // CONFIG_HOST_OS_LINUX

exit:
	return ret;
}

/**************************************
 *
 * HDRADIO_Write_LM_Packet
 *
 *************************************/
/*
 * Writes a LM Packet to the HDRADIO
 *
 * Must be called after BSP_BusConfig_HDRADIO
 *
 * Parameters:
 *  buf - pointer to the buffer
 *  len - size of the buffer in bytes
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - write failure or system not configured
 *
 */
static tS32 HDRADIO_Write_LM_Packet(tU8 *buf, tU32 len)
{
	tU32 i;

	// LM final formatting: Header and Checksum
	buf[0] = buf[1] = LM_BYTE_HEADER;

	// Checksum computing in buf[<LAST>]
	for(i = 0; i < len-1; i++)
	{
		buf[len-1] = (tU8)(buf[len-1] + buf[i]);
	}

#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_COMPONENT)
	//ETAL_tracePrintBuffer(TR_LEVEL_COMPONENT, TR_CLASS_HDRADIO, "wrt", buf, len);
	COMMON_tracePrintBufComponent(TR_CLASS_HDRADIO, buf, len,  "wrt");
#endif

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	return HDRADIO_LinuxWrite(buf, len);
#else
	return OSAL_OK;
#endif
}

/**************************************
 *
 * HDRADIO_Read_LM_Packet
 *
 *************************************/
/*
 * Reads a LM Packet from the HDRADIO
 *
 * Must be called after BSP_BusConfig_HDRADIO
 *
 * Parameters:
 *  pBuf - pointer of pointer to the buffer
 *  pLen - pointer to the buffer size in bytes
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - read failure or system not configured
 *
 */
static tS32 HDRADIO_Read_LM_Packet(tU8 **pBuf, tS32 *pLen)
{
	tU8 chsumField, chsumComputed = (tU8)0;
	tU32 len = 0;
	tU32 i;
	tS32 ret;

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	if (HDRADIO_LinuxRead (&(*pBuf), pLen) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		goto exit;
	}
#endif

#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_COMPONENT)
	COMMON_tracePrintBufComponent(TR_CLASS_HDRADIO, *pBuf, *pLen, "rdd");
#endif

	if (*pLen >= 2)
	{
		// Header checking
		if ((*pBuf)[0] != LM_BYTE_HEADER || (*pBuf)[1] != LM_BYTE_HEADER)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
	}

	if (*pLen >= 6)
	{
		// LM length computing
		len = (tU32)((*pBuf)[4] + ((tU32)((*pBuf)[5]) << 8));
	}

	if (*pLen != (tS32)len)
	{
		ret = OSAL_ERROR;
		goto exit;
	}

	// Checksum checking
	// The transmitted Checksum is in (*pBuf)[<LAST>]
	chsumField = (*pBuf)[len-1];

	// Checksum computing till the (*pBuf)[<LAST-1>]
	for (i = 0; i < len-1; i++)
	{
		chsumComputed = (tU8)(chsumComputed + (*pBuf)[i]);
	}

	if (chsumComputed != chsumField)
	{
		ret = OSAL_ERROR;
		goto exit;
	}

	ret = OSAL_OK;

exit:
	return ret;
}

#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
/***************************
 *
 * HDRADIO_sendResetWithExternalDriver
 *
 **************************/
/*!
 * \brief		Issue a reset to the HDRADIO DCOP using the external driver
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 * \todo		Update when the Protocol Layer will provide the reset support
 */
static tSInt HDRADIO_sendResetWithExternalDriver(tVoid)
{
	tU8 h0, h1, h2, h3;
	tU32 clen = 0;
	tU8 *cmd = NULL;
	tU32 address;
    EtalDeviceDesc deviceDescription;

    if(HDRADIODeviceConfiguration[HDRADIO_DCOP_ID_0].communicationBusType == BusI2C)
    {
    	address = HDRADIODeviceConfiguration[HDRADIO_DCOP_ID_0].communicationBus.i2c.deviceAddress;

    	/* No semaphore taken because the current HDRadio code always calls ETAL_sendCommandTo_HDRADIO
    	 * with HDRadio device lock, or uses the ETAL_cmd*HDRADIO which take the device lock
    	 */

    	h0 = HDRADIO_EXTERNAL_DRIVER_TYPE_RESET;
    	h1 = (tU8)INSTANCE_1;
    	h2 = (tU8)address;
    	h3 = HDRADIO_EXTERNAL_DRIVER_RESERVED;
    	ForwardPacketToCtrlAppPort(ProtocolLayerIndexDCOP, cmd, clen, (tU8)0x1D, BROADCAST_LUN, h0, h1, h2, h3);

    	OSAL_s32ThreadWait(HDRADIO_FW_LOADING_TIME);

    	/* reset has a response, get it to empty the FIFO */
    	(LINT_IGNORE_RET)ProtocolLayer_FifoPop(ProtocolLayerIndexDCOP, NULL, PROTOCOL_LAYER_INTERNAL_BUFFER_LEN, NULL, NULL);
    	/* don't parse the response, other higher level checks will be done later */

    	ret = OSAL_OK;
    }
    else
    {
    	ret = OSAL_ERROR;
    }

	return ret;
}
#endif // CONFIG_COMM_DRIVER_EXTERNAL


/**************************************
 *
 * HDRADIO_init
 *
 *************************************/
/*
 * HDRADIO global initialization
 *
 * Parameters:
 * deviceConfiguration : indicate the device configuration
 *
 * The function is called to initialize HDRADIO and configure
 * the communication with the DCOP device.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - failure during reconfiguration
 *  OSAL_ERROR_INVALID_PARAM - invalid parameter
 *
 *
 */
tS32 HDRADIO_init(tyHDRADIODeviceConfiguration *deviceConfiguration)
{
	tS32 ret;

	HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_init");
	if(deviceConfiguration != NULL)
	{
	
		/* Store HDRADIO device configuration */
		HDRADIODeviceConfiguration[HDRADIO_DCOP_ID_0] = *deviceConfiguration;
		
        if(deviceConfiguration->communicationBusType == BusI2C)
        {
            HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_init : Bus name = %s, device address = 0x%x, GPIO_reset = %d",
                    deviceConfiguration->communicationBus.i2c.busName,
                    deviceConfiguration->communicationBus.i2c.deviceAddress,
                    deviceConfiguration->GPIO_RESET);
        }
        else
        {
            HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_init : Bus name = %s, SPI mode = %d, SPI speed = %d, GPIO_CS = %d, GPIO_reset = %d",
                    deviceConfiguration->communicationBus.spi.busName,
                    deviceConfiguration->communicationBus.spi.mode,
                    deviceConfiguration->communicationBus.spi.speed,
                    deviceConfiguration->communicationBus.spi.GPIO_CS,
                    deviceConfiguration->GPIO_RESET);
        }

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
        STECI_tracePrintSystem(TR_CLASS_STECI, "HDRADIO_init : Start TCP/IP thread for EXTERNAL driver operation with DCOP with IP : %s",
        		HDRADIOExternalDriverIPAddress);
        if (ipforwardInit(HDRADIOExternalDriverIPAddress, TCP_IP_CLIENT_PORT_NUMBER_DCOP, LOG_MASK_DCOP, LOG_FILENAME_DCOP, &ProtocolLayerIndexDCOP) != OSAL_OK)
		{
			ret = OSAL_ERROR;
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_init : Return = %d, ERROR in ipforwardInit()", ret);
		}
        else
        {
    		/* Send a RESET command to the DCOP */
    		HDRADIO_sendResetWithExternalDriver();
            HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_init : Return = %d, SUCCESS", ret);
        }
#else
        ret = BSP_DeviceInit_HDRADIO(HDRADIO_DCOP_ID_0);
        if(ret != OSAL_OK)
        {
        	HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_init : Return = %d, ERROR in BSP_DeviceInit_HDRADIO()", ret);
        }
#endif //CONFIG_COMM_DRIVER_EXTERNAL
    }
    else
    {
        ret = OSAL_ERROR_INVALID_PARAM;
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_init : Return = %d, ERROR pointer is NULL (deviceConfiguration = 0x%x)", ret);
    }

    if(ret == OSAL_OK)
    {
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_init : Return = %d, SUCCESS", ret);
    }
	return ret;
}

/**************************************
 *
 * HDRADIO_deinit
 *
 *************************************/
/*
 * HDRADIO deinitialization
 *
 * Parameters:
 *  none
 *
 * The function is called to close the resources configured to access the
 * HDRADIO DCOP device.
 *
 * Returns:
 *  OSAL_OK     - success
 *  OSAL_ERROR  - failure closing configured resources
 *
 */
tS32 HDRADIO_deinit(tVoid)
{
    tS32 ret = OSAL_OK;

    HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_deinit");

#ifdef CONFIG_COMM_DRIVER_EMBEDDED
	ret = BSP_DeviceDeinit_HDRADIO(HDRADIO_DCOP_ID_0);
    if(ret != OSAL_OK)
    {
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_deinit : Return = %d, ERROR in BSP_DeviceDeinit_HDRADIO()", ret);
    }
    else
    {
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_deinit : Return = %d, SUCCESS", ret);
    }
#else
	STECI_tracePrintSystem(TR_CLASS_STECI, "HDRADIO_deinit : Stop TCP/IP thread for EXTERNAL driver operation with DCOP");
	if (ipforwardDeinit(ProtocolLayerIndexDCOP) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_STECI, "HDRADIO_deinit : Return = %d, ERROR in ipforwardDeinit()", ret);
	}
	else
	{
		HDRADIO_tracePrintSystem(TR_CLASS_STECI, "HDRADIO_deinit : Return = %d, SUCCESS", ret);
	}
#endif //CONFIG_COMM_DRIVER_EMBEDDED
    return ret;
}

/**************************************
 *
 * HDRADIO_reset
 *
 *************************************/
/*
 * Issue a hardware reset the HDRADIO DCOP
 *
 * Parameters:
 *  none
 *
 * The function is called to initialize HDRADIO DCOP device.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - failure in opening GPIO
 *
 */
tS32 HDRADIO_reset(tVoid)
{
    tS32 ret = OSAL_OK;

    HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_reset");

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	if (TcpIpProtocolIsConnected(ProtocolLayerIndexDCOP))
	{
		/* Send a RESET command to the DCOP */
		HDRADIO_sendResetWithExternalDriver();
	    STECI_tracePrintError(TR_CLASS_STECI, "HDRADIO_reset : Return = %d, SUCCESS", ret);
	}
	else
	{
        ret = OSAL_ERROR;
        STECI_tracePrintError(TR_CLASS_STECI, "HDRADIO_reset : Return = %d, ERROR in TcpIpProtocolIsConnected()", ret);
	}
#else
	ret = BSP_DeviceReset_HDRADIO(HDRADIO_DCOP_ID_0);
    if (ret != OSAL_OK)
    {
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_reset : Return = %d, ERROR in BSP_DeviceReset_HDRADIO()", ret);
    }
    else
    {
        /*
         * Time to leave to the DCOP for recovering after reseting BSP
         */
        (void)OSAL_s32ThreadWait(HDRADIO_DCOP_RESETING_TIME);
    }
#endif //CONFIG_COMM_DRIVER_EMBEDDED

    if(ret == OSAL_OK)
    {
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_reset : Return = %d, SUCCESS", ret);
    }
    return ret;
}

/***************************
 *
 * HDRADIO_setTcpIpAddress
 *
 **************************/
/*!
 * \brief		Set TCP/IP address for external driver
 * \param[in]	IPAddress - IP address
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt HDRADIO_setTcpIpAddress(tChar *IPAddress)
{
	tSInt ret = OSAL_OK;

	if(IPAddress != NULL)
	{
		strncpy ( HDRADIOExternalDriverIPAddress, IPAddress, IP_ADDRESS_CHARLEN );
	}
	else
	{
		strncpy ( HDRADIOExternalDriverIPAddress, DEFAULT_IP_ADDRESS, IP_ADDRESS_CHARLEN );
		ret = OSAL_ERROR;
	}
	return ret;
}

/**************************************
 *
 * HDRADIO_reconfiguration
 *
 *************************************/
/*
 * HDRADIO device reconfiguration
 *
 * Parameters:
 * deviceConfiguration : pointer to device configuration.
 *
 * The function is called to reconfigure HDRADIO device.
 * (limited to mode or/and speed of the SPI bus).
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - failure during reconfiguration
 *  OSAL_ERROR_INVALID_PARAM - invalid parameter
 *
 *
 */
tS32 HDRADIO_reconfiguration(tyHDRADIODeviceConfiguration *deviceConfiguration)
{
	tyHDRADIODeviceConfiguration vl_currentConfig;
	tS32 ret = OSAL_OK;

    HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_reconfiguration");
    if(deviceConfiguration != NULL)
    {
        if(deviceConfiguration->communicationBusType == BusI2C)
        {
            HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_reconfiguration : Bus name = %s, device address = 0x%x, GPIO_reset = %d",
                    deviceConfiguration->communicationBus.i2c.busName,
                    deviceConfiguration->communicationBus.i2c.deviceAddress,
                    deviceConfiguration->GPIO_RESET);
        }
        else
        {
            HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_reconfiguration : Bus name = %s, SPI mode = %d, SPI speed = %d, GPIO_CS = %d, GPIO_reset = %d",
                    deviceConfiguration->communicationBus.spi.busName,
                    deviceConfiguration->communicationBus.spi.mode,
                    deviceConfiguration->communicationBus.spi.speed,
                    deviceConfiguration->communicationBus.spi.GPIO_CS,
                    deviceConfiguration->GPIO_RESET);
        }

        // save current configuration if a revert back is needed
        vl_currentConfig = *deviceConfiguration;

        if (deviceConfiguration->communicationBusType == BusSPI)
        {
            /* Sanity check */
            if((deviceConfiguration->communicationBus.spi.mode != SPI_CPHA0_CPOL0) &&
               (deviceConfiguration->communicationBus.spi.mode != SPI_CPHA1_CPOL1))
            {
                ret = OSAL_ERROR_INVALID_PARAM;
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_reconfiguration : Return = %d, ERROR SPI mode not correct (%d)",
                        ret , deviceConfiguration->communicationBus.spi.mode);
            }
            else
            {
                if((deviceConfiguration->communicationBus.spi.speed <= SPI_SPEED_MIN) ||
                    (deviceConfiguration->communicationBus.spi.speed >= SPI_SPEED_MAX))
                {
                    ret = OSAL_ERROR_INVALID_PARAM;
                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_reconfiguration : Return = %d, ERROR SPI frequency is out of range (Min = %d, Max = %d)",
                            ret, SPI_SPEED_MIN, SPI_SPEED_MAX);
                }
                else
                {
                    if(HDRADIODeviceConfiguration[HDRADIO_DCOP_ID_0].communicationBus.spi.mode != deviceConfiguration->communicationBus.spi.mode)
                    {
                        // set the new configuration :
                        HDRADIODeviceConfiguration[HDRADIO_DCOP_ID_0].communicationBus.spi.mode = deviceConfiguration->communicationBus.spi.mode;

                        /* Set up the spi mode */
						ret = BSP_BusConfigSPIMode_HDRADIO(HDRADIO_DCOP_ID_0);
                        if(ret != OSAL_OK)
                        {
                            // revert to prior configuration
                            HDRADIODeviceConfiguration[HDRADIO_DCOP_ID_0].communicationBus.spi.mode = vl_currentConfig.communicationBus.spi.mode;
                            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_reconfiguration : Return = %d, ERROR in BSP_BusConfigSPIMode_HDRADIO()", ret);
                        }
                    }

                    if(HDRADIODeviceConfiguration[HDRADIO_DCOP_ID_0].communicationBus.spi.speed != deviceConfiguration->communicationBus.spi.speed)
                    {
                        HDRADIODeviceConfiguration[HDRADIO_DCOP_ID_0].communicationBus.spi.speed = deviceConfiguration->communicationBus.spi.speed;

                        /* Set up the spi speed */
						ret = BSP_BusConfigSPIFrequency_HDRADIO(HDRADIO_DCOP_ID_0);
                        if(ret != OSAL_OK)
                        {
                            // revert to prior configuration
                            HDRADIODeviceConfiguration[HDRADIO_DCOP_ID_0].communicationBus.spi.speed = vl_currentConfig.communicationBus.spi.speed;
                            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_reconfiguration : Return = %d, ERROR in BSP_BusConfigSPIFrequency_HDRADIO()", ret);
                        }
                    }
                }
            }
        }
        else
        {
            ret = OSAL_ERROR_INVALID_PARAM;
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_reconfiguration : Return = %d, ERROR only SPI can be reconfigured", ret);
        }
    }
    else
    {
        ret = OSAL_ERROR_INVALID_PARAM;
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_init : Return = %d, ERROR pointer is NULL (deviceConfiguration = 0x%x)", ret);
    }

    if(ret == OSAL_OK)
    {
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HDRADIO_reconfiguration : Return = %d, SUCCESS", ret);
    }
    return ret;
}

/**************************************
 *
 * HDRADIO_Write_Command
 *
 *************************************/
/*
 * Writes a buffer to the HDRADIO
 *
 * Parameters:
 *  instId - HD instance number
 *  buf    - pointer to the buffer
 *  len    - size of the buffer in bytes
 *
 * The format of buf is:
 * 0            - opcode
 * 1 to 4       - data length (N)
 * 5 to (5+N-1) - command payload starting with the function code byte
 * 5 + N        - operation (0 for request, 1 for write)
 * 5 + N+1      - reserved (0)
 * 5 + N+2      - command status
 *
 * The function creates one or more LM packets, fragmenting the payload
 * if required.
 *
 * Returns:
 *  OSAL_OK                  - success
 *  OSAL_ERROR               - write failure or system not configured
 *  OSAL_ERROR_INVALID_PARAM - invalid parameter
 *
 */
tS32 HDRADIO_Write_Command(tyHDRADIOInstanceID instId, tU8 *buf, tU32 buf_len)
{
	tU32 u32CPDataLen;
	tU8 u8Opcode;
	tU8 u8CmdStatus;
	tU16 u16NumOfSeg;
	tU16 u16RemainderSeg;
	tU8 *wtBuf = LM_buf; //The buffer is statically allocated being defined with file scope. Although paying in ".bss" size, this way saves function's stack
	tU32 i;
	tS32 ret = OSAL_OK;

    HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Write_Command : InstanceID = %d, buffer = 0x%x, buf_len = %d", instId, buf, buf_len);

	/* Sanity check */
    if(((instId != INSTANCE_UNDEF) && (instId != INSTANCE_1) && (instId != INSTANCE_2)) || (buf == NULL))
	{
		ret = OSAL_ERROR_INVALID_PARAM;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Command : Return = %d, ERROR invalid instanceID or buffer with NULL pointer", ret);
		goto exit;
	}

	u8Opcode = buf[0];
	u32CPDataLen = (tU32)buf[1] + ((tU32)buf[2] << 8) + ((tU32)buf[3] << 16) + ((tU32)buf[4] << 24);
	u8CmdStatus = buf[u32CPDataLen+7];

	if(u32CPDataLen + CP_OVERHEAD != buf_len)
	{
		/*
		 * Data are computed, not got by the communication
		 * This is an abnormal situation
		 */
		ASSERT_ON_DEBUGGING(0);			//lint !e960 - MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Command : Return = %d, ERROR incorrect data length (%d)",
                ret, (u32CPDataLen + CP_OVERHEAD));
		goto exit;
	}

	if((tS32)buf[u32CPDataLen + 5] != (tS32)HDR_CMD_WRITE_TYPE)
	{
		/*
		 * Data are computed, not got by the communication
		 * This is an abnormal situation
		 */
		ASSERT_ON_DEBUGGING(0);			//lint !e960 - MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Command : Return = %d, ERROR invalid command type (%d)",
                ret, (tS32)buf[u32CPDataLen + 5]);
		goto exit;
	}

	// Segmentation of HC Cmd in LM packets

	// The result of the modulo is 0..LM_PAYLOAD_MAX_LEN so fits in a tU16 for sure
	// we can cast without problems
	u16RemainderSeg = (tU16)(u32CPDataLen % LM_PAYLOAD_MAX_LEN);
	// u32CPDataLen could be 32bit but in reality the LM provides only 16 bits for
	// the number of data segments (wtBuf[8], wtBuf[9]) so the upper layer
	// will need to limit the u32CPDataLen to 2^16*LM_PAYLOAD_MAX_LEN
	// Thus we can safely cast
	u16NumOfSeg = (tU16)(u32CPDataLen / LM_PAYLOAD_MAX_LEN) +1;

	(void)OSAL_pvMemorySet((tVoid *)wtBuf, 0, LM_FULL_SIZE);

	// increment the LM count for a new message 1st.
	LMCount++;
	
	wtBuf[2] = LMCount;
	wtBuf[3] = (tU8)instId;
	wtBuf[6] = u8Opcode;
	wtBuf[7] = u8CmdStatus;
	wtBuf[8] = (tU8)(u16NumOfSeg & 0xFF);
	wtBuf[9] = (tU8)((u16NumOfSeg >> 8) & 0xFF);

	for(i = 0; i < u32CPDataLen / LM_PAYLOAD_MAX_LEN; i++)
	{
		(void)OSAL_pvMemoryCopy((tVoid *)&wtBuf[LM_PAYLOAD_INDEX], (tPCVoid)&buf[CP_PAYLOAD_INDEX + i*LM_PAYLOAD_MAX_LEN], LM_PAYLOAD_MAX_LEN);

		wtBuf[4] = (tU8)((LM_PAYLOAD_MAX_LEN + LM_OVERHEAD) & 0xFF);
		wtBuf[5] = (tU8)(((LM_PAYLOAD_MAX_LEN + LM_OVERHEAD) >> 8) & 0xFF);
		wtBuf[10] = (tU8)(i & 0xFF);
		wtBuf[11] = (tU8)((i >> 8) & 0xFF);

		if(HDRADIO_Write_LM_Packet(wtBuf, LM_PAYLOAD_MAX_LEN + LM_OVERHEAD) != OSAL_OK)
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Command : Return = %d, ERROR in HDRADIO_Write_LM_Packet()", ret);
			goto exit;
		}
	}

	if(u16RemainderSeg != 0)
	{
		(void)OSAL_pvMemorySet((tVoid *)&wtBuf[LM_PAYLOAD_INDEX], 0, LM_PAYLOAD_MAX_LEN);

		(void)OSAL_pvMemoryCopy((tVoid *)&wtBuf[LM_PAYLOAD_INDEX], (tPCVoid)&buf[CP_PAYLOAD_INDEX + i*LM_PAYLOAD_MAX_LEN], u16RemainderSeg);

		wtBuf[4] = (tU8)((u16RemainderSeg + LM_OVERHEAD) & 0xFF);
		wtBuf[5] = (tU8)(((u16RemainderSeg + LM_OVERHEAD) >> 8) & 0xFF);
		wtBuf[10] = (tU8)(i & 0xFF);
		wtBuf[11] = (tU8)((i >> 8) & 0xFF);

		if(HDRADIO_Write_LM_Packet(wtBuf, u16RemainderSeg + LM_OVERHEAD) != OSAL_OK)
		{
			ret = OSAL_ERROR;
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Command : Return = %d, ERROR in HDRADIO_Write_LM_Packet()", ret);
			goto exit;
		}
	}

exit:
    if(ret == OSAL_OK)
    {
        HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Write_Command : Return = %d, SUCCESS", ret);
    }
	return ret;
}

/**************************************
 *
 * HDRADIO_Read_Response
 *
 *************************************/
/*
 * Reads a buffer from the HDRADIO
 *
 * Parameters:
 *  instId  - pointer to the HD instance number
 *  buf     - pointer to the buffer
 *  buf_len - size of the buffer in bytes
 *
 * Returns:
 *  OSAL_OK                   - success
 *  OSAL_ERROR                - read response failure
 *  OSAL_ERROR_INVALID_PARAM  - invalid parameter
 *
 */
tS32 HDRADIO_Read_Response(tyHDRADIOInstanceID *pInstId, tU8 *buf, tS32 *buf_len)
{
	tS32 rdbuf_len, ret = OSAL_OK;
	tU8 u8Opcode = (tU8)0;
	tU8 u8LMCount = (tU8)0;
	tU8 u8CmdStatus = (tU8)0;
	tU16 u16NumOfSeg = 0;
	tU16 u16SegNum = 0;
	tU32 u32CPDataLen = 0;
	tU16 u16LMLen, u16LMDataLen;
	tU8 *rdBuf = LM_buf; //The buffer is statically allocated being defined with file scope. Although paying in ".bss" size, this way saves function's stack
	tU32 u32Temp = 0;
	tS32 s32Temp = 0;

	HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : InstanceID = %d, buffer = 0x%x, buf_len = 0x%x", *pInstId, buf, buf_len);

	/* Sanity check */
#if 0
	if ((pInstId == NULL))
	{
		ret = OSAL_ERROR_INVALID_PARAM;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR incorrect pointer pInstId (0x%x) or buf (0x%x) or buf_len (0x%x)",
                ret, pInstId, buf, buf_len);
		goto exit;
	}
	else
	{
		if((*pInstId != INSTANCE_UNDEF) && (*pInstId != INSTANCE_1) && (*pInstId != INSTANCE_2))
		{
			ret = OSAL_ERROR_INVALID_PARAM;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR incorrect pInstId (%d)",
                    ret, *pInstId);
			goto exit;
		}
	}

	if ((buf == NULL) || (buf_len == NULL))
	{
		ret = OSAL_ERROR_INVALID_PARAM;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR incorrect buf (0x%x) or buf_len pointer (0x%x)",
                ret, buf, buf_len);
		goto exit;
	}
#endif
	// Reassemble of Response
	// The parameter is CP_buf to be reassembled by reading LM_buf
	do
	{
		//LM reading then checking for Status to acquire any possible error detected by the BBP
		//LM Count, Instance, LM Length, Opcode, Number of Data Segments and Data Segment Number inconsistency signal error from the BBP
		if (HDRADIO_Read_LM_Packet(&rdBuf, &rdbuf_len) != OSAL_OK)
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR in HDRADIO_Read_LM_Packet()", ret);
			goto exit;
		}

		if(rdbuf_len < 0)
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR incorrect rdbuf_len (%d)", ret, rdbuf_len);
			goto exit;
		}

		// LINT devitaion for pointer manipulation
		// an analysis must be done in code to check this ones directly at the source inside HDRADIO_LinuxRead fucntion.
		if((rdbuf_len > LM_PAYLOAD_MAX_LEN + LM_OVERHEAD) || (&rdBuf[rdbuf_len - 1] >= &LM_buf[LM_FULL_SIZE]) || (&rdBuf[0] < &LM_buf[0]))	//lint !e946 - MISRA 17.2 and 17.3 - Code check protection
		{
			/*
			 * The content read is more than the memory reserved
			 * This is an abnormal situation
			 */
			ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR incorrect rdbuf_len (%d) or rdBuf or LM_buf", ret, rdbuf_len);
			goto exit;
		}

		s32Temp = rdbuf_len - LM_OVERHEAD;
		u16LMDataLen = (tU16)(s32Temp);

		//memcpy to CP buf
		(void)OSAL_pvMemoryCopy((tVoid *)&buf[CP_PAYLOAD_INDEX + u32CPDataLen], (tPCVoid)&rdBuf[LM_PAYLOAD_INDEX], u16LMDataLen);
		u32CPDataLen += u16LMDataLen;

		if(u8CmdStatus == (tU8)0 || u8CmdStatus == LM_STATUS_SUCCESS)
		{
			u8CmdStatus = rdBuf[7]; //Updated only if no error
		}

		if(u8Opcode == (tU8)0)
		{
			u8Opcode = rdBuf[6]; //Updated only the first loop iteration. Should be equal in segments
		}
		else if(u8Opcode != rdBuf[6])
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR u8Opcode (%d) != rdBuf[6] (%d)", ret, u8Opcode, rdBuf[6]);
			goto enddowhile;
		}
		else
		{
			/* Nothing to do */
		}

		u16LMLen = (tU16)rdBuf[4] + (tU16)((tU16)rdBuf[5] << 8);
		if((tU16)rdbuf_len != u16LMLen)
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR rdbuf_len (%d) != u16LMLen (%d)", ret, rdbuf_len, u16LMLen);
			goto enddowhile;
		}

		if(u8LMCount == (tU8)0)
		{
			u8LMCount = rdBuf[2]; //Updated only the first loop iteration. Should be equal in segments
		}
		else if(u8LMCount != rdBuf[2])
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR u8LMCount (%d) != rdBuf[2] (%d)", ret, u8LMCount, rdBuf[2]);
			goto enddowhile;
		}
		else
		{
			/* Nothing to do */
		}

		if(*pInstId == INSTANCE_UNDEF)
		{
			if((tU32)INSTANCE_1 == (tU32)rdBuf[3] || (tU32)INSTANCE_2 == (tU32)rdBuf[3])
			{
				*pInstId = (tyHDRADIOInstanceID)rdBuf[3]; //Updated only the first loop iteration. Then it must be equal in segments
			}
			else
			{
				ret = OSAL_ERROR;
				HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %lu, ERROR invalid rdBuf[3] (%lu)", (tU32)ret, (tU32)rdBuf[3]);
				goto enddowhile;
			}
		}
		else if((tU32)*pInstId != (tU32)rdBuf[3])
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR *pInstId (%lu) != rdBuf[3] (%lu)", ret, (tU32)*pInstId, (tU32)rdBuf[3]);
			goto enddowhile;
		}
		else
		{
			/* Nothing to do */
		}

		if(u16NumOfSeg == 0)
		{
			u16NumOfSeg = (tU16)rdBuf[8] + (tU16)((tU16)rdBuf[9] << 8); //Updated only the first loop iteration. Should be equal in segments
		}
		else if(u16NumOfSeg != (tU16)rdBuf[8] + (tU16)((tU16)rdBuf[9] << 8))
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR u16NumOfSeg (%u) != (rdBuf[8] + (rdBuf[9] << 8)) (%u)", 
					ret, u16NumOfSeg, ((tU16)rdBuf[8] + (tU16)((tU16)rdBuf[9] << 8)));
			goto enddowhile;
		}
		else
		{
			/* Nothing to do */
		}

		if(u16SegNum != (tU16)rdBuf[10] + (tU16)((tU16)rdBuf[11] << 8))
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR u16SegNum (%u) != (rdBuf[10] + (rdBuf[11] << 8)) (%u)",
				ret, u16SegNum, ((tU16)rdBuf[10] + (tU16)((tU16)rdBuf[11] << 8)));
			goto enddowhile;
		}
		u16SegNum++;

	}while(rdbuf_len == LM_PAYLOAD_MAX_LEN + LM_OVERHEAD);
enddowhile:
	
	if(((u16NumOfSeg != 0) && (u16NumOfSeg != u16SegNum)) || ((u16NumOfSeg == 0) && (u16SegNum != 1)))
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR incorrect u16NumOfSeg (%d) or u16SegNum (%d)", ret, u16NumOfSeg, u16SegNum);
	}

	if(u8LMCount != LMCount)
	{
		/*
		 * Count alignment is lost and won't be obtained unless by forcing
		 * This is an abnormal situation
		 */
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, ERROR u8LMCount (%d) !=  LMCount (%d)", ret, u8LMCount, LMCount);
	}
	else
	{
		// EPR note : I think we should always increment the LM count, because we do not do retransmission
		//LMCount += (tU8)1;
	}

	buf[0] = u8Opcode;

	buf[1] = (tU8)(u32CPDataLen & 0xFF);
	buf[2] = (tU8)((u32CPDataLen >> 8) & 0xFF);
	buf[3] = (tU8)((u32CPDataLen >> 16) & 0xFF);
	buf[4] = (tU8)((u32CPDataLen >> 24) & 0xFF);

	buf[u32CPDataLen + 5] = (tU8)HDR_CMD_READ_TYPE;
	buf[u32CPDataLen + 6] = (tU8)0x00; // reserved
	buf[u32CPDataLen + 7] = u8CmdStatus;

	if(ret != OSAL_ERROR)
	{
		u32Temp = u32CPDataLen + CP_OVERHEAD;
		*buf_len = (tS32)(u32Temp);
	}
	else
	{
		*buf_len = 0;
	}

exit:
    if(ret == OSAL_OK)
    {
        HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Read_Response : Return = %d, SUCCESS %lu bytes received", ret, *buf_len);
#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_SYSTEM)
        COMMON_tracePrintBufComponent(TR_CLASS_HDRADIO, buf, *buf_len, NULL);
#endif
    }
	return ret;
}

/**************************************
 *
 * HDRADIO_Write_Segment_Command
 *
 *************************************/
/*
 * Writes a segment buffer to the HDRADIO
 *
 * Parameters:
 *  instId     - HD instance number
 *  nb_data_segments  - number of data segments
 *  data_segment_nb   - data segment number
 *  buf        - pointer to the buffer
 *  buf_len    - size of the buffer in bytes
 *
 * The format of buf is:
 * 0            - opcode
 * 1 to 4       - data length (N)
 * 5 to (5+N-1) - command payload starting with the function code byte
 * 5 + N        - operation (0 for request, 1 for write)
 * 5 + N+1      - reserved (0)
 * 5 + N+2      - command status
 *
 * The function creates one long LM packet
 *
 * Returns:
 *  OSAL_OK                   - success
 *  OSAL_ERROR                - write failure
 *  OSAL_ERROR_INVALID_PARAM  - invalid parameter
 *
 */
tS32 HDRADIO_Write_Segment_Command(tyHDRADIOInstanceID instId, tU16 nb_data_segments, tU16 data_segment_nb, tU8 *buf, tU32 buf_len)
{
	tU32 u32CPDataLen;
	tU8 u8Opcode;
	tU8 u8CmdStatus;
	tU16 u16LmLength;
	tU8 *wtBuf = LM_buf; /* The buffer is statically allocated being defined with file scope. Although paying in ".bss" size, this way saves function's stack */
	tS32 ret = OSAL_OK;
	

    HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Write_Segment_Command : InstanceID = %d, nb_data_segments = %d, data_segment_nb = %d, buffer = 0x%x, buf_len = %d",
            instId, nb_data_segments, data_segment_nb, buf, buf_len);

	/* Sanity check */
    if(((instId != INSTANCE_UNDEF) && (instId != INSTANCE_1) && (instId != INSTANCE_2)) || (buf == NULL))
	{
		ret = OSAL_ERROR_INVALID_PARAM;
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Segment_Command : Return = %d, ERROR incorrect instId value or buffer pointer", ret);
		goto exit;
	}

	u8Opcode = buf[0];
	u32CPDataLen = (tU32)buf[1] + ((tU32)buf[2] << 8) + ((tU32)buf[3] << 16) + ((tU32)buf[4] << 24);
	u16LmLength = (tU16)(u32CPDataLen + (tU32)LM_OVERHEAD); // tested below to fit in tU16
	u8CmdStatus = buf[u32CPDataLen+7];

	if ((u32CPDataLen + CP_OVERHEAD != buf_len) || (u32CPDataLen > (tU32)0xFFFF) || (u32CPDataLen > (tU32)LM_PAYLOAD_MAX_LEN))
	{
		/*
		 * Data are computed, not got by the communication
		 * This is an abnormal situation
		 */
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Segment_Command : Return = %d, ERROR incorrect u32CPDataLen (%lu)", ret, u32CPDataLen);
		goto exit;
	}

	if((tS32)buf[u32CPDataLen + 5] != (tS32)HDR_CMD_WRITE_TYPE)
	{
		/*
		 * Data are computed, not got by the communication
		 * This is an abnormal situation
		 */
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Segment_Command : Return = %d, ERROR incorrect command type (%l)", ret, (tS32)buf[u32CPDataLen + 5]);
		goto exit;
	}

	/* add LM header */
	wtBuf[2] = LMCount;
	wtBuf[3] = (tU8)instId;
	wtBuf[4] = (tU8)(u16LmLength & (tU32)0xFF);
	wtBuf[5] = (tU8)((u16LmLength >> 8) & (tU32)0xFF);
	wtBuf[6] = u8Opcode;
	wtBuf[7] = u8CmdStatus;
	wtBuf[8] = (tU8)(nb_data_segments & (tU16)0xFF);
	wtBuf[9] = (tU8)((nb_data_segments >> 8) & (tU16)0xFF);
	wtBuf[10] = (tU8)(data_segment_nb & (tU16)0xFF);
	wtBuf[11] = (tU8)((data_segment_nb >> 8) & (tU16)0xFF);

	/* copy data payload */
	(void)OSAL_pvMemoryCopy((tVoid *)&wtBuf[LM_PAYLOAD_INDEX], (tPCVoid)&buf[CP_PAYLOAD_INDEX], u32CPDataLen);

	/* clear checksum */
	wtBuf[LM_PAYLOAD_INDEX + u32CPDataLen] = (tU8)0;

	/* send data segment */
	if(HDRADIO_Write_LM_Packet(wtBuf, u16LmLength) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Segment_Command : Return = %d, ERROR in HDRADIO_Write_LM_Packet()", ret);
		goto exit;
	}

exit:
    if(ret == OSAL_OK)
    {
        HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Write_Segment_Command : Return = %d, SUCCESS", ret);
    }
	return ret;
}

/**************************************
 *
 * HDRADIO_Read_Segment_Status
 *
 *************************************/
/*
 * Reads a response status buffer from the HDRADIO
 *
 * Parameters:
 *  instId - expected HD instance number of response status
 *  u8Opcode       - expected opcode of response status
 *  u16NumOfSeg - expected number of segments of response status
 *  u16SegNum    - expected segment number of response status
 *  buf    - pointer to the buffer
 *  buf_len    - pointer to the buffer length
 *
 * Returns:
 *  OSAL_OK                   - success
 *  OSAL_ERROR                - read failure
 *  OSAL_ERROR_INVALID_PARAM  - invalid parameter
 *
 */
tS32 HDRADIO_Read_Segment_Status(tyHDRADIOInstanceID instId, tU8 u8Opcode, tU16 u16NumOfSeg, tU16 u16SegNum, tU8 *buf, tS32 * buf_len)
{
	tS32 rdbuf_len, ret = OSAL_OK;
	tU8 u8LMCount = (tU8)0;
	tU8 u8CmdStatus = (tU8)0;
	tU32 u32CPDataLen = 0;
	tU16 u16LMLen, u16LMDataLen;
	tU8 *rdBuf = LM_buf; //The buffer is statically allocated being defined with file scope. Although paying in ".bss" size, this way saves function's stack
	tU32 u32Temp = 0;
	tS32 s32Temp = 0;

    HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : InstanceID = %d, u8Opcode = %d, u16NumOfSeg = %d, u16SegNum = %d, buffer = 0x%x, buf_len = 0x%x",
            instId, u8Opcode, u16NumOfSeg, u16SegNum, buf, buf_len);

	/* Sanity check */
    if(((instId != INSTANCE_UNDEF) && (instId != INSTANCE_1) && (instId != INSTANCE_2)) || ((buf == NULL) || (buf_len == NULL)))
	{
		ret = OSAL_ERROR_INVALID_PARAM;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR incorrect instId value, buf or buf_len pointer", ret);
		goto exit;
	}

	//LM reading then checking for Status to acquire any possible error detected by the BBP
	//LM Count, Instance, LM Length, Opcode, Number of Data Segments and Data Segment Number inconsistency signal error from the BBP
	if (HDRADIO_Read_LM_Packet(&rdBuf, &rdbuf_len) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR in HDRADIO_Read_LM_Packet()", ret);
		goto exit;
	}

	if((rdbuf_len < 0) || (rdbuf_len == LM_PAYLOAD_MAX_LEN + LM_OVERHEAD))
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR incorrect rdbuf_len (%d)", ret, rdbuf_len);
		goto exit;
	}

	// LINT devitaion for pointer manipulation
	// an analysis must be done in code to check this ones directly at the source inside HDRADIO_LinuxRead fucntion.
	if((rdbuf_len > LM_PAYLOAD_MAX_LEN + LM_OVERHEAD) || (&rdBuf[rdbuf_len - 1] >= &LM_buf[LM_FULL_SIZE]) || (&rdBuf[0] < &LM_buf[0]))		//lint !e946 - MISRA 17.2 and 17.3 - Code check protection
	{
		/*
		 * The content read is more than the memory reserved
		 * This is an abnormal situation
		 */
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR incorrect rdbuf_len (%d) or rdBuf or LM_buf", ret, rdbuf_len);
		goto exit;
	}

	s32Temp = rdbuf_len - LM_OVERHEAD;
	u16LMDataLen = (tU16)(s32Temp);

	//memcpy to CP buf
	(void)OSAL_pvMemoryCopy((tVoid *)&buf[CP_PAYLOAD_INDEX + u32CPDataLen], (tPCVoid)&rdBuf[LM_PAYLOAD_INDEX], u16LMDataLen);
	u32CPDataLen += u16LMDataLen;

	if(u8CmdStatus == (tU8)0 || u8CmdStatus == LM_STATUS_SUCCESS)
	{
		u8CmdStatus = rdBuf[7]; //Updated only if no error
	}

	if(u8Opcode != rdBuf[6])
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR u8Opcode (%d) != rdBuf[6] (%d)", ret, u8Opcode, rdBuf[6]);
	}

	u16LMLen = (tU16)rdBuf[4] + (tU16)((tU16)rdBuf[5] << 8);
	if((tU16)rdbuf_len != u16LMLen)
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR rdBuf_len (%d) != u16LMLen (%d)", ret, rdbuf_len, u16LMLen);
	}

	if(u8LMCount == (tU8)0)
	{
		u8LMCount = rdBuf[2]; //Updated only the first loop iteration. Should be equal in segments
	}
	else if(u8LMCount != rdBuf[2])
	{
		ret = OSAL_ERROR;
	}
	else
	{
		/* Nothing to do */
	}

	if((tU8)INSTANCE_1 != rdBuf[3] && (tU8)INSTANCE_2 != rdBuf[3])
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR u8LMCount (%d) != rdBuf[2] (%d)", ret, u8LMCount, rdBuf[2]);
	}
	if(instId != INSTANCE_UNDEF)
	{
		if((tU8)instId != rdBuf[3])
		{
			ret = OSAL_ERROR;
			HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR incorrect rdBuf[3] (%d)", ret, rdBuf[3]);
		}
	}

	if(u16NumOfSeg != (tU16)rdBuf[8] + (tU16)((tU16)rdBuf[9] << 8))
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR u16NumOfSeg (%d) != rdBuf[8] + (rdBuf[9] << 8) (%d)",
			ret, u16NumOfSeg, ((tU16)rdBuf[8] + (tU16)((tU16)rdBuf[9] << 8)));
	}

	if (u16SegNum != (tU16)rdBuf[10] + (tU16)((tU16)rdBuf[11] << 8))
	{
		ret = OSAL_ERROR;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR u16SegNum (%d) != rdBuf[10] + (rdBuf[11] << 8) (%d)", 
			ret, u16SegNum, ((tU16)rdBuf[10] + (tU16)((tU16)rdBuf[11] << 8)));
	}

	if(u8LMCount != LMCount)
	{
		/*
		 * Count alignment is lost and won't be obtained unless by forcing
		 * This is an abnormal situation
		 */
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, ERROR u8LMCount (%d) != LMCount (%d)", ret, u8LMCount, LMCount);
	}
	else
	{
		LMCount += (tU8)1;
	}

	buf[0] = u8Opcode;

	buf[1] = (tU8)(u32CPDataLen & 0xFF);
	buf[2] = (tU8)((u32CPDataLen >> 8) & 0xFF);
	buf[3] = (tU8)((u32CPDataLen >> 16) & 0xFF);
	buf[4] = (tU8)((u32CPDataLen >> 24) & 0xFF);

	buf[u32CPDataLen + 5] = (tU8)HDR_CMD_READ_TYPE;
	buf[u32CPDataLen + 6] = (tU8)0x00; // reserved
	buf[u32CPDataLen + 7] = u8CmdStatus;

	if(ret != OSAL_ERROR)
	{
		u32Temp = u32CPDataLen + CP_OVERHEAD;
		*buf_len = (tS32)(u32Temp);
	}
	else
	{
		*buf_len = 0;
	}

exit:
    if(ret == OSAL_OK)
    {
        HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Read_Segment_Status : Return = %d, SUCCESS %lu bytes received", ret, *buf_len);
#if defined(CONFIG_TRACE_CLASS_HDRADIO) && (CONFIG_TRACE_CLASS_HDRADIO >= TR_LEVEL_SYSTEM)
        COMMON_tracePrintBufComponent(TR_CLASS_HDRADIO, buf, *buf_len, NULL);
#endif
    }
	return ret;
}


/**************************************
 *
 * HDRADIO_Write_Raw_Data
 *
 *************************************/
/*
 * Write a raw data from the DCOP
 *
 * Parameters:
 *  buf         - pointer to the buffer
 *  buf_len     - the buffer length
 *
 * Returns:
 *  OSAL_OK                   - success
 *  OSAL_ERROR                - read failure
 *  OSAL_ERROR_INVALID_PARAM  - invalid parameter
 *
 */
tS32 HDRADIO_Write_Raw_Data(tU8 *buf, tU32 buf_len)
{
	tS32 ret;
	
	HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Write_Raw_Data : buffer = 0x%x, buf_len = %d", buf, buf_len);

	if((buf == NULL) || (buf_len > 0x7FFFF000))
	{
		ret = OSAL_ERROR_INVALID_PARAM;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Raw_Data : Return = %d, ERROR incorrect buf or buf_len", ret);
	}
	else
	{
		if(BSP_Write_HDRADIO(HDRADIO_DCOP_ID_0, buf, buf_len) != (tS32)buf_len)
		{
			ret = OSAL_ERROR;
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Write_Raw_Data : Return = %d, ERROR in BSP_Write_HDRADIO()", ret);
		}
		else
		{
			ret = OSAL_OK;
            HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Write_Raw_Data : Return = %d, SUCCESS %lu bytes written", ret, buf_len);
		}
	}
	
	return ret;
}


/**************************************
 *
 * HDRADIO_Read_Raw_Data
 *
 *************************************/
/*
 * Read a raw data from the DCOP
 *
 * Parameters:
 *  buf         - pointer to the buffer
 *  buf_len     - the buffer length
 *
 * Returns:
 *  OSAL_OK                   - success
 *  OSAL_ERROR                - read failure
 *  OSAL_ERROR_INVALID_PARAM  - invalid parameter
 *
 */
tS32 HDRADIO_Read_Raw_Data(tU8 *buf, tU32 buf_len)
{
	tS32 ret;

	HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Read_Raw_Data : buffer = 0x%x, buf_len = %d", buf, buf_len);

	if((buf == NULL) || (buf_len > 0x7FFFF000))
	{
		ret = OSAL_ERROR_INVALID_PARAM;
		HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Raw_Data : Return = %d, ERROR incorrect buf or buf_len", ret);
	}
	else
	{
		if(BSP_Raw_Read_HDRADIO(HDRADIO_DCOP_ID_0, buf, buf_len) != (tS32)buf_len)
		{
			ret = OSAL_ERROR;
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "HDRADIO_Read_Raw_Data : Return = %d, ERROR in BSP_Raw_Read_HDRADIO()", ret);
        }
        else
        {
            ret = OSAL_OK;
            HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "HDRADIO_Read_Raw_Data : Return = %d, SUCCESS %lu bytes received", ret, buf_len);
        }
	}
	return ret;
}

tyCommunicationBusType HDRADIO_GetBusType(tU32 deviceID)
{
	return HDRADIODeviceConfiguration[deviceID].communicationBusType;
}

tU32 HDRADIO_GetGPIOReset(tU32 deviceID)
{
	return HDRADIODeviceConfiguration[deviceID].GPIO_RESET;
}

tVoid *HDRADIO_GetCommunicationBusConfig(tU32 deviceID)
{
	tVoid *ret;
	if(HDRADIODeviceConfiguration[deviceID].communicationBusType == BusSPI)
	{
		ret = (tVoid*)&(HDRADIODeviceConfiguration[deviceID].communicationBus.spi);
	}
	else
	{
		ret = (tVoid*)&(HDRADIODeviceConfiguration[deviceID].communicationBus.i2c);
	}

	return ret;
}

#endif //#if (defined CONFIG_COMM_DRIVER_EMBEDDED) && (defined CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

//EOF
