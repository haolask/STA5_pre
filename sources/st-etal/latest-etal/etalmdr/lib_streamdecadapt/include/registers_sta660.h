// _____________________________________________________________________________
//| FILE:         registers_sta660.h
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
//| 2009.10.12  | Added MTU memory map       | Luigi Cotignano
//| 2009.10.14  | Added SPI memory map       | Luigi Cotignano
//|_____________________________________________________________________________


#ifndef _REGISTERS_STA660_H_
#define _REGISTERS_STA660_H_

//------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------
#include "types.h"
#include "macro.h"
#include "mapping_sta660.h"

//------------------------------------------------------------------------
// General Purpose IOs
//------------------------------------------------------------------------
typedef struct
{
    tU8 Data;
    gap8(3);
} GPIO_DATATy;

typedef volatile struct
{
    // address offset = 0x000
    GPIO_DATATy GPIODATA[256];  // (rw) Data register
    // address offset = 0x400
    tU8 GPIODIR;                // (rw) Data Direction register

    gap8(3);

    // address offset = 0x404
    tU8 GPIOIS;                 // (rw) Interrupt sense register

    gap8(3);

    // address offset = 0x408
    tU8 GPIOIBE;                // (rw) Interrupt both edges selection register

    gap8(3);

    // address offset = 0x40C
    tU8 GPIOIEV;                // (rw) Interrupt event register

    gap8(3);

    // address offset = 0x410
    tU8 GPIOIE;                 // (rw) Interrupt mask register

    gap8(3);

    // address offset = 0x414
    tU8 GPIORIS;                // (rw) Raw interrupt status register

    gap8(3);

    // address offset = 0x418
    tU8 GPIOMIS;                // (rw) Masked interrupt status register

    gap8(3);

    // address offset = 0x41C
    tU8 GPIOIC;                 // (rw) Interrupt clear register

    gap8(3);

    // address offset = 0x420
    tU8 GPIOAFSEL;              // (rw) Mode control select register
} GpioMap;

//------------------------------------------------------------------------
// Universal asynchronous Receiver and Transmitter (UART)
//------------------------------------------------------------------------
typedef volatile struct
{
    struct                              // addr offset = 0x00
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

    struct                              // addr offset = 0x08
    {
        tU8 TXDMAWM:3;
        tU8 RXDMAWM:3;
        tU8 reserved:2;
    }UARTDMAWM;
    intra8;
    gap(1);

    struct                              // addr offset = 0x0C
    {
        tU32 TIMEOUT:22;
        tU32 reserved:10;
    }UARTTIMEOUT;

    gap(4);                           // addr offset = 0x10, 0x14

// ************** END AVAILABLE JUST ON CUT2 CARTESIO+ ***************************//

    struct                              // addr offset = 0x18
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

    struct                              // addr offset = 0x1C
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



    tU8 UARTILPR;                       // addr offset = 0x20

    intra8;
    intra16;

    tU16 UARTIBRD;                      // addr offset = 0x24

    intra16;

    tU8 UARTFBRD:6;                     // addr offset = 0x28

    intra8;
    intra16;

    struct                              // addr offset = 0x2C
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

    struct                              // addr offset = 0x30
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

    struct                              // addr offset = 0x34
    {
        tU16 TXIFLSEL:3;
        tU16 RXIFLSEL:3;
        tU16 reserved:10;
    }UARTIFLS;

    intra16;

    struct                              // addr offset = 0x38
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

    struct                              // addr offset = 0x3C
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

    struct                              // addr offset = 0x40
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

    struct                              // addr offset = 0x44
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

    struct                              // addr offset = 0x48
    {
        tU16 RXDMAE:1;
        tU16 TXDMAE:1;
        tU16 DMAONERR:1;
        tU16 reserved:13;
    }UARTDMACR;

    gap(1995);

    struct                              // addr offset = 0xFE0
    {
        tU16 PartNumber0:8;
        tU16 reserved:8;
    }UARTPeriphID0;

    intra16;

    struct                              // addr offset = 0xFE4
    {
        tU16 PartNumber1:4;
        tU16 Designer0:4;
        tU16 reserved:8;
    }UARTPeriphID1;

    intra16;

    struct                              // addr offset = 0xFE8
    {
        tU16 Designer1:4;
        tU16 Revision:4;
        tU16 reserved:8;
    }UARTPeriphID2;

    intra16;

    struct                              // addr offset = 0xFEC
    {
        tU16 Configuration:8;
        tU16 reserved:8;
    }UARTPeriphID3;

    intra16;

    struct                              // addr offset = 0xFF0
    {
        tU16 UARTPCellID0:8;
        tU16 reserved:8;
    }UARTPCellID0;

    intra16;

    struct                              // addr offset = 0xFF4
    {
        tU16 UARTPCellID1:8;
        tU16 reserved:8;
    }UARTPCellID1;

    intra16;


    struct                              // addr offset = 0xFF8
    {
        tU16 UARTPCellID2:8;
        tU16 reserved:8;
    }UARTPCellID2;

    intra16;


    struct                              // addr offset = 0xFFC
    {
        tU16 UARTPCellID3:8;
        tU16 reserved:8;
    }UARTPCellID3;

    intra16;
}UartMap;

//------------------------------------------------------------------------
// I2C (0..2)
//------------------------------------------------------------------------
typedef volatile struct
{
    struct
    {
        tU8 ite:1;
        tU8 stop:1;
        tU8 ack:1;
        tU8 start:1;
        tU8 ddc2ben:1;
        tU8 pe:1;
        tU8 trans:1;
        tU8 ddc1en:1;
    } cr;       // Control Register                 - Offset 0x00 - 8 bits

    gap8(3);

    struct
    {
        tU8 sb:1;
        tU8 msl:1;
        tU8 adsl:1;
        tU8 btf:1;
        tU8 busy:1;
        tU8 tra:1;
        tU8 add10:1;
        tU8 evf:1;
    } sr1;      // Status Register 1                - Offset 0x04 - 8 bits

    gap8(3);

    struct
    {
        tU8 ddc2b:1;
        tU8 berr:1;
        tU8 arlo:1;
        tU8 stopf:1;
        tU8 af:1;
        tU8 endad:1;
        tU8 res:1;
        tU8 sclfal:1;
    } sr2;      // Status Register 2                - Offset 0x08 - 8 bits

    gap8(3);

    struct
    {
        tU8 clockDivL:  7;
        tU8 fmsm:       1;
    } ccr;      // Clock Control Register           - Offset 0x0C - 8 bits

    gap8(3);

    tU8 oar1;   // Own Address Register 1           - Offset 0x10 - 8 bits

    gap8(3);

    struct
    {
        tU8 res0:1;
        tU8 addressH:2;
        tU8 res1:2;
        tU8 fr:3;
    } oar2;     // Own Address Register 2           - Offset 0x14 - 8 bits

    gap8(3);

    tU8 data;   //  Data Register                   - Offset 0x18 - 8 bits

    gap8(3);

    struct
    {
        tU8 clockDivH:5;
        tU8 res:3;
    } eccr;     // Extended Clock Control Register  - Offset 0x1C - 8 bits

    gap8(3);
} I2cMap;



#if (LLD_DMA_STA660_ROM_USED == TRUE)
//------------------------------------------------------------------------
// Direct Memory Access Controller (DMAC)
//------------------------------------------------------------------------
typedef struct
{
    tU32 TransferSize:12;
    tU32 SBSize:3;
    tU32 DBSize:3;
    tU32 SWidth:3;
    tU32 DWidth:3;
    tU32 S:1;
    tU32 D:1;
    tU32 SI:1;
    tU32 DI:1;
    tU32 priviledgeMode:1;
    tU32 bufferable:1;
    tU32 cacheable:1;
    tU32 I:1;
} DMACCControlTy;
#else
//------------------------------------------------------------------------
//!
//! \struct LLD_DMA_ChannelControlRegisterTy
//! DMA channel control register structure.
//!
typedef struct
{
    tU32    transferSize:12;                    //! Trasfer size
    tU32    sourceBurstSize:3;                  //! Source burst size
    tU32    destinationBurstSize:3;             //! Destination burst size
    tU32    sourceWidth:3;                      //! Source width
    tU32    destinationWidth:3;                 //! Destination width
    tU32    sourceMaster:1;                     //! Source master
    tU32    destinationMaster:1;                //! Destination master
    tU32    sourceAddressIncrement:1;           //! Source address increment ('1') or not ('0')
    tU32    destinationAddressIncrement:1;      //! Destination address increment ('1') or not ('0')
    tU32    priviledgeMode:1;                   //! Privilege mode
    tU32    bufferable:1;                       //! Bufferable
    tU32    cacheable:1;                        //! Cacheable
    tU32    terminalCounterInterruptEnabled:1;  //! Terminal counter interrupt enabled
}
LLD_DMA_ChannelControlRegisterTy;
#endif

#if (LLD_DMA_STA660_ROM_USED == TRUE)
// DMA channel configuration register structure
typedef struct
{
    tU32 E:1;
    tU32 SrcPeripheral:5;
    tU32 DestPeripheral:5;
    tU32 FlowCntrl:3;
    tU32 IE:1;
    tU32 ITC:1;
    tU32 L:1;
    tU32 A:1;
    tU32 H:1;
    tU32 Reserved2:13;
} DMACCConfigurationTy;
#else
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
#endif

typedef struct
{
    tU32 LM:1;
    tU32 R:1;
    tU32 LLI:30;
} DMACCLLITy;


#if (LLD_DMA_STA660_ROM_USED == TRUE)
typedef struct
{
    tU32                 DMACCSrcAddr;
    tU32                 DMACCDestAddr;
    DMACCLLITy           DMACCLLI;
    DMACCControlTy       DMACCControl;
    DMACCConfigurationTy DMACCConfiguration;
    tU32                 DMAEmptySpace_2[3];
}DMACChannelTy;
#else
typedef struct
{
    tU32                 DMACCSrcAddr;
    tU32                 DMACCDestAddr;
    DMACCLLITy           DMACCLLI;
    LLD_DMA_ChannelControlRegisterTy         DMACCControl;
    LLD_DMA_ChannelConfigurationRegisterTy   DMACCConfiguration;
    tU32                 DMAEmptySpace_2[3];

}DMACChannelTy;
#endif

