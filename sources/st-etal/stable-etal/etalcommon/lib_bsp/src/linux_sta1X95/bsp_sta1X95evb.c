//!
//!  \file 		bsp_sta1X95evb.c
//!  \brief 	<i><b> ETAL BSP for STA1X95 EVB </b></i>
//!  \details   Low level drivers for the Accordo2 and Accordo5 EVB
//!  \author 	Raffaele Belardi, Roberto Allevi, Yann Hemon
//!
/*
 * This file contains the glue logic to access the Accordo2 and Accordo5 EVB
 * hardware from the ETAL
 */

#include "target_config.h"

#if (defined(CONFIG_COMM_DRIVER_EMBEDDED) || defined(CONFIG_COMM_DRIVER_EXTERNAL)) && (defined (CONFIG_BOARD_ACCORDO2) || defined (CONFIG_BOARD_ACCORDO5)) && defined (CONFIG_HOST_OS_LINUX)
#include "osal.h"

#include "bsp_sta1095evb.h"
#include "bsp_linux.h"
#endif //(CONFIG_COMM_DRIVER_EMBEDDED||CONFIG_COMM_DRIVER_EXTERNAL) && CONFIG_HOST_OS_LINUX && CONFIG_BOARD_ACCORDO2||CONFIG_BOARD_ACCORDO5

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && (defined (CONFIG_BOARD_ACCORDO2) || defined (CONFIG_BOARD_ACCORDO5)) && defined (CONFIG_HOST_OS_LINUX)

#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
//#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	#include <linux/spi/spidev.h>
//#endif

#include "common_trace.h"
#include "bsp_trace.h"

#if (defined BOOT_READ_BACK_AND_COMPARE_CMOST) && (CONFIG_TRACE_CLASS_BSP < TR_LEVEL_COMPONENT)
	#warning "BOOT_READ_BACK_AND_COMPARE_CMOST requires CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT"
	#warning "for complete output"
#endif

#define ETAL_EARLY_TUNER_SAVE_CTX_FILE	"/sys/devices/platform/soc/soc:tuner_backup/tuner_ctx"

/**************************************
 *
 * Macros
 *
 *************************************/
/*
 * Provide APIs to access a GPIO to perform debugging tasks
 * The chosen GPIO is the also used as IRQ input from the CMOST
 * so this functionality is not available if CONFIG_COMM_ENABLE_RDS_IRQ
 * is selected
 *
 * To use the functionality undefine CONFIG_COMM_ENABLE_RDS_IRQ and 
 * define BSP_INCLUDE_SIGNALLING_GPIO in the #else below
 */
#undef BSP_INCLUDE_SIGNALLING_GPIO

/*
 * INCLUDE_INTERACTIVE_DCOP_RESET_CODE
 *
 * Include code to let the test application decide if the DCOP
 * should be reset at startup or not
 * This is useful when the DCOP is loaded through the JTAG debugger
 * rather than the Flash memory, since the reset would disconnect
 * the debugger
 * To use the feature also run-time option must be set in the etaltest application
 */
#undef INCLUDE_INTERACTIVE_DCOP_RESET_CODE
//#define INCLUDE_INTERACTIVE_DCOP_RESET_CODE

/*
 * Macros in this section normally should not be changed directly
 * except when porting the driver to a new platform
 */

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/**************************************
 *
 * HDRADIO Macros
 *
 *************************************/
/*********************/
/* SPI configuration */
/*********************/
/*
 * Max millisec to wait for the BBP to process a forward LM from 
 * the HC.
 * Responses to all commands shall occur within 46 milliseconds
 */
 #define HDRADIO_MS_TO_HAVE_CMD_PROCESSED   46
/*
 * The minimum number of bytes have the LM fields information
 */
 #define HDRADIO_MIN_LMLENGTH               6

/*********************/
/* I2C configuration */
/*********************/

/*
 * Max millisec to wait for the I2C FIFO filled of the minimum amount of bytes.
 * Some or all of a response LM in the FIFO tells that BBP processed a forward 
 * LM from the HC.
 * Responses to all commands shall occur within 46 milliseconds
 */
 #define HDRADIO_MS_TO_HAVE_FIFO_MIN_FILLED    46
/*
 * Millisec to wait for the I2C FIFO fully filled after that the minimum 
 * amount of bytes is already in
 */
 #define HDRADIO_MS_TO_HAVE_FIFO_FILLED        5
/*
 * The I2C FIFO depth
 */
 #define HDRADIO_FIFO_SIZE_IN_BYTES            64
/*
 * The minimum number of bytes available in the I2C FIFO to 
 * have the LM fields information
 */
 #define HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH   13

/*
 * Byte value of the header indicating the beginning of the logical message. 
 * The full value is 0xA5A5
 */
#define HDRADIO_LM_BYTE_HEADER              0xA5
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

/*
 * Max number of concurrently supported tuners
 * on the I2C CMOST supports are at most 3 different addresses
 * on the SPI we support at most 3 different chip selects
 */
#define BSP_MAX_TUNER_PER_BUS               3

#define BSP_PRINTBUF_HEX_NS  (tU32)100
#define BSP_PRINTBUF_SIZE_NS (BSP_PRINTBUF_HEX_NS * 2 + 6)

/**************************************
 *
 * Local types
 *
 *************************************/
#ifdef CONFIG_ETAL_SUPPORT_CMOST
typedef struct
{
	tBool isValid;
	tS32  CMOST_LinuxDevice_fd;
	tS32  CMOST_Irq_fd;
	tU32  CMOST_device_Id;
	tS32  CMOST_cs_id; // for SPI only
} CMOSTAddressTy;
#endif

/**************************************
 *
 * Local variables
 *
 *************************************/
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
static tS32 MDR_LinuxDevice_fd;
static tS32 MDR_LinuxSPI_CS_fd;
static tS32 MDR_LinuxSPI_REQ_fd;
#ifdef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
static tS32 MDR_LinuxSPI_BOOT_fd;
#endif //CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_CMOST
static CMOSTAddressTy BSP_LinuxDevice[BSP_MAX_TUNER_PER_BUS];
#ifdef BSP_INCLUDE_SIGNALLING_GPIO
static tS32 BSP_LinuxSignalling_fd;
#endif //BSP_INCLUDE_SIGNALLING_GPIO
#endif //CONFIG_ETAL_SUPPORT_CMOST

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
static tS32 HDRADIO_I2C_LinuxDevice_fd;
static tS32 HDRADIO_SPI_LinuxDevice_fd;
static tS32 HDRADIO_LinuxSPI_CS_fd;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

static tS32 BSP_DeviceAddressToFileDescriptor(tU32 deviceID);

static tSInt BSP_DeviceAddressSetCs(tU32 deviceID, tS32 vI_CS_Id);
static tS32 BSP_DeviceAddressToChipSelectDescriptor(tU32 deviceID);
static tSInt BSP_BusConfigSPIFrequency_CMOST(tU32 deviceID); 

//static tSInt BSP_BusConfigSPIMode_CMOST(tU32 deviceID);


static tS32 BSP_DeviceAddressToIRQFileDescriptor(tU32 deviceID);
static tSInt BSP_DeviceAddressSetIRQFileDescriptor(tU32 deviceID, tS32 vI_IRQ_fd);
tVoid BSP_PrintBufNoSpace(tS32 level, tBool useI2C, tUChar *buf, tS32 len, BSPPrintOperationTy op);


/**************************************
 * Function prototypes
 *************************************/

#if ((defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)) || \
	(defined BOOT_READ_BACK_AND_COMPARE_CMOST)
 /**************************************
 *
 * BSP_PrintBufNoSpace
 *
 *************************************/
/*
 * Print a buffer content, one hex after the other
 */
tVoid BSP_PrintBufNoSpace(tS32 level, tBool useI2C, tUChar *buf, tS32 len, BSPPrintOperationTy op)
{
	static tChar sbuf[BSP_PRINTBUF_SIZE_NS];
	tU32 i;

	if (op == BSP_WRITE_OPERATION)
	{
		sprintf(sbuf, "W ");
	}
	else
	{
		sprintf(sbuf, "R ");
	}
	if (useI2C)
	{
		sprintf(sbuf + 2, "I2C ");
	}
	else
	{
		sprintf(sbuf + 2, "SPI ");
	}

	if (len > 0)
	{
		for (i = 0; (i < (tU32) len) && (i < BSP_PRINTBUF_HEX_NS); i++)
		{
			sprintf(sbuf + (i * 2) + 6, "%.2x", buf[i]);
		}
		COMMON_tracePrint(level, TR_CLASS_BSP, sbuf);
	}
	else if (len == 0)
	{
		COMMON_tracePrint(level, TR_CLASS_BSP, "empty");
	}
	else
	{
		COMMON_tracePrint(level, TR_CLASS_BSP, "error");
	}
	if ((tU32) len > BSP_PRINTBUF_HEX_NS)
	{
		COMMON_tracePrint(level, TR_CLASS_BSP, "Output truncated to %d bytes", BSP_PRINTBUF_HEX_NS);
	}
}

#endif /*((defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)) ||
(defined BOOT_READ_BACK_AND_COMPARE_CMOST)*/

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
/**************************************
 *
 * BSP_DeviceReset_MDR
 *
 *************************************/
/*
 * Manages the MDR reset on the Accordo EVB, where the MDR's RSTN line
 * is connected to MDR_ACCORDO2_RESET_GPIO
 * RSTN is active low
 */
