// _____________________________________________________________________________
//| FILE:         mapping_sta660.h
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
//| 2010.09.09  | Added STA662 mapping       | Luigi Cotignano
//|_____________________________________________________________________________

#ifndef _MAPPING_STA660_H_
#define _MAPPING_STA660_H_

//**************************************************************
//*              STA662 PERIPHERALS MAPPING                    *
//**************************************************************

//--------------------------------------------------------------
// AHB address mapping
//--------------------------------------------------------------
#define ROM								    0x00000000		// ROM (64KB)

#define RAM0							    0x10000000		// RAM Bank 0 (64KB)

#define RAM1				                0x10010000		// RAM Bank 1 (64KB)

#define RAM2				                0x10020000		// RAM Bank 1 (64KB)

#define RAM3				                0x10030000		// RAM Bank 1 (64KB)

#define D_TCM                			    0x04000000		// D-TCM (8KB)

#define I_TCM                			    0x00000000		// I-TCM (128KB)

#ifdef PLATFORM_IS_STA661
//--------------------------------------------------------------
// AHB address Alias buffered
//--------------------------------------------------------------

#define TDI1_BUFF				            0x40000000		// TDI1

#define TDI2_BUFF				            0x42000000		// TDI2

#define AHB2IP1_BUFF				        0x44000000		// AHB2IP1

#define AHB2IP2_BUFF				        0x48000000		// AHB2IP2

#define APB0_BUFF				        	0x60000000		// APB0

#define APB1_BUFF				        	0x68000000		// APB1

#define APBC_BUFF				        	0x6C000000		// APBC

#define EME_BUFF				        	0x6F800000		// EMERALD

#define RAM_BUFF							0x80000000		// RAM Bank 0

#define RAM0_BUFF							0x80000000		// RAM Bank 0

#define RAM1_BUFF				            0x80010000		// RAM Bank 1

#define RAM2_BUFF				            0x80020000		// RAM Bank 1

#define RAM3_BUFF				            0x80030000		// RAM Bank 1

#define XP70_DTCM_BUFF 						0xA0000000		// XP70_DTCM

#define XP70_L2PMEM_BUFF					0xA0400000		// XP70_L2PMEM

#endif //PLATFORM_IS_STA661
//--------------------------------------------------------------
// AHB address Alias unbuffered
//--------------------------------------------------------------
#define TDI1_UNBUFF				            0x50000000		// TDI1

#define TDI2_UNBUFF				            0x52000000		// TDI2

#define AHB2IP1_UNBUFF				        0x54000000		// AHB2IP1

#define AHB2IP2_UNBUFF				        0x58000000		// AHB2IP2

#define APB0_UNBUFF				        	0x70000000		// APB0

#define APB1_UNBUFF				        	0x78000000		// APB1

#define APBC_UNBUFF				        	0x7C000000		// APBC

#ifdef PLATFORM_IS_STA662
#define EME_UNBUFF				        	0xA0000000		// EMERALD
#else
#define EME_UNBUFF				        	0x7F800000		// EMERALD
#endif // PLATFORM_IS_STA662

#define ROM_UNBUFF							0x98000000		// ROM

#define RAM_UNBUFF							0x90000000		// RAM Bank 0

#define RAM0_UNBUFF							0x90000000		// RAM Bank 0

#define RAM1_UNBUFF				            0x90010000		// RAM Bank 1

#define RAM2_UNBUFF				            0x90020000		// RAM Bank 1

#define RAM3_UNBUFF				            0x90030000		// RAM Bank 1

#define XP70_DTCM_UNBUFF 					0xB0000000		// XP70_DTCM

#define XP70_L2PMEM_UNBUFF					0xB0400000		// XP70_L2PMEM
//--------------------------------------------------------------
// RAM ALTERNATE VECTOR AND VIC
//--------------------------------------------------------------
#define RAM_ALTVECT				            0x90030000		// RAM Bank 1

#define VIC_REG_START_ADDR					0xFFFFF000		// VIC

#ifdef PLATFORM_IS_STA661
//--------------------------------------------------------------
// APB peripheral address mapping BUFFERED
//--------------------------------------------------------------
#define DMA1_REG_BUFF            			0x20000000

#define DMA2_REG_BUFF             			0x22000000

#define DBI_REG_BUFF             			0x24000000

#define RS1_REG_BUFF						0x26000000

#define RS2_REG_BUFF						0x28000000

#define DAB_REG_BUFF						0x2A000000

//--------------------------------------------------------------
// TO KEEP THE SAME NAME OF STA206x
//--------------------------------------------------------------
#define DMA_0_CNTRL            			    0x20000000   // DMA 1

#define DMA_1_CNTRL             		    0x22000000   // DMA 2

#endif // PLATFORM_IS_STA661
//--------------------------------------------------------------
// APB peripheral address mapping UNBUFFERED
//--------------------------------------------------------------
#define DMA1_REG_UNBUFF            			0x30000000      // DMA1 Controller

#define DMA2_REG_UNBUFF             		0x32000000      // DMA2 Controller

#define DBI_REG_UNBUFF             			0x34000000      // DBI Interface

#define RS1_REG_UNBUFF						0x36000000      // Reed Solomon Decoder 1

