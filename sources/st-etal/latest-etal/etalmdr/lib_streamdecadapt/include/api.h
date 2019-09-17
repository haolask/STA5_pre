// _____________________________________________________________________________
//| FILE:         api.h
//| PROJECT:      ADR3
//|_____________________________________________________________________________
//| DESCRIPTION:  Functions wrapper to access ROM LLDs
//|_____________________________________________________________________________
//| COPYRIGHT:    (c) 2009 STMicroelectronics, Naples - ITALY
//|
//| HISTORY:
//| Date      | Modification               | Author
//|_____________________________________________________________________________
//| 27.10.09  | Initial rev. (Ver. 1.0)    | Luigi Cotignano
//|_____________________________________________________________________________

#ifndef _API_H_
#define _API_H_

//----------------------------------------------------------------------
//                      ROM API enable flags
//----------------------------------------------------------------------
// To ENABLE usage of a specified LLD contained into Library ROM you
// have only to change the corresponding definition to TRUE, to FALSE
// otherwise
//----------------------------------------------------------------------
#define LLD_DMA_STA660_ROM_USED	    FALSE
#define LLD_GPIO_STA660_ROM_USED	FALSE
#define LLD_I2C_STA660_ROM_USED  	FALSE
#define LLD_MTU_STA660_ROM_USED	    FALSE
#define LLD_SPI_STA660_ROM_USED	    FALSE
#define LLD_SSP_STA660_ROM_USED	    FALSE
#define LLD_UART_STA660_ROM_USED	FALSE
#define LLD_VIC_STA660_ROM_USED	    FALSE
#define TABLES_ROM_USED             FALSE


//**********************************************************************
//*                                                                    *
//*               TAKE CARE TO NOT CHANGE THE CODE BELOW               *
//*                                                                    *
//**********************************************************************
typedef tVoid (*Function_Pointer)(tVoid);
#define ROM_LLD_Table	((Function_Pointer)0x98004000)


#if (TABLES_ROM_USED == TRUE)
#define rom_table_firecode_ptr                  ((const tU16*)  0x980081ac)
#define rom_table__crc_ccitt_table_ptr          ((const tU16*)  0x980083ac)
#define rom_table__crc32_table_ptr              ((const tU32*)  0x980085ac)
#define rom_table__rs_gexp_ptr                  ((const tU8*)   0x980089ac)
#define rom_table__rs_glog_ptr                  ((const tU8*)   0x98008aac)
#define rom_table__rs_ielem_ptr                 ((const tU8*)   0x98008bac)
#define rom_table__rs_mult_ptr                  ((const tU8*)   0x98008cac)
#define rom_table__bitswap_ptr                  ((const tU8*)   0x98009cac)
#define rom_table__onescnt_ptr                  ((const tU8*)   0x98009dac)
#define rom_table__yellowbook_scrambler_ptr     ((const tU8*)   0x98009eac)
#define rom_table__cdrom_crc32tab_ptr           ((const tU32*)  0x9800a7d0)
#define w32r_ptr                                ((const tS16*)  0x9800abd0)
#define w64r_ptr                                ((const tS16*)  0x9800abe2)
#define w128r_ptr                               ((const tS16*)  0x9800ac04)
#define w256r_ptr                               ((const tS16*)  0x9800ac46)
#define w512r_ptr                               ((const tS16*)  0x9800acc8)
#define w1152r_ptr                              ((const tS16*)  0x9800adca)
#define w32i_ptr                                ((const tS16*)  0x9800b00c)
#define w64i_ptr                                ((const tS16*)  0x9800b01e)
#define w128i_ptr                               ((const tS16*)  0x9800b040)
#define w256i_ptr                               ((const tS16*)  0x9800b082)
#define w512i_ptr                               ((const tS16*)  0x9800b104)
#define w1152i_ptr                              ((const tS16*)  0x9800b206)
#define FreqRefPositions_ptr                    ((const tS16*)  0x9800b448)
#define FreqRefPhaseIndices_ptr                 ((const tU16*)  0x9800b460)
#define TimeRefPositions_ptr                    ((const tS16*)  0x9800b478)
#define TimeRefPhaseIndices_ptr                 ((const tU16*)  0x9800b530)
#define Q_1024_ModeA_ptr                        ((const tU16*)  0x9800b5d8)
#define Q_1024_ModeB_ptr                        ((const tU16*)  0x9800b5da)
#define Q_1024_ModeC_ptr                        ((const tU16*)  0x9800b5dc)
#define Q_1024_ModeD_ptr                        ((const tU16*)  0x9800b5de)
#define W_1024_ModeA_ptr                        ((const tU16*)  0x9800b5e0)
#define Z_256_ModeA_ptr                         ((const tUChar*)0x9800b5fe)
#define W_1024_ModeB_ptr                        ((const tU16*)  0x9800b60e)
#define Z_256_ModeB_ptr                         ((const tUChar*)0x9800b62c)
#define W_1024_ModeC_ptr                        ((const tU16*)  0x9800b63c)
#define Z_256_ModeC_ptr                         ((const tUChar*)0x9800b664)
#define W_1024_ModeD_ptr                        ((const tU16*)  0x9800b678)
#define Z_256_ModeD_ptr                         ((const tUChar*)0x9800b6a8)
#define posFacCellsA_ptr                        ((const tS16*)  0x9800b6c0)
#define posFacCellsB_ptr                        ((const tS16*)  0x9800b7b0)
#define posFacCellsC_ptr                        ((const tS16*)  0x9800b864)
#define posFacCellsD_ptr                        ((const tS16*)  0x9800b92c)
#define pXORLengths_ptr                         ((const tS16*)  0x9800b9ec)  
#define pXORIndexes_ptr                         ((const tS16*)  0x9800b9f4)
#define PuncMasks_ptr                           ((const tS16*)  0x9800ba1c)  
#define PuncShifts_ptr                          ((const tS16*)  0x9800ba24)
#define gpbPuncTableRep_ptr                     ((const tU16*)  0x9800ba2c)  
#define gpbTailPuncTableRep_ptr                 ((const tU16*)  0x9800be00)  
#define gpbPuncTable_Length_ptr                 ((const tU16*)  0x9800c070)  
#define TailbitsMasks_ptr                       ((const tS16*)  0x9800c08c)  
#define MaskOrder_ptr                           ((const tS16*)  0x9800c11c)
#define InputIncrement_ptr                      ((const tS16*)  0x9800c1fc)

