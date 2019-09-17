//!
//!  \file 		bsp_sta1095evb.c
//!  \brief 	<i><b> ETAL BSP for STA1095 EVB </b></i>
//!  \details   Low level drivers for the Accordo2 EVB
//!  \author 	Raffaele Belardi, Roberto Allevi
//!
/*
 * This file contains the glue logic to access the Accordo2 EVB
 * hardware from the ETAL
 */

#include "target_config.h"

#define I2C_CLOCK 51200000

/*+DEBUG*/
// Switch to activate for CMOST SPI problem DEBUG
#undef ETAL_BSP_DEBUG_SPI
#define ETAL_BSP_DEBUG_SPI_RW_WORD

/*-DEBUG*/

#if (defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_BOARD_ACCORDO2)) || defined (CONFIG_HOST_OS_FREERTOS)

#include "FreeRTOS.h"

 #include "osal.h"
#include <stdio.h>
#include <unistd.h>
#include "bsp_sta1095evb.h"
#include "bsp_linux.h"

#include "common_trace.h"
#include "bsp_trace.h"

#if (defined BOOT_READ_BACK_AND_COMPARE_CMOST) && (CONFIG_TRACE_CLASS_BSP < TR_LEVEL_COMPONENT)
	#error "BOOT_READ_BACK_AND_COMPARE_CMOST requires CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT"
 #error "for complete output"
#endif


#include "sta_i2c_service.h"
#include "sta_gpio.h"



//***************************
// *
// * Local functions
// *
// **************************

#endif
static tSInt BSP_DeviceAddressSetIRQFileDescriptor(tU32 deviceID, tS32 vI_IRQ_fd);
static tS32 BSP_DeviceAddressToIRQFileDescriptor(tU32 deviceID);


static tS32 BSP_DeviceAddressToFileDescriptor(tU32 deviceID);

static int _i2c_read( tS32 dd, tU32 start, void* buf, tU32 size);
static int _i2c_write( tS32 dd, tU32 start, void* buf, tU32 size);
static int _i2c_data_write(tS32 id, tU8* buffer, tS32 i,  tU32   slv_add);
static int _i2c_data_read(tS32 id, tS32 i, tU8* read_data,  tU32   slv_add);
static void  _i2c_set_cond_f(tS32 id);
static int tnr_write(tS32 fd, tS32 device_address, tU8 *w_buf, tS32 w_len );
static int tnr_read(tS32 fd, tS32 device_address, tU8 *r_buf, tS32 r_len );

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
static tSInt BSP_BusConfig_MDR(tU32 deviceID);
#endif
static tSInt BSP_Reset_CMOST(tU32 deviceID);
static tSInt BSP_RDSInterruptInit_CMOST(tU32 deviceID);
static tVoid BSP_RDSInterruptDeinit_CMOST(tU32 deviceID);
static tS32 *BSP_DeviceAddressAdd(tU32 deviceID);
static tVoid BSP_DeviceAddressRelease(tU32 deviceID);

#if ((defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)) | (defined BOOT_READ_BACK_AND_COMPARE_CMOST)
tVoid BSP_PrintBufNoSpace(tS32 level, tBool useI2C, tUChar *buf, tS32 len, BSPPrintOperationTy op);
#endif


#ifdef CONFIG_DIGITAL_AUDIO
tVoid BSP_AudioSelect(BSPAudioSelectTy src);
#endif //CONFIG_DIGITAL_AUDIO

#define CPSDVR_MIN 0x02
#define CPSDVR_MAX 0xFE
#define SCR_MIN 0x00
#define SCR_MAX 0xFF
#define TNR_SPI_SSP_MAX_CLOCK	102400000
#define TNR_SPI_SSP_MAX_SPEED	4000000

#define BSP_SPI_BUF_SIZE	300

//static UB g_read_data[BSP_SPI_BUF_SIZE];  // g_read_data[260];



