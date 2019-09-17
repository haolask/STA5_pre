//!
//!  \file 		DAB_Protocol.c
//!  \brief 	<i><b> DAB DCOP driver </b></i>
//!  \details   Low level driver for the DAB DCOP device
//!  \author 	Jean-Hugues Perrin
//!
/*
 * This file contains:
 * - the raw driver to access the DAB DCOP over ..... bus from a Linux environment,
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

#if defined CONFIG_ETAL_SUPPORT_DCOP_MDR

#include "osal.h"

#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	#include "ipfcomm.h"
#endif

#if defined (CONFIG_HOST_OS_LINUX)
	#include <sys/ioctl.h>
#endif
#include <unistd.h>

#include "bsp_sta1095evb.h"
#include "bsp_trace.h"
#include "DAB_Protocol.h"
#include "DAB_internal.h"
#include "steci_lld.h"
#include "steci_helpers.h"
#include "connection_modes.h"
#include "steci_protocol.h"
#include "common_trace.h"
#include "steci_trace.h"

/**************************************
 * Defines and macros
 *************************************/
/* SPI frequency */
#define SPI_SPEED_MIN           1076
#define SPI_SPEED_MAX           7056000

/**************************************
 * Local types
 *************************************/

/**************************************
 * Local variables
 *************************************/

#ifndef CONFIG_COMM_DRIVER_EXTERNAL
static STECI_paramsTy STECIParam;
#endif //CONFIG_COMM_DRIVER_EXTERNAL
static STECI_deviceInfoTy deviceInfo[MAX_DAB_DCOP_DEVICE];
static tyDABDeviceConfiguration DABDeviceConfiguration[MAX_DAB_DCOP_DEVICE];
static tChar DABExternalDriverIPAddress[IP_ADDRESS_CHARLEN] = DEFAULT_IP_ADDRESS;

/**************************************
 * Function prototypes
 *************************************/
#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
static tSInt DAB_sendResetWithExternalDriver(tVoid);
static tBool commMdr_wait_reset_response = FALSE;
#endif //CONFIG_COMM_DRIVER_EXTERNAL

/**************************************
 * Function definitions
 *************************************/

#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
/***************************
 *
 * DAB_sendResetWithExternalDriver
 *
 **************************/
/*!
 * \brief		Issue a reset to the DAB DCOP using the external driver
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 * \todo		Update when the Protocol Layer will provide the reset support
 */
static tSInt DAB_sendResetWithExternalDriver(tVoid)
{
	tU8 h0, h1, h2, h3;
	tU32 clen = 0;
	tU8 *cmd = NULL;

	h0 = DAB_EXTERNAL_DRIVER_TYPE_RESET;
	h1 = (tU8)0;
	h2 = DAB_EXTERNAL_DRIVER_RESERVED;
	h3 = DAB_EXTERNAL_DRIVER_RESERVED;
	ForwardPacketToCtrlAppPort(ProtocolLayerIndexDCOP, cmd, clen, (tU8)0x1D, BROADCAST_LUN, h0, h1, h2, h3);
	return OSAL_OK;
}
#endif // CONFIG_COMM_DRIVER_EXTERNAL


/**************************************
 *
 * DAB_Driver_Init
 *
 *************************************/
/*
 * DAB_Driver_Init global initialization
 *
 * Parameters:
 * deviceConfiguration : indicate the device configuration
 *
 * The function is called to initialize DAB and configure
 * the communication with the DCOP device.
 * it does not reset the DCOP
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - failure during initialization
 */
