/*--------------------------------------------------------------------------------------------
* STMicroelectronics
* Reproduction and Communication of this document is strictly prohibited
* unless specifically authorized in writing by STMicroelectronics
*---------------------------------------------------------------------------------------------
* Public Header file of MSP Controller  module
* Specification release related to this implementation:
*---------------------------------------------------------------------------------------------
*//******************************************************************************/



#ifndef LLD_MSP_H_
#define LLD_MSP_H_


#ifdef __cplusplus
extern "C"
{	/* In case C++ needs to use this header.*/
#endif

#ifndef  PLATFORM_IS_ADR3

#define LLD_MSP0_ADDRESS       MSP0_REG_START_ADDR
#define LLD_MSP1_ADDRESS       MSP1_REG_START_ADDR
#define LLD_MSP2_ADDRESS       MSP2_REG_START_ADDR
#define LLD_MSP3_ADDRESS       MSP3_REG_START_ADDR

#define MSPid(x)               ((MspMap*)x)

#define MSP_REGISTER_RESET_VALUE 0x0
/*-----------------------------------------------------------------------------
	Typedefs
-----------------------------------------------------------------------------*/
typedef tU32 LLD_MSP_IdTy;

typedef enum
{
   /*   For MSP0 HS controller  STA_2062 */
   MSP_IRQ_RECEIVE			        = 0x1,
   MSP_IRQ_RECEIVE_OVERRUN_ERROR	= 0x2,
   MSP_IRQ_RECEIVE_FRAME_SYNC_ERROR = 0x4,
   MSP_IRQ_RECEIVE_FRAME_SYNC	    = 0x8,

   MSP_IRQ_TRANSMIT			        = 0x10,
   MSP_IRQ_TRANSMIT_OVERRUN_ERROR	= 0x20,
   MSP_IRQ_TRANSMIT_FRAME_SYNC_ERROR= 0x40,
   MSP_IRQ_TRANSMIT_FRAME_SYNC	    = 0x80,

   MSP_IRQ_RX_CLEARABLE				= 0x0F,
   MSP_IRQ_TX_CLEARABLE				= 0xF0,
   MSP_IRQ_ALL_CLEARABLE			= 0xFF,

   MSP_IRQ_RECEIVE_FIFO_NOT_EMPTY   = 0x100,
   MSP_IRQ_TRANSMIT_FIFO_NOT_FULL  = 0x200,

} LLD_MSP_INTty;


typedef enum
{
   MSP_FRAME_PL_HIGH,
   MSP_FRAME_PL_LOW
} LLD_MSP_FramePol_ty;

typedef enum
{
   MSP_FRAME_SEL_EXTERNAL		  = 0x0,
   MSP_FRAME_SEL_GEN_LOGIC		  = 0x2,
   MSP_FRAME_SEL_GEN_LOGIC_PERIOD = 0x3,
} LLD_MSP_FrameSel_ty;

typedef enum
{
  MSP_CLOCK_POL_RISE,
  MSP_CLOCK_POL_FALL
} LLD_MSP_ClockPol_ty;

typedef enum
{
   MSP_CLOCK_SEL_EXT 		= 0x0,
   MSP_CLOCK_SEL_INT 		= 0x1,
   MSP_CLOCK_FREE_EXT 		= 0x2,
   MSP_CLOCK_RESYNCRO_EXT 	= 0x3,
} LLD_MSP_ClockSel_ty;

typedef enum
{
   MSP_NO_SPICLOCK = 0x0,
   MSP_ZERO_DELAY_SPICLOCK = 0x2,
   MSP_HALF_DELAY_SPICLOCK = 0x3
} LLD_MSP_SPIClockMode_ty;

typedef enum
{
   MSP_8_BIT  = 0x0,
   MSP_10_BIT = 0x1,
   MSP_12_BIT = 0x2,
   MSP_14_BIT = 0x3,
   MSP_16_BIT = 0x4,
   MSP_20_BIT = 0x5,
   MSP_24_BIT = 0x6,
   MSP_32_BIT = 0x7,
} LLD_MSP_Bits4Elem_ty;

typedef enum
{
   MSP_NO_COMPANDING = 0x0,
   MSP_uLAW_COMPANDING = 0x2,
   MSP_ALAW_COMPANDING = 0x3
} LLD_MSP_DataType_ty;

typedef enum
{
   MSP_MSB_FIRST,
   MSP_LSB_FIRST,
} LLD_MSP_EndianForm_ty;

typedef enum
{
   MSP_0_Clock_Delay,
   MSP_1_Clock_Delay,
   MSP_2_Clock_Delay,
   MSP_3_Clock_Delay,
} LLD_MSP_DataDelay_ty;

typedef enum
{
   MSP_2PH_IMMEDIATE,
   MSP_2PH_DELAY,
} LLD_MSP_2PHStartMode_ty;

typedef enum
{
   MSP_NO_SWAP,
   MSP_WORD_SWAP,
   MSP_EACH_HALFWORD_SWAP,
   MSP_HALFWORD_SWAP
} LLD_MSP_HalfWordSwap_ty;

typedef enum
{
   MSP_FLAG_RECEIVE_BUSY 		=0x1,
   MSP_FLAG_RECEIVE_FIFOEMPTY	=0x2,
   MSP_FLAG_RECEIVE_FIFOFULL	=0x4,
   MSP_FLAG_TRANSMIT_BUSY		=0x8,
   MSP_FLAG_TRANSMIT_FIFOEMPTY	=0x10,
   MSP_FLAG_TRANSMIT_FIFOFULL	=0x20
} LLD_MSP_StatusFlag_ty;


typedef enum
{
   MSP_SubFrame_0_TO_31,
   MSP_SubFrame_32_TO_63,
   MSP_SubFrame_64_TO_95,
   MSP_SubFrame_98_TO_127,
} LLD_MSP_MCHSubFrame_ty;

typedef enum
{
   MSP_MCH_COMP_DISABLED  = 0x0,
   MSP_MCH_COMP_b2b_FALSE = 0x2,
   MSP_MCH_COMP_b2b_TRUE  = 0x3
} LLD_MSP_MCHComMD_ty;


//************** FUNCTION INTERFACE *************************//

extern tVoid LLD_MSP_ResetReg(MspMap * MSP);
extern tVoid LLD_MSP_WriteData(MspMap * MSP, tU32 data);
extern tU32 LLD_MSP_ReadData(MspMap * MSP);
extern tVoid LLD_MSP_DisRxFifo(MspMap * MSP);
extern tVoid LLD_MSP_EnRxFifo(MspMap * MSP);
extern tVoid LLD_MSP_SetRxFrameSyncroPolarity(MspMap * MSP, LLD_MSP_FramePol_ty polarity);
extern LLD_MSP_FramePol_ty LLD_MSP_GetRxFrameSyncroPolarity(MspMap * MSP);
extern tVoid LLD_MSP_DisDCM(MspMap * MSP);
extern tVoid LLD_MSP_EnDCM(MspMap * MSP);
extern tVoid LLD_MSP_SetRxFrameSyncroSelection(MspMap * MSP, LLD_MSP_FrameSel_ty Selection);
extern LLD_MSP_FrameSel_ty LLD_MSP_GetRxFrameSyncroSelection(MspMap * MSP);
extern tVoid LLD_MSP_SetRxClockPolarity(MspMap * MSP, LLD_MSP_ClockPol_ty Polarity);
extern LLD_MSP_ClockPol_ty LLD_MSP_GetRxClockPolarity(MspMap * MSP);
extern tVoid LLD_MSP_SetRxClock(MspMap * MSP, LLD_MSP_ClockSel_ty clock);
extern LLD_MSP_ClockSel_ty LLD_MSP_GetRxClock(MspMap * MSP);
extern tVoid LLD_MSP_DisLoopBack(MspMap * MSP);
extern tVoid LLD_MSP_EnLoopBack(MspMap * MSP);
extern tVoid LLD_MSP_DisTxFifo(MspMap * MSP);
extern tVoid LLD_MSP_EnTxFifo(MspMap * MSP);
extern tVoid LLD_MSP_SetTxFrameSyncroPolarity(MspMap * MSP, LLD_MSP_FramePol_ty polarity);
extern LLD_MSP_FramePol_ty LLD_MSP_GetTxFrameSyncroPolarity(MspMap * MSP);
extern tVoid LLD_MSP_SetTxFrameSyncroSelection(MspMap * MSP, LLD_MSP_FrameSel_ty Selection);
extern LLD_MSP_FrameSel_ty LLD_MSP_GetTxFrameSyncroSelection(MspMap * MSP);
extern tVoid LLD_MSP_SetTxClockPolarity(MspMap * MSP, LLD_MSP_ClockPol_ty Polarity);
extern LLD_MSP_ClockPol_ty LLD_MSP_GetTxClockPolarity(MspMap * MSP);
extern tVoid LLD_MSP_SetTxClock(MspMap * MSP, LLD_MSP_ClockSel_ty clock);
extern LLD_MSP_ClockSel_ty LLD_MSP_GetTxClock(MspMap * MSP);
extern tVoid LLD_MSP_DisTxDataExtraDelay(MspMap * MSP);
extern tVoid LLD_MSP_EnTxDataExtraDelay(MspMap * MSP);
extern inline tVoid LLD_MSP_DisSampleRateGen(MspMap * MSP);
extern inline tVoid LLD_MSP_EnSampleRateGen(MspMap * MSP);
extern tVoid LLD_MSP_SetSampleRateGenClockPolarity(MspMap * MSP, LLD_MSP_ClockPol_ty Polarity);
extern LLD_MSP_ClockPol_ty LLD_MSP_GetSampleRateGenClockPolarity(MspMap * MSP);
extern tVoid LLD_MSP_SetSampleRateGenClock(MspMap * MSP, LLD_MSP_ClockSel_ty clock);
extern LLD_MSP_ClockSel_ty LLD_MSP_GetSampleRateGenClock(MspMap * MSP);
extern inline tVoid LLD_MSP_DisFrameGen(MspMap * MSP);
extern inline tVoid LLD_MSP_EnFrameGen(MspMap * MSP);
extern tVoid LLD_MSP_SetSPIClockMode(MspMap * MSP, LLD_MSP_SPIClockMode_ty mode);
extern LLD_MSP_SPIClockMode_ty LLD_MSP_GetSPIClockMode(MspMap * MSP);
extern tVoid LLD_MSP_DisSPIBurstMd(MspMap * MSP);
extern tVoid LLD_MSP_EnSPIBurstMd(MspMap * MSP);
extern tVoid LLD_MSP_SetTxElemLength1PH(MspMap * MSP, LLD_MSP_Bits4Elem_ty length);
extern LLD_MSP_Bits4Elem_ty LLD_MSP_GetTxElemLength1PH(MspMap * MSP);
extern tVoid LLD_MSP_SetTxElemNumb1PH(MspMap * MSP, tU8 amount);
extern tU8 LLD_MSP_GetTxElemNumb1PH(MspMap * MSP);
extern tVoid LLD_MSP_SetTxDataType(MspMap * MSP, LLD_MSP_DataType_ty type);
extern LLD_MSP_DataType_ty LLD_MSP_GetTxDataType(MspMap * MSP);
extern tVoid LLD_MSP_SetTxEndianForm(MspMap * MSP, LLD_MSP_EndianForm_ty type);
extern LLD_MSP_EndianForm_ty LLD_MSP_GetTxEndianForm(MspMap * MSP);
extern tVoid LLD_MSP_SetTxDataDelay(MspMap * MSP, LLD_MSP_DataDelay_ty delay);
extern LLD_MSP_DataDelay_ty LLD_MSP_GetTxDataDelay(MspMap * MSP);
extern tVoid LLD_MSP_TxIgnoreUnexpectedPulse(MspMap * MSP);
extern tVoid LLD_MSP_TxConsiderUnexpectedPulse(MspMap * MSP);
extern tVoid LLD_MSP_SetTxElemLength2PH(MspMap * MSP, LLD_MSP_Bits4Elem_ty length);
extern LLD_MSP_Bits4Elem_ty LLD_MSP_GetTxElemLength2PH(MspMap * MSP);
extern tVoid LLD_MSP_SetTxElemNumb2PH(MspMap * MSP, tU8 amount);
extern tU8 LLD_MSP_GetTxElemNumb2PH(MspMap * MSP);
extern tVoid LLD_MSP_SetTxStartMode2PH(MspMap * MSP, LLD_MSP_2PHStartMode_ty mode);
extern LLD_MSP_2PHStartMode_ty LLD_MSP_GetTxStartMode2PH(MspMap * MSP);
extern tVoid LLD_MSP_DisTx2PH(MspMap * MSP);
extern tVoid LLD_MSP_EnTx2PH(MspMap * MSP);
extern tVoid LLD_MSP_SetTxHalfWordSwap(MspMap * MSP, LLD_MSP_HalfWordSwap_ty swap);
extern LLD_MSP_HalfWordSwap_ty LLD_MSP_GetTxHalfWordSwap(MspMap * MSP);
extern tVoid LLD_MSP_SetRxElemLength1PH(MspMap * MSP, LLD_MSP_Bits4Elem_ty length);
extern LLD_MSP_Bits4Elem_ty LLD_MSP_GetRxElemLength1PH(MspMap * MSP);
extern tVoid LLD_MSP_SetRxElemNumb1PH(MspMap * MSP, tU8 amount);
extern tU8 LLD_MSP_GetRxElemNumb1PH(MspMap * MSP);
extern tVoid LLD_MSP_SetRxDataType(MspMap * MSP, LLD_MSP_DataType_ty type);
extern LLD_MSP_DataType_ty LLD_MSP_GetRxDataType(MspMap * MSP);
extern tVoid LLD_MSP_SetRxEndianForm(MspMap * MSP, LLD_MSP_EndianForm_ty type);
extern LLD_MSP_EndianForm_ty LLD_MSP_GetRxEndianForm(MspMap * MSP);
extern tVoid LLD_MSP_SetRxDataDelay(MspMap * MSP, LLD_MSP_DataDelay_ty delay);
extern LLD_MSP_DataDelay_ty LLD_MSP_GetRxDataDelay(MspMap * MSP);
extern tVoid LLD_MSP_RxIgnoreUnexpectedPulse(MspMap * MSP);
extern tVoid LLD_MSP_RxConsiderUnexpectedPulse(MspMap * MSP);
extern tVoid LLD_MSP_SetRxElemLength2PH(MspMap * MSP, LLD_MSP_Bits4Elem_ty length);
extern LLD_MSP_Bits4Elem_ty LLD_MSP_GetRxElemLength2PH(MspMap * MSP);
extern tVoid LLD_MSP_SetRxElemNumb2PH(MspMap * MSP, tU8 amount);
extern tU8 LLD_MSP_GetRxElemNumb2PH(MspMap * MSP);
extern tVoid LLD_MSP_SetRxStartMode2PH(MspMap * MSP, LLD_MSP_2PHStartMode_ty mode);
extern LLD_MSP_2PHStartMode_ty LLD_MSP_GetRxStartMode2PH(MspMap * MSP);
extern tVoid LLD_MSP_DisRx2PH(MspMap * MSP);
extern tVoid LLD_MSP_EnRx2PH(MspMap * MSP);
extern tVoid LLD_MSP_SetRxHalfWordSwap(MspMap * MSP, LLD_MSP_HalfWordSwap_ty swap);
extern LLD_MSP_HalfWordSwap_ty LLD_MSP_GetRxHalfWordSwap(MspMap * MSP);
extern tVoid LLD_MSP_ConfigSampleRateGen(MspMap * MSP, tU32 ClockFreq, tU32 FrameFreq, tU32 DataClock,tU8 width);
extern tVoid LLD_MSP_GetSampleFreq(MspMap * MSP, tU32 ClockFreq, tU32* SampleFreq, tU32* period);
extern tVoid LLD_MSP_SetActiveFrameWidth(MspMap * MSP, tU8 width);
extern tU8 LLD_MSP_GetFrameWidth(MspMap * MSP);
extern tVoid LLD_MSP_SetFramePeriod(MspMap * MSP, tU16 period);
extern tU16 LLD_MSP_GetFramePeriod(MspMap * MSP);
extern tBool LLD_MSP_GetFlagStatus(MspMap * MSP, LLD_MSP_StatusFlag_ty flag);
extern tVoid LLD_MSP_EnTxDMA(MspMap * MSP);
extern tVoid LLD_MSP_DisTxDMA(MspMap * MSP);
extern tVoid LLD_MSP_EnRxDMA(MspMap * MSP);
extern tVoid LLD_MSP_DisRxDMA(MspMap * MSP);
extern tVoid LLD_MSP_IRQEnable(MspMap * MSP, LLD_MSP_INTty IRQ);
extern tVoid LLD_MSP_IRQDisable(MspMap * MSP, LLD_MSP_INTty IRQ);
extern tU16 LLD_MSP_GetIRQConfig(MspMap * MSP);
extern tU16 LLD_MSP_GetRawIRQStatus(MspMap * MSP);
extern tU16 LLD_MSP_GetIRQStatus(MspMap * MSP);
extern inline tVoid LLD_MSP_IRQClear(MspMap * MSP, LLD_MSP_INTty IRQ);
extern tVoid LLD_MSP_EnRxMultiCH(MspMap * MSP);
extern tVoid LLD_MSP_DisRxMultiCH(MspMap * MSP);
extern LLD_MSP_MCHSubFrame_ty LLD_MSP_GetRxMultiCHSubFrame(MspMap * MSP);
extern tVoid LLD_MSP_SetMultiCHCompareMD(MspMap * MSP, LLD_MSP_MCHComMD_ty mode);
extern LLD_MSP_MCHComMD_ty LLD_MSP_GetMultiCHCompareMD(MspMap * MSP);
extern tVoid LLD_MSP_EnTxMultiCH(MspMap * MSP);
extern tVoid LLD_MSP_DisTxMultiCH(MspMap * MSP);
extern LLD_MSP_MCHSubFrame_ty LLD_MSP_GetTxMultiCHSubFrame(MspMap * MSP);
extern tVoid LLD_MSP_SetRxMultiCompareReg(MspMap * MSP, tU32 mask);
extern tU32 LLD_MSP_GetRxMultiCompareReg(MspMap * MSP);
extern tVoid LLD_MSP_EnRxPINMultiCHCompare(MspMap * MSP, tU8 Channel);
extern tVoid LLD_MSP_DisRxPINMultiCHCompare(MspMap * MSP, tU8 Channel);
extern tVoid LLD_MSP_SetRxMultiCompareMask(MspMap * MSP, tU32 mask);
extern tU32 LLD_MSP_GetRxMultiCompareMask(MspMap * MSP);
extern tVoid LLD_MSP_EnRxPINMultiCHCompareMask(MspMap * MSP, tU8 Channel);
extern tVoid LLD_MSP_DisRxPINMultiCHCompareMask(MspMap * MSP, tU8 Channel);
extern tVoid LLD_MSP_SetTxMultiCHEnable(MspMap * MSP, tU8 bank,tU32 reg);
extern tU32 LLD_MSP_GetTxMultiCHEnable(MspMap * MSP, tU8 bank);
extern tVoid LLD_MSP_EnPINTxMultiCH(MspMap * MSP, tU8 Channel);
extern tVoid LLD_MSP_DisPINTxMultiCH(MspMap * MSP, tU8 Channel);
extern tVoid LLD_MSP_SetRxMultiCHEnable(MspMap * MSP, tU8 bank,tU32 reg);
extern tU32 LLD_MSP_GetRxMultiCHEnable(MspMap * MSP, tU8 bank);
extern tVoid LLD_MSP_EnPINRxMultiCH(MspMap * MSP, tU8 Channel);
extern tVoid LLD_MSP_DisPINRxMultiCH(MspMap * MSP, tU8 Channel);
extern tVoid LLD_MSP_EmptyRxFifo(MspMap * MSP);
extern tVoid LLD_MSP_EmptyTxFifo(MspMap * MSP);
extern tVoid LLD_MSP_DisableTxRx(MspMap * MSP);
extern tVoid LLD_MSP_DisableTx(MspMap * MSP);
extern tVoid LLD_MSP_DisableRx(MspMap * MSP);
extern tVoid LLD_MSP_EnableTxRx(MspMap * MSP);
extern tVoid LLD_MSP_EnableTx(MspMap * MSP);
extern tVoid LLD_MSP_EnableRx(MspMap * MSP);


#endif

#ifdef __cplusplus
}   /* Allow C++ to use this header */
#endif  /* __cplusplus              */

#endif	/* LLD_MSP_H_               */

// End of file