#ifdef CONFIG_DIGITAL_AUDIO
tVoid BSP_AudioSelect(BSPAudioSelectTy src)
{

}
#endif //CONFIG_DIGITAL_AUDIO


/*!
 * @brief	  _i2c_read:
 *      
 * @param    :  
 * @note     :  
 *
 */
static int _i2c_read( tS32 dd, tU32 start, void* buf, tU32 size)
{
        portTickType  timeout = 1000;
        return i2c_read((struct i2c_com_handler_s *)dd, 0, 0, buf, size, &timeout);
}

/*!
 * @brief	  		_i2c_write:
 * 			       This routine set the I2C start condition.
 * @param          	: id - I2X Device descriptor 	
 * @note  
 *
 */
static int _i2c_write( tS32 dd, tU32 start, void* buf, tU32 size)
{   
        portTickType  timeout = 10000;
        return i2c_write((struct i2c_com_handler_s *)dd, 0, 0, buf, size, size, &timeout);
}


/*!
 * @brief	  		_i2c_data_write:
 * 			       This routine configure the SLV address and request a writee to I2C driver.
 * @param     : id - I2C Device descriptor 	
 * @param     : buffer - I2C data buffer
 * @param     : i   - transfer lenght		
 * @note 
 *
 */
static int _i2c_data_write(tS32 id, tU8* buffer, tS32 i,  tU32   slv_add)
{ 
	int errorCode;
    errorCode=_i2c_write(id, slv_add, buffer, i);
	return errorCode;
}


/*!
 * @brief	 _i2c_data_read:
 * @param    
 * @param    
 * @param    
 * @note  
 *
 */
static int _i2c_data_read(tS32 id, tS32 i, tU8* read_data,  tU32   slv_add)
{


    return _i2c_read(id, slv_add, read_data, i);
}




/*!
 * @brief	  		_i2c_set_cond_f:
 * 			       This routine set the I2C start condition.
 * @param          	: id - I2C Device descriptor 	
 * @note 			: Use to control PCA9536 RGB device
 *
 */
static void  _i2c_set_cond_f(tS32 id)
{
    /*ST_I2C_COND_PARAM buf = { FALSE };
    (void)_i2c_write(id, ST_I2C_WSL_COND, &buf, sizeof(buf));*/
}


////////// Linux write and read redefinition for tkernel //////////////////// //m//
//This is wrong and does not differentiate I2C and SPI writes

static int tnr_write(tS32 fd, tS32 device_address, tU8 *w_buf, tS32 w_len )
{
    tS32 ret = -1;

	_i2c_set_cond_f(fd);  /* stop condition */
	ret = _i2c_data_write(fd, (tU8 *)w_buf, w_len, (tU32)(device_address) >> 1);

		
    return ret;  
}

