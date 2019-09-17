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

/*+DEBUG*/
// Switch to activate for CMOST SPI problem DEBUG
#undef ETAL_BSP_DEBUG_SPI
#define ETAL_BSP_DEBUG_SPI_RW_WORD

/*-DEBUG*/

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_BOARD_ACCORDO2) && defined (CONFIG_HOST_OS_TKERNEL)

#include <tk/typedef.h>

 #include "osal.h"
#include <sys/ioctl.h>
#include <unistd.h>

#include "bsp_sta1095evb.h"
#include "bsp_linux.h"

#include "common_trace.h"
#include "bsp_trace.h"

#if (defined BOOT_READ_BACK_AND_COMPARE_CMOST) && (CONFIG_TRACE_CLASS_BSP < TR_LEVEL_COMPONENT)
	#error "BOOT_READ_BACK_AND_COMPARE_CMOST requires CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT"
 #error "for complete output"
#endif

#include <errno.h>
#include <device/sdrvif.h>
#include <device/gpio_if.h>
#include <device/ssp_if.h>


/**************I2C****************/
#include <device/i2c_if.h>





IMPORT ID i2c_devid;


ID SPIdevid_0;


//***************************
// *
// * Local functions
// *
// **************************

static void callback_master(UINT interrupt, ER status);

ER TNR_SPI_write_SSP( ID dd, W start, void* buf, W size );
extern  void TNR_SPI_setconf_SSP(ID id, tU32 vI_speed, tBool vI_cpha1cpol1, tBool vI_dataSize32bits);
//static ER TNR_SPI_read_SSP( ID dd, W start, void* buf, W size);
#if 0
static void TNR_Clear_Set_RSTN (void); 
#endif

#endif
static tSInt BSP_DeviceAddressSetCs(tU32 deviceID, tS32 vI_CS_Id);
static tS32 BSP_DeviceAddressToChipSelectDescriptor(tU32 deviceID);

static tSInt BSP_DeviceAddressSetIRQFileDescriptor(tU32 deviceID, tS32 vI_IRQ_fd);
static tS32 BSP_DeviceAddressToIRQFileDescriptor(tU32 deviceID);


static int spi_write(tS32 device_Id, tU8 *w_buf, tS32 w_len, tBool vI_dataSize32bits);
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
static int spi_read(tS32 device_Id, tU8 *r_buf, tS32 r_len, tBool vI_dataSize32bits );
#endif

static tS32 BSP_DeviceAddressToFileDescriptor(tU32 deviceID);

static ER _i2c_read( ID dd, W start, void* buf, W size);
static ER _i2c_write( ID dd, W start, void* buf, W size);
static ER _i2c_sample_set_config(ID id);
static ER _i2c_data_write(ID id, UB* buffer, INT i,  UINT   slv_add);
static ER _i2c_data_read(ID id, INT i, UB* read_data,  UINT   slv_add);
static void  _i2c_set_cond_f(ID id);
static int tnr_write(tS32 fd, tS32 device_address, tU8 *w_buf, tS32 w_len );
static int tnr_read(tS32 fd, tS32 device_address, tU8 *r_buf, tS32 r_len );

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
static tSInt BSP_BusConfig_MDR(tU32 deviceID);
static tVoid BSP_SteciOpenBOOT_GPIO_MDR();
#endif
static tSInt BSP_Reset_CMOST(tU32 deviceID);
static tSInt BSP_RDSInterruptInit_CMOST(tU32 deviceID);
static tVoid BSP_RDSInterruptDeinit_CMOST(tU32 deviceID);
static tS32 *BSP_DeviceAddressAdd(tU32 deviceID);
static tVoid BSP_DeviceAddressRelease(tU32 deviceID);

#if ((defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)) | (defined BOOT_READ_BACK_AND_COMPARE_CMOST)
tVoid BSP_PrintBufNoSpace(tS32 level, tBool useI2C, tUChar *buf, tS32 len, BSPPrintOperationTy op);
#endif

#define CPSDVR_MIN 0x02
#define CPSDVR_MAX 0xFE
#define SCR_MIN 0x00
#define SCR_MAX 0xFF
#define TNR_SPI_SSP_MAX_CLOCK	102400000
#define TNR_SPI_SSP_MAX_SPEED	4000000

#define BSP_SPI_BUF_SIZE	300

//static UB g_read_data[BSP_SPI_BUF_SIZE];  // g_read_data[260];


/*!
 * @brief	  		callback_master:
 * 			       Callback routine. STUB
 * @param     :
 * @note 			: 
 *
 */
static void callback_master(UINT interrupt, ER status)
{
	
}


/*!
 * @brief	  _i2c_read:
 *      
 * @param    :  
 * @note     :  
 *
 */
static ER _i2c_read( ID dd, W start, void* buf, W size)
{   W  asize;
    return tk_srea_dev(dd, start, buf, size, &asize);
}


/*!
 * @brief	  		_i2c_write:
 * 			       This routine set the I2C start condition.
 * @param          	: id - I2X Device descriptor 	
 * @note  
 *
 */
static ER _i2c_write( ID dd, W start, void* buf, W size)
{   W  asize;
    return tk_swri_dev(dd, start, buf, size, &asize);
}



/*!
 * @brief	  		_i2c_sample_set_config:
 * 			       This routine csets I2C configuration.
 * @param     : id - I2C Device descriptor 	
 * @note  
 *
 */
static ER _i2c_sample_set_config(ID id)
{   ER err;
    ST_I2C_CONFIG_PARAM buf = { ST_I2C_OPE_MAS,
						        ST_I2C_ADDR_NORMAL,
						        ST_I2C_SPEED_FAST, //400Kbps  //ST_I2C_SPEED_STD,   // 100Kbps  
						        0,
						        ST_I2C_SGC_TRANSP,
						        FALSE,
						        FALSE,
						        FALSE,
						        ST_I2C_NORMAL,
						        ST_I2C_FILTER_NO,
						        &callback_master,
						        0x00 };
    err = _i2c_write(id, ST_I2C_WSL_CONFIG, &buf, sizeof(buf));

	
    return err;
}


/*!
 * @brief	  		TNR_SPI_write_SSP:
 * 			       This routine writes cmd or data on SSP device.
 * @param          	: id - SSP Device descriptor 	
 * @param          	: start - start command 
 * @param          	: buf   - pointer to data that will be written on SSP dev 
 * @param          	: size - buffer size
 * @param          	: data - Identifies if the request is a command or a data write
 * @note 			: 
 *
 */
 ER TNR_SPI_write_SSP( ID dd, W start, void* buf, W size )
{
    ER err;
    W  asize;
    
    err = tk_swri_dev(dd, start, buf, size, &asize);
  	
    return err;
}
#if 0
static ER TNR_SPI_read_SSP( ID dd, W start, void* buf, W size)
{
    ER err;
    W  asize;
    
    err = tk_srea_dev(dd, start, buf, size, &asize);
    
    return err;
}

#endif


/*!
 * @brief	  		TNR_SPI_setconf_SSP:
 * 			       This routine csets SPI configuration.
 * @param     : id - SPI Device descriptor 	
 * @note  
 *
 */