tS32 DAB_Driver_Init(tyDABDeviceConfiguration *deviceConfiguration, tBool vI_reset)
{
    tS32 ret = OSAL_OK;

    DABDeviceConfiguration[DAB_DCOP_ID_0] = *deviceConfiguration;
    deviceInfo[DAB_DCOP_ID_0].DABDeviceConfigID = DAB_DCOP_ID_0;

    if(deviceConfiguration != NULL)
    {
        if(deviceConfiguration->communicationBusType == BusI2C)
        {
            STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Driver_Init : IsBootMode = %d, Bus name = %s, device address = 0x%x, GPIO_reset = %d, GPIO_REQ = %d, GPIO_BOOT = %d",
                    deviceConfiguration->isBootMode,
                    deviceConfiguration->communicationBus.i2c.busName,
                    deviceConfiguration->communicationBus.i2c.deviceAddress,
                    deviceConfiguration->GPIO_RESET,
                    deviceConfiguration->GPIO_REQ,
                    deviceConfiguration->GPIO_BOOT);

			ret = OSAL_ERROR;
           STECI_tracePrintError(TR_CLASS_STECI, "DAB_Driver_Init : Return = %d, I2C not supported for DCOP", ret);
        }
        else
        {
            STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Driver_Init : IsBootMode = %d, Bus name = %s, SPI mode = %d, SPI speed = %d, GPIO_CS = %d, GPIO_reset = %d, GPIO_REQ = %d, GPIO_BOOT = %d",
                    deviceConfiguration->isBootMode,
                    deviceConfiguration->communicationBus.spi.busName,
                    deviceConfiguration->communicationBus.spi.mode,
                    deviceConfiguration->communicationBus.spi.speed,
                    deviceConfiguration->communicationBus.spi.GPIO_CS,
                    deviceConfiguration->GPIO_RESET,
                    deviceConfiguration->GPIO_REQ,
                    deviceConfiguration->GPIO_BOOT);
        }

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
        if (!TcpIpProtocolIsConnected(ProtocolLayerIndexDCOP))
        {
        	STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Driver_Init : Start TCP/IP thread for EXTERNAL driver operation with DCOP with IP : %s",
        			DABExternalDriverIPAddress);
			if (ipforwardInit(DABExternalDriverIPAddress, TCP_IP_CLIENT_PORT_NUMBER_DCOP, LOG_MASK_DCOP, LOG_FILENAME_DCOP, &ProtocolLayerIndexDCOP) != OSAL_OK)
			{
				ret = OSAL_ERROR;
				STECI_tracePrintError(TR_CLASS_STECI, "DAB_Driver_Init : Return = %d, ERROR DCOP TCP/IP thread or semaphore creation", ret);
			}
			else
			{
				/* Send a RESET command to the DCOP */
				DAB_sendResetWithExternalDriver();
				STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Driver_Init : Return = %d, SUCCESS", ret);
			}
        }
        else
        {
        	STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Driver_Init : TCP/IP thread for EXTERNAL driver operation with DCOP already started");
        }
#else //CONFIG_COMM_DRIVER_EXTERNAL
        if (BSP_DeviceInit_MDR(deviceInfo->DABDeviceConfigID) == OSAL_OK)
        {
			if (TRUE == vI_reset)
			{		
	            /* Set BOOT GPIO */
    	        BSP_SteciSetBOOT_MDR(deviceConfiguration->isBootMode);

				if(STECI_ResetDevice(deviceInfo) != STECI_STATUS_SUCCESS)
				{
					ret = OSAL_ERROR;
					STECI_tracePrintError(TR_CLASS_STECI, "DAB_Driver_Init : Return = %d, ERROR in STECI_ResetDevice()", ret);
				}
			}
			
            if (ETAL_InitSteciProtocol(&STECIParam) != OSAL_OK)
            {
                ret = OSAL_ERROR;
                STECI_tracePrintError(TR_CLASS_STECI, "DAB_Driver_Init : Return = %d, ERROR in ETAL_InitSteciProtocol()", ret);
            }
            else
            {
                STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Driver_Init : Return = %d, SUCCESS", ret);
            }
        }
        else
        {
            ret = OSAL_ERROR;
            STECI_tracePrintError(TR_CLASS_STECI, "DAB_Driver_Init : Return = %d, ERROR in BSP_DeviceInit_MDR()", ret);
        }