// DMA linked list structure
typedef struct
{
    tU32                                sourceAddress;
    tU32                                destinationAddress;
    struct LLD_DMA_LinkedListTy         *nextLinkedList;
    LLD_DMA_ChannelControlRegisterTy    channelControlRegister;
}
LLD_DMA_LinkedListTy;


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
    tU32 IS:6;                  // (rw) Interrupt source
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

    tU32 reserved[(0xFE0-0x028)>>2];    //  Reserved                                    //0x090
    tU32 ssp_periphid0;                 //  SSP Peripheral Id Register bits 7:0         //0xFE0
    tU32 ssp_periphid1;                 //  SSP Peripheral Id Register bits 15:8        //0xFE4
    tU32 ssp_periphid2;                 //  SSP Peripheral Id Register bits 23:16       //0xFE8
    tU32 ssp_periphid3;                 //  SSP Peripheral Id Register bits 31:24       //0xFEC
    tU32 ssp_pcellid0;                  //  PrimeCell Id Register bits 7:0              //0xFF0
    tU32 ssp_pcellid1;                  //  PrimeCell id register: bits 15:8            //0xFF4
    tU32 ssp_pcellid2;                  //  PrimeCell id register: bits 23:16           //0xFF8
    tU32 ssp_pcellid3;                  //  PrimeCell id register: bits 31:24           //0xFFC

} SspMap;





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
        tU32 TEN:1;             // (rw) Timer Enable
        tU32 reserved1:24;      // (r-) Reserved
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
        tU32 T1IM:1;            // (rw) Timer 1 interrupt mask
        tU32 T2IM:1;            // (rw) Timer 2 interrupt mask
        tU32 T3IM:1;            // (rw) Timer 3 interrupt mask
        tU32 reserved:28;
    }MTU_IMSC;

    // addr offset = 0x004
    struct
    {
        tU32 T0RIS:1;           // (r-) Timer 0 raw interrupt status bit
        tU32 T1RIS:1;           // (r-) Timer 1 raw interrupt status bit
        tU32 T2RIS:1;           // (r-) Timer 2 raw interrupt status bit
        tU32 T3RIS:1;           // (r-) Timer 3 raw interrupt status bit
        tU32 reserved:28;
    }MTU_RIS;

    // addr offset = 0x008
    struct
    {
        tU32 T0MIS:1;           // (r-) Timer 0 masked interrupt status bit
        tU32 T1MIS:1;           // (r-) Timer 1 masked interrupt status bit
        tU32 T2MIS:1;           // (r-) Timer 2 masked interrupt status bit
        tU32 T3MIS:1;           // (r-) Timer 3 masked interrupt status bit
        tU32 reserved:28;
    }MTU_MIS;

    // addr offset = 0x00C
    struct
    {
        tU32 T0IC:1;            // (rw) Timer 0 interrupt clear
        tU32 T1IC:1;            // (rw) Timer 1 interrupt clear
        tU32 T2IC:1;            // (rw) Timer 2 interrupt clear
        tU32 T3IC:1;            // (rw) Timer 3 interrupt clear
        tU32 reserved:28;
    }MTU_ICR;


    // addr offset = 0x010 + y*0x10 (y=0..3)
    MtuRegTy MTU_COMMON[4];       // see Mtu_CommonTy structure
} MtuMap;

//------------------------------------------------------------------------
// SPI (SSI Synopsys)
//------------------------------------------------------------------------
typedef volatile struct
{
    // addr offset 0x0000
    struct
    {
        tU32 DFS:4;             // (rw) Data Frame Size
        tU32 FRF:2;             // (rw) Frame Format
        tU32 SCPH:1;            // (rw) Serial Clock Phase
        tU32 SCPOL:1;           // (rw) Serial Clock Polarity
        tU32 TMOD:2;            // (rw) Transfer Mode
        tU32 SLV_OE:1;          // (rw) Slave Output Enable
        tU32 SRL:1;             // (rw) Shift Register Loop
        tU32 CFS:4;             // (rw) Control Frame Size
        tU32 reserved:16;       // Reserved
    } CTRLR0;

    // addr offset 0x0004
    struct
    {
        tU32 NDF:16;            // (rw) Number of Data Frames
        tU32 reserved:16;       // Reserved
    } CTRLR1;

    // addr offset 0x0008
    struct
    {
        tU32 SSI_EN:1;          // (rw) SSI Enable
        tU32 reserved:31;       // Reserved
    } SSIENR;

    // addr offset 0x000C
    struct
    {
        tU32 MWMOD:1;           // (rw) Microwire Transfer Mode
        tU32 MDD:1;             // (rw) Microwire Control
        tU32 MHS:1;             // (rw) Microwire Handshaking
        tU32 reserved:29;       // Reserved
    } MWCR;

    // addr offset 0x0010
    struct
    {
        tU32 SER:1;             // (rw) Slave Select Enable Flag
        tU32 reserved:31;       // Reserved
    } SER;

    // addr offset 0x0014
    struct
    {
        tU32 SCKDV:16;          // (rw) SSI Clock Divider
        tU32 reserved:16;       // Reserved
    } BAUDR;

    // addr offset 0x0018
    struct
    {
        tU32 TFT:5;             // (rw) Transmit FIFO Threshold
        tU32 reserved:27;       // Reserved
    } TXFTLR;

   // addr offset 0x001C
    struct
    {
        tU32 RFT:5;             // (rw) Receive FIFO Threshold
        tU32 reserved:27;       // Reserved
    } RXFTLR;

   // addr offset 0x0020
    struct
    {
        tU32 TXFL:5;            // (rw) Transmit FIFO Current Level
        tU32 reserved:27;       // Reserved
    } TXFLR;

   // addr offset 0x0024
    struct
    {
        tU32 RXFL:5;            // (rw) Receive FIFO Current Level
        tU32 reserved:27;       // Reserved
    } RXFLR;

   // addr offset 0x0028
    struct
    {
        tU32 BUSY:1;            // (r-) Busy flag
        tU32 TFNF:1;            // (r-) Transmit FIFO Not Full
        tU32 TFE:1;             // (r-) Transmit FIFO Empty
        tU32 RFNE:1;            // (r-) Receive FIFO Not Empty
        tU32 RFF:1;             // (r-) Receive FIFO Full
        tU32 TXE:1;             // (r-) Transmission Error
        tU32 DCOL:1;            // (r-) Data Collision Error
        tU32 reserved:25;       // Reserved
    } SR;

   // addr offset 0x002C
    struct
    {
        tU32 TXEIM:1;            // (rw) Transmit FIFO Empty Interrupt Mask
        tU32 TXOIM:1;            // (rw) Transmit FIFO Overflow Interrupt Mask
        tU32 RXUIM:1;            // (rw) Receive FIFO Underflow Interrupt Mask
        tU32 RXOIM:1;            // (rw) Receive FIFO Overflow Interrupt Mask
        tU32 RXFIM:1;            // (rw) Receive FIFO Full Interrupt Mask
        tU32 MSTIM:1;            // (rw) Multi-Master Contention Interrupt Mask
        tU32 reserved:26;        // Reserved
    } IMR;

   // addr offset 0x0030
    struct
    {
        tU32 TXEIS:1;            // (r-) Transmit FIFO Empty Interrupt Status
        tU32 TXOIS:1;            // (r-) Transmit FIFO Overflow Interrupt Status
        tU32 RXUIS:1;            // (r-) Receive FIFO Underflow Interrupt Status
        tU32 RXOIS:1;            // (r-) Receive FIFO Overflow Interrupt Status
        tU32 RXFIS:1;            // (r-) Receive FIFO Full Interrupt Status
        tU32 MSTIS:1;            // (r-) Multi-Master Contention Interrupt Status
        tU32 reserved:26;        // Reserved
    } ISR;

   // addr offset 0x0034
    struct
    {
        tU32 TXEIR:1;            // (r-) Transmit FIFO Empty Raw Interrupt Status
        tU32 TXOIR:1;            // (r-) Transmit FIFO Overflow Raw Interrupt Status
        tU32 RXUIR:1;            // (r-) Receive FIFO Underflow Raw Interrupt Status
        tU32 RXOIR:1;            // (r-) Receive FIFO Overflow Raw Interrupt Status
        tU32 RXFIR:1;            // (r-) Receive FIFO Full Raw Interrupt Status
        tU32 MSTIR:1;            // (r-) Multi-Master Contention Raw Interrupt Status
        tU32 reserved:26;        // Reserved
    } RISR;

   // addr offset 0x0038
    struct
    {
        tU32 TXOIC:1;             // (r-) Transmit FIFO Overflow Interrupt Clear
        tU32 reserved:31;         // Reserved
    } TXOICR;

   // addr offset 0x003C
    struct
    {
        tU32 RXOIC:1;             // (r-) Receive FIFO Overflow Interrupt Clear
        tU32 reserved:31;         // Reserved
    } RXOICR;

   // addr offset 0x0040
    struct
    {
        tU32 RXUIC:1;             // (r-) Receive FIFO Underflow Interrupt Clear
        tU32 reserved:31;         // Reserved
    } RXUICR;

   // addr offset 0x0044
    struct
    {
        tU32 MSTIC:1;             // (r-) Multi-Master Interrupt Clear
        tU32 reserved:31;         // Reserved
    } MSTICR;

   // addr offset 0x0048
    struct
    {
        tU32 IC:1;                // (r-) General Interrupt Clear
        tU32 reserved:31;         // Reserved
    } ICR;

   // addr offset 0x004C
    struct
    {
        tU32 RDMAE:1;             // (rw) Receive DMA Enable
        tU32 TDMAE:1;             // (rw) Transmit DMA Enable
        tU32 reserved:30;         // Reserved
    } DMACR;

   // addr offset 0x0050
    struct
    {
        tU32 DMATDL:5;            // (rw) DMA Transmit Data Level
        tU32 reserved:27;         // Reserved
    } DMATDLR;

   // addr offset 0x0054
    struct
    {
        tU32 DMARDL:5;            // (rw) DMA Receive Data Level
        tU32 reserved:27;         // Reserved
    } DMARDLR;

   // addr offset 0x58
   tU32 IDR;                      // (r-) Identification Register

   // addr offset 0x5C
   tU32 VERSION;                  // (r-) CoreKit Revision Register

   // addr offset 0x60            // (rw) Data Register
   tU16 DR;

   intra16;

} SpiMap;

//------------------------------------------------------------------------
// ESAI - Enhanced Serial Audio Interface
//------------------------------------------------------------------------
typedef volatile struct
{
    intra32;

    tU32 RSMB;                            // addr offset = 0x04

    tU32 RSMA;                            // addr offset = 0x08

    tU32 TSMB;                            // addr offset = 0x0C

    tU32 TSMA;                            // addr offset = 0x10

    struct
    {                                     // addr offset = 0x14
        tU32 PM:8;
        tU32 TLSIC:1;
        tU32 RLSIC:1;
        tU32 reserved1:1;
        tU32 PSR:1;
        tU32 DC:5;
        tU32 reserved2:1;
        tU32 ALC:1;
        tU32 WL:3;
        tU32 SSC1:1;
        tU32 reserved3:9;
    } CRA;

    struct
    {                                     // addr offset = 0x18
        tU32 OF0:1;
        tU32 OF1:1;
        tU32 SCD0:1;
        tU32 SCD1:1;
        tU32 SCD2:1;
        tU32 SCKD:1;
        tU32 SHFD:1;
        tU32 FSL:2;
        tU32 FSR:1;
        tU32 FSP:1;
        tU32 CKP:1;
        tU32 SYN:1;
        tU32 MOD:1;
        tU32 TE2:1;
        tU32 TE1:1;
        tU32 TE0:1;
        tU32 RE:1;
        tU32 TIE:1;
        tU32 RIE:1;
        tU32 TLIE:1;
        tU32 RLIE:1;
        tU32 TEIE:1;
        tU32 REIE:1;
        tU32 reserved:8;
    } CRB;

    struct
    {                                     // addr offset = 0x1C
        tU32 IF0:1;
        tU32 IF1:1;
        tU32 TFS:1;
        tU32 RFS:1;
        tU32 TUE:1;
        tU32 ROE:1;
        tU32 TDE:1;
        tU32 RDF:1;
        tU32 reserved:24;
    } SSISR;

    tU32 RX;                              // addr offset = 0x20

    tU32 TSR;                             // addr offset = 0x24

    tU32 TX2;                             // addr offset = 0x28

    tU32 TX1;                             // addr offset = 0x2C

    tU32 TX0;                             // addr offset = 0x30

    struct
    {                                     // addr offset = 0x34
        tU32 pd0:1;
        tU32 pd1:1;
        tU32 pd2:1;
        tU32 pd3:1;
        tU32 pd4:1;
        tU32 pd5:1;
        tU32 reserved:26;
    } PDR;

    struct
    {                                     // addr offset = 0x38
        tU32 pdc0:1;
        tU32 pdc1:1;
        tU32 pdc2:1;
        tU32 pdc3:1;
        tU32 pdc4:1;
        tU32 pdc5:1;
        tU32 reserved:26;
    } PRR;

    struct
    {                                     // addr offset = 0x3C
        tU32 pc0:1;
        tU32 pc1:1;
        tU32 pc2:1;
        tU32 pc3:1;
        tU32 pc4:1;
        tU32 pc5:1;
        tU32 reserved:26;
    } PCR;

    gap32(33);                            // addr offset = 0x40

    struct                                // addr offset = 0xC4
    {
        tU32 APB_FIFO_EN:1;
        tU32 FIFOTX_EN:1;
        tU32 FIFORX_EN:1;
        tU32 TXIFLSEL:3;
        tU32 RXIFLSEL:3;
        tU32 reserved:23;
    } FIFOCNTR;

    struct                                // addr offset = 0xC8
    {
        tU32 OverunDet:1;
        tU32 RXFF:1;
        tU32 RXFE:1;
        tU32 RXFGTE1Full:1;
        tU32 TXFF:1;
        tU32 TXFE:1;
        tU32 TXFLTE15Full:1;
        tU32 BUSY:1;
        tU32 reserved:24;
    } FIFOFLAG;

    struct                                // addr offset = 0xCC
    {
        tU32 TXIM:1;
        tU32 RXIM:1;
        tU32 OEIM:1;
        tU32 IntEdge:1;
        tU32 reserved:28;
    } FIFOIMSC;

    struct                                // addr offset = 0xD0
    {
        tU32 TXRIS:1;
        tU32 RXRIS:1;
        tU32 OERIS:1;
        tU32 TXIntLevel:1;
        tU32 RXIntLevel:1;
        tU32 reserved:27;
    } FIFORIS;

    struct                                // addr offset = 0xD4
    {
        tU32 TXMIS:1;
        tU32 RXMIS:1;
        tU32 OEMIS:1;
        tU32 reserved:29;
    } FIFOMIS;

    struct                                // addr offset = 0xD8
    {
        tU32 TXIC:1;
        tU32 RXIC:1;
        tU32 OEIC:1;
        tU32 OECLR:1;
        tU32 reserved:28;
    } FIFOICLR;
}EsaiMap;

