
//_______________________________________________________________________
//| FILE:         lld_sdi_sta206x.h
//| PROJECT:
//| SW-COMPONENT: LLD
//|_______________________________________________________________________
//| DESCRIPTION:  SDI low level driver header file
//|_______________________________________________________________________
//| COPYRIGHT:    (c)STMicroelectronics,
//| HISTORY:
//| Date      | Modification               | Author
//|_______________________________________________________________________
//| 07.11.09  | Initial revision           |
//|_______________________________________________________________________
//| 09.07.14  |    |
//|           |                   |
//|_______________________________________________________________________

#ifndef _LLD_SDI_H_
#define _LLD_SDI_H_

//----------------------------------------------------------------------
// ---------------------------- MACRO ----------------------------------
//----------------------------------------------------------------------

#define READ_BIT(reg, bit)					(((tU32)reg) &  ((tU32)1  << (tU32)bit))
#define SET_BIT(Reg, Bit)					(((tU32)reg) |= (((tU32)1 << (tU32)bit)))

#define CLEAR_BITS(Reg,Bit)						((reg) &= ~(mask))

#define READ_BITFIELD(Reg,StartPos,EndPos)		(Reg<<(31-EndPos))>>(31-(EndPos-StartPos))	
#define WRITE_BITFIELD(StartPos,EndPos,Val,Reg)	Reg=(Val|(Reg& ~((0xFFFFFFFF<<(31-EndPos))>>(31-(EndPos-StartPos))<<StartPos)))

//----------------------------------------------------------------------
// ---------------------------- DEFINES ---------------------------------
//----------------------------------------------------------------------


#define HALF_FIFO_SIZE			8

#define DUMMY_RCA			0x00000000

// SDI POWER CONTROL REGISTER DEFINES 
#define PWCTRL_ENABLE			(0x03)
#define DAT2DIREN_MASK		(1<<2)
#define CMDDIREN_MASK			(1<<3)      
#define DAT0DIREN_MASK		(1<<4)
#define DAT31DIREN_MASK		(1<<5)
#define SDICMD_MASK    			(1<<6)
#define FBCLKEN_MASK    		(1<<7)    
#define DAT74DIREN_MASK		(1<<8)

//SDI CLOCK CONTROL REGISTER DEFINES  

#define CLKDIV_300KHZ   			0x0000009E//divisor without  bypass bit
#define CLKDIV_400KHZ   			0x00000076//divisor without  bypass bit

#define CLKDIV_48MHZ			     0x00000000//divisor with bypass bit set
#define CLKDIV_24MHZ			     0x00000000//divisor without  bypass bit

#define CLK_EN_MASK 				(1<<8)
#define CLK_PWRSAVE_MASK 		(1<<9)
#define CLK_BYPASS_MASK 			(1<<10)
#define WIDEBUS_MASK 				(0x00001800)

#define CLK_NEGEDGE_MASK 			(1<<13)
#define CLK_HWFCTRL_MASK 			(1<<14)

//SDI COMMAND REGISTER DEFINES  
#define WAITRESP_MASK 				(1<<6)
#define LONGRESP_MASK 				(1<<7)
#define WAITINT_MASK 				      (1<<8)
#define WAITPEND_MASK 				(1<<9)
#define CPSMEN_MASK 				      (1<<10)
#define CMD_COMPLETION_MASK 		(1<<12)

#define DATA_CTRL_WRITE_SETTINGS 		0x99
#define DATA_CTRL_READ_SETTINGS 		0x9B


//SDI STATUS REGISTER  DEFINES  
#define	CMDCRCFAIL_MASK     		       (0x00000001)   //Command Response received (CRC check failed)                                       
#define	DATACRCFAIL_MASK   		       (0x00000002)   //Data block sent/received (CRC check failed)                                        
#define	CMDTIMEOUT_MASK      		       (0x00000004)    //Command Response Time-out                                                          
#define	DATATIMEOUT_MASK     		     (0x00000008)  //Data Time-out                                                                      
#define	TXUNDERR_MASK        		      (0x00000010)     //Transmit FIFO Underrun error                                                       
#define	RXOVERR_MASK         		       (0x00000020)    //Received FIFO Overrun error                                                        
#define	CMDREND_MASK         		       (0x00000040)    //Command Response received (CRC check passed)                                       
#define	CMDSENT_MASK         		        (0x00000080)     //Command Sent (no response required)                                                
#define	DATAEND_MASK         		      (0x00000100)      //Data End (Data Counter, SDIDCOUNT, is zero)                                        
#define	STARTBIT_ERR_MASK    		      (0x00000200)  //Start Bit not detected on all data signals in wide bus mode                        
#define	DATABLOCK_END_MASK          	(0x00000400)    //Data Block sent/received (CRC check passed)                                        
#define	CMDACT_MASK       		       	(0x00000800)      //Command transfer in progress                                                       
#define	TXACT_MASK                             (0x00001000)      //Data transmit in progress                                                          
#define	RXACT_MASK                   			(0x00002000)		//Data receive in progress                                                           
#define	TX_FIFOBURST_WRITE        		(0x00004000)			//Transmit FIFO Burst Writable.At least a burst (8 words) can be written in the FIFO)
#define	RX_FIFOBURST_READ         		(0x00008000) 	//Receive FIFO Burst Readable.There is at least a burst (8 words) in the FIFO        
#define	TX_FIFOFULL_MASK       	        	(0x00010000)  //Transmit FIFO Full                                                                 
#define	RX_FIFOFULL_MASK                    (0x00020000)		//Receive FIFO Full                                                                  
#define	TX_FIFOEMPTY_MASK                (0x00040000)    //Transmit FIFO Empty                                                                
#define	RX_FIFOEMPTY_MASK   			 (0x00080000)      //Receive FIFO Empty                                                                 
#define	TX_DATA_AVAIL_MASK           	 (0x00100000)		//Data available in transmit FIFO                                                    
#define	RX_DATA_AVAIL_MASK               (0x00200000)	//Data available in receive FIFO 