tSInt BSP_DeviceReset_MDR(tU32 deviceID)
{
	tS32 fd;
    char str_GPIO_RESET[11];

    BSP_tracePrintComponent(TR_CLASS_BSP, "MDR reset");

    sprintf(str_GPIO_RESET, "%u", DAB_GetGPIOReset(deviceID));

	fd = BSP_Linux_OpenGPIO(str_GPIO_RESET, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
	if (fd < 0)
	{
		return OSAL_ERROR;
	}

	/*
	 * Assert the reset line
	 */
	BSP_Linux_WriteGPIO(fd, FALSE);
	/*
	 * Let the signal settle
	 */
	OSAL_s32ThreadWait(5);
	/*
	 * Bring out of reset
	 */
	BSP_Linux_WriteGPIO(fd, TRUE);

	BSP_Linux_CloseGPIO(fd);

	BSP_tracePrintComponent(TR_CLASS_BSP, "MDR reset complete");
	return OSAL_OK;
}

/**************************************
 *
 * BSP_SteciReadREQ_MDR
 *
 *************************************/
/*
 * Returns the current value of the STECI's REQ line
 */
tU8 BSP_SteciReadREQ_MDR(tVoid)
{
	tBool val;

	val = BSP_Linux_ReadGPIO(MDR_LinuxSPI_REQ_fd);
	return (tU8)((val) ? 1 : 0);
}

/**************************************
 *
 * BSP_SteciSetCS_MDR
 *
 *************************************/
/*
 * Sets the value of the STECI's CS line
 */
tVoid BSP_SteciSetCS_MDR(tBool value)
{
	if (MDR_LinuxSPI_CS_fd < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "Invalid file descriptor in BSP_SteciSetCS_MDR");
		return;
	}
	BSP_tracePrintComponent(TR_CLASS_BSP, "CS set %d", value);
	BSP_Linux_WriteGPIO(MDR_LinuxSPI_CS_fd, value);
}

/**************************************
 *
 * BSP_SteciSetBOOT_MDR
 *
 *************************************/
/*
 * Sets the value of the STECI's BOOT line
 */
tVoid BSP_SteciSetBOOT_MDR(tBool value)
{

#ifdef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO

	// Open the SPI boot GPIO if needed
    if (MDR_LinuxSPI_BOOT_fd < 0)
    {	
    	BSP_tracePrintError(TR_CLASS_BSP, "BOOT gpio not opened");
    }
	else
	{
		BSP_tracePrintComponent(TR_CLASS_BSP, "BOOT set %d", value);
		BSP_Linux_WriteGPIO(MDR_LinuxSPI_BOOT_fd, value);
	}
#endif /* CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO */
}

/**************************************
 *
 * BSP_TransferSpi_MDR
 *
 *************************************/
/*
 * Performs an SPI read/write (full-duplex) operation; both read and write
 * buffers are assumed to be <len> bytes long.
 */
tVoid BSP_TransferSpi_MDR(tU8 *buf_wr, tU8 *buf_rd, tU32 len)
{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, 0, buf_wr, len, BSP_WRITE_OPERATION);
#endif //(defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_Linux_TransferSpi(MDR_LinuxDevice_fd, buf_wr, buf_rd, len, true);
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, 0, buf_rd, len, BSP_READ_OPERATION);
#endif //(defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
}

/**************************************
 *
 * BSP_BusConfigSPIFrequency_MDR
 *
 *************************************/
/*
 * Change the SPI communication bus frequency
 * <deviceID> - device ID.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfigSPIFrequency_MDR(tU32 deviceID)
{
    if (ioctl(MDR_LinuxDevice_fd, SPI_IOC_WR_MAX_SPEED_HZ, &(((tySpiCommunicationBus*)DAB_GetCommunicationBusConfig(deviceID))->speed)) < 0)
    {
        BSP_tracePrintError(TR_CLASS_BSP, "configuring the SPI frequency");
        (LINT_IGNORE_RET)close(MDR_LinuxDevice_fd);
        MDR_LinuxDevice_fd = -1;
        return OSAL_ERROR;
    }
    return OSAL_OK;
}


/**************************************
 *
 * BSP_BusConfig_MDR
 *
 *************************************/
/*
 * Prepares the application for read/write access
 * to the MDR through SPI bus
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
static tSInt BSP_BusConfig_MDR(tU32 deviceID)
{
    char strGPIO_CS[11], strGPIO_REQ[11], strGPIO_BOOT[11];

    sprintf(strGPIO_CS, "%u", ((tySpiCommunicationBus*)DAB_GetCommunicationBusConfig(deviceID))->GPIO_CS);
    sprintf(strGPIO_REQ, "%u", DAB_GetGPIOReq(deviceID));
    sprintf(strGPIO_BOOT, "%u", DAB_GetGPIOBoot(deviceID));

    if(DAB_GetBusType(deviceID) == BusSPI)
    {
        /* initialize in case of early exit from the function due to error */
        MDR_LinuxDevice_fd = -1;
        MDR_LinuxSPI_CS_fd = -1;
        MDR_LinuxSPI_REQ_fd = -1;
#ifdef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
        MDR_LinuxSPI_BOOT_fd = -1;
#endif //CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO

        MDR_LinuxDevice_fd = BSP_Linux_OpenDevice(((tySpiCommunicationBus*)DAB_GetCommunicationBusConfig(deviceID))->busName);
        if (MDR_LinuxDevice_fd < 0)
        {
            return OSAL_ERROR;
        }

        /* Set up the SPI speed */
        if(BSP_BusConfigSPIFrequency_MDR(deviceID) != OSAL_OK)
        {
            return OSAL_ERROR;
        }

        MDR_LinuxSPI_CS_fd = BSP_Linux_OpenGPIO(strGPIO_CS, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
        if (MDR_LinuxSPI_CS_fd < 0)
        {
            BSP_tracePrintError(TR_CLASS_BSP, "configuring the SPI CS");
            return OSAL_ERROR;
        }

        MDR_LinuxSPI_REQ_fd = BSP_Linux_OpenGPIO(strGPIO_REQ, GPIO_OPEN_READ, GPIO_IRQ_NONE, NULL);
        if (MDR_LinuxSPI_REQ_fd < 0)
        {
            BSP_tracePrintError(TR_CLASS_BSP, "configuring the SPI REQ");
            return OSAL_ERROR;
        }

#ifdef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
	   MDR_LinuxSPI_BOOT_fd = BSP_Linux_OpenGPIO(strGPIO_BOOT, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);

       if (MDR_LinuxSPI_BOOT_fd < 0)
        {
            BSP_tracePrintError(TR_CLASS_BSP, "configuring the SPI BOOT");
            return OSAL_ERROR;
        }
#endif //CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
    }
    else
    {
        return OSAL_ERROR;
    }
	return OSAL_OK;
}

/**************************************
 *
 * BSP_DeviceInit_MDR
 *
 *************************************/
tSInt BSP_DeviceInit_MDR(tU32 deviceID)
{
#ifdef INCLUDE_INTERACTIVE_DCOP_RESET_CODE
	extern tBool etalTestOptionSkipDCOPReset;
#endif //INCLUDE_INTERACTIVE_DCOP_RESET_CODE

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_SYSTEM_MIN)
    BSP_tracePrintSysmin(TR_CLASS_BSP, "DCOP DAB init");
#endif

	if (BSP_BusConfig_MDR(deviceID) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	BSP_tracePrintComponent(TR_CLASS_BSP, "DCOP DAB init complete");

	return OSAL_OK;
}

/**************************************
 *
 * BSP_DeviceDeinit_MDR
 *
 *************************************/
tVoid BSP_DeviceDeinit_MDR(tVoid)
{
	(LINT_IGNORE_RET) BSP_Linux_CloseDevice(MDR_LinuxDevice_fd);
	(LINT_IGNORE_RET) BSP_Linux_CloseGPIO(MDR_LinuxSPI_CS_fd);
	(LINT_IGNORE_RET) BSP_Linux_CloseGPIO(MDR_LinuxSPI_REQ_fd);
#ifdef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
	(LINT_IGNORE_RET) BSP_Linux_CloseGPIO(MDR_LinuxSPI_BOOT_fd);
#endif //CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO

	MDR_LinuxDevice_fd = -1;
	MDR_LinuxSPI_CS_fd = -1;
	MDR_LinuxSPI_REQ_fd = -1;
#ifdef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
	MDR_LinuxSPI_BOOT_fd = -1;
#endif //CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
}

#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_CMOST
/**************************************
 *
 * BSP_Reset_CMOST
 *
 *************************************/
/*
 * Manages the CMOST reset on the Accordo EVB, where the CMOST's RSTN line
 * is connected to CMOST_ACCORDO2_RESET_GPIO
 * RSTN is active low
 *
 * Parameters:
 *  reset_me - if TRUE puts the CMOST in reset
 *             if FALSE pulls the CMOST out of reset
 *
 * Return:
 *  OSAL_OK    - no error
 *  OSAL_ERROR - error accessing the device
 *
 */
