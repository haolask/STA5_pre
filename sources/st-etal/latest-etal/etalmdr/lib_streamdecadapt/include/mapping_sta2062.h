// _____________________________________________________________________________
//| FILE:         mapping_sta2062.h
//| PROJECT:      ADR3 - STA660
//|_____________________________________________________________________________
//| DESCRIPTION:  Memory map
//|_____________________________________________________________________________
//| COPYRIGHT:    (c) 2009 STMicroelectronics, Agrate Brianza (MI) (ITALY)
//|
//| HISTORY:
//| Date        | Modification               | Author
//|_____________________________________________________________________________
//| 2009.06.26  | Initial revision           | A^_^L
//|_____________________________________________________________________________

#ifndef _MAPPING_STA2062_H_
#define _MAPPING_STA2062_H_

//--------------------------------------------------------------
// AHB address mapping
//--------------------------------------------------------------
#define SDRAM_BANK_0					0x00000000		// SDRAM Bank 0

#define SDRAM_BANK_1		            0x08000000		// SDRAM Bank 1

#define USB_OTG_FS  					0x10200000		// USB-OTG Full Speed

#define FSMC_BANK_0        		       	0x30000000		// FSMC Bank 0

#define FSMC_BANK_1        		       	0x34000000		// FSMC Bank 1

#define FSMC_BANK_2        		       	0x38000000		// FSMC Bank 2

#define PCCARD_NAND_BANK_0 		       	0x40000000		// PC Card/Nand Flash Bank 0

#define PCCARD_NAND_BANK_1 		       	0x50000000		// PC Card/Nand Flash Bank 1

#define EMBEDDED_ROM					0x80000000		// Embedded ROM (32KB)

#define EMBEDDED_ESRAM_BACKUP			0x80010000		// Embedded SRAM backup (512B)

#define EMBEDDED_SRAM_BANK_0			0xA0000000		// Embedded RAM Bank 0 (64KB)

#define EMBEDDED_SRAM_BANK_1			0xA0020000		// Embedded RAM Bank 1 (64KB)

#define D_TCM                			0xFFFE0000		// D-TCM (8KB)

#define I_TCM                			0xFFFF0000		// I-TCM (8KB)

//--------------------------------------------------------------
// APB address mapping
//--------------------------------------------------------------
#define FSMC_CNTRL        		       	0x10100000		// FSMC Controller

#define SDRAM_CNTRL        		       	0x10110000		// FSMC Controller

#define CLCD_CNTRL         		       	0x10120000		// FSMC Controller

#define DMA1_REG_START_ADDR    		       	0x10130000		// FSMC Controller

#define VIC_CNTRL          		       	0x10140000		// FSMC Controller

#define DMA2_REG_START_ADDR        	       	0x10150000		// FSMC Controller0

#define USB_OTG_HS         		       	0x10170000		// USB OTG High Speed

#define APB_DMA_BRIDGE_EXTENSION    	0x101C0000		// APB DMA Bridge Extensions (MSP3)

//--------------------------------------------------------------
// APB peripheral address mapping
//--------------------------------------------------------------
 // VIC controller
#define VIC_REG_START_ADDR             0x10140000

// APB Audio-Automotive Bridge
#define APB_AHB_CONFIG_REGS				0x10180000		// APB/AHB Configuration registers

#define EFT0_REG_START_ADDR				0x10181000		// Extended Function Timer 0

#define EFT1_REG_START_ADDR				0x10182000		// Extended Function Timer 1

#define CAN0_REG_START_ADDR				0x10183000		// Controller Area Network 0

#define CAN1_REG_START_ADDR				0x10184000		// Controller Area Network 1

#define SPDIF_REG_START_ADDR 			0x10185000		// Sony/Philips Digital Interface

#define C3F_REG_START_ADDR 				0x10186000		// C3 Formator

#define BD_REG_START_ADDR  				0x10187000		// Block Decoder

#define SARAC_REG_START_ADDR			0x10188000		// Sample Rate Converter

#define CHITF_REG_START_ADDR			0x10189000		// Channel Interface

#define EFT2_REG_START_ADDR				0x1018A000		// Extended Function Timer 2

#define EFT3_REG_START_ADDR				0x1018B000		// Extended Function Timer 3

// GPS Subsystem
#define GPS_SUBSYSTEM_RAMSPACE			0x10190000

#define GPS_SUBSYSTEM_PERIPHERAL_SPACE	0x101A0000

#define GPS_SUBSYSTEM_EMERALD_SPACE		0x101B0000

#define MSP3_REG_START_ADDR             0x101C0000

// APB Core Peripheral Bridge
#define SRC_REG_START_ADDR				0x101E0000		// System and Reset Controller

#define WDT_REG_START_ADDR				0x101E1000		// Watchdog Timer

#define MTU0_REG_START_ADDR				0x101E2000		// Multi Timer Unit 0

#define MTU1_REG_START_ADDR				0x101E3000		// Multi Timer Unit 1

#define GPIO0_REG_START_ADDR			0x101E4000		// General Purpose I/Os Bank 0

#define GPIO1_REG_START_ADDR			0x101E5000		// General Purpose I/Os Bank 1

#define GPIO2_REG_START_ADDR			0x101E6000		// General Purpose I/Os Bank 2

#define GPIO3_REG_START_ADDR			0x101E7000		// General Purpose I/Os Bank 3

#if defined (PLATFORM_IS_CARTESIOPLUS)
/* added by maik scholz because GPIO4 needed on C+ */
#define GPIO4_REG_START_ADDR			0x101EA000		// General Purpose I/Os Bank 4
#endif

#define RTC_RTT_PWL_REG_START_ADDR		0x101E8000		// Real Time Clock Unit, Real Time Timer, Pulse Width Light Modulator

#define PMU_REG_START_ADDR				0x101E9000		// Power Management Unit

// APB DMA Bridge
#define MSP2_REG_START_ADDR				0x101F0000		// Multichannel Serial Port 2

#define MSP1_REG_START_ADDR				0x101F1000		// Multichannel Serial Port 1

#define UART2_REG_START_ADDR			0x101F2000		// Asynchronous Serial Port 2

#define I2C2_REG_START_ADDR				0x101F3000		// Inter-Integrated Circuit 2

#define SSP1_REG_START_ADDR				0x101F4000		// Synchronous Serial Port 1

#define SD_SDIO_MMC1_REG_START_ADDR		0x101F5000		// Secure Digital Card, SD IO, Multi-Media Card 1

#define SD_SDIO_MMC0_REG_START_ADDR		0x101F6000		// Secure Digital Card, SD IO, Multi-Media Card 0

#define I2C1_REG_START_ADDR				0x101F7000		// Inter-Integrated Circuit 1

#define I2C0_REG_START_ADDR      		0x101F8000		// Inter-Integrated Circuit 0

#define MSP0_REG_START_ADDR 			0x101F9000		// Multichannel Serial Port 0

#define IRDA_REG_START_ADDR				0x101FA000		// Infrared Data Association

#define UART1_REG_START_ADDR			0x101FB000		// Asynchronous Serial Port 1

#define SSP0_REG_START_ADDR				0x101FC000		// Synchronous Serial Port 0

#define UART0_REG_START_ADDR			0x101FD000		// Asynchronous Serial Port 0

#define UART3_REG_START_ADDR			0x101FE000		// Asynchronous Serial Port 3

#endif // _MAPPING_STA2062_H_

// End of file - mapping_sta2062.h