#define ALL_STATICFLAGS_MASK			0x004007FF
#define ALL_ERRORBITS_MASK			0xFDFFE008 //1111 1101 1111 1111 1110 0000 0000 1000b

//SDI INTERRUPT REGISTER  DEFINES  
#define ALL_INTERRUPTS_MASK    		0x007FFFFF


//SDI DATA CONTROL REGISTER  DEFINES  
#define DPSM_EN_MASK   				(1<<0)
#define DMA_EN_MASK         				(1<<3)

#define READ_DIR            					0x00000002
#define WRITE_DIR           				0x00000000

#define STREAM_MODE         				0x00000004
#define BLOCK_MODE         				0x00000000


// SDI PERIPHERAL ID REGISTER DEFINES  
#define PERIPHERAL_ID0					0x00000080
#define PERIPHERAL_ID1					0x00000001
#define PERIPHERAL_ID2					0x00000048
#define PERIPHERAL_ID3					0x00000000

// P-CELL ID REGISTER DEFINES   
#define PCELL_ID0						0x0000000D
#define PCELL_ID1						0x000000F0
#define PCELL_ID2						0x00000005
#define PCELL_ID3						0x000000B1

//PROTOCOLS COMMANDS DEFINES
#define CMD0_GO_IDLE_STATE			   (0)
#define CMD1_MMC_SEND_OP_COND		   (1) //MMC only
#define CMD2_ALL_SEND_CID             		   (2)
#define CMD3_SET_RELATIVE_ADDR		   (3)  //MMC only 
#define CMD4_SET_DSR                  		   (4)
#define CMD6_MMC_SWITCH				   (6)

#define CMD6_SET_BUS_WIDTH		         (6)//SD only

#define CMD7_SELECT_DESELECT_CARD	   (7)
#define CMD8_SEND_IF_COND 				   (8)
#define CMD8_MMC_SEND_EXT_CSD 		   (8)

#define CMD9_SEND_CSD                 		   (9)
#define CMD10_SEND_CID                 		   (10)
#define CMD11_SWITCH      				         (11)
#define CMD12_STOP_TRANSMISSION		(12)  
#define CMD13_SEND_STATUS              		(13)
#define CMD15_GO_INACTIVE_STATE        	(15)
#define CMD16_SET_BLOCKLEN             		(16)
#define CMD17_READ_SINGLE_BLOCK       	(17)
#define CMD18_READ_MULTIPLE_BLOCK	(18)
#define CMD19_SEND_TUNING_PATTERN	(19)
#define CMD20_SPEED_CLASS_CONTROL     	(20)
#define CMD23_SET_BLOCK_COUNT          	(23)
#define CMD24_WRITE_SINGLE_BLOCK       	(24)   
#define CMD25_WRITE_MULTIPLE_BLOCK	(25)
#define CMD26_PROGRAM_CID                 	(26) 
#define CMD27_PROGRAM_CSD                 	(27)
#define CMD28_SET_WRITE_PROT           	(28)
#define CMD29_CLR_WRITE_PROT           	(29)  
#define CMD30_SEND_WRITE_PROT          	(30)
#define CMD32_ERASE_WR_BLK_START       	(32)
#define CMD33_ERASE_WR_BLK_END	       (33)
#define CMD34_UNTAG_SECTOR	       	(34)
#define CMD35_ERASE_GROUP_START          (35)  
#define CMD36_ERASE_GROUP_END		(36)
#define CMD37_UNTAG_ERASE_GROUP		(37)
#define CMD38_ERASE                    			(38)
#define CMD39_FAST_IO                  		(39) 
#define ACMD41_SEND_OP_COND              	(41)
#define CMD42_LOCK_UNLOCK             		(42)
#define CMD55_APP_CMD                  		(55)
#define CMD56_GEN_CMD                 		(56)