//----------------------------------------------------------------------
//                             ROM TABLES (11)
//----------------------------------------------------------------------
#define rom_table_firecode(x)                   (*(rom_table_firecode_ptr+x))
#define rom_table__crc_ccitt_table(x)     		(*(rom_table__crc_ccitt_table_ptr+x))
#define rom_table__crc32_table(x)         		(*(rom_table__crc32_table_ptr+x))
#define rom_table__rs_gexp(x)             		(*(rom_table__rs_gexp_ptr+x))
#define rom_table__rs_glog(x)             		(*(rom_table__rs_glog_ptr+x))
#define rom_table__rs_ielem(x)            		(*(rom_table__rs_ielem_ptr+x))
#define rom_table__rs_mult(x)             		(*(rom_table__rs_mult_ptr+x))
#define rom_table__bitswap(x)             		(*(rom_table__bitswap_ptr+x))
#define rom_table__onescnt(x)             		(*(rom_table__onescnt_ptr+x))
#define rom_table__yellowbook_scrambler(x)		(*(rom_table__yellowbook_scrambler_ptr+x))
#define rom_table__cdrom_crc32tab(x)      		(*(rom_table__cdrom_crc32tab_ptr+x))

//----------------------------------------------------------------------
//                             xP70 TABLES (42)
//----------------------------------------------------------------------
#define w32r(x)                           		(*(w32r_ptr+x))
#define w64r(x)                           		(*(w64r_ptr+x))
#define w128r(x)                          		(*(w128r_ptr+x))
#define w256r(x)                          		(*(w256r_ptr+x))
#define w512r(x)                          		(*(w512r_ptr+x))
#define w1152r(x)                         		(*(w1152r_ptr+x))
#define w32i(x)                           		(*(w32i_ptr+x))
#define w64i(x)                           		(*(w64i_ptr+x))
#define w128i(x)                          		(*(w128i_ptr+x))
#define w256i(x)                          		(*(w256i_ptr+x))
#define w512i(x)                          		(*(w512i_ptr+x))
#define w1152i(x)                         		(*(w1152i_ptr+x))
#define FreqRefPositions(x,y)              		((FreqRefPositions_ptr+(x*3))[y])
#define FreqRefPhaseIndices(x,y)           		((FreqRefPhaseIndices_ptr+(x*3))[y])
#define TimeRefPositions(x,y)              		((TimeRefPositions_ptr+(x*23))[y])
#define TimeRefPhaseIndices(x,y)           		((TimeRefPhaseIndices_ptr+(x*21))[y])
#define Q_1024_ModeA                   		    (*(Q_1024_ModeA_ptr))
#define Q_1024_ModeB                   		    (*(Q_1024_ModeB_ptr))
#define Q_1024_ModeC                   		    (*(Q_1024_ModeC_ptr))
#define Q_1024_ModeD                   		    (*(Q_1024_ModeD_ptr))
#define W_1024_ModeA(x,y)                  		((W_1024_ModeA_ptr+(x*3))[y])
#define Z_256_ModeA(x,y)                   		((Z_256_ModeA_ptr+(x*3))[y])
#define W_1024_ModeB(x,y)                  		((W_1024_ModeB_ptr+(x*5))[y])
#define Z_256_ModeB(x,y)                   		((Z_256_ModeB_ptr+(x*5))[y])
#define W_1024_ModeC(x,y)                  		((W_1024_ModeC_ptr+(x*10))[y])
#define Z_256_ModeC(x,y)                   		((Z_256_ModeC_ptr+(x*10))[y])
#define W_1024_ModeD(x,y)                  		((W_1024_ModeD_ptr+(x*8))[y])
#define Z_256_ModeD(x,y)                   		((Z_256_ModeD_ptr+(x*8))[y])
#define posFacCellsA(x,y)                  		((posFacCellsA_ptr+(x*8))[y])
#define posFacCellsB(x,y)                  		((posFacCellsB_ptr+(x*6))[y])
#define posFacCellsC(x,y)                  		((posFacCellsC_ptr+(x*5))[y])
#define posFacCellsD(x,y)                  		((posFacCellsD_ptr+(x*4))[y])
#define pXORLengths(x)                    		(*(pXORLengths_ptr+x))
#define pXORIndexes(x,y)                   		((pXORIndexes_ptr+(x*5))[y])
#define PuncMasks(x)                      		(*(PuncMasks_ptr+x))
#define PuncShifts(x)                     		(*(PuncShifts_ptr+x))
#define gpbPuncTableRep(x,y)              		((gpbPuncTableRep_ptr+(x*35))[y])
#define gpbTailPuncTableRep(x,y)           		((gpbTailPuncTableRep_ptr+(x*26))[y])
#define gpbPuncTable_Length(x)            		(*(gpbPuncTable_Length_ptr+x))
#define TailbitsMasks(x,y)                 		((TailbitsMasks_ptr+(x*6))[y])
#define MaskOrder(x,y)                     		((MaskOrder_ptr+(x*8))[y])
#define InputIncrement(x,y)                		((InputIncrement_ptr+(x*8))[y])
#endif // TABLES_ROM_USED