//------------------------------------------------------------------------
// AIF - Audio Interface
//------------------------------------------------------------------------

typedef volatile struct
{                                           // conversion  AHB addr = (0x58000000 + cell*4)
    //------------------                    // ADDR cell AF1  0x58048000  -- in manual (0x12000)
    //  SAMPLE RATE CONV                    // ADDR cell AF2  0x58054000  -- in manual (0x15000)
    //------------------
    struct                                  // addr offset = 0x0
    {
        tU32 DRLL_THRES:4;
        tU32 DITHER_EN:1;
        tU32 ROU:1;
        tU32 BYPASS:1;
        tU32 reserved0:5;
        tU32 DRLL_DIFF:14;
        tU32 reserved1:2;
        tU32 DRLL_LOCK:2;
        tU32 reserved2:2;
    }SRC_CSR[6];

    gap32(10);                          // addr offset = 0x18    (0x6)

    /*
    struct                                // addr offset = 0x40  (0x10)
    {
        tU32 reserved0:10;
        tU32 DATA:22;
    }SRC_D0[6][2];
    */
  tU32 SRCDO0L;
  tU32 SRCDO0R;
  tU32 SRCDO1L;
  tU32 SRCDO1R;
  tU32 SRCDO2L;
  tU32 SRCDO2R;
  tU32 SRCDO3L;
  tU32 SRCDO3R;
  tU32 SRCDO4L;
  tU32 SRCDO4R;
  tU32 SRCDO5L;
  tU32 SRCDO5R;

    gap32(20);                            // addr offset = 0x70  (0x1C)

    tU32 SRC_RATIO[6];                  // addr offset = 0xC0 (0x30)

    gap32(202);                         // addr offset = 0xD8 (0x36)

    //------------------
    //  LOW PASS FILTER  // ADDR cell 0x58048400  -- -- in manual (0x12100)
    //------------------
    struct                              // addr offset = 0x400 (0x100)
    {
        tU32 FS:12;
        tU32 TEST_OFD:1;
        tU32 CLRF:1;
        tU32 reserved0:2;
        tU32 LPU_UFL:6;
        tU32 reserved1:2;
        tU32 LPU_OVF:6;
        tU32 reserved2:2;
    }LPFCSR;

    gap32(15);                          // addr offset = 0x404 (0x101)

    /*
    struct                                // addr offset = 0x440 (0x110)
    {
        tU32 reserved0:12;
        tU32 DATA:20;
    }LPFDI[6][2];
    */
  tU32 LPFDI0L;
  tU32 LPFDI0R;
  tU32 LPFDI1L;
  tU32 LPFDI1R;
  tU32 LPFDI2L;
  tU32 LPFDI2R;
  tU32 LPFDI3L;
  tU32 LPFDI3R;
  tU32 LPFDI4L;
  tU32 LPFDI4R;
  tU32 LPFDI5L;
  tU32 LPFDI5R;

    gap32(4);                            // addr offset = 0x470 (0x11C)

    /*
    struct                               // addr offset = 0x480 (0x120)
    {
        tU32 reserved0:12;
        tU32 DATA:20;
    }LPFD0[6][2];
    */
  tU32 LPFDO0L;
  tU32 LPFDO0R;
  tU32 LPFDO1L;
  tU32 LPFDO1R;
  tU32 LPFDO2L;
  tU32 LPFDO2R;
  tU32 LPFDO3L;
  tU32 LPFDO3R;
  tU32 LPFDO4L;
  tU32 LPFDO4R;
  tU32 LPFDO5L;
  tU32 LPFDO5R;


    gap32(3796);                        // addr offset = 0x4B0 (0x12C)

    //------------------
    //  SAI                             // ADDR cell 0x5804C000  -- -- in manual (0x13000)
    //------------------
    struct                              // addr offset = 0x4000 (0x1000)
    {
        tU32 ENA:1;
        tU32 IO:1;
        tU32 reserved0:1;
        tU32 MME:1;
        tU32 LENGTH:3;
        tU32 DIR:1;
        tU32 LPR:1;
        tU32 CKP:1;
        tU32 REL:1;
        tU32 ADJ:1;
        tU32 CNT:2;
        tU32 SYN:2;
        tU32 reserved1:7;
        tU32 TM:1;
        tU32 reserved2:8;
    }SAI_CSR[4];

    //tU32 SPDIF;                               // addr offset = 0x4010 (0x1004)
  union
  {
    struct /* bits */
    {
      tU32 ENA : 1;
      tU32 SSEL  : 2;
      tU32 CHS : 1;
      tU32 EMV : 1;
      tU32 EM  : 3;
      tU32 reserved1 : 4;
      tU32 Plr : 1;
      tU32 Pl  : 1;
      tU32 Pr  : 1;
      tU32 Vlr : 1;
      tU32 Vl  : 1;
      tU32 Vr  : 1;
      tU32 ERFlr : 1;
      tU32 ERFl  : 1;
      tU32 ERFr  : 1;
      tU32 ERCLR : 1;
      tU32 reserved2 : 10;
    } bit;
    tU32 reg;
  } SPDIFCSR;

    gap32(11);                              // addr offset = 0x4014 (0x1005)

    //tU32 SAI_DATA[48];                        // addr offset = 0x4040 (0x1010)
  tU32 SAI1RXL;
  tU32 SAI1RXR;
  gap32(2);
  tU32 SAI2RX1L;
  tU32 SAI2RX1R;
  tU32 SAI2RX2L;
  tU32 SAI2RX2R;
  tU32 SAI2RX3L;
  tU32 SAI2RX3R;
  tU32 SAI2RX4L;
  tU32 SAI2RX4R;
  tU32 SAI2TX1L;
  tU32 SAI2TX1R;
  gap32(2);
  tU32 SAI3RX1L;
  tU32 SAI3RX1R;
  tU32 SAI3RX2L;
  tU32 SAI3RX2R;
  tU32 SAI3RX3L;
  tU32 SAI3RX3R;
  tU32 SAI3RX4L;
  tU32 SAI3RX4R;
  tU32 SAI3TX1L;
  tU32 SAI3TX1R;
  tU32 SAI3TX2L;
  tU32 SAI3TX2R;
  tU32 SAI3TX3L;
  tU32 SAI3TX3R;
  tU32 SAI3TX4L;
  tU32 SAI3TX4R;
  tU32 SAI4RX1L;
  tU32 SAI4RX1R;
  tU32 SAI4RX2L;
  tU32 SAI4RX2R;
  tU32 SAI4RX3L;
  tU32 SAI4RX3R;
  tU32 SAI4RX4L;
  tU32 SAI4RX4R;
  tU32 SAI4TX1L;
  tU32 SAI4TX1R;
  tU32 SAI4TX2L;
  tU32 SAI4TX2R;
  tU32 SAI4TX3L;
  tU32 SAI4TX3R;
  tU32 SAI4TX4L;
  tU32 SAI4TX4R;

    gap32(192);                             // addr offset = 0x4100 (0x1040)

    //------------------
    //  MUX                         // ADDR cell 0x5804C400  -- -- in manual (0x13100)
    //------------------

    tU32 AIMUX_CSR;                         // addr offset = 0x4400 (0x1100)
    tU32 AOMUX_CSR;                         // addr offset = 0x4404 (0x1101)

}AIFMap;

//------------------------------------------------------------------------
// SMU
//------------------------------------------------------------------------

