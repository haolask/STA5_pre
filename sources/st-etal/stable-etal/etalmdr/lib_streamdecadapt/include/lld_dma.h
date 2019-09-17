// _____________________________________________________________________________
//| FILE:         lld_dma.h
//| PROJECT:      ADR3 - STA660
//|_____________________________________________________________________________
//| DESCRIPTION:  DMA low level driver header file
//|_____________________________________________________________________________
//| COPYRIGHT:    (c) 2009 STMicroelectronics, Agrate Brianza (MI) (ITALY)
//|
//| HISTORY:
//| Date        | Modification               | Author
//|_____________________________________________________________________________
//| 2009.06.26  | Porting from Newcastle     | A^_^L
//|               project
//| 2009.06.26  | Initial revision           | A^_^L
//|_____________________________________________________________________________

//------------------------------------------------------------------------------
//!  \file lld_dma.h
//!  \brief <i><b>DMA low level driver header file</b></i>
//!  \author Alberto Saviotti
//!  \author (original version) Luigi Cotignano
//!  \version 1.0
//!  \date 2009.06.26
//!  \bug Unknown
//!  \warning None
//------------------------------------------------------------------------------

#ifndef _LLD_DMA_H_
#define _LLD_DMA_H_

//----------------------------------------------------------------------
// defines
//----------------------------------------------------------------------
//! \def LLD_DMA_ENABLE
//!      Define for DMA enable.
#define LLD_DMA_ENABLE          	(SET)
//! \def LLD_DMA_DISABLE
//!      Define for DMA disable.
#define LLD_DMA_DISABLE         	(CLEAR)
//! \def LLD_DMA_CHANNELS_NUM
//!      Number of channel for a DMA cell.
#define LLD_DMA_CHANNELS_NUM    	(8)
//! \def LLD_DMA_MAX_TRANSFERS
//!      Maximum number of DMA movements.
#define LLD_DMA_MAX_TRANSFERS		(0xFFF)

//! \def DMA_0_ID
//!      DMA ID for the first DMA (total of 2 DMA are present).
#define DMA_0_ID					(LLD_DMA_IdTy)0
//! \def DMA_1_ID
//!      DMA ID for the second DMA (total of 2 DMA are present).
#define DMA_1_ID					(LLD_DMA_IdTy)1

//! \def DMA_0_1_OFFSET
//!      Offset between the first and the second DMAs memory addresses.
#define DMA_0_1_OFFSET				((tU32)(DMA2_REG_START_ADDR - DMA1_REG_START_ADDR))

//! \def DMA_REG_START_ADDR
//!      DMA primecell and DMA multiplexer base addresses
#define DMA_REG_START_ADDR			DMA1_REG_START_ADDR

#define LLD_DMA_ADDRESS(id)		(DmaMap*)(DMA_REG_START_ADDR + (DMA_0_1_OFFSET * id));

//----------------------------------------------------------------------
// typedefs
//----------------------------------------------------------------------
//!
//! \typedef LLD_DMA_IdTy
//!          DMA identifier.
//!          Define the DMA addressed by the caller.
//!          2 different DMA instances are embedded:
//!          - DMA ID = 0
//!			 - DMA ID = 1.
//!
typedef tU32 LLD_DMA_IdTy;

// Peripheral's line identifier
typedef tU16 LLD_DMA_PeripheralTy;

//!
//! \enum LLD_DMA_EndianessTy
//! This enumerator indicates GPIO mode on IC set into SLEEP mode:
//! - LLD_DMA_LITTLE_ENDIAN, DMA endianess set to little endian
//! - LLD_DMA_BIG_ENDIAN, DMA endianess set to big endian.
//!
typedef enum
{
    LLD_DMA_LITTLE_ENDIAN = 0x0,
    LLD_DMA_BIG_ENDIAN    = 0x1
}
LLD_DMA_EndianessTy;

// DMA master
typedef enum
{
    LLD_DMA_MASTER1 = 0,
    LLD_DMA_MASTER2 = 1
}
LLD_DMA_MasterTy;

// DMA burst size
typedef enum
{
    LLD_DMA_BURST_SIZE_1   = 0x0,
    LLD_DMA_BURST_SIZE_4   = 0x1,
    LLD_DMA_BURST_SIZE_8   = 0x2,
    LLD_DMA_BURST_SIZE_16  = 0x3,
    LLD_DMA_BURST_SIZE_32  = 0x4,
    LLD_DMA_BURST_SIZE_64  = 0x5,
    LLD_DMA_BURST_SIZE_128 = 0x6,
    LLD_DMA_BURST_SIZE_256 = 0x7
}
LLD_DMA_BurstSizeTy;