enum ENU_ROM_LLD_Table
{
	// DMA ROM functions (73)
    ENU_LLD_DMA_SetLinkedList,
    ENU_LLD_DMA_SetSynchronization,
    ENU_LLD_DMA_GetInterruptStatus,
    ENU_LLD_DMA_GetTerminalCountInterruptStatus,
    ENU_LLD_DMA_GetTerminalCountRawInterruptStatus,
    ENU_LLD_DMA_GetErrorInterruptStatus,
    ENU_LLD_DMA_GetErrorRawInterruptStatus,
    ENU_LLD_DMA_GetEnabledChannel,
    ENU_LLD_DMA_GenerateSoftwareInterrupt,
    ENU_LLD_DMA_SetChannelSourceAddress,
    ENU_LLD_DMA_GetChannelSourceAddress,
    ENU_LLD_DMA_SetChannelDestinationAddress,
    ENU_LLD_DMA_GetChannelDestinationAddress,
    ENU_LLD_DMA_SetMasterEndianness,
    ENU_LLD_DMA_EnableTransfer,
    ENU_LLD_DMA_EnableChannel,
    ENU_LLD_DMA_Reset,
    ENU_LLD_DMA_GetChannelConfiguration,
    ENU_LLD_DMA_GetChannelControl,
    ENU_LLD_DMA_GetSynchronization,
    ENU_LLD_DMA_GetConfiguration,
    ENU_LLD_DMA_ClearTerminalCountInterrupt,
    ENU_LLD_DMA_ClearErrorInterrupt,
    ENU_LLD_DMA_EnableTerminalCountInterrupt,
    ENU_LLD_DMA_SetInterruptMask,
    ENU_LLD_DMA_SetChannelControl,
    ENU_LLD_DMA_SetChannelConfiguration,
    ENU_LLD_DMA_SetFlowControl,
    ENU_LLD_DMA_SetChannelDestinationPeripheral,
    ENU_LLD_DMA_SetChannelSourcePeripheral,
    ENU_LLD_DMA_SetChannelHalt,
    ENU_LLD_DMA_SetProtectionBits,
    ENU_LLD_DMA_SetChannelLock,
    ENU_LLD_DMA_SetChannelTransferSize,
    ENU_LLD_DMA_GetChannelTransferSize,
    ENU_LLD_DMA_IncrementChannelSourceAddress,
    ENU_LLD_DMA_DecrementChannelSourceAddress,
    ENU_LLD_DMA_IncrementChannelDestinationAddress,
    ENU_LLD_DMA_DecrementChannelDestinationAddress,
    ENU_LLD_DMA_GetChannelActive,
    ENU_LLD_DMA_GetChannelHalt,
    ENU_LLD_DMA_GetChannelLock,
    ENU_LLD_DMA_GetChannelTerminalCount,
    ENU_LLD_DMA_SetChannelTerminalCount,
    ENU_LLD_DMA_SetChannelInterruptError,
    ENU_LLD_DMA_GetChannelInterruptError,
    ENU_LLD_DMA_GetChannelFlowController,
    ENU_LLD_DMA_GetChannelDestinationPeripheral,
    ENU_LLD_DMA_GetChannelSourcePeripheral,
    ENU_LLD_DMA_GetChannelEnable,
    ENU_LLD_DMA_GetChannelTerminalCountInterruptEnable,
    ENU_LLD_DMA_SetChannelPrivilegeMode,
    ENU_LLD_DMA_GetChannelPrivilegeMode,
    ENU_LLD_DMA_SetChannelBufferable,
    ENU_LLD_DMA_GetChannelBufferable,
    ENU_LLD_DMA_SetChannelCacheable,
    ENU_LLD_DMA_GetChannelCacheable,
    ENU_LLD_DMA_SetChannelDestinationIncrement,
    ENU_LLD_DMA_GetChannelDestinationIncrement,
    ENU_LLD_DMA_SetChannelSourceIncrement,
    ENU_LLD_DMA_GetChannelSourceIncrement,
    ENU_LLD_DMA_SetChannelSourceMaster,
    ENU_LLD_DMA_GetChannelSourceMaster,
    ENU_LLD_DMA_SetChannelDestinationMaster,
    ENU_LLD_DMA_GetChannelDestinationMaster,
    ENU_LLD_DMA_SetChannelSourceWidth,
    ENU_LLD_DMA_GetChannelSourceWidth,
    ENU_LLD_DMA_SetChannelDestinationWidth,
    ENU_LLD_DMA_GetChannelDestinationWidth,
    ENU_LLD_DMA_SetChannelSourceBurstSize,
    ENU_LLD_DMA_GetChannelSourceBurstSize,
    ENU_LLD_DMA_SetChannelDestinationBurstSize,
    ENU_LLD_DMA_GetChannelDestinationBurstSize,
    
