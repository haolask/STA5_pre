/*********************************************************FileHeaderBegin*****
 *
 * FILE:
 *     types.h
 *
 * REVISION:
 *
 * AUTHOR:
 *     Christian Mittendorf, Christian.Mittendorf@de.bosch.com
 *
 * CREATED:
 *     20.06.2003
 *
 * DESCRIPTION:
 *     Types for DAB verification.
 *
 * NOTES:
 *
 *
 ***********************************************************FileHeaderEnd*****/

 /******************************************************************************
 * Copyright (C) Blaupunkt GmbH, 2003
 * This software is property of Blaupunkt GmbH. Unauthorized
 * duplication and disclosure to third parties is prohibited.
 *****************************************************************************/

 /******************************************************************************
 * ChangeLog:
 * Revision 1.3  2010/10/06 13:10:16  snm2hi
 * cvs key words removed
 *
 * Revision 1.1.1.1  2010/10/01 15:37:55  gol2hi
 * Import from STM
 *
 * Revision 1.2  2010/04/20 12:42:01  bmg2hi
 * changed Bool to DABBool
 *
 * Revision 1.1  2009/08/24 12:56:07  lrc2hi
 * first check in of DAB-IP test functions. Adapted from Arion DAB-IP test. First step, 2 tests adapted (test_dab_system_2_3_4 (full path test) and dab_system_1 (register test)).
 *
 * Revision 1.1  2008/07/21 14:43:04  mfc2hi
 * initial version
 *
 * Revision 1.8  2004/02/12 08:49:45  christianm
 * FIG_0_0_LENGTH_CHECK switched off
 *
 * Revision 1.7  2004/01/14 11:35:00  christianm
 * 'signed' added to typedef
 *
 * Revision 1.6  2004/01/13 11:23:53  christianm
 * define 'MPEG_AUDIO_HEADER' deleted
 *
 * Revision 1.5  2004/01/13 11:21:44  christianm
 * define 'FIC_BUFFER_SIZE' deleted
 *
 * Revision 1.4  2004/01/12 14:13:04  christianm
 * newline added
 *
 * Revision 1.3  2003/12/19 12:58:32  christianm
 * address section deleted
 *
 * Revision 1.2  2003/12/18 13:28:07  christianm
 * file "dab_base_addresses.h" included
 *
 * Revision 1.1  2003/12/17 13:29:41  christianm
 * initial version:
 * main DAB-IP test function
 *
 *
 * ChangeLogEnd
 ******************************************************************************/
/* define Boolean type */
#ifndef __dab_defines_types_h
#define __dab_defines_types_h


#if 0 	/* use just global types.h"  */
typedef enum {
    DABFALSE = 0,
    DABTRUE  = 1
} DABBool;



/******************************************************************************
 * switches
 ******************************************************************************/
//#define FIB_DEC
//#define FLASH_CODE_POLL_REGISTER
#define PROD_TEST

/* defines f�r FIB-Decoder */
//#define FIG_0_0_LENGTH_CHECK
//#define PRINT_FIB_DEBUG



/******************************************************************************
 * MISC. DEFINES
 ******************************************************************************/


/******************************************************************************
 * TYPEDEFs
 ******************************************************************************/
/* old typedefs */
typedef          char     int8;
typedef unsigned char    uint8;
typedef          short    int16;
typedef unsigned short   uint16;
typedef          int     int32;
typedef unsigned int     uint32;
//typedef unsigned int     uint;


/* Typendefinitionen nach CM-Richtlinie */
typedef signed   char     tS8;
typedef unsigned char     tU8;
typedef signed   short    tS16;
typedef unsigned short    tU16;
typedef signed   int      tS32;
typedef unsigned int      tU32;




#define INT_CTRL00_TYPE (volatile unsigned int*)


/******************************************************************************
 * VARIABLES
 ******************************************************************************/

/******************************************************************************
 * MACROS
 ******************************************************************************/
#define INT_MS(base_addr) (INT_CTRL00_TYPE (base_addr  + 0x00 ))	// INT Mask Set

#endif /* #if 0 */

#endif
/* end of file types.h */


