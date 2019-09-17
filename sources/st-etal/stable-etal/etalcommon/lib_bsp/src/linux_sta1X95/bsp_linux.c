//!
//!  \file 		bsp_linux.c
//!  \brief 	<i><b> BSP for Linux-based hardware resources access </b></i>
//!  \details   Primitives to abstract the access to some low level resources
//!             made available by the linux kernel
//!  \author 	Raffaele Belardi
//!

#include "target_config.h"

#if (defined(CONFIG_COMM_DRIVER_EMBEDDED) || defined(CONFIG_COMM_DRIVER_EXTERNAL)) && defined(CONFIG_HOST_OS_LINUX)
#include "osal.h"
#endif

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_HOST_OS_LINUX)

	#include <unistd.h>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <poll.h>
#include "bsp_linux.h"
#include "common_trace.h"
#include "bsp_trace.h"

/*
 * These should normally not be changed
 */
#define BSP_STRING_BUF_SIZE          64
#define BSP_A2_GPIO_SYS_INTERFACE    "/sys/class/gpio"
#define BSP_A2_DEV_INTERFACE         "/dev"

static tChar BSP_stringBuf[BSP_STRING_BUF_SIZE];

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
 * a valid (>=0) file descripto if all is ok
 * -1 in case of error
 */
tS32 BSP_Linux_OpenGPIO(tChar *gpio_name, tBool mode, tU32 interrupt_mode, tVoid *IRQCallbackFunction)
{
	tS32 fd, fd1, fd2;
	size_t size;
	ssize_t ssize;
	tChar direction[4];
	tS32 open_mode;
	tChar irq[8];

	/*
	 * First make sure the gpio is exported by the kernel interface.
	 *
	 * This is equivalent to 
	 *  # echo gpio_name > /sys/class/gpio/export
	 * from the shell
	 *
	 */
	if (OSAL_s32NPrintFormat(BSP_stringBuf, BSP_STRING_BUF_SIZE, "%s/export", BSP_A2_GPIO_SYS_INTERFACE) < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "library error");
		return -1;
	}
	fd = open(BSP_stringBuf, O_WRONLY);
	if (fd < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "opening GPIO export for %s", gpio_name);
		return -1;
	}
	size = strlen(gpio_name);
	ssize = write(fd, gpio_name, size);
	if (((size_t)ssize < size))
	{
		BSP_tracePrintError(TR_CLASS_BSP, "configuring GPIO export for %s", gpio_name);
		(LINT_IGNORE_RET)close(fd);
		return -1;
	}
	if (close(fd) != 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "closing GPIO fd export for %s", gpio_name);
		return -1;
	}

	/*
	 * Now configure the gpio for output.
	 *
	 * This is equivalent to 
	 *  # echo out > /sys/class/gpio/gpio<gpio_name>/direction
	 * from the shell
	 *
	 */

	if (OSAL_s32NPrintFormat(BSP_stringBuf, BSP_STRING_BUF_SIZE, "%s/gpio%s/direction", BSP_A2_GPIO_SYS_INTERFACE, gpio_name) < 0 )
	{
		BSP_tracePrintError(TR_CLASS_BSP, "library error");
		return -1;
	}
	fd1 = open(BSP_stringBuf, O_WRONLY);
	if (fd1 < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "configuring GPIO direction for %s", gpio_name);
		return -1;
	}
	switch (mode)
	{
		case GPIO_OPEN_WRITE:
			strcpy(direction, "out");
			break;
		case GPIO_OPEN_READ:
			strcpy(direction, "in");
			break;
		default:
			BSP_tracePrintSysmin(TR_CLASS_BSP, "illegal GPIO direction");
			(LINT_IGNORE_RET)close(fd1);
			return -1;
	}
	size = strlen(direction);
	ssize = write(fd1, direction, size);
	if ((ssize < 0) || ((size_t)ssize < size))
	{
		BSP_tracePrintError(TR_CLASS_BSP, "configuring GPIO for %s", gpio_name);
		(LINT_IGNORE_RET)close(fd1);
		return -1;
	}
	if (close(fd1) != 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "closing GPIO fd for %s", gpio_name);
		return -1;
	}

	if(IRQCallbackFunction != NULL)
	{
		/*
		 * Configure interrupt mode
		 * This is equivalent to
		 *  # echo "rising" > /sys/class/gpio/gpio<gpio_name>/edge
		 * from the shell
		 *
		 */
		if (OSAL_s32NPrintFormat(BSP_stringBuf, BSP_STRING_BUF_SIZE, "%s/gpio%s/edge", BSP_A2_GPIO_SYS_INTERFACE, gpio_name) < 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "library error");
			return -1;
		}
		fd1 = open(BSP_stringBuf, O_WRONLY);
		if (fd1 < 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "configuring GPIO edge for %s", gpio_name);
			return -1;
		}
		switch (interrupt_mode)
		{
			case GPIO_IRQ_NONE:
				strcpy(irq, "none");
				break;
			case GPIO_IRQ_RISING:
				strcpy(irq, "rising");
				break;
			case GPIO_IRQ_FALLING:
				strcpy(irq, "falling");
				break;
			default:
				BSP_tracePrintSysmin(TR_CLASS_BSP, "illegal GPIO interrupt mode");
				(LINT_IGNORE_RET)close(fd1);
				return -1;
		}
		size = strlen(irq);
		ssize = write(fd1, irq, size);
		if ((ssize < 0) || ((size_t)ssize < size))
		{
			BSP_tracePrintError(TR_CLASS_BSP, "configuring GPIO interrupt for %s", gpio_name);
			(LINT_IGNORE_RET)close(fd1);
			return -1;
		}
		if (close(fd1) != 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "closing GPIO fd interrupt for %s", gpio_name);
			return -1;
		}
	}

	/*
	 * prepare the file descriptor to set the GPIO value
	 */
	if (OSAL_s32NPrintFormat(BSP_stringBuf, BSP_STRING_BUF_SIZE, "%s/gpio%s/value", BSP_A2_GPIO_SYS_INTERFACE, gpio_name) < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "library error");
		return -1;
	}
	if (mode == GPIO_OPEN_WRITE)
	{
		open_mode = O_WRONLY;
	}
	else
	{
		open_mode = O_RDONLY;
	}
	fd2 = open(BSP_stringBuf, open_mode);
	if (fd2 < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "accessing GPIO value for %s", gpio_name);
		return -1;
	}

	return fd2;
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
	tChar value[2];
	ssize_t ssize;

	if (fd < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "Invalid file descriptor in BSP_Linux_WriteGPIO");
		return;
	}

	if (req_value)
	{
		value[0] = '1';
	}
	else
	{
		value[0] = '0';
	}
	value[1] = '\0';

	/*
	 * This is equivalent to 
	 *  # echo 1 > /sys/class/gpio/gpio<gpio_name>/value, or
	 *  # echo 0 > /sys/class/gpio/gpio<gpio_name)/value
	 * from the shell
	 */
	ssize = write(fd, value, 2);

	if (ssize < (ssize_t)2)
	{
		BSP_tracePrintSysmin(TR_CLASS_BSP, "unexpected return in write to GPIO");
	}

	// let's wait some ms before 
	// wait some time for the GPIO to be settled ? 
	(void)OSAL_s32ThreadWait_us(10);
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
	tChar value[2] = "0";
	ssize_t ssize;

	if (fd < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "Invalid file descriptor in BSP_Linux_ReadGPIO");
		return 0;
	}

	/*
	 * fundamental, otherwise reads on the GPIO descriptor
	 * following the first one fail due to end of file!
	 */
	(LINT_IGNORE_RET) lseek(fd, 0, SEEK_SET);

	/*
	 * This is equivalent to 
	 *  # cat /sys/class/gpio/gpio<gpio_name>/value
	 * from the shell
	 */
	ssize = read(fd, value, 1);

	if (ssize <= (ssize_t)0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "reading from GPIO");
	}

	// TODO if there was an error reading the GPIO we should notify the caller
	return (tBool)(value[0] == '0' ? 0 : 1);
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
	if (fd < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "invalid file descriptor");
		return;
	}
	if (close(fd) != 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "closing file descriptor");
	}
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
	tS32 ret;

	if (OSAL_s32NPrintFormat(BSP_stringBuf, BSP_STRING_BUF_SIZE, "%s/%s", BSP_A2_DEV_INTERFACE, dev_name) < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "library open error");
		return -1;
	}
	ret = open(BSP_stringBuf, O_RDWR);
	if (ret < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "opening Linux device %s/%s", BSP_A2_DEV_INTERFACE, dev_name);
		return -1;
	}
	return ret;
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
	tS32 ret;

	if (fd < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "invalid file descriptor to close");
		return -1;
	}
	
	ret = close(fd);
	if (ret < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "closing Linux device");
		return -1;
	}
	return 0;
}