#define RS2_REG_UNBUFF						0x38000000      // Reed Solomon Decoder 2

#define DAB_REG_UNBUFF						0x3A000000      // DAB IP

//--------------------------------------------------------------
// AIF peripheral address mapping
//--------------------------------------------------------------

#define AIF1_REG_START_ADDR            	    0x58048000		// AIF1
#define AIF_1                               (*(volatile AIFMap *)(AIF1_REG_START_ADDR))

#define AIF2_REG_START_ADDR            	    0x58054000		// AIF2
#define AIF_2                               (*(volatile AIFMap *)(AIF2_REG_START_ADDR))

#define BCO_REG_START_ADDR            	    0x58031040		// BCO
#define BCO                                 (*(volatile BCOMap *)(BCO_REG_START_ADDR))

#define FEI_REG_START_ADDR               	  0x58094000      // FEI
#define FEI                                 (*(volatile FEIMapTy *)(FEI_REG_START_ADDR))

#define SLINK_REG_START_ADDR               	0x58080000      // SLINK
#define SLINK                               (*(volatile SLINKMapTy *)(SLINK_REG_START_ADDR))

#define IFP_REG_START_ADDR                 0x58038000
#define IFP                                (*(volatile IFPMapTy *)(IFP_REG_START_ADDR))

#define DPMAILBOX_REG_START_ADDR			0x54000000		// DPMAILBOX
#define DPMAILBOX 							(*(volatile DPMAILBOXMapTy *)(DPMAILBOX_REG_START_ADDR))

#define VPA_REG_START_ADDR                 0x5803A400
#define VPA                                (*(volatile VPAMapTy *)(VPA_REG_START_ADDR))

#define ASD_REG_START_ADDR                 0x5803C000
#define ASD                                (*(volatile ASDMapTy *)(ASD_REG_START_ADDR))

#define FPR_REG_START_ADDR                 0x58044000
#define FPR                                (*(volatile FPRMapTy *)(FPR_REG_START_ADDR))

#define DSPIF_REG_START_ADDR                0x5A80C400      // DSPIF
#define DSPIF                              (*(volatile DSPIFMapTy *)(DSPIF_REG_START_ADDR))

//--------------------------------------------------------------
// APB0 APB1 APBC peripheral address mapping
//--------------------------------------------------------------
#define DMA1_REG_START_ADDR            		0x30000000      // DMA1 Controller

#define DMA2_REG_START_ADDR            		0x32000000      // DMA2 Controller

#define MTU_REG_START_ADDR            	    0x70000000		// TIMER

// APB Audio-Automotive Bridge
#define UART0_REG_START_ADDR			    0x70800000		// UART0

#define UART1_REG_START_ADDR			    0x71000000		// UART1

#define UART2_REG_START_ADDR			    0x71800000		// UART2

#define SSP0_REG_START_ADDR				    0x72000000		// SSP0

#define SSP1_REG_START_ADDR				    0x72800000		// SSP1

#define SSP2_REG_START_ADDR				    0x73000000		// SSP2

#define SSP3_REG_START_ADDR				    0x73800000		// SSP3

#define SSP4_REG_START_ADDR				    0x74000000		// SSP4

#define I2C0_REG_START_ADDR				    0x74800000		// I2C

#define GPIO0_REG_START_ADDR			    0x75000000		// GPIO0

#define GPIO1_REG_START_ADDR			    0x75800000		// GPIO1

#define GPIO2_REG_START_ADDR			    0x76000000		// GPIO2

#define GPIO3_REG_START_ADDR			    0x76800000		// GPIO3

#define GPIO4_REG_START_ADDR			    0x77000000		// GPIO4

#define GPIO5_REG_START_ADDR			    0x77800000		// GPIO5

#ifdef PLATFORM_IS_STA662
#define GPIO6_REG_START_ADDR			    0x7F800000		// GPIO6

#define GPIO7_REG_START_ADDR			    0x7F880000		// GPIO7
#endif

#define SPI0_REG_START_ADDR				    0x78000000		// SPI0

#define SPI1_REG_START_ADDR				    0x78800000		// SPI1

#define SPI2_REG_START_ADDR				    0x79000000		// SPI2

#define SPI3_REG_START_ADDR				    0x79800000		// SPI3

#ifdef PLATFORM_IS_STA662
#define SMU_REG_START_ADDR				    0x7C000000		// SMU
#else
#define SMU_REG_START_ADDR				    0x6C000000		// SMU
#endif // PLATFORM_IS_STA662

#define RS1_REG_START_ADDR				    0x7C800000		// RS1 decoder

#define RS2_REG_START_ADDR				    0x7D000000		// RS2 decoder

#define WDT_REG_START_ADDR				    0x7E000000		// WatchDog

#define EFT_REG_START_ADDR				    0x7E800000		// EFT

#define ESAI_REG_START_ADDR				    0x7F000000		// ESAI

#define RDS_BROADCAST_REG_START_ADDR        0x58010000      // RDS Broadcast

#define RDS_REG_START_ADDR                  0x5803E000      // RDS
#define RDS                                (*(volatile RDSMapTy *)(RDS_REG_START_ADDR))


#endif // _MAPPING_STA660_H_

// End of file - mapping_sta660.h