    // GPIO ROM functions (30)
    ENU_LLD_GPIO_SetDirection,
    ENU_LLD_GPIO_SetDirectionInput,
    ENU_LLD_GPIO_SetDirectionOutput,
    ENU_LLD_GPIO_GetDirection,
    ENU_LLD_GPIO_SetState,
    ENU_LLD_GPIO_GetPinState,
    ENU_LLD_GPIO_GetPortState,
    ENU_LLD_GPIO_GetInterruptStatus,
    ENU_LLD_GPIO_SetStateHigh,
    ENU_LLD_GPIO_SetStateLow,
    ENU_LLD_GPIO_SetInterruptType,
    ENU_LLD_GPIO_GetInterruptType,
    ENU_LLD_GPIO_InterruptEnable,
    ENU_LLD_GPIO_InterruptDisable,
    ENU_LLD_GPIO_ClearInterrupt,
    ENU_LLD_GPIO_SetControlMode,
    ENU_LLD_GPIO_GetControlMode,
    ENU_LLD_GPIO_ReadPin,
    ENU_LLD_GPIO_SetPinHigh,
    ENU_LLD_GPIO_SetPinLow,
    ENU_LLD_GPIO_SetPinDirection,
    ENU_LLD_GPIO_GetPinDirection,
    ENU_LLD_GPIO_PinIRQEnable,
    ENU_LLD_GPIO_PinIRQDisable,
    ENU_LLD_GPIO_PinIRQGetConfig,
    ENU_LLD_GPIO_PinIRQClear,
    ENU_LLD_GPIO_SetPinMode,
    ENU_LLD_GPIO_GetPinMode,
    ENU_LLD_GPIO_EnAltFunction,
    ENU_LLD_GPIO_DisAltFunction,     
    
