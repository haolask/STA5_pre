//!
//!  \file 		dabmw_crc.c
//!  \brief 	<i><b> CRC functions for Dab Middleware </b></i>
//!  \details	This file contains CCITT CRC implementation
//!  \author 	Alberto Saviotti
//!  \author 	(original version) Alberto Saviotti
//!  \version 	n.a.
//!  \date 		2011.12.16
//!  \bug 		Unknown
//!  \warning	None
//!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osal.h"

#include "dabmw_crc.h"

#ifdef __cplusplus
extern "C" {
#endif

// The CRC's are computed using polynomials. The  coefficients
// for the algorithms are defined by the following constants
#define P_CCITT                     (tU16)0x1021

static tBool DABMW_CRCtabccitt_init = false;

static tU16 DABMW_CRC_tabccitt[256];

static tVoid DABMW_InitCRCcccitt (tVoid);

tU16 DABMW_CRCccitt (tU16 crc, tU8 byte)
{
    tU16 tmp;
    tU16 short_c;

    short_c = (tU16)0x00FF & (tU16)byte;

    if (false == DABMW_CRCtabccitt_init)
    {
        DABMW_InitCRCcccitt ();
    }

    tmp = (crc >> 8) ^ short_c;
    crc = (crc << 8) ^ DABMW_CRC_tabccitt[tmp];

    return crc;
}

static tVoid DABMW_InitCRCcccitt (tVoid) 
{
    tSInt i, j;
    tU16 crc, c;

    for (i = 0; i < 256; i++) 
    {
        crc = 0;
        c   = ((tU16)i) << 8;

        for (j = 0; j < 8; j++) 
        {
            if ((crc ^ c) & (tU16)0x8000) 
            {
                crc = (crc << 1) ^ P_CCITT;
            }
            else 
            {
                crc = crc << 1;
            }

            c = c << 1;
        }

        DABMW_CRC_tabccitt[i] = crc;
    }

    DABMW_CRCtabccitt_init = true;
}

#ifdef __cplusplus
}
#endif

// End of file
    