static tSInt BSP_Reset_CMOST(tU32 deviceID)
{
	tS32 fd;
	tS32 fd_irq = -1;
	char str_GPIO_RESET[11];
	char str_GPIO_IRQ[11];
	tSInt ret = OSAL_OK;

	sprintf(str_GPIO_RESET, "%u", TUNERDRIVER_GetGPIOReset(deviceID));
	sprintf(str_GPIO_IRQ, "%u", TUNERDRIVER_GetGPIOIrq(deviceID));
	
	fd = BSP_Linux_OpenGPIO(str_GPIO_RESET, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
	if (fd < 0)
	{
		ret = OSAL_ERROR;
		goto exit;
	}

	// for SPI case : set the PIN to SPI configuration
	// IRQ to High && MISO to HIGH
	//
	if (TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
		fd_irq = BSP_Linux_OpenGPIO(str_GPIO_IRQ, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
		
		if (fd_irq < 0)
		{
			ret = OSAL_ERROR;
			goto exit;
		}

		// force the IRQ HIGH
		
		BSP_Linux_WriteGPIO(fd_irq, TRUE);
		OSAL_s32ThreadWait(5);
	}

	// force the reset to high as a start
	/*
	 * Perform the reset
	 * Negate because the reset line is active low
	 */
	BSP_Linux_WriteGPIO(fd, TRUE);
		
	/*
	 * let the signal settle
	 */
	OSAL_s32ThreadWait(20);

	/*
	* Switch to Low for the Reset
	 * Perform the reset
	 * Negate because the reset line is active low
	 */
	BSP_Linux_WriteGPIO(fd, FALSE);

	/*
	 * let the signal settle
	 */
	OSAL_s32ThreadWait(20);

	/*
	 * Set back the signal to High
	 * Negate because the reset line is active low
	 */
	BSP_Linux_WriteGPIO(fd, TRUE);

	/*
	 * let the signal settle
	 */
	OSAL_s32ThreadWait(20);

	// close the GPIOs
	BSP_Linux_CloseGPIO(fd);

	// for SPI case : free the fd
	// set back to default low value
	// IRQ to High && MISO to HIGH
	//
	if (TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
		// force the IRQ HIGH
		
		BSP_Linux_WriteGPIO(fd_irq, FALSE);

		// close the GPIOs
		BSP_Linux_CloseGPIO(fd_irq);
			
	}
	

	/*
	 * let the signal settle
	 */
	OSAL_s32ThreadWait(5);

exit:
	return ret;
}

/**************************************
 *
 * BSP_RDSInterruptInit_CMOST
 *
 *************************************/
static tSInt BSP_RDSInterruptInit_CMOST(tU32 deviceID)
{
	tS32 irq_fd;
	char str_GPIO_IRQ[11];

	sprintf(str_GPIO_IRQ, "%u", TUNERDRIVER_GetGPIOIrq(deviceID));

	irq_fd = BSP_Linux_OpenGPIO(str_GPIO_IRQ, GPIO_OPEN_READ, GPIO_IRQ_FALLING, TUNERDRIVER_GetIRQCallbackFunction(deviceID));
	if (irq_fd < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "configuring the CMOST IRQ");
		return OSAL_ERROR;
	}
	else
	{
		if(BSP_DeviceAddressSetIRQFileDescriptor(deviceID, irq_fd) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	return OSAL_OK;
}

/**************************************
 *
 * BSP_RDSInterruptDeinit_CMOST
 *
 *************************************/
static tVoid BSP_RDSInterruptDeinit_CMOST(tU32 deviceID)
{
	BSP_Linux_CloseGPIO(BSP_DeviceAddressToIRQFileDescriptor(deviceID));
	BSP_DeviceAddressSetIRQFileDescriptor(deviceID, -1);
}

/*
 * Manage a simple associative array where array locations
 * containing Linux file desriptors are indexex by <deviceID>
 *
 * Update the CS information of the device
 * location
 */
static tSInt BSP_DeviceAddressSetCs(tU32 deviceID, tS32 vI_CS_Id)
{
	tU32 i;
	CMOSTAddressTy *addrp;
	tSInt ret;

	if (TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
		for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
		{
				addrp = &BSP_LinuxDevice[i];
				if (addrp->isValid && (addrp->CMOST_device_Id == deviceID))
			{
				addrp->CMOST_cs_id = vI_CS_Id;
				
				ret = OSAL_OK;
				goto exit;
			}
		}

		/* device not found ! */
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
		goto exit;
	}
	else
	{
		ret = OSAL_ERROR;
		goto exit;
	}

exit:
	return ret;
}

#if 0
/************************************** 
* BSP_BusConfigSPIMode_CMOST * 
*************************************/
/* 
* Change the SPI communication bus mode 
* <mode> Indicate the SPI mode. 
* 
* Returns: 
*  OSAL_OK    - success 
*  OSAL_ERROR - error accessing the kernel driver interface */

static tSInt BSP_BusConfigSPIMode_CMOST(tU32 deviceID)
{	
	tS32 vl_fd;
	tSInt vl_res = OSAL_OK;
	
	vl_fd = BSP_DeviceAddressToFileDescriptor(deviceID);
	if (vl_fd < 0)
	{
		
		BSP_tracePrintError(TR_CLASS_BSP, "BSP_BusConfigSPIFrequency_CMOST : invalid device ID %d", deviceID);
		vl_res = OSAL_ERROR;
	}
	else
	{
	    if (ioctl(vl_fd, SPI_IOC_WR_MODE, &(((tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->mode)) < 0)
	    {
	        BSP_tracePrintError(TR_CLASS_BSP, "BSP_BusConfigSPIMode_CMOST : configuring the SPI mode");

	        vl_res = OSAL_ERROR;
	    }
	}

	printf("BSP_BusConfigSPIMode_CMOST :  deviceID = %d, vl_res = %d, mode = %d \n", deviceID, vl_res, (((tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->mode));
	
    return vl_res;
}
#endif 

/**************************************
 *
 * BSP_BusConfigSPIFrequency_CMOST
 *
 *************************************/
/*
 * Change the SPI communication bus frequency
 * <deviceID> - device ID.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
static tSInt BSP_BusConfigSPIFrequency_CMOST(tU32 deviceID)
{
	tS32 vl_fd;
	tSInt vl_res = OSAL_OK;
	
	vl_fd = BSP_DeviceAddressToFileDescriptor(deviceID);
	if (vl_fd < 0)
	{
		
		BSP_tracePrintError(TR_CLASS_BSP, "BSP_BusConfigSPIFrequency_CMOST : invalid device ID %d", deviceID);
		vl_res = OSAL_ERROR;
	}
	else
	{
	    if (ioctl(vl_fd, SPI_IOC_WR_MAX_SPEED_HZ, &(((tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->speed)) < 0)
	    {
	        BSP_tracePrintError(TR_CLASS_BSP, "BSP_BusConfigSPIFrequency_CMOST : configuring the SPI frequency");

	        vl_res = OSAL_ERROR;
	    }
	}

	BSP_tracePrintComponent(TR_CLASS_BSP,"BSP_BusConfigSPIFrequency_CMOST :  deviceID = %d, vl_res = %d, speed = %d \n", deviceID, vl_res, (((tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->speed));
	
    return vl_res;
}

/*
 * Manage a simple associative array where array locations
 * containing Linux file desriptors are indexex by <device_address>
 *
 * Returns the file descriptor addressed by <device_address> or,
 * if not yet present, -1
 */
static tS32 BSP_DeviceAddressToChipSelectDescriptor(tU32 deviceID)
{
	tU32 i;
	CMOSTAddressTy *addrp;
	tS32 ret = -1;

	if(TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
		for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
		{
				addrp = &BSP_LinuxDevice[i];
				if (addrp->isValid && (addrp->CMOST_device_Id == deviceID))
			{
				ret = addrp->CMOST_cs_id;
				goto exit;
			}
		}
		ret = -1;
	}
	else
	{
		ret = -1;
	}

exit:
	return ret;
}



/**************************************
 *
 * BSP_DeviceAddressToIRQFileDescriptor
 *
 *************************************/
/*
 * Manage a simple associative array where array locations
 * containing Linux file desriptors are indexex by <deviceID>
 *
 * Returns the file descriptor addressed by <deviceID> or,
 * if not yet present, -1
 */
static tS32 BSP_DeviceAddressToIRQFileDescriptor(tU32 deviceID)
{
	tU32 i;
	CMOSTAddressTy *addrp;

	for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
	{
		addrp = &BSP_LinuxDevice[i];
		if (addrp->isValid && (addrp->CMOST_device_Id == deviceID))
		{
			return addrp->CMOST_Irq_fd;
		}
	}
	return -1;
}


/**************************************
 *
 * BSP_DeviceAddressSetIRQFileDescriptor
 *
 *************************************/
/*
 * Manage a simple associative array where array locations
 * containing Linux file desriptors are indexex by <deviceID>
 *
 * Update the CS information of the device
 * location
 */
static tSInt BSP_DeviceAddressSetIRQFileDescriptor(tU32 deviceID, tS32 vI_IRQ_fd)
{
	tU32 i;
	CMOSTAddressTy *addrp;

	for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
	{
		addrp = &BSP_LinuxDevice[i];
		if (addrp->isValid && (addrp->CMOST_device_Id == deviceID))
		{
			addrp->CMOST_Irq_fd = vI_IRQ_fd;

			return OSAL_OK;
		}
	}
	/* device not found ! */
	ASSERT_ON_DEBUGGING(0);
	return OSAL_ERROR;
}


/**************************************
 *
 * BSP_WaitForRDSInterrupt_CMOST
 *
 *************************************/
tVoid BSP_WaitForRDSInterrupt_CMOST(tU32 deviceID)
{
	ASSERT_ON_DEBUGGING(BSP_DeviceAddressToIRQFileDescriptor(deviceID) >= 0);
	BSP_Linux_WaitForInterrupt(BSP_DeviceAddressToIRQFileDescriptor(deviceID));
}
#ifdef BSP_INCLUDE_SIGNALLING_GPIO
/**************************************
 *
 * BSP_initSignallingGPIO
 *
 *************************************/
static tSInt BSP_initSignallingGPIO(tU32 deviceID)
{
	char str_GPIO_IRQ[11];

	sprintf(str_GPIO_IRQ, "%u", TUNERDRIVER_GetGPIOIrq(deviceID));

	BSP_LinuxSignalling_fd = BSP_Linux_OpenGPIO(str_GPIO_IRQ, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
	if (BSP_LinuxSignalling_fd < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "configuring the Signalling GPIO");
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/**************************************
 *
 * BSP_deinitSignallingGPIO
 *
 *************************************/
static tVoid BSP_deinitSignallingGPIO(tVoid)
{
	BSP_Linux_CloseGPIO(BSP_LinuxSignalling_fd);
	BSP_LinuxSignalling_fd = -1;
}

/**************************************
 *
 * BSP_setSignallingGPIO
 *
 *************************************/
tVoid BSP_setSignallingGPIO(tBool val)
{
	ASSERT_ON_DEBUGGING(BSP_LinuxSignalling_fd >= 0);

	BSP_Linux_WriteGPIO(BSP_LinuxSignalling_fd, val);
}
#endif //BSP_INCLUDE_SIGNALLING_GPIO

/**************************************
 *
 * BSP_DeviceAddressToFileDescriptor
 *
 *************************************/
/*
 * Manage a simple associative array where array locations
 * containing Linux file descriptors are indexex by <deviceID>
 *
 * Returns the file descriptor addressed by <deviceID> or,
 * if not yet present, -1
 */
static tS32 BSP_DeviceAddressToFileDescriptor(tU32 deviceID)
{
	tU32 i;
	CMOSTAddressTy *addrp;

	for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
	{
		addrp = &BSP_LinuxDevice[i];
		if (addrp->isValid && (addrp->CMOST_device_Id == deviceID))
		{
			return addrp->CMOST_LinuxDevice_fd;
		}
	}

	return -1;
}


/**************************************
 *
 * BSP_DeviceAddressAdd
 *
 *************************************/
/*
 * Manage a simple associative array where array locations
 * containing Linux file descriptors are indexed by <device_address>
 *
 * Returns the pointer to the first free file descriptor
 * location
 */
static tS32 *BSP_DeviceAddressAdd(tU32 deviceID)
{
	tU32 i;
	CMOSTAddressTy *addrp;

	for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
	{
		addrp = &BSP_LinuxDevice[i];
		if (!addrp->isValid)
		{
			addrp->isValid = TRUE;
			addrp->CMOST_device_Id = deviceID;
			// for now set the CS as invalid.
			addrp->CMOST_cs_id = -1;
			return &addrp->CMOST_LinuxDevice_fd;
		}
		else
		{
			if (deviceID == addrp->CMOST_device_Id)
			{
				// already present ... 
				return &addrp->CMOST_LinuxDevice_fd;
			}
		}
	}
	ASSERT_ON_DEBUGGING(0);
	return NULL;
}

/**************************************
 *
 * BSP_DeviceAddressRelease
 *
 *************************************/
static tVoid BSP_DeviceAddressRelease(tU32 deviceID)
{
	tU32 i;
	CMOSTAddressTy *addrp;

	for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
	{
		addrp = &BSP_LinuxDevice[i];


		if (addrp->isValid && (addrp->CMOST_device_Id == deviceID))
		{
			OSAL_pvMemorySet((tVoid *)addrp, 0x00, sizeof(CMOSTAddressTy));
			addrp->CMOST_Irq_fd = -1;
			addrp->CMOST_LinuxDevice_fd = -1;
			return;
		}
	}
	ASSERT_ON_DEBUGGING(0);
}


/**************************************
 *
 * BSP_BusConfig_CMOST
 *
 *************************************/
/*
 * Prepares the application for read/write access
 * to the CMOST through I2C or SPI bus
 *
 * Returns:
 *  the new file descriptor or
 *  -1 in case of error
 */
tS32 BSP_BusConfig_CMOST(tU32 deviceID)
{
	tVoid *addr;
	tS32 *fdp;
	tS32 ret = OSAL_OK;
	tySpiCommunicationBus *spiBusConfig = NULL;
	tS32 vl_CS_fd = -1;
	char str_GPIO_CS[11];
//	tU8 u8FakeWrForClkLevel = 0xFF;


	fdp = BSP_DeviceAddressAdd(deviceID);
	if (fdp == NULL)
	{
		 ret = OSAL_ERROR;
		goto exit;
	}

	if(TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
		spiBusConfig = (tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID);

		/* set ssp configration */
		// for TUNER : the SPI should be 11
		if (SPI_CPHA0_CPOL0 == spiBusConfig->mode)
		{
			// warning this is an error !!
			BSP_tracePrintError(TR_CLASS_BSP, "wrong SPI configuration for CMOST, SPI_CPHA0_CPOL0 not supported");
			ret = OSAL_ERROR;
			goto exit;
		}
		

		*fdp = BSP_Linux_OpenDevice(spiBusConfig->busName);

		if (*fdp < 0)
		{
			BSP_DeviceAddressRelease(deviceID);
			ret = -1;
			goto exit;
		}
		
		ret = (tS32) fdp;
		


		// configure the SPI speed....
		//
		// The clock line is born by A2 with a level that might be not compatible with the mode selected.		
		// The fake write w/o FS driving is to adjust the clock line level		
		//write(*fdp, &u8FakeWrForClkLevel, 0);		/* Open the GPIO for SPI control CS. */

		(void) BSP_BusConfigSPIFrequency_CMOST(deviceID);
		
		/* Open the GPIO for SPI control CS. */
	
		if(spiBusConfig->GPIO_CS == ETAL_CS_TRUE_SPI)
		{
#ifndef CONFIG_BOARD_ACCORDO2_CUSTOM2
			/* Set the function as true SPI1 : Alternate A Function  */
			// EPR TO DO 
			
#endif // CONFIG_BOARD_ACCORDO2_CUSTOM2

			// CS not controlled : mark it as invalid
			vl_CS_fd = -1;
		}	
		else
		{
				sprintf(str_GPIO_CS, "%u", spiBusConfig->GPIO_CS);

				// on SPI 0 : do not control the CS
				vl_CS_fd  = BSP_Linux_OpenGPIO(str_GPIO_CS, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);

				if (vl_CS_fd < 0)
			 	{
				 	BSP_tracePrintError(TR_CLASS_BSP, "configuring the SPI CS for CMOST");
				 	ret = OSAL_ERROR;
				 	goto exit;
			 	}
		}
			 

		if (OSAL_OK != BSP_DeviceAddressSetCs(deviceID, vl_CS_fd))
	 	{
		 	BSP_tracePrintError(TR_CLASS_BSP, "configuring the SPI CS for CMOST");
		 	ret = OSAL_ERROR;
		 	goto exit;
	 	}
		else
		{
			/* Nothing to do */
		}	
		
		/*
		 * On the modified A2 the SPI0 is routed to the CMOST using the same pins
		 * normally connected to the I2C of the A2, so we need to put them in
		 * input mode to avoid electrical clashes
		 *
		if (BSP_Linux_OpenGPIO("34", GPIO_OPEN_READ, GPIO_IRQ_NONE, NULL) < 0)
		{
			return -1;
		}
		if (BSP_Linux_OpenGPIO("35", GPIO_OPEN_READ, GPIO_IRQ_NONE, NULL) < 0)
		{
			return -1;
		}
		*/
	}
	else
	{



		*fdp = BSP_Linux_OpenDevice(((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->busName);

		if (*fdp < 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "BSP_BusConfig_CMOST , Error opening the I2C device");
			BSP_DeviceAddressRelease(deviceID);
			return -1;
		}
		ret = (tS32) fdp;
		
		/*
		 * Set up the device address
		 * Linux kernel expects the address on 7 bits
		 */
		addr = (tVoid *)(((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress >> 1);

		if (ioctl(*fdp, I2C_SLAVE, addr) < 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "configuring the I2C device");
			(LINT_IGNORE_RET)close(*fdp);
			BSP_DeviceAddressRelease(deviceID);
			return -1;
		}
	}

	if (BSP_RDSInterruptInit_CMOST(deviceID) != OSAL_OK)
	{
		return -1;
	}
#ifdef BSP_INCLUDE_SIGNALLING_GPIO
	/*
	 * Not really CMOST specific, but can only be performed
	 * if the CMOST does not use the GPIO for IRQ
	 */
	if (BSP_initSignallingGPIO() != OSAL_OK)
	{
		return -1;
	}
#endif //BSP_INCLUDE_SIGNALLING_GPIO

exit:
	return ret;
}

/**************************************
 *
 * BSP_BusUnconfig_CMOST
 *
 *************************************/
/*
 * Closes the Operating System resources
 * used to access the CMOST device
 *
 * Returns:
 *  0  no error
 *  -1 in case of error
 */
tS32 BSP_BusUnconfig_CMOST(tU32 deviceID)
{
	tS32 ret = 0;
	tS32 fd;
	tS32 vl_CS_fd = -1;

	fd = BSP_DeviceAddressToFileDescriptor(deviceID);
	if (fd < 0)
	{
		ret = -1;
	}
	else
	{
		if (BSP_Linux_CloseDevice(fd) != 0)
		{
			ret = -1;
		}
	}

	// We should also close the CS if SPI
	vl_CS_fd = BSP_DeviceAddressToChipSelectDescriptor(deviceID);
	if (-1 != vl_CS_fd)
	{
		// close the GPIO
		BSP_Linux_CloseGPIO(vl_CS_fd);
	}

	
	BSP_RDSInterruptDeinit_CMOST(deviceID);
	BSP_DeviceAddressRelease(deviceID);
#ifdef BSP_INCLUDE_SIGNALLING_GPIO
	BSP_deinitSignallingGPIO();
#endif //BSP_INCLUDE_SIGNALLING_GPIO

	return ret;
}

/**************************************
 *
 * BSP_DeviceReset_CMOST
 *
 *************************************/
/*
 * Give a reset pulse to the CMOST
 */
tSInt BSP_DeviceReset_CMOST(tU32 deviceID)
{
	/* For now, device_adresse & I2C or SPI not used */
	if (BSP_Reset_CMOST(deviceID) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/**************************************
 *
 * BSP_Write_CMOST
 *
 *************************************/
/*
 * Write <len> bytes from buffer <buf> to the
 * I2C or SPI bus connected to the CMOST.
 *
 * Returns:
 * the number of bytes read, or
 * -1 in case of error
 */
tS32 BSP_Write_CMOST(tU32 deviceID, tU8* buf, tU32 len)
{
	tS32 ret;
	tS32 fd;

	fd = BSP_DeviceAddressToFileDescriptor(deviceID);
	if (fd < 0)
	{
		/*
		 * The kernel was not yet configured for use of the <device_address>
		 * try to configure it.
		 * This is needed in the CONFIG_APP_TUNERDRIVER_LIBRARY build,
		 * where there is no explicit bus configuration.
		 */
		if ((fd = BSP_BusConfig_CMOST(deviceID)) < 0)
		{
			ASSERT_ON_DEBUGGING(0);
			return -1;
		}
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	{
		tU32 i;
		/*
		 * Don't print the I2C or SPI header
		 */

		if (TUNERDRIVER_GetBusType(deviceID) == BusI2C)
		{
			BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, TRUE, buf, 3, BSP_WRITE_OPERATION);
			for (i = 3; i < len; i += 4)
			{
				BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, TRUE, buf + i, 4, BSP_WRITE_OPERATION);
			}
		}
		else
		{
			BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, FALSE, buf, 4, BSP_WRITE_OPERATION);
			for (i = 4; i < len; i += 4)
			{
				BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, FALSE, buf + i, 4, BSP_WRITE_OPERATION);
			}
		}
	}
#endif //(defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	if (TUNERDRIVER_GetBusType(deviceID) == BusSPI)
		{
		ret = BSP_Write_CMOST_SPI(deviceID, buf, len);
		}
	else
		{

		ret = (tS32)write(fd, buf, (size_t)len);
		}

	if (ret < 0)
	{

		BSP_tracePrintError(TR_CLASS_BSP, "writing data to CMOST");
	}
	else if (ret < (tS32)len)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "truncated write to CMOST");
		return -1;
	}
	return ret;
}

/**************************************
 *
 * BSP_Read_CMOST
 *
 *************************************/
/*
 * Read <len> bytes from the CMOST into
 * buffer <buf>.
 * <buf> must be allocated by the caller.
 *
 * Returns:
 * the number of bytes read, or
 * -1 in case of error
 */
tS32 BSP_Read_CMOST(tU32 deviceID, tU8* buf, tU32 len)
{
	tS32 ret;
	tS32 fd;

	fd = BSP_DeviceAddressToFileDescriptor(deviceID);
	if (fd < 0)
	{
		return -1;
	}

	if(TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{

		//		spiBusConfig = (tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID);
			// Error not used in LINUX/TK 
			// the read only function is not used in SPI configuration in that case
			
			//(void)BSP_Write_CMOST_SPI(deviceID, buf, len);
			BSP_tracePrintError(TR_CLASS_BSP, "BSP_Read_CMOST not applicable to SPI");
			len = 0;
			ret = -1;
	}
	else
	{
		ret = (tS32)read(fd, buf, (size_t)len);
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	if(TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
		BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, FALSE, buf, len, BSP_READ_OPERATION);
	}
	else
	{
		BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, TRUE, buf, len, BSP_READ_OPERATION);
	}
#endif //(defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)

	if (ret < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "reading data from CMOST");
	}
	else if (ret < (tS32)len)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "truncated read from CMOST");
		return -1;
	}
	return ret;
}

/**************************************
 *
 * BSP_TransferSpi_CMOST
 *
 *************************************/
/*
 * Performs an SPI read/write (full-duplex) operation; both read and write
 * buffers are assumed to be <len> bytes long.
 */
tVoid BSP_TransferSpi_CMOST(tU32 deviceID, tU8 *buf_wr, tU8 *buf_rd, tU32 len)
{
	tS32 fd;
	fd = BSP_DeviceAddressToFileDescriptor(deviceID);
	if (fd >= 0)
	{
		BSP_Linux_TransferSpi(fd, buf_wr, buf_rd, len, true);
	}
	else
	{
		ASSERT_ON_DEBUGGING(0);
	}
}

/**************************************
 *
 * BSP_Write_CMOST_SPI
 *
 *************************************/
/*
 * Write <len> bytes from buffer <buf> to the
 *  SPI bus connected to the CMOST.
 *
 * Returns:
 * the number of bytes read, or
 * -1 in case of error
 */

tS32 BSP_Write_CMOST_SPI(tU32 deviceID, tU8* buf, tU32 len)
{
	tS32 vl_fdp;
	tS32 vl_ret;
	// retrieve the IDs 
	//
	
	vl_fdp = BSP_DeviceAddressToFileDescriptor(deviceID);

	// Chipselect


	(void)BSP_SetCS_CMOST_SPI(deviceID, 0);
	// write the data
	vl_ret = write(vl_fdp, buf, (size_t)len);

	// Set back the chipselect to high 
	//	BSP_Linux_WriteGPIO(vl_CS_fd,1);

	(void)BSP_SetCS_CMOST_SPI(deviceID, 1);


	return(vl_ret);

}

/**************************************
 *
 * BSP_Set_CMOST_CS_SPI
 *
 *************************************/
/*
 *set the Chipselect to requested position
 * TRUE = HIGH, FALSE = DOWN
 * <buf> must be allocated by the caller.
 * over SPI bus connection to the CMOST

 * Returns:
 * the number of bytes read, or
 * -1 in case of error
 */


tSInt BSP_SetCS_CMOST_SPI(tU32 deviceID, tBool vI_value)
{	
	tS32 vl_CS_fd;
	tSInt vl_res = OSAL_OK;
	
	// Chipselect
	if(((tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->GPIO_CS == ETAL_CS_TRUE_SPI)
	{
		// do nothing on SPI0
		goto exit;
	}

	vl_CS_fd = BSP_DeviceAddressToChipSelectDescriptor(deviceID);

	if ((-1) != vl_CS_fd)
	{
		// Set the chipselect to low level for writing start
		BSP_Linux_WriteGPIO(vl_CS_fd, vI_value);
	}
	else
	{
		// chipselect not found
		vl_res = OSAL_ERROR;
	}

exit:
	return (vl_res);
}

/**************************************
 *
 * BSP_WriteRead_CMOST_SPI
 *
 *************************************/
/*
 * Read <len> bytes from the CMOST into
 * buffer <buf>.
 * <buf> must be allocated by the caller.
 * over SPI bus connection to the CMOST

 * Returns:
 * the number of bytes read, or
 * -1 in case of error
 */


tS32 BSP_WriteRead_CMOST_SPI(tU32 deviceID, tU8 *pI_InputBuf, tU16 vI_ByteToWrite, tU8* pO_OutBuf, tU16 vI_ByteToRead, tBool vI_ChipSelectControl)
{
	tS32 vl_fdp, vl_CS_fd, vl_ret;
	tU8 *pl_InputBfr, *pl_OutputBfr;
	tU16 vl_size;
	
#ifdef ETAL_BSP_DEBUG_SPI // tmp add-on
	tU32	vl_CmostHeaderLenVal;
#endif
		
	// some checks 
	// if input buffer is null whereas byte are supposed to be writter
	// or if output buffer is null
	if (((NULL == pI_InputBuf) && (0 != vI_ByteToWrite))
		|| (NULL == pO_OutBuf))
	{
		// error in allocation
		ASSERT_ON_DEBUGGING(0); 		//lint !e960	- MISRA 12.10 - use of assert code
		vl_ret = -1;
		goto exit;
	}
		
	// Prepare the buffers in case 
	if (vI_ByteToRead > vI_ByteToWrite)
	{
		// the input write buffer may not be big enough
		// allocate memory
		
		pl_InputBfr = (tU8 *) OSAL_pvMemoryAllocate(vI_ByteToRead);
		if (NULL == pl_InputBfr)
		{
			// error in allocation
			ASSERT_ON_DEBUGGING(0); 		//lint !e960	- MISRA 12.10 - use of assert code
			vl_ret = -1;
			goto exit;
				
		}
		// init the buffer with 0
		//
		(void)OSAL_pvMemorySet(pl_InputBfr, 0x00,vI_ByteToRead);
		// cpy the data to write
		if (NULL != pI_InputBuf)
		{
			(void)OSAL_pvMemoryCopy(pl_InputBfr, pI_InputBuf, vI_ByteToWrite);
		}
		else
		{
			// nothing to write... keep 0x00
		}

		// set the buffers
		vl_size = vI_ByteToRead;
		pl_OutputBfr = pO_OutBuf;
		
	}
	else if (vI_ByteToRead < vI_ByteToWrite)
	{
		// the output read buffer may not be big enough
		// allocate memory
		
		pl_OutputBfr = (tU8 *) OSAL_pvMemoryAllocate(vI_ByteToWrite);
		if (NULL == pl_OutputBfr)
		{
			// error in allocation
			ASSERT_ON_DEBUGGING(0); 		//lint !e960	- MISRA 12.10 - use of assert code
			vl_ret = -1;
			goto exit;
		}
		// init the buffer with 0
		//
		(void)OSAL_pvMemorySet(pl_OutputBfr, 0x00,vI_ByteToWrite);

		// set the buffers
		vl_size = vI_ByteToWrite;
		pl_InputBfr = pI_InputBuf;

	}
	else
	{
		// read = write 
		// set the buffers
		vl_size = vI_ByteToRead;
		pl_OutputBfr = pO_OutBuf;
		pl_InputBfr = pI_InputBuf;
	}
	
	// retrieve the IDs 
	//
	
	vl_fdp = BSP_DeviceAddressToFileDescriptor(deviceID);
	
	if (-1 == vl_fdp)
	{
		// invalid file descriptor
		ASSERT_ON_DEBUGGING(0); 		//lint !e960	- MISRA 12.10 - use of assert code

		// free what it is needed
		
		// 
		// if we come here, the allocated buffer are not null
		//	free memory
		if (vI_ByteToRead > vI_ByteToWrite)
		{
			// the input write buffer was reallocated, free it
			
			OSAL_vMemoryFree(pl_InputBfr);
		
		}
		else if (vI_ByteToRead < vI_ByteToWrite)
		{
			// the ouput write buffer was reallocated, free it

			OSAL_vMemoryFree(pl_OutputBfr);
		}
		else
		{
			// all is fine
		}


		vl_ret = (-1);
		goto exit;
	}
		
	// Chipselect
	vl_CS_fd = BSP_DeviceAddressToChipSelectDescriptor(deviceID);

	if (TRUE == vI_ChipSelectControl)
	{
		// Set the chipselect to low level for writing start
		BSP_Linux_WriteGPIO(vl_CS_fd,0);
	}

			
	// CMOST over SPI is write / read : 1st part is the header in theory 3 bytes
	BSP_Linux_TransferSpi(vl_fdp, pl_InputBfr, pl_OutputBfr, vl_size, false);

	// get the len 		
	if (TRUE == vI_ChipSelectControl)
	{
		// Set back the chipselect to high 
		BSP_Linux_WriteGPIO(vl_CS_fd,1);
	}

	// add a check on the len before copy
	//
	// for specific case of commands / answer to track CRC errors
	//
	// case : 100 bytes frame : CMOST_PHY_HEADER_LEN_SPI + CMOST_MAX_RESPONSE_LEN
	//
#ifdef ETAL_BSP_DEBUG_SPI // tmp add-on	
	if (100 == vI_ByteToRead)
	{
		// check the response len value 
		// 

		//vl_CmostHeaderLenVal = *(pl_OutputBfr + CMOST_PHY_HEADER_LEN_SPI + ETAL_CMOST_LEN_BYTE_POSITION);
		// the answer frame is (from SPI protocol) : 1st byte is not meaning full it is the write ==> bytetowrite ie CMOST_HEADER_LEN not usefull
		// the answer is then : CMOST_HEADER + PARAM + CRC
		// in the header, LEN is at position 2
		//
		vl_CmostHeaderLenVal = *(pl_OutputBfr + 4 + 2);
		// if it is not valid do a print
		//
		if (vl_CmostHeaderLenVal >= 30)
//		if (vl_CmostHeaderLenVal >= 3)
		{
			// the CMOST read is more than 30 param which is not valid
?????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
			BSP_PrintBufNoSpace(TR_LEVEL_ERRORS, FALSE, pl_InputBfr, vl_size, BSP_WRITE_OPERATION);
			BSP_PrintBufNoSpace(TR_LEVEL_ERRORS, FALSE, pl_OutputBfr, vl_size, BSP_READ_OPERATION);
		}

	}
#endif 
	
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_tracePrintComponent(TR_CLASS_BSP, "BSP_WriteRead_CMOST_SPI : vI_ByteToWrite %d, vI_ByteToRead %d", vI_ByteToWrite, vI_ByteToRead);
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, FALSE, pl_InputBfr, vl_size, BSP_WRITE_OPERATION);
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, FALSE, pl_OutputBfr, vl_size, BSP_READ_OPERATION);
#endif
	
	// recopy the result if needed and free memory
	// Prepare the buffers in case 
	if (vI_ByteToRead > vI_ByteToWrite)
	{
		// the input write buffer was reallocated, free it
		
		OSAL_vMemoryFree(pl_InputBfr);
		
	}
	else if (vI_ByteToRead < vI_ByteToWrite)
	{
		// the ouput write buffer was reallocated, free it
		// but before copy the data

		(void)OSAL_pvMemoryCopy(pO_OutBuf, pl_OutputBfr, vI_ByteToRead);
		OSAL_vMemoryFree(pl_OutputBfr);
	}
	else
	{
		// all is fine
	}
	

	vl_ret = vI_ByteToRead;
	
exit:	
	return(vl_ret);
}

#endif //CONFIG_ETAL_SUPPORT_CMOST

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/**************************************
 *
 * BSP_DeviceReset_HDRADIO
 *
 *************************************/
/*
 * Manages the HDRADIO reset on the A2 EVB, where the HDRADIO's RSTN line
 * is connected to GPIO20
 * RSTN is active low
 *
 * Return:
 *  OSAL_OK    - no error
 *  OSAL_ERROR - error accessing the device
 *
 */
tSInt BSP_DeviceReset_HDRADIO(tU32 deviceID)
{
	tS32 fd;
	char str_GPIO_RESET[11];

	sprintf(str_GPIO_RESET, "%u", HDRADIO_GetGPIOReset(deviceID));

	fd = BSP_Linux_OpenGPIO(str_GPIO_RESET, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
	if (fd < 0)
	{
		return OSAL_ERROR;
	}

	/*
	 * Assert the reset line
	 */
	BSP_Linux_WriteGPIO(fd, FALSE);
	/*
	 * Let the signal settle
	 */
	OSAL_s32ThreadWait(2);
	/*
	 * Deassert the reset line
	 */
	BSP_Linux_WriteGPIO(fd, TRUE);
	/*
	 * Let the signal settle
	 */
	OSAL_s32ThreadWait(2);

	BSP_Linux_CloseGPIO(fd);

	BSP_tracePrintComponent(TR_CLASS_BSP, "HDRADIO reset complete");

	return OSAL_OK;
}

/**************************************
 *
 * BSP_SPI0DriveCS_HDRADIO
 *
 *************************************/
/*
 * Drives the value of the SPI0 CS line by GPIO
 * (not native SPI-SS because of MDR's STECI connection)
 */
tVoid BSP_SPI0DriveCS_HDRADIO(tBool value)
{
	BSP_Linux_WriteGPIO(HDRADIO_LinuxSPI_CS_fd, value);
}

/**************************************
 *
 * BSP_BusConfigSPIMode_HDRADIO
 *
 *************************************/
/*
 * Change the SPI communication bus mode
 * <mode> Indicate the SPI mode.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfigSPIMode_HDRADIO(tU32 deviceID)
{
	if (ioctl(HDRADIO_SPI_LinuxDevice_fd, SPI_IOC_WR_MODE, &(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode)) < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "configuring the spi mode");
		(LINT_IGNORE_RET)close(HDRADIO_SPI_LinuxDevice_fd);
		HDRADIO_SPI_LinuxDevice_fd = -1;
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/**************************************
 *
 * BSP_BusConfigSPIFrequency_HDRADIO
 *
 *************************************/
/*
 * Change the SPI communication bus frequency
 * <deviceID> - device ID.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfigSPIFrequency_HDRADIO(tU32 deviceID)
{
	if (ioctl(HDRADIO_SPI_LinuxDevice_fd, SPI_IOC_WR_MAX_SPEED_HZ, &(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->speed)) < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "configuring the SPI frequency");
		(LINT_IGNORE_RET)close(HDRADIO_SPI_LinuxDevice_fd);
		HDRADIO_SPI_LinuxDevice_fd = -1;
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/**************************************
 *
 * BSP_BusConfig_HDRADIO
 *
 *************************************/
/*
 * Prepares the application for read/write access
 * to the HDRADIO through I2C or SPI bus
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfig_HDRADIO(tU32 deviceID)
{
	tSInt ret = OSAL_ERROR;

	if(HDRADIO_GetBusType(deviceID) == BusSPI)
	{
		tU8 u8FakeWrForClkLevel = 0xFF;
		char strGPIO_CS[11];

		sprintf(strGPIO_CS, "%u", ((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->GPIO_CS);

		HDRADIO_SPI_LinuxDevice_fd = -1;

		HDRADIO_SPI_LinuxDevice_fd = BSP_Linux_OpenDevice(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->busName);
		if (HDRADIO_SPI_LinuxDevice_fd < 0)
		{
			return OSAL_ERROR;
		}

		/* Set up the SPI mode */
		if(BSP_BusConfigSPIMode_HDRADIO(deviceID) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		/* Set up the SPI speed */
		if(BSP_BusConfigSPIFrequency_HDRADIO(deviceID) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		// The clock line is born by A2 with a level that might be not compatible with the mode selected.
		// The fake write w/o FS driving is to adjust the clock line level
		write(HDRADIO_SPI_LinuxDevice_fd, &u8FakeWrForClkLevel, 0);

		HDRADIO_LinuxSPI_CS_fd = -1;

		HDRADIO_LinuxSPI_CS_fd = BSP_Linux_OpenGPIO(strGPIO_CS, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
		if (HDRADIO_LinuxSPI_CS_fd < 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "configuring the SPI CS");
			return OSAL_ERROR;
		}

		BSP_Linux_WriteGPIO(HDRADIO_LinuxSPI_CS_fd, TRUE);

		ret = OSAL_OK;
	}
	else
	{
		/*
		 * the I2C address is 7 bits plus the direction; in Linux
		 * the direction bit must be discarded
		 */
		tS32 addr = (tS32)(((tyI2cCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->deviceAddress >> 1); // was HDRADIO_ACCORDO2_I2C_ADDRESS

		HDRADIO_I2C_LinuxDevice_fd = -1;

		HDRADIO_I2C_LinuxDevice_fd = BSP_Linux_OpenDevice(((tyI2cCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->busName);
		if (HDRADIO_I2C_LinuxDevice_fd < 0)
		{
			return OSAL_ERROR;
		}

		/*
		 * Set up the device address
		 */
		if (ioctl(HDRADIO_I2C_LinuxDevice_fd, I2C_SLAVE, addr) < 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "configuring the I2C device");
			(LINT_IGNORE_RET)close(HDRADIO_I2C_LinuxDevice_fd);
			HDRADIO_I2C_LinuxDevice_fd = -1;
			return OSAL_ERROR;
		}

		ret = OSAL_OK;
	}
	return ret;
}

/**************************************
 *
 * BSP_BusUnconfig_HDRADIO
 *
 *************************************/
/*
 * Closes the Operating System resources previously
 * configured to access the HDRADIO through I2C or SPI bus
 *
 * Returns:
 *  0  - success
 *  -1 - error accessing the kernel driver interface
 */
tSInt BSP_BusUnconfig_HDRADIO(tU32 deviceID)
{
	tSInt ret = -1;

	if(HDRADIO_GetBusType(deviceID) == BusSPI)
	{
		ret = BSP_Linux_CloseDevice(HDRADIO_SPI_LinuxDevice_fd);
		HDRADIO_SPI_LinuxDevice_fd = -1;
	}
	else
	{
		ret = BSP_Linux_CloseDevice(HDRADIO_I2C_LinuxDevice_fd);
		HDRADIO_I2C_LinuxDevice_fd = -1;
	}

	return ret;
}

/**************************************
 *
 * BSP_DeviceInit_HDRADIO
 *
 *************************************/
tSInt BSP_DeviceInit_HDRADIO(tU32 deviceID)
{
	if (BSP_BusConfig_HDRADIO(deviceID) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	BSP_tracePrintComponent(TR_CLASS_BSP, "HDRADIO init complete");

	return OSAL_OK;
}

/**************************************
 *
 * BSP_DeviceDeinit_HDRADIO
 *
 *************************************/
tSInt BSP_DeviceDeinit_HDRADIO(tU32 deviceID)
{
	if (BSP_BusUnconfig_HDRADIO(deviceID) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	BSP_tracePrintComponent(TR_CLASS_BSP, "HDRADIO deinit complete");

	return OSAL_OK;
}

/**************************************
 *
 * BSP_Write_HDRADIO
 *
 *************************************/
tS32 BSP_Write_HDRADIO(tU32 deviceID , tU8* buf, tU32 len)
{
	tS32 ret = -1;

	if(HDRADIO_GetBusType(deviceID) == BusSPI)
	{
		if(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0)
		{
			tU32 bufIndex = 0;
			tS32 tmpRet = 0;

			while (bufIndex < len)
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				tmpRet += write(HDRADIO_SPI_LinuxDevice_fd, &buf[bufIndex], 1);
				BSP_SPI0DriveCS_HDRADIO (1);
				bufIndex++;
			}
			ret = tmpRet;
		}
		else
		{
			BSP_SPI0DriveCS_HDRADIO (0);
			ret = write(HDRADIO_SPI_LinuxDevice_fd, buf, len);
			BSP_SPI0DriveCS_HDRADIO (1);
		}
	}
	else
	{
		ret = (tS32)write (HDRADIO_I2C_LinuxDevice_fd, buf, (size_t)len);
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
		BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, (HDRADIO_GetBusType(deviceID) == BusSPI)? 0 : 1, buf, len, BSP_WRITE_OPERATION);
#endif //(defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)

	if (ret < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "writing command to HDRADIO");
	}
	else if (ret < (tS32)len)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "truncated cmd write to HDRADIO");
		return -1;
	}

	return ret;
}

/**************************************
 *
 * BSP_Read_HDRADIO
 *
 *************************************/
tS32 BSP_Read_HDRADIO (tU32 deviceID, tU8** pBuf)
{
	tS32 ret = -1;
	tU32 len;
	tU16 lmAchievedBytes = 0;
	tU8 *pLocalBuf = &((*pBuf)[0]);

	if(HDRADIO_GetBusType(deviceID) == BusSPI)
	{
		tU16 bufIndex;
		tU8 msecForCmdProcessed = 0;

		memset (pLocalBuf, 0x00, HDRADIO_MIN_LMLENGTH);

		if(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0)
		{
			// Reading of zeros till the cmd is processed
			while (pLocalBuf[0] == 0 && msecForCmdProcessed < HDRADIO_MS_TO_HAVE_CMD_PROCESSED)
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret = read (HDRADIO_SPI_LinuxDevice_fd, &pLocalBuf[0], 1);
				BSP_SPI0DriveCS_HDRADIO (1);
				OSAL_s32ThreadWait (1);
				msecForCmdProcessed++;
			}

			if (msecForCmdProcessed == HDRADIO_MS_TO_HAVE_CMD_PROCESSED)
			{
				BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO");
				return -1;
			}

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;
			ret = 0;

			for (bufIndex = 1; bufIndex < HDRADIO_MIN_LMLENGTH; bufIndex++)
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret += read (HDRADIO_SPI_LinuxDevice_fd, &pLocalBuf[bufIndex], 1);
				BSP_SPI0DriveCS_HDRADIO (1);
			}

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;
			ret = 0;

			// LM Header checking: the len fields would be meaningless if the content just read is not an LM
			if (pLocalBuf[0] == HDRADIO_LM_BYTE_HEADER && pLocalBuf[1] == HDRADIO_LM_BYTE_HEADER)
			{
				// LM length computing
				len = pLocalBuf[4] + (pLocalBuf[5] << 8);
			}
			else
			{
				return (tS32)lmAchievedBytes;
			}

			for (bufIndex = HDRADIO_MIN_LMLENGTH; bufIndex < len; bufIndex++)
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret += read (HDRADIO_SPI_LinuxDevice_fd, &pLocalBuf[bufIndex], 1);
				BSP_SPI0DriveCS_HDRADIO (1);
			}

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;
		}
		else
		{
			tU8 msecForCmdProcessed = 0;

			memset (pLocalBuf, 0x00, HDRADIO_MIN_LMLENGTH);

			BSP_SPI0DriveCS_HDRADIO (0);

			// Reading of zeros till the cmd is processed
			while (pLocalBuf[0] == 0 && msecForCmdProcessed < HDRADIO_MS_TO_HAVE_CMD_PROCESSED)
			{
				ret = read (HDRADIO_SPI_LinuxDevice_fd, pLocalBuf, 1);
				OSAL_s32ThreadWait (1);
				msecForCmdProcessed++;
			}

			if (msecForCmdProcessed == HDRADIO_MS_TO_HAVE_CMD_PROCESSED)
			{
				BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO");
				BSP_SPI0DriveCS_HDRADIO (1);

				return -1;
			}

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;

			ret = read (HDRADIO_SPI_LinuxDevice_fd, pLocalBuf+1, HDRADIO_MIN_LMLENGTH-1);

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;

			// LM Header checking: the len fields would be meaningless if the content just read is not an LM
			if (pLocalBuf[0] == HDRADIO_LM_BYTE_HEADER && pLocalBuf[1] == HDRADIO_LM_BYTE_HEADER)
			{
				// LM length computing
				len = pLocalBuf[4] + (pLocalBuf[5] << 8);
			}
			else
			{
				BSP_SPI0DriveCS_HDRADIO (1);

				return (tS32)lmAchievedBytes;
			}

			ret = read (HDRADIO_SPI_LinuxDevice_fd, pLocalBuf+HDRADIO_MIN_LMLENGTH, len-HDRADIO_MIN_LMLENGTH);

			BSP_SPI0DriveCS_HDRADIO (1);

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;
		}
	}
	else
	{
		// The buffer passed is allocated with 4 bytes in addition (4 bytes for word alignment). This allows reading the "byte-count" byte indicating the
		// number of bytes available in the I2C FIFO, as for the Master-Read Transaction protocol, and shifting the pointer after having managed it.
		// This way to avoid having a temp buffer to be copied, therefore in order to save memory

		tU8 numberOfBytesInsideFifo;
		tU8 msecForFifoFilled;
		tU16 lmRemainingBytes;
		tU8 lmRemainingPackets;
		tU16 lmBytesToRead;
		tU16 lmBufferIndex;
		tU8 lmTmpBuffer[HDRADIO_FIFO_SIZE_IN_BYTES+1];

		// Read first the "byte-count in the FIFO" until it has the minimum amount therefore with the LM length field inside the FIFO
		numberOfBytesInsideFifo = (tU8)0;
		msecForFifoFilled = (tU8)0;

		memset (pLocalBuf, 0x00, HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH+1);

		while ((tS32)numberOfBytesInsideFifo < HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH && (tS32)msecForFifoFilled < HDRADIO_MS_TO_HAVE_FIFO_MIN_FILLED)
		{
			if (read (HDRADIO_I2C_LinuxDevice_fd, &numberOfBytesInsideFifo, 1) < 0)
			{
				return -1;
			}
			OSAL_s32ThreadWait (1);
			msecForFifoFilled += (tU8)1;
		}

		if ((tS32)msecForFifoFilled == HDRADIO_MS_TO_HAVE_FIFO_MIN_FILLED)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO");
			return -1;
		}

		// Read the "min LM" bytes certified to be in the FIFO
		ret = (tS32)read (HDRADIO_I2C_LinuxDevice_fd, pLocalBuf, HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH+1);

		if (ret < 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO");
			return -1;
		}
		else if ((tU32)ret < HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH+1)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "truncated data read from HDRADIO");
			// Update the caller buffer to throw away the "byte-count" byte
			(*pBuf) += 1;
			lmAchievedBytes += (tU16)(ret-1);
			return (tS32)lmAchievedBytes;
		}

		// Update the bytes actually read
		lmAchievedBytes += (tU16)(ret-1);

		// Local pointer management and LM length computing
		pLocalBuf += 1;

		// LM Header checking: the len fields would be meaningless if the content just read is not an LM
		if ((tS32)pLocalBuf[0] == HDRADIO_LM_BYTE_HEADER && (tS32)pLocalBuf[1] == HDRADIO_LM_BYTE_HEADER)
		{
			// LM length computing
			len = (tU32)pLocalBuf[4] + ((tU32)pLocalBuf[5] << 8);
		}
		else
		{
			*pBuf = pLocalBuf;
			return (tS32)lmAchievedBytes;
		}

		lmRemainingBytes = (tU16)(len - HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH);
		lmRemainingPackets = len / HDRADIO_FIFO_SIZE_IN_BYTES;

		lmBufferIndex = 0;
		while (lmRemainingBytes > 0)
		{
			// Define number of bytes to read
			if (lmRemainingPackets > (tU8)0)
			{
				if (0 == lmBufferIndex)
				{
					lmBytesToRead = HDRADIO_FIFO_SIZE_IN_BYTES - HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH;
				}
				else
				{
					lmBytesToRead = HDRADIO_FIFO_SIZE_IN_BYTES;
				}
			}
			else
			{
				lmBytesToRead = lmRemainingBytes;
			}

			numberOfBytesInsideFifo = (tU8)0;
			msecForFifoFilled = (tU8)0;
			while ((tU16)numberOfBytesInsideFifo < lmBytesToRead && (tS32)msecForFifoFilled < HDRADIO_MS_TO_HAVE_FIFO_FILLED)
			{
				if (read (HDRADIO_I2C_LinuxDevice_fd, &numberOfBytesInsideFifo, 1) < 0)
				{
					return -1;
				}
				OSAL_s32ThreadWait (1);
				msecForFifoFilled += (tU8)1;
			}

			if ((tS32)msecForFifoFilled == HDRADIO_MS_TO_HAVE_FIFO_FILLED)
			{
				BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO");
				// Update the caller buffer to throw away the "byte-count" byte
				(*pBuf) += 1;
				return (tS32)lmAchievedBytes;
			}

			// Read the "Delta" bytes certified to be in the FIFO
			ret = (tS32)read (HDRADIO_I2C_LinuxDevice_fd, &lmTmpBuffer[0], (size_t)lmBytesToRead+1);

			if (ret < 0)
			{
				BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO");
				// Update the caller buffer to throw away the "byte-count" byte
				(*pBuf) += 1;
				return (tS32)lmAchievedBytes;
			}
			else if ((tU32)ret < (tU32)lmBytesToRead+1) /* both casts to get rid of compiler warning */
			{
				BSP_tracePrintError(TR_CLASS_BSP, "truncated data read from HDRADIO");
				if (0 == lmBufferIndex)
				{
					// Copy another message chunk
					OSAL_pvMemoryCopy ((tVoid *)(pLocalBuf+HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH), (tPCVoid)&lmTmpBuffer[1], (tU32)(ret-1));
				}
				else
				{
					// Copy another message chunk
					OSAL_pvMemoryCopy ((tVoid *)(pLocalBuf+lmBufferIndex), (tPCVoid)&lmTmpBuffer[1], (tU32)(ret-1));
				}

				// Update the caller buffer after that the local pointer was incremented to throw away the "byte-count" byte
				*pBuf = pLocalBuf;
				lmAchievedBytes += (tU16)(ret-1);
				return (tS32)lmAchievedBytes;
			}

			if (0 == lmBufferIndex)
			{
				// Copy another message chunk
				OSAL_pvMemoryCopy ((tVoid *)(pLocalBuf+HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH), (tPCVoid)&lmTmpBuffer[1], lmBytesToRead);
			}
			else
			{
				// Copy another message chunk
				OSAL_pvMemoryCopy ((tVoid *)(pLocalBuf+lmBufferIndex), (tPCVoid)&lmTmpBuffer[1], lmBytesToRead);
			}

			// Update the bytes actually read
			lmAchievedBytes += (tU16)(ret-1);

			// Decrement the bytes read and the packets processed
			lmRemainingBytes -= lmBytesToRead;
			if (lmRemainingPackets > (tU8)0)
			{
				lmRemainingPackets -= (tU8)1;
			}

			// Increment number of packets read
			lmBufferIndex += HDRADIO_FIFO_SIZE_IN_BYTES;
		}

		// Update the caller buffer after that the local pointer was incremented to throw away the "byte-count" byte
		*pBuf = pLocalBuf;
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, (HDRADIO_GetBusType(deviceID) == BusSPI)? 0 : 1,
		*pBuf, len, BSP_READ_OPERATION);
#endif //(defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)

	return (tS32)lmAchievedBytes;
}

/**************************************
 *
 * BSP_Raw_Read_HDRADIO
 *
 *************************************/
tS32 BSP_Raw_Read_HDRADIO(tU32 deviceID, tU8 *pBuf, tU32 len)
{
	tS32 ret = -1;

	if(HDRADIO_GetBusType(deviceID) == BusSPI)
	{
		if(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0)
		{
			tU16 bufIndex;

			ret = 0;
			for (bufIndex = 0; bufIndex < len; bufIndex++)
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret += read(HDRADIO_SPI_LinuxDevice_fd, &(pBuf[bufIndex]), 1);
				BSP_SPI0DriveCS_HDRADIO (1);
			}
		}
		else
		{
			BSP_SPI0DriveCS_HDRADIO (0);
			ret = read (HDRADIO_SPI_LinuxDevice_fd, pBuf, len);
			BSP_SPI0DriveCS_HDRADIO (1);
		}
	}
	else
	{
		ret = (tS32)read (HDRADIO_I2C_LinuxDevice_fd, pBuf, (size_t)len);
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, (HDRADIO_GetBusType(deviceID) == BusSPI)? 0 : 1, pBuf, len, BSP_READ_OPERATION);
#endif //(defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)

	if (ret < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "raw reading response from HDRADIO");
	}
	else if (ret < (tS32)len)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "truncated raw read response from HDRADIO");
		return -1;
	}

	return ret;
}