typedef volatile struct
{
    gap32(12);                  // addr offset = 0x00

    tU32 FREQADD_REG;           // addr offset = 0x30

    gap32(1);                   // addr offset = 0x34

    tU32 CLOCKOUT_MASK_REG0;    // addr offset = 0x38

    gap32(1);                   // addr offset = 0x3C

    tU32 CLK_GATING_REG0;       // addr offset = 0x40

    gap32(3);                   // addr offset = 0x44

    tU8  PLL_DIV_INT;           // addr offset = 0x50

    gap8(3);

    struct                      // addr offset = 0x54
    {
        tU32 ODF:3;
        tU32 IDF:3;
        tU32 reserved:26;
    }IDF_ODF;

    tU16  PLL_DIV_FRAC;         // addr offset = 0x58

    gap8(2);

    tU32  EN_PLL_FRAC;          // 1 bit addr offset = 0x5C

    tU32  STROBE_PLL_FRAC;      // 1 bit addr offset = 0x60

    struct                      // addr offset = 0x64
    {
        tU32 DITH:2;
        tU32 STROBE:1;
        tU32 FRAC:1;
        tU32 reserved:28;
    }PLL_FRAC_CTRL;

    struct              // addr offset = 0x68
    {
        tU32 SAI3_DATA_IN:1;
        tU32 ESAI_DATA_IN:1;
        tU32 SAI3_CLK:2;
        tU32 SAI3_WS:2;
        tU32 ESAI_CLK:1;
        tU32 ESAI_WS:1;
        tU32 reserved:24;
    }AUDIO_INT;

    //gap32(0x1);
    tU32 LVDS_CONFIG_IF;   // 8 bits addr offset = 0x6C

    tU32 LVDS_IF;          // 4 bits addr offset = 0x70

    tU32 OSCI_REG;         // 1 bit addr offset = 0x74

    tU32 XP70_FREQ;

    gap32(1);

    tU32 ARM_TCM;

    gap32(0x15);

    tU32 XP70[11];        // 11*4 from 0xD8 to 0x100

    gap32(0x4);

    struct               // 4 bits addr offset = 0x114
        {
            tU32 ESAI_EN:1;
            tU32 ESAI_SYNC_EN:1;
            tU32 WS_TX_SRC:1;
            tU32 reserved:29;
        } ESAI_REG;

    gap32(0x5);

    struct
    {
        tU32 IDO_SEL:2;
        tU32 reserved:30;
    } IDO_REG;			// 2 bit addr offset = 0x12C

    gap32(0xD);


    tU32  ESAI_AUDPLL_DIV;  //  13bit addr offset = 0x164
    tU32  AIF1_AUDPLL_DIV;  //  13bit addr offset = 0x168
    tU32  AIF2_AUDPLL_DIV;  //  13bit addr offset = 0x16C

    struct
    {
        tU32 ESAI:1;
        tU32 AIF1:1;
        tU32 AIF2:1;
        tU32 reserved:29;
    } AUDPLL_DIV_RESET;     //  3bit addr offset = 0x170

    gap32(3);
    struct
    {
        tU32 SCK:2;
        tU32 reserved:30;
    } DBI_SCK;     //  3bit addr offset = 0x180    
    struct
    {
        tU32 WS:2;
        tU32 reserved:30;
    } DBI_WS;      //  3bit addr offset = 0x184
    gap32(2);

    tU32 MUX_ARM_PAD_REG_0; //   addr offset = 0x190

    tU32  DMA_SEL_MODE1;    //   addr offset = 0x194
    tU32  DMA_SEL_MODE2;    //   addr offset = 0x198

    tU32  ESAI_MUX;         //  6bit addr offset = 0x19C

    gap32(2);
    tU32  IP_RESET;         //  6bit addr offset = 0x1A8

    gap32(2);

    tU32  MEM_PRIORITY;     // addr offset = 0x1B4  only on sta662

    gap32(11);

    tU32  CGU1_BITCLK;      // addr offset = 0x1E4 (SYSDIV_PL2)
    tU32  CGU1_WS_DUTY_C;   // addr offset = 0x1E8 (SYSDIV_PL1)
    tU32  CGU2_BITCLK;      // addr offset = 0x1EC (SYSDIV_PL4)
    tU32  CGU2_WS_DUTY_C;   // addr offset = 0x1F0 (SYSDIV_PL3)

    struct
    {
        tU32 CGU1_PH:1;
        tU32 CGU2_PH:1;
        tU32 reserved:30;
    } CGU_CLK_PHASE;        //  3bit addr offset = 0x1F4 (SYSDIV_PL5)

    gap32(2);               // VIT_REG1 and VIT_REG2

    tU32 SDEC_IF;           // addr offset = 0x200

    tU32 SDEC_GPIO_IF;      // addr offset = 0x204

    gap32(2);

   struct                   // addr offset = 0x210
    {
        tU32 AIF1:1;
        tU32 AIF2:1;
        tU32 reserved:30;
    }AIF_PLL_SRC;

    struct                  // addr offset = 0x214
    {
        tU32 AIF1:1;
        tU32 AIF2:1;
        tU32 reserved:30;
    }AIF_128_512;


    struct              // addr offset = 0x218
    {
        tU32 WS1:1;
        tU32 CLK1:1;
        tU32 SAI1:1;
        tU32 WS2:1;
        tU32 CLK2:1;
        tU32 SAI2:1;
        tU32 reserved:26;
    }CGU_PLLSRC;

    struct              // addr offset = 0x21C
    {
        tU32 CGU1:1;
        tU32 CGU2:1;
        tU32 reserved:30;
    }CGU_128_512;


    tU32 SYSPLL_DIV[4];         // addr offset = 0x220
                                // 1st and 3rd reg have 8 bits
                                // 2nd and 4th reg have 13 bits

   struct
    {
        tU32 DIV1:1;
        tU32 DIV2:1;
        tU32 DIV3:1;
        tU32 DIV4:1;
        tU32 reserved:28;
    } SYSPLL_DIV_RESET;         // 4 bits, addr offset = 0x230 (SYS_DIVIDER_REG_4)

    gap32(31);

    struct
    {
        tU32 OCLK_MCLKOUT_SEL_0:    1;
        tU32 OCLK_MCLKOUT_SEL_1:    1;
        tU32 MCLKOUT_SET_0:         1;
        tU32 MCLKOUT_SET_1:         1;
        tU32 OCLK_GPIO13_SEL_0:     1;
        tU32 OCLK_GPIO13_SEL_1:     1;
        tU32 GPIO13_SET:            1;
        tU32 reserved:              25;
    } OCLK_SEL;                 // 7 bits, addr offset = 0x2B0 (OCLK_SEL)
}SMUMap;

//------------------------------------------------------------------------
// BCO
//------------------------------------------------------------------------

typedef volatile struct BCOMap   //0x58031040
{

    struct              // addr offset = 0x214
        {
            tU32 OprMode:2;
            tU32 NewTx:1;
            tU32 TxMode:1;
            tU32 reserved:28;
        } BCO_CS1; // 3 bits

    tU32 BCO_TCR[5]; //32 bits
    tU32 BCO_STS1; // 1 bits
    tU32 BCO_PRIO; // 3*4 bits
    tU32 BCO_WTC[5]; //32 bits
    gap32(3);
    struct              // addr offset = 0x214
    {
        struct
        {
            tU32 addr:24;
            tU32 freq:2;
            tU32 reserved:6;
        }SRC;
        struct
        {
            tU32 addr:24;
            tU32 reserved:8;
        }DST;
    }BCO_PRG[160];

}BCOMap;

//------------------------------------------------------------------------
// FEI
//------------------------------------------------------------------------

#if defined (PLATFORM_IS_STA661) || defined (PLATFORM_IS_STA662)

//!
//! \struct IRC register map. This interface is mapped at base address
//!         0x5809_40a0/0x5809_42a0 on the AHB bus. On the IPBUS the address is 0x2_5028/A8.
//!
typedef volatile struct tFEIMapIRC
{
  union
  {
    struct
    {
      tU32 cent_win :8;
      tU32 agc :1;
      tU32 reserved_1 :1;
      tU32 acc_clear :1;
      tU32 reserved_2 :1;
      tU32 enable :1;
      tU32 cent_bypass :1;
      tU32 hp_en :1;
      tU32 reserved_3 :17;
    } bit;
    tU32 reg;
  } CFG;
  tU32 U1;
  tU32 ACC0;
  tU32 ACC1;
  tU32 K11;
  tU32 K12;
  tU32 K13;
  tU32 K14;
  tU32 K21;
  tU32 K22;
  tU32 K23;
  tU32 K24;
  tU32 K31;
  tU32 K32;
  tU32 K33;
  tU32 K34;
  tU32 K41;
  tU32 K42;
  tU32 K43;
  tU32 K44;
  tU32 K51;
  tU32 Klast;
  tU32 SCALE_MAX;
} tFEIMapIRC;

//!
//! \struct DOC register map.
//!
typedef volatile struct tFEIMapDOC
{
  union
  {
    struct /* bits */
    {
      tU32 ENA :1;
      tU32 LMS_ON :1;
      tU32 SW_ON :1;
      tU32 RATE_SEL :1;
      tU32 DC_CLEAR :1;
      tU32 CENT_BP :1;
#if defined(CONFIG_TARGET_SYS_STA662_Bx) || defined(CONFIG_TARGET_SYS_STA661)
      tU32 CENT_WIN :11;
      tU32 reserved :15;
#else
      tU32 CENT_WIN :12;
      tU32 reserved_1 :6;
      tU32 DYN_IS_IIR_CFG :3;
      tU32 reserved_2 :5;
#endif
    } bit;
    tU32 reg;
  } CFG0;

  tU32 CFG1;
  tU32 CFG2;
  tU32 CFG3;
  tU32 CFG4;
  tU32 CFG5;
  tU32 CFG6;
  tU32 OUT_I; // HW->SW I
  tU32 OUT_Q; // HW->SW Q
  tU32 INIT_I;
  tU32 INIT_Q;
} tFEIMapDOC;

//!
//! \struct S-AGC register map.
//!
typedef volatile struct tFEIMapAGC
{
  union
  {
    struct
    {
#if defined(CONFIG_TARGET_SYS_STA662_Bx) || defined(CONFIG_TARGET_SYS_STA661)
      tU32 RTC_GAIN :4;
      tU32 AGC_CMP_CNT :16;
      tU32 SW_AGC_SL :3;
      tU32 BP_AGC_CMP :1;
      tU32 BP_SW_CMP :1;
      tU32 BP_MIXER_GAIN :1;
      tU32 reserved_1 :6;
#else
      tU32 RTC_GAIN :4;
      tU32 AGC_CMP_CNT :16;
      tU32 reserved_1 :3;
      tU32 BP_AGC_CMP :1;
      tU32 BP_SW_CMP :1;
      tU32 BP_MIXER_GAIN :1;
      tU32 AMP_OUT_SR :3;
      tU32 reserved_2 :3;
#endif
    } bit;
    tU32 reg;
  } AMP_CFG0;
  tU32 INIT;
  // BE CAREFUL: these 5 registers are unused in ANT3 and ANT4 on STA662Cx
  tU32 B0;
  tU32 B1;
  tU32 B2;
  tU32 A0;
  tU32 A1;
  // -------------------
#if defined(CONFIG_TARGET_SYS_STA662_Bx) || defined(CONFIG_TARGET_SYS_STA661)
  tU32 SW_GAIN;
#else
  union {
      struct {
        tU32 SW_SHIFT   :3;
        tU32 reserved   :5;
        tU32 SW_SCALE   :24;
      } bit;
      tU32 reg;
  } SW_GAIN;
#endif
  tU32 MIXER_GAIN_COMP;
} tFEIMapAGC;