    // I2C ROM functions (21)
    ENU_LLD_I2C_SetBusFrequency,
    ENU_LLD_I2C_InitMaster,
    ENU_LLD_I2C_InitSlave,
    ENU_LLD_I2C_InterruptEnable,
    ENU_LLD_I2C_InterruptDisable,
    ENU_LLD_I2C_Reset,
    ENU_LLD_I2C_Read,
    ENU_LLD_I2C_ReadRepeatedStart,
    ENU_LLD_I2C_Write,
    ENU_LLD_I2C_ReturnStatus,
    ENU_LLD_I2C_GetCurrentStatus,
    ENU_LLD_I2C_WritePolling,
    ENU_LLD_I2C_ReadPolling,
    ENU_LLD_I2C_ReadRepeatedStartPolling,
    ENU_LLD_I2C_ReturnStatusRegister1,
    ENU_LLD_I2C_ReturnStatusRegister2,
    ENU_LLD_I2C_SetTimeoutValue,
    ENU_LLD_I2C_GetTimeoutValue,
    ENU_LLD_I2C_INT_MasterManager,
    ENU_LLD_I2C_INT_SlaveManager,
    ENU_LLD_I2C_CalculateFreqBits, 

    // MTU ROM functions (11)
    ENU_LLD_MTU_Enable,
    ENU_LLD_MTU_Disable,
    ENU_LLD_MTU_Config,
    ENU_LLD_MTU_SetLoadRegister,
    ENU_LLD_MTU_SetBackgroundLoadRegister,
    ENU_LLD_MTU_GetValueRegister,
    ENU_LLD_MTU_SetInterruptMask,
    ENU_LLD_MTU_ClearInterruptMask,
    ENU_LLD_MTU_ClearInterrupt,
    ENU_LLD_MTU_GetRawInterruptStatus,
    ENU_LLD_MTU_GetMaskedInterruptStatus,

    // SPI ROM functions (43)
    ENU_LLD_SPI_Init,
    ENU_LLD_SPI_FlushReceiveFifo,
    ENU_LLD_SPI_SetBaudRate,
    ENU_LLD_SPI_SetDataFrames,
    ENU_LLD_SPI_GetDataFrames,
    ENU_LLD_SPI_SetConfiguration,
    ENU_LLD_SPI_GetRawInterruptStatus,
    ENU_LLD_SPI_GetMaskedInterruptStatus,
    ENU_LLD_SPI_ReadData,
    ENU_LLD_SPI_ReadRawData,
    ENU_LLD_SPI_WriteData,
    ENU_LLD_SPI_WriteRawData,
    ENU_LLD_SPI_Enable,
    ENU_LLD_SPI_Disable,
    ENU_LLD_SPI_DataRegAddress,
    ENU_LLD_SPI_DMAModeEnable,
    ENU_LLD_SPI_DMAModeDisable,
    ENU_LLD_SPI_RxFifoFlush,
    ENU_LLD_SPI_TxFifoFlush,
    ENU_LLD_SPI_IsBusy,
    ENU_LLD_SPI_IsRxFifoFull,
    ENU_LLD_SPI_IsRxFifoOverflow,
    ENU_LLD_SPI_IsRxFifoUnderflow,
    ENU_LLD_SPI_IsTxFifoOverflow,
    ENU_LLD_SPI_IsTxFifoEmpty,
    ENU_LLD_SPI_GetStatusRegister,
    ENU_LLD_SPI_InterruptEnable,
    ENU_LLD_SPI_InterruptDisable,
    ENU_LLD_SPI_ClearTransmitOverflowInt,
    ENU_LLD_SPI_ClearReceiveOverflowInt,
    ENU_LLD_SPI_ClearReceiveUnderflowInt,
    ENU_LLD_SPI_ClearMasterContentionInt,
    ENU_LLD_SPI_ClearGlobalInt,
    ENU_LLD_SPI_SetTransmitFifoThresholdLevel,
    ENU_LLD_SPI_GetTransmitFifoThresholdLevel,
    ENU_LLD_SPI_GetCurrentTransmitFifoEntries,
    ENU_LLD_SPI_SetDMATransmitDataLevel,
    ENU_LLD_SPI_SetReceiveFifoThresholdLevel,
    ENU_LLD_SPI_GetReceiveFifoThresholdLevel,
    ENU_LLD_SPI_GetCurrentReceiveFifoEntries,
    ENU_LLD_SPI_SetDMAReceiveDataLevel,
    ENU_LLD_SPI_GetIdentificationNumber,
    ENU_LLD_SPI_GetRevisionNumber,

