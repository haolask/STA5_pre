/***************************************************************************/
//
//     Name:      memory_config_A2_DSP1.h
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
#define PROG_RAM_SZ         6144

/*-------------------------------------------------------------------------*/
/* XRAM memory                                                             */
/*-------------------------------------------------------------------------*/

/*==========================================================================
 * A2 DSP0 & 1 XRAM map
 * 
 * XRAM_EXT       0x1100 - 0x2003  (3844)     --> _XMEM_EXT
 * IO (XIN+XOUT)  0x1000 - 0x10FF  (128+128)  --> _XMEM_EXT
 * XRAM           0x0000 - 0x0FFF  (4K)       --> _XMEM
 ==========================================================================*/

#define XDATA_RAM_START     0
#define XDATA_RAM_SZ        4096

#define XDATA_IO_START      4096
#define XDATA_IO_SZ         256

//NOTE: XRAM_EXT START = 0x1100 (as per mem mapping) - 4K (implicit offset) = 256
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
#define YDATA_RAM_SZ        4096

#define YDATA_ROM_START     0
#define YDATA_ROM_SZ        0	

/* dummy YDATA_IO - its impossible to define this sector to zero size */
#define YDATA_IO_START      4096
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