// DMA transfer width
typedef enum
{
    LLD_DMA_BYTE_WIDTH     = 0x0,
    LLD_DMA_HALFWORD_WIDTH = 0x1,
    LLD_DMA_WORD_WIDTH     = 0x2
}
LLD_DMA_TransferWidthTy;

// DMA flow control
typedef enum
{
    MEMORY_TO_MEMORY_DMA_CONTROLLER                             = 0x0,
    MEMORY_TO_PERIPHERAL_DMA_CONTROLLER                         = 0x1,
    PERIPHERAL_TO_MEMORY_DMA_CONTROLLER                         = 0x2,
    PERIPHERAL_TO_PERIPHERAL_DMA_CONTROLLER                     = 0x3,
    PERIPHERAL_TO_PERIPHERAL_DESTINATION_PERIPHERAL_CONTROLLER  = 0x4,
    MEMORY_TO_PERIPHERAL_PERIPHERAL_CONTROLLER                  = 0x5,
    PERIPHERAL_TO_MEMORY_PERIPHERAL_CONTROLLER                  = 0x6,
    PERIPHERAL_TO_PERIPHERAL_SOURCE_PERIPHERAL_CONTROLLER       = 0x7
}
LLD_DMA_FlowControlTy;

//----------------------------------------------------------------------
// DMA 0: Request Line DMA Source
//----------------------------------------------------------------------
// Request Line		DMA Source
// 0 				Reserved
// 1 				Reserved
// 2 				Reserved
// 3 				EFT3
// 4 				EFT2
// 5 				SD/SDIO/MMC 1
// 6 				Ext. Req DREQ0
// 7 				Ext. Req DREQ1
// 8 				EFT0
// 9 				IrDA(SIR/MIR/FIR) Tx /Rx
// 10 				MSP0 Rx
// 11 				MSP0 Tx
// 12 				SSP0 Rx
// 13 				SSP0 Tx
// 14 				UART0 Rx
// 15 				UART0 Tx
// 16 				EFT1
// 17 				USB FS CH0
// 18 				USB FS CH1
// 19 				USB FS CH2
// 20 				USB FS CH3
// 21 				USB FS CH4
// 22 				MSP2 Rx
// 23 				MSP2 Tx
// 24 				USB FS CH5
// 25 				Reserved
// 26 				SRC_DMA_DRF_REQ
// 27 				SRC_DMA_DRE_REQ
// 28 				MSP3 Rx
// 29 				MSP3 Tx
// 30 				MSP1Rx
// 31 				MSP1Tx

//----------------------------------------------------------------------
// DMA 1: Request Line DMA Source
//----------------------------------------------------------------------
// Request Line		DMA Source
// 0 				Reserved
// 1 				Reserved
// 2 				Reserved
// 3 				Reserved
// 4 				I2C2 Tx/Rx
// 5 				Reserved
// 6 				Ext. Req DREQ0
// 7 				Ext. Req DREQ1
// 8 				SPDIF
// 9 				IrDA(SIR/MIR/FIR) Tx/Rx
// 10				MSP0 Rx
// 11 				MSP0 Tx
// 12 				SSP1Rx
// 13 				SSP1Tx
// 14 				UART0 Rx
// 15 				UART0 Tx
// 16 				UART3 Rx
// 17 				UART3 Tx
// 18 				I2C1 Tx/Rx
// 19 				I2C0 Tx/Rx
// 20 				CHITF (RS)
// 21 				SD/SDIO/MMC 0
// 22 				UART1 Rx
// 23 				UART1 Tx
// 24 				USB OTG Ch. 0
// 25 				USB OTG Ch. 1
// 26 				USB OTG Ch. 2
// 27 				USB OTG Ch. 3
// 28 				USB OTG Ch. 4
// 29 				USB OTG Ch. 5
// 30 				UART2 Rx
// 31 				UART2 Tx

