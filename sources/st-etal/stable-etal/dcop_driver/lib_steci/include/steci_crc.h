//!
//!  \file 		 steci_crc.h
//!  \brief 	 <i><b> STECI CRC interface </b></i>
//!  \details Interface file for CRC fucntionalities.
//!  \author 	Alberto Saviotti
//!

#ifndef STECICRC_H
#define STECICRC_H

#ifdef __cplusplus
extern "C" {
#endif

// This module provides members for CRC calculation.
// It implements a CRC7 algorithm based on polynomial:
//    x^7 + x^3 + 1
// with initial and final masks equal to 0.
// For fast operations the look-up table is generated
extern tVoid STECI_CrcInit (tVoid);

// Functions for CRC calculation
extern tU8 STECI_CalculateCRC (tU8 *messagePtr, tS32 length);

// Functions for PARITY calculation
extern tU8 STECI_CalculateParity (tU8 *messagePtr, tS32 length);

// Function for CRC validation
extern tS32 STECI_ValidateCRC (tU8 crc, tU8 *messagePtr, tU32 length);

#ifdef __cplusplus
}
#endif

#endif // STECICRC_H

// End of file

