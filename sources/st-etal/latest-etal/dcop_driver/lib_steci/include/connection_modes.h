//!
//!  \file    connection_modes.h
//!  \brief   <i><b> Data related to teh connection mode </b></i>
//!  \details Data contained in this file is exposed to the main functions and to threads in
//!           order to setup the connection.
//!  \author  Alberto Saviotti
//!

#ifndef CONNECTION_MODES_H
#define CONNECTION_MODES_H

#ifdef __cplusplus
extern "C" {
#endif

// Define the transmission method, wire means that no particular protocol
// is used. A2B or ETHERNET mean that a certain protocol has to be applied 
// when data is going to or retrieved from the device
typedef enum
{
	DEV_TXMODE_WIRE = 0,
	DEV_TXMODE_A2B = 1,
	DEV_TXMODE_ETHERNET = 2,
	DEV_TXMODE_UNKNOWN = 0xFF
} DEV_txModeEnumTy;

#ifdef __cplusplus
}
#endif

#endif // CONNECTION_MODES_H

// End of file