static int tnr_read(tS32 fd, tS32 device_address, tU8 *r_buf, tS32 r_len )
{
	
    _i2c_set_cond_f(fd);  /* stop condition */
    (void)_i2c_data_read(fd, r_len, (tU8 *)r_buf, (tU32) device_address >> 1);


    return r_len;  
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
static int spi_read(tS32 device_Id, tU8 *r_buf, tS32 r_len, tBool vI_dataSize32bits )
{
        /* Not used on M3 context */
	return vl_ret;
	
}

#endif // #ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

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
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
	#undef BSP_INCLUDE_SIGNALLING_GPIO
#else
	//#define BSP_INCLUDE_SIGNALLING_GPIO
	#undef BSP_INCLUDE_SIGNALLING_GPIO
#endif

/*
 * Don't reset the MDR at startup.
 * This is useful when the MDR is loaded through the JTAG debugger
 * rather than the Flash memory, since the reset would disconnect
 * the debugger
 */
#undef DONT_RESET_MDR_AT_STARTUP

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
 
/*
 * The time taken by the HDRADIO to load the FW from flash and execute it
 */
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
#define BSP_MAX_TUNER_PER_BUS                   3

#define BSP_PRINTBUF_HEX_NS  ((tU32)(100))
#define BSP_PRINTBUF_SIZE_NS (BSP_PRINTBUF_HEX_NS * 2 + 2 + 6)


/**************************************
 *
 * Local variables
 *
 *************************************/
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
static tS32 MDR_LinuxDevice_fd = -1;
static tS32 MDR_LinuxSPI_CS_fd = -1;
static tS32 MDR_LinuxSPI_REQ_fd = -1;
#ifdef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
static tS32 MDR_LinuxSPI_BOOT_fd = -1;
#endif
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_CMOST
typedef struct
{
	tBool isValid;
	tS32  CMOST_LinuxDevice_fd;
	tS32  CMOST_Irq_fd;
	tU32  CMOST_device_Id;
	tS32  CMOST_cs_id; // for SPI only
} CMOSTAddressTy;
CMOSTAddressTy BSP_LinuxDevice[BSP_MAX_TUNER_PER_BUS];
#if defined (BSP_INCLUDE_SIGNALLING_GPIO)
tS32 BSP_LinuxSignalling_fd;
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
tS32 HDRADIO_I2C_LinuxDevice_fd;
tS32 HDRADIO_SPI_LinuxDevice_fd;
tS32 HDRADIO_LinuxSPI_CS_fd;
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO


#if ((defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)) || defined (BOOT_READ_BACK_AND_COMPARE_CMOST) || defined (ETAL_BSP_DEBUG_SPI)
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
#if 0
	static tChar sbuf[BSP_PRINTBUF_SIZE_NS];
	tS32 i;

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
		for (i = 0; (i < len) && (i < BSP_PRINTBUF_HEX_NS); i++)
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
	if (len > BSP_PRINTBUF_HEX_NS)
	{
		COMMON_tracePrint(level, TR_CLASS_BSP, "Output truncated to %d bytes", BSP_PRINTBUF_HEX_NS);
	}
#endif

}
#endif 

#ifdef CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS
/**************************************
 *
 * BSP_DeviceReset_MDR
 *
 *************************************/
/*
 * Manages the MDR reset on the Accordo2 EVB, where the MDR's RSTN line
 * is connected to GPIO20
 * RSTN is active low
 * In case of freeRTOS usage, the field deviceID is used to provide the GPIO number
 */
tSInt BSP_DeviceReset_MDR(tU32 deviceID)
{
	tS32 fd;
    char str_GPIO_RESET[11];
    BSP_tracePrintSysmin(TR_CLASS_BSP, "MDR reset");

    sprintf(str_GPIO_RESET, "%u", deviceID);

	fd = BSP_Linux_OpenGPIO(str_GPIO_RESET, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);

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
	OSAL_s32ThreadWait(5);

	BSP_Linux_CloseGPIO(fd);

    BSP_tracePrintSysmin(TR_CLASS_BSP, "MDR reset complete");

	return OSAL_OK;

}

/**************************************
 *
 * BSP_SteciSetBOOT_MDR
 *
 *************************************/
/*
 * Sets the value of the STECI's BOOT line
 */
tVoid BSP_SteciSetBOOT_MDR(tU32 GPIOnb, tBool value)
{
	tS32 fd;
    char str_GPIO_BOOT[11];
    BSP_tracePrintSysmin(TR_CLASS_BSP, "Set Boot MDR GPIO to %d",value);

    sprintf(str_GPIO_BOOT, "%u", GPIOnb);

	fd = BSP_Linux_OpenGPIO(str_GPIO_BOOT, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);

	/*
	 * Assert the reset line
	 */
	BSP_Linux_WriteGPIO(fd, value);
	/*
	 * Let the signal settle
	 */
	OSAL_s32ThreadWait(5);

	BSP_Linux_CloseGPIO(fd);

    BSP_tracePrintSysmin(TR_CLASS_BSP, "Set Boot MDR GPIO to %d complete", value);
	
}

