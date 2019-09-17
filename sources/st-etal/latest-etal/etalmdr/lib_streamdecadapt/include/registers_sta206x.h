// _____________________________________________________________________________
//| FILE:         registers_sta2062.h
//| PROJECT:      ADR3 - STA660
//|_____________________________________________________________________________
//| DESCRIPTION:  Peripherals register maps
//|_____________________________________________________________________________
//| COPYRIGHT:    (c) 2009 STMicroelectronics, Agrate Brianza (MI) (ITALY)
//|
//| HISTORY:
//| Date        | Modification               | Author
//|_____________________________________________________________________________
//| 2009.06.26  | Initial revision           | A^_^L
//| 2009.10.12  | Added MTU map              | Luigi Cotignano
//| 2009.10.13  | Added SDI map              | Marco Barboni
//|_____________________________________________________________________________


#ifndef _REGISTERS_STA2062_H_
#define _REGISTERS_STA2062_H_

//------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------
#include "types.h"
#include "macro.h"

//------------------------------------------------------------------------
// General Purpose IOs
//------------------------------------------------------------------------
typedef volatile struct
{
   tU32 GPIO_DAT; // address offset = 0x000

   tU32 GPIO_DATS; // address offset = 0x004

   tU32 GPIO_DATC; // address offset = 0x008

   tU32 GPIO_PDIS; // address offset = 0x00C

   tU32 GPIO_DIR; // address offset = 0x010

   tU32 GPIO_DIRS; // address offset = 0x014

   tU32 GPIO_DIRC; // address offset = 0x018

   tU32 GPIO_SLPM;   // address offset = 0x01C

   tU32 GPIO_AFSLA; // address offset = 0x020

   tU32 GPIO_AFSLB; // address offset = 0x024

   gap(12);

   tU32 GPIO_RIMSC; // address offset = 0x040

   tU32 GPIO_FIMSC; // address offset = 0x044

   tU32 GPIO_IS; // address offset = 0x048

   tU32 GPIO_IC; // address offset = 0x04C

   gap(1992);

   tU32 GPIOPeriphID0; // address offset = 0xFE0
   tU32 GPIOPeriphID1; // address offset = 0xFE4
   tU32 GPIOPeriphID2; // address offset = 0xFE8
   tU32 GPIOPeriphID3; // address offset = 0xFEC

	tU32 GPIOPCellID0; // address offset = 0xFF0
	tU32 GPIOPCellID1; // address offset = 0xFF4
	tU32 GPIOPCellID2; // address offset = 0xFF8
	tU32 GPIOPCellID3; // address offset = 0xFFC

} GpioMap;

//------------------------------------------------------------------------
// Universal asynchronous Receiver and Transmitter (UART)
//------------------------------------------------------------------------
typedef volatile struct
{
	struct								// addr offset = 0x00
	{
		tU16 DATA:8;
		tU16 FE:1;
		tU16 PE:1;
		tU16 BE:1;
		tU16 OE:1;
		tU16 reserved:4;
	}UARTDR;

    intra16;

    tU8 UARTRSR; // and UARTECR     // addr offset = 0x04

	intra8;
	gap(1);

// ************** AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//

	struct								// addr offset = 0x08
	{
		tU8 TXDMAWM:3;
		tU8 RXDMAWM:3;
		tU8 reserved:2;
	}UARTDMAWM;
	intra8;
	gap(1);

	struct								// addr offset = 0x0C
	{
		tU32 TIMEOUT:22;
		tU32 reserved:10;
	}UARTTIMEOUT;

	gap(4);							  // addr offset = 0x10, 0x14

// ************** END AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//

	struct								// addr offset = 0x18
	{
		tU16 CTS:1;
		tU16 DSR:1;
		tU16 DCD:1;
		tU16 BUSY:1;
		tU16 RXFE:1;
		tU16 TXFF:1;
		tU16 RXFF:1;
		tU16 TXFE:1;
// ************** AVAILABLE JUST ON CUT1 CARTESIO+ / CARTESIO ***************************//
		tU16 reserved0:1;
		tU16 DCTS:1;
		tU16 DDSR:1;
		tU16 DDCD:1;
		tU16 TERI:1;
// ************** END AVAILABLE JUST ON CUT1 CARTESIO+ / CARTESIO  ***************************//
		tU16 RTX_DIS:1;
		tU16 reserved:2;
	}UARTFR;

	gap(1);

// ************** AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//

	struct								// addr offset = 0x1C
	{
		tU16 reserved0:1;
		tU16 PEN:1;
		tU16 EPS:1;
		tU16 STP2:1;
		tU16 FEN:1;
		tU16 WLEN:2;
		tU16 SPS:1;
		tU16 reserved1:8;
	}UARTLCR_H_RX;

	intra16;

// ************** END AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//



	tU8 UARTILPR;						// addr offset = 0x20

	intra8;
	intra16;

	tU16 UARTIBRD;						// addr offset = 0x24

	intra16;

	tU8 UARTFBRD:6;						// addr offset = 0x28

	intra8;
	intra16;

	struct								// addr offset = 0x2C
	{
		tU16 BRK:1;
		tU16 PEN:1;
		tU16 EPS:1;
		tU16 STP2:1;
		tU16 FEN:1;
		tU16 WLEN:2;
		tU16 SPS:1;
		tU16 reserved:8;
	}UARTLCR_H_TX;


	intra16;

	struct								// addr offset = 0x30
	{
		tU16 UARTEN:1;
		tU16 SIREN:1;
		tU16 SIRLP:1;
		tU16 reserved:4;
		tU16 LBE:1;
		tU16 TXE:1;
		tU16 RXE:1;
		tU16 DTR:1;
		tU16 RTS:1;
		tU16 OUT1:1;
		tU16 OUT2:1;
		tU16 RTSEN:1;
		tU16 CTSEN:1;
	}UARTCR;

	intra16;

	struct								// addr offset = 0x34
	{
		tU16 TXIFLSEL:3;
		tU16 RXIFLSEL:3;
		tU16 reserved:10;
	}UARTIFLS;

	intra16;

	struct								// addr offset = 0x38
	{
		tU16 RIMIM:1;
		tU16 CTSMIM:1;
		tU16 DCDMIM:1;
		tU16 DSRMIM:1;
		tU16 RXIM:1;
		tU16 TXIM:1;
		tU16 RTIM:1;
		tU16 FEIM:1;
		tU16 PEIM:1;
		tU16 BEIM:1;
		tU16 OEIM:1;
		tU16 reserved:5;
	}UARTIMSC;

	intra16;

	struct								// addr offset = 0x3C
	{
		tU16 RIRMIS:1;
		tU16 CTSRMIS:1;
		tU16 DCDRMIS:1;
		tU16 DSRRMIS:1;
		tU16 RXRIS:1;
		tU16 TXRIS:1;
		tU16 RTRIS:1;
		tU16 FERIS:1;
		tU16 PERIS:1;
		tU16 BERIS:1;
		tU16 OERIS:1;
		tU16 reserved:5;
	}UARTRIS;

	intra16;

	struct								// addr offset = 0x40
	{
		tU16 RIMMIS:1;
		tU16 CTSMMIS:1;
		tU16 DCDMMIS:1;
		tU16 DSRMMIS:1;
		tU16 RXMIS:1;
		tU16 TXMIS:1;
		tU16 RTMIS:1;
		tU16 FEMIS:1;
		tU16 PEMIS:1;
		tU16 BEMIS:1;
		tU16 OEMIS:1;
		tU16 reserved:5;
	}UARTMIS;

	intra16;

	struct								// addr offset = 0x44
	{
		tU16 RIMIC:1;
		tU16 CTSMIC:1;
		tU16 DCDMIC:1;
		tU16 DSRMIC:1;
		tU16 RXIC:1;
		tU16 TXIC:1;
		tU16 RTIC:1;
		tU16 FEIC:1;
		tU16 PEIC:1;
		tU16 BEIC:1;
		tU16 OEIC:1;
		tU16 reserved:5;
	}UARTICR;

	intra16;

	struct								// addr offset = 0x48
	{
		tU16 RXDMAE:1;
		tU16 TXDMAE:1;
		tU16 DMAONERR:1;
		tU16 reserved:13;
	}UARTDMACR;

	gap(1995);

	struct								// addr offset = 0xFE0
	{
		tU16 PartNumber0:8;
		tU16 reserved:8;
	}UARTPeriphID0;

	intra16;

	struct								// addr offset = 0xFE4
	{
		tU16 PartNumber1:4;
		tU16 Designer0:4;
		tU16 reserved:8;
	}UARTPeriphID1;

	intra16;

	struct								// addr offset = 0xFE8
	{
		tU16 Designer1:4;
		tU16 Revision:4;
		tU16 reserved:8;
	}UARTPeriphID2;

	intra16;

	struct								// addr offset = 0xFEC
	{
		tU16 Configuration:8;
		tU16 reserved:8;
	}UARTPeriphID3;

	intra16;

	struct								// addr offset = 0xFF0
	{
		tU16 UARTPCellID0:8;
		tU16 reserved:8;
	}UARTPCellID0;

	intra16;

	struct								// addr offset = 0xFF4
	{
		tU16 UARTPCellID1:8;
		tU16 reserved:8;
	}UARTPCellID1;

	intra16;


	struct								// addr offset = 0xFF8
	{
		tU16 UARTPCellID2:8;
		tU16 reserved:8;
	}UARTPCellID2;

	intra16;


	struct								// addr offset = 0xFFC
	{
		tU16 UARTPCellID3:8;
		tU16 reserved:8;
	}UARTPCellID3;

	intra16;
}UartMap;


