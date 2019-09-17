/***************************************************************************/
//
//     Name:      memory_config_A2_DSP2.h
//     Purpose:   Accordo2 Emerald memory configuration
//     Author:    Christophe Quarre
//     Revision:  1
//     Modified:  2013-07-03
//     Copyright: STMicroelectronics
//
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/* Program memory                                                          */
/*-------------------------------------------------------------------------*/
#define PROG_ROM_START      0
#define PROG_ROM_SZ         0
#define PROG_RAM_START      0
#define PROG_RAM_SZ         8192

/*-------------------------------------------------------------------------*/
/* XRAM memory                                                             */
/*-------------------------------------------------------------------------*/

/*==========================================================================
 * A2 DSP2 XRAM map
 * 
 * XRAM_EXT       0x3100 - 0x4003  ()         --> _XMEM_EXT
 * IO (XIN+XOUT)  0x3000 - 0x30FF  (128+128)  --> _XMEM_EXT
 * XRAM           0x0000 - 0x2FFF  (12K)      --> _XMEM
 ==========================================================================*/

#define XDATA_RAM_START     0
#define XDATA_RAM_SZ        12288

#define XDATA_IO_START      12288
#define XDATA_IO_SZ         256

//NOTE: XRAM_EXT START = 0x3100 (as per mem mapping) - 12K (implicit offset) = 256
#define XDATA_RAM_EXT_START 256
//NOTE: XDATA_RAM_EXT_SZ = 4K - 256 IO + 4 special registers = 3844
#define XDATA_RAM_EXT_SZ    3844


//the following are not available in Accordo2

#define XDATA_ROM_START     0
#define XDATA_ROM_SZ        0

#define XDATA_ROM_EXT_START	0
#define XDATA_ROM_EXT_SZ    0

#define XDATA_RW_START      0
#define XDATA_RW_SZ         0

/*-------------------------------------------------------------------------*/
/* YRAM memory                                                             */
/*-------------------------------------------------------------------------*/
#define YDATA_RAM_START     0
#define YDATA_RAM_SZ        12288

#define YDATA_ROM_START     0
#define YDATA_ROM_SZ        0	

/* dummy YDATA_IO - its impossible to define this sector to zero size */
#define YDATA_IO_START      12288
#define YDATA_IO_SZ         2

#define YDATA_RAM_EXT_START 0
#define YDATA_RAM_EXT_SZ    0

#define YDATA_ROM_EXT_START	0
#define YDATA_ROM_EXT_SZ    0

/*
 * Default real sizes for input/output lines. These values can be
 * redefined at command level when compiling your application (options
 * -xin, -xout and -par of <name>cc and ws_<name>cc).
 */
#ifndef XIN_SIZE
#define XIN_SIZE            128
#endif

#ifndef XOUT_SIZE
#define XOUT_SIZE           128
#endif

#ifndef PARAM_SIZE
#define PARAM_SIZE          0
#endif