/**************************************
 *
 * BSP_TransferSpi_HDRADIO
 *
 *************************************/
/*
 * Performs an SPI read/write (full-duplex) operation; both read and write
 * buffers are assumed to be <len> bytes long.
 */
tVoid BSP_TransferSpi_HDRADIO(tU32 deviceID, tU8 *buf_wr, tU8 *buf_rd, tU32 len)
{
	tU16 bufIndex;

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, 0, buf_wr, len, BSP_WRITE_OPERATION);
#endif //(defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)

	if(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0)
	{
		for (bufIndex = 0; bufIndex < len; bufIndex++)
		{
			BSP_SPI0DriveCS_HDRADIO (0);
			BSP_Linux_TransferSpi(HDRADIO_SPI_LinuxDevice_fd, buf_wr, &(buf_rd[bufIndex]), 1, false);
			BSP_SPI0DriveCS_HDRADIO (1);
		}
	}
	else
	{
		BSP_SPI0DriveCS_HDRADIO (0);
		BSP_Linux_TransferSpi(HDRADIO_SPI_LinuxDevice_fd, buf_wr, buf_rd, len, true);
		BSP_SPI0DriveCS_HDRADIO (1);
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, 0, buf_rd, len, BSP_READ_OPERATION);
#endif //(defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
}

