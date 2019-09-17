//!
//!  \file 		bsp_linux.c
//!  \brief 	<i><b> BSP for Linux-based hardware resources access </b></i>
//!  \details   Primitives to abstract the access to some low level resources
//!             made available by the linux kernel
//!  \author 	Raffaele Belardi
//!

#include "target_config.h"
#include <tk/typedef.h>

#define ETAL_BSP_DEBUG_SPI_RW_WORD

// extern function which are needed :
extern  ER TNR_SPI_write_SSP( ID dd, W start, void* buf, W size );

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_HOST_OS_TKERNEL)

 #include "osal.h"
#include <unistd.h>
//m// #include <linux/spi/spidev.h>
//m// #include <sys/ioctl.h>
#include <poll.h>
#include "bsp_linux.h"
#include "common_trace.h"

//m//
#include <device/sdrvif.h>
#include <device/gpio_if.h>
#include <device/ssp_if.h>
/// static ID SPIdevid;

ID  flg_id_IrqRDS;


extern ID SPIdevid_0;


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

	ST_GPIO_WRITEPIN_PARAM 	param;
	ST_GPIO_ALTFUNC_PARAM   altf_param = {0};
#ifdef CONFIG_DEVICE_TUNER_STAR_ENABLE_RDS_IRQ
    ST_GPIO_WRITEPIN_PARAM  set_param;
#endif
	ST_GPIO_INT_PARAM set_iparam;
	tS32 vl_fd =-1;
	
	/* Disable Alternate Function  */
 	altf_param.pin 		=  atoi(gpio_name) ; // 
 	altf_param.altfunc 	= ALTFUNC_DISABLE;   // 
 	(void)st_gpio_set_altfunc(&altf_param);        // 


	param.pin		= atoi(gpio_name) ; 
	if (mode == GPIO_OPEN_WRITE)
	{
		/* Set Direction Output   */
		param.wdata 	= TRUE ; 
	}
	else
	{
		/* Set Direction Input   */
		param.wdata 	= FALSE;
	}
	(void)st_gpio_set_direction(&param);
	(void)tk_dly_tsk(20);

	set_iparam.pin = atoi(gpio_name) ; // ;

	switch (interrupt_mode)
	{
		case GPIO_IRQ_NONE:
			set_iparam.edge = INTEDGE_DISABLE;
			set_iparam.callback = (FP) NULL;  
			break;
		case GPIO_IRQ_RISING:
			set_iparam.edge = INTEDGE_RISING;
			set_iparam.callback = (FP) IRQCallbackFunction;   //lint !e960 - MISRA 11.1 - cast required for warnings
			break;
		case GPIO_IRQ_FALLING:
			set_iparam.edge = INTEDGE_FALLING;
			set_iparam.callback = (FP) IRQCallbackFunction;  //lint !e960 - MISRA 11.1 - cast required for warnings
			break;
		default:
			set_iparam.edge = INTEDGE_DISABLE;
			set_iparam.callback = (FP) NULL;  
	}


	if (GPIO_IRQ_NONE != interrupt_mode)
	{
		(void)st_gpio_set_interrupt(&set_iparam); 
		(void)tk_dly_tsk(20); 
	}

	vl_fd = atoi(gpio_name);


	return(vl_fd);

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
//	tChar value[2];
	tS32 ret;

#if 1
	ST_GPIO_WRITEPIN_PARAM  set_param;

	set_param.pin = fd;  //  13;  //atoi(gpio_name) ; 
	if (req_value)   
	{
		//	value[0] = '1';
			set_param.wdata = 1;
	}
	else
	{
		//	value[0] = '0';
			set_param.wdata = 0;
	}
	// value[1] = '\0';


//	set_param.wdata = value[0];
	ret = st_gpio_set_data(&set_param); 
	
	// wait some time for the GPIO to be settled ? 
	(void)OSAL_s32ThreadWait_us(10);
	
//	printf("BSP_Linux_WriteGPIO writing GPIO :  fd = %d , value = %d\n", fd, req_value);

	//tk_dly_tsk(20); //m// 

#else
	/*
	 * This is equivalent to 
	 *  # echo 1 > /sys/class/gpio/gpio<gpio_name>/value, or
	 *  # echo 0 > /sys/class/gpio/gpio<gpio_name)/value
	 * from the shell
	 */
	ret = write(fd, value, 2);
#endif // 1
	if (ret != E_OK)
//m//	if (ret < 2)
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
//m//
#if 1

	tBool value;
	tS32 ret;

    ST_GPIO_READPIN_PARAM   get_param;
    get_param.pin = fd;
    ret = st_gpio_get_data(&get_param); 
    value = get_param.rdata ;

#else
	/*
	 * fundamental, otherwise reads on the GPIO descriptor
	 * following the firs one fail due to end of file!
	 */
	lseek(fd, 0, SEEK_SET);

	/*
	 * This is equivalent to 
	 *  # cat /sys/class/gpio/gpio<gpio_name>/value
	 * from the shell
	 */
	ret = read(fd, value, 1);
#endif // 0
	 if (ret < 0)
//m//	if (ret <= 0)
	{
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "reading from GPIO");
	}

	// TODO if there was an error reading the GPIO we should notify the caller
	//m// return value[0] == '0' ? 0 : 1;
	return value;
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
	//m//