void TNR_SPI_setconf_SSP(ID id, tU32 vI_speed, tBool vI_cpha1cpol1, tBool vI_dataSize32bits)
{
    ER err = E_OK;
	UINT vl_cpsdvsr;
	UINT vl_scr = 0;

#ifdef ETAL_BSP_DEBUG_SPI_RW_WORD

	ST_SSP_CONFIG_PARAM vl_configParam = {
		ST_SSP_OPE_MAS, 	/* Master or Slave mode select */
		ST_SSP_SPH1,		//m//  /* Motorola SSP Clock phase */
		ST_SSP_SPO_HIGH,	//m//  /* SSPCLK Clock Polarity */
		0x1F, 				/* Data Size Select */
		ST_SSP_TXFIFO_2,	/* Transmit Interrupt FIFO Level Select */
		ST_SSP_RXFIFO_2,	// ST_SSP_RXFIFO_4,	/* Recieve Interrupt FIFO Level Select */
		ST_SSP_ENDIAN_MSB,	/* Transmit Endian Format */
		ST_SSP_ENDIAN_MSB,	/* Receive Endian Format */
		FALSE,				/* Slave-mode Output Disable */
		ST_SSP_NORMAL,		/* Loopback Mode */
		0,					/* Serial Clock Rate */
		1,  				// 6, // 1, 	/* Clock Prescale divisor */
		FALSE,				/* Transmit DMA Enable */
		FALSE				/* Receive DMA Enable */
	};
#else
ST_SSP_CONFIG_PARAM vl_configParam = {
	ST_SSP_OPE_MAS, 	/* Master or Slave mode select */
	ST_SSP_SPH1,		//m//  /* Motorola SSP Clock phase */
	ST_SSP_SPO_HIGH,	//m//  /* SSPCLK Clock Polarity */
	0x07,				/* Data Size Select */
	ST_SSP_TXFIFO_4,	/* Transmit Interrupt FIFO Level Select */
	ST_SSP_RXFIFO_2,	// ST_SSP_RXFIFO_4, /* Recieve Interrupt FIFO Level Select */
	ST_SSP_ENDIAN_MSB,	/* Transmit Endian Format */
	ST_SSP_ENDIAN_MSB,	/* Receive Endian Format */
	FALSE,				/* Slave-mode Output Disable */
	ST_SSP_NORMAL,		/* Loopback Mode */
	0,					/* Serial Clock Rate */
	1,					// 6, // 1, 	/* Clock Prescale divisor */
	FALSE,				/* Transmit DMA Enable */
	FALSE				/* Receive DMA Enable */
};
	
#endif

	tU32 vl_tmpSpeed, vl_bestSpeed;
	UINT vl_best_cpsdvsr;
	UINT vl_best_scr = 0;
	tBool vl_found = false;
	
	// default clock of the SSP is Fsspclock  = 102.4 MHz.
	// the bus clock is choosen by the divider, and Serial Clock
	// Clock = Fsspclock / [Clock Prescale divisor * (1 + Serial Clock Rate)]
	// in our config : Clock = 102.4 / [ 40 * (1 + 0)] = 2.56 MHz
	//
	// Configuration is : 

	/*
		typedef struct _ typedef struct _ st _sspsspssp_config _param_ {
			BOOL ope_mode; // Operating Mode
			BOOL sph ; // Motorola SPI Clock phase 
			BOOL spo ; // SSPCLK Clock Polarity 
			UINT dss ; // Data Size Select 
			UINT txiflsel ; // Transmit Interrupt FIFO Level Select 
			BOOL tendn ; // Transmit Endian Format 
			BOOL rendn ; // Receive Endian Format 
			BOOL sod ; // Slave -mode Output Disablemode 
			BOOL loopback_mode; loopback_mode; 
			UINT scr; // Serial Clock Rate 
			UINT cpsdvsr ; // Clock Prescale divisor 
			BOOL dma_tx ; // Transmit DMA 
			BOOL dma_rx ; // Receive DMA 
			} ST _SSP _CONFIG
	*
	* info by field :
	*
	*
		BOOL ope_mode ==> Operating Mode ==> 
		ST _SSP _OPE_SLV: Slave ModeSlave 
		ST _SSP _OPE_MAS : Master Mode
		
		BOOL  sph
		Motorola SPI Clock phase 
		ST _SSP _SPH0 : SPO=0:rising edge SPO=1:falling edge 
		ST _SSP _SPH1 : SPO=0:falling SPO=1:rising edge 
		
		BOOL spo
		SSPCLK Clock Polarity
		ST _SSP _SPO_LOW : The inactive or idle state of SSPCLK is LOW 
		ST_ SSP _SPO_HIGH: The inactive or idle state of SSPCLK is HIGH 
		
		UINT  dss 
		Data Size Select
		Set 0x3 to  0x1F
		
		UINT  txiflsel 
		Transmit Interrupt FIFO Level Select
		ST _SSP _TXFIFO_32 : Transmit FIFO becomes >= 1/32 empty
		ST_ SSP _TXFIFO_8  : Transmit FIFO becomes >= 1/8 empty 
		ST_ SSP _TXFIFO_4 : Transmit FIFO becomes >= 1/4 empty 
		ST_ SSP _TXFIFO_2 : Transmit FIFO becomes >= 1/2 empty 

		UINT  rxiflsel
		Receive Interrupt FIFO Level Select 
		ST_ SSP _RXFIFO_32 : Receive FIFO becomes >= 1/32 full 
		ST_ SSP _RXFIFO_8 : Receive FIFO becomes >= 1/8 full 
		ST_ SSP _RXFIFO_4: Receive FIFO becomes >= 14 full
		ST_ SSP _RXFIFO_2 : Receive FIFO becomes >= 1/2 full

		BOOL tendn 
		Transmit Endian Format
		ST_ SSP _ENDIAN_MSB :  The element is transmitted MSBit first
		ST_ SSP _ENDIAN_LSB : The element is transmitted LSBit first 

		BOOL rendn
		Receive Endian Format 
		ST_ SSP _ENDIAN_MSB :  The element is received MSBit first
		ST_ SSP _ENDIAN_LSB : The element is received LSBit first 

		BOOL sod
		Slave-mode Output Disable
		FALSE  : SSP can drive the SSPTXD output in slave mode
		TRUE  : SSP must not drive the SSPTXD output in slave mode

		BOOL loopback_mode
		Loop Back Mode 
		ST _SSP _NORMAL : Normal Mode 
		ST_ SSP _LOOPBACK: Loopback Mode Loopback 

		UINT scr U
		
		Serial Clock Rate 
		Set 0 to 255
		The bit rate is:   Fsspclock / [Clock Prescale divisor * (1 + Serial Clock Rate)]

		UINT  cpsdvsr
		Clock Prescale divisor 
		Set 2 to 254 (even value)
		The bit rate is:   Fsspclock / [Clock Prescale divisor * (1 + Serial Clock Rate)]

		BOOL dma_tx 
		Transmit DMA Enable
		TRUE  : Enable 
		FALSE : Disable 

		BOOL dma_rx 
		Receive DMA Enable 
		TRUE  : Enable 
		FALSE : Disable
	*/

#if 0
	ST_SSP_CONFIG_PARAM buf = {
		ST_SSP_OPE_MAS, 	/* Master or Slave mode select */
		ST_SSP_SPH1,     //m//  /* Motorola SSP Clock phase */
		ST_SSP_SPO_HIGH, //m//  /* SSPCLK Clock Polarity */
		0x07, // 0x08,		/* Data Size Select */
		ST_SSP_TXFIFO_2,	/* Transmit Interrupt FIFO Level Select */
		ST_SSP_RXFIFO_2, // ST_SSP_RXFIFO_4,	/* Recieve Interrupt FIFO Level Select */
		ST_SSP_ENDIAN_MSB,	/* Transmit Endian Format */
		ST_SSP_ENDIAN_MSB,	/* Receive Endian Format */
		FALSE,				/* Slave-mode Output Disable */
		ST_SSP_NORMAL,		/* Loopback Mode */
		0,					/* Serial Clock Rate */
		40,  // 6, // 1, 	/* Clock Prescale divisor */
		FALSE,				/* Transmit DMA Enable */
		FALSE				/* Receive DMA Enable */
	};
#endif

	if (false == vI_cpha1cpol1)
	{
		// we are not in default config
		// assuming cph0pol0
		vl_configParam.sph = ST_SSP_SPH0;
		vl_configParam.spo = ST_SSP_SPO_LOW;
	}

	if (false == vI_dataSize32bits)
	{
		// data size is byte 
		// we support 32 bits or 8
		// here 8
		vl_configParam.dss = 0x07;
	}

	// calculate the clock divider
	// 
	// we keep the serial clock rate to 0
	vl_scr = SCR_MIN;
	vl_cpsdvsr = CPSDVR_MIN;
	vl_found = false;
	vl_bestSpeed = 0;
	vl_best_cpsdvsr = SCR_MIN;
	vl_best_scr = CPSDVR_MIN;

	// is calculated depending on the speed
	// we have =
	// Clock (i.e speed) = Fsspclock / [Clock Prescale divisor * (1 + Serial Clock Rate)]
	// 
	// Clock Prescale divisor  = (Fsspclock / speed) / (1 + Serial Clock Rate)
	
	//
	// loop to find best suitable coefficient
	// For each Prescalor, try all serial clock rate
	// keep the best frequency : closest to requested frequency
	while ((vl_cpsdvsr <= CPSDVR_MAX) && (false == vl_found))
	{
		while (vl_scr <= SCR_MAX)
		{
			vl_tmpSpeed =  TNR_SPI_SSP_MAX_CLOCK / (vl_cpsdvsr * (1 + vl_scr));

			if (vl_tmpSpeed > vI_speed)
			{
				/* we need lower freq */
				vl_scr++;
			}
			else
			{
			/*
			 * If found exact value, mark found and break.
			 * If found more closer value, update and break.
			 */
			if (vl_tmpSpeed > vl_bestSpeed)
			{
				vl_bestSpeed = vl_tmpSpeed;
				vl_best_cpsdvsr = vl_cpsdvsr;
				vl_best_scr = vl_scr;

				if (vl_tmpSpeed == vI_speed)
				{
					vl_found = true;
				}
			}
			/*
			 * increased scr will give lower rates, which are not
			 * required
			 */
			break;
		}
		}
		
		vl_cpsdvsr += 2;
		vl_scr = SCR_MIN;
	}
		

	printf(" ------------ SPI configuration --------------\n");
	printf("Requested speed = %d\n", vI_speed);
	printf("Result	  speed = %d\n", vl_bestSpeed);
	printf("Parameters 	scr = %d, cpsdvsr = %d\n", vl_best_scr, vl_best_cpsdvsr);
	printf("Parameters 	data size %d\n", vl_configParam.dss);
	printf(" ------------ ---------------------------------\n");



	vl_configParam.scr =  vl_best_scr;
	vl_configParam.cpsdvsr =  vl_best_cpsdvsr;


	err = TNR_SPI_write_SSP(id, ST_SSP_WSL_CONFIG, &vl_configParam, sizeof(vl_configParam));
	if (err != E_OK) 
	{
		printf("SSP/SPI sample : write config error CODE=%08x\n", err);
	}

	//lets wait the configuration to be settle ?
	
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
static ER _i2c_data_write(ID id, UB* buffer, INT i,  UINT   slv_add)
{ 
	ER errorCode;
    ST_I2C_DATA_PARAM buf = {   1,
                                0,
                                FALSE,
                                0,
                                NULL,
                                sizeof(buffer)};
	buf.slave_addr = slv_add;
    buf.data        = buffer;
    buf.trans_len   = i;
    errorCode=_i2c_write(id, 0, &buf, sizeof(buf));
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
static ER _i2c_data_read(ID id, INT i, UB* read_data,  UINT   slv_add)
{

    ST_I2C_DATA_PARAM buf = {
        1,
        0,
        FALSE,
        0,
        NULL,
        0  // sizeof(read_data)
    };
	buf.slave_addr = slv_add;
	
    buf.data    = read_data;
    buf.trans_len = i;      //    buf.trans_len =4;
    buf.data_size = i; // from DD
    return _i2c_read(id, 0, &buf, sizeof(buf));
}




/*!
 * @brief	  		_i2c_set_cond_f:
 * 			       This routine set the I2C start condition.
 * @param          	: id - I2C Device descriptor 	
 * @note 			: Use to control PCA9536 RGB device
 *
 */
static void  _i2c_set_cond_f(ID id)
{
    ST_I2C_COND_PARAM buf = { FALSE };
    (void)_i2c_write(id, ST_I2C_WSL_COND, &buf, sizeof(buf));
}


////////// Linux write and read redefinition for tkernel //////////////////// //m//
//This is wrong and does not differentiate I2C and SPI writes

static int tnr_write(tS32 fd, tS32 device_address, tU8 *w_buf, tS32 w_len )
{
    tS32 ret = -1;

	_i2c_set_cond_f(fd);  /* stop condition */
	ret = _i2c_data_write(fd, (UB *)w_buf, w_len, (tU32)(device_address) >> 1);

		
    return ret;  
}

static int tnr_read(tS32 fd, tS32 device_address, tU8 *r_buf, tS32 r_len )
{
	
    _i2c_set_cond_f(fd);  /* stop condition */
    (void)_i2c_data_read(fd, r_len, (UB *)r_buf, (UINT) device_address >> 1);


    return r_len;  
}

static int spi_write(tS32 device_Id, tU8 *w_buf, tS32 w_len,  tBool vI_dataSize32bits)
{
	int	vl_ret = OSAL_OK;

	
#ifdef ETAL_BSP_DEBUG_SPI_RW_WORD

	BSP_Linux_TransferSpi(device_Id, w_buf, NULL, w_len, vI_dataSize32bits);
	vl_ret = w_len;

#else
	UB *pl_BufRead = NULL;
	ST_SSP_DATA_PARAM buf_ssp;
	tS32 err = -1;

	pl_BufRead =  (UW *) OSAL_pvMemoryAllocate(w_len);
		
	if (NULL == pl_BufRead)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
			COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SSP/SPI sample : write error CODE=%08x\n", err);
#endif
	
			vl_ret = OSAL_ERROR;
		}
		else
		{
			vl_ret = w_len;
		}
		

	// free the allocated buffer : 
	OSAL_pvMemorySet(pl_BufRead, 0, w_len);
		
	buf_ssp.trans_len  = w_len;  
	buf_ssp.write_data = w_buf;
	buf_ssp.read_data  = pl_BufRead;
	buf_ssp.data_size  = w_len;

	err = TNR_SPI_write_SSP(device_Id, ST_SSP_WSL_DATA, &buf_ssp, sizeof(buf_ssp));
	if (err != E_OK)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SSP/SPI sample : write error CODE=%08x\n", err);
#endif

		vl_ret = OSAL_ERROR;
	}
	else
	{
		vl_ret = w_len;
	}

	// free the allocated buffer : 

	OSAL_vMemoryFree(pl_BufRead);
#endif
		
		
	return vl_ret;
		
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
static int spi_read(tS32 device_Id, tU8 *r_buf, tS32 r_len, tBool vI_dataSize32bits )
{



	//	UB *pl_BufRead;
	int	vl_ret = OSAL_OK;


	
#ifdef ETAL_BSP_DEBUG_SPI_RW_WORD
	BSP_Linux_TransferSpi(device_Id, NULL, r_buf, r_len, vI_dataSize32bits);
	vl_ret = r_len;
#else
	
	// the read input buffer is assumed well sized:
	// no need to reallocate
	// only the write
	UB *pl_BufWrite;
	ST_SSP_DATA_PARAM buf_ssp;
	tS32 err = -1;
		
	pl_BufWrite = (UB *) OSAL_pvMemoryAllocate(r_len);
			
	if (NULL == pl_BufWrite)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SSP/SPI  : allocation error pl_BufWrite CODE=%08x\n");
#endif
		vl_ret = -1
		goto exit;
	}
	
	// reset the allocated write data
	OSAL_pvMemorySet(pl_BufWrite, 0, r_len);
	
	buf_ssp.trans_len  = r_len;
	buf_ssp.write_data = pl_BufWrite;
	buf_ssp.read_data  = r_buf;
	buf_ssp.data_size  = r_len;
	
	err = TNR_SPI_write_SSP(device_Id,ST_SSP_WSL_DATA,&buf_ssp,sizeof(buf_ssp));
	if (err != E_OK)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "SSP/SPI sample : read error CODE=%08x\n", err);