#endif /* CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS */

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

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
        /* Not used on M3 context */
	return 0;
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
        /* Not used on M3 context */
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
        /* Not used on M3 context */
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
        /* Not used on M3 context */
    return 0
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
	/* Not used on M3 context */
        return 0;
}

/**************************************
 *
 * BSP_DeviceInit_MDR
 *
 *************************************/
tSInt BSP_DeviceInit_MDR(tU32 deviceID)
{
        /* Not used on M3 context */
        return OSAL_ERROR;
}

/**************************************
 *
 * BSP_DeviceDeinit_MDR
 *
 *************************************/
tVoid BSP_DeviceDeinit_MDR(tVoid)
{
        /* Not used on M3 context */
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_CMOST

/**************************************
 *
 * BSP_Reset_CMOST
 *
 *************************************/
/*
 * Manages the CMOST reset on the A2 EVB, where the CMOST's RSTN line
 * is connected to S_GPIO6
 * RSTN is active low
 *
 * Paramters:
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
	tS32 fd = -1;
//	tySpiCommunicationBus *spiBusConfig = (tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID);
	char str_GPIO_RESET[11], str_GPIO_IRQ[11];
	tSInt ret = OSAL_OK;

	sprintf(str_GPIO_RESET, "%u", TUNERDRIVER_GetGPIOReset(deviceID));
	sprintf(str_GPIO_IRQ, "%u", TUNERDRIVER_GetGPIOIrq(deviceID));

 
	fd = BSP_Linux_OpenGPIO(str_GPIO_RESET, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
	
	if (fd < 0)
	{
		ret = OSAL_ERROR;
		goto exit;
	}
	
	if (TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SPI bus not available in this configuration (M3 used)");
#endif
	}

	// force the reset to high as a start
	/*
	 * Perform the reset
	 * Negate because the reset line is active low
	 */
	BSP_Linux_WriteGPIO(fd, TRUE);
	(void)OSAL_s32ThreadWait(20);

	/* Perform the reset
	 * Negate because the reset line is active low */
	BSP_Linux_WriteGPIO(fd, FALSE);
	(void)OSAL_s32ThreadWait(20);
	
	/*
	 * Perform the reset
	 * Negate because the reset line is active low
	 */
	BSP_Linux_WriteGPIO(fd, TRUE);

	/* Let the signal settle */
	(void)OSAL_s32ThreadWait(20);


	/* Now GPIO can be closed */
	BSP_Linux_CloseGPIO(fd);

	if (TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SPI bus not available in this configuration (M3 used)");
#endif
	}

	/* Let the signal settle */
	(void)OSAL_s32ThreadWait(5);

	// TDA7708 Patch :

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
	char str_GPIO_IRQ[11];
	tS32 vl_IRQ_fd;
	tSInt ret = OSAL_OK;

	sprintf(str_GPIO_IRQ, "%u", TUNERDRIVER_GetGPIOIrq(deviceID));

	vl_IRQ_fd = BSP_Linux_OpenGPIO(str_GPIO_IRQ, GPIO_OPEN_READ, GPIO_IRQ_FALLING, TUNERDRIVER_GetIRQCallbackFunction(deviceID));
	
	if (vl_IRQ_fd < 0)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "configuring the CMOST IRQ");
#endif
		ret = OSAL_ERROR;
		goto exit;
	}
	else if (OSAL_OK != BSP_DeviceAddressSetIRQFileDescriptor(deviceID, vl_IRQ_fd))
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "configuring the CMOST IRQ");
#endif
		ret = OSAL_ERROR;
		goto exit;
	}
	else
	{
		/* Nothing to do */
	}

exit:
	return ret;
}

/**************************************
 *
 * BSP_RDSInterruptDeinit_CMOST
 *
 *************************************/