//===============================
//-----CMD8_SEND_IF_COND  DEFINES ----|
//===============================
#define CMD8_HIGH_VOLTAGE_RANGE			0x00000100	//2.7V - 3.6V--->High Voltage
#define CMD8_LOW_VOLTAGE_RANGE			0x00000200	//1.65V - 1.95V--->Low Voltage 
#define CMD8_CHECK_PATTERN				0x000000AA	//10101010b
#define CMD8_HIGH_VOLTAGE_ARG                    (CMD8_HIGH_VOLTAGE_RANGE |CMD8_CHECK_PATTERN)    //0x000001AA 

//===============================
//----ACMD41_SEND_OP_COND DEFINES ---|
//===============================
#define ACMD41_HIGH_VOLTAGE_RANGE         (0x00FF8000)  // high voltage range for SD card : 2.7 - 3.6V 
#define ACMD41_LOW_VOLTAGE_RANGE          (0x00000080) //low voltage range  for SD card : 1.65 - 1.95V 
#define ACMD41_HCS_BIT				(1<<30)//HOST high capacity status bit in ACMD41 argument (0x40000000)

#define ACMD41_OCR_CCS_BIT			(1<<30)//CARD high capacity status bit in ACMD41 response (OCR ) (0x40000000)
#define ACMD41_OCR_BUSY_BIT         (1<<31)//card busy bit in ACMD41 response (OCR )  0x80000000  



//==================================
//----CMD1_MMC_SEND_OP_COND  DEFINES ---|
//==================================
#define CMD1_HIGH_VOLTAGE_RANGE         (0x00FF8000)  // high voltage range for MMC card: 2.7 - 3.6V 
#define CMD1_LOW_VOLTAGE_RANGE          (0x00000080) //low voltage range  for MMC card : 1.65 - 1.95V 
#define CMD1_SECTOR_MODE		         (1<<30)//----- CMD1 ---- (0x40000000)

//==========================
//--------SD CARD  DEFINES ------|
//==========================

#define SD_CSD_V_1_0          		      0
#define SD_CSD_V_2_0          		      1
//#define SD_SPEC_VERS_HS	      4

//===========================
//--------MMC CARD  DEFINES ------|
//===========================
#define MMC_SPEC_VERS_HS	      4
//#define CARD_CAPACITY_2GB            0x500000000

//===========================
//-------TIMEOUTS   DEFINES ------|
//===========================
#define CMD0_TIMEOUT				0xFFFF
#define CMD1_TIMEOUT				0xFFFF
#define DATATIMEOUT				0x000FFFFF
#define CMD0TIMEOUT				100000
#define ACMD41_TIMEOUT			0xFFFF

//==================================
//--------MISC COMMANDS  DEFINES --------|
//==================================



#define SDI_FIFO_ADDRESS          		0x101F5080
#define SDI_SCR_CLOCK_VAL			0x00100000
#define SDI_SCR_CLOCK				0x101E0058




#define SDI_ALL_INT_FLAG   			0x007FFFFF
#define ALL_ONE          					0xFFFFFFFF
#define ALL_ZERO         				0x00000000


#define SDI_MAXBLSIZE				2048
#define BUFFSIZE 					32768 //65024	

#define SPEC_VERSION				0x3C000000
#define SPEC_HIGHSPEED				0x4

#define MMC_MAXBYTES				0x0000FFFC
#define MMC_MAXDATALENGTH		0x0000FFFF

#define EXIT			0XFF

#define SC_SDMMC_SIZE			2

#define FAT_SECTOR_SIZE			512



#define     TAAC_TU_1nS            1
#define     TAAC_TU_10nS          10
#define     TAAC_TU_100nS        100
#define     TAAC_TU_1uS            1
#define     TAAC_TU_10uS          10
#define     TAAC_TU_100uS        100
#define     TAAC_TU_1mS           1
#define     TAAC_TU_10mS         10

#define     TAAC_MF_1_0            (float)1
#define     TAAC_MF_1_2            (float)1.2
#define     TAAC_MF_1_3            (float)1.3
#define     TAAC_MF_1_5            (float)1.5
#define     TAAC_MF_2_0            (float)2
#define     TAAC_MF_2_5            (float)1.5
#define     TAAC_MF_3_0            (float)3
#define     TAAC_MF_3_5            (float)3.5
#define     TAAC_MF_4_0            (float)4
#define     TAAC_MF_4_5            (float)4.5
#define     TAAC_MF_5_0            (float)5
#define     TAAC_MF_5_5            (float)5.5
#define     TAAC_MF_6_0            (float)6
#define     TAAC_MF_7_0            (float)7
#define     TAAC_MF_8_0            (float)8