    // SSP ROM functions (25)
    ENU_LLD_SSP_Init,
    ENU_LLD_SSP_Enable,
    ENU_LLD_SSP_EnableLoopBackMode,
    ENU_LLD_SSP_Reset,
    ENU_LLD_SSP_ResolveClockFrequency,
    ENU_LLD_SSP_SetConfiguration,
    ENU_LLD_SSP_SetData,
    ENU_LLD_SSP_GetData,
    ENU_LLD_SSP_GetFIFOStatus,
    ENU_LLD_SSP_FIFOFlush,
    ENU_LLD_SSP_GetIRQSrc,
    ENU_LLD_SSP_GetAllData,
    ENU_LLD_SSP_SetAllData,
    ENU_LLD_SSP_DataRegAddress,
    ENU_LLD_SSP_EnableIRQSrc,
    ENU_LLD_SSP_DisableIRQSrc,
    ENU_LLD_SSP_ClearIRQSrc,
    ENU_LLD_SSP_InitInterface,
    ENU_LLD_SSP_IsPendingIRQSrc,
    ENU_LLD_SSP_IsMaster,
    ENU_LLD_SSP_IsBusy,
    ENU_LLD_SSP_IsRxFifoFull,
    ENU_LLD_SSP_IsRxFifoNotEmpty,
    ENU_LLD_SSP_IsTxFifoNotFull,
    ENU_LLD_SSP_IsTxFifoEmpty, 