//!
//! \struct FEI stage1 register map.
//!
typedef volatile struct tFEIMapStage1
{
  //! DDC_CFG1 register is defined as union of 3 bitfiels and one 32 bit register
  //! bit12 is dedicated for ANT1 and ANT2 (no meaning for ANT3 and ANT4)
  //! bit34 is dedicated for ANT3 and ANT4 (no meaning for ANT1 and ANT2)
  //! bit has common bits shared between all antennas
  union
  {
    struct
    {
#if defined(CONFIG_TARGET_SYS_STA662_Bx) || defined(CONFIG_TARGET_SYS_STA661)
      tU32 reserved_1 :2;
#else
      tU32 RTC_SWAP    :1;
      tU32 AGC_CMP_1X  :1;
#endif
      tU32 DDC_IN :2;
      tU32 DAB_MODE :1;
      tU32 HDAM_MODE :1;
      tU32 HDAM_FIR_I :1;
      tU32 MIXER_I_SEL :2;
      tU32 reserved_2 :1;
      tU32 BB_TS_SEL :3;
      tU32 CFIR_GAIN :2;
      tU32 FIR1_GAIN :2;
      tU32 FIR2_GAIN :2;
      tU32 HDFM_GAIN :2;
      tU32 HDAM_GAIN :2;
      tU32 EN_CRCT :1;
#if defined(CONFIG_TARGET_SYS_STA662_Bx) || defined(CONFIG_TARGET_SYS_STA661)
      tU32 reserved_3 :1;
#else
      tU32 DYNIS_IPBUS :1;
#endif
      tU32 IPBUS_DBG_DATA1_SEL :2;
      tU32 IPBUS_DBG_DATA2_SEL :2;
      tU32 IPBUS_DBG_DATA3_SEL :3;
    } bit12;
    struct
    {
#if defined(CONFIG_TARGET_SYS_STA662_Bx) || defined(CONFIG_TARGET_SYS_STA661)
      tU32 reserved_1 :2;
#else
      tU32 RTC_SWAP    :1;
      tU32 reserved_1  :1;
#endif
      tU32 DDC_IN :2;
      tU32 DAB_MODE :1;
      tU32 reserved_2 :3;
      tU32 DAB_SCL :3;
      tU32 reserved_3 :2;
      tU32 CFIR_GAIN :2;
      tU32 reserved_4 :12;
      tU32 IPBUS_DBG_DATA_SEL :2;
      tU32 reserved_5 :3;
    } bit34;
    struct
    {
#if defined(CONFIG_TARGET_SYS_STA662_Bx) || defined(CONFIG_TARGET_SYS_STA661)
      tU32 reserved_1 :2;
#else
      tU32 RTC_SWAP    :1;
      tU32 reserved_1  :1;
#endif
      tU32 DDC_IN :2;
      tU32 DAB_MODE :1;
      tU32 reserved_2 :8;
      tU32 CFIR_GAIN :2;
      tU32 reserved_3 :12;
      tU32 IPBUS_DBG_DATA_SEL :2;
      tU32 reserved_4 :3;
    } bit;
    tU32 reg;
  } DDC_CFG1;

  /* 11 registers */
  tFEIMapDOC DOC;

  /* 9 registers */
  tFEIMapAGC AGC;

} tFEIMapStage1;

//!
//! \struct FEI stage2 register map.
//!
typedef volatile struct tFEIMapStage2
{
  tU32 NCO_IF;
  tU32 FIR2_COEF_SET[16];

  /* 23 registers */
  tFEIMapIRC IRC;
} tFEIMapStage2;

//!
//! \struct FEI ANT1 & ANT2 register map.
//!
typedef volatile struct tFEIAnt12Map
{
  tFEIMapStage1 stage1;
  tFEIMapStage2 stage2;
  tU32 IPBUS_DATA2_P0;
  tU32 IPBUS_DATA2_P1;
  tU32 IPBUS_DATA1_P0;
  tU32 IPBUS_DATA1_P1;
  tU32 IPBUS_DATA1_P2;
  tU32 IPBUS_DATA1_P3;
  tU32 IPBUS_DATA3_P0;
  tU32 IPBUS_DATA3_P1;
  tU32 IPBUS_DATA3_P2;
  tU32 IPBUS_DATA3_P3;
} tFEIAnt12Map;

//!
//! \struct FEI ANT3 & ANT4 register map.
//!
typedef volatile struct tFEIAnt34Map
{
  tFEIMapStage1 stage1;
  tU32 IPBUS_DATA_P0;
  tU32 IPBUS_DATA_P1;
} tFEIAnt34Map;

//!
//! \struct FEI SRC HD0 and HD1 register map.
//!
typedef volatile struct tFEISRCHDMap
{
  tU32 RATIO1;
  tU32 RATIO2;
  tU32 RATIO3;
} tFEISRCHDMap;


//!
//! \struct FEI SRC FM0 and FM1 register map.
//!
typedef volatile struct tFEISRCMap
{
  tU32 IDEAL_RATIO;
  tU32 IDEAL_RATIO1;
  tU32 IDEAL_RATIO2;
  tU32 IDEAL_RATIO3;
} tFEISRCMap;

//!
//! \struct FEI register map. This interface is mapped at base address
//!         0x5809_0000 on the AHB bus. On the IPBUS the address is 0x2_5000.
//!
typedef volatile struct FEIMapTy
{
  union
  {
    struct
    {
      tU32 reserved_0 :4;
      tU32 DDC_CLK_GATING :4;
      tU32 DDC_RST_GATING :4;
      tU32 DMUX_DAB1 :3;
      tU32 DMUX_DAB2 :3;
      tU32 DAB1_IQ_SWAP :1;
      tU32 DAB2_IQ_SWAP :1;
#if defined(CONFIG_TARGET_SYS_STA662_Bx) || defined(CONFIG_TARGET_SYS_STA661)
      tU32 reserved_1 :4;
#else
      tU32 IPBUS_RW_DEBUG :4;
#endif
      tU32 reserved_2 :4;
      tU32 reserved_3 :4;
    } bit;
    tU32 reg;
  } FEI_DDC_CFG0;                // @ 0x5809_4000

  union
  {
    struct
    {
        tU32 SAI0_EN :1;
        tU32 SAI0_MODE :3;
        tU32 SAI0_DIR :1;
        tU32 SAI0_LRP :1;
        tU32 SAI0_SKP :1;
        tU32 SAI0_REL :1;
        tU32 SAI0_SYN :1;
        tU32 SAI0_TM :1;
        tU32 SAI0_BCLK :1;
        tU32 SAI0_CH_SEL :1;
        tU32 SAI0_HD456 :1;
        tU32 reserved :3;
        tU32 SAI1_EN :1;
        tU32 SAI1_MODE :3;
        tU32 SAI1_DIR :1;
        tU32 SAI1_LRP :1;
        tU32 SAI1_SKP :1;
        tU32 SAI1_REL :1;
        tU32 SAI1_SYN :1;
        tU32 SAI1_TM :1;
        tU32 SAI1_BCLK :1;
        tU32 SAI1_CH_SEL :1;
        tU32 SAI1_HD456 :1;
    } bit;
    tU32 reg;
  } FEI_SAI_CFG;

  tFEIAnt12Map ANT1;
  tFEIAnt34Map ANT3;
#if defined(CONFIG_TARGET_SYS_STA662_Bx) || defined(CONFIG_TARGET_SYS_STA661)
  gap32(34);
#else
  tU32 SAGC_CMP_GAIN[12];
  gap32(22);
#endif
  tFEIAnt12Map ANT2;
  tFEIAnt34Map ANT4;
  gap32(3872);
  union
  {
    struct
    {
      tU32 reserved_1 :8;
      tU32 HD0_DRM_MODE :1;
      tU32 HD0_SEL_RATIO :1;
      tU32 HD0_LCK_RT_EN :1;
      tU32 HD0_IQ_SWAP :1;
      tU32 HD0_I_SEL :2;
      tU32 HD0_456_MODE :1;
      tU32 reserved_2 :1;
      tU32 HD1_DRM_MODE :1;
      tU32 HD1_SEL_RATIO :1;
      tU32 HD1_LCK_RT_EN :1;
      tU32 HD1_IQ_SWAP :1;
      tU32 HD1_I_SEL :2;
      tU32 HD1_456_MODE :1;
      tU32 reserved_3 :1;
      tU32 SRC_HD0_CLK_GATE :1;
      tU32 SRC_HD1_CLK_GATE :1;
      tU32 SRC_FM0_CLK_GATE :1;
      tU32 SRC_FM1_CLK_GATE :1;
      tU32 SRC_HD0_RST_GATE :1;
      tU32 SRC_HD1_RST_GATE :1;
      tU32 SRC_FM0_RST_GATE :1;
      tU32 SRC_FM1_RST_GATE :1;
    } bit;
    tU32 reg;
  } SRC_HD_CFG;
  gap32(2);
  tFEISRCHDMap SRC_HD0;
  gap32(7);
  tFEISRCHDMap SRC_HD1;
  gap32(5);
  union
  {
    struct
    {
        tU32 FM0_INTP2 :1;
        tU32 FM0_SEL_RATIO :1;
        tU32 FM0_LCK_RATIO_EN :1;
        tU32 reserved_0 :1;
        tU32 FM0_FIR_SEL :1;
        tU32 FM0_DRLL_GAIN :3;
        tU32 FM0_FIR_GAIN :2;
        tU32 FM0_I_SEL  :2;
        tU32 reserved_1 :4;
        tU32 FM1_INTP2 :1;
        tU32 FM1_SEL_RATIO :1;
        tU32 FM1_LCK_RATIO_EN :1;
        tU32 reserved_2 :1;
        tU32 FM1_FIR_SEL :1;
        tU32 FM1_DRLL_GAIN :3;
        tU32 FM1_FIR_GAIN :2;
        tU32 FM1_I_SEL  :2;
        tU32 reserved_3 :4;
    } bit;
    tU32 reg;
  } FEI_SRC_FM_CFG;
  tFEISRCMap SRC_FM0;
  gap32(5);
  tFEISRCMap SRC_FM1;
} FEIMapTy;