//----------------------------------------------------------------------
// -------------------------ENUMS  -----------------------------
//----------------------------------------------------------------------

typedef enum
{
      IDLE_STATE                     =0,
      READY_STATE                 =1,
      IDENTIFICATION_STATE   =2,
      STANDBY_STATE             =3,
      TRANSFER_STATE            =4,
      SEND_DATA_STATE         =5,
      RCV_DATA_STATE          =6,
      PROGRAMMING_STATE    =7,
      DISCONNECTED_STATE     =8,
      RESERVED_09__STATE          =9,
      RESERVED_10_STATE            =10,
      RESERVED_11_STATE            =11,
      RESERVED_12_STATE            =12,
      RESERVED_13_STATE            =13,
      RESERVED_14_STATE            =14
    	
}SDI_CardStateTy;

typedef enum
{
    	BYTE_MODE  =0,
    	SECTOR_MODE=1
    	
}SDI_CMD1_ModeBitTy;

typedef enum
{
      STD_MMC,
      SECURE_MMC,
      CONT_PROT_SECURE_MMC,
      SECURE_MMC_20,
      ATA_ON_MMC

}SDI_MMC_extCSD_CmdSetTy;


typedef enum
{
	CLK_FREQ_300KHZ =0,
	CLK_FREQ_400KHZ,
	CLK_FREQ_24MHZ,
	CLK_FREQ_48MHZ
	
}SDI_ClockFreqTy;


typedef enum
{
	SDI_WRITE_DIRECTION=0,
	SDI_READ_DIRECTION=1
	
}SDI_TranDirectionTy;

typedef enum
{
	SDI_BLOCK_MODE=0,
	SDI_STREAM_MODE=1
	
}SDI_TranModeTy;

typedef enum
{   
      LOW_SPEED_MMC_CARD,     //
      HIGH_SPEED_MMC_CARD,   //

      LOW_SPEED_SD_CARD,     //
      HIGH_SPEED_SD_CARD,   //


      HIGH_SPEED_MMC_CARD_26MHZ,
      HIGH_SPEED_MMC_CARD_52MHZ

}CARD_SpeedTy;



typedef enum
{   
      //MMC cards voltage range labels   
      HIGH_VOLTAGE_MMC_CARD,
      DUAL_VOLTAGE_MMC_CARD,
      
      //SD cards voltage range labels   
      HIGH_VOLTAGE_SD_CARD,
      DUAL_VOLTAGE_SD_CARD

}CARD_VoltageRangeTy;

typedef enum
{   
      //MMC cards capacity labels
      STD_CAPACITY_MMC_CARD,     //SDSC  up 2 GB --> host compliant to ph.layer v. 1.xx and higher
      HIGH_CAPACITY_MMC_CARD,   //SDHC up 32 GB--> host compliant to ph.layer v. 2.0 and higher
      STD_CAPACITY,
      HIGH_CAPACITY,
      
      MMC_CARD,//dummy type for initialization only
      SD_CARD,//dummy type for initialization only
      //SD cards capacity labels
      STD_CAPACITY_SD_CARD,     //SDSC  up 2 GB --> host compliant to ph.layer v. 1.xx and higher
      LEGACY_SD_CARD,
      HIGH_CAPACITY_SD_CARD,   //SDHC up 32 GB--> host compliant to ph.layer v. 2.0 and higher
      EXT_CAPACITY_SD_CARD,      //SDXC  up 2 TB--> host compliant to ph.layer v. 3.0 and higher

}CARD_CapacityTy;


typedef enum
{
    	SDI_TRANSFER_IN_PROGRESS,
    	SDI_NO_TRANSFER
    	
}SDI_TransferStateTy;

typedef enum
{
    	SDI_POLLING_MODE,
    	SDI_INTERRUPT_MODE,
    	SDI_DMA_MODE
    	
}SDI_DeviceModeTy;

typedef enum
{
   	SDI_BUS_1_BIT_WIDE =0,
    	SDI_BUS_4_BIT_WIDE =1,
   	SDI_BUS_8_BIT_WIDE =2
    
} SDI_BusModeTy;


typedef enum
{
	SDI_CARD_DESELECT,
	SDI_CARD_SELECT
	
} SDI_CardSelectOptTy;


typedef enum
{
	SWITCH_BUS_WIDE,
	SWITCH_HS_TIMING,
     SWITCH_GET_extCSD

} SDI_SwitchOperationTy;

typedef enum 
{ 
	SDI_CARD_ID_0,
	SDI_CARD_ID_1,
	SDI_CARD_ID_2
	
} SDI_CardId_Ty;

typedef enum 
{ 
	SDI_DMA_WRITE_TO_FIFO,
	SDI_DMA_READ_FROM_FIFO
} SDI_DMA_OperationTy;