/**************************************
 *
 * BSP_Linux_TransferSpi 
 *
 *************************************/
/*
 * Performs a full-duplex data transfer over the SPI
 *
 * WARNING: according to the Linux kernel documentation
 * (Documentation/spi/spidev), there is a limit on the
 * number of bytes that can be transferred on each ioctl:
 *
 * " - There's a limit on the number of bytes each I/O request can transfer
 *     to the SPI device.  It defaults to one page, but that can be changed
 *     using a module parameter."
 *
 * The page size on ARMv7 seems to be 4Kbyte; thus the call to ioctl
 * will fail for sizes bigger than this limit (and no data will be
 * transferred on the bus)
 */
tVoid BSP_Linux_TransferSpi(tS32 fd, tU8 *buf_wr, tU8 *buf_rd, tU32 len, tBool vI_dataSize32bits)
{
    struct spi_ioc_transfer	xfer[1];
    int ret;

	 // param not used in Linux
	(tVoid) vI_dataSize32bits;
	

	OSAL_pvMemorySet(xfer, 0x00, sizeof(xfer));
	if (buf_rd)
	{
		OSAL_pvMemorySet((tPVoid)buf_rd, 0x00, sizeof(tU8) * len);
	}
	xfer[0].tx_buf = (tU32) buf_wr;
	xfer[0].rx_buf = (tU32) buf_rd;
	xfer[0].len = len;
	xfer[0].bits_per_word = 8;
	
		
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
    if (ret == -1)
    {
        switch (errno)
        {
            case EBADF:
                BSP_tracePrintError(TR_CLASS_BSP, "SPI data tranfer error EBADF (%d)", errno);
                break;
            case EFAULT:
                BSP_tracePrintError(TR_CLASS_BSP, "SPI data tranfer error EFAULT (%d)", errno);
                break;
            case EINVAL:
                BSP_tracePrintError(TR_CLASS_BSP, "SPI data tranfer error EINVAL (%d)", errno);
                break;
            case ENOTTY:
                BSP_tracePrintError(TR_CLASS_BSP, "SPI data tranfer error ENOTTY (%d)", errno);
                break;
            default:
                BSP_tracePrintError(TR_CLASS_BSP, "SPI data tranfer error (%d)", errno);
                break;
        }
		ASSERT_ON_DEBUGGING(0);
    }
}

/**************************************
 *
 * BSP_Linux_WaitForInterrupt 
 *
 *************************************/
tVoid BSP_Linux_WaitForInterrupt(tS32 fd)
{
	struct pollfd param[1];

	param[0].fd = fd;
	param[0].events = POLLPRI;
	param[0].revents = 0;

	if (poll(param, 1, -1) <= 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "Waiting for GPIO IRQ");
	}
}

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_HOST_OS_LINUX

#if (defined(CONFIG_COMM_DRIVER_EMBEDDED) || defined(CONFIG_COMM_DRIVER_EXTERNAL)) && defined(CONFIG_HOST_OS_LINUX)

#if defined(CONFIG_DIGITAL_AUDIO) || defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
tVoid BSP_Linux_SystemCmd(tChar *cmd_line)
{
	system(cmd_line);
}
#endif

#endif // CONFIG_COMM_DRIVER_EMBEDDED||CONFIG_COMM_DRIVER_EXTERNAL && CONFIG_HOST_OS_LINUX