#endif //CONFIG_COMM_DRIVER_EXTERNAL
    }
    else
    {
        ret = OSAL_ERROR;
        STECI_tracePrintError(TR_CLASS_STECI, "DAB_Driver_Init : Return = %d, ERROR pointer is NULL (deviceConfiguration = 0x%x)", ret, deviceConfiguration);
    }

    return ret;
}


/**************************************
 *
 * DAB_Deinit
 *
 *************************************/
/*
 * DAB_Deinit
 *
 * Parameters:
 *
 * The function is called to close the resources configured to access the
 * DAB DCOP device.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - failure during deinitialization
 */
tS32 DAB_Deinit(tVoid)
{
    tS32 ret = OSAL_OK;

    STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Deinit");

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Deinit : Stop TCP/IP thread for EXTERNAL driver operation with DCOP");
	if (ipforwardDeinit(ProtocolLayerIndexDCOP) != OSAL_OK)
	{
		ret = OSAL_ERROR;
        STECI_tracePrintError(TR_CLASS_STECI, "DAB_Deinit : Return = %d, ERROR in ipforwardDeinit()", ret);
	}
	else
	{
		STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Deinit : Return = %d, SUCCESS", ret);
	}
#else //CONFIG_COMM_DRIVER_EXTERNAL
	if (ETAL_DeinitSteciProtocol() != OSAL_OK)
    {
        ret = OSAL_ERROR;
        STECI_tracePrintError(TR_CLASS_STECI, "DAB_Deinit : Return = %d, ERROR in ETAL_DeinitSteciProtocol()", ret);
    }
    else
    {
        BSP_DeviceDeinit_MDR();
        STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Deinit : Return = %d, SUCCESS", ret);
    }
#endif //CONFIG_COMM_DRIVER_EXTERNAL
    return ret;
}

/***************************
 *
 * DAB_setTcpIpAddress
 *
 **************************/
/*!
 * \brief		Set TCP/IP address for external driver
 * \param[in]	IPAddress - IP address
 * \return		OSAL_OK
 * \see			CMOST_helperSendMessage
 * \callgraph
 * \callergraph
 */
tSInt DAB_setTcpIpAddress(tChar *IPAddress)
{
	tSInt ret = OSAL_OK;

	if(IPAddress != NULL)
	{
		strncpy ( DABExternalDriverIPAddress, IPAddress, IP_ADDRESS_CHARLEN );
	}
	else
	{
		strncpy ( DABExternalDriverIPAddress, DEFAULT_IP_ADDRESS, IP_ADDRESS_CHARLEN );
		ret = OSAL_ERROR;
	}
	return ret;
}

/**************************************
 *
 * DAB_Reset
 *
 *************************************/
/*
 * Issue a hardware reset the DAB DCOP
 * and deinit/reinit the STECI protocol
 *
 *
 * Parameters:
 *
 * The function is called to reset DAB DCOP device.

 *
 * Returns:
 *  OSAL_OK         - success
 *  OSAL_ERROR      - failure during reset
 */
tS32 DAB_Reset(tVoid)
{
    tS32 ret = OSAL_OK;

    STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Reset");

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	if (TcpIpProtocolIsConnected(ProtocolLayerIndexDCOP))
	{
		/* Send a RESET command to the DCOP */
		DAB_sendResetWithExternalDriver();
		commMdr_wait_reset_response = TRUE;
		STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Reset : Return = %d, SUCCESS", ret);
	}
	else
	{
        ret = OSAL_ERROR;
        STECI_tracePrintError(TR_CLASS_STECI, "DAB_Reset : Return = %d, ERROR in TcpIpProtocolIsConnected()", ret);
	}
#else //CONFIG_COMM_DRIVER_EXTERNAL
    if (ETAL_DeinitSteciProtocol() != OSAL_OK)
    {
        ret = OSAL_ERROR;
        STECI_tracePrintError(TR_CLASS_STECI, "DAB_Reset : Return = %d, ERROR in ETAL_DeinitSteciProtocol()", ret);
    }
    else
    {
        /* Set BOOT GPIO */
        BSP_SteciSetBOOT_MDR(DABDeviceConfiguration[DAB_DCOP_ID_0].isBootMode);

        if(STECI_ResetDevice(deviceInfo) != STECI_STATUS_SUCCESS)
        {
            ret = OSAL_ERROR;
            STECI_tracePrintError(TR_CLASS_STECI, "DAB_Reset : Return = %d, ERROR in STECI_ResetDevice()", ret);
        }
        else
        {
            if (ETAL_InitSteciProtocol(&STECIParam) != OSAL_OK)
            {
                ret = OSAL_ERROR;
                STECI_tracePrintError(TR_CLASS_STECI, "DAB_Reset : Return = %d, ERROR in ETAL_InitSteciProtocol()", ret);
            }
            else
            {
                STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Reset : Return = %d, SUCCESS", ret);
            }
        }
    }
#endif //CONFIG_COMM_DRIVER_EXTERNAL
    return ret;
}