//!
//! \struct SLINK register map. This interface is mapped at base address
//!         0x5808_0000  on the AHB bus. On the IPBUS the address is 0x2_0000.
//!
typedef volatile struct SLINKMapTy
{
    union {
        struct {
            tU32 MODE : 2;
            tU32 reserved_0 : 14;
            tU32 SLINK0_CLK_SEL : 2;
            tU32 SLINK1_CLK_SEL : 2;
            tU32 SLINK2_CLK_SEL : 2;
            tU32 SLINK3_CLK_SEL : 2;
            tU32 SLINK0_WS_SEL : 2;
            tU32 SLINK1_WS_SEL : 2;
            tU32 SLINK2_WS_SEL : 2;
            tU32 SLINK3_WS_SEL : 2;
        } bit;
        tU32 reg;
    } SLINK_CNTR0;
    gap32(255);
    union {
        struct {
            tU32 LIF0_DDR_SEL : 1;
            tU32 LIF0_DDR_POL_I : 1;
            tU32 LIF0_DDR_POL_Q : 1;
            tU32 LIF0_DDR_PHS : 1;
            tU32 reserved_0 : 4;
            tU32 LIF1_DDR_SEL : 1;
            tU32 LIF1_DDR_POL_I : 1;
            tU32 LIF1_DDR_POL_Q : 1;
            tU32 LIF1_DDR_PHS : 1;
            tU32 reserved_1 : 4;
            tU32 LIF2_DDR_SEL : 1;
            tU32 LIF2_DDR_POL_I : 1;
            tU32 LIF2_DDR_POL_Q : 1;
            tU32 LIF2_DDR_PHS : 1;
            tU32 reserved_2 : 4;
            tU32 LIF3_DDR_SEL : 1;
            tU32 LIF3_DDR_POL_I : 1;
            tU32 LIF3_DDR_POL_Q : 1;
            tU32 LIF3_DDR_PHS : 1;
            tU32 reserved_3 : 4;
        } bit;
        tU32 reg;
    } SLINK_CNTR1;
    gap32(255);
    union {
        struct {
            tU32 RTC0_DDR_SEL : 1;
            tU32 RTC0_DDR_POL_I : 1;
            tU32 RTC0_DDR_POL_Q : 1;
            tU32 RTC0_DDR_PHS : 1;
            tU32 reserved_0 : 4;
            tU32 RTC1_DDR_SEL : 1;
            tU32 RTC1_DDR_POL_I : 1;
            tU32 RTC1_DDR_POL_Q : 1;
            tU32 RTC1_DDR_PHS : 1;
            tU32 reserved_1 : 4;
            tU32 RTC2_DDR_SEL : 1;
            tU32 RTC2_DDR_POL_I : 1;
            tU32 RTC2_DDR_POL_Q : 1;
            tU32 RTC2_DDR_PHS : 1;
            tU32 reserved_2 : 4;
            tU32 RTC3_DDR_SEL : 1;
            tU32 RTC3_DDR_POL_I : 1;
            tU32 RTC3_DDR_POL_Q : 1;
            tU32 RTC3_DDR_PHS : 1;
            tU32 reserved_3 : 4;
        } bit;
        tU32 reg;
    } SLINK_CNTR2;
    gap32(255);
    union {
        struct {
            tU32 SAI0_CLK_POL : 1;
            tU32 SAI0_WS_LNG : 1;
            tU32 SAI0_LNG : 1;
            tU32 SAI0_CTL : 1;
            tU32 SAI0_DLY : 1;
            tU32 SAI0_SPL : 1;
            tU32 SAI0_PAD : 1;
            tU32 SAI0_EN : 1;
            tU32 SAI1_CLK_POL : 1;
            tU32 SAI1_WS_LNG : 1;
            tU32 SAI1_LNG : 1;
            tU32 SAI1_CTL : 1;
            tU32 SAI1_DLY : 1;
            tU32 SAI1_SPL : 1;
            tU32 SAI1_PAD : 1;
            tU32 SAI1_EN : 1;
            tU32 SAI2_CLK_POL : 1;
            tU32 SAI2_WS_LNG : 1;
            tU32 SAI2_LNG : 1;
            tU32 SAI2_CTL : 1;
            tU32 SAI2_DLY : 1;
            tU32 SAI2_SPL : 1;
            tU32 SAI2_PAD : 1;
            tU32 SAI2_EN : 1;
            tU32 SAI3_CLK_POL : 1;
            tU32 SAI3_WS_LNG : 1;
            tU32 SAI3_LNG : 1;
            tU32 SAI3_CTL : 1;
            tU32 SAI3_DLY : 1;
            tU32 SAI3_SPL : 1;
            tU32 SAI3_PAD : 1;
            tU32 SAI3_EN : 1;
        } bit;
        tU32 reg;
    } SLINK_CNTR3;
    gap32(255);
    union {
        struct {
            tU32 reserved_0 : 16;
            tU32 SLINK0_AGC_DATA : 16;
        } bit;
        tU32 reg;
    } SLINK_CNTR4;
    gap32(255);
    union {
        struct {
            tU32 reserved_0 : 16;
            tU32 SLINK1_AGC_DATA : 16;
        } bit;
        tU32 reg;
    } SLINK_CNTR5;
    gap32(255);
    union {
        struct {
            tU32 reserved_0 : 16;
            tU32 SLINK2_AGC_DATA : 16;
        } bit;
        tU32 reg;
    } SLINK_CNTR6;
    gap32(255);
    union {
        struct {
            tU32 reserved_0 : 16;
            tU32 SLINK3_AGC_DATA : 16;
        } bit;
        tU32 reg;
    } SLINK_CNTR7;
    gap32(255);
    union {
        struct {
            tU32 reserved_0 : 16;
            tU32 SAI0_CONTROL : 16;
        } bit;
        tU32 reg;
    } SLINK_CNTR8;
    gap32(255);
    union {
        struct {
            tU32 reserved_0 : 16;
            tU32 SAI1_CONTROL : 16;
        } bit;
        tU32 reg;
    } SLINK_CNTR9;
    gap32(255);
    union {
        struct {
            tU32 reserved_0 : 16;
            tU32 SAI2_CONTROL : 16;
        } bit;
        tU32 reg;
    } SLINK_CNTR10;
    gap32(255);
    union {
        struct {
            tU32 reserved_0 : 16;
            tU32 SAI3_CONTROL : 16;
        } bit;
        tU32 reg;
    } SLINK_CNTR11;

  gap32(255);
  union {
        struct {
          tU32 reserved_0 : 30;
          tU32 VCORANGE : 1;
          tU32 reserved_1 : 1;
        } bit;
        tU32 reg;
  } SLINK_CNTR12;
  gap32(255);
  union {
        struct {
          tU32 reserved_0 : 30;
          tU32 VCORANGE : 1;
          tU32 reserved_1 : 1;
        } bit;
        tU32 reg;
  } SLINK_CNTR13;
  gap32(255);
  union {
        struct {
          tU32 reserved_0 : 30;
          tU32 VCORANGE : 1;
          tU32 reserved_1 : 1;
        } bit;
        tU32 reg;
  } SLINK_CNTR14;
  gap32(255);
  union {
        struct {
          tU32 reserved_0 : 30;
          tU32 VCORANGE : 1;
          tU32 reserved_1 : 1;
        } bit;
        tU32 reg;
  } SLINK_CNTR15;

} SLINKMapTy;

//------------------------------------------------------------------------
// IFP
//------------------------------------------------------------------------
//!
//! \struct IFP register map. This interface is mapped at base address
//!         0x5803_8000  on the AHB bus. On the IPBUS the address is 0xE000.
//!
typedef volatile struct IFPMapTy
{
  union
  {
    struct /* bits */
    {
      tU32 ifp_mode  : 2;
      tU32 cfg_path1 : 1;
      tU32 cfg_path2 : 1;
      tU32 VPA_off0on1 : 1;
      tU32 setsel_iff1 : 2;
      tU32 setsel_iff2 : 2;
      tU32 inmux_iff1  : 1;
      tU32 inmux_iff2  : 1;
      tU32 inmux_dem1  : 2;
      tU32 inmux_dem2  : 2;
      tU32 vpa_fcorr : 1;
      tU32 vpa_shift_out_factor  : 3;
      tU32 stereo_mute1_source : 1;
      tU32 stereo_mute2_source : 1;
      tU32 deci45_6_inmux1 : 1;
      tU32 deci45_6_inmux2 : 1;
      tU32 dds_highcut_on  : 1;
      tU32 inmux_dsb1  : 2;
      tU32 inmux_dsb2  : 2;
      tU32 dsb1_use_fstcorr  : 1;
      tU32 dsb2_use_fstcorr  : 1;
      tU32 reserved : 2;
    } bit;
    tU32 reg;
  } IFP_CS1;

  tU32 IFP_IFF1_DATA1_IN;
  tU32 IFP_IFF2_DATA2_IN;
  gap32(5);
  tU32 IFP_FST_SHIFT1;
  tU32 IFP_FST_SCALE1;
  tU32 IFP_FST_SHIFT2;
  tU32 IFP_FST_SCALE2;
  tU32 IFP_DEM_AM_I1;
  tU32 IFP_DEM_AM_Q1;
  tU32 IFP_DEM_AM_I2;
  tU32 IFP_DEM_AM_Q2;
  tU32 IFP_DECI45_6_AM_IN_I1;
  tU32 IFP_DECI45_6_AM_IN_Q1;
  tU32 IFP_DECI45_6_AM_IN_I2;
  tU32 IFP_DECI45_6_AM_IN_Q2;
  tU32 IFP_IIR_LP_A0;
  tU32 IFP_IIR_LP_B1;
  gap32(10);
  tU32 IFP_MPX_STEREO_IN1;
  tU32 IFP_MPX_STEREO_SHIFT1;
  tU32 IFP_MPX_STEREO_MUTE1;
  gap32(1);
  tU32 IFP_MPX_STEREO_IN2;
  tU32 IFP_MPX_STEREO_SHIFT2;
  tU32 IFP_MPX_STEREO_MUTE2;
  gap32(1);
  tU32 IFP_STEREO1_G38CD;
  tU32 IFP_STEREO1_G38SD;
  tU32 IFP_STEREO1_OFFSET;
  tU32 IFP_STEREO1_AFEAMU;
  tU32 IFP_STEREO2_G38CD;
  tU32 IFP_STEREO2_G38SD;
  tU32 IFP_STEREO2_OFFSET;
  tU32 IFP_STEREO2_AFEAMU;
  gap32(208);
  tU32 IFP_RAM1[256];
  tU32 IFP_RAM2[256];
  tU32 IFP_RAM3[128];
  gap32(128);
  tU32 IFP_IFF1_COEFF[16];
  tU32 IFP_IFF2_COEFF[16];
  tU32 IFP_IFF12_COEFF3[16];
  tU32 IFP_HP12_FIR_CSET[5];
  gap32(203);
  tU32 IFP_FSTMIN_OFFSET1;
  tU32 IFP_FADING1_SHIFT;
  tU32 IFP_FSTMIN_OFFSET2;
  tU32 IFP_FADING2_SHIFT;
  gap32(252);
  tU32 IFP_MPX_OUT1;
  tU32 IFP_MPX_HP_OUT1;
  tU32 IFP_FST_OUT1;
  tU32 IFP_MPX_OUT2;
  tU32 IFP_MPX_HP_OUT2;
  tU32 IFP_FST_OUT2;
  tU32 IFP_IFF1_HP_OUT;
  tU32 IFP_IFF2_HP_OUT;
  tU32 IFP_DEM_FST_OUT1;
  tU32 IFP_DEM_MPX_OUT1;
  tU32 IFP_DEM_FST_OUT2;
  tU32 IFP_DEM_MPX_OUT2;
  gap32(4);
  tU32 IFP_DECI45_6_OUT_I1;
  tU32 IFP_DECI45_6_OUT_Q1;
  tU32 IFP_DECI45_6_OUT_I2;
  tU32 IFP_DECI45_6_OUT_Q2;
  tU32 IFP_STEREO_LEFT_OUT1;
  tU32 IFP_STEREO_RIGHT_OUT1;
  tU32 IFP_STEREO_LEFT_OUT2;
  tU32 IFP_STEREO_RIGHT_OUT2;
  tU32 IFP_SIN_FILTER1;
  tU32 IFP_COS_FILTER1;
  tU32 IFP_SIN_FILTER2;
  tU32 IFP_COS_FILTER2;
  gap32(212);
  tU32 IFP_DEBUG_CONFIG;
  tU32 IFP_DEBUG1_DATA1;
  tU32 IFP_DEBUG_DATA2;
  tU32 IFP_DEBUG_STATUS;
  gap32(2388);
  tU32 IFP_ASU_FA_1;
  tU32 IFP_ASU_FB_1;
  tU32 IFP_ASU_C1_1;
  tU32 IFP_ASU_C2_1;
  tU32 IFP_ASU_FST_OFFSET_1;
  tU32 IFP_ASU_HP_OFFSET_1;
  tU32 IFP_ASU_FSTMIN_1;
  tU32 IFP_ASU_FSTA0_1;
  tU32 IFP_ASU_FSTMAX_1;
  tU32 IFP_ASU_HPMAX_1;
  gap32(1);
  tU32 IFP_ASU_TIMECNT_1;
  tU32 IFP_LP_A0_01;
  tU32 IFP_LP_A0_11;
  tU32 IFP_LP_B1_1;
  tU32 IFP_OFFSET_ASU_1;
  tU32 IFP_SHIFT_AM_1;
  tU32 IFP_MAXCNT_1;
  gap32(110);
  tU32 IFP_ASU_FA_2;
  tU32 IFP_ASU_FB_2;
  tU32 IFP_ASU_C1_2;
  tU32 IFP_ASU_C2_2;
  tU32 IFP_ASU_FST_OFFSET_2;
  tU32 IFP_ASU_HP_OFFSET_2;
  tU32 IFP_ASU_FSTMIN_2;
  tU32 IFP_ASU_FSTA0_2;
  tU32 IFP_ASU_FSTMAX_2;
  tU32 IFP_ASU_HPMAX_2;
  gap32(1);
  tU32 IFP_ASU_TIMECNT_2;
  tU32 IFP_LP_A0_02;
  tU32 IFP_LP_A0_12;
  tU32 IFP_LP_B1_2;
  tU32 IFP_OFFSET_ASU_2;
  tU32 IFP_SHIFT_AM_2;
  tU32 IFP_MAXCNT_2;   // 0xF0D9 (ASD_MAXCNT_2 is new name)

  gap32(0x1E000-0xF0D9-1);

  // IFP 3 @ 0x1E000 (IPBus address) / 0x58078000 ARM address
  union
  {
    struct /* bits */
    {
      tU32 ifp3_omode  : 2;
      tU32 cfg_path3   : 1;
      tU32 reserved_1  : 1;
      tU32 dmq_off0on1 : 1;
      tU32 setsel_iff3 : 2;
      tU32 reserved_2  : 2;
      tU32 inmux_iff3  : 1;
      tU32 reserved_3  : 1;
      tU32 inmux_dem3  : 3;
      tU32 inmux_dmq   : 2;
      tU32 reserved_4  : 3;
      tU32 inmux_stm3  : 1;
      tU32 reserved_5  : 1;
      tU32 inmux_dec3  : 1;
      tU32 reserved_6  : 1;
      tU32 ads_highcut_on  : 1;
      tU32 inmux_dsb3  : 2;
      tU32 reserved_7  : 2;
      tU32 dmq_use_fstmin  : 1;
      tU32 dsb3_use_fstcorr  : 1;
      tU32 reserved_8  : 2;
    } bit;
    tU32 reg;
  } IFP_CS2;

  tU32 IFP_IFF3_DATA1_IN;
  gap32(6);
  tU32 IFP_FST_SHIFT3;
  tU32 IFP_FST_SCALE3;
  gap32(2);
  tU32 IFP_DEM_AM_I3;
  tU32 IFP_DEM_AM_Q3;
  gap32(2);
  tU32 IFP_DECI45_6_AM_IN_I3;
  tU32 IFP_DECI45_6_AM_IN_Q3;
  gap32(2);
  tU32 IFP_DEC3_LPA0;
  tU32 IFP_DEC3_LPB1;
  gap32(10);
  tU32 IFP_MPX_STEREO_IN3;
  tU32 IFP_MPX_STEREO_SHIFT3;
  tU32 IFP_MPX_STEREO_MUTE3;
  gap32(5);
  tU32 IFP_STEREO3_G38CD;
  tU32 IFP_STEREO3_G38SD;
  tU32 IFP_STEREO3_OFFSET;
  tU32 IFP_STEREO3_AFEAMU;
  gap32(212);
  tU32 IFP_RAM4[256];
  tU32 IFP_RAM5[256];
  tU32 IFP_RAM6[128];
  gap32(128);
  tU32 IFP_IFF3_COEFF[16];
  gap32(16);
  tU32 IFP_IFF3_COEFF3[16];
  tU32 IFP_HP3_FIR_CSET[5];
  gap32(203);
  tU32 IFP_FSTMIN_OFFSET3;
  tU32 IFP_FADING3_SHIFT;
  gap32(2);
  tU32 IFP_DSB3_IN_FST;
  tU32 IFP_DSB3_IN_MPX;
  tU32 IFP_DMQ_IN_FST;
  tU32 IFP_DMQ_IN_MPX;
  gap32(248);
  tU32 IFP_MPX_OUT3;
  tU32 IFP_MPX_HP_OUT3;
  tU32 IFP_FST_OUT3;
  gap32(3);
  tU32 IFP_IFF3_HP_OUT;
  gap32(1);
  tU32 IFP_DEM_FST_OUT3;
  tU32 IFP_DEM_MPX_OUT3;
  tU32 IFP_IFF3_OUT_I;
  tU32 IFP_IFF3_OUT_Q;
  tU32 IFP_DMQ_OUT_FST;
  tU32 IFP_DMQ_OUT_MPX;
  tU32 IFP_DMQ_OUT_QUC;
  gap32(1);
  tU32 IFP_DECI45_6_OUT_I3;
  tU32 IFP_DECI45_6_OUT_Q3;
  gap32(2);
  tU32 IFP_STEREO_LEFT_OUT3;
  tU32 IFP_STEREO_RIGHT_OUT3;
  gap32(2);
  tU32 IFP_SIN_FILTER3;
  tU32 IFP_COS_FILTER3;
  gap32(6);
  tU32 IFP_DMQ_FSTMEAN;
  tU32 IFP_DMQ_K1;
  tU32 IFP_DMQ_K2;
  tU32 IFP_DMQ_THSD;
  tU32 IFP_DMQ_MUTEDTH;
  gap32(203);
  tU32 IFP_DEBUG3_CONFIG;
  tU32 IFP_DEBUG3_DATA1;
  tU32 IFP_DEBUG3_DATA2;
  tU32 IFP_DEBUG3_STATUS;
  gap32(2388);
  tU32 ASD3_NB_FA_1;
  tU32 ASD3_NB_FB_1;
  tU32 ASD3_NB_C1_1;
  tU32 ASD3_NB_C2_1;
  tU32 ASD3_NB_FST_OFFSET_1;
  tU32 ASD3_NB_HP_OFFSET_1;
  tU32 ASD3_NB_FSTMIN_1;
  tU32 ASD3_NB_FSTA0_1;
  tU32 ASD3_NB_FSTMAX_1;
  tU32 ASD3_NB_HPMAX_1;
  gap32(1);
  tU32 ASD3_NB_TIMECNT_1;
  tU32 ASD3_LP_A0_01;
  tU32 ASD3_LP_A0_11;
  tU32 ASD3_LP_B1_1;
  tU32 ASD3_OFFSET_NB_1;
  tU32 ASD3_SHIFT_AM_1;
  tU32 ASD3_MAXCNT_1;
} IFPMapTy;