typedef enum 
{ 
	SDI_POWER_OFF,
	SDI_POWER_ON	
	
} SDI_PowerStatusTy;


typedef enum 
{ 
	SDI_DISABLE,
	SDI_ENABLE

} SDI_EnableTy;

typedef enum
{
	SDI_CARD_UNSELECTED=0,
	SDI_CARD_SELECTED=1
	
}SDI_CardSelected_StatusTy;

typedef enum
{
	SDI_PUSH_PULL,
	SDI_OPEN_DRAIN
}SDI_CmdBusTy;

//***ERROR LABELS***
typedef enum
{
    
    SDI_CMD_CRC_FAIL      =1,            //Command response received (but CRC check failed) 
    SDI_DATA_CRC_FAIL                   =2,            /* Data bock sent/received (CRC check Failed) */
    SDI_CMD_RSP_TIMEOUT                 =3,            /* Command response timeout */
    SDI_DATA_TIMEOUT                    =4,            /* Data time out*/
    SDI_TX_UNDERRUN                     =5,            /* Transmit FIFO under-run */
    SDI_RX_OVERRUN                      =6,            /* Receive FIFO over-run */
    SDI_START_BIT_ERR                   =7,            /* Start bit not detected on all data signals in widE bus mode */
    SDI_CMD_OUT_OF_RANGE                =8,            /* CMD's argument was out of range.*/
    SDI_ADDR_MISALIGNED                 =9,            /* Misaligned address */
    SDI_BLOCK_LEN_ERR                   =10,           /* Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
    SDI_ERASE_SEQ_ERR                   =11,           /* An error in the sequence of erase command occurs.*/
    SDI_BAD_ERASE_PARAM                 =12,           /* An Invalid selection for erase groups */
    SDI_WRITE_PROT_VIOLATION            =13,           /* Attempt to program a write protect block */
    SDI_LOCK_UNLOCK_FAILED              =14,           /* Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
    SDI_COM_CRC_FAILED                  =15,           /* CRC check of the previous command failed */
    SDI_ILLEGAL_CMD                     =16,           /* Command is not legal for the card state */
    SDI_CARD_ECC_FAILED                 =17,           /* Card internal ECC was applied but failed to correct the data */
    SDI_CC_ERROR                        =18,           /* Internal card controller error */
    SDI_GENERAL_UNKNOWN_ERROR           =19,           /* General or Unknown error */
    SDI_STREAM_READ_UNDERRUN            =20,           /* The card could not sustain data transfer in stream read operation. */
    SDI_STREAM_WRITE_OVERRUN            =21,           /* The card could not sustain data programming in stream mode */
    SDI_CID_CSD_OVERWRITE               =22,           /* CID/CSD overwrite error */
    SDI_WP_ERASE_SKIP                   =23,           /* only partial address space was erased */
    SDI_CARD_ECC_DISABLED               =24,           /* Command has been executed without using internal ECC */
    SDI_ERASE_RESET                     =25,           /* Erase sequence was cleared before executing because an out of erase sequence command was received */
    SDI_AKE_SEQ_ERROR                   =26,           /* Error in sequence of authentication. */
    SDI_INVALID_VOLTRANGE               =27,
    SDI_SEL_DESEL_STATUS_UNKNOW              =28,
    SDI_SWITCH_ERROR                    =29,
   
    //standard error defines 
    SDI_INTERNAL_ERROR                       =30,
    SDI_NOT_CONFIGURED                        =31,
    SDI_REQUEST_PENDING                      =32,
    SDI_REQUEST_NOT_APPLICABLE        =33,
    SDI_INVALID_PARAMETER                 =34,
    SDI_UNSUPPORTED_FEATURE             =35,
    SDI_UNSUPPORTED_HW                      =36,
    SDI_INTERNAL_EVENT                       =38,
    SDI_APP_CMD_ERROR                       =39,
    SDI_DATA_TRANSFER_ERROR =40,

    
    //global errors
   SDI_OK                              =0X00,
   SDI_ERROR                         =0xFF,

}SDI_ErrorTy;

typedef enum
{
      TAAC_TU_0      = 0,
      TAAC_TU_1     = 1,
      TAAC_TU_2   = 2,
      TAAC_TU_3       = 3,
      TAAC_TU_4     = 4,
      TAAC_TU_5   = 5,
      TAAC_TU_6       = 6,
      TAAC_TU_7     = 7

}TAAC_TimeUnitTy;

typedef enum
{
      TAAC_MUL_FACT_0       = 0,
      TAAC_MUL_FACT_1       = 1,
      TAAC_MUL_FACT_2       = 2,
      TAAC_MUL_FACT_3       = 3,
      TAAC_MUL_FACT_4       = 4,
      TAAC_MUL_FACT_5       = 5,
      TAAC_MUL_FACT_6       = 6,
      TAAC_MUL_FACT_7       = 7,
      TAAC_MUL_FACT_8       = 8,
      TAAC_MUL_FACT_9       = 9,
      TAAC_MUL_FACT_10     = 10,
      TAAC_MUL_FACT_11     = 11,
      TAAC_MUL_FACT_12     = 12,
      TAAC_MUL_FACT_13     = 13,
      TAAC_MUL_FACT_14     = 14,
      TAAC_MUL_FACT_15     = 15,
      TAAC_MUL_FACT_16     = 16

}TAAC_MultFactTy;