/**************************************
 *
 * DAB_StartupReset
 *
 *************************************/
/*
 * Issue only an hardware reset the DAB DCOP
 *
 * Parameters:
 *
 * The function is called to reset DAB DCOP device.
 *
 * Returns:
 *  OSAL_OK         - success
 *  OSAL_ERROR      - failure during reset
 */
tS32 DAB_StartupReset(tyDABDeviceConfiguration *deviceConfiguration)
{
    tS32 ret = OSAL_OK;

    STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_StartupReset");


    DABDeviceConfiguration[DAB_DCOP_ID_0] = *deviceConfiguration;
    deviceInfo[DAB_DCOP_ID_0].DABDeviceConfigID = DAB_DCOP_ID_0;

    if(deviceConfiguration != NULL)
    {
#ifdef CONFIG_COMM_DRIVER_EXTERNAL
        STECI_tracePrintError(TR_CLASS_STECI, "DAB_StartupReset : Return = %d, SUCCESS (Warning : this function should has no effect)", ret);
#else //CONFIG_COMM_DRIVER_EXTERNAL
    	if(deviceConfiguration->communicationBusType == BusI2C)
        {
           STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_StartupReset : IsBootMode = %d, Bus name = %s, device address = 0x%x, GPIO_reset = %d, GPIO_REQ = %d, GPIO_BOOT = %d",
           deviceConfiguration->isBootMode,
           deviceConfiguration->communicationBus.i2c.busName,
           deviceConfiguration->communicationBus.i2c.deviceAddress,
           deviceConfiguration->GPIO_RESET,
           deviceConfiguration->GPIO_REQ,
           deviceConfiguration->GPIO_BOOT);
		   
           ret = OSAL_ERROR;
           STECI_tracePrintError(TR_CLASS_STECI, "DAB_StartupReset : Return = %d, I2C not supported for DCOP", ret);
        }
        else
        {
            STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_StartupReset : IsBootMode = %d, Bus name = %s, SPI mode = %d, SPI speed = %d, GPIO_CS = %d, GPIO_reset = %d, GPIO_REQ = %d, GPIO_BOOT = %d",
                    deviceConfiguration->isBootMode,
                    deviceConfiguration->communicationBus.spi.busName,
                    deviceConfiguration->communicationBus.spi.mode,
                    deviceConfiguration->communicationBus.spi.speed,
                    deviceConfiguration->communicationBus.spi.GPIO_CS,
                    deviceConfiguration->GPIO_RESET,
                    deviceConfiguration->GPIO_REQ,
                    deviceConfiguration->GPIO_BOOT);
			
			if (BSP_DeviceInit_MDR(deviceInfo->DABDeviceConfigID) == OSAL_OK)
			 {
				 /* Set BOOT GPIO */
				 BSP_SteciSetBOOT_MDR(deviceConfiguration->isBootMode);

				/*Reset MDR */
    			BSP_DeviceReset_MDR(DAB_DCOP_ID_0);
            	STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_StartupReset : Return = %d, SUCCESS", ret);
			}
			else
        	{
        		ret = OSAL_ERROR;
        		STECI_tracePrintError(TR_CLASS_STECI, "DAB_StartupReset : Return = %d, ERROR in BSP_DeviceInit_MDR()", ret);
        	}
        }