//---------------------------------------
//    SDI interface memory map
//---------------------------------------
typedef volatile struct
{
	union   //POWER CONTROL REGISTER - Offset 0x00
	{
		struct
		{
			tU32 PWRCTRL:2;		// Power Supply Control Bits(00b: Power off: the clock to card is stopped;11b: Power on: the card is clocked)
			tU32 DAT2DIREN:1;	//
			tU32 CMDDIREN:1;	//
			tU32 DAT0DIREN:1;	//
			tU32 DAT31DIREN:1;	//
			tU32 SDI_CMD:1 ;			// SDICMD Output Control Bit
			tU32 FBCLKEN:1;		//
			tU32 DAT74DIREN:1;	//
			tU32 RESERVED:23;	//
		}BIT;
		tU32 REG;
	}SDI_PWR;

	union    //CLOCK CONTROL REGISTER - Offset 0x04
	{
		struct
		{
			tU32 CLKDIV:8;
			tU32 CLKEN:1;		//Clock Enable Bit
			tU32 PWRSAV:1;
			tU32 BYPASS:1;
			tU32 WIDBUS:2;
			tU32 NEGEDGE:1;
			tU32 HWFC_EN:1;
			tU32 RESERVED:15;
		}BIT;
		tU32 REG;
	}SDI_CLKCR;

	tU32 SDI_ARG; //SDI ARGUMENT REGISTER - Offset 0x08

	union     		//SDI COMMAND REGISTER - Offset 0x0C
	{
		struct
		{
			tU32 CMDINDEX:6;
			tU32 WAITRESP:1;
			tU32 LONGRESP:1;
			tU32 WAITINT:1;
			tU32 WAITPEND:1;
			tU32 CPSMEN:1;
			tU32 SDIO_SUSP:1;
			tU32 ENCMD:1;
			tU32 NIEN:1;
			tU32 CE_ATA_CMD:1;
			tU32 CBOOTMODEEN:1;
			tU32 RESERVED:16;
		}BIT;
		tU32 REG;
	}SDI_CMD;

	union    //SDI Command Response Register - Offset 0x10
	{
		struct
		{
			tU32 RESPCMD:6;
			tU32 RESERVED:26;
		}BIT;
		tU32 REG;
	}SDI_RESPCMD;

	tU32 SDI_RESP0;    // SDI Response Registers 0 - Offset 0x14
	tU32 SDI_RESP1;    // SDI Response Registers 1 - Offset 0x18
	tU32 SDI_RESP2;    // SDI Response Registers 2 - Offset 0x1C
	tU32 SDI_RESP3;    // SDI Response Registers 3 - Offset 0x20

	tU32 SDI_DTIMER;   // SDI Data Timer Register - Offset 0x24

	union   // SDI Data LENGHT Register - Offset 0x28
	{
		struct
		{
			tU32 DATALENGTH:25;
			tU32 RESERVED:7;
		}BIT;
		tU32 REG;
	}SDI_DLEN;

	union      // SDI Data CONTROL Register - Offset 0x2C
	{
		struct
		{
			tU32 DTEN:1;
			tU32 DTDIR:1;
			tU32 DTMODE:1;
			tU32 DMAEN:1;
			tU32 DBLOCKSIZE:4;
			tU32 RWSTART:1;
			tU32 RWSTOP:1;
			tU32 RWMOD:1;
			tU32 SDIOEN:1;
			tU32 DMAREQCTL:1;
			tU32 DBOOTMODEEN:1;
			tU32 BUSYMODE:1;
			tU32 RESERVED:17;
		}BIT;
		tU32 REG;
	}SDI_DCTRL;

	union    // SDI Data COUNTER Register - Offset 0x30
		{
			struct
			{
				tU32 DATACOUNT:24;
				tU32 RESERVED:8;
			}BIT;
			tU32 REG;
		}SDI_DCOUNT;

	union // SDI STSTUS Register - Offset 0x34
	{
		struct
		{
			tU32	CMD_CRC_FAIL					:1;           //Command Response received (CRC check failed)
			tU32	DATA_CRC_FAIL					:1;           //Data block sent/received (CRC check failed)
			tU32	CMD_TIMEOUT		    		:1;           //Command Response Time-out
			tU32	DATA_TIMEOUT		    	:1;           //Data Time-out
			tU32	TX_UNDERR		    			:1;             //Transmit FIFO Underrun error
			tU32	RX_OVERR 		    			:1;               //Received FIFO Overrun error
			tU32	CMD_REND 		    			:1;               //Command Response received (CRC check passed)
			tU32	CMD_SENT 		    			:1;             //Command Sent (no response required)
			tU32	DATA_END 		    			:1;             //Data End (Data Counter, SDIDCOUNT, is zero)
			tU32	START_BIT_ERR		    	:1;           //Start Bit not detected on all data signals in wide bus mode
			tU32	DBCKEND 		    			:1;             //Data Block sent/received (CRC check passed)
			tU32	CMD_IN_ACT          	:1;       //Command transfer in progress
			tU32	TX_IN_ACT           	:1;       //Data transmit in progress
			tU32	RX_IN_ACT			      	:1;          //Data receive in progress
			tU32	TX_FIFO_BURST_W				:1;           //Transmit FIFO Burst Writable.At least a burst (8 words) can be written in the FIFO)
			tU32	RX_FIFO_BURST_R      	:1;	   //Receive FIFO Burst Readable.There is at least a burst (8 words) in the FIFO
			tU32	TX_FIFO_FULL  				:1;             //Transmit FIFO Full
			tU32	RX_FIFO_FULL      		:1;       //Receive FIFO Full
			tU32	TX_FIFO_EMPTY					:1;               //Transmit FIFO Empty
			tU32	RX_FIFO_EMPTY					:1;               //Receive FIFO Empty
			tU32	TX_DATA_AVAILABLE			:1;			 //Data available in transmit FIFO
			tU32	RX_DATA_AVAILABLE			:1;		   //Data available in receive FIFO
			tU32	SDIOIT								:1;               //SDIO Interrupt received
			tU32	CEATAEND							:1;	             //CE-ATA command completion signal received for CMD61
			tU32	CARD_BUSY							:1;	             //Card busy (card is pulling down DAT0)
			tU32	BOOT_MODE							:1;		           //Boot Mode by keeping CMD LOW is ongoing
			tU32	BOOT_ACK_ERR					:1;				     //Boot Acknowledge is not valid
			tU32	BOOT_ACK_TIMEOUT			:1;         //Boot Acknowledge Timeout
			tU32 	RESERVED							:4;

		}BIT;
		tU32 REG;
	}SDI_STATUS;

	union   // SDI INTERRUPT CLEAR Register - Offset 0x38
	{
		struct
		{
			tU32 CMDCRCFAIL_CLR:1;
			tU32 DATACRCFAIL_CLR:1;
			tU32 CMDTIMEOUT_CLR:1;
			tU32 DATATIMEOUT_CLR:1;
			tU32 TXUNDERRUN_CLR:1;
			tU32 RXOVERRUN_CLR:1;
			tU32 CMDREND_CLR:1;
			tU32 CMDSENT_CLR:1;
			tU32 DATAEND_CLR:1;
			tU32 STARTBITERR_CLR:1;
			tU32 DBCKEND_CLR:1;
			tU32 RESERVED_11:11;
			tU32 SDIOIT_CLR:1;
			tU32 CEATAEND_CLR:1;
			tU32 BUSYEND_CLR:1;
			tU32 RESERVED_1:1;
			tU32 BOOTACKERR_CLR:1;
			tU32 BOOTACKTIMEOUT_CLR:1;
			tU32 RESERVED_4:4;
		}BIT;
		tU32 REG;
	}SDI_ICR;

	union   // SDI MASK 0 Register - Offset 0x3C
	{
		struct
		{
			tU32 MASK0:1;
			tU32 MASK1:1;
			tU32 MASK2:1;
			tU32 MASK3:1;
			tU32 MASK4:1;
			tU32 MASK5:1;
			tU32 MASK6:1;
			tU32 MASK7:1;
			tU32 MASK8:1;
			tU32 MASK9:1;
			tU32 MASK10:1;
			tU32 MASK11:1;
			tU32 MASK12:1;
			tU32 MASK13:1;
			tU32 MASK14:1;
			tU32 MASK15:1;
			tU32 MASK16:1;
			tU32 MASK17:1;
			tU32 MASK18:1;
			tU32 MASK19:1;
			tU32 MASK20:1;
			tU32 MASK21:1;
			tU32 MASK22:1;
			tU32 MASK23:1;
			tU32 MASK24:1;
			tU32 MASK25:1;
			tU32 MASK26:1;
			tU32 MASK27:1;
			tU32 RESERVED:4;
		}BIT;
		tU32 REG;
	}SDI_MASK0;

	union    // SDI MASK 1 Register Offset 0x40
	{
		struct
		{
			tU32 MASK0:1;
			tU32 MASK1:1;
			tU32 MASK2:1;
			tU32 MASK3:1;
			tU32 MASK4:1;
			tU32 MASK5:1;
			tU32 MASK6:1;
			tU32 MASK7:1;
			tU32 MASK8:1;
			tU32 MASK9:1;
			tU32 MASK10:1;
			tU32 MASK11:1;
			tU32 MASK12:1;
			tU32 MASK13:1;
			tU32 MASK14:1;
			tU32 MASK15:1;
			tU32 MASK16:1;
			tU32 MASK17:1;
			tU32 MASK18:1;
			tU32 MASK19:1;
			tU32 MASK20:1;
			tU32 MASK21:1;
			tU32 RESERVED:10;
		}BIT;
		tU32 REG;
	}SDI_MASK1;

	union   //SDI CARD SELECT Register  - Offset 0x44
	{
		struct
		{
			tU32 SDCARD_ADD:4;
			tU32 RESERVED:28;
		}BIT;
		tU32 REG;
	}SDI_CSEL;

	union   //SDI FIFO COUNTER Register  -  Offset 0x48
	{
		struct
		{
			tU32 DATACOUNT:24;
			tU32 RESERVED:8;
		}BIT;
		tU32 REG;
	}SDI_FIFOCNT;

	tU16  gap_26[26];

//	gap(26); //0x4C->0x80

	tU32 SDI_FIFO; //SDI Data FIFO Register - Offset 0x80

	tU16  gap_30[30];
	//gap(30); //0x84->0xC0

	tU32 SDI_DBTIME; //SDI Boot Acknowledge Time Register - Offset 0xC0

	tU16  gap_1932[1934];
//	gap(1934); //0xC4->0xFE0

	//SDI Peripheral Identification Registers
	tU32 SDIPeriphID0; // address offset = 0xFE0
	tU32 SDIPeriphID1; // address offset = 0xFE4
	tU32 SDIPeriphID2; // address offset = 0xFE8
	tU32 SDIPeriphID3; // address offset = 0xFEC

	//SDI PCell Identification Registers
	tU32 SDIPCellID0; // address offset = 0xFF0
	tU32 SDIPCellID1; // address offset = 0xFF4
	tU32 SDIPCellID2; // address offset = 0xFF8
	tU32 SDIPCellID3; // address offset = 0xFFC

}SdiMap;

