/*--------------------------------------------------------------------------------------------
* STMicroelectronics
* Reproduction and Communication of this document is strictly prohibited
* unless specifically authorized in writing by STMicroelectronics
*---------------------------------------------------------------------------------------------
* Public Header file of I2C Controller  module
* Specification release related to this implementation:
*---------------------------------------------------------------------------------------------
*//******************************************************************************/



#ifndef LLD_I2C_H_
#define LLD_I2C_H_


#ifdef __cplusplus
extern "C"
{	/* In case C++ needs to use this header.*/
#endif


#define LLD_I2C0_ADDRESS       I2C0_REG_START_ADDR
#ifndef  PLATFORM_IS_ADR3
	#define LLD_I2C1_ADDRESS       I2C1_REG_START_ADDR
	#define LLD_I2C2_ADDRESS       I2C2_REG_START_ADDR
#endif
#define I2Cid(x)               ((I2cMap*)x)

#define LLD_I2C_GET_CARTESIOPLUS_CUT(a)           ((tU32)((tU32)(((I2cMap *)a)->PhID2)>>5))
/*-----------------------------------------------------------------------------
	Typedefs
-----------------------------------------------------------------------------*/
typedef enum
{
   /*   For I2C0 HS controller  STA_2062 */
   I2C_IRQ_TRANSMIT_FIFO_EMPTY        = 0x1,
   I2C_IRQ_TRANSMIT_FIFO_NEARLY_EMPTY = 0x2,
   I2C_IRQ_TRANSMIT_FIFO_FULL         = 0x4,
   I2C_IRQ_TRANSMIT_FIFO_OVERRUN      = 0x8,

   I2C_IRQ_RECEIVE_FIFO_EMPTY         = 0x10,
   I2C_IRQ_RECEIVE_FIFO_NEARLY_FULL   = 0x20,
   I2C_IRQ_RECEIVE_FIFO_FULL          = 0x40,

   I2C_IRQ_READ_FROM_SLAVE_REQUEST    = 0x10000,
   I2C_IRQ_READ_FROM_SLAVE_EMPTY      = 0x20000,
   I2C_IRQ_WRITE_TO_SLAVE_REQUEST     = 0x40000,
   I2C_IRQ_MASTER_TRANSACTION_DONE    = 0x80000,
   I2C_IRQ_SLAVE_TRANSACTION_DONE     = 0x100000,

   I2C_IRQ_MASTER_ARBITRATION_LOST    = 0x1000000,
   I2C_IRQ_BUS_ERROR                  = 0x2000000,

// ************** AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//
   I2C_IRQ_MASTER_TRANSACTION_DONE_WS = 0x10000000,

   I2C_IRQ_ALL_CLEARABLE              = 0x131F0008,
   I2C_IRQ_ALL                        = 0x131F007F
// ************** END AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//

 //  I2C_IRQ_ALL_CLEARABLE              = 0x031F0008,
 //  I2C_IRQ_ALL                        = 0x031F007F

} LLD_I2C_INTty;

typedef enum
{
   I2C_STANDARD_MODE 	= 100000,  //100 kb/s
   I2C_FAST_MODE 		= 400000, //400 kb/s
   I2C_HIGH_SPEED_MODE	=3400000 //3.4 Mb/s
} LLD_I2C_SpeedMty;

typedef enum
{
	I2C_MASTER_WRITE,
	I2C_MASTER_READ
} LLD_I2C_MAST_R_W;

typedef enum
{
	I2C_NO_STOP_CONDITION,
	I2C_STOP_CONDITION
} LLD_I2CStopCond;

typedef enum
{
	I2C_M_ADDR_GENERAL_CALL,
	I2C_M_ADDR_7_BITS,
	I2C_M_ADDR_10_BITS
} LLD_I2C_MAddrTy;

typedef enum
{
	I2C_NO_START_PROC,
	I2C_START_PROC
} LLD_I2CStartB;

typedef enum
{
   I2C_DIGITAL_FILTER_OFF,
   I2C_DIGITAL_FILTER_1_CLK_SPIKES,
   I2C_DIGITAL_FILTER_2_CLK_SPIKES,
   I2C_DIGITAL_FILTER_4_CLK_SPIKES
}LLD_I2C_Digital_Filter;

//typedef enum
//{
//   I2C_SPEED_STANDARD   =  100000,   /* Max clock frequency (Hz) for Standard Mode.*/
//   I2C_SPEED_FAST       =  400000,   /* Max clock frequency (Hz) for Fast Mode.*/
//   I2C_SPEED_HIGH_SPEED  = 3400000    /* Max clock frequency (Hz) for HS Mode.*/
//} LLD_I2C_MaxClocks;

typedef enum
{
   I2C_ADDR_7BITS,
   I2C_ADDR_10BITS
} LLD_I2C_addr_mode;

typedef enum
{
   I2C_BUS_SLAVE_MD ,             /*              Slave Mode */
   I2C_BUS_MASTER_MD,                /*             Master Mode */
   I2C_BUS_MASTER_SLAVE_MD           /* Dual Configuration Mode */
} LLD_I2C_bus_control_mode;

typedef enum
{
   I2C_STATUS_MASTER_TX,
   I2C_STATUS_MASTER_RX,
   I2C_STATUS_SLAVE_RX,
   I2C_STATUS_SLAVE_TX,
}LLD_I2C_OP_STATUS;

typedef enum
{
   I2C_NO_OP,
   I2C_ON_GOING,
   I2C_OK,
   I2C_ABORT,
}LLD_I2C_CONTROL_STATUS;

typedef enum
{
   I2C_NACK_ADDR,
   I2C_NACK_DATA,
   I2C_ACK_MCODE,
   I2C_ARB_LOST,
   I2C_BERR_START,
   I2C_BERR_STOP,
   I2C_OVFL,
}LLD_I2C_ABORT_CAUSE;

typedef enum
{
   I2C_FRAME,
   I2C_GCALL,
   I2C_HW_GCALL,
}LLD_I2C_RX_TYPE;
/* Uart comunication control block struct definition */
typedef struct
{
	I2cMap *	   id;							// I2C id (it corresponds with the peripheral's base address)
	tU8*           pu8WrBuffer;      /* Write buffer start address */
	tU8*           pu8WrBufferEnd;
	tU32           u32WrBuffSize;    /* Size of Write-Buffer */
	tU8*           pu8WrBufferISR;   /* Pointer to Tx Fifo buffer used by ISR */
	tU8*           pu8WrBufferDEV;   /* Pointer to Tx Fifo buffer used by DEVICE */
	tU32           u32WrNbytes;      /* Number of bytes to be Tx */
	/* Info's */
	//   tU32           u32TransmitBytes; /* Total Number of bytes transmitted */
	//   tU32           u32WrNumberOfOverflowBytes; /* Number of bytes thrown away because of buffer overflow */
	//  tU32           u32WrNumberOfOverflows;     /* Number of Buffer-overflow situations */

	tU8*           pu8RdBuffer;      /* Rx Fifo buffer start address */
	tU8*           pu8RdBufferEnd;
	tU32           u32RdBuffSize;    /* Size of Read-Buffer */
	tU8*           pu8RdBufferISR;   /* Pointer to Rx Fifo buffer used by ISR */
	tU8*           pu8RdBufferDEV;   /* Pointer to Rx Fifo buffer used by DEVICE */
	tU32           u32RdNbytes;      /* Number of Bytes in Read buffer */
	/* Info's */
	//  tU32           u32ReceivedBytes; /* Total Number of bytes received */
	// tU32           u32RdNumberOfOverflowBytes; /* Number of bytes thrown away because of buffer overflow */
	//  tU32           u32RdNumberOfOverflows;     /* Number of Buffer-overflow situations */

} I2C_trBuffCtrl;

typedef union
{
	struct
	{
	tU8 OP:1;
	tU8 A10:1;
	tU8 SB:1;
	tU8 P:1;
	tU8 LENGTH:1;
	tU8 reserved:3;
	}BIT;
	tU8 REG;
}tI2C_MCR_flags;

extern tVoid LLD_I2C_ResetReg(I2cMap * I2C);
extern tVoid LLD_I2C_SetDigitalFilter(I2cMap * I2C, LLD_I2C_Digital_Filter Filter);
extern tVoid LLD_I2C_En_LoopBack(I2cMap * I2C);
extern tVoid LLD_I2C_Dis_LoopBack(I2cMap * I2C);
extern tVoid LLD_I2C_En_DMASyncLogic(I2cMap * I2C);
extern tVoid LLD_I2C_Dis_DMASyncLogic(I2cMap * I2C);
extern tVoid LLD_I2C_Enable_DMA_Rx_En(I2cMap * I2C);
extern tVoid LLD_I2C_Disable_DMA_Rx_En(I2cMap * I2C);
extern tVoid LLD_I2C_Enable_DMA_Tx_En(I2cMap * I2C);
extern tVoid LLD_I2C_Disable_DMA_Tx_En(I2cMap * I2C);
extern tVoid LLD_I2C_RxFlush(I2cMap * I2C);
extern tVoid LLD_I2C_TxFlush(I2cMap * I2C);
extern tVoid LLD_I2C_Enable_SGCM(I2cMap * I2C);
extern tVoid LLD_I2C_Disable_SGCM(I2cMap * I2C);
extern tVoid LLD_I2C_SetSpeedMode(I2cMap * I2C, LLD_I2C_SpeedMty clock);
extern tVoid LLD_I2C_SetOpMode(I2cMap * I2C, LLD_I2C_bus_control_mode mode);
extern tVoid LLD_I2C_Enable(I2cMap * I2C);
extern tVoid LLD_I2C_Disable(I2cMap * I2C);
extern tVoid LLD_I2C_SlaveSetUpTime(I2cMap * I2C, tU16 SLSU);
extern tBool LLD_I2C_SetOwnSlaveAddr(I2cMap * I2C, const tU32 Addr);
extern tVoid LLD_I2C_SetHS_MCR(I2cMap * I2C, tU8 MC);
extern tVoid LLD_I2C_SetMasterRWB(I2cMap * I2C, LLD_I2C_MAST_R_W mode);
extern tBool LLD_I2C_SetSlaveAddr(I2cMap * I2C, const tU32 Addr);
extern tVoid LLD_I2C_Master_GenericCall(I2cMap * I2C);
extern tVoid LLD_I2C_SetMasterStartB(I2cMap * I2C, LLD_I2CStartB SB);
extern tVoid LLD_I2C_SetMasterStop(I2cMap * I2C, LLD_I2CStopCond cond);
extern tVoid LLD_I2C_ByteCommunication(I2cMap * I2C, tU16 Length);
//tVoid LLD_I2C_StartMasterOp(I2cMap * I2C);
extern tVoid LLD_I2C_SetMasterControl(I2cMap * I2C,tI2C_MCR I2C_MCR);
extern tI2C_MCR LLD_I2C_GetMasterControl(I2cMap * I2C);
extern inline tU8 LLD_I2C_WriteFifo(I2cMap * I2C, tU8 Nchar, tU8 * byte);
extern inline tU8 LLD_I2C_ReadFifo(I2cMap * I2C, tU8 Nchar, tU8 * byte);
extern tVoid LLD_I2C_Get_TX_THRESHOLD(I2cMap * I2C, tU16 * ThresHold);
extern tVoid LLD_I2C_Get_RX_THRESHOLD(I2cMap * I2C, tU16 * ThresHold);
extern tVoid LLD_I2C_Set_TX_THRESHOLD(I2cMap * I2C, tU16 ThresHold);
extern tVoid LLD_I2C_Set_RX_THRESHOLD(I2cMap * I2C, tU16 ThresHold);
extern tVoid LLD_I2C_SetDMAR_TX(I2cMap * I2C,tU8 size);
extern tVoid LLD_I2C_SetDMAR_RX(I2cMap * I2C,tU8 size);
extern tVoid LLD_I2C_SetBaudRateCR(I2cMap * I2C, LLD_I2C_SpeedMty SpeedMode, tU32 InputFreq);
extern tVoid LLD_I2C_InterruptEnable(I2cMap * I2C, LLD_I2C_INTty Mask);
extern tVoid LLD_I2C_InterruptDisable(I2cMap * I2C, LLD_I2C_INTty Mask);
extern tU32 LLD_I2C_GetInterruptStatus(I2cMap * I2C, LLD_I2C_INTty Mask);
extern tU32 LLD_I2C_GetRawInterruptStatus(I2cMap * I2C, LLD_I2C_INTty Mask);
extern tVoid LLD_I2C_ClearInterrupt(I2cMap * I2C, LLD_I2C_INTty Mask);
extern tS32 LLD_I2C_MasterTx_polling(I2cMap * I2C, tU32 AddrSlave, tU32 Nchar, tU8* pdata);
extern tS32 LLD_I2C_MasterRx_polling (I2cMap * I2C, tU32 AddrSlave, tU32 Nchar, tU8* pdata);
extern tS32 LLD_I2C_MasterRx_SubAddr_Poll (I2cMap * I2C, tU32 AddrSlave, tU8* subAddr, tU8 NsubAddr,tU32 Nchar, tU8* pdata);
extern tU32 LLD_I2C_MasterTx_IRQ(I2cMap * I2C, tU32 AddrSlave, tU32 Nchar, tU8* pdata, LLD_I2CStopCond STOP_cond );
extern tVoid LLD_I2C_MasterRx_IRQ_SubAddr (I2cMap * I2C, tU32 AddrSlave,  tU32 Nchar, tU8* pdata);
extern tVoid LLD_I2C_MasterRx_NOACK (I2cMap * I2C, tU32 AddrSlave,  tU32 Nchar, tU8* pdata);

// End of file



#ifdef __cplusplus
}   /* Allow C++ to use this header */
#endif  /* __cplusplus              */

#endif	/* _I2CHCL_H_               */