#endif //CONFIG_COMM_DRIVER_EXTERNAL
    }
	else
	{
		STECI_tracePrintError(TR_CLASS_STECI,"DAB_StartupReset : invalid Device Configuration");
		ret = OSAL_ERROR;
	}

    return ret;
}


/**************************************
 *
 * DAB_Reconfiguration
 *
 *************************************/
/*
 * DAB device reconfiguration
 *
 * Parameters:
 * deviceConfiguration : pointer to device configuration.
 *
 * The function is called to reconfigure DAB device.
 * (limited to mode or/and speed of the SPI bus).
 *
 * Returns:
 *  OSAL_OK         - success
 *  OSAL_ERROR      - failure during reconfiguration
 */
tS32 DAB_Reconfiguration(tyDABDeviceConfiguration *deviceConfiguration)
{
    tS32 ret = OSAL_OK;

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
    ret = OSAL_ERROR;
    STECI_tracePrintError(TR_CLASS_STECI, "DAB_Reconfiguration : Return = %d, ERROR This function should not be called", ret);
#else //CONFIG_COMM_DRIVER_EXTERNAL
    tyDABDeviceConfiguration vl_currentConfig;

    if(deviceConfiguration != NULL)
    {
        if(deviceConfiguration->communicationBusType == BusI2C)
        {
            STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Reconfiguration : Bus name = %s, device address = 0x%x, GPIO_reset = %d, GPIO_REQ = %d, GPIO_BOOT = %d",
                    deviceConfiguration->communicationBus.i2c.busName,
                    deviceConfiguration->communicationBus.i2c.deviceAddress,
                    deviceConfiguration->GPIO_RESET,
                    deviceConfiguration->GPIO_REQ,
                    deviceConfiguration->GPIO_BOOT);
        }
        else
        {
            STECI_tracePrintSystem(TR_CLASS_STECI, " DAB_Reconfiguration : Bus name = %s, SPI mode = %d, SPI speed = %d, GPIO_CS = %d, GPIO_reset = %d, GPIO_REQ = %d, GPIO_BOOT = %d",
                    deviceConfiguration->communicationBus.spi.busName,
                    deviceConfiguration->communicationBus.spi.mode,
                    deviceConfiguration->communicationBus.spi.speed,
                    deviceConfiguration->communicationBus.spi.GPIO_CS,
                    deviceConfiguration->GPIO_RESET,
                    deviceConfiguration->GPIO_REQ,
                    deviceConfiguration->GPIO_BOOT);
        }

        // save current configuration if a revert back is needed
        vl_currentConfig = *deviceConfiguration;

        if (deviceConfiguration->communicationBusType == BusSPI)
        {
            /* Sanity check */
            if((deviceConfiguration->communicationBus.spi.speed <= SPI_SPEED_MIN) ||
               (deviceConfiguration->communicationBus.spi.speed >= SPI_SPEED_MAX))
            {
                ret = OSAL_ERROR_INVALID_PARAM;
                STECI_tracePrintError(TR_CLASS_STECI, "DAB_Reconfiguration : Return = %d, ERROR SPI frequency is out of range (Min = %d, Max = %d)",
                        ret, SPI_SPEED_MIN, SPI_SPEED_MAX);
            }
            else
            {
                if(DABDeviceConfiguration[DAB_DCOP_ID_0].communicationBus.spi.speed != deviceConfiguration->communicationBus.spi.speed)
                {
                    DABDeviceConfiguration[DAB_DCOP_ID_0].communicationBus.spi.speed = deviceConfiguration->communicationBus.spi.speed;

                    /* Set up the spi speed */
                    if(BSP_BusConfigSPIFrequency_MDR(DAB_DCOP_ID_0) != OSAL_OK)
                    {
                        // revert to prior configuration
                        DABDeviceConfiguration[DAB_DCOP_ID_0].communicationBus.spi.speed = vl_currentConfig.communicationBus.spi.speed;

                        ret = OSAL_ERROR;
                        STECI_tracePrintError(TR_CLASS_STECI, "DAB_Reconfiguration : Return = %d, ERROR in changing SPI frequency", ret);
                    }
                    else
                    {
                        // all is fine
                        STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Reconfiguration : Return = %d, SUCCESS", ret);
                    }
                }
                else
                {
                    STECI_tracePrintSystem(TR_CLASS_STECI, "DAB_Reconfiguration : Return = %d, SUCCESS", ret);
                }
            }
        }
        else
        {
            ret = OSAL_ERROR;
            STECI_tracePrintError(TR_CLASS_STECI, "DAB_Reconfiguration : Return = %d, ERROR communication bus is not a SPI bus", ret);
        }
    }
    else
    {
        ret = OSAL_ERROR;
        STECI_tracePrintError(TR_CLASS_STECI, "DAB_Reconfiguration : Return = %d, ERROR pointer is NULL (deviceConfiguration = 0x%x)", ret, deviceConfiguration);
    }
