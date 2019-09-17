/*********************************************************FileHeaderBegin*****
 *
 * FILE:
 *     dab_base_addresses.h
 *
 * REVISION:
 *
 * AUTHOR:
 *     Christian Mittendorf, Christian.Mittendorf@de.bosch.com
 *
 * CREATED:
 *     18.12.2003
 *
 * DESCRIPTION:
 *     DAB base addresses (excalibur board)
 *
 * NOTES:
 *
 * MODIFIED:
 *
 *
 ***********************************************************FileHeaderEnd*****/

 /****************************************************************************
 * Copyright (C) Blaupunkt GmbH, 2003
 * This software is property of Blaupunkt GmbH. Unauthorized
 * duplication and disclosure to third parties is prohibited.
 *****************************************************************************/

 /******************************************************************************
 * ChangeLog:
 * Revision 1.5  2009/10/08 11:49:46  lrc2hi
 * shared memory size adapted
 *
 * Revision 1.4  2009/10/05 12:41:06  lrc2hi
 * first version of data path test for DAB-IP path 3
 *
 * Revision 1.3  2009/09/18 14:15:04  lrc2hi
 * bug fix: use correct base address of shared memory
 *
 * Revision 1.2  2009/09/10 15:37:37  lrc2hi
 * Update of DAB base adresses
 *
 * Revision 1.1  2009/08/24 12:56:06  lrc2hi
 * first check in of DAB-IP test functions. Adapted from Arion DAB-IP test. First step, 2 tests adapted (test_dab_system_2_3_4 (full path test) and dab_system_1 (register test)).
 *
 * Revision 1.4  2009/03/30 12:16:54  rem2hi
 * update according to new ADR3 spec (2 DAB-IP pathes, modified memory map)
 *
 * Revision 1.3  2008/10/28 10:05:37  mfc2hi
 * minor changes
 *
 * Revision 1.2  2008/07/24 12:58:24  mfc2hi
 * SHARED_MEMORY_SIZE changed
 *
 * Revision 1.1  2008/07/21 14:28:17  mfc2hi
 * initial version
 *
 * ChangeLogEnd
 ******************************************************************************/

#ifndef _DAB_BASE_ADDRESSES_H_
#define _DAB_BASE_ADDRESSES_H_

/*****************************************************************************
 *  includes                                                                 *
 *****************************************************************************/

/*****************************************************************************
 *  defines                                                                  *
 *****************************************************************************/

#define SHARED_MEMORY_BASE_ADR          0x3A000000
#define SHARED_MEMORY3_BASE_ADR          0x3A040000

#define SHARED_MEMORY_SIZE              0x1FFFF
#define SHARED_MEMORY3_SIZE              0x1FFFF

#define EXCAL_DF2_CPI_BASE_ADDRESS      0x3A080000

/* CU-Pointer (2*2*864 bytes) base address */
#define CU_POINTER_PAGE_0               (SHARED_MEMORY_BASE_ADR + 0x0000)
#define CU_POINTER_PAGE_1               (SHARED_MEMORY_BASE_ADR + 0x06C0)

/* TFPR and TII symbol address */
#define TII_SYMBOL_ADDRESS              (SHARED_MEMORY_BASE_ADR + 0x7000)
#define TFPR_SYMBOL_ADDRESS             (SHARED_MEMORY_BASE_ADR + 0x8000)

/* start addresses subchannel parameter */
#define SUBCHANNEL_PARAMETER_PAGE_0     (SHARED_MEMORY_BASE_ADR + 0x0E00)
#define SUBCHANNEL_PARAMETER_PAGE_1     (SHARED_MEMORY_BASE_ADR + 0x1000)

/* start addresses STD parameter */
#define STD_PARAMETER_PAGE_0            (SHARED_MEMORY_BASE_ADR + 0x0D80)
#define STD_PARAMETER_PAGE_1            (SHARED_MEMORY_BASE_ADR + 0x0DC0)

/* TDI memory start address */
#define TDI_METRIC_MEMORY_START_ADDRESS (SHARED_MEMORY_BASE_ADR + 0xA000)
#define TDI_METRIC_MEMORY_SIZE          0x36000
                                        /* ( = 864 CUs * 256 bytes ) */

