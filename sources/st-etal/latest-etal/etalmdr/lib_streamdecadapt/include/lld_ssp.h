////////////////////////////////////////////////////////////////////
//             C STMicroelectronics
//   Reproduction and Communication of this document is
//   strictly prohibited unless specifically autorized in
//   writing by STMicroelectronics.
////////////////////////////////////////////////////////////////////
//
// Public Header file of Synchronous Serial Port
// (PL022) module
// Specification release related to this implementation: A_V1.2
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
///  \file  ssp.h
///  \brief <i><b>Public Header file of Synchronous Serial Port </b></i>
///  \author Emanuela Zaccaria
///  \author (original version)  HCL, Luigi Cotignano
///  \version
///  \date    2008-2009
///  \bug Unknown.
///  \warning None.
////////////////////////////////////////////////////////////////////


#ifndef __INC_SSP_H
#define __INC_SSP_H

#ifdef	__cplusplus
extern "C"
{
#endif

////////////////////////////////////////////////////////////////////
// Defines and macros
////////////////////////////////////////////////////////////////////
// SSP Interrupt Mask Set/Clear Register
// SSP Interrupt Mask Set/Clear Register (SSP_IMSC)
#define SSP_IMSC_MASK_RORIM			0x01  // Receive Overrun Interrupt mask
#define SSP_IMSC_MASK_RTIM			0x02  // Receive timeout Interrupt mask
#define SSP_IMSC_MASK_RXIM			0x04  // Receive FIFO Interrupt mask
#define SSP_IMSC_MASK_TXIM			0x08  // Transmit FIFO Interrupt mask

// SSP Raw Interrupt Status Register (SSP_RIS)
#define SSP_RIS_MASK_RORRIS			0x01  // Receive Overrun Raw Interrupt status
#define SSP_RIS_MASK_RTRIS			0x02  // Receive Timeout Raw Interrupt status
#define SSP_RIS_MASK_RXRIS			0x04  // Receive FIFO Raw Interrupt status
#define SSP_RIS_MASK_TXRIS			0x08  // Transmit FIFO Raw Interrupt status

// SSP Masked Interrupt Status Register (SSP_MIS)
#define SSP_MIS_MASK_RORMIS			0x01  // Receive Overrun Masked Interrupt status
#define SSP_MIS_MASK_RTMIS			0x02  // Receive Timeout Masked Interrupt status
#define SSP_MIS_MASK_RXMIS			0x04  // Receive FIFO Masked Interrupt status
#define SSP_MIS_MASK_TXMIS			0x08  // Transmit FIFO Masked Interrupt status

// SSP Interrupt Clear Register (SSP_ICR)
#define SSP_ICR_MASK_RORIC			0x01  // Receive Overrun Raw Clear Interrupt bit
#define SSP_ICR_MASK_RTIC			0x02  // Receive Timeout Clear Interrupt bit

// SSP Data Register
// Transmit or Receive Data
#define SSP_DR_MASK_DATA		0xFFFFFFFF

// SSP Status Register
#define SSP_SR_MASK_TFE 0x01  // Transmit FIFO empty
#define SSP_SR_MASK_TNF 0x02  // Transmit FIFO not full
#define SSP_SR_MASK_RNE 0x04  // Receive FIFO not empty
#define SSP_SR_MASK_RFF 0x08  // Receive FIFO full
#define SSP_SR_MASK_BSY 0x10  // Busy Flag

// SSP DMA Control Register
#define SSP_DMACR_MASK_RXDMAE 0x01  // Receive DMA Enable bit
#define SSP_DMACR_MASK_TXDMAE 0x02  // Transmit DMA Enable bit

#define MIN_CPSDVR 0x02
#define MAX_CPSDVR 0xFE
#define MIN_SCR 0x00
#define MAX_SCR 0xFF
#define SSP_ENABLE_ALL_INTERRUPT  0x0F
#define SSP_DISABLE_ALL_INTERRUPT  0x00
#define SSP_LAST8BITS 0x000000FF
#define SSP_ALLZERO   0x00000000

// Define depending on SSP Type: Lite or Full
#define SSP_PERIPHID0 0x22
#define SSP_PERIPHID1 0x00
#define SSP_PERIPHID2 0x08
#define SSP_PERIPHID3 0x01
#define SSP_PCELLID0  0x0D
#define SSP_PCELLID1  0xF0
#define SSP_PCELLID2  0x05
#define SSP_PCELLID3  0xB1


#define LLD_SSP0_ADDRESS           SSP0_REG_START_ADDR
#define LLD_SSP1_ADDRESS           SSP1_REG_START_ADDR
#ifdef PLATFORM_IS_ADR3
	#define LLD_SSP2_ADDRESS           SSP2_REG_START_ADDR
	#define LLD_SSP3_ADDRESS           SSP3_REG_START_ADDR
	#define LLD_SSP4_ADDRESS           SSP4_REG_START_ADDR
#endif
////////////////////////////////////////////////////////////////////
// Structures and enums
////////////////////////////////////////////////////////////////////

// Interrupt related enum

// Enum to check which interrupt is asserted
typedef enum
{
	SSP_IRQ_SRC_TRANSMIT=0x08, 			//  Asserted when the number of elements in Tx
									  	//  FIFO is less than the programmed Watermark level
	SSP_IRQ_SRC_RECEIVE=0x04, 			//  Asserted when the number of elements in Rx
										//  FIFO is more than the programmed Watermark level
	SSP_IRQ_SRC_RECEIVE_TIMEOUT=0x02, 	//  Asserted when Rx FIFO is not empty & no
										//  further data is received over a 32 bit period
	SSP_IRQ_SRC_RECEIVE_OVERRUN=0x01 	//  Receive FIFO is already full & an additional
										//  frame is received
} t_ssp_irq_src_id;

typedef tU32 t_ssp_irq_src ;


//////////////////////////////////////////////////////////////////////////////////////
// The following variables will be defined in the SSP test file (utils_t_valid_ssp.c)
// as global variables.
typedef volatile struct
{
    tVoid             *TransferPointer;   // Pointer to user data buffer
	tU32         TransferSize;       // Number of (AddressingMode width) transfers
} SSP_TransferState;


typedef struct
{
    tBool   rcv_flag;
    tBool   trans_flag;
    tBool   rcv_timeout;
    tBool   rcv_overrun;
    tBool   receive_it_mode;
    tBool   transmit_it_mode;
    tU32    rx_index;
    tU32    tx_index;
    tU32    rx_trig_level;
    tU32    tx_trig_level;
    tU32    transmit_size;
    tU32    receive_size;
} ser_ssp_context;

// Defines for instances  // Now defined in the test code
// #define SER_NUM_SSP_INSTANCES	2
// extern volatile ser_ssp_context g_ser_ssp_context[SER_NUM_SSP_INSTANCES];
//////////////////////////////////////////////////////////////////////////////////////



// Enum to identify SSP device blocks
typedef enum
{
    SSP_DEVICE_ID_0,
    SSP_DEVICE_ID_1,
    SSP_DEVICE_ID_INVALID       = 0x2
} t_ssp_device_id;


//	Define SSP errors
typedef enum
{
   SSP_OK 							= 0,  //HCL_OK,
   SSP_UNSUPPORTED_HW 				= -1, //HCL_UNSUPPORTED_HW,
   SSP_INVALID_PARAMETER			= -4, //HCL_INVALID_PARAMETER,
   SSP_UNSUPPORTED_FEATURE 		    = -4, //HCL_UNSUPPORTED_FEATURE,
   SSP_REQUEST_NOT_APPLICABLE		= -5, //HCL_REQUEST_NOT_APPLICABLE,
   SSP_INTERNAL_ERROR 				= -8, //HCL_INTERNAL_ERROR,
   SSP_RECEIVE_ERROR 				= -9
} t_ssp_error;

// enabling Rx or Tx of SSP
typedef enum
{
	SSP_ENABLE_RX_TX,					//  Both RX and TX are enabled.
	SSP_ENABLE_RX_DISABLE_TX,			//  RX enabled, and TX disabled.
	SSP_DISABLE_RX_ENABLE_TX,			//  RX disabled, and TX enabled
	SSP_DISABLE_RX_TX					//  both RX and TX disabled.
} t_ssp_enable;



// Interface Type
typedef enum
{
	SSP_INTERFACE_MOTOROLA_SPI,         // Motorola Interface
	SSP_INTERFACE_TI_SYNC_SERIAL,       // Texas Instrument Synchronous Serial interface
	SSP_INTERFACE_NATIONAL_MICROWIRE,   // National Semiconductor Microwire interface
	SSP_INTERFACE_UNIDIRECTIONAL        // Unidirectional interface (STn8810&STn8815 only)
} t_ssp_interface;


// master or slave configuration
typedef enum
{
	SSP_MASTER,							//  MuPoC SSP is master (provides the clock)
	SSP_SLAVE							//  MuPoC SSP is slave (receives the clock)

} t_ssp_hierarchy;


// Clock parameters
typedef struct
{
	tU8 cpsdvsr;					//  value from 2 to 254 (even only!)
	tU8 scr;						//  value from 0 to 255

} t_ssp_clock_params;


// Endianness
typedef enum
{
	SSP_RX_MSB_TX_MSB,				// receive: MSBit first & transmit: MSBit first
	SSP_RX_MSB_TX_LSB,				// receive: MSBit first & transmit: LSBit first
	SSP_RX_LSB_TX_MSB,				// receive: LSBit first & transmit: MSBit first
	SSP_RX_LSB_TX_LSB				// receive: LSBit first & transmit: LSBit first

} t_ssp_endian;


// Number of bits in one data element

typedef enum
{
    SSP_DATA_BITS_4 = 0x03,
    SSP_DATA_BITS_5,
    SSP_DATA_BITS_6,
    SSP_DATA_BITS_7,
    SSP_DATA_BITS_8,
    SSP_DATA_BITS_9,
    SSP_DATA_BITS_10,
    SSP_DATA_BITS_11,
    SSP_DATA_BITS_12,
    SSP_DATA_BITS_13,
    SSP_DATA_BITS_14,
    SSP_DATA_BITS_15,
    SSP_DATA_BITS_16,
    SSP_DATA_BITS_17,
    SSP_DATA_BITS_18,
    SSP_DATA_BITS_19,
    SSP_DATA_BITS_20,
    SSP_DATA_BITS_21,
    SSP_DATA_BITS_22,
    SSP_DATA_BITS_23,
    SSP_DATA_BITS_24,
    SSP_DATA_BITS_25,
    SSP_DATA_BITS_26,
    SSP_DATA_BITS_27,
    SSP_DATA_BITS_28,
    SSP_DATA_BITS_29,
    SSP_DATA_BITS_30,
    SSP_DATA_BITS_31,
    SSP_DATA_BITS_32
}t_ssp_data_size;


// Receive FIFO watermark level which triggers IT
typedef enum
{
	SSP_RX_1_OR_MORE_ELEM,				// IT fires when 1 or more elements in RX FIFO
	SSP_RX_4_OR_MORE_ELEM,				// IT fires when 4 or more elements in RX FIFO
	SSP_RX_8_OR_MORE_ELEM,				// IT fires when 8 or more elements in RX FIFO
	SSP_RX_16_OR_MORE_ELEM				// IT fires when 16 or more elements in RX FIFO
	//SSP_RX_32_OR_MORE_ELEM			// IT fires when 32 or more elements in RX FIFO

} t_ssp_rx_level_trig;


// Transmit FIFO watermark level which triggers IT
typedef enum
{
	SSP_TX_1_OR_MORE_EMPTY_LOC,			// IT fires when 1 or more empty locations in TX FIFO
	SSP_TX_4_OR_MORE_EMPTY_LOC,			// IT fires when 4 or more empty locations in TX FIFO
	SSP_TX_8_OR_MORE_EMPTY_LOC,			// IT fires when 8 or more empty locations in TX FIFO
	SSP_TX_16_OR_MORE_EMPTY_LOC		    // IT fires when 16 or more empty locations in TX FIFO
	//SSP_TX_32_OR_MORE_EMPTY_LOC		// IT fires when 32 or more empty locations in TX FIFO

} t_ssp_tx_level_trig;


// clock phase (Motorola SPI interface only)
typedef enum
{
   SSP_CLK_FALLING_EDGE,				// Receive data on falling edge.
   SSP_CLK_RISING_EDGE                  // Receive data on rising edge.
} t_ssp_spi_clk_phase;


// clock polarity (Motorola SPI interface only)
typedef enum
{
	SSP_CLK_POL_IDLE_LOW,				//  Low inactive level
	SSP_CLK_POL_IDLE_HIGH				//  High inactive level
} t_ssp_spi_clk_pol;


// Command size in microwire format
typedef enum
{
    SSP_BITS_4 = 0x03,
    SSP_BITS_5,
    SSP_BITS_6,
    SSP_BITS_7,
    SSP_BITS_8,
    SSP_BITS_9,
    SSP_BITS_10,
    SSP_BITS_11,
    SSP_BITS_12,
    SSP_BITS_13,
    SSP_BITS_14,
    SSP_BITS_15,
    SSP_BITS_16,
    SSP_BITS_17,
    SSP_BITS_18,
    SSP_BITS_19,
    SSP_BITS_20,
    SSP_BITS_21,
    SSP_BITS_22,
    SSP_BITS_23,
    SSP_BITS_24,
    SSP_BITS_25,
    SSP_BITS_26,
    SSP_BITS_27,
    SSP_BITS_28,
    SSP_BITS_29,
    SSP_BITS_30,
    SSP_BITS_31,
    SSP_BITS_32
} t_ssp_microwire_ctrl_len;



// wait state
typedef enum
{
	SSP_MICROWIRE_WAIT_ZERO,			//  No wait state inserted after last command bit
	SSP_MICROWIRE_WAIT_ONE				//  One wait state inserted after last command bit
} t_ssp_microwire_wait_state;


// Full/Half Duplex
// !!! WARNING: SPI Motorola does not support native half duplex protocols.

typedef enum
{
	SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,   //  SSPTXD becomes bi-directional, SSPRXD not used
	SSP_MICROWIRE_CHANNEL_HALF_DUPLEX    //  SSPTXD is an output, SSPRXD is an input.
} t_ssp_duplex;


// communication mode
typedef enum
{
	SSP_POLLING_MODE,					//  Polling mode
	SSP_IT_MODE,						//  Interrupt mode
	SSP_DMA_MODE						// DMA mode
} t_ssp_mode;

// SSP FIFO status
typedef enum
{
	SSP_TRANSMIT_FIFO_EMPTY    = 0x01,
	SSP_TRANSMIT_FIFO_NOT_FULL = 0x02,
	SSP_RECEIVE_FIFO_NOT_EMPTY = 0x04,
	SSP_RECEIVE_FIFO_FULL      = 0x08,
	SSP_BUSY                   = 0x10
} t_ssp_fifo_status;

// Configuration parameters
typedef struct
{
	t_ssp_interface iface;			 		// Interface type
	t_ssp_hierarchy hierarchy;		 		// sets whether interface is master or slave
	tBool slave_tx_disable; 				// SSPTXD is disconnected (in slave mode only)
	t_ssp_clock_params clk_freq;			// Freq. of SSP interface (when master)
	t_ssp_endian endian; 			 		// sets whether MSBit or LSBit is first
	t_ssp_data_size data_size;		 		// size of data elements (4 to 32 bits)
	t_ssp_mode txcom_mode;			 		// tx communication mode : polling, IT or DMA
	t_ssp_mode rxcom_mode; 			 		// rx communication mode : polling, IT or DMA
	t_ssp_rx_level_trig rx_lev_trig; 		// Rx FIFO watermark level (for IT & DMA mode)
	t_ssp_tx_level_trig tx_lev_trig; 		// Tx FIFO watermark level (for IT & DMA mode)
	t_ssp_spi_clk_phase clk_phase; 			// Motorola SPI interface Clock phase
	t_ssp_spi_clk_pol clk_pol; 				// Motorola SPI interface Clock polarity
	t_ssp_microwire_ctrl_len ctrl_len; 		// Microwire interface: Control length
	t_ssp_microwire_wait_state wait_state;	// Microwire interface: Wait state
	t_ssp_duplex duplex; 			        // Microwire interface: Full/Half duplex
	tBool loopback;                         //loop back mode
} t_ssp_config;






////////////////////////////////////////////////////////////////////
// Functions prototypes
////////////////////////////////////////////////////////////////////
t_ssp_error  LLD_SSP_Init(SspMap*  );
t_ssp_error  LLD_SSP_Enable(SspMap *, t_ssp_enable,   t_ssp_config * );
t_ssp_error  LLD_SSP_EnableLoopBackMode(SspMap *, tBool  );
t_ssp_error  LLD_SSP_Reset( SspMap *  );
t_ssp_error  LLD_SSP_ResolveClockFrequency(tU32, tU32, tU32 *, t_ssp_clock_params *);
t_ssp_error  LLD_SSP_SetDataSize (SspMap *SspAdd, t_ssp_data_size data_size );
t_ssp_error  LLD_SSP_SetConfiguration(SspMap * , t_ssp_config * );
t_ssp_error  LLD_SSP_SetData(SspMap * , tU32 );
t_ssp_error  LLD_SSP_GetData(SspMap *, tU32 * );
t_ssp_error  LLD_SSP_GetFIFOStatus(SspMap * , tU32 *  );
t_ssp_error  LLD_SSP_FIFOFlush(SspMap *   );
t_ssp_irq_src	LLD_SSP_GetIRQSrc( SspMap *  );
tU32 LLD_SSP_GetAllData(SspMap *, SSP_TransferState  * );
tU32 LLD_SSP_SetAllData(SspMap *, SSP_TransferState  *  );
tU32 LLD_SSP_DataRegAddress(SspMap * );
tVoid LLD_SSP_EnableIRQSrc( SspMap *,  t_ssp_irq_src  );
tVoid LLD_SSP_DisableIRQSrc(SspMap *, t_ssp_irq_src   );
tVoid LLD_SSP_ClearIRQSrc(SspMap *,  t_ssp_irq_src  );
tVoid LLD_SSP_InitInterface( SspMap *, tU8, tU32, tU8, tU8);
tBool LLD_SSP_IsPendingIRQSrc(SspMap *, t_ssp_irq_src_id );
tBool LLD_SSP_IsMaster( SspMap * );
t_ssp_error  LLD_SSP_IsBusy(SspMap * );
t_ssp_error  LLD_SSP_IsRxFifoFull(SspMap *  );
t_ssp_error  LLD_SSP_IsRxFifoNotEmpty(SspMap *  );
t_ssp_error  LLD_SSP_IsTxFifoNotFull(SspMap *  );
t_ssp_error  LLD_SSP_IsTxFifoEmpty(SspMap *  );
tVoid LLD_SSP_SetClkPol(SspMap *SspAdd, t_ssp_spi_clk_pol clk_pol);
tVoid LLD_SSP_SetClkPhase(SspMap *SspAdd, t_ssp_spi_clk_phase clk_phase);
tVoid LLD_SSP_SetMode(SspMap *SspAdd, t_ssp_hierarchy mode);
tS32 LLD_SSP_SetBaudRate(SspMap *SspAdd, tU32 rate, tU32 sys_clock);


#if (LLD_SSP_STA660_ROM_USED == TRUE)
#define LLD_SSP_Init(a)                                      ((t_ssp_error (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_Init])(a)
#define LLD_SSP_Enable(a,b,c)                                ((t_ssp_error (*)(SspMap *,t_ssp_enable,t_ssp_config *))ROM_LLD_Table[ENU_LLD_SSP_Enable])(a,b,c)
#define LLD_SSP_EnableLoopBackMode(a,b)                      ((t_ssp_error (*)(SspMap *,tBool))ROM_LLD_Table[ENU_LLD_SSP_EnableLoopBackMode])(a,b)
#define LLD_SSP_Reset(a)                                     ((t_ssp_error (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_Reset])(a)
#define LLD_SSP_ResolveClockFrequency(a,b,c,d)               ((t_ssp_error (*)(tU32,tU32,tU32 *,t_ssp_clock_params *))ROM_LLD_Table[ENU_LLD_SSP_ResolveClockFrequency])(a,b,c,d)
#define LLD_SSP_SetConfiguration(a,b)                        ((t_ssp_error (*)(SspMap *,t_ssp_config *))ROM_LLD_Table[ENU_LLD_SSP_SetConfiguration])(a,b)
#define LLD_SSP_SetData(a,b)                                 ((t_ssp_error (*)(SspMap *,tU32))ROM_LLD_Table[ENU_LLD_SSP_SetData])(a,b)
#define LLD_SSP_GetData(a,b)                                 ((t_ssp_error (*)(SspMap *,tU32 *))ROM_LLD_Table[ENU_LLD_SSP_GetData])(a,b)
#define LLD_SSP_GetFIFOStatus(a,b)                           ((t_ssp_error (*)(SspMap *,tU32 *))ROM_LLD_Table[ENU_LLD_SSP_GetFIFOStatus])(a,b)
#define LLD_SSP_FIFOFlush(a)                                 ((t_ssp_error (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_FIFOFlush])(a)
#define LLD_SSP_GetIRQSrc(a)                                 ((t_ssp_irq_src (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_GetIRQSrc])(a)
#define LLD_SSP_GetAllData(a,b)                              ((tU32 (*)(SspMap *,SSP_TransferState *))ROM_LLD_Table[ENU_LLD_SSP_GetAllData])(a,b)
#define LLD_SSP_SetAllData(a,b)                              ((tU32 (*)(SspMap *,SSP_TransferState *))ROM_LLD_Table[ENU_LLD_SSP_SetAllData])(a,b)
#define LLD_SSP_DataRegAddress(a)                            ((tU32 (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_DataRegAddress])(a)
#define LLD_SSP_EnableIRQSrc(a,b)                            ((tVoid (*)(SspMap *,t_ssp_irq_src))ROM_LLD_Table[ENU_LLD_SSP_EnableIRQSrc])(a,b)
#define LLD_SSP_DisableIRQSrc(a,b)                           ((tVoid (*)(SspMap *,t_ssp_irq_src))ROM_LLD_Table[ENU_LLD_SSP_DisableIRQSrc])(a,b)
#define LLD_SSP_ClearIRQSrc(a,b)                             ((tVoid (*)(SspMap *,t_ssp_irq_src))ROM_LLD_Table[ENU_LLD_SSP_ClearIRQSrc])(a,b)
#define LLD_SSP_InitInterface(a,b,c,d,e)                     ((tVoid (*)(SspMap *,tU8,tU32,tU8,tU8))ROM_LLD_Table[ENU_LLD_SSP_InitInterface])(a,b,c,d,e)
#define LLD_SSP_IsPendingIRQSrc(a,b)                         ((tBool (*)(SspMap *,t_ssp_irq_src_id))ROM_LLD_Table[ENU_LLD_SSP_IsPendingIRQSrc])(a,b)
#define LLD_SSP_IsMaster(a)                                  ((tBool (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_IsMaster])(a)
#define LLD_SSP_IsBusy(a)                                    ((t_ssp_error (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_IsBusy])(a)
#define LLD_SSP_IsRxFifoFull(a)                              ((t_ssp_error (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_IsRxFifoFull])(a)
#define LLD_SSP_IsRxFifoNotEmpty(a)                          ((t_ssp_error (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_IsRxFifoNotEmpty])(a)
#define LLD_SSP_IsTxFifoNotFull(a)                           ((t_ssp_error (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_IsTxFifoNotFull])(a)
#define LLD_SSP_IsTxFifoEmpty(a)                             ((t_ssp_error (*)(SspMap *))ROM_LLD_Table[ENU_LLD_SSP_IsTxFifoEmpty])(a)

#define LLD_SSP_SetClkPol(a,b) 						   		 ((tVoid (*)(SspMap *,t_ssp_spi_clk_pol))ROM_LLD_Table[ENU_LLD_SSP_SetClkPol])(a,b)
#define LLD_SSP_SetClkPhase(a,b) 							 ((tVoid (*)(SspMap *,t_ssp_spi_clk_phase))ROM_LLD_Table[ENU_LLD_SSP_SetClkPhase])(a,b)
#define LLD_SSP_SetMode(a,b) 								 ((tVoid (*)(SspMap *,t_ssp_hierarchy))ROM_LLD_Table[ENU_LLD_SSP_SetMode])(a,b)
#define LLD_SSP_SetBaudRate(a,b,c) 							 ((tS32 (*)(SspMap *,tU32,tU32))ROM_LLD_Table[ENU_LLD_SSP_SetMode])(a,b,c)


#endif


#ifdef __cplusplus
} // allow C++ to use these headers
#endif	// __cplusplus
#endif // __INC_SSP_H

// End of file - ssp.h