//----------------------------------------------------------------------
// ------------------------ STRUCT ------------------------------
//----------------------------------------------------------------------

//Card access mode: 
            //12.5 MB/sec interface speed (default)
            //25 MB/sec interface speed. (highspeed)
//Card command system:
            //Standard command set (default)
            // eCommerce command set 

typedef union 
{
	struct 
	{
            tU8		Card_Acc_Mode        :4; //[3:0] function group 1 for Card access mode
            tU8		Card_Cmd_Sys           :4;//[7:4] function group 2 for Card command system
            tU8		FG3_Dummy              :4;//[11:8] reserved for function group 3 (0h or Fh)
            tU8		FG4_Dummy              :4;//[15:12] reserved for function group 4 (0h or Fh)
            tU8		FG5_Dummy             :4;//[19:16] reserved for function group 5 (0h or Fh)
            tU8		FG6_Dummy              :4;//[23:20] reserved for function group 6 (0h or Fh)
            tU8		reserved                    :7;//[30:24] reserved (All ’0’)
            tU8		Mode                          :1;//[31] Mode, 0:Check function,1:Switch function
        }FIELDS;	
	tU32 	REG ;
}SDI_SD_SwitchTy;

typedef union 
{
	struct 
	{
		tU32		Cmd_Set:3;
		tU32		res1:5;
		tU32		Value:8;
		tU32		Reg_Byte_pt:8;
		tU32		Access_Mode:2;
		tU32		res2:6;
      
	}FIELDS;	
	tU32 	REG ;
}SDI_MMC_SwitchTy;


typedef struct
{
      CARD_CapacityTy		                     Capacity;			         
      CARD_VoltageRangeTy                    VoltRange;
      CARD_SpeedTy                                 Speed;
      CARD_CapacityTy                             Dummy_Type;
} SDI_CardTy;


typedef union // 32-BIT CARD STATUS REGISTER
{
	struct 
	{
		tU32		res_0_1_2:                      3;//bits 0,1,2
		tU32		Ake_Seq_Error:               1;//bit 3
		tU32		res_4:                              1;//bit4
		tU32		App_Cmd:                        1;//bit 5
              tU32		res_6_7:                          2;//bit 6,7
		//tU32		Switch_Error:                   1;//only MMC Card
		tU32		Ready_For_Data:             1;//bit 8
		tU32		Current_State:                 4;// bits 9,10,11,12
		tU32		Erase_Reset:                    1;//bit 13
		tU32		Card_ECC_Disabled:         1;//bit 14
		tU32		Wp_Erase_Skip:                1;//bit 15
		tU32		CSD_Overwrite:                1;//bit 16
		//tU32		Stream_Read_Underrun:  1;//only MMC Card
		//tU32		Stream_Write_Overrun:   1;//only MMC Card
              tU32		res_17_18:                       2;//bits 17, 18 
		tU32 	      General_Error:                  1;//bits 19
		tU32		CC_Error:                          1;//20
		tU32		Card_ECC_Fail:                 1;//21
		tU32		Illegal_Command:              1;//22
		tU32		Cmd_CRC_Error:                 1;//23
		tU32		Lock_Unlock_Fail:                 1;//24
		tU32		Card_Is_Locked:                  1;//25
		tU32 	      Wp_Violation:                    1;            //26
		tU32		Erase_Param_Error:           1;//27
		tU32		Erase_Seq_Error:                1;//28
		tU32		Block_Lenght_Error:          1;//29
		tU32		Address_Error:                   1;//30
		tU32		Cmd_Out_Of_Range:            1;//31
	}FIELDS;	

	tU32 	REG ;
}SDI_CARD_STATUS_Ty;


typedef union 
{
	struct 
	{
	 tU32   time_unit_msec;//
            tU32   time_unit;//
            float   mult_factor;
	}FIELDS;	
	tU32 	REG ;
}TAAC_Ty;


typedef struct
{  
   tU32   CSD_STRUCTURE;
   tU32 Nac;

      tU32   MMC_COMMAND_SET;
      tU32   MMC_SEC_COUNT;
      TAAC_Ty   TAAC;                   //
      tU32   NSAC;                   //
      tU32   TRAN_SPEED;       //value reads from CSD-->max clock frequency supported to card (hz)

} SDI_RegsFieldsTy;

