//!
//!  \file		cmost_crc.h
//!  \brief		<i><b> CMOST driver </b></i>
//!  \details	CRC Calculation for CMOST driver.
//!  $Author$
//!  \author	(original version) Raffaele Belardi, Roberto Allevi
//!  $Revision$
//!  $Date$
//!

#ifndef CMOST_CRC_H
#define CMOST_CRC_H

#ifdef __cplusplus
extern "C" {
#endif

extern tU32 CMOST_CalculateCrc (tU8 *buf, tU32 len);

#ifdef __cplusplus
}
#endif

#endif // CMOST_CRC_H

// End of file