//------------------------------------------------------------------------
// VPA
//------------------------------------------------------------------------
//!
//! \struct VPA register map. This interface is mapped at base address
//!         0x5803_A400 on the AHB bus. On the IPBUS the address is 0x0_EE00.
//!
typedef volatile struct VPAMapTy // VPA
{
  tU32 VPA_RAM1[64];
  gap32(1216);
  tU32 VPA_IDATA_IN1;
  tU32 VPA_QDATA_QN1;
  tU32 VPA_IDATA_IN2;
  tU32 VPA_QDATA_QN2;
  tU32 VPA_GR;
  tU32 VPA_GI;
  tU32 VPA_FST_COR;
  gap32(9);
  tU32 VPA_DEBUG_CONFIG;
  tU32 VPA_DEBUG1_DATA1;
  tU32 VPA_DEBUG2_DATA2;
} VPAMapTy;

//------------------------------------------------------------------------
// VPA
//------------------------------------------------------------------------
//!
//! \struct ASD register map. This interface is mapped at base address
//!         0x5803_C000 on the AHB bus. On the IPBUS the address is 0x0_F000.
//!
typedef volatile struct  ASDMapTy // ASD
{
  tU32 ASD_CS1;
  tU32 ASD_CS2;
  tU32 ASD_MAD_MR;
  tU32 ASD_MAD_FA;
  tU32 ASD_MAD_BAD;
  tU32 ASD_ICNT;
  tU32 ASD_ICNT_ALT;
  tU32 ASD_ICNT_ALT1;
  tU32 ASD1_OUT_FST;
  tU32 ASD1_OUT_MPX;
  tU32 ASD1_OUT_MPXHP;
  tU32 ASD2_OUT_FST;
  tU32 ASD2_OUT_MPX;
  tU32 ASD2_OUT_MPXHP;
  tU32 ASD1_OUT_AMI;
  tU32 ASD1_OUT_AMQ;
  tU32 ASD2_OUT_AMI;
  tU32 ASD2_OUT_AMQ;
  tU32 ASD_NB1_INPUT1;
  tU32 ASD_NB1_INPUT2;
  tU32 ASD_NB1_INPUT3;
  tU32 ASD_Y11_1;
  tU32 ASD_DBG1;
  tU32 ASD_DBG2;
  tU32 ASD_DBG3;
  tU32 ASD_DBG4;
  tU32 ASD_DBG5;
  gap32(5);
  tU32 ASD_RAM[256];
  gap32(224);
  tU32 ASD1_IN_MPX;
  tU32 ASD1_IN_MPXHP;
  tU32 ASD1_IN_FST;
  tU32 ASD2_IN_MPX;
  tU32 ASD2_IN_MPXHP;
  tU32 ASD2_IN_FST;
  gap32(65018);

  // IFP3 @ 0x1F000
  tU32 ASD_CS3;
  tU32 ASD_CS4;
  tU32 ASD3_MAD_MR;
  tU32 ASD3_MAD_FA;
  tU32 ASD3_MAD_BAD;
  tU32 ASD3_ICNT;
  tU32 ASD3_ICNT_ALT;
  tU32 ASD3_ICNT_ALT1;
  tU32 ASD3_OUT_FST;
  tU32 ASD3_OUT_MPX;
  tU32 ASD3_OUT_MPXHP;
  gap32(3);
  tU32 ASD3_OUT_AMI;
  tU32 ASD3_OUT_AMQ;
  gap32(2);
  tU32 ASD_NB3_INPUT1;
  gap32(1);
  tU32 ASD_NB3_INPUT3;
  tU32 ASD3_Y11_1;
  tU32 ASD3_DBG1;
  tU32 ASD3_DBG2;
  tU32 ASD3_DBG3;
  tU32 ASD3_DBG4;
  tU32 ASD3_DBG5;
  gap32(5);
  tU32 ASD3_RAM[256];
  gap32(224);
  tU32 ASD3_IN_MPX;
  tU32 ASD3_IN_MPXHP;
  tU32 ASD3_IN_FST;
} ASDMapTy;

//------------------------------------------------------------------------
// FPR
//------------------------------------------------------------------------
//!
//! \struct FPR register map. This interface is mapped at base address
//!         0x5804_4000 on the AHB bus. On the IPBUS the address is 0x1_1000.
//!
typedef struct FPRMapTy // FPR
{
  tU32 FPR_CS1;
  union
  {
    struct /* bits */
    {
      tU32 reserved : 1;
      tU32 is1 : 1;
      tU32 is2 : 1;
      tU32 is3 : 1;
      tU32 is4 : 1;
      tU32 is5 : 1;
      tU32 is6 : 1;
      tU32 is7 : 1;
      tU32 is8 : 1;
      tU32 is9 : 1;
      tU32 is10  : 1;
      tU32 is11  : 1;
      tU32 is12  : 1;
      tU32 is13  : 1;
      tU32 is14  : 1;
      tU32 is15  : 1;
      tU32 is16  : 1;
      tU32 is17  : 1;
      tU32 is18  : 1;
      tU32 is19  : 1;
      tU32 is20  : 1;
      tU32 is21  : 1;
      tU32 is22  : 1;
      tU32 is23  : 1;
      tU32 is24  : 1;
      tU32 is25  : 1;
      tU32 is26  : 1;
      tU32 is27  : 1;
      tU32 is28  : 1;
      tU32 is29  : 1;
      tU32 is30  : 1;
      tU32 is31  : 1;
    } bit;
    tU32 reg;
  } FPR_CIM1;

  union
  {
    struct /* bits */
    {
      tU32 is32  : 1;
      tU32 is33  : 1;
      tU32 is34  : 1;
      tU32 is35  : 1;
      tU32 is36  : 1;
      tU32 is37  : 1;
      tU32 is38  : 1;
      tU32 is39  : 1;
      tU32 reserved : 24;
    } bit;
    tU32 reg;
  } FPR_CIM2;

  tU32 FPR_COEFF_UPDATE;
  tU32 FPR_TAV;
  tU32 FPR_AT;
  tU32 FPR_1RT;
  gap32(241);
  tU32 FPR_CSGCU[8];
  tU32 FPR_IN[40];
  tU32 FPR_XRMS[24];
  tU32 FPR_OUT[40];
  tU32 FPR_XPEAK[24];
  gap32(128);
  tU32 FPR_RAM1_DATA[512];
  tU32 FPR_RAM2_DATA[1024];
} FPRMapTy;