//	ER err = E_OK;
	tS32 vl_deviceID = -1; 
	
	//////// prova: write ssp0 //////////////////
	(void)ST_SSP_DrvInit();
	
	 /* open ssp device */

	vl_deviceID = tk_opn_dev((CONST UB *) dev_name,TD_UPDATE);
 

	return(vl_deviceID);

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
	// T-Kernel : 
	(void)tk_cls_dev(fd, 0);
	
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
	 // ST_SSP_DATA_SPCSIZE_PARAM buf;
	 ST_SSP_DATA_PARAM buf;

	 UW *pl_BufRead = NULL;
	 UW  *pl_BufWrite = NULL;
 	 UB  *pl_BufRead_UB = NULL;
	 UB  *pl_BufWrite_UB = NULL;
	 tS32 vl_len = 0;
	 tS32 vl_nbWord = 0;
	 tS32 vl_index;


	 
#ifdef ETAL_BSP_DEBUG_SPI_RW_WORD

		 // we transfer by block with a data size of 32 bits
		 // handle a format transfer from byte to word
		 //

		 if (true == vI_dataSize32bits)
		 {		
			 // w_len is a number of byte
			 // let's find the len in number of word
			 //
			 vl_nbWord = (len  / sizeof(UW));
			 
			 if ((vl_nbWord * sizeof(UW)) != len)
			 {
				 vl_nbWord = vl_nbWord + 1;
			 }
		 
			 // allocate a rounded number of word
			 vl_len = vl_nbWord *  sizeof(UW);
		 
			 pl_BufRead =  (UW *) OSAL_pvMemoryAllocate(vl_len);

			 if (NULL == pl_BufRead)
			 {
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
				 COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SSP/SPI	: allocation error pl_BufRead CODE=%08x\n");
#endif
			 
				 goto exit ;
			 }
			 
			 pl_BufWrite =	(UW *) OSAL_pvMemoryAllocate(vl_len);	 

			 if (NULL == pl_BufWrite)
			 {
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
				 COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SSP/SPI	: allocation error pl_BufWrite CODE=%08x\n");
#endif

				// free buf Read
				OSAL_vMemoryFree(pl_BufRead);
				goto exit;
			 }	 

			 (void)OSAL_pvMemorySet(pl_BufWrite, 0, vl_len);
			 (void)OSAL_pvMemorySet(pl_BufRead, 0, vl_len);

			 // copy the data
			 //  OSAL_pvMemoryCopy(pl_BufWrite, buf_wr, len);
			 // copy the data
			 // if they exist

			if (NULL != buf_wr)
			{
				vl_index = 0;

				while (vl_index < len)
				{
					*(pl_BufWrite+(vl_index/sizeof(UW))) |= ((UW) buf_wr[vl_index]) << (8*(3-(vl_index%sizeof(UW))));
					vl_index++;
				}
			}

		 	buf.trans_len  = vl_nbWord;  
			buf.data_size  = vl_nbWord;
			buf.write_data = pl_BufWrite; 
			buf.read_data  = pl_BufRead; 
			
			(void)TNR_SPI_write_SSP(fd, ST_SSP_WSL_DATA, &buf, sizeof(buf));

			// copy the data
			// if they exist

			if (NULL != buf_rd)
			{
				vl_index = 0;

				while (vl_index < len)
				{
					buf_rd[vl_index] = (tU8) ((*(pl_BufRead+(vl_index/sizeof(UW)))) >> (8*(3-(vl_index%sizeof(UW)))));
					vl_index++;
				}
			}
			
			// free buf Read
			OSAL_vMemoryFree(pl_BufRead);
			OSAL_vMemoryFree(pl_BufWrite);		

	 	}
		else
		{
			if (NULL != buf_rd)
			{
				pl_BufRead_UB = buf_rd;
			}
			else
			{
				pl_BufRead_UB =  (UB *) OSAL_pvMemoryAllocate(len);

				 if (NULL == pl_BufRead_UB)
				 {
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
					 COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SSP/SPI	: allocation error pl_BufRead_UB CODE=%08x\n");
#endif
				 
					 goto exit ;
				 }
			}

			if (NULL != buf_wr)
			{
				pl_BufWrite_UB = buf_wr;
			}
			else
			{
				pl_BufWrite_UB =	(UB *) OSAL_pvMemoryAllocate(len);	 

			 	if (NULL == pl_BufWrite_UB)
			 	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
					COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SSP/SPI	: allocation error pl_BufWrite_UB CODE=%08x\n");
#endif
					// free buf Read if needed
					if (NULL == buf_rd)
					{
						OSAL_vMemoryFree(pl_BufRead_UB);
					}
					goto exit;
			 	}

				// set 00 in writte data
				(void)OSAL_pvMemorySet(pl_BufWrite_UB, 0, len);
			}	 

			
			// reset the read
			(void)OSAL_pvMemorySet(pl_BufRead_UB, 0, len);

			 buf.trans_len  = len;  
			 buf.data_size  = len;
 			buf.write_data = pl_BufWrite_UB; 
			buf.read_data  = pl_BufRead_UB; 

	 		(void)TNR_SPI_write_SSP(fd, ST_SSP_WSL_DATA, &buf, sizeof(buf));
	 
			// no data copy needed : all is fine if required
			// if they exist
	
			// free buf Read if needed
			if (NULL == buf_rd)
			{
				OSAL_vMemoryFree(pl_BufRead_UB);
			}

			// free buf Read if needed
			if (NULL == buf_wr)
			{
				OSAL_vMemoryFree(pl_BufWrite_UB);
			}

}

 
#else // ETAL_BSP_DEBUG_SPI_RW_WORD
		
		// we want read and write potentially
		 buf.trans_len  = len;  
		 buf.write_data = buf_wr; 
		 buf.read_data  = buf_rd;
		 buf.data_size  = len;		 
		 TNR_SPI_write_SSP(fd, ST_SSP_WSL_DATA, &buf, sizeof(buf));
#endif

exit:
	return;
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
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "Waiting for GPIO IRQ");
	}
}

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_HOST_OS_TKERNEL