#endif
		vl_ret = -1;
	}
	else
	{
		//memcpy(r_buf, g_read_data, r_len);
			
		vl_ret = r_len;
	}

	OSAL_vMemoryFree(pl_BufWrite);

exit:
#endif

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
}
#endif 

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

/**************************************
 *
 * BSP_DeviceReset_MDR
 *
 *************************************/
/*
 * Manages the MDR reset on the Accordo2 EVB, where the MDR's RSTN line
 * is connected to GPIO20
 * RSTN is active low
 */
tSInt BSP_DeviceReset_MDR(tU32 deviceID)
{
//	tS32 fd;
    char str_GPIO_RESET[11];

    BSP_tracePrintSysmin(TR_CLASS_BSP, "MDR reset");

    sprintf(str_GPIO_RESET, "%u", DAB_GetGPIOReset(deviceID));

	BSP_Linux_OpenGPIO(str_GPIO_RESET, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);

	/*
	 * Assert the reset line
	 */
	BSP_Linux_WriteGPIO(atoi(str_GPIO_RESET), 0);
	/*
	 * Let the signal settle
	 */
	OSAL_s32ThreadWait(5);
	/*
	 * Bring out of reset
	 */
	BSP_Linux_WriteGPIO(atoi(str_GPIO_RESET), 1);

	BSP_Linux_CloseGPIO(atoi(str_GPIO_RESET));

    BSP_tracePrintSysmin(TR_CLASS_BSP, "MDR reset complete");

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
	static tBool vl_req_val = 0;
	
	tBool val;
	val = BSP_Linux_ReadGPIO(MDR_LinuxSPI_REQ_fd);
	if (vl_req_val != val)
	{
	//	printf("BSP_SteciReadREQ_MDR : Req changed val = %d\n", val);
		vl_req_val = val;
	}

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
	tBool vl_current_val;
	
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	COMMON_tracePrint(TR_LEVEL_COMPONENT, TR_CLASS_BSP, "MDR CS set %d", value);
#endif
 
	vl_current_val = BSP_Linux_ReadGPIO(MDR_LinuxSPI_CS_fd);

	if (vl_current_val != value)
	{
		BSP_Linux_WriteGPIO(MDR_LinuxSPI_CS_fd, value);
	//	printf("write MDR CS value : %d\n", value);
	}
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
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	COMMON_tracePrint(TR_LEVEL_COMPONENT, TR_CLASS_BSP, "BSP_SteciSetBOOT_MDR set %d", value);
#endif

	// Open the SPI boot GPIO if needed
    if (MDR_LinuxSPI_BOOT_fd < 0)
    {		
		// this is an error
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "BSP_SteciSetBOOT_MDR not fd\n");
    }
	
	BSP_Linux_WriteGPIO(MDR_LinuxSPI_BOOT_fd, value);
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
#endif