typedef enum
{


#ifndef PLATFORM_IS_ADR3
	// DMA 0: Request Line DMA Source
	D0_RESERVED0			= 0,
	D0_RESERVED1			= 1,
	D0_RESERVED2			= 2,
	D0_EFT3					= 3,
	D0_EFT2					= 4,
	D0_SD_SDIO_MMC_1		= 5,
	D0_EXT_REQ_DREQ0		= 6,
	D0_EXT_REQ_DREQ1		= 7,
	D0_EFT0					= 8,
	D0_IRDA_TX_RX			= 9,
	D0_MSP0_RX				= 10,
	D0_MSP0_TX				= 11,
	D0_SSP0_RX				= 12,
	D0_SSP0_TX				= 13,
	D0_UART0_RX				= 14,
	D0_UART0_TX				= 15,
	D0_EFT1					= 16,
	D0_USB_FS_CH0			= 17,
	D0_USB_FS_CH1			= 18,
	D0_USB_FS_CH2			= 19,
	D0_USB_FS_CH3			= 20,
	D0_USB_FS_CH4			= 21,
	D0_MSP2_RX				= 22,
	D0_MSP2_TX				= 23,
	D0_USB_FS_CH5			= 24,
	D0_RESERVED25			= 25,
	D0_SRC_DMA_DRF_REQ		= 26,
	D0_SRC_DMA_DRE_REQ		= 27,
	D0_MSP3_RX				= 28,
	D0_MSP3_TX				= 29,
	D0_MSP1_RX				= 30,
	D0_MSP1_TX				= 31,

	// DMA 1: Request Line DMA Source
	D1_RESERVED32			= 32,
	D1_RESERVED33			= 33,
	D1_RESERVED34			= 34,
	D1_RESERVED35			= 35,
	D1_I2C2_TX_RX			= 36,
	D1_SD_SDIO_MMC_2		= 37,
	D1_EXT_REQ_DREQ0		= 38,
	D1_EXT_REQ_DREQ1		= 39,
	D1_SPDIF     		  	= 40,
	D1_IRDA_TX_RX			= 41,
	D1_MSP0_RX				= 42,
	D1_MSP0_TX				= 43,
	D1_SSP1_RX				= 44,
	D1_SSP1_TX				= 45,
	D1_UART0_RX				= 46,
	D1_UART0_TX				= 47,
	D1_UART3_RX				= 48,
	D1_UART3_TX				= 49,
	D1_I2C1_TX_RX			= 50,
	D1_I2C0_TX_RX			= 51,
	D1_CHITF_RS				= 52,
	D1_SD_SDIO_MMC_0		= 53,
	D1_UART1_RX				= 54,
	D1_UART1_TX				= 55,
	D1_USB_OTG_CH_0			= 56,
	D1_USB_OTG_CH_1			= 57,
	D1_USB_OTG_CH_2			= 58,
	D1_USB_OTG_CH_3			= 59,
	D1_USB_OTG_CH_4			= 60,
	D1_USB_OTG_CH_5			= 61,
	D1_UART2_RX				= 62,
	D1_UART2_TX				= 63
#else  // ADR3a
	// DMA 0: Request Line DMA Source
	D0_DAB_DRQ0           = 0,
	D0_DAB_DRQ1           = 1,
	D0_DAB_DRQ2           = 2,
	D0_DAB_DRQ3           = 3,
	D0_DAB_DRQ4           = 4,
	D0_DAB_DRQ5           = 5,
	D0_DAB_DRQ6           = 6,
	D0_DAB_DRQ7           = 7,
	D0_DRM_DRQ0           = 8,
	D0_DRM_DRQ1           = 9,
	D0_UART0_TX          = 10,
	D0_UART0_RX          = 11,
	D0_UART1_TX          = 12,
	D0_UART1_RX          = 13,
	D0_UART2_TX          = 14,
	D0_UART2_RX          = 15,
	D0_SSP0_TX           = 16,
	D0_SSP0_RX           = 17,
	D0_SSP1_TX           = 18,
	D0_SSP1_RX           = 19,
	D0_SSP2_TX           = 20,
	D0_SSP2_RX           = 21,
	D0_SSP3_TX           = 22,
	D0_SSP3_RX           = 23,
	D0_SSP4_TX           = 24,
	D0_SSP4_RX           = 25,
	D0_DMA_I2C           = 26,
	D0_SPI0_TX           = 27,
	D0_SPI1_RX           = 28,
	D0_SPI2_TX           = 29,
	D0_SPI3_RX           = 30,
	D0_RESERVED1         = 31,

	// DMA 1: Request Line DMA Source
	D1_DAB_DRQ0            = 32,
	D1_DAB_DRQ1            = 33,
	D1_DAB_DRQ2            = 34,
	D1_DAB_DRQ3            = 35,
	D1_DAB_DRQ4            = 36,
	D1_DAB_DRQ5            = 37,
	D1_DAB_DRQ6            = 38,
	D1_DAB_DRQ7            = 39,
	D1_DRM_DRQ0            = 40,
	D1_DRM_DRQ1            = 41,
	D1_UART0_TX            = 42,
	D1_UART0_RX            = 43,
	D1UART1_TX             = 44,
	D1_UART1_RX            = 45,
	D1_UART2_TX            = 46,
	D1_UART2_RX            = 47,
	D1_FM_RD_FIFO1_REQ     = 48,
	D1_FM_WR_FIFO2_REQ     = 49,
	D1_FM_RD_FIFO3_REQ     = 50,
	D1_FM_WR_FIFO4_REQ     = 51,
	D1_RS_DAB              = 52,
	D1_RS_DMB              = 53,
	D1_RESERVED1           = 54,
	D1_RESERVED2           = 55,
	D1_SSP4_TX             = 56,
	D1_SSP4_RX             = 57,
	D1_DMA_I2C             = 58,
	D1_ESAI_FIFOTXINT      = 59,
	D1_ESAI_FIFORXINT      = 60,
	D1_ESAI_INTR_ORED      = 61,
	D1_RESERVED3          = 62,
	D1_RESERVED4          = 63
#endif
}
LLD_DMA_RequestLineTy;

