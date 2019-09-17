//!
//!  \file    cmost_lld.h
//!  \brief   <i><b> CMOST LLD header file </b></i>
//!  \details CMOST Low Levl Drivers related fucntionalities.
//!  \author  Raffaele Belardi
//!

#ifndef CMOST_LLD_H
#define CMOST_LLD_H

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///

#if defined (CONFIG_HOST_OS_WIN32)
#define TX_BUFFER_LEN_BYTES			    2048
#define TX_BUFFER_TOTAL_LEN_BYTES	    (TX_BUFFER_LEN_BYTES * 8)

///
// Const
///
#define MSB_RISING_EDGE_CLOCK_BYTE_IN		0x20;
#define MSB_FALLING_EDGE_CLOCK_BYTE_IN		0x24;
#define MSB_FALLING_EDGE_CLOCK_BYTE_OUT		0x11;
#define MSB_RISING_EDGE_CLOCK_BIT_IN		0x22;
#define MSB_FALLING_EDGE_CLOCK_BIT_IN		0x26;
#endif

///
// Exported functions
///
extern tU32 CMOST_WriteReadData (tVoid *devicehandler, tU8 *sourceBufferPnt, tU16 bytesToWrite,
									tU8 *destinationBufferPnt, tU16 bytesToRead, tU32 deviceID, tBool commkeptActive);


extern tS32 CMOST_ReadRaw (tVoid *devicehandler, tU8 *buf, tU32 len, tU32 deviceID,
                           tBool i2CStartOn, tBool i2CStopOn);

extern tS32 CMOST_WriteRaw (tVoid *devicehandler, tU8 *buf, tU32 len, tU32 deviceID);

extern tVoid CMOST_ResetDevice (tVoid *devicehandler, tU32 deviceID);

#ifdef __cplusplus
}
#endif

#endif // CMOST_LLD_H

// End of file