typedef struct
{

      tU32                                            OCR;
      tU32      					               CID[4];
      tU32      					               CSD[4];
      tU32                                            *extCSD;//MMC only    
      tU32     					               RCA;
      
      SDI_CARD_STATUS_Ty              CardStatus;
      SDI_RegsFieldsTy                      Fields;

} SDI_Card_RegsTy;

typedef struct
{

	SDI_CardId_Ty          		Card_Id;	
	SDI_PowerStatusTy		Power_Status;
	SDI_CmdBusTy			     Cmd_BusMode;
	SDI_BusModeTy 			Bus_Wide;
	SDI_DeviceModeTy 		Device_Mode;
	SDI_EnableTy				HS_Timing_Ctrl;
	SDI_CardTy					Card_Type;
	SDI_CardSelected_StatusTy    	Card_Selected_Status;

      tU32        			Block_Len;
      SDI_ErrorTy	     ErrorStatus;
      tU32				     Card_BaseAddress;   
      tU64                       Card_Capacity;      //value reads from CSD-->total memory card capacity (bytes) 
      tU32        			Clock_Divisor;
      tU32                    	ClockFreq;        //current clock frequency (KHz)
      SDI_CardStateTy   Current_State; 

} SDI_CardControlTy;

/**********************************************************************
***********************************************************************
**********************************************************************/

typedef struct
{
	tU32 		      Total_Number_Of_Byte;	//drivers defined value ---> total number of bytes to transfer in read/write operation
	tU32			Curr_Read_Block_Len;	//drivers defined value--> size  of use block lenght unit (bytes)
	tU32			Curr_Write_Block_Len;	//drivers defined value--> size  of use block lenght unit (bytes)
	tU32			Curr_Card_Block_Len;	//drivers defined value--> size  of use block lenght unit (bytes)
	tU16			Transferred_Bytes;		//drivers defined value ---> total number of bytes to transferred in read/write operation
	
} SDI_DPSM_ControlTy;


typedef struct
{
     SDI_SD_SwitchTy                        SD_Switch_Fields;
     SDI_MMC_SwitchTy                     MMC_Switch_Fields;
     
} SDI_CPSM_ControlTy;


typedef struct
{
	SDI_DPSM_ControlTy		DPSM_Context;
	SDI_CPSM_ControlTy     	CPSM_Context;
  	SDI_CardControlTy           Card_Context;
	SDI_Card_RegsTy             Card_Regs;
   
}SDI_SystemContextTy;


/**********************************************************************
***********************************************************************
**********************************************************************/


extern SDI_SystemContextTy SDI_SystemContext;
extern SDI_SystemContextTy *SDI_SystemContext_pt;

/**********************************************************************
***********************************************************************
**********************************************************************/
extern SDI_ErrorTy SDI_GPIO_Config(SDI_SystemContextTy *SystemContext, AltFunc_Ty *GPIO_SD_MMC1_AF_SETUP);
extern tVoid SDI_DMA_Enable(DmaMap *DMA, SDI_SystemContextTy *SystemContext , tU32 *SDI_Buffer,SDI_DMA_OperationTy DMA_Operation);

//extern SDI_ErrorTy SDI_Set_ClockFreq (SdiMap *SDI, SDI_SystemContextTy *SystemContext,tU32 ClockFreq);