// DMA channel
typedef enum
{
    LLD_DMA_CHANNEL_0 = 0,
    LLD_DMA_CHANNEL_1 = 1,
    LLD_DMA_CHANNEL_2 = 2,
    LLD_DMA_CHANNEL_3 = 3,
    LLD_DMA_CHANNEL_4 = 4,
    LLD_DMA_CHANNEL_5 = 5,
    LLD_DMA_CHANNEL_6 = 6,
    LLD_DMA_CHANNEL_7 = 7,
}
LLD_DMA_ChannelTy;

// DMA interrupt mask
typedef enum
{
    LLD_DMA_TERMINAL_COUNT_INTERRUPT_MASK = 0x8000,
    LLD_DMA_ERROR_INTERRUPT_MASK          = 0x4000
}
LLD_DMA_InterruptMaskTy;

// Software interrupt types
typedef enum
{
    LLD_DMA_SOFT_INT_SINGLE         = 0,
    LLD_DMA_SOFT_INT_LAST_SINGLE    = 1,
    LLD_DMA_SOFT_INT_BURST          = 2,
    LLD_DMA_SOFT_INT_LAST_BURST     = 3
}
LLD_DMA_SoftwareInterruptTy;


//----------------------------------------------------------------------
// macros
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// function prototypes
//----------------------------------------------------------------------
tVoid                                   LLD_DMA_SetLinkedList                       	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_LinkedListTy *lli, LLD_DMA_MasterTy masterNumber);
tU32	 								LLD_DMA_GetLinkedList							(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tBool                                   LLD_DMA_SetSynchronization                  	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool value);
tU8                                     LLD_DMA_GetInterruptStatus                  	(LLD_DMA_IdTy id);
tU8                                     LLD_DMA_GetTerminalCountInterruptStatus     	(LLD_DMA_IdTy id);
tU8                                     LLD_DMA_GetTerminalCountRawInterruptStatus  	(LLD_DMA_IdTy id);
tU8                                     LLD_DMA_GetErrorInterruptStatus             	(LLD_DMA_IdTy id);
tU8                                     LLD_DMA_GetErrorRawInterruptStatus          	(LLD_DMA_IdTy id);
tU8                                     LLD_DMA_GetEnabledChannel                   	(LLD_DMA_IdTy id);
tVoid                                   LLD_DMA_GenerateSoftwareInterrupt           	(LLD_DMA_IdTy id, LLD_DMA_RequestLineTy requestLine, LLD_DMA_SoftwareInterruptTy type);
tVoid                                   LLD_DMA_SetChannelSourceAddress             	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tU32 sourceAddress);
tU32                                    LLD_DMA_GetChannelSourceAddress             	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid                                   LLD_DMA_SetChannelDestinationAddress        	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tU32 sourceAddress);
tU32                                    LLD_DMA_GetChannelDestinationAddress        	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tBool                                   LLD_DMA_SetMasterEndianness                 	(LLD_DMA_EndianessTy master1endianness, LLD_DMA_EndianessTy master2endianness);
tBool                                   LLD_DMA_EnableTransfer                      	(tBool enabled);
tBool                                   LLD_DMA_EnableChannel                           (LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool flag);
tVoid                                   LLD_DMA_Reset                               	(LLD_DMA_IdTy id);
LLD_DMA_ChannelConfigurationRegisterTy  LLD_DMA_GetChannelConfiguration             	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
LLD_DMA_ChannelControlRegisterTy        LLD_DMA_GetChannelControl                   	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tU16                                    LLD_DMA_GetSynchronization                  	(tVoid);
tU32                                    LLD_DMA_GetConfiguration                    	(tVoid);
tVoid                                   LLD_DMA_ClearTerminalCountInterrupt         	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid                                   LLD_DMA_ClearErrorInterrupt                 	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid                                   LLD_DMA_EnableTerminalCountInterrupt        	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool value);
tVoid                                   LLD_DMA_SetInterruptMask                    	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_InterruptMaskTy interruptMask, tBool value);
tVoid                                   LLD_DMA_SetChannelControl                   	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_ChannelControlRegisterTy *channelControlReg);
tVoid                                   LLD_DMA_SetChannelConfiguration             	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_ChannelConfigurationRegisterTy *channelConfigurationReg);
tVoid                                   LLD_DMA_SetFlowControl                      	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_FlowControlTy flowControl);
tVoid                                   LLD_DMA_SetChannelDestinationPeripheral        	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_RequestLineTy requestLine);
tVoid                                   LLD_DMA_SetChannelSourcePeripheral             	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_RequestLineTy requestLine);
tVoid                                   LLD_DMA_SetChannelHalt                      	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool value);
tVoid                                   LLD_DMA_SetProtectionBits                   	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool mode, tBool bufferable, tBool cacheable);
tVoid                                   LLD_DMA_SetChannelLock			            	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool lockFlag);
tVoid 									LLD_DMA_SetChannelTransferSize					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tU16 transferSize);
tU16 									LLD_DMA_GetChannelTransferSize					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_IncrementChannelSourceAddress			(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_TransferWidthTy width);
tVoid 									LLD_DMA_DecrementChannelSourceAddress			(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_TransferWidthTy width);
tVoid 									LLD_DMA_IncrementChannelDestinationAddress		(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_TransferWidthTy width);
tVoid 									LLD_DMA_DecrementChannelDestinationAddress		(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_TransferWidthTy width);
tBool 									LLD_DMA_GetChannelActive						(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tBool 									LLD_DMA_GetChannelHalt							(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tBool 									LLD_DMA_GetChannelLock							(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tBool 									LLD_DMA_GetChannelTerminalCount					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelTerminalCount					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool flag);
tVoid 									LLD_DMA_SetChannelInterruptError				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool flag);
tBool 									LLD_DMA_GetChannelInterruptError				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
LLD_DMA_FlowControlTy 					LLD_DMA_GetChannelFlowController				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
LLD_DMA_RequestLineTy 					LLD_DMA_GetChannelDestinationPeripheral			(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
LLD_DMA_RequestLineTy 					LLD_DMA_GetChannelSourcePeripheral				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tBool 									LLD_DMA_GetChannelEnable						(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tBool 									LLD_DMA_GetChannelTerminalCountInterruptEnable	(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelPrivilegeMode					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool flag);
tBool									LLD_DMA_GetChannelPrivilegeMode					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelBufferable					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool flag);
tBool 									LLD_DMA_GetChannelBufferable					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelCacheable						(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool flag);
tBool 									LLD_DMA_GetChannelCacheable						(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelDestinationIncrement			(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool flag);
tBool 									LLD_DMA_GetChannelDestinationIncrement			(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelSourceIncrement				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, tBool flag);
tBool 									LLD_DMA_GetChannelSourceIncrement				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelSourceMaster					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_MasterTy master);
LLD_DMA_MasterTy 						LLD_DMA_GetChannelSourceMaster					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelDestinationMaster				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_MasterTy master);
LLD_DMA_MasterTy 						LLD_DMA_GetChannelDestinationMaster				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid	 								LLD_DMA_SetChannelSourceWidth					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_TransferWidthTy width);
LLD_DMA_TransferWidthTy 				LLD_DMA_GetChannelSourceWidth					(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid	 								LLD_DMA_SetChannelDestinationWidth				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_TransferWidthTy width);
LLD_DMA_TransferWidthTy 				LLD_DMA_GetChannelDestinationWidth				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelSourceBurstSize				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_BurstSizeTy burst_size);
LLD_DMA_BurstSizeTy 					LLD_DMA_GetChannelSourceBurstSize				(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);
tVoid 									LLD_DMA_SetChannelDestinationBurstSize			(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel, LLD_DMA_BurstSizeTy burst_size);
LLD_DMA_BurstSizeTy 					LLD_DMA_GetChannelDestinationBurstSize			(LLD_DMA_IdTy id, LLD_DMA_ChannelTy channel);