//------------------------------------------------------------------------
// I2C (0..2)
//------------------------------------------------------------------------
typedef union
{
	struct
	{
		tU32 OP:1;
		tU32 A10:10;
		//tU32 EA10:3;
		tU32 SB:1;
		tU32 AM:2;
		tU32 P:1;
		tU32 LENGTH:11;
		tU32 reserved:6;
	}BIT;
	tU32 REG;
}tI2C_MCR;

typedef volatile struct
{
	union
	{
		struct
		{
			tU16 PE:1;
			tU16 OM:2;
			tU16 SAM:1;
			tU16 SM:2;
			tU16 SGCM:1;
			tU16 FTX:1;
			tU16 FRX:1;
			tU16 DMA_TX_EN:1;
			tU16 DMA_RX_EN:1;
			tU16 DMA_SLE:1;
			tU16 LM:1;
			tU16 FON:2;
			tU16 reserved:1;
		}BIT;
		tU16 REG;
	}
    I2C_CR;		// Control Register					Offset 0x00 - 16 bits

    intra16;

	union
	{
		struct
		{
			tU32 SA10:10;
			//tU32 ESA10:3;
			tU32 reserved:6;
			tU32 SLSU:6;
		} BIT;
		tU32 REG;// Status Register 1				- Offset 0x04 -	32 bits
	}I2C_SCR;

		//gap8(3);
	union
	{
		struct
		{
			tU8 MC:3;
			tU8 reserved:5;
		} BIT;
		tU8 REG;
	}I2C_HSMCR;	    // Status Register 2			- Offset 0x08 - 8 bits

		gap8(3);

//	union
//	{
//		struct
//		{
//			tU32 OP:1;
//			tU32 A10:10;
//			//tU32 EA10:3;
//			tU32 SB:1;
//			tU32 AM:2;
//			tU32 P:1;
//			tU32 LENGTH:11;
//			tU32 reserved:6;
//		}BIT;
//		tU32 REG;
//	}I2C_MCR;	    // Clock Control Register			- Offset 0x0C - 32 bits
	tI2C_MCR I2C_MCR;


		tU8 I2C_TFR;	// Own Address Register 1			- Offset 0x10 - 8 bits

		gap8(3);

	union
	{
		struct
		{
			tU32 OP:2;
			tU32 STATUS:2;
			tU32 CAUSE:3;
			tU32 TYPE:2;
			tU32 LENGTH:11;
			tU32 reserved:12;
		}BIT;
		tU32 REG;
	}I2C_SR;    // Own Address Register 2			- Offset 0x14 - 32 bits

		//gap8(3);

		tU8 I2C_RFR;	//	Data Register					- Offset 0x18 - 8 bits

		gap8(3);

	union
	{
		struct
		{
			tU16 THRESHOLD_TX:10;
			tU16 reserved:6;
		}BIT;
		tU16 REG;
	}I2C_TFTR;  	    // Extended Clock Control Register	- Offset 0x1C - 16 bits

		gap8(2);

	union
	{
		struct
		{
			tU16 THRESHOLD_RX:10;
			tU16 reserved:6;
		}BIT;
		tU16 REG;
	}I2C_RFTR;  	    // Extended Clock Control Register	- Offset 0x20 - 8 bits

		gap8(2);

	union
	{
		struct
		{
			tU16 SBSIZE_RX:3;
			tU16 BURST_RX:1;
			tU16 reserved:4;
			tU16 DBSIZE_TX:3;
			tU16 BURST_TX:1;
			tU16 reserved1:4;
		}BIT;
		tU16 REG;
	}I2C_DMAR;  	    // Extended Clock Control Register	- Offset 0x24 - 16 bits

		gap8(2);

	union
	{
		struct
		{
			tU32 BRCNT2:16;
			tU32 BRCNT1:16;
		}BIT;
		tU32 REG;
	}I2C_BRCR;    // Own Address Register 2			- Offset 0x28 - 32 bits

	union
	{
		struct
		{
			tU32 TXFEM:1;
			tU32 TXFNEM:1;
			tU32 TXFFM:1;
			tU32 TXFOVRM:1;
			tU32 RXFEM:1;
			tU32 RXFNFM:1;
			tU32 RXFFM:1;
			tU32 reserved:9;
			tU32 RFSRM:1;
			tU32 RFSEM:1;
			tU32 WTSRM:1;
			tU32 MTDM:1;
			tU32 STDM:1;
			tU32 reserved1:3;
			tU32 MALM:1;
			tU32 BERRM:1;
// ************** AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//
			tU32 reserved2:2;
			tU32 MTDWSM:1;
			tU32 reserved3:3;
// ************** END AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//

		} BIT;
		tU32 REG;
	}I2C_IMSCR;    // Own Address Register 2			- Offset 0x2C - 32 bits

	union
	{
		struct
		{
			tU32 TXFE:1;
			tU32 TXFNE:1;
			tU32 TXFF:1;
			tU32 TXFOVR:1;
			tU32 RXFE:1;
			tU32 RXFNF:1;
			tU32 RXFF:1;
			tU32 reserved:9;
			tU32 RFSR:1;
			tU32 RFSE:1;
			tU32 WTSR:1;
			tU32 MTD:1;
			tU32 STD:1;
			tU32 reserved1:3;
			tU32 MAL:1;
			tU32 BERR:1;
// ************** AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//
			tU32 reserved2:2;
			tU32 MTDWS:1;
			tU32 reserved3:3;
// ************** END AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//
		}BIT;
		tU32 REG;
	}I2C_RISR;    // Own Address Register 2			- Offset 0x30 - 32 bits


	union
	{
		struct
		{
			tU32 TXFEMIS:1;
			tU32 TXFNEMIS:1;
			tU32 TXFFMIS:1;
			tU32 TXFOVRMIS:1;
			tU32 RXFEMIS:1;
			tU32 RXFNFMIS:1;
			tU32 RXFFMIS:1;
			tU32 reserved:9;
			tU32 RFSRMIS:1;
			tU32 RFSEMIS:1;
			tU32 WTSRMIS:1;
			tU32 MTDMIS:1;
			tU32 STDMIS:1;
			tU32 reserved1:3;
			tU32 MALMIS:1;
			tU32 BERRMIS:1;
// ************** AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//
			tU32 reserved2:2;
			tU32 MTDWSMIS:1;
			tU32 reserved3:3;
// ************** END AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//
		} BIT;
		tU32 REG;
	}I2C_MISR;    // Own Address Register 2			- Offset 0x34 - 32 bits

	union
	{
		struct
		{
			tU32 reserved:3;
			tU32 TXFOVRIC:1;
			tU32 reserved1:12;
			tU32 RFSRIC:1;
			tU32 RFSEIC:1;
			tU32 WTSRIC:1;
			tU32 MTDIC:1;
			tU32 STDIC:1;
			tU32 reserved2:3;
			tU32 MALIC:1;
			tU32 BERIC:1;
// ************** AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//
			tU32 reserved3:2;
			tU32 MTDWSIC:1;
			tU32 reserved4:3;
// ************** END AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//
		} BIT;
		tU32 REG;
	}I2C_ICR;    // Own Address Register 2			- Offset 0x38 - 32 bits


// ************** AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//
	gap(8);

	union
	{
		struct
		{
			tU16 THDDAT:9;
			tU16 reserved:7;
		} BIT;
		tU16 REG;
	}I2C_THDDAT; // Own Address Register 2			- Offset 0x4C - 16 bits

	gap(1);

	union
	{
		struct
		{
			tU32 THDSTA_STD:9;
			tU32 reserved0:7;
			tU32 THDSTA_FST:9;
			tU32 reserved1:7;
		} BIT;
		tU32 REG;
	}I2C_THDSTA_FST_STD; // Own Address Register 2			- Offset 0x50 - 32 bits

	union
	{
		struct
		{
			tU32 THDSTA_FMP:9;
			tU32 reserved0:7;
			tU32 THDSTA_HS:9;
			tU32 reserved1:7;
		} BIT;
		tU32 REG;
	}I2C_THDSTA_FMP_HS; // Own Address Register 2			- Offset 0x54 - 32 bits

	union
	{
		struct
		{
			tU32 TSUSTA_STD:9;
			tU32 reserved0:7;
			tU32 TSUSTA_FST:9;
			tU32 reserved1:7;
		} BIT;
		tU32 REG;
	}I2C_TSUSTA_FST_STD; // Own Address Register 2			- Offset 0x58 - 32 bits

	union
	{
		struct
		{
			tU32 TSUSTA_FMP:9;
			tU32 reserved0:7;
			tU32 TSUSTA_HS:9;
			tU32 reserved1:7;
		} BIT;
		tU32 REG;
	}I2C_TSUSTA_FMP_HS; // Own Address Register 2			- Offset 0x5C - 32 bits

	gap(1984);
// ************** END AVAILABLE JUST ON CUT2 ***************************//

   tU8 PhID0;             // 0xFE0
   gap8(3);
   tU8 PhID1;	          // 0xFE4
   gap8(3);
   tU8 PhID2;             // 0xFE8
   gap8(3);
   tU8 PhID3;             // 0xFEC
   gap8(3);
   tU8 CellID0;
   gap8(3);
   tU8 CellID1;
   gap8(3);
   tU8 CellID2;
   gap8(3);
   tU8 CellID3;


} I2cMap;