extern SDI_ErrorTy SDI_Decode_CSD (SdiMap *SDI, SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_Get_CSD (SdiMap *SDI, SDI_SystemContextTy *SystemContext, tU32 rca);
extern SDI_ErrorTy SDI_Set_RCA (SdiMap *SDI, SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_Get_CID (SdiMap *SDI, SDI_SystemContextTy *SystemContext, tU32 CmdIndex, tU32 rca);
extern SDI_ErrorTy MMC_Get_ExtCSD(SdiMap *SDI, SDI_SystemContextTy *SystemContext, tU32 *p_extcsd);

extern SDI_ErrorTy SDI_SDHS_Timing_En(SdiMap *SDI, SDI_SystemContextTy *SystemContext, SDI_EnableTy HS_TimingCtrl);
extern SDI_ErrorTy SDI_MMCHS_Timing_En (SdiMap *SDI, SDI_SystemContextTy *SystemContext, SDI_EnableTy HS_TimingCtrl);
extern SDI_ErrorTy SDI_HS_Timing_En (SdiMap *SDI,SDI_SystemContextTy *SystemContext, SDI_EnableTy HS_TimingCtrl);

extern SDI_ErrorTy SDI_SDMMC_WideBus_En (SdiMap *SDI, SDI_SystemContextTy *SystemContext, SDI_BusModeTy BusWide);
extern SDI_ErrorTy SDI_SD_WideBus_En (SdiMap *SDI, SDI_SystemContextTy *SystemContext, SDI_BusModeTy BusWide);
extern SDI_ErrorTy SDI_MMC_WideBus_En (SdiMap *SDI, SDI_SystemContextTy *SystemContext, SDI_BusModeTy BusWide);

extern SDI_ErrorTy SDI_Last_Transfer_Info(SdiMap *SDI,SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_Cmd0_Error(SdiMap *SDI,SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_R1_Error(SdiMap *SDI,SDI_SystemContextTy *SystemContext, tU32 CmdIndex);
extern SDI_ErrorTy SDI_R2_Error(SdiMap *SDI,SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_R3_Error(SdiMap *SDI,SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_R6_Error (SdiMap *SDI,SDI_SystemContextTy *SystemContext, tU32 CmdIndex);
extern SDI_ErrorTy SDI_R7_Error(SdiMap *SDI,SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_Card_Identify(SdiMap *SDI,SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_Card_Initialize(SdiMap *SDI, SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_SelectCard (SdiMap *SDI,SDI_SystemContextTy *SystemContext, SDI_CardSelectOptTy SelectOperation);
extern SDI_ErrorTy SDI_Card_WakeUp(SdiMap *SDI,SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_Pcell_Pid_Check(SdiMap *SDI, SDI_SystemContextTy *SystemContext);
extern SDI_ErrorTy SDI_Check_CardState(SdiMap *SDI, SDI_SystemContextTy *SystemContext, tU32 *CardState);
extern SDI_ErrorTy SDI_WriteBlocks(SdiMap *SDI, SDI_SystemContextTy *SystemContext, tU64 lba, tU16 blocksize, tU32 no_of_blocks);
extern SDI_ErrorTy SDI_FAT_WriteSector(SdiMap *SDI, DmaMap *Dma, SDI_SystemContextTy *SystemContext, tU64 lba, tU32 *Outbuff_ptr);
//extern SDI_ErrorTy SDI_FAT_WriteSectors(SdiMap *SDI,	DmaMap *Dma, SDI_SystemContextTy *SystemContext, tU64 lba, tU32 num, tU32 *Outbuff_ptr);
extern SDI_ErrorTy SDI_ReadBlocks(SdiMap *SDI,SDI_SystemContextTy *SystemContext, tU64 lba, tU16 blocksize, tU32 no_of_blocks );

extern tVoid  FAT_SDI_GetCardData (tU64 *dimension, tU32 *sectorspertrack, tU32 *heads, SDI_SystemContextTy *SystemContext);

extern SDI_ErrorTy SDI_FAT_ReadSector(SdiMap *SDI, DmaMap *Dma, SDI_SystemContextTy *SystemContext, tU64 lba, tU32 *Inbuff_ptr);
//extern SDI_ErrorTy SDI_FAT_ReadSectors(SdiMap *SDI,	DmaMap *Dma, SDI_SystemContextTy *SystemContext, tU64 lba, tU32 num, tU32 *Inbuff_ptr);
extern SDI_ErrorTy SDI_FAT_Card_Initialize(SdiMap *SDI, SDI_SystemContextTy *SystemContext, tU32 Card_BaseAddress, AltFunc_Ty *GPIO_SD_MMC1_AF);
extern tVoid  SDI_FAT_GetCardData (tU64 *dimension, tU32 *sectorspertrack, tU32 *heads, SDI_SystemContextTy *SystemContext);				
extern void SDI_PowerOFF(SdiMap *SDI,SDI_SystemContextTy *SystemContext);
extern void SDI_Set_Card_ID(SDI_SystemContextTy *SystemContext, tU32 Card_BaseAddress );
extern void SDI_Context_Initialize(SDI_SystemContextTy *SystemContext, SDI_DeviceModeTy DeviceMode, SDI_BusModeTy BusMode);
extern void SDI_CmdBus_Config(SdiMap *SDI,SDI_SystemContextTy *SystemContext,SDI_CmdBusTy CmdBusMode);

extern SDI_TransferStateTy SDI_Check_Host_TransferState(SdiMap *SDI);

extern tU32 PowerOfTwo (tU32 exp);
extern SDI_ErrorTy  SDI_Check_ReadyForData (SdiMap *SDI, SDI_SystemContextTy *SystemContext);
extern tVoid SDI_Calculate_Nac (SDI_SystemContextTy *SystemContext);
extern tVoid wait_clock (long nop_nb);
extern SDI_ErrorTy SDI_CheckProgrammingState(SdiMap *SDI, SDI_SystemContextTy *SystemContext);

extern tU8 SDI_FromBytesToPowOfTwo(tU16 NumberOfBytes);
extern SDI_ErrorTy SDI_Get_Current_State (SdiMap *SDI, SDI_SystemContextTy *SystemContext, tU32 *Current_State, tU32 *ReadyForData );

/**********************************************************************
***********************************************************************
**********************************************************************/



//----------------------------------------------------------------------
// function prototypes
//----------------------------------------------------------------------

#endif // _LLD_SDI_H_

// End of file