#if (LLD_DMA_STA660_ROM_USED == TRUE)
#define LLD_DMA_SetLinkedList(a,b,c,d)                       ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_LinkedListTy *,LLD_DMA_MasterTy))ROM_LLD_Table[ENU_LLD_DMA_SetLinkedList])(a,b,c,d)
#define LLD_DMA_GetLinkedList(a,b)                       	 ((tU32 (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetLinkedList])(a,b)
#define LLD_DMA_SetSynchronization(a,b,c)                    ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetSynchronization])(a,b,c)
#define LLD_DMA_GetInterruptStatus(a)                        ((tU8 (*)(LLD_DMA_IdTy))ROM_LLD_Table[ENU_LLD_DMA_GetInterruptStatus])(a)
#define LLD_DMA_GetTerminalCountInterruptStatus(a)           ((tU8 (*)(LLD_DMA_IdTy))ROM_LLD_Table[ENU_LLD_DMA_GetTerminalCountInterruptStatus])(a)
#define LLD_DMA_GetTerminalCountRawInterruptStatus(a)        ((tU8 (*)(LLD_DMA_IdTy))ROM_LLD_Table[ENU_LLD_DMA_GetTerminalCountRawInterruptStatus])(a)
#define LLD_DMA_GetErrorInterruptStatus(a)                   ((tU8 (*)(LLD_DMA_IdTy))ROM_LLD_Table[ENU_LLD_DMA_GetErrorInterruptStatus])(a)
#define LLD_DMA_GetErrorRawInterruptStatus(a)                ((tU8 (*)(LLD_DMA_IdTy))ROM_LLD_Table[ENU_LLD_DMA_GetErrorRawInterruptStatus])(a)
#define LLD_DMA_GetEnabledChannel(a)                         ((tU8 (*)(LLD_DMA_IdTy))ROM_LLD_Table[ENU_LLD_DMA_GetEnabledChannel])(a)
#define LLD_DMA_GenerateSoftwareInterrupt(a,b,c)             ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_RequestLineTy,LLD_DMA_SoftwareInterruptTy))ROM_LLD_Table[ENU_LLD_DMA_GenerateSoftwareInterrupt])(a,b,c)
#define LLD_DMA_SetChannelSourceAddress(a,b,c)               ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tU32))ROM_LLD_Table[ENU_LLD_DMA_SetChannelSourceAddress])(a,b,c)
#define LLD_DMA_GetChannelSourceAddress(a,b)                 ((tU32 (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelSourceAddress])(a,b)
#define LLD_DMA_SetChannelDestinationAddress(a,b,c)          ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tU32))ROM_LLD_Table[ENU_LLD_DMA_SetChannelDestinationAddress])(a,b,c)
#define LLD_DMA_GetChannelDestinationAddress(a,b)            ((tU32 (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelDestinationAddress])(a,b)
#define LLD_DMA_SetMasterEndianness(a,b)                     ((tBool (*)(LLD_DMA_EndianessTy,LLD_DMA_EndianessTy))ROM_LLD_Table[ENU_LLD_DMA_SetMasterEndianness])(a,b)
#define LLD_DMA_EnableTransfer(a)                            ((tBool (*)(tBool))ROM_LLD_Table[ENU_LLD_DMA_EnableTransfer])(a)
#define LLD_DMA_EnableChannel(a,b,c)                         ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_EnableChannel])(a,b,c)
#define LLD_DMA_Reset(a)                                     ((tVoid (*)(LLD_DMA_IdTy))ROM_LLD_Table[ENU_LLD_DMA_Reset])(a)
#define LLD_DMA_GetChannelConfiguration(a,b)                 ((LLD_DMA_ChannelConfigurationRegisterTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelConfiguration])(a,b)
#define LLD_DMA_GetChannelControl(a,b)                       ((LLD_DMA_ChannelControlRegisterTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelControl])(a,b)
#define LLD_DMA_GetSynchronization                           ((tU16 (*)(tVoid))ROM_LLD_Table[ENU_LLD_DMA_GetSynchronization])
#define LLD_DMA_GetConfiguration                             ((tU32 (*)(tVoid))ROM_LLD_Table[ENU_LLD_DMA_GetConfiguration])
#define LLD_DMA_ClearTerminalCountInterrupt(a,b)             ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_ClearTerminalCountInterrupt])(a,b)
#define LLD_DMA_ClearErrorInterrupt(a,b)                     ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_ClearErrorInterrupt])(a,b)
#define LLD_DMA_EnableTerminalCountInterrupt(a,b,c)          ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_EnableTerminalCountInterrupt])(a,b,c)
#define LLD_DMA_SetInterruptMask(a,b,c,d)                    ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_InterruptMaskTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetInterruptMask])(a,b,c,d)
#define LLD_DMA_SetChannelControl(a,b,c)                     ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_ChannelControlRegisterTy *))ROM_LLD_Table[ENU_LLD_DMA_SetChannelControl])(a,b,c)
#define LLD_DMA_SetChannelConfiguration(a,b,c)               ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_ChannelConfigurationRegisterTy *))ROM_LLD_Table[ENU_LLD_DMA_SetChannelConfiguration])(a,b,c)
#define LLD_DMA_SetFlowControl(a,b,c)                        ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_FlowControlTy))ROM_LLD_Table[ENU_LLD_DMA_SetFlowControl])(a,b,c)
#define LLD_DMA_SetChannelDestinationPeripheral(a,b,c)       ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_RequestLineTy))ROM_LLD_Table[ENU_LLD_DMA_SetChannelDestinationPeripheral])(a,b,c)
#define LLD_DMA_SetChannelSourcePeripheral(a,b,c)            ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_RequestLineTy))ROM_LLD_Table[ENU_LLD_DMA_SetChannelSourcePeripheral])(a,b,c)
#define LLD_DMA_SetChannelHalt(a,b,c)                        ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetChannelHalt])(a,b,c)
#define LLD_DMA_SetProtectionBits(a,b,c,d,e)                 ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool,tBool,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetProtectionBits])(a,b,c,d,e)
#define LLD_DMA_SetChannelLock(a,b,c)                        ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetChannelLock])(a,b,c)
#define LLD_DMA_SetChannelTransferSize(a,b,c)                ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tU16))ROM_LLD_Table[ENU_LLD_DMA_SetChannelTransferSize])(a,b,c)
#define LLD_DMA_GetChannelTransferSize(a,b)                  ((tU16 (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelTransferSize])(a,b)
#define LLD_DMA_IncrementChannelSourceAddress(a,b,c)         ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_TransferWidthTy))ROM_LLD_Table[ENU_LLD_DMA_IncrementChannelSourceAddress])(a,b,c)
#define LLD_DMA_DecrementChannelSourceAddress(a,b,c)         ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_TransferWidthTy))ROM_LLD_Table[ENU_LLD_DMA_DecrementChannelSourceAddress])(a,b,c)
#define LLD_DMA_IncrementChannelDestinationAddress(a,b,c)    ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_TransferWidthTy))ROM_LLD_Table[ENU_LLD_DMA_IncrementChannelDestinationAddress])(a,b,c)
#define LLD_DMA_DecrementChannelDestinationAddress(a,b,c)    ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_TransferWidthTy))ROM_LLD_Table[ENU_LLD_DMA_DecrementChannelDestinationAddress])(a,b,c)
#define LLD_DMA_GetChannelActive(a,b)                        ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelActive])(a,b)
#define LLD_DMA_GetChannelHalt(a,b)                          ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelHalt])(a,b)
#define LLD_DMA_GetChannelLock(a,b)                          ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelLock])(a,b)
#define LLD_DMA_GetChannelTerminalCount(a,b)                 ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelTerminalCount])(a,b)
#define LLD_DMA_SetChannelTerminalCount(a,b,c)               ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetChannelTerminalCount])(a,b,c)
#define LLD_DMA_SetChannelInterruptError(a,b,c)              ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetChannelInterruptError])(a,b,c)
#define LLD_DMA_GetChannelInterruptError(a,b)                ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelInterruptError])(a,b)
#define LLD_DMA_GetChannelFlowController(a,b)                ((LLD_DMA_FlowControlTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelFlowController])(a,b)
#define LLD_DMA_GetChannelDestinationPeripheral(a,b)         ((LLD_DMA_RequestLineTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelDestinationPeripheral])(a,b)
#define LLD_DMA_GetChannelSourcePeripheral(a,b)              ((LLD_DMA_RequestLineTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelSourcePeripheral])(a,b)
#define LLD_DMA_GetChannelEnable(a,b)                        ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelEnable])(a,b)
#define LLD_DMA_GetChannelTerminalCountInterruptEnable(a,b)  ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelTerminalCountInterruptEnable])(a,b)
#define LLD_DMA_SetChannelPrivilegeMode(a,b,c)               ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetChannelPrivilegeMode])(a,b,c)
#define LLD_DMA_GetChannelPrivilegeMode(a,b)                 ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelPrivilegeMode])(a,b)
#define LLD_DMA_SetChannelBufferable(a,b,c)                  ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetChannelBufferable])(a,b,c)
#define LLD_DMA_GetChannelBufferable(a,b)                    ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelBufferable])(a,b)
#define LLD_DMA_SetChannelCacheable(a,b,c)                   ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetChannelCacheable])(a,b,c)
#define LLD_DMA_GetChannelCacheable(a,b)                     ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelCacheable])(a,b)
#define LLD_DMA_SetChannelDestinationIncrement(a,b,c)        ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetChannelDestinationIncrement])(a,b,c)
#define LLD_DMA_GetChannelDestinationIncrement(a,b)          ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelDestinationIncrement])(a,b)
#define LLD_DMA_SetChannelSourceIncrement(a,b,c)             ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,tBool))ROM_LLD_Table[ENU_LLD_DMA_SetChannelSourceIncrement])(a,b,c)
#define LLD_DMA_GetChannelSourceIncrement(a,b)               ((tBool (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelSourceIncrement])(a,b)
#define LLD_DMA_SetChannelSourceMaster(a,b,c)                ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_MasterTy))ROM_LLD_Table[ENU_LLD_DMA_SetChannelSourceMaster])(a,b,c)
#define LLD_DMA_GetChannelSourceMaster(a,b)                  ((LLD_DMA_MasterTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelSourceMaster])(a,b)
#define LLD_DMA_SetChannelDestinationMaster(a,b,c)           ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_MasterTy))ROM_LLD_Table[ENU_LLD_DMA_SetChannelDestinationMaster])(a,b,c)
#define LLD_DMA_GetChannelDestinationMaster(a,b)             ((LLD_DMA_MasterTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelDestinationMaster])(a,b)
#define LLD_DMA_SetChannelSourceWidth(a,b,c)                 ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_TransferWidthTy))ROM_LLD_Table[ENU_LLD_DMA_SetChannelSourceWidth])(a,b,c)
#define LLD_DMA_GetChannelSourceWidth(a,b)                   ((LLD_DMA_TransferWidthTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelSourceWidth])(a,b)
#define LLD_DMA_SetChannelDestinationWidth(a,b,c)            ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_TransferWidthTy))ROM_LLD_Table[ENU_LLD_DMA_SetChannelDestinationWidth])(a,b,c)
#define LLD_DMA_GetChannelDestinationWidth(a,b)              ((LLD_DMA_TransferWidthTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelDestinationWidth])(a,b)
#define LLD_DMA_SetChannelSourceBurstSize(a,b,c)             ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_BurstSizeTy))ROM_LLD_Table[ENU_LLD_DMA_SetChannelSourceBurstSize])(a,b,c)
#define LLD_DMA_GetChannelSourceBurstSize(a,b)               ((LLD_DMA_BurstSizeTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelSourceBurstSize])(a,b)
#define LLD_DMA_SetChannelDestinationBurstSize(a,b,c)        ((tVoid (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy,LLD_DMA_BurstSizeTy))ROM_LLD_Table[ENU_LLD_DMA_SetChannelDestinationBurstSize])(a,b,c)
#define LLD_DMA_GetChannelDestinationBurstSize(a,b)          ((LLD_DMA_BurstSizeTy (*)(LLD_DMA_IdTy,LLD_DMA_ChannelTy))ROM_LLD_Table[ENU_LLD_DMA_GetChannelDestinationBurstSize])(a,b)
#endif

#endif	// _LLD_DMA_H_

// End of file

