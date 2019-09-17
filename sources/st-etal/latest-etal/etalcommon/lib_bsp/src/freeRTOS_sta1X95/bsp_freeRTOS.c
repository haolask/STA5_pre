//!
//!  \file 		bsp_linux.c
//!  \brief 	<i><b> BSP for Linux-based hardware resources access </b></i>
//!  \details   Primitives to abstract the access to some low level resources
//!             made available by the linux kernel
//!  \author 	Raffaele Belardi
//!

#include "target_config.h"
#include "FreeRTOS.h"

#define ETAL_BSP_DEBUG_SPI_RW_WORD

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_HOST_OS_FREERTOS)

 #include "osal.h"
#include <unistd.h>
//m// #include <linux/spi/spidev.h>
//m// #include <sys/ioctl.h>
//#include <poll.h>
#include "bsp_linux.h"
#include "common_trace.h"

//m//
#include "sta_gpio.h"
/// static ID SPIdevid;

/*
 * These should normally not be changed
 */
#define BSP_STRING_BUF_SIZE          64
#define BSP_A2_GPIO_SYS_INTERFACE    "/sys/class/gpio"
#define BSP_A2_DEV_INTERFACE         "/dev"

tChar BSP_stringBuf[BSP_STRING_BUF_SIZE];

/**************************************
 *
 * BSP_Linux_OpenGPIO
 *
 *************************************/
/*
 * Prepare the kernel to access a GPIO from userspace
 *
 * <mode> is GPIO_OPEN_WRITE or GPIO_OPEN_READ
 * <interrupt_mode> is GPIO_IRQ_NONE, GPIO_IRQ_RISING or GPIO_IRQ_FALLING
 *
 * Returns:
 * a valid (>=0) file description if all is ok
 * -1 in case of error
 */
tS32 BSP_Linux_OpenGPIO(tChar *gpio_name, tBool mode, tU32 interrupt_mode, tVoid *IRQCallbackFunction)
{

	struct gpio_config pin;
	int err = 0;
	tS32 ret_val = atoi(gpio_name);

	/* Set the direction of GPIO */
	if (mode == GPIO_OPEN_WRITE)
	{
		/* Set Direction Output   */
		pin.direction = GPIO_DIR_OUTPUT; 
	}
	else
	{
		/* Set Direction Input   */
		pin.direction = GPIO_DIR_INPUT;
	}

	/* Set the trigger */
	switch (interrupt_mode)
	{
		case GPIO_IRQ_NONE:
			pin.trig = GPIO_TRIG_DISABLE;
			break;
		case GPIO_IRQ_RISING:
			pin.trig = GPIO_TRIG_RISING_EDGE;
			break;
		case GPIO_IRQ_FALLING:
			pin.trig =  GPIO_TRIG_FALLING_EDGE;
			break;
		default:
			pin.trig = GPIO_TRIG_DISABLE;
			break;
	}
	
	/* Disable the Alternate function, set the SW mode */
	pin.mode = GPIO_MODE_SOFTWARE;
	
	/* Set the level for trigger */
	pin.level = GPIO_LEVEL_PULLDOWN;

	err = gpio_set_pin_config(atoi(gpio_name), &pin);
	if(err)
	{
		COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, TR_CLASS_BSP, "configure GPIO %d failed", atoi(gpio_name));
		ret_val = -1;
	}


	if(pin.trig != GPIO_TRIG_DISABLE)
	{
		err = gpio_request_irq(atoi(gpio_name), IRQCallbackFunction);
		if(err)
		{
			COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, TR_CLASS_BSP, "request IRQ for GPIO %d failed", atoi(gpio_name));
			ret_val = -1;
		}
		
	}

	return(ret_val);
}


/**************************************
 *
 * BSP_Linux_WriteGPIO
 *
 *************************************/
/*
 * Sets the value of the GPIO previously opened with BSP_Linux_OpenGPIO
 * and identified by <fd> to <req_value>
 */
tVoid BSP_Linux_WriteGPIO(tS32 fd, tBool req_value)
{
	int ret;
	unsigned int pin = (unsigned int)fd;

	if (req_value)   
	{
		ret = gpio_set_gpio_pin(pin);
	}
	else
	{
		ret = gpio_clear_gpio_pin(pin);
	} 
	
	(void)OSAL_s32ThreadWait_us(10);
	
	if (ret != 0)
	{
		COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, TR_CLASS_BSP, "unexpected return in write to GPIO");
	}

}

/**************************************
 *
 * BSP_Linux_ReadGPIO
 *
 *************************************/
/*
 * Returns the value of the GPIO previously opened with BSP_Linux_OpenGPIO
 * and identified by <fd>
 */
tBool BSP_Linux_ReadGPIO(tS32 fd)
{
	tBool ret_value;
	int value;
	int ret;
	unsigned int pin = (unsigned int)fd;

	ret = gpio_read_gpio_pin(pin, &value);

	if (value == 0)
	{
		ret_value = 0;
	}else{
		ret_value = 1;
	}

	if (ret != 0)
	{
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "reading from GPIO");
	}

	return ret_value;
}

/**************************************
 *
 * BSP_Linux_CloseGPIO
 *
 *************************************/
/*
 * Closes the GPIO previously opened with BSP_Linux_OpenGPIO
 *
 * Not mandatory but frees the associated file descriptor
 */
tVoid BSP_Linux_CloseGPIO(tS32 fd)
{
#if 0
	if (fd < 0)
	{
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "invalid file descriptor");
		return;
	}
	close(fd);
#endif
}


/**************************************
 *
 * BSP_Linux_OpenDevice 
 *
 *************************************/
/*
 * Prepares the access to the device specified in <dev_name>
 * and accessible through the /dev/ Linux interface
 *
 * Returns:
 * the device file descriptor, or
 * -1 in case of error
 */
tS32 BSP_Linux_OpenDevice(tChar *dev_name)
{

	/* SPI not managed inside M3 */
	return(-1);

}


/**************************************
 *
 * BSP_Linux_CloseDevice 
 *
 *************************************/
/*
 * Closes the device specified in <dev_name>
 * and accessible through the /dev/ Linux interface
 *
 * Returns:
 * 0  no error
 * -1 in case of error
 */
tS32 BSP_Linux_CloseDevice(tS32 fd)
{
	/* SPI not managed inside M3 */
	
	return 0;
}


/**************************************
 *
 * BSP_Linux_TransferSpi 
 *
 *************************************/
/*
 * Performs a full-duplex data transfer over the SPI
 */
 
//static UB g_write_data[300]; // g_write_data[260];
//static UB g_read_data[300];  // g_read_data[260];

tVoid BSP_Linux_TransferSpi(tS32 fd, tU8 *buf_wr, tU8 *buf_rd, tU32 len, tBool vI_dataSize32bits)
{
	/* SPI not managed inside M3 */
	return;
}

/**************************************
 *
 * BSP_Linux_WaitForInterrupt 
 *
 *************************************/
tVoid BSP_Linux_WaitForInterrupt(tS32 fd)
{
	/* SPI not managed inside M3 */
	return;

}

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_HOST_OS_TKERNEL