static tVoid BSP_RDSInterruptDeinit_CMOST(tU32 deviceID)
{

	tS32 vl_IRQ_fd;

	vl_IRQ_fd = BSP_DeviceAddressToIRQFileDescriptor(deviceID);

	if ((-1) != vl_IRQ_fd)
	{
		BSP_Linux_CloseGPIO(vl_IRQ_fd);
	}

}

#if defined (BSP_INCLUDE_SIGNALLING_GPIO)
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
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "configuring the Signalling GPIO");
#endif
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
#endif // BSP_INCLUDE_SIGNALLING_GPIO

/**************************************
 *
 * BSP_DeviceAddressToFileDescriptor
 *
 *************************************/
/*
 * Manage a simple associative array where array locations
 * containing Linux file descriptors are indexed by <device_address>
 *
 * Returns the file descriptor addressed by <device_address> or,
 * if not yet present, -1
 */
static tS32 BSP_DeviceAddressToFileDescriptor(tU32 deviceID)
{
	tU32 i;
	CMOSTAddressTy *addrp;
	tS32 ret = -1;

	for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
	{
		addrp = &BSP_LinuxDevice[i];
		if (addrp->isValid &&
			(addrp->CMOST_device_Id == 	deviceID ))
		{
			ret = addrp->CMOST_LinuxDevice_fd;
			goto exit;
		}
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
	tS32 ret = -1;

	for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
	{
		addrp = &BSP_LinuxDevice[i];
			if (addrp->isValid && (addrp->CMOST_device_Id == deviceID))
			{
				ret = addrp->CMOST_Irq_fd;
				goto exit;
			}
	}

exit:
	return ret;

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
	tS32 *ret = NULL;

	for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
	{
		addrp = &BSP_LinuxDevice[i];
		if (!addrp->isValid)
		{
			addrp->isValid = TRUE;
			addrp->CMOST_device_Id = deviceID;
			
			// for now set the CS as invalid.
			addrp->CMOST_cs_id = -1;
			ret = &addrp->CMOST_LinuxDevice_fd;
			goto exit;
		}
	}
	ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code

exit:
	return ret;
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
	tSInt ret = OSAL_ERROR;

	for (i = 0; i < BSP_MAX_TUNER_PER_BUS; i++)
	{
		addrp = &BSP_LinuxDevice[i];
		if (addrp->isValid && (addrp->CMOST_device_Id == deviceID))
		{
			addrp->CMOST_Irq_fd = vI_IRQ_fd;
				
			ret = OSAL_OK;
			goto exit;
		}
	}

	/* device not found ! */
	ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
exit:	
	return ret;
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
			(void)OSAL_pvMemorySet((tVoid *)addrp, 0x00, sizeof(CMOSTAddressTy));
			goto exit;
		}
	}
	ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code

exit:
	return;
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
 *  OSAL_OK in case of success or
 *  OSAL_ERROR in case of error
 */