/* TIBFE memory start address */
#define TIBFE_FLSTD_PAGE0               (SHARED_MEMORY_BASE_ADR + 0x4140)
#define TIBFE_FLSTD_PAGE1               (SHARED_MEMORY_BASE_ADR + 0x4160)

/* TDI Cache memory start address */
#define TDI_CACHE_PAGE0                 (SHARED_MEMORY_BASE_ADR + 0x4180)
#define TDI_CACHE_PAGE1                 (SHARED_MEMORY_BASE_ADR + 0x44C0)

/* MDS memory start address */
#define MDS_START_ADDRESS               (SHARED_MEMORY_BASE_ADR + 0x4800)
#define MDS_SAMPLE_BUFFER               (SHARED_MEMORY_BASE_ADR + 0x5000)

/* FFT-FIC memory start address */
#define FFT_FIC_SYMBOL_ADDRESS          (SHARED_MEMORY_BASE_ADR + 0x9000)

/* BBF Sampels memory start address */
#define BBF_SAMPELS_PAGE0               (SHARED_MEMORY_BASE_ADR + 0x4800)
#define BBF_SAMPELS_PAGE1               (SHARED_MEMORY_BASE_ADR + 0x7000)

/* start addresses of buffers written by the Excalibur DAB signal path */
#define FIC_BUFFER_START_PAGE_A         (SHARED_MEMORY_BASE_ADR + 0x1200)
#define FIC_BUFFER_START_PAGE_B         (SHARED_MEMORY_BASE_ADR + 0x1390)

// #define FIC_BUFFER_START_PORT_0_PAGE_A  0x50001200
// #define FIC_BUFFER_START_PORT_0_PAGE_B  0x50001390

#define MSC_BUFFER_START_PAGE_A         (SHARED_MEMORY_BASE_ADR + 0x1520)
#define MSC_BUFFER_START_PAGE_B         (SHARED_MEMORY_BASE_ADR + 0x2B30)


/* base address of shared memory, from which it is set to zero at test start */
#define SHARED_MEMORY_SET_ZERO_START   (SHARED_MEMORY_BASE_ADR  + 0x0D80)
#define SHARED_MEMORY_SIZE_ZERO        (SHARED_MEMORY_BASE_ADR + SHARED_MEMORY_SIZE - SHARED_MEMORY_SET_ZERO_START)


/* CU-Pointer (2*2*864 bytes) base address */
#define CU3_POINTER_PAGE_0               (SHARED_MEMORY3_BASE_ADR + 0x0000)
#define CU3_POINTER_PAGE_1               (SHARED_MEMORY3_BASE_ADR + 0x06C0)

/* TFPR and TII symbol address */
#define TII3_SYMBOL_ADDRESS              (SHARED_MEMORY3_BASE_ADR + 0x7000)
#define TFPR3_SYMBOL_ADDRESS             (SHARED_MEMORY3_BASE_ADR + 0x8000)

/* start addresses subchannel parameter */
#define SUBCHANNEL3_PARAMETER_PAGE_0     (SHARED_MEMORY3_BASE_ADR + 0x0E00)
#define SUBCHANNEL3_PARAMETER_PAGE_1     (SHARED_MEMORY3_BASE_ADR + 0x1000)

/* start addresses STD parameter */
#define STD3_PARAMETER_PAGE_0            (SHARED_MEMORY3_BASE_ADR + 0x0D80)
#define STD3_PARAMETER_PAGE_1            (SHARED_MEMORY3_BASE_ADR + 0x0DC0)

/* TDI memory start address */
#define TDI3_METRIC_MEMORY_START_ADDRESS (SHARED_MEMORY3_BASE_ADR + 0xA000)
#define TDI3_METRIC_MEMORY_SIZE          0x36000
                                        /* ( = 864 CUs * 256 bytes ) */

/* TIBFE memory start address */
#define TIBFE3_FLSTD_PAGE0               (SHARED_MEMORY3_BASE_ADR + 0x4140)
#define TIBFE3_FLSTD_PAGE1               (SHARED_MEMORY3_BASE_ADR + 0x4160)