//------------------------------------------------------------------------
// Direct Memory Access Controller (DMAC)
//------------------------------------------------------------------------
//!
//! \struct LLD_DMA_ChannelControlRegisterTy
//! DMA channel control register structure.
//!
typedef struct
{
    tU32    transferSize:12;					//! Trasfer size
    tU32    sourceBurstSize:3;					//! Source burst size
    tU32    destinationBurstSize:3;				//! Destination burst size
    tU32    sourceWidth:3;						//! Source width
    tU32    destinationWidth:3;					//! Destination width
    tU32    sourceMaster:1;						//! Source master
    tU32    destinationMaster:1;				//! Destination master
    tU32    sourceAddressIncrement:1;			//! Source address increment ('1') or not ('0')
    tU32    destinationAddressIncrement:1;		//! Destination address increment ('1') or not ('0')
    tU32    priviledgeMode:1;					//! Privilege mode
    tU32    bufferable:1;						//! Bufferable
    tU32    cacheable:1;						//! Cacheable
    tU32    terminalCounterInterruptEnabled:1;  //! Terminal counter interrupt enabled
}
LLD_DMA_ChannelControlRegisterTy;

// DMA linked list structure
typedef struct
{
    tU32                                sourceAddress;
    tU32                                destinationAddress;
    struct LLD_DMA_LinkedListTy         *nextLinkedList;
    LLD_DMA_ChannelControlRegisterTy    channelControlRegister;
}
LLD_DMA_LinkedListTy;