#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

#endif //CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_HOST_OS_LINUX && CONFIG_BOARD_ACCORDO2||CONFIG_BOARD_ACCORDO5

#if (defined(CONFIG_COMM_DRIVER_EMBEDDED) || defined(CONFIG_COMM_DRIVER_EXTERNAL)) && (defined (CONFIG_BOARD_ACCORDO2) || defined (CONFIG_BOARD_ACCORDO5)) && defined (CONFIG_HOST_OS_LINUX)

#if defined(CONFIG_DIGITAL_AUDIO) || defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
tVoid BSP_AudioSelect(BSPAudioSelectTy src)
{
//	static BSPAudioSelectTy vl_src = BSP_AUDIO_SELECT_RX1;
		
//	if (src != vl_src)
	{
//		printf("BSP_AudioSelect change source to %d\n", src);
		
		switch (src)
		{
			case BSP_AUDIO_SELECT_RX1:
#ifdef CONFIG_DIGITAL_AUDIO
				// audio path selection
				BSP_Linux_SystemCmd("amixer -c 3 sset Source sai4rx1fm > /dev/null" );
#else
				// audio path selection
				BSP_Linux_SystemCmd("amixer -c 3 sset Source adcauxdac > /dev/null" );
				// select the audio channel
				BSP_Linux_SystemCmd("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
				break;
			case BSP_AUDIO_SELECT_RX2:
#if defined(CONFIG_DIGITAL_AUDIO) && (!defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST))
				// audio path selection
				BSP_Linux_SystemCmd("amixer -c 3 sset Source sai4rx2dab > /dev/null" );
#elif defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
				// audio path selection
				BSP_Linux_SystemCmd("amixer -c 3 sset Source tunerss > /dev/null" );
				// select the MSP clock
				BSP_Linux_SystemCmd("amixer -c2 set Clock ext > /dev/null");
#else // !defined(CONFIG_DIGITAL_AUDIO) && !defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
				// audio path selection
				BSP_Linux_SystemCmd("amixer -c 3 sset Source adcauxdac > /dev/null" );
				// select the audio channel
				BSP_Linux_SystemCmd("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
				break;
			default:
				//error case
				break;
		}
		//vl_src = src;
	}
}
#endif //CONFIG_DIGITAL_AUDIO || CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST


#if defined(CONFIG_HOST_OS_LINUX) && defined(CONFIG_BOARD_ACCORDO5) && defined (CONFIG_ETAL_SUPPORT_EARLY_TUNER)
tS32 BSP_backup_context_for_early_audio(const BSPCtxBackupEarlyAudioTy *CtxEarlyAudio)
{
	tS32 fd;
	size_t SizeSent;
	size_t SizeToSend;

	/* openen the early audio backup file */
	fd = open(ETAL_EARLY_TUNER_SAVE_CTX_FILE, O_WRONLY);
	if (fd < 0)
	{
		return -1;
	}

	/* Write info to save */
	SizeToSend = sizeof(BSPCtxBackupEarlyAudioTy);
	SizeSent = write(fd, CtxEarlyAudio, SizeToSend);
	if (SizeSent < SizeToSend)
	{
		(LINT_IGNORE_RET)close(fd);
		return -1;
	}
	
	/* close the early audio backup file */
	if (close(fd) != 0)
	{
		return -1;
	}

	return 0;

}
#endif

#endif //CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_HOST_OS_LINUX && CONFIG_BOARD_ACCORDO2||CONFIG_BOARD_ACCORDO5
