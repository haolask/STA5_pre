//!
//!  \file    dcop_boot.h
//!  \brief   <i><b> DCOP boot header </b></i>
//!  \details Interface file for DCOP boot functionalities.
//!  \author  Alberto Saviotti
//!

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///

// Status return of special functions 
#define FLASH_STATUS_ERROR_MEMORY	        ((tSInt)-9)
#define FLASH_STATUS_ERROR_SOURCENOTAVAIL	((tSInt)-8)
#define FLASH_STATUS_ERROR_READ     	    ((tSInt)-7)
#define FLASH_STATUS_ERROR_WRITE    	    ((tSInt)-6)
#define FLASH_STATUS_ERROR_BOOTSTRAP	    ((tSInt)-5)
#define FLASH_STATUS_ERROR_ERASE			((tSInt)-4)
#define FLASH_STATUS_ERROR_DUMP				((tSInt)-3)
#define FLASH_STATUS_ERROR_PROGRAM			((tSInt)-2)
#define FLASH_STATUS_ERROR_GENERIC			((tSInt)-1)
#define FLASH_STATUS_OK						((tSInt)0)

// Timeouts
#define DCOP_TIMEOUT_FLASH_ERASE			((tS64)6000)
#define DCOP_TIMEOUT_FLASH_BULKERASE		((tU32)50000) // tS64 in STECI but SPLINT doesn't like it
#define DCOP_TIMEOUT_FLASH_WRITE			((tS64)6000)
#define DCOP_TIMEOUT_FLASH_READ				((tU32)2000)  // tS64 in STECI but SPLINT doesn't like it

///
// Enums
///

///
// Types
///

///
// Exported functions
///
extern tSInt DCOP_ReadPort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataPtr, tU32 timeoutValue, tU8 *outputBuffer);
extern tSInt DCOP_WritePort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataPtr, tU8 *outputBuffer);

#ifdef __cplusplus
}
#endif

// End of file