#ifdef	ETAL_BSP_DEBUG_SPI_RW_WORD
	BSP_Linux_TransferSpi(MDR_LinuxDevice_fd, buf_wr, buf_rd, len, true);
#else
	BSP_Linux_TransferSpi(MDR_LinuxDevice_fd, buf_wr, buf_rd, len, false);
#endif

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, 0, buf_rd, len, BSP_READ_OPERATION);
#endif
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

	tSInt vl_ret = OSAL_OK;

    // set the speed
    // the MDR is SPI & SPI_11
    if (((tySpiCommunicationBus*)DAB_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA1_CPOL1) 
    {
    	TNR_SPI_setconf_SSP(MDR_LinuxDevice_fd, ((tySpiCommunicationBus*)DAB_GetCommunicationBusConfig(deviceID))->speed,
            (((tySpiCommunicationBus*)DAB_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0) ? FALSE : TRUE,
            true);
    }
	else
	{
		// this is an error
		vl_ret = OSAL_ERROR;
	}

    return vl_ret;
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
	tSInt vl_ret = OSAL_OK;

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
#endif

        MDR_LinuxDevice_fd = BSP_Linux_OpenDevice(((tySpiCommunicationBus*)DAB_GetCommunicationBusConfig(deviceID))->busName);
		if (MDR_LinuxDevice_fd < 0)
		{
			vl_ret= OSAL_ERROR;
			goto exit;
		}

		// set the speed
		vl_ret = BSP_BusConfigSPIFrequency_MDR(deviceID);
       	if (OSAL_OK != vl_ret)
	   	{
	   		goto exit;
       	}
		
        MDR_LinuxSPI_CS_fd = BSP_Linux_OpenGPIO(strGPIO_CS, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);

		if (MDR_LinuxSPI_CS_fd < 0)
		{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
			COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "configuring the SPI CS");
#endif
			vl_ret = OSAL_ERROR;
			goto exit;
		}

        MDR_LinuxSPI_REQ_fd = BSP_Linux_OpenGPIO(strGPIO_REQ, GPIO_OPEN_READ, GPIO_IRQ_NONE, NULL);
		if (MDR_LinuxSPI_REQ_fd < 0)
		{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
			COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "configuring the SPI REQ");
#endif
			vl_ret = OSAL_ERROR;
			goto exit;
		}

#ifdef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
        BSP_SteciOpenBOOT_GPIO_MDR();

	    if (MDR_LinuxSPI_BOOT_fd < 0)
	    {
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
	        COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "configuring the SPI BOOT");
#endif
	        vl_ret = OSAL_ERROR;
			goto exit;
	    }
#endif /* CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO */
    }
    else
    {
    	// invalid request : SPI and wrong mode
    	
        vl_ret = OSAL_ERROR;
		goto exit;
    }

exit:

	return vl_ret;
	
}

/**************************************
 *
 * BSP_DeviceInit_MDR
 *
 *************************************/
tSInt BSP_DeviceInit_MDR(tU32 deviceID)
{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_SYSTEM_MIN)
	COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, TR_CLASS_BSP, "MDR init");
#endif

	if (BSP_BusConfig_MDR(deviceID) != OSAL_OK)
	{
        return OSAL_ERROR;
	}

    BSP_tracePrintSysmin(TR_CLASS_BSP, "MDR init complete");

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
#endif

	MDR_LinuxDevice_fd = -1;
	MDR_LinuxSPI_CS_fd = -1;
	MDR_LinuxSPI_REQ_fd = -1;
#ifdef CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO
	MDR_LinuxSPI_BOOT_fd = -1;
#endif
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_CMOST
#if 0

/*!
 * @brief	  	 	TNR_Clear_Set_RSTN:
 * 			       This funtion release the Tuner RESET line
 * @param     :	void
 * @return    : 
 * @note 			:
 *
 */	

void TNR_Clear_Set_RSTN (void)
{
	ST_GPIO_WRITEPIN_PARAM  param;
	ST_GPIO_ALTFUNC_PARAM   altf_param = {0};
	ST_GPIO_PINNO vl_GPIO;

#ifdef CONFIG_BOARD_ACCORDO2_CUSTOM2
	vl_GPIO = R4_GPIO_PIN_96;
#else
	vl_GPIO = S_GPIO_PIN_6;
#endif

	/* Disable Alt function */
	altf_param.pin 	= vl_GPIO;
	altf_param.altfunc 	= ALTFUNC_DISABLE;
	st_gpio_set_altfunc(&altf_param);

	param.pin 	= vl_GPIO;
	param.wdata 	= TRUE;
	st_gpio_set_direction(&param);

	/* set RESET level */
	param.pin 	= vl_GPIO;
	param.wdata 	= TRUE;
	st_gpio_set_data(&param);
	tk_dly_tsk(20);
	
	/* clear level */
	param.pin 	= vl_GPIO;
	param.wdata 	= FALSE;
	st_gpio_set_data(&param);
	tk_dly_tsk(20);
	
	/* set RESET level */
	param.pin 	= vl_GPIO;
	param.wdata 	= TRUE;
	st_gpio_set_data(&param);

}
#endif

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
	tS32 fd_irq = -1;
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
		fd_irq = BSP_Linux_OpenGPIO(str_GPIO_IRQ, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
		
		if (fd_irq < 0)
		{
			ret = OSAL_ERROR;
			goto exit;
		}

		// force the IRQ HIGH
		
		BSP_Linux_WriteGPIO(fd_irq, TRUE);
		(void)tk_dly_tsk(5);
	}

	// force the reset to high as a start
	/*
	 * Perform the reset
	 * Negate because the reset line is active low
	 */
	BSP_Linux_WriteGPIO(fd, TRUE);
	(void)tk_dly_tsk(20);

	/* Set the IRQ to 1 for SPI */
	
	/* Perform the reset
	 * Negate because the reset line is active low */
	BSP_Linux_WriteGPIO(fd, FALSE);
	(void)tk_dly_tsk(20);
	
	/*
	 * Perform the reset
	 * Negate because the reset line is active low
	 */
	BSP_Linux_WriteGPIO(fd, TRUE);

	/* Let the signal settle */