// DMA channel configuration register structure
typedef struct
{
    tU32 enabled:1;
    tU32 sourcePeripheral:5;
    tU32 destinationPeripheral:5;
    tU32 flowControl:3;
    tU32 interruptErrorMaskEnable:1;
    tU32 terminalCountMaskEnable:1;
    tU32 lockedTransfer:1;
    tU32 channelActive:1;
    tU32 halt:1;
    tU32 reserved2:13;
}
LLD_DMA_ChannelConfigurationRegisterTy;



typedef struct
{
    tU32 LM:1;
    tU32 R:1;
    tU32 LLI:30;
} DMACCLLITy;

typedef struct
{
    tU32                 DMACCSrcAddr;
    tU32                 DMACCDestAddr;
    DMACCLLITy           DMACCLLI;
    LLD_DMA_ChannelControlRegisterTy         DMACCControl;
    LLD_DMA_ChannelConfigurationRegisterTy   DMACCConfiguration;
    tU32                 DMAEmptySpace_2[3];

}DMACChannelTy;

// WORD(32) access
typedef volatile struct
{
    tU32 DMACIntStatus;                    // offset = 0x000
    tU32 DMACIntTCStatus;                  // offset = 0x004
    tU32 DMACIntTCClear;                   // offset = 0x008
    tU32 DMACIntErrorStatus;               // offset = 0x00C
    tU32 DMACIntErrClr;                    // offset = 0x010
    tU32 DMACIRawIntTCStatus;              // offset = 0x014
    tU32 DMACRawIntErrorStatus;            // offset = 0x018
    tU32 DMACEnbldChns;                    // offset = 0x01C
    tU32 DMACSoftBReq;                     // offset = 0x020
    tU32 DMACSoftSReq;                     // offset = 0x024
    tU32 DMACSoftLBReq;                    // offset = 0x028
    tU32 DMACSoftLSReq;                    // offset = 0x02C
    tU32 DMAEmptySpace_1[52];
    DMACChannelTy DMACChannelReg[8];       // offset = 0x100
} DmaMap;

