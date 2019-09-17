/*!
  @file         sct_types.h
  @brief        <b>  Basic type definitions for ARM  </b>
  @author       CM-AI/PJ-CF42
  @version      0.1 (adapted from DFIRE)
  @date         2009.11.05
  @bug          Unknown
  @warning      None

 Copyright © Robert Bosch Car Multimedia GmbH, 1995-2012
 This code is property of the Robert Bosch Car Multimedia GmbH. Unauthorized
 duplication and disclosure to third parties is prohibited.

*/
#ifndef _SCT_TYPES_
#define _SCT_TYPES_

#ifndef OSIO_H
#include "osalIO.h" /*W.S: forbidden include*/
#endif

#include "types.h"
#include "dab_base_addresses.h"
/* #define boolean unsigned char */

/******************************************************************************
 * DEFINES
 ******************************************************************************/
/*** BIT-DEFS ***************************************************
 */
#define BIT0     0x0001
#define BIT1     0x0002
#define BIT2     0x0004
#define BIT3     0x0008
#define BIT4     0x0010
#define BIT5     0x0020
#define BIT6     0x0040
#define BIT7     0x0080
#define BIT8     0x0100
#define BIT9     0x0200
#define BIT10    0x0400
#define BIT11    0x0800
#define BIT12    0x1000
#define BIT13    0x2000
#define BIT14    0x4000
#define BIT15    0x8000
#define BIT16    0x00010000
#define BIT17    0x00020000
#define BIT18    0x00040000
#define BIT19    0x00080000
#define BIT20    0x00100000
#define BIT21    0x00200000
#define BIT22    0x00400000
#define BIT23    0x00800000
#define BIT24    0x01000000
#define BIT25    0x02000000
#define BIT26    0x04000000
#define BIT27    0x08000000
#define BIT28    0x10000000
#define BIT29    0x20000000
#define BIT30    0x40000000
#define BIT31    0x80000000


#ifndef ASM_IS_INCLUDING   /* the assembler does not understand the following statements */

#define ABS(x) ( ((x) < 0) ? (-(x)) : ((x)) )

/******************************************************************************
 * TYPE DEFINITIONS
 ******************************************************************************/
#ifndef NULL
#define NULL ((void *)0)
#endif

#define HOT  0xffffffff

/* Basic integer types for 8, 16 and 32 bit */
/*  typedef          char     int8;
typedef unsigned char    uint8;
typedef          short   int16;
typedef unsigned short  uint16;
typedef          int    int32;
typedef unsigned int   uint32;
*/

/* Boolean type */
typedef enum {
    DABFALSE = 0,
    DABTRUE  = 1
} DABBool;

/* a switch (on or off) */
typedef enum {
    SWITCH_OFF = 0,
    SWITCH_ON  = 1
} Switch;

typedef enum {
    OK    =  0,
    ERROR = -1,
    BUSY  = -2
} Error;

/* TriBoolean type */
typedef enum {
    FALSE_TRI = 0,
    TRUE_TRI  = 1,
    NO_INFO   = 2
} TriBool;

/* A direction, up or down */
typedef enum {
    DOWN = 0,
    UP   = 1
} Dirctn;

/* A wait for events implementation */
typedef struct {
	DABBool active;        /* are we waiting with this timer           */
    uint event_cntr;    /* the current number of timer event counts */
    uint max_event_cnt; /* the counter value until which we wait    */
} WaitEventsTimer;

/* Sytem mode */
typedef enum {
    DAB = 0,
    FM  = 1,
    AM  = 2,
    DRM = 3,
    HDRADIO = 4,
    INVALID_SMODE = 5
} teSystemMode;

typedef struct {
//	uint8 conc_dec_id;
	tBool state;
	uint8 subch_id;
}concstruct;

/*** EXPORTED FUNCTIONS ********************************************
 */
#define  write_uint32(ADR,VAL) ( (*(volatile uint32*)(ADR)) = (VAL) )

#endif  /* #ifndef ASM_IS_INCLUDING  */
#endif /* #ifndef _SCT_TYPES_ */