tS32 BSP_BusConfig_CMOST(tU32 deviceID)
{
	tS32 *fdp;
	tS32 ret = OSAL_OK;
	t_i2c* i2cnb;
	tU16 addr;

	int ret_i2c;


	if(TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SPI bus not available in this configuration (M3 used)");
#endif

	}
	else
	{
		// I2C case
		
		fdp = BSP_DeviceAddressAdd(deviceID);
		if (fdp == NULL)
		{
			ret = OSAL_ERROR;
			goto exit;
		}

		/* open device */
//	    *fdp = tk_opn_dev((CONST UB *)((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->busName, TD_UPDATE);

		if(!strcmp(((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->busName, "i2c-0"))
		{
			i2cnb = i2c0_regs;
		}
		else if(!strcmp(((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->busName, "i2c-1"))
		{
			i2cnb = i2c1_regs;
		}
		else
		{
			/* Nothing to do */
			i2cnb = NULL;
			goto exit;
		}

		addr = (((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress >> 1);
		
		ret_i2c = i2c_service_init(I2C_CLOCK);
		if(ret_i2c != 0)
		{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "i2c_service_init return error %d", ret_i2c);
#endif
		}

		ret_i2c = i2c_open_port(i2cnb, 0, 0, I2C_BUSCTRLMODE_MASTER);
		if(ret_i2c != 0)
		{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "i2c_open_port return error %d", ret_i2c);
#endif
		}

		ret_i2c = i2c_set_port_mode(i2cnb, I2C_BUSCTRLMODE_MASTER);
		if(ret_i2c != 0)
		{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "i2c_set_port_mode return error %d", ret_i2c);
#endif
		}

		*fdp = (tS32)i2c_create_com(i2cnb, I2C_FAST_MODE, (tU16)(addr));
	}

	// init the IRQ if an IRQ is set
	if (NULL  != TUNERDRIVER_GetIRQCallbackFunction(deviceID))
	{
		if (BSP_RDSInterruptInit_CMOST(deviceID) != OSAL_OK)
			{
				ret = OSAL_ERROR;
				goto exit;
			}
	}

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
	t_i2c* i2cnb;

	if(!strcmp(((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->busName, "i2c-0"))
	{
		i2cnb = i2c0_regs;
	}
	else if(!strcmp(((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->busName, "i2c-1"))
	{
		i2cnb = i2c1_regs;
	}
	else
	{
		/* Nothing to do */
		i2cnb = NULL;
		goto exit;
	}

	i2c_reset_port(i2cnb);

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
	
	// init the IRQ if an IRQ is set
	if (NULL  != TUNERDRIVER_GetIRQCallbackFunction(deviceID))
	{
		BSP_RDSInterruptDeinit_CMOST(deviceID);
	}

	BSP_DeviceAddressRelease(deviceID);
	
#if defined(BSP_INCLUDE_SIGNALLING_GPIO)
	BSP_deinitSignallingGPIO();
#endif

exit:
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
	tSInt ret = OSAL_OK;
	
	if (BSP_Reset_CMOST(deviceID) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	return ret;
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
	tS32 ret = 0;
	static tS32 fd; //m//

	fd = BSP_DeviceAddressToFileDescriptor(deviceID);
	if (fd < 0)
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = -1;
		goto exit;
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
#endif

	if (TUNERDRIVER_GetBusType(deviceID) == BusSPI)
		{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SPI bus not available in this configuration (M3 used)");
#endif
		}
	else
		{
		ret = tnr_write(fd, ((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress, buf, (size_t)len);
		}

	if (ret < 0)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "writing data to CMOST");
#endif
	}

exit:
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
	tS32 ret = 0;
	tS32 fd;
//	tySpiCommunicationBus *spiBusConfig = NULL;
	tyI2cCommunicationBus *i2cBusConfig = NULL;

	fd = BSP_DeviceAddressToFileDescriptor(deviceID);
	if (fd < 0)
	{
		ret = -1;
		goto exit;
	}

	if(TUNERDRIVER_GetBusType(deviceID) == BusSPI)
		{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SPI bus not available in this configuration (M3 used)");
#endif
		}
	else
		{			
		i2cBusConfig = (tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID);
		ret = tnr_read(fd, i2cBusConfig->deviceAddress, buf, (size_t)len);
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
#endif

	if (ret < 0)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "reading data from CMOST");
#endif
	}
	else if (ret < (tS32)(len))
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "truncated read from CMOST");
#endif
		ret = -1;
		goto exit;
	}
	else
	{
		/* Nothing to do */
}

exit:
	return (ret);
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
        /* Not used on M3 context SPI not available*/
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
        /* Not used on M3 context SPI not available*/

	return(-1);

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
        /* Not used on M3 context SPI not available*/

	return (-1);
	
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
        /* Not used on M3 context SPI not available*/

	return(-1);

}
#endif // CONFIG_ETAL_SUPPORT_CMOST

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
 * Parameters:
 *  <GPIO_reset> - GPIO reset number
 *
 * Return:
 *  OSAL_OK    - no error
 *  OSAL_ERROR - error accessing the device
 *
 */
tSInt BSP_DeviceReset_HDRADIO(tU32 deviceID)
{
	/* Not used on M3 context */
	return OSAL_ERROR;
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
	/* Not used on M3 context */
	
}

/**************************************
 *
 * BSP_BusConfigSPIMode_HDRADIO
 *
 *************************************/
/*
 * Change the SPI communication bus mode
 * <deviceID> Indicate the SPI mode.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfigSPIMode_HDRADIO(tU32 deviceID)
{

	/* Not used on M3 context */
	return OSAL_ERROR;
}

/**************************************
 *
 * BSP_BusConfigSPIFrequency_HDRADIO
 *
 *************************************/
/*
 * Change the SPI communication bus frequency
 * <deviceID> - device ID
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfigSPIFrequency_HDRADIO(tU32 deviceID)
{
	/* Not used on M3 context */
	return OSAL_ERROR;
}


/**************************************
 *
 * BSP_BusConfigSPIDataSize_HDRADIO
 *
 *************************************/
/*
 * Change the SPI communication bus data size information
 * <deviceID> - device ID
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfigSPIDataSize_HDRADIO(tU32 deviceID, tBool vI_dataSizeIs32bits)
{
	/* Not used on M3 context */

	return OSAL_ERROR;
}


/**************************************
 *
 * BSP_BusConfig_HDRADIO
 *
 *************************************/
/*
 * Prepares the application for read/write access
 * to the HDRADIO through I2C or SPI bus
 * <deviceID> - device ID
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfig_HDRADIO(tU32 deviceID)
{
	/* Not used on M3 context */

	return OSAL_ERROR;
}


/**************************************
 *
 * BSP_BusUnconfig_HDRADIO
 *
 *************************************/
/*
 * Closes the Operating System resources previously
 * configured to access the HDRADIO through I2C or SPI bus
 * <deviceConfiguration> - Pointer to device configuration
 *
 * Returns:
 *  0  - success
 *  -1 - error accessing the kernel driver interface
 */
tSInt BSP_BusUnconfig_HDRADIO(tU32 deviceID)
{

        /* Not used on M3 context */

	return OSAL_ERROR;
}

/**************************************
 *
 * BSP_DeviceInit_HDRADIO
 *
 *************************************/
tSInt BSP_DeviceInit_HDRADIO(tU32 deviceID)
{
        /* Not used on M3 context */

	return OSAL_ERROR;
}

/**************************************
 *
 * BSP_DeviceDeinit_HDRADIO
 *
 *************************************/
tSInt BSP_DeviceDeinit_HDRADIO(tU32 deviceID)
{
        /* Not used on M3 context */

	return OSAL_ERROR;

}


/**************************************
 *
 * BSP_Write_HDRADIO
 *
 *************************************/
tS32 BSP_Write_HDRADIO(tU32 deviceID, tU8* buf, tU32 len)
{
        /* Not used on M3 context */

	return OSAL_ERROR;
}

/**************************************
 *
 * BSP_Read_HDRADIO
 *
 *************************************/
tS32 BSP_Read_HDRADIO (tU32 deviceID, tU8** pBuf)
{
        /* Not used on M3 context */
        
	return OSAL_ERROR;
}

/**************************************
 *
 * BSP_Raw_Read_HDRADIO
 *
 *************************************/
tS32 BSP_Raw_Read_HDRADIO(tU32 deviceID, tU8 *pBuf, tU32 len)
{
        /* Not used on M3 context */

	return OSAL_ERROR;
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
        /* Not used on M3 context */
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

#endif // CONFIG_BUILD_DRIVER &&  CONFIG_HOST_OS_TKERNEL