#endif //CONFIG_COMM_DRIVER_EXTERNAL

    return ret;
}

/**************************************
 *
 * DAB_Send_Message
 *
 *************************************/
/*
 * Function to send DAB message
 *
 * Parameters:
 * deviceConfiguration : pointer to device configuration.
 *
 * The function is called to send DAB message.
 *
 * Returns:
 *  OSAL_OK         - success
 *  OSAL_ERROR      - failure during message sending
 */
tS32 DAB_Send_Message(tU8 LunId, tU8 *DataToSend,
                             tU8 specific0, tU8 specific1, tU8 specific2, tU8 specific3, tU16 BytesNumber)
{
    tS32 ret = OSAL_OK;

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	/* send to MDR over LUN 0x30, using InterLayerProtocol SYNC_BYTE 0x1D */
	/* TODO use cmd->LunId, instead of ETAL_CONTROL_LUN (is it initialized?) */
	ForwardPacketToCtrlAppPort(ProtocolLayerIndexDCOP, DataToSend, BytesNumber, (tU8)0x1D, (tU8)0x30, specific0, specific1, specific2, specific3);
#else //CONFIG_COMM_DRIVER_EXTERNAL

    STECI_tracePrintComponent(TR_CLASS_STECI, "DAB_Send_Message : LunId = 0x%x, DataToSend = 0x%x, specific0 = %d, specific1 = %d, specific2 = %d, specific3 = %d, BytesNumber = %d",
            LunId, DataToSend, specific0, specific1, specific2, specific3, BytesNumber);

    if(STECI_SendMessageStart (&STECIParam, LunId, DataToSend, specific0, specific1, specific2, specific3, BytesNumber) != STECI_STATUS_SUCCESS)
    {
        ret = OSAL_ERROR;
        STECI_tracePrintError(TR_CLASS_STECI, "DAB_Send_Message : Return = %d, ERROR in STECI_SendMessageStart()", ret);
    }
    else
    {
        STECI_tracePrintComponent(TR_CLASS_STECI, "DAB_Send_Message : Return = %d, SUCCESS Message %d bytes", ret, BytesNumber);
    }
#endif //CONFIG_COMM_DRIVER_EXTERNAL

    return ret;
}


/**************************************
 *
 * DAB_Get_Response
 *
 *************************************/
/*
 * Function use to poll DAB DCOP and get response.
 *
 * Parameters:
 *
 * The function is called to get DAB response.
 *
 * Returns:
 *  OSAL_OK         - success
 *  OSAL_ERROR      - failure when getting DAB response
 */
