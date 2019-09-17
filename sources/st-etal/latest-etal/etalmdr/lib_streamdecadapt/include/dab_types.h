/**********************************************************FileHeaderBegin*****
 *
 * FILE:
 *     dab_types.h
 *
 * REVISION:
 *
 * AUTHOR:
 *     Markus Stepen, FV/SLM, Robert Bosch GmbH
 *
 * CREATED:
 *     10. July 1998
 *
 * DESCRIPTION:
 *     Header file containing types and constants of DAB
 *
 * NOTES:
 *     Project D-Fire II
 *
 * MODIFIED:
 *     date, name of modifier
 *         description of modification
 *
 ***********************************************************FileHeaderEnd******/

/******************************************************************************
 * Copyright (C) Robert Bosch GmbH, 1998
 * This software is property of Robert Bosch GmbH. Unauthorized
 * duplication and disclosure to third parties is prohibited.
 ******************************************************************************/

/******************************************************************************
 * ChangeLog:
 * Revision 1.4  2011/02/04 09:11:50  bmg2hi
 * merged st import
 *
 * Revision 1.1.1.2  2010/11/09 11:45:44  gol2hi
 * Import from STM
 *
 * Revision 1.2  2010/10/06 13:10:16  snm2hi
 * cvs key words removed
 *
 * Revision 1.1.1.1  2010/10/01 15:37:58  gol2hi
 * Import from STM
 *
 * Revision 1.1  2009/08/24 12:56:09  lrc2hi
 * first check in of DAB-IP test functions. Adapted from Arion DAB-IP test. First step, 2 tests adapted (test_dab_system_2_3_4 (full path test) and dab_system_1 (register test)).
 *
 * Revision 1.1  2008/07/21 14:34:18  mfc2hi
 * initial version
 *
 * Revision 1.1  2003/12/17 13:34:26  christianm
 * initial version:
 * taken from D-FIRE software
 *
 *
 * ChangeLogEnd
 ******************************************************************************/

#ifndef _DAB_TYPES_H_
#define _DAB_TYPES_H_

/******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "types.h"
#include "dab_defines_types.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

/* The maximum number of subchannels in  a DAB ensemble */
#define MAX_NUM_SUBCH_PER_ENSEMBLE  64u

/* The number of capacity units in a CIF */
#define NUM_CU_PER_CIF  864u

/* smallest possible subchannel occupies 4 CUs (prot.-level 4A) */
#define NUM_CU_SMALLEST_POSSIBLE_SUBCH 4

/* invalid (inactive) TDI and VIT params */
#define INVALID_SUBCH_ID   64u
#define INVALID_CU       1023u
#define INVALID_VIT_LP      0u

/* number of bytes of a FIB (ex- or including CRC) */
#define LEN_FIB          30
#define LEN_FIB_WITH_CRC 32

/* number of CIFs in a 384ms period and log-2 of this number */
#define NUM_CIFS_FOR_384MS    16
#define LD_NUM_CIFS_FOR_384MS  4 /* attention */

/* maximum TII Main Identifier */
#define NUM_TII_MAIN_IDS 70

/******************************************************************************
 * TYPE DEFINITIONS
 ******************************************************************************/
#ifndef ASM_IS_INCLUDING      /* the assembler does not understand the following statements */

/* A DAB transmission mode */
typedef enum {
    NO_DAB_MODE  = 0,
    DAB_MODE_I   = 1,
    DAB_MODE_II  = 2,
    DAB_MODE_III = 3,
    DAB_MODE_IV  = 4
} DABTransmMode;

/* A DAB ensemble centre frequency (in kHz, 19 bit) */
typedef uint32 DABFreq;

/* A DAB service identifier */
typedef uint32 ServID;

/* A DAB service component Identifier */
typedef uint32 ServCmpntID;

/* A DAB subchannel Identifier */
typedef uint32 SubchID;

/* A CU index in a CIF (valid range is 0..NUM_SUBCHS_PER_ENSEMBLE-1) */
typedef uint32 CUIndex;

/* A bit error rate */
typedef uint32 BER;


#endif /* ASM_IS_INCLUDING */
#endif /* #ifndef _DAB_TYPES_H_ */
