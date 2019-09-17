//!
//!  \file    defines.h
//!  \brief   <i><b> Project common defines </b></i>
//!  \details Project common defines.
//!  \author  Alberto Saviotti
//!

#ifndef DEFINES_H
#define DEFINES_H

#ifdef __cplusplus
extern "C" {
#endif
    
// Unused variables warnings suppressor
#define UNUSED(x) ((void)(x))

// Invalid LUN
#define INVALID_LUN								((tU8)0xFF)

// Broadcast LUN (when applicable)
#define BROADCAST_LUN							((tU8)0xFF)

// Control LUN
#define PROTOCOL_LUN							((tU8)0xFE)

// Invalid Data Type
#define INVALID_DATA_TYPE						((tU8)0xFF)

// Invalid Port Number
#define INVALID_PORT_NUMBER						((tU16)0xFFFF)

// GUI command defines
#define TARGET_MESSAGE_SYNC_BYTE                ((tU8)0x1D)
#define PROTOCOL_LAYER_MESSAGE_SYNC_BYTE        ((tU8)0x2D)

// Timeout
#define MAX_TIMEOUT_WHILECYCLE					6000

#ifdef __cplusplus
}
#endif

#endif // DEFINES_H

// End of file