//------------------------------------------------------------------------
// Vectored interrupt controller (VIC)
//------------------------------------------------------------------------
typedef struct
{
    // addr offset = 0x000 + y*0x20 (y=0,1)
    tU32 VIC_IRQSR;             // (r-) IRQ Interrupt status register
    // addr offset = 0x004 + y*0x20 (y=0,1)
    tU32 VIC_FIQSR;             // (r-) FIQ Interrupt status register
    // addr offset = 0x008 + y*0x20 (y=0,1)
    tU32 VIC_RIS;               // (r-) Raw Interrupt status register
    // addr offset = 0x00C + y*0x20 (y=0,1)
    tU32 VIC_ISEL;              // (rw) Interrupt selection register
    // addr offset = 0x010 + y*0x20 (y=0,1)
    tU32 VIC_IENS;              // (rw) Interrupt enable set register
    // addr offset = 0x014 + y*0x20 (y=0,1)
    tU32 VIC_IENC;              // (rw) Interrupt enable clear register
    // addr offset = 0x018 + y*0x20 (y=0,1)
    tU32 VIC_SWISR;             // (rw) Software interrupt set register
    // addr offset = 0x01C + y*0x20 (y=0,1)
    tU32 VIC_SWICR;             // (rw) Software interrupt clear register
}VicRegTy;

// addr offset = 0x200 + y*0x4 (y=0,1)
typedef struct
{
    tU32 IS:6;				    // (rw) Interrupt source
    tU32 E:1;                   // (rw) Interrupt enable
    tU32 reserved:25;           // (r-) Reserved
}VicVcrTy;

typedef volatile struct
{
    // addr offset = 0x000 + y*0x20 (y=0,1)
    VicRegTy VIC_COMMON[2];     // see VicRegTy structure

    // addr offset = 0x040
    struct
    {
        tU32 PROT:1;            // (rw) Protection
        tU32 reserved:31;       // (r-) Reserved
    }VIC_PER;

    gap32(3);

    // addr offset = 0x050
    tU32 VIC_CVAR;              // (rw) ISR Current Vector Address Register

    // addr offset = 0x054
    tU32 VIC_DVAR;              // (rw) ISR Default Vector Address Register

    gap32(42);

    // addr offset = 0x100 + y*4 (y=0..15)
    tU32 VIC_VAR[16];           // (rw) ISR vector address register

    gap32(48);

    // addr offset = 0x200 + y*4 (y=0..15)
    VicVcrTy VIC_VCR[16];       // (rw) Vectored interrupt control register

} VicMap;

//------------------------------------------------------------------------
// Serial Synchronous Port (SSP)
//------------------------------------------------------------------------
typedef volatile struct
{   // addr offset = 0x000
  union
  {
     struct
     {
        tU32 DSS:5;       // (rw) Data size select
        tU32 HALFDUP:1;   // (rw) Frame format
        tU32 SPO:1;       // (rw) SSPCLKOUT polarity
        tU32 SPH:1;       // (rw) SSPCLKOUT phase
        tU32 SCR:8;       // (rw) Serial clock rate
        tU32 CSS:5;
        tU32 FRF:2;
        tU32 reserved:9;
     }BIT;
     tU32 REG;
  } SSPCR0;

    // addr offset = 0x004
  union
  {
     struct
     {
        tU32 LBM:1;       // (rw) Loop back mode select
        tU32 SSE:1;       // (rw) Synchronous serial port enable
        tU32 MS:1;        // (rw) Master or slave mode select
        tU32 SOD:1;       // (rw) Slave-mode output disable
        tU32 RENDN:1;
        tU32 TENDN:1;
        tU32 MWAIT:1;
        tU32 RXIFLSEL:3;
        tU32 TXIFLSEL:3;
        tU32 reserved:18; // Read unpredictable,should be written as 0
     }BIT;
     tU32 REG;
  } SSPCR1;

    // addr offset = 0x008
  union
  {
     tU32 REG;
  } SSPDR;          // (rw) SSP Data Register

    // addr offset = 0x00C
  union
  {
     struct
     {
        tU32 TFE:1;      // (r-) Transmit FIFO empty
        tU32 TNF:1;      // (r-) Transmit FIFO not full
        tU32 RNE:1;      // (r-) Receive FIFO not empty
        tU32 RFF:1;      // (r-) Receive FIFO full
        tU32 BSY:1;      // (r-) PrimeCell SSP busy flag
        tU32 reserved:27;// Read unpredictable, should be written as 0
     }BIT;
     tU32 REG;
  } SSPSR;

    // addr offset = 0x010
  union
  {
     struct
     {
        tU32 CPSDVSR:8;  // (rw) Clock prescale divisor
        tU32 reserved:24;// Read unpredictable, should be written as 0
     }BIT;
     tU32 REG;
  } SSPCPSR;

    // addr offset = 0x014
  union
  {
     struct
     {
        tU32 RORIM:1;    // (rw) Receive overrun interrupt mask
        tU32 RTIM:1;     // (rw) Receive timeout interrupt mask
        tU32 RXIM:1;     // (rw) Receive FIFO interrupt mask
        tU32 TXIM:1;     // (rw) Transmit FIFO interrupt mask
        tU32 reserved:28;// Read as zero, do not modify
     }BIT;
     tU32 REG;
  } SSPIMSC;

   // addr offset = 0x018
  union
  {
     struct
     {
        tU32 RORRIS:1;   // (r-) Gives the raw interrupt state (prior to masking) of the SSPRORINTR interrupt
        tU32 RTRIS:1;    // (r-) Gives the raw interrupt state (prior to masking) of the SSPRTINTR interrupt
        tU32 RXRIS:1;    // (r-) Gives the raw interrupt state (prior to masking) of the SSPRXINTR interrupt
        tU32 TXRIS:1;    // (r-) Gives the raw interrupt state (prior to masking) of the SSPTXINTR interrupt
        tU32 reserved:28;// Read as zero, do not modify
     }BIT;
     tU32 REG;
  } SSPRIS;

   // addr offset = 0x01C
  union
  {
     struct
     {
        tU32 RORMIS:1;   // (r-) Gives the receive overrun masked interrupt status (after masking) of the SSPRORINTR interrupt
        tU32 RTMIS:1;    // (r-) Gives the receive timeout masked interrupt status (after masking) of the SSPRTINTR interrupt
        tU32 RXMIS:1;    // (r-) Gives the receive FIFO masked interrupt status (after masking) of the SSPRXINTR interrupt
        tU32 TXMIS:1;    // (r-) Gives the transmit FIFO masked interrupt status (after masking) of the SSPTXINTR interrupt
        tU32 reserved:28;// Read as zero, do not modify
     }BIT;
     tU32 REG;
  } SSPMIS;

   // addr offset = 0x020
  union
  {
     struct
     {
        tU32 RORIC:1;    // (-w) Clears the SSPRORINTR interrupt
        tU32 RTIC:1;     // (-w) Clears the SSPRTINTR interrupt
        tU32 reserved:30;// Read as zero, do not modify
     }BIT;
     tU32 REG;
  } SSPICR;

   // addr offset = 0x024
  union
  {
     struct
     {
        tU32 RXDMAE:1;   // (rw) If this bit is set to 1, DMA for receive FIFO is enabled
        tU32 TXDMAE:1;   // (rw) If this bit is set to 1, DMA for transmit FIFO is enabled
        tU32 reserved:30;// Read as zero, do not modify
     }BIT;
     tU32 REG;
  } SSPDMACR;

	tU32 reserved[(0xFE0-0x028)>>2];	//  Reserved  									//0x090
	tU32 ssp_periphid0;					//  SSP Peripheral Id Register bits 7:0  		//0xFE0
	tU32 ssp_periphid1;					//  SSP Peripheral Id Register bits 15:8  		//0xFE4
	tU32 ssp_periphid2;					//  SSP Peripheral Id Register bits 23:16 		//0xFE8
	tU32 ssp_periphid3;					//  SSP Peripheral Id Register bits 31:24 		//0xFEC
	tU32 ssp_pcellid0;					//  PrimeCell Id Register bits 7:0  			//0xFF0
	tU32 ssp_pcellid1;					//  PrimeCell id register: bits 15:8  			//0xFF4
	tU32 ssp_pcellid2;					//  PrimeCell id register: bits 23:16   		//0xFF8
	tU32 ssp_pcellid3;					//  PrimeCell id register: bits 31:24  			//0xFFC

} SspMap;


