//!
//!  \file 		 rif_steci_crc.cpp
//!  \brief 	 <i><b> STECI CRC module </b></i>
//!  \details This module implements STECI CRC fucntionalities.
//!  \author 	Alberto Saviotti
//!

#include "target_config.h"

#if (!defined (CONFIG_COMM_DRIVER_EMBEDDED) || !defined (CONFIG_ETAL_SUPPORT_DCOP_MDR))

#include <stdio.h> // for size_t in steci_helpers.h
#include "steci_helpers.h"
#include "steci_crc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CRC_TABLE_SIZE          256

static tU8 crcPoly;

static tU8 crcTable[CRC_TABLE_SIZE];

static tU8 STECI_AddCRC (tU8 crc, tU8 message_byte);

static tVoid STECI_GenerateCRCTable (tVoid);

tVoid STECI_CrcInit (tVoid)
{
    // The value of our CRC-7 polynomial x^7 + x^3 + 1
    crcPoly = (tU8)0x89;

    // Generate the table for fast CRC calculation
    STECI_GenerateCRCTable ();
}

static tVoid STECI_GenerateCRCTable (tVoid)
{
    tS32 i, j;

    // Generate a table value for all 256 possible byte values
    for (i = 0; i < 256; i++)
    {
        crcTable[i] = (tU8)(((i & 0x80) != 0) ? i ^ crcPoly : i);

        for (j = 1; j < 8; j++)
        {
            crcTable[i] <<= 1;
            if ((crcTable[i] & 0x80) != 0)
            {
                crcTable[i] ^= crcPoly;
            }
        }
    }
}

// Adds a message byte to the current CRC-7 to get a the new CRC-7
tU8 STECI_AddCRC (tU8 crc, tU8 message_byte)
{
    return crcTable[(crc << 1) ^ message_byte];
}

// Returns the CRC-7 for a message of "length" bytes
tU8 STECI_CalculateCRC (tU8 *messagePtr, tS32 length)
{
    tU8 crc = (tU8)0;
    tS32 cnt;

    for (cnt = 0; cnt < length; cnt++)
    {
        crc = STECI_AddCRC (crc, *(messagePtr + cnt));
    }

    return crc;
}

tU8 STECI_CalculateParity (tU8 *messagePtr, tS32 length)
{
    tS32 cnt;
    tU8 byteVal = (tU8)0;
    tU8 parity = (tU8)0;

    for (cnt = 0; cnt < length; cnt++)
    {
        byteVal = *(messagePtr + cnt);

        byteVal ^= byteVal >> 4;
        byteVal ^= byteVal >> 2;
        byteVal ^= byteVal >> 1;
        byteVal &= 1;

        parity ^= byteVal;
    }

    return parity;
}

tS32 STECI_ValidateCRC (tU8 crc, tU8 *messagePtr, tU32 length)
{
    tS32 res = STECI_STATUS_SUCCESS;
    tU32 cnt;
    tU8 txCrc = crc;

    crc = (tU8)0;

    for (cnt = 0; cnt < length; cnt++)
    {
        crc = STECI_AddCRC (crc, *(messagePtr + cnt));
    }

    crc = txCrc ^ crc;

    if ((tU8)0 != crc)
    {
        res = STECI_STATUS_DATA_CRC_ERROR;
    }

    return res;
}

#ifdef __cplusplus
}
#endif

#endif // CONFIG_COMM_DRIVER_EMBEDDED &&  !CONFIG_ETAL_SUPPORT_DCOP_MDR

// End of file