/* TDI Cache memory start address */
#define TDI3_CACHE_PAGE0                 (SHARED_MEMORY3_BASE_ADR + 0x4180)
#define TDI3_CACHE_PAGE1                 (SHARED_MEMORY3_BASE_ADR + 0x44C0)

/* MDS memory start address */
#define MDS3_START_ADDRESS               (SHARED_MEMORY3_BASE_ADR + 0x4800)
#define MDS3_SAMPLE_BUFFER               (SHARED_MEMORY3_BASE_ADR + 0x5000)

/* FFT-FIC memory start address */
#define FFT3_FIC_SYMBOL_ADDRESS          (SHARED_MEMORY3_BASE_ADR + 0x9000)

/* BBF Sampels memory start address */
#define BBF3_SAMPELS_PAGE0               (SHARED_MEMORY3_BASE_ADR + 0x4800)
#define BBF3_SAMPELS_PAGE1               (SHARED_MEMORY3_BASE_ADR + 0x7000)

/* start addresses of buffers written by the Excalibur DAB signal path */
#define FIC3_BUFFER_START_PAGE_A         (SHARED_MEMORY3_BASE_ADR + 0x1200)
#define FIC3_BUFFER_START_PAGE_B         (SHARED_MEMORY3_BASE_ADR + 0x1390)

// #define FIC_BUFFER_START_PORT_0_PAGE_A  0x50001200
// #define FIC_BUFFER_START_PORT_0_PAGE_B  0x50001390

#define MSC3_BUFFER_START_PAGE_A         (SHARED_MEMORY3_BASE_ADR + 0x1520)
#define MSC3_BUFFER_START_PAGE_B         (SHARED_MEMORY3_BASE_ADR + 0x2B30)


/* base address of shared memory, from which it is set to zero at test start */
#define SHARED_MEMORY3_SET_ZERO_START   (SHARED_MEMORY3_BASE_ADR  + 0x0D80)
#define SHARED_MEMORY3_SIZE_ZERO        (SHARED_MEMORY3_BASE_ADR + SHARED_MEMORY3_SIZE - SHARED_MEMORY3_SET_ZERO_START)



/* base address offset of CPI register blocks */
#define CPI_FRI1_BASE        0x0100
#define CPI_FRI3_BASE        0x0400
#define CPI_NTC1_BASE        0x0120
#define CPI_NTC3_BASE        0x0420
#define CPI_BBF1_BASE        0x0110
#define CPI_BBF3_BASE        0x0410
#define CPI_FSY1_BASE        0x0140
#define CPI_FSY2_BASE        0x02A0
#define CPI_FSY3_BASE        0x0440
#define CPI_TIB1_BASE        0x0280
#define CPI_TIB2_BASE        0x02c0
#define CPI_TIB3_BASE        0x0580
#define CPI_TIBFE1_BASE      0x0160
#define CPI_TIBFE3_BASE      0x0460
#define CPI_MDS_BASE         0x0180
#define CPI_MDS3_BASE        0x0480
#define CPI_FFT_BASE         0x01C0
#define CPI_FFT3_BASE        0x04C0
#define CPI_DEM_BASE         0x01E0
#define CPI_DEM3_BASE        0x04E0
#define CPI_TDI_BASE         0x0200
#define CPI_TDI3_BASE        0x0500
#define CPI_VIT_BASE         0x0240
#define CPI_VIT3_BASE        0x0540
#define CPI_MWP_BASE         0x0020
#define CPI_MWP3_BASE        0x0320
#define CPI_IRC_BASE         0x0700
#define CPI_GCC_BASE         0x0620
#define CPI_ETP_BASE         0x0640
#define CPI_FES_BASE         0x0600
#define CPI_IRS_BASE         0x0660
#define CPI_CPI_BASE         0x0680

/* ARM register base address */


/*****************************************************************************
 *  prototypes                                                               *
 *****************************************************************************/

/*****************************************************************************
 *  global variables                                                         *
 *****************************************************************************/

#endif
/* DAB_BASE_ADDRESSES */