tS32 DAB_Get_Response(tU8 *buf, tU32 max_buf, tU32 *len, tU8 *lun)
{
    tS32 ret = OSAL_OK;

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	CtrlAppMessageHeaderType head;

	ret = ProtocolLayer_FifoPop(ProtocolLayerIndexDCOP, buf, PROTOCOL_LAYER_INTERNAL_BUFFER_LEN, len, &head);
	if (ret == OSAL_OK)
	{
		if ((commMdr_wait_reset_response == TRUE) && (head.CtrlAppLunId == BROADCAST_LUN) &&
				(*len == 1) && (buf[0] == DAB_EXTERNAL_DRIVER_TYPE_RESET))
		{
			commMdr_wait_reset_response = FALSE;
			/* is Reset response from DAB_sendResetWithExternalDriver => drop response */
			/* to avoid crossing case with first MDR ping command */
			STECI_tracePrintComponent(TR_CLASS_STECI, "Received message ID 0x%.3x from DCOP on LUN 0x%.2x, %d bytes, reset response not sent to upper layer", ((((tU16)(buf)[1] & 0x03) << 8) | ((buf)[2] & 0xFF)), head.CtrlAppLunId, *len);
			*len = 0;
		}
		else if (commMdr_wait_reset_response == TRUE)
		{
			/* drop all response until reset mdr response received */
			STECI_tracePrintComponent(TR_CLASS_STECI, "Received message ID 0x%.3x from DCOP on LUN 0x%.2x, %d bytes, not sent to upper layer, waiting reset response", ((((tU16)(buf)[1] & 0x03) << 8) | ((buf)[2] & 0xFF)), head.CtrlAppLunId, *len);
			*len = 0;
		}
		else
		{
			*lun = head.CtrlAppLunId;
			if(*len != 0)
			{
				STECI_tracePrintComponent(TR_CLASS_STECI, "DAB_Get_Response : SUCCESS buf = 0x%x, max_buf = 0x%x, len = %d, LunID = 0x%x", *buf, max_buf, *len, *lun);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
				COMMON_tracePrintBufSystem(TR_CLASS_STECI, buf, *len, NULL);
#endif
			}
		}
	}
	else if (ret == OSAL_ERROR_TIMEOUT_EXPIRED)
	{
		// Queue empty
	}
	else
	{
		STECI_tracePrintError(TR_CLASS_STECI, "DAB_Get_Response : Return = %d, ERROR in ProtocolLayer_FifoPop() buf = 0x%x", ret, *buf);
	}
#else //CONFIG_COMM_DRIVER_EXTERNAL
    if((ret = STECI_fifoPop(buf, max_buf, len, lun)) != OSAL_OK)
    {
        STECI_tracePrintError(TR_CLASS_STECI, "DAB_Get_Response : Return = %d, ERROR in STECI_fifoPop() buf = 0x%x, max_buf = 0x%x", ret, *buf, max_buf);
    }
    else
    {
        if(*len != 0)
        {
            STECI_tracePrintComponent(TR_CLASS_STECI, "DAB_Get_Response : SUCCESS buf = 0x%x, max_buf = 0x%x, len = %d, LunID = 0x%x", *buf, max_buf, *len, *lun);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
            COMMON_tracePrintBufComponent(TR_CLASS_STECI, buf, *len, NULL);
#endif
        }
    }
#endif //CONFIG_COMM_DRIVER_EXTERNAL
    return ret;
}


tyCommunicationBusType DAB_GetBusType(tU32 deviceID)
{
	return DABDeviceConfiguration[deviceID].communicationBusType;
}

tU32 DAB_GetGPIOReset(tU32 deviceID)
{
	return DABDeviceConfiguration[deviceID].GPIO_RESET;
}

tU32 DAB_GetGPIOReq(tU32 deviceID)
{
    return DABDeviceConfiguration[deviceID].GPIO_REQ;
}

tU32 DAB_GetGPIOBoot(tU32 deviceID)
{
    return DABDeviceConfiguration[deviceID].GPIO_BOOT;
}

tVoid *DAB_GetCommunicationBusConfig(tU32 deviceID)
{
	if(DABDeviceConfiguration[deviceID].communicationBusType == BusSPI)
	{
		return (tVoid*)&(DABDeviceConfiguration[deviceID].communicationBus.spi);
	}
	else
	{
		return (tVoid*)&(DABDeviceConfiguration[deviceID].communicationBus.i2c);
	}
}
#endif //#if defined CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

//EOF