    // UART ROM functions (71)
    ENU_LLD_UART_ReadRxFifo,
    ENU_LLD_UART_WriteTxFifo,
    ENU_LLD_UART_DataRegAddress,
    ENU_LLD_UART_GetRxBytes,
    ENU_LLD_UART_GetTxBytes,
    ENU_LLD_UART_GetInterruptStatus,
    ENU_LLD_UART_GetRawInterruptStatus,
    ENU_LLD_UART_GetFlagRegister,
    ENU_LLD_UART_ClearInterrupt,
    ENU_LLD_UART_IsInterruptRaised,
    ENU_LLD_UART_ReadWait,
    ENU_LLD_UART_RxBufferEmpty,
    ENU_LLD_UART_TxBufferEmpty,
    ENU_LLD_UART_IsTxFifoEmpty,
    ENU_LLD_UART_IsTxFifoFull,
    ENU_LLD_UART_IsRxFifoEmpty,
    ENU_LLD_UART_IsRxFifoFull,
    ENU_LLD_UART_IsBusy,
    ENU_LLD_UART_Enable,
    ENU_LLD_UART_Disable,
    ENU_LLD_UART_TxReset,
    ENU_LLD_UART_RxReset,
    ENU_LLD_UART_FifoEnable,
    ENU_LLD_UART_FifoDisable,
    ENU_LLD_UART_RxEnable,
    ENU_LLD_UART_RxDisable,
    ENU_LLD_UART_TxEnable,
    ENU_LLD_UART_TxDisable,
    ENU_LLD_UART_SetBaudRate,
    ENU_LLD_UART_SetParity,
    ENU_LLD_UART_SetStopBits,
    ENU_LLD_UART_SetDataLen,
    ENU_LLD_UART_SetRxFifoTriggerLevel,
    ENU_LLD_UART_SetTxFifoTriggerLevel,
    ENU_LLD_UART_SetRxBuffer,
    ENU_LLD_UART_WriteWait,
    ENU_LLD_UART_Config,
    ENU_LLD_UART_Init,
    ENU_LLD_UART_InterruptEnable,
    ENU_LLD_UART_InterruptDisable,
    ENU_LLD_UART_DMAEnable,
    ENU_LLD_UART_DMADisable,
    ENU_LLD_UART_IRDAEnable,
    ENU_LLD_UART_IRDADisable,
    ENU_LLD_UART_GetRxFifoTriggerLevel,
    ENU_LLD_UART_GetTxFifoTriggerLevel,
    ENU_LLD_UART_INT_RxEnter,
    ENU_LLD_UART_INT_TxEnter,
    ENU_LLD_UART_INT_TimeoutEnter,
    ENU_LLD_UART_RxByteCounterReset,
    ENU_LLD_UART_TxByteCounterReset,
    ENU_LLD_UART_GetOverflowStatus,
    ENU_LLD_UART_ResetOverflowStatus,
    ENU_LLD_UART_Read,
    ENU_LLD_UART_ReadByte,
    ENU_LLD_UART_GetBufferFreeSpace,
    ENU_LLD_UART_GetBufferFullSpace,
    ENU_LLD_UART_INT_BasicRxManager,
    ENU_LLD_UART_INT_RxManager,
    ENU_LLD_UART_INT_TxManager,
    ENU_LLD_UART_INT_TimeOutManager,
    ENU_LLD_UART_EnableHwFlowControl,
    ENU_LLD_UART_DisableHwFlowControl,
    ENU_LLD_UART_DisableLoopBack,
    ENU_LLD_UART_EnableLoopBack,
    ENU_LLD_UART_Clear_ErrorStatusReg,
    ENU_LLD_UART_ResetReg,
    ENU_LLD_UART_u32ReadFIFO,
    ENU_LLD_UART_u32Read,
    ENU_LLD_UART_WriteFIFO,
    ENU_LLD_UART_Write,

    // VIC ROM functions (30)
    ENU_LLD_VIC_DefaultISR,
    ENU_LLD_VIC_ClearAllPriorities,
    ENU_LLD_VIC_VectoredChannelConfig,
    ENU_LLD_VIC_NonVectoredInterruptManager,
    ENU_LLD_VIC_NonVectoredTableInit,
    ENU_LLD_VIC_NonVectoredChannelConfig,
    ENU_LLD_VIC_SetProtectionStatus,
    ENU_LLD_VIC_GetProtectionStatus,
    ENU_LLD_VIC_SetCurrentInterruptAddress,
    ENU_LLD_VIC_GetCurrentInterruptAddress,
    ENU_LLD_VIC_SetDefaultInterruptAddress,
    ENU_LLD_VIC_GetDefaultInterruptAddress,
    ENU_LLD_VIC_SetVectoredInterruptAddress,
    ENU_LLD_VIC_GetVectoredInterruptAddress,
    ENU_LLD_VIC_SetVectoredInterruptControlRegister,
    ENU_LLD_VIC_GetVectoredInterruptControlRegister,
    ENU_LLD_VIC_SetInterruptType,
    ENU_LLD_VIC_GetInterruptType,
    ENU_LLD_VIC_EnableChannel,
    ENU_LLD_VIC_DisableChannel,
    ENU_LLD_VIC_DisableAllChannels,
    ENU_LLD_VIC_EnableAllChannels,
    ENU_LLD_VIC_BackupAndDisableAllChannels,
    ENU_LLD_VIC_RestoreAllChannelsStatus,
    ENU_LLD_VIC_GenerateSoftwareInterrupt,
    ENU_LLD_VIC_ClearSoftwareInterrupt,
    ENU_LLD_VIC_GetRawInterruptStatus,
    ENU_LLD_VIC_GetIRQInterruptStatus,
    ENU_LLD_VIC_GetFIQInterruptStatus,
    ENU_LLD_VIC_SetDefaultISR, 
};

#endif // _API_H_