//	OSAL_s32ThreadWait(5);
	(void)tk_dly_tsk(20);


	/* Now GPIO can be closed */
	BSP_Linux_CloseGPIO(fd);

	if (TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
		/* Force the IRQ HIGH */
		BSP_Linux_CloseGPIO(fd_irq);

		/* Open in Read again */
		fd_irq = BSP_Linux_OpenGPIO(str_GPIO_IRQ, GPIO_OPEN_READ, GPIO_IRQ_NONE, NULL);
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
 * BSP_DeviceAddressToChipSelectDescriptor
 *
 *************************************/
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
 * BSP_DeviceAddressSetCs
 *
 *************************************/
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
 *  the new file descriptor or
 *  -1 in case of error
 */
tS32 BSP_BusConfig_CMOST(tU32 deviceID)
{
	tS32 *fdp;
	tS32 vl_CS_fd = -1;
	tS32 ret = OSAL_OK;
	tySpiCommunicationBus *spiBusConfig = NULL;
	char str_GPIO_CS[11];

#ifndef CONFIG_BOARD_ACCORDO2_CUSTOM2	
		ST_GPIO_ALTFUNC_PARAM	altf_param = {0};
#endif

	if(TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
		spiBusConfig = (tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID);

		fdp = BSP_DeviceAddressAdd(deviceID);

		 if (fdp == NULL)
		 {
			 ret = OSAL_ERROR;
			 goto exit;
		 }

		*fdp = BSP_Linux_OpenDevice(spiBusConfig->busName);
		
		 if (*fdp < 0)
		 {
			 ret = OSAL_ERROR;
			 BSP_DeviceAddressRelease(deviceID);
			 goto exit;
		 }

		ret = *fdp;
		
		/* set ssp configration */
		// for TUNER : the SPI should be 11
		if (SPI_CPHA0_CPOL0 == spiBusConfig->mode)
		{
			// warning this is an error !!
			BSP_tracePrintError(TR_CLASS_BSP, "wrong SPI configuration for CMOST, SPI_CPHA0_CPOL0 not supported");
		}

		TNR_SPI_setconf_SSP(*fdp, spiBusConfig->speed, true, true);

		/* Open the GPIO for SPI control CS. */
	
		if(spiBusConfig->GPIO_CS == ETAL_CS_TRUE_SPI)
		{
#ifndef CONFIG_BOARD_ACCORDO2_CUSTOM2
			/* Set the function as true SPI1 : Alternate A Function  */
			altf_param.pin		=  spiBusConfig->GPIO_CS ; //
			altf_param.altfunc	= ALTFUNC_A;	 // 
			(void)st_gpio_set_altfunc(&altf_param);		 // 
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
		
	}
	else
	{
		// I2C case
		
		fdp = BSP_DeviceAddressAdd(deviceID);
		if (fdp == NULL)
		{
			ret = -1;
			goto exit;
		}

		/* open device */
	    *fdp = tk_opn_dev((CONST UB *)((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->busName, TD_UPDATE);

	    if (*fdp >= E_OK)
	    {
	         (void)_i2c_sample_set_config(*fdp);
			 ret = *fdp;
	    }
		else
		{
			BSP_DeviceAddressRelease(deviceID);
			ret = -1;
			goto exit;
		}
	}

	// init the IRQ if an IRQ is set
	if (NULL  != TUNERDRIVER_GetIRQCallbackFunction(deviceID))
	{
		if (BSP_RDSInterruptInit_CMOST(deviceID))
			{
				ret = 1;
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
	
	// init the IRQ if an IRQ is set
	if (NULL  != TUNERDRIVER_GetIRQCallbackFunction(deviceID))
	{
		BSP_RDSInterruptDeinit_CMOST(deviceID);
	}

	// We should also close the CS if SPI
	vl_CS_fd = BSP_DeviceAddressToChipSelectDescriptor(deviceID)
	if (-1 != vl_CS_fd)
	{
		// close the GPIO
		BSP_Linux_CloseGPIO(vl_CS_fd);
	}
	
	BSP_DeviceAddressRelease(deviceID);
	
#if defined(BSP_INCLUDE_SIGNALLING_GPIO)
	BSP_deinitSignallingGPIO();
#endif

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
		/*
		 * the kernel was not yet configured for use of the <device_address>
		 * try to configure it
		 */
		fd = BSP_BusConfig_CMOST(deviceID);
		if (fd < 0)
		{
			ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
			ret = -1;
			goto exit;
		}
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	{
		tS32 i;
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
		ret = BSP_Write_CMOST_SPI(deviceID, buf, len);
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
//		spiBusConfig = (tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID);
		// Error not used in LINUX/TK 
		// the read only function is not used in SPI configuration in that case
		
		//(void)BSP_Write_CMOST_SPI(deviceID, buf, len);
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "BSP_Read_CMOST not applicable to SPI");
		len = 0;
		ret = -1;
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
	else if (ret < len)
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
	tS32 fd;
	fd = BSP_DeviceAddressToFileDescriptor(deviceID);
	if (fd >= 0)
	{
#ifdef	ETAL_BSP_DEBUG_SPI_RW_WORD
		BSP_Linux_TransferSpi(fd, buf_wr, buf_rd, len, true);
#else
		BSP_Linux_TransferSpi(fd, buf_wr, buf_rd, len, false);
#endif
        }
	else
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
	}


  //m//	BSP_Linux_TransferSpi(CMOST_LinuxDevice_fd, buf_wr, buf_rd, len);
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

	/*
	vl_CS_fd = BSP_DeviceAddressToChipSelectDescriptor(device_address, false);

	// Set the chipselect to low level for writing start
	BSP_Linux_WriteGPIO(vl_CS_fd,0);
	*/

	(void)BSP_SetCS_CMOST_SPI(deviceID, 0);
	
	// write the data
#ifdef ETAL_BSP_DEBUG_SPI_RW_WORD
	vl_ret = spi_write(vl_fdp, buf, len, true);
#else
	vl_ret = spi_write(vl_fdp, buf, len, false);
#endif

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
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
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
			ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
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
			ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
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
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code

		// free what it is needed
		
		// 
		// if we come here, the allocated buffer are not null
		//  free memory
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
#ifdef	ETAL_BSP_DEBUG_SPI_RW_WORD
	BSP_Linux_TransferSpi(vl_fdp, pl_InputBfr, pl_OutputBfr, vl_size, true);
#else
	BSP_Linux_TransferSpi(vl_fdp, pl_InputBfr, pl_OutputBfr, vl_size, false);
#endif
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
			COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "BSP_WriteRead_CMOST_SPI : vI_ByteToWrite %d, vI_ByteToRead %d, vl_CmostHeaderLenVal = %d, deviceID = %d", vI_ByteToWrite, vI_ByteToRead, vl_CmostHeaderLenVal, deviceID);
			BSP_PrintBufNoSpace(TR_LEVEL_ERRORS, FALSE, pl_InputBfr, vl_size, BSP_WRITE_OPERATION);
			BSP_PrintBufNoSpace(TR_LEVEL_ERRORS, FALSE, pl_OutputBfr, vl_size, BSP_READ_OPERATION);
		}

	}
#endif 

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	COMMON_tracePrint(TR_LEVEL_COMPONENT, TR_CLASS_BSP, "BSP_WriteRead_CMOST_SPI : vI_ByteToWrite %d, vI_ByteToRead %d", vI_ByteToWrite, vI_ByteToRead);
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
	tSInt ret = OSAL_OK;
#if 0
	ST_GPIO_WRITEPIN_PARAM 	param;
	ST_GPIO_ALTFUNC_PARAM   altf_param = {0};
	
	// Disable Alt function
	altf_param.pin 		= R4_GPIO_PIN_20;
	altf_param.altfunc 	= ALTFUNC_DISABLE;
	st_gpio_set_altfunc(&altf_param);
	
	param.pin 		= R4_GPIO_PIN_20;
    param.wdata 	= TRUE;
    st_gpio_set_direction(&param);
	
		  /* set RESET level */
    param.pin 	= R4_GPIO_PIN_20;
    param.wdata = TRUE;
    st_gpio_set_data(&param);
 		tk_dly_tsk(20);
  	  /* clear level */
    param.pin 	= R4_GPIO_PIN_20;
    param.wdata = FALSE;
    st_gpio_set_data(&param);
    tk_dly_tsk(20);
     /* set RESET level */
    param.pin 	= R4_GPIO_PIN_20;
    param.wdata = TRUE;
    st_gpio_set_data(&param);
#else
	tS32 fd;
	char str_GPIO_RESET[11];

	sprintf(str_GPIO_RESET, "%u", HDRADIO_GetGPIOReset(deviceID));

	fd = BSP_Linux_OpenGPIO(str_GPIO_RESET, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
	if (fd < 0)
	{
		ret = OSAL_ERROR;
		goto exit;
	}

	/*
	 * Assert the reset line
	 */
	BSP_Linux_WriteGPIO(fd, 0);

	/*
	 * Let the signal settle
	 */
	(void)OSAL_s32ThreadWait(2);

	/*
	 * Deassert the reset line
	 */
	BSP_Linux_WriteGPIO(fd, 1);
	
	/*
	 * Let the signal settle
	 */
	(void)OSAL_s32ThreadWait(2);

	BSP_Linux_CloseGPIO(fd);

#endif // 0	

exit:
	return ret;
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
 * <deviceID> Indicate the SPI mode.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfigSPIMode_HDRADIO(tU32 deviceID)
{

	// by default : assume that data size if 8 bits in SPI_00, 32 in SPI_11 
	tBool vl_dataSizeIs32bits;

	if (true == (((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0))
	{
		vl_dataSizeIs32bits = false;
	}
	else
	{
#ifdef ETAL_BSP_DEBUG_SPI_RW_WORD		
		vl_dataSizeIs32bits = true;
#else
		vl_dataSizeIs32bits = false;
#endif
	}
	
	// set the configuration
	TNR_SPI_setconf_SSP(HDRADIO_SPI_LinuxDevice_fd, ((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->speed,
			(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0) ? false : true,
			vl_dataSizeIs32bits);

	return OSAL_OK;
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
	// same processing than changing the SPI mode
	// in TK all is done simultaneously
	//
	return BSP_BusConfigSPIMode_HDRADIO(deviceID);
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
	// set the speed
	TNR_SPI_setconf_SSP(HDRADIO_SPI_LinuxDevice_fd, ((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->speed,
			(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0) ? false : true,
			vI_dataSizeIs32bits);

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
 * <deviceID> - device ID
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - error accessing the kernel driver interface
 */
tSInt BSP_BusConfig_HDRADIO(tU32 deviceID)
{
	tSInt ret = OSAL_OK;
	char strGPIO_CS[11];

	if(HDRADIO_GetBusType(deviceID) == BusSPI)
	{
		sprintf(strGPIO_CS, "%u", ((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->GPIO_CS);

		HDRADIO_SPI_LinuxDevice_fd = -1;

		HDRADIO_SPI_LinuxDevice_fd = BSP_Linux_OpenDevice(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->busName);
		if (HDRADIO_SPI_LinuxDevice_fd < 0)
		{
			ret = OSAL_ERROR;
			goto exit;
		}

		/*
		 * Set up the spi mode
		 */
		// already in WR MODE


		/*
		 * Set up the spi speed
		 */
		(void)BSP_BusConfigSPIFrequency_HDRADIO(deviceID);

		// The clock line is born by A2 with a level that might be not compatible with the mode selected.
		// The fake write w/o FS driving is to adjust the clock line level
		//write(HDRADIO_SPI_LinuxDevice_fd, &u8FakeWrForClkLevel, 0);

		HDRADIO_LinuxSPI_CS_fd = -1;

		HDRADIO_LinuxSPI_CS_fd = BSP_Linux_OpenGPIO(strGPIO_CS, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
		if (HDRADIO_LinuxSPI_CS_fd < 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "configuring the SPI CS");
			ret = OSAL_ERROR;
			goto exit;
		}

		BSP_Linux_WriteGPIO(HDRADIO_LinuxSPI_CS_fd, 1);
	}
	else
	{
		/*
		 * the I2C address is 7 bits plus the direction; in Linux
		 * the direction bit must be discarded
		 */
		// tS32 addr = (tS32)(deviceConfiguration->communicationBus.i2c.deviceAddress >> 1); // was HDRADIO_ACCORDO2_I2C_ADDRESS

		HDRADIO_I2C_LinuxDevice_fd = -1;

		/* open device */
		HDRADIO_I2C_LinuxDevice_fd = tk_opn_dev((CONST UB *)((tyI2cCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->busName, TD_UPDATE);

		if (HDRADIO_I2C_LinuxDevice_fd >= E_OK)
		{
			 (void)_i2c_sample_set_config(HDRADIO_I2C_LinuxDevice_fd);
		}
		else
		{
			BSP_tracePrintError(TR_CLASS_BSP, "BSP_BusConfig_HDRADIO error opening I2C device ");
			ret = OSAL_ERROR;
		}
	}

exit:
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
 * <deviceConfiguration> - Pointer to device configuration
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
	tSInt ret = OSAL_OK;
	
	if (BSP_BusConfig_HDRADIO(deviceID) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_SYSTEM_MIN)
	COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, TR_CLASS_BSP, "HDRADIO init complete");
#endif
	}

	return ret;
}

/**************************************
 *
 * BSP_DeviceDeinit_HDRADIO
 *
 *************************************/
tSInt BSP_DeviceDeinit_HDRADIO(tU32 deviceID)
{
	tSInt ret = OSAL_OK;
	if (BSP_BusUnconfig_HDRADIO(deviceID) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else
	{

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_SYSTEM_MIN)
	COMMON_tracePrint(TR_LEVEL_SYSTEM_MIN, TR_CLASS_BSP, "HDRADIO deinit complete");
#endif
		ret = OSAL_OK;
	}

	return ret;

}


/**************************************
 *
 * BSP_Write_HDRADIO
 *
 *************************************/
tS32 BSP_Write_HDRADIO(tU32 deviceID, tU8* buf, tU32 len)
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
				tmpRet += spi_write(HDRADIO_SPI_LinuxDevice_fd, &buf[bufIndex], 1, false);
				BSP_SPI0DriveCS_HDRADIO (1);
				bufIndex++;
			}
			ret = tmpRet;
		}
		else
		{
#ifdef ETAL_BSP_DEBUG_SPI_RW_WORD
			// specific handling for small datasize
			// in case the data are < 4, change datasize for byte to byte transport
			// 
			if (len < 4)
			{
				// change datasize
				(void)BSP_BusConfigSPIDataSize_HDRADIO(deviceID, false);

				// transport data
				BSP_SPI0DriveCS_HDRADIO (0);
					ret = spi_write(HDRADIO_SPI_LinuxDevice_fd, buf, len, false);
				BSP_SPI0DriveCS_HDRADIO (1);

				// come back to default setting
				// change datasize
				(void)BSP_BusConfigSPIDataSize_HDRADIO(deviceID, true);
			}
			else
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret = spi_write(HDRADIO_SPI_LinuxDevice_fd, buf, len, true);
				BSP_SPI0DriveCS_HDRADIO (1);
	}
#else
				BSP_SPI0DriveCS_HDRADIO (0);
				ret = spi_write(HDRADIO_SPI_LinuxDevice_fd, buf, len, false);
				BSP_SPI0DriveCS_HDRADIO (1);				
#endif

		}
	}
	else
	{
		ret = (tS32)tnr_write(HDRADIO_I2C_LinuxDevice_fd, 
			((tyI2cCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->deviceAddress, buf, (size_t)len);
	}

	if (ret < 0)
	{
#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_ERRORS)
		COMMON_tracePrint(TR_LEVEL_ERRORS, TR_CLASS_BSP, "writing command to HDRADIO");
#endif
	}
	else if (ret == 0 )
	{
		ret = len;
	}
	else
	{
		/* Nothing to do */
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT,
			(HDRADIO_GetBusType(deviceID) == BusSPI) ? 0 : 1,
			buf, len, BSP_WRITE_OPERATION);
#endif

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
	tS32 ret_fct = -1;
	tU32 len;
	tU16 lmAchievedBytes = 0;
	tU8 *pLocalBuf = &((*pBuf)[0]);
	tU16 bufIndex;
	tS32 s32Temp = 0;
#ifdef	ETAL_BSP_DEBUG_SPI_RW_WORD
	tU8 vl_index;
	tU8 vl_index_2;
#endif

	if(HDRADIO_GetBusType(deviceID) == BusSPI)
	{
		tU8 msecForCmdProcessed = 0;

		(void)OSAL_pvMemorySet (pLocalBuf, 0x00, HDRADIO_MIN_LMLENGTH);

#ifdef ETAL_BSP_DEBUG_SPI
		(void)OSAL_pvMemorySet (pLocalBuf+1, 0xBA, LM_FULL_SIZE-1);
#endif

		if(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0)
		{
			// Reading of zeros till the cmd is processed
			while (pLocalBuf[0] == 0 && msecForCmdProcessed < HDRADIO_MS_TO_HAVE_CMD_PROCESSED)
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, &pLocalBuf[0], 1, false);
				BSP_SPI0DriveCS_HDRADIO (1);
				(void)OSAL_s32ThreadWait (1);
				msecForCmdProcessed++;
			}

			if (msecForCmdProcessed == HDRADIO_MS_TO_HAVE_CMD_PROCESSED)
			{
				BSP_tracePrintError(TR_CLASS_BSP, "Error reading data from HDRADIO, SPI, CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0");
				ret_fct = -1;
				goto exit;
			}

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;
			ret = 0;

			for (bufIndex = 1; bufIndex < HDRADIO_MIN_LMLENGTH; bufIndex++)
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret += spi_read (HDRADIO_SPI_LinuxDevice_fd, &pLocalBuf[bufIndex], 1, false);
				BSP_SPI0DriveCS_HDRADIO (1);
			}

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;
			ret = 0;

			// LM Header checking: the len fields would be meaningless if the content just read is not an LM
			if (pLocalBuf[0] == HDRADIO_LM_BYTE_HEADER && pLocalBuf[1] == HDRADIO_LM_BYTE_HEADER)
			{
				// LM length computing
				len = (tU32)(pLocalBuf[4]) + (tU32)((tU32)(pLocalBuf[5]) << 8);
				// EPR correct : 
				// add a protection/ check on the len
				//
				if (len > LM_FULL_SIZE)
				{
					// len is not correct
					// return
					ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
					ret_fct = (tS32)lmAchievedBytes;
					goto exit;
				}
			}
			else
			{
				ret_fct = (tS32)lmAchievedBytes;
				goto exit;
			}

			// EPR correction : limit the bufIndex to full size
			for (bufIndex = HDRADIO_MIN_LMLENGTH; bufIndex < len; bufIndex++)
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret += spi_read (HDRADIO_SPI_LinuxDevice_fd, &pLocalBuf[bufIndex], 1, false);
				BSP_SPI0DriveCS_HDRADIO (1);
			}

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;
		}
		else
		{
			BSP_SPI0DriveCS_HDRADIO (0);

#ifdef	ETAL_BSP_DEBUG_SPI_RW_WORD
			// Reading of zeros till the cmd is processed
			while (((pLocalBuf[0] == 0) && (pLocalBuf[1] == 0) && (pLocalBuf[2] == 0) && (pLocalBuf[3] == 0))
				&& (msecForCmdProcessed < HDRADIO_MS_TO_HAVE_CMD_PROCESSED))
			{
				ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, pLocalBuf, 4, true);
				(void)OSAL_s32ThreadWait (1);
				msecForCmdProcessed++;
			}	
#else
			// Reading of zeros till the cmd is processed
			while (pLocalBuf[0] == 0 && msecForCmdProcessed < HDRADIO_MS_TO_HAVE_CMD_PROCESSED)
			{
				ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, pLocalBuf, 1, false);
				(void)OSAL_s32ThreadWait (1);
				msecForCmdProcessed++;
			}
#endif
			if (msecForCmdProcessed == HDRADIO_MS_TO_HAVE_CMD_PROCESSED)
			{
				BSP_tracePrintError(TR_CLASS_BSP, "Error reading data from HDRADIO, SPI, CONFIG_COMM_HDRADIO_SPI_CPHA1_CPOL1");
				BSP_SPI0DriveCS_HDRADIO (1);

				ret_fct = -1;
				goto exit;
			}

#ifdef	ETAL_BSP_DEBUG_SPI_RW_WORD
			// in this mode 4 bytes have been read
			// get back to initial one : starting from 0xa5 ie here not null
			vl_index = 0;
			while (pLocalBuf[vl_index] == 0)
			{
				vl_index++;
			} 

			// recopy data
			for (vl_index_2=0;vl_index_2<(4-vl_index);vl_index_2++)
			{
				pLocalBuf[vl_index_2] = pLocalBuf[vl_index_2+vl_index];
			}
			
			// simulate 1 byte read only
			ret = 4 - vl_index;
#endif

			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;

#ifdef	ETAL_BSP_DEBUG_SPI_RW_WORD
			// read a number of word entire
			ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, pLocalBuf+lmAchievedBytes, 8, true);
#else
			ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, pLocalBuf+1, HDRADIO_MIN_LMLENGTH-1, false);
#endif


			// Update the bytes actually read
			lmAchievedBytes += (tU16)ret;

			// LM Header checking: the len fields would be meaningless if the content just read is not an LM
			if (pLocalBuf[0] == HDRADIO_LM_BYTE_HEADER && pLocalBuf[1] == HDRADIO_LM_BYTE_HEADER)
			{
				// LM length computing
				len = (tU32)(pLocalBuf[4]) + (tU32)((tU32)(pLocalBuf[5]) << 8);
				
				// EPR correct : 
				// add a protection/ check on the len
				//
				if (len > LM_FULL_SIZE)
				{
					// len is not correct
					// return
					ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
					ret_fct = (tS32)lmAchievedBytes;				
					goto exit;
				}
			}
			else
			{
				BSP_SPI0DriveCS_HDRADIO (1);

				ret_fct = (tS32)lmAchievedBytes;
				goto exit;
			}
#ifdef	ETAL_BSP_DEBUG_SPI_RW_WORD
			ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, pLocalBuf+lmAchievedBytes, len-lmAchievedBytes, true);
#else
			ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, pLocalBuf+HDRADIO_MIN_LMLENGTH, len-HDRADIO_MIN_LMLENGTH, false);
#endif
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
		numberOfBytesInsideFifo = 0;
		msecForFifoFilled = 0;

		memset (pLocalBuf, 0x00, HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH+1);

		while (numberOfBytesInsideFifo < HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH && msecForFifoFilled < HDRADIO_MS_TO_HAVE_FIFO_MIN_FILLED)
		{
	//		read (HDRADIO_I2C_LinuxDevice_fd, &numberOfBytesInsideFifo, 1);
			(void)tnr_read (HDRADIO_I2C_LinuxDevice_fd, 
				((tyI2cCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->deviceAddress, 
				&numberOfBytesInsideFifo, 1);

			(void)OSAL_s32ThreadWait (1);
			msecForFifoFilled++;
		}

		if (msecForFifoFilled == HDRADIO_MS_TO_HAVE_FIFO_MIN_FILLED)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO, CONFIG_COMM_HDRADIO_I2C, HDRADIO_MS_TO_HAVE_FIFO_MIN_FILLED");
			ret_fct = -1;
			goto exit;
		}

		// Read the "min LM" bytes certified to be in the FIFO
	//	ret = (tS32)read (HDRADIO_I2C_LinuxDevice_fd, pLocalBuf, HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH+1);
		ret = (tS32)tnr_read (HDRADIO_I2C_LinuxDevice_fd, 
						((tyI2cCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->deviceAddress,
						pLocalBuf, HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH+1);

		if (ret < 0)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO, CONFIG_COMM_HDRADIO_I2C, ret = %d", ret);
			ret_fct = -1;
			goto exit;
		}
		else if ((tU32)ret < HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH+1)
		{
			BSP_tracePrintError(TR_CLASS_BSP, "truncated data read from HDRADIO");
			// Update the caller buffer to throw away the "byte-count" byte
			(*pBuf) += 1;
			s32Temp = ret-1;
			lmAchievedBytes += (tU16)(s32Temp);
			ret_fct = (tS32)lmAchievedBytes;
			goto exit;
		}
		else
		{
			/* Nothing to do */
		}

		// Update the bytes actually read
		s32Temp = ret-1;
		lmAchievedBytes += (tU16)(s32Temp);

		// Local pointer management and LM length computing
		pLocalBuf += 1;

		// LM Header checking: the len fields would be meaningless if the content just read is not an LM
		if (pLocalBuf[0] == HDRADIO_LM_BYTE_HEADER && pLocalBuf[1] == HDRADIO_LM_BYTE_HEADER)
		{
			// LM length computing
			len = (tU32)(pLocalBuf[4]) + (tU32)((tU32)(pLocalBuf[5]) << 8);
		}
		else 
		{
			*pBuf = pLocalBuf;
			ret_fct = (tS32)lmAchievedBytes;
			goto exit;
		}

		lmRemainingBytes = (tU16)(len - HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH);
		lmRemainingPackets = len / HDRADIO_FIFO_SIZE_IN_BYTES;

		lmBufferIndex = 0;
		while (lmRemainingBytes > 0)
		{
			// Define number of bytes to read
			if (lmRemainingPackets > 0)
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

			numberOfBytesInsideFifo = 0;
			msecForFifoFilled = 0;
			while ((tU16)numberOfBytesInsideFifo < lmBytesToRead && msecForFifoFilled < HDRADIO_MS_TO_HAVE_FIFO_FILLED)
			{
	//			read (HDRADIO_I2C_LinuxDevice_fd, &numberOfBytesInsideFifo, 1);
				(void)tnr_read (HDRADIO_I2C_LinuxDevice_fd, 
					((tyI2cCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->deviceAddress, 
					&numberOfBytesInsideFifo, 1);
				(void)OSAL_s32ThreadWait (1);
				msecForFifoFilled++;
			}

			if (msecForFifoFilled == HDRADIO_MS_TO_HAVE_FIFO_FILLED)
			{
				BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO, CONFIG_COMM_HDRADIO_I2C, HDRADIO_MS_TO_HAVE_FIFO_FILLED ");
				// Update the caller buffer to throw away the "byte-count" byte
				(*pBuf) += 1;
				ret_fct = (tS32)lmAchievedBytes;
				goto exit;
				}

				// Read the "Delta" bytes certified to be in the FIFO
		//		ret = (tS32)read (HDRADIO_I2C_LinuxDevice_fd, &lmTmpBuffer[0], lmBytesToRead+1);
				ret = (tS32)tnr_read (HDRADIO_I2C_LinuxDevice_fd, 
								((tyI2cCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->deviceAddress, 
								&lmTmpBuffer[0], lmBytesToRead+1);

				if (ret < 0)
				{
					BSP_tracePrintError(TR_CLASS_BSP, "reading data from HDRADIO, CONFIG_COMM_HDRADIO_I2C, HDRADIO_MS_TO_HAVE_FIFO_FILLED  ret = %d", ret);
					// Update the caller buffer to throw away the "byte-count" byte
					(*pBuf) += 1;
					ret_fct = (tS32)lmAchievedBytes;
					goto exit;
				}
				else if ((tU32)ret < (tU32)lmBytesToRead+1) /* both casts to get rid of compiler warning */
				{
					BSP_tracePrintError(TR_CLASS_BSP, "truncated data read from HDRADIO");
					if (0 == lmBufferIndex)
					{
						// Copy another message chunk
						s32Temp = ret-1;
						(void)OSAL_pvMemoryCopy ((tVoid *)(pLocalBuf+HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH), (tPCVoid)&lmTmpBuffer[1], (tU32)(s32Temp));
					}
					else
					{
						// Copy another message chunk
						s32Temp = ret-1;
						(void)OSAL_pvMemoryCopy ((tVoid *)(pLocalBuf+lmBufferIndex), (tPCVoid)&lmTmpBuffer[1], (tU32)(s32Temp));
					}

					// Update the caller buffer after that the local pointer was incremented to throw away the "byte-count" byte
					*pBuf = pLocalBuf;
					s32Temp = ret-1;
					lmAchievedBytes += (tU16)(s32Temp);
					ret_fct = (tS32)lmAchievedBytes;
					goto exit;
				}
				else
				{
					/* Nothing to do */
				}

				if (0 == lmBufferIndex)
				{
					// Copy another message chunk
					(void)OSAL_pvMemoryCopy ((tVoid *)(pLocalBuf+HDRADIO_FIFO_BYTECOUNT_MIN_LMLENGTH), (tPCVoid)&lmTmpBuffer[1], lmBytesToRead);
				}
				else
				{
					// Copy another message chunk
					(void)OSAL_pvMemoryCopy ((tVoid *)(pLocalBuf+lmBufferIndex), (tPCVoid)&lmTmpBuffer[1], lmBytesToRead);
				}

				// Update the bytes actually read
				s32Temp = ret-1;
				lmAchievedBytes += (tU16)(s32Temp);

				// Decrement the bytes read and the packets processed
				lmRemainingBytes -= lmBytesToRead;
				if (lmRemainingPackets > 0)
				{
					lmRemainingPackets--;
				}

				// Increment number of packets read
				lmBufferIndex += HDRADIO_FIFO_SIZE_IN_BYTES;
			}

		// Update the caller buffer after that the local pointer was incremented to throw away the "byte-count" byte
		*pBuf = pLocalBuf;
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT,
			(HDRADIO_GetBusType(deviceID) == BusSPI) ? 0 : 1,
			*pBuf, len, BSP_READ_OPERATION);
#endif

	ret_fct = (tS32)lmAchievedBytes;
exit:
	return ret_fct;
}

/**************************************
 *
 * BSP_Raw_Read_HDRADIO
 *
 *************************************/
tS32 BSP_Raw_Read_HDRADIO(tU32 deviceID, tU8 *pBuf, tU32 len)
{
	tS32 ret = -1;
	tU16 bufIndex;

	if(HDRADIO_GetBusType(deviceID) == BusSPI)
	{
		if(((tySpiCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->mode == SPI_CPHA0_CPOL0)
		{
			ret = 0;
			for (bufIndex = 0; bufIndex < len; bufIndex++)
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret += spi_read(HDRADIO_SPI_LinuxDevice_fd, &(pBuf[bufIndex]), 1, false);
				BSP_SPI0DriveCS_HDRADIO (1);
			}
		}
		else
		{
#ifdef ETAL_BSP_DEBUG_SPI_RW_WORD	
			// specific handling for small datasize
			// in case the data are < 4, change datasize for byte to byte transport
			// 
			if (len < 4)
			{
				// change datasize
				(void)BSP_BusConfigSPIDataSize_HDRADIO(deviceID, false);

				// read data
				
				BSP_SPI0DriveCS_HDRADIO (0);
				ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, pBuf, len, false);
				BSP_SPI0DriveCS_HDRADIO (1);

				//set back the datasize
				// change datasize
				(void)BSP_BusConfigSPIDataSize_HDRADIO(deviceID, true);
			}
			else
			{
				BSP_SPI0DriveCS_HDRADIO (0);
				ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, pBuf, len, true);
				BSP_SPI0DriveCS_HDRADIO (1);
			}
#else
			BSP_SPI0DriveCS_HDRADIO (0);
			ret = spi_read (HDRADIO_SPI_LinuxDevice_fd, pBuf, len, false);
			BSP_SPI0DriveCS_HDRADIO (1);
#endif
		}
	}
	else
	{
		ret = (tS32)tnr_read (HDRADIO_I2C_LinuxDevice_fd, 
						((tyI2cCommunicationBus*)HDRADIO_GetCommunicationBusConfig(deviceID))->deviceAddress,
						pBuf, (size_t)len);
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT,
			(HDRADIO_GetBusType(deviceID) == BusSPI) ? 0 : 1,
			pBuf, len, BSP_READ_OPERATION);
#endif

	if (ret < 0)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "raw reading response from HDRADIO");
	}
	else if (ret < (tS32)len)
	{
		BSP_tracePrintError(TR_CLASS_BSP, "truncated raw read response from HDRADIO");
		ret = -1;
	}
	else
	{
		/* Nothing to do */
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
#endif

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
#ifdef ETAL_BSP_DEBUG_SPI_RW_WORD	
		// specific handling for small datasize
		// in case the data are < 4, change datasize for byte to byte transport
		// 
		if (len < 4)
		{
			// change datasize
			(void)BSP_BusConfigSPIDataSize_HDRADIO(deviceID, false);

			BSP_SPI0DriveCS_HDRADIO (0);
			BSP_Linux_TransferSpi(HDRADIO_SPI_LinuxDevice_fd, buf_wr, buf_rd, len, false);
			BSP_SPI0DriveCS_HDRADIO (1);

			// change datasize back
			(void)BSP_BusConfigSPIDataSize_HDRADIO(deviceID, true);
		}
		else
		{
			BSP_SPI0DriveCS_HDRADIO (0);
			BSP_Linux_TransferSpi(HDRADIO_SPI_LinuxDevice_fd, buf_wr, buf_rd, len, true);
			BSP_SPI0DriveCS_HDRADIO (1);
		}
#else
		BSP_SPI0DriveCS_HDRADIO (0);
		BSP_Linux_TransferSpi(HDRADIO_SPI_LinuxDevice_fd, buf_wr, buf_rd, len, false);
		BSP_SPI0DriveCS_HDRADIO (1);
#endif
	}

#if (defined CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
	BSP_PrintBufNoSpace(TR_LEVEL_COMPONENT, 0, buf_rd, len, BSP_READ_OPERATION);
#endif
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

#endif // CONFIG_BUILD_DRIVER &&  CONFIG_HOST_OS_TKERNEL

