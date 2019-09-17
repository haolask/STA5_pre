//!
//!  \file		cmost_crc.cpp
//!  \brief		<i><b> CMOST driver </b></i>
//!  \details	CRC Calculation for CMOST driver.
//!  $Author$
//!  \author	(original version) Raffaele Belardi, Roberto Allevi
//!  $Revision$
//!  $Date$
//!

#include "target_config.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_CMOST)

#include "osal.h"

#include "cmost_crc.h"

/***************************
 *
 * CMOST_CalculateCrc
 *
 **************************/
/*!
 * \brief		Calculates the CRC of a CMOST command or response buffer
 * \details		The checksum is calculated as 24-bit modulo of the sum of
 * 				all the command words, including header.
 * \param[in]	buf - data
 * \param[in]	len - number of bytes in *buf*
 * \return		the calculated CRC
 * \callgraph
 * \callergraph
 */
tU32 CMOST_CalculateCrc (tU8 *buf, tU32 len)
{
	tU32 u32Checksum = 0;
	tU32 u32Index;

	for (u32Index = 0; u32Index < len; u32Index++)
	{
		tU32 u32Elem, u32Shift[] = {16, 8, 0};

		u32Elem = (tU32)buf[u32Index];
		u32Checksum += (u32Elem << u32Shift[(u32Index % 3)]);
	}

	u32Checksum &= 0xFFFFFF;

	return u32Checksum;
}

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_ETAL_SUPPORT_CMOST

//EOF