//------------------------------------------------------------------------
// RDS
//------------------------------------------------------------------------

//!
//! \struct RDS Broadcast register map. This interface is mapped at base address
//!         0x5801_0000 on the AHB bus. On the IPBUS the address is 0x0_4000.
//!
typedef volatile struct RDSBroadcastMapTy
{
    //!
    //! \struct RDS Broadcaster Confguration Register (RDS_CS1)
    //!
    struct
    {
        tU32 rds_omode:                 2;
        tU32 padding_0:                 30;
     } rds_cs1;                                // @ 0x5801_0000
} RDSBroadcastMapTy;

//!
//! \struct RDS register map. This interface is mapped at base address
//!         0x5803_E000 on the AHB bus. On the IPBUS the address is 0xF_8000.
//!
typedef volatile struct RDSMapTy
{
    //!
    //! \struct RDS Output Data Register (RDS_OUT1)
    //!
    union
    {
        struct
        {
            tU32 rds1_data:                 16;
            tU32 qc:                        4;
            tU32 bb:                        2;
            tU32 be:                        1;
            tU32 z:                         1;
            tU32 syn:                       1;
            tU32 dok:                       1;
            tU32 bm:                        1;
            tU32 cp:                        5;
        } bit;

        tU32 reg;
    } rds_out1;                                 // @ 0x5803_E000

    gap32(1);

    //!
    //! \struct RDS Confguration Register (RDS_CS1)
    //!
    union
    {
        struct
        {
            tU32 rds_omode:                 2;
            tU32 mon_stb:                   1;
            tU32 rds1_irq_en:               1;
            tU32 rds1_irq_f0t1:             1;
            tU32 rds1_irq_tset:             1;
            tU32 padding_0:                 26;
        } bit;

        tU32 reg;
    } rds_cs1;                                // @ 0x5803_E008

    //!
    //! \struct RDS Output Data Register (RDS_OUT2)
    //!
    union
    {
        struct
        {
            tU32 rds2_data:                 16;
            tU32 qc:                        4;
            tU32 bb:                        2;
            tU32 be:                        1;
            tU32 z:                         1;
            tU32 syn:                       1;
            tU32 dok:                       1;
            tU32 bm:                        1;
            tU32 cp:                        5;
        } bit;

        tU32 reg;
    } rds_out2;                                // @ 0x5803_E00C

    gap32(1);

    //!
    //! \struct RDS Confguration Register (RDS_CS2)
    //!
    union
    {
        struct
        {
            tU32 rds_omode:                 2;
            tU32 mon_stb:                   1;
            tU32 rds2_irq_en:               1;
            tU32 rds2_irq_f0t1:             1;
            tU32 rds2_irq_tset:             1;
            tU32 padding_0:                 26;
        } bit;

        tU32 reg;
    } rds_cs2;                                // @ 0x5803_E014

    //!
    //! \struct RDS Input Data Register (RDS_HI1)
    //!
    union
    {
        struct
        {
            tU32 padding_0:                 8;
            tU32 h1:                        1;
            tU32 h2:                        1;
            tU32 h3:                        1;
            tU32 h5:                        1;
            tU32 padding_1:                 20;
        } bit;

        tU32 reg;
    } rds_hi1;                                // @ 0x5803_E018

    gap32(1);

    //!
    //! \struct RDS Input Data Register (RDS_HI2)
    //!
    union
    {
        struct
        {
            tU32 padding_0:                 8;
            tU32 h1:                        1;
            tU32 h2:                        1;
            tU32 h3:                        1;
            tU32 h5:                        1;
            tU32 padding_1:                 20;
        } bit;

        tU32 reg;
    } rds_hi2;                                // @ 0x5803_E020

    gap32(1);

    //!
    //! \var RDS Reset Access Register (RDS_RSAR1)
    //!
    tU32 rds_rsar1;                           // @ 0x5803_E028

    //!
    //! \var RDS Reset Access Register (RDS_RSAR2)
    //!
    tU32 rds_rsar2;                          // @ 0x5803_E02C
} RDSMapTy;


//!
//! \struct DSP-IPBus bridge register map. This interface is mapped at base address
//!         0x5A80_C400 on the AHB bus. On the IPBUS the address is 0x00A0_3100.
//!
typedef volatile struct DSPIFMapTy
{
  tS32 DSP_CHIN[96];
  tS32 DSP_CHOUT[96];
} DSPIFMapTy;

//------------------------------------------------------------------------
// DBI
//------------------------------------------------------------------------
typedef volatile struct DBIMapTy
{
    tU32 BROADCAST;           // @ offset: 0x000 0x0000_0000

    gap32(32);

    tU32 IRC_CS1;             // @ offset: 0x084 0x0000_0000 : IRC_MODE

    gap32(2);

    tU32 IRC_ISTS;            // @ offset: 0x090 0x0000_0000
    tU32 IRC_IINV;            // @ offset: 0x094 0x0000_0000
    tU32 IRC_IMSK;            // @ offset: 0x098 0x0000_0000 : IRC_INT1_MASK
    tU32 IRC_IEDG;            // @ offset: 0x09C 0x0000_0000

    gap32(1);

    tU32 IRC_D1_INV;          // @ offset: 0x0A4 0x0000_0000
    tU32 IRC_D1_MSK;          // @ offset: 0x0A8 0x0000_0000 : IC_DRQ0_MSK
    tU32 IRC_D1_EDG;          // @ offset: 0x0AC 0x0000_0000

    gap32(1);

    tU32 IRC_D2_INV;          // @ offset: 0x0B4 0x0000_0000
    tU32 IRC_D2_MSK;          // @ offset: 0x0B8 0x0000_0000 : IC_DRQ1_MSK
    tU32 IRC_D2_EDG;          // @ offset: 0x0BC 0x0000_0000

    gap32(16);

    tU32 I2S_MODE;            // @ offset: 0x100 0x0000_0000
    tU32 I2S_SLOT;            // @ offset: 0x104 0x0000_2211
    tU32 I2S_SLOT_FILL;       // @ offset: 0x108 0x0000_8040
    tU32 I2S_FSYNC;           // @ offset: 0x10C 0x0000_0049
    tU32 I2S_BCK;             // @ offset: 0x110 0x0000_0010
    tU32 I2S_BCK_DIV;         // @ offset: 0x114 0x0000_0000
    tU32 I2S_BCK_MIN;         // @ offset: 0x118 0x0000_0000
    tU32 I2S_BCK_MAX;         // @ offset: 0x11C 0x0000_0000
    tU32 I2S_BCK_CNT;         // @ offset: 0x120 0x0000_0000
    tU32 I2S_DEBUG;           // @ offset: 0x124 0x0000_0000

    gap32(55);

    tU32 RFS_CS1;             // @ offset: 0x204 0x0000_0000 : RFS_MODE
    tU32 RFS_TRG_CFG;         // @ offset: 0x208 0x0000_0000 : RFS_TRIGGER_CFG

    gap32(1);

    tU32 RFS_TRG1_CNT;        // @ offset: 0x210 0x0000_0000
    tU32 RFS_MSK1_CNT;        // @ offset: 0x214 0x0000_0000
    tU32 RFS_MOD1_CNT;        // @ offset: 0x218 0x0000_0000
    tU32 RFS_VAL1_CNT;        // @ offset: 0x21C 0x0000_0000

    gap32(57);

    tU32 DRU_CS1;             // @ offset: 0x304 0x0000_0000
    tU32 DRU_WM1;             // @ offset: 0x308 0x0000_0000
    tU32 DRU_WM2;             // @ offset: 0x30C 0x0000_0000
    tU32 DRU_STS;             // @ offset: 0x310 0x0000_0000
    tU32 DRU_FDAT;            // @ offset: 0x314 0x0000_0000

    gap32(827);

    tU32 SFP_CS1;             // @ offset: 0x1004 0x0000_0000

    gap32(2);

    tU32 SFP_INDATI;          // @ offset: 0x1010 0x0000_0000
    tU32 SFP_INDATQ;          // @ offset: 0x1014 0x0000_0000

    gap32(2);

    tU32 DECIN_I;             // @ offset: 0x1020 0x0000_0000
    tU32 DECIN_Q;             // @ offset: 0x1024 0x0000_0000
    tU32 DECOUT_I;            // @ offset: 0x1028 0x0000_0000
    tU32 DECOUT_Q;            // @ offset: 0x102C 0x0000_0000

    gap32(4);

    tU32 NTCIN_I;             // @ offset: 0x1040 0x0000_0000
    tU32 NTCIN_Q;             // @ offset: 0x1044 0x0000_0000
    tU32 NTCOUT_I;            // @ offset: 0x1048 0x0000_0000
    tU32 NTCOUT_Q;            // @ offset: 0x104C 0x0000_0000
    tU32 NTCCO_N1;            // @ offset: 0x1050 0x0000_0000 : SFP_NTC_COEFN1
    tU32 NTCCO_N2;            // @ offset: 0x1054 0x0000_0000 : SFP_NTC_COEFN2
    tU32 NTCCO_D1;            // @ offset: 0x1058 0x0000_0000 : SFP_NTC_COEFD1
    tU32 NTCCO_D2;            // @ offset: 0x105C 0x0000_0000 : SFP_NTC_COEFD2

    gap32(40);

    tU32 DEC_VC;              // @ offset: 0x1100 - 0x115C 0x0000_0000

    gap32(63);

    tU32 BCF_VC;              // @ offset: 0x1200 - 0x12FC 0x0000_0000

    gap32(127);

    tU32 DBIRAM1;             // @ offset: 0x1400 – 0x17FC 0x0000_0000

    gap32(255);

    tU32 DBIRAM2;             // @ offset: 0x1800 – 0x1FFC 0x0000_0000

    gap32(511);

    tU32 DRU_VRAM;            // @ offset: 0x2000 – 0x2FFC 0x0000_0000
} DBIMapTy;


//------------------------------------------------------------------------
// DPMAILBOX
//------------------------------------------------------------------------
typedef volatile struct DPMAILBOXMapTy
{
	tU32 fifo1;										// @ offset: 0x0000
	gap32(3);
	tU32 fifo2;										// @ offset: 0x0010
	gap32(3);
	tU32 fifo3;										// @ offset: 0x0020
	gap32(3);
	tU32 fifo4;										// @ offset: 0x0030
	gap32(115);
	union											// @ offset: 0x0200
	{
		struct
		{
			tU32 fifo1_maxFillLevel :5;
			tU32 fifo1_reset :1;
			tU32 reserved_0 :2;
			tU32 fifo2_minFillLevel :5;
			tU32 fifo2_reset :1;
			tU32 reserved_1 :2;
			tU32 fifo3_maxFillLevel :5;
			tU32 fifo3_reset :1;
			tU32 reserved_2 :2;
			tU32 fifo4_minFillLevel :5;
			tU32 fifo4_reset :1;
			tU32 reserved_3 :1;
			tU32 mode :1;
		} bit;
		tU32 reg;
	} cfg;
} DPMAILBOXMapTy;

#endif // #if defined (PLATFORM_IS_STA661) || defined (PLATFORM_IS_STA662)

#endif // _REGISTERS_STA660_H_

// End of file registers_sta660.h