//------------------------------------------------------------------------
// Serial Synchronous Port (MSP)
//------------------------------------------------------------------------
typedef volatile struct
{

	tU32 MSP_DR;			// addr offset = 0x000

	union
	{
		struct
		{
			tU32 RXEN:1;
			tU32 RFFEN:1;
			tU32 RFSPOL:1;
			tU32 DCM:1;
			tU32 RFSSEL:1;
			tU32 RCKPOL:1;
			tU32 RCKSEL:1;
			tU32 LBM:1;
			tU32 TXEN:1;
			tU32 TFFEN:1;
			tU32 TFSPOL:1;
			tU32 TFSSEL:2;
			tU32 TCKPOL:1;
			tU32 TCKSEL:1;
			tU32 TXDDL:1;
			tU32 SGEN:1;
			tU32 SCKPOL:1;
			tU32 SCKSEL:2;
			tU32 FGEN:1;
			tU32 SPICKML:2;
			tU32 SPIBME:1;
			tU32 reserved:8;
		} BIT;
		tU32 REG;
	}MSP_GCR;    // Own Address Register 			- Offset 0x04 - 32 bits

	union
	{
		struct
		{
			tU32 TP1ELEN:3;
			tU32 TP1FLEN:7;
			tU32 TDTYP:2;
			tU32 TENDN:1;
			tU32 TDDLY:2;
			tU32 TFSIG:1;
			tU32 TP2ELEN:3;
			tU32 TP2FLEN:7;
			tU32 TP2SM:1;
			tU32 TP2EN:1;
			tU32 TBSWAP:2;
			tU32 reserved:2;
		} BIT;
		tU32 REG;
	}MSP_TCF;    // Own Address Register 			- Offset 0x08 - 32 bits

	union
	{
		struct
		{
			tU32 RP1ELEN:3;
			tU32 RP1FLEN:7;
			tU32 RDTYP:2;
			tU32 RENDN:1;
			tU32 RDDLY:2;
			tU32 RFSIG:1;
			tU32 RP2ELEN:3;
			tU32 RP2FLEN:7;
			tU32 RP2SM:1;
			tU32 RP2EN:1;
			tU32 RBSWAP:2;
			tU32 reserved:2;
		} BIT;
		tU32 REG;
	}MSP_RCF;    // Own Address Register 			- Offset 0x0C - 32 bits

	union
	{
		struct
		{
			tU32 SCKDIV:10;
			tU32 FRWID:6;
			tU32 FRPER:13;
			tU32 reserved:3;
		} BIT;
		tU32 REG;
	}MSP_SRG;    // Own Address Register 			- Offset 0x10 - 32 bits

	union
	{
		struct
		{
			tU32 RBUSY:1;
			tU32 RFE:1;
			tU32 RFU:1;
			tU32 TBUSY:1;
			tU32 TFE:1;
			tU32 TFU:1;
			tU32 reserved:26;
		} BIT;
		tU32 REG;
	}MSP_FLR;    // Own Address Register 			- Offset 0x14 - 32 bits

	union
	{
		struct
		{
			tU32 RDMAE:1;
			tU32 TDMAE:1;
			tU32 reserved:30;
		} BIT;
		tU32 REG;
	}MSP_DMACR;  //						 			- Offset 0x18 - 32 bits

	gap32(1);

	union
	{
		struct
		{
			tU32 RXIM:1;
			tU32 ROEIM:1;
			tU32 RSEIM:1;
			tU32 RFSIM:1;
			tU32 TXIM:1;
			tU32 TUEIM:1;
			tU32 TSEIM:1;
			tU32 TFSIM:1;
			tU32 RFOIM:1;
			tU32 TFOIM:1;
			tU32 reserved:22;
		} BIT;
		tU32 REG;
	}MSP_IMSC; //						 			- Offset 0x20 - 32 bits

	union
	{
		struct
		{
			tU32 RXRIS:1;
			tU32 ROERIS:1;
			tU32 RSERIS:1;
			tU32 RFSRIS:1;
			tU32 TXRIS:1;
			tU32 TUERIS:1;
			tU32 TSERIS:1;
			tU32 TFSRIS:1;
			tU32 RFORIS:1;
			tU32 TFORIS:1;
			tU32 reserved:22;
		} BIT;
		tU32 REG;
	}MSP_RIS; //						 			- Offset 0x24 - 32 bits

	union
	{
		struct
		{
			tU32 RXMIS:1;
			tU32 ROEMIS:1;
			tU32 RSEMIS:1;
			tU32 RFSMIS:1;
			tU32 TXMIS:1;
			tU32 TUEMIS:1;
			tU32 TSEMIS:1;
			tU32 TFSMIS:1;
			tU32 RFOMIS:1;
			tU32 TFOMIS:1;
			tU32 reserved:22;
		} BIT;
		tU32 REG;
	}MSP_MIS; //						 			- Offset 0x28 - 32 bits

	union
	{
		struct
		{
			tU32 RXIC:1;
			tU32 ROEIC:1;
			tU32 RSEIC:1;
			tU32 RFSIC:1;
			tU32 TXIC:1;
			tU32 TUEIC:1;
			tU32 TSEIC:1;
			tU32 TFSIC:1;
			tU32 RFOIC:1;
			tU32 TFOIC:1;
			tU32 reserved:22;
		} BIT;
		tU32 REG;
	}MSP_ICR; //						 			- Offset 0x2C - 32 bits

	union
	{
		struct
		{
			tU32 RMCEN:1;
			tU32 RMCSF:2;
			tU32 RMCMP:2;
			tU32 TMCEN:1;
			tU32 TMCSF:2;
			tU32 reserved:24;
		} BIT;
		tU32 REG;
	}MSP_MCR; //						 			- Offset 0x30 - 32 bits

	tU32 MSP_RCV; //						 			- Offset 0x34 - 32 bits

	tU32 MSP_RCM; //						 			- Offset 0x38 - 32 bits

	tU32 MSP_TCE[4]; //						 			- Offset 0x40-0x44-0x48-0x4C - 32 bits

	gap32(4);

	tU32 MSP_RCE[4]; //						 			- Offset 0x60-0x64-0x68-0x6C - 32 bits

	gap32(4);
// these register are in the HCL but not in the manual
	tU32 MSP_TSTCR;
	tU32 MSP_ITIP;
	tU32 MSP_ITOP;
	tU32 MSP_TSTDR;
// end
	gap32(980);

	tU8 MSP_PeriphID0;
	gap8(3);

	union
	{
		struct
		{
		tU8 PartNumber1:4;
		tU8 Designer0:4;
		} BIT;
		tU8 REG;
	}MSP_PeriphID1; //

	gap8(3);

	union
	{
		struct
		{
		tU8 Designer1:4;
		tU8 Revision:4;
		} BIT;
		tU8 REG;
	}MSP_PeriphID2; //

	gap8(3);

	tU8 MSP_PeriphID3;
	gap8(3);

	tU8 MSP_PCellID0;
	gap8(3);
	tU8 MSP_PCellID1;
	gap8(3);
	tU8 MSP_PCellID2;
	gap8(3);
	tU8 MSP_PCellID3;
	gap8(3);


} MspMap;

//------------------------------------------------------------------------
// Multi Timer Unit (MTU)
//------------------------------------------------------------------------
typedef struct
{
    // addr offset = 0x010 + y*0x10 (y=0..3)
    tU32 MTU_TLR;               // (rw) Timer x Load Register

    // addr offset = 0x014 + y*0x10 (y=0..3)
    tU32 MTU_TVAL;              // (rw) Timer x Value Register

    // addr offset = 0x018 + y*0x10 (y=0..3)
    struct
    {
        tU32 TOS:1;             // (rw) Timer One Shot count
        tU32 TSZ:1;             // (rw) Timer Size
        tU32 TPRE:2;            // (rw) Timer Prescaler
        tU32 reserved:2;        // (r-) Reserved
        tU32 TMOD:1;            // (rw) Timer Mode
        tU32 TEN:1;				// (rw) Timer Enable
        tU32 reserved1:24;		// (r-) Reserved
    }MTU_TCR;                   // (rw) Timer x Control Register

    // addr offset = 0x01C + y*0x10 (y=0..3)
    tU32 MTU_TBGLR;             // (rw) Timer Background Load value

}MtuRegTy;

typedef volatile struct
{
    // addr offset = 0x000
    struct
    {
        tU32 T0IM:1;            // (rw) Timer 0 interrupt mask
        tU32 T1IM:1;			// (rw) Timer 1 interrupt mask
        tU32 T2IM:1;            // (rw) Timer 2 interrupt mask
        tU32 T3IM:1;            // (rw) Timer 3 interrupt mask
        tU32 reserved:28;
    }MTU_IMSC;

    // addr offset = 0x004
    struct
    {
        tU32 T0RIS:1;			// (r-) Timer 0 raw interrupt status bit
        tU32 T1RIS:1;			// (r-) Timer 1 raw interrupt status bit
        tU32 T2RIS:1;			// (r-) Timer 2 raw interrupt status bit
        tU32 T3RIS:1;			// (r-) Timer 3 raw interrupt status bit
        tU32 reserved:28;
    }MTU_RIS;

    // addr offset = 0x008
    struct
    {
        tU32 T0MIS:1;			// (r-) Timer 0 masked interrupt status bit
        tU32 T1MIS:1;			// (r-) Timer 1 masked interrupt status bit
        tU32 T2MIS:1;           // (r-) Timer 2 masked interrupt status bit
        tU32 T3MIS:1;           // (r-) Timer 3 masked interrupt status bit
        tU32 reserved:28;
    }MTU_MIS;

    // addr offset = 0x00C
    struct
    {
        tU32 T0IC:1;			// (rw) Timer 0 interrupt clear
        tU32 T1IC:1;			// (rw) Timer 1 interrupt clear
        tU32 T2IC:1;            // (rw) Timer 2 interrupt clear
        tU32 T3IC:1;            // (rw) Timer 3 interrupt clear
        tU32 reserved:28;
    }MTU_ICR;


    // addr offset = 0x010 + y*0x10 (y=0..3)
    MtuRegTy MTU_COMMON[4];       // see Mtu_CommonTy structure
} MtuMap;

#endif // _REGISTERS_STA2062_H_

// End of file registers_sta2062.h
