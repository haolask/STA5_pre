//!
//!  \file    hdr_lld.h
//!  \brief   <i><b> HDRadio low level driver header file </b></i>
//!  \details HDRadio (STA680) low level driver functionalities.
//!           This module interface with the physical communication channel.
//!           This module present the "Bus Controller Layer" for the HDRadio.
//!  \author  Alberto Saviotti
//!

#ifndef HDR_LLD_H
#define HDR_LLD_H

#ifdef __cplusplus
extern "C" {
#endif

    // Status return of special functions 
#define HD_FLASH_STATUS_ERROR_MEMORY	        ((tSInt)-9)
#define HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL	((tSInt)-8)
#define HD_FLASH_STATUS_ERROR_READ     	        ((tSInt)-7)
#define HD_FLASH_STATUS_ERROR_WRITE    	        ((tSInt)-6)
#define HD_FLASH_STATUS_ERROR_BOOTSTRAP	        ((tSInt)-5)
#define HD_FLASH_STATUS_ERROR_ERASE			    ((tSInt)-4)
#define HD_FLASH_STATUS_ERROR_DUMP				((tSInt)-3)
#define HD_FLASH_STATUS_ERROR_PROGRAM			((tSInt)-2)
#define HD_FLASH_STATUS_ERROR_GENERIC			((tSInt)-1)
#define HD_FLASH_STATUS_OK						((tSInt)0)

   // Define error codes
#define HDR_STATUS_SUCCESS                  ((tS32)0)  // Operation correctly executed

#define HDR_STATUS_ERROR                  ((tS32)-1) // Generic error
#define HDR_STATUS_CMD_COMMAND_FAILURE    ((tS32)-2)
#define HDR_STATUS_DEV_NOT_RESPONDING     ((tS32)-3)
#define HDR_STATUS_PORT_NOT_OPENED        ((tS32)-4)
#define HDR_STATUS_INTERMEDIATE_FRAME     ((tS32)-5) // Used to indicate that a frame that is not the last has been received
#define HDR_STATUS_INVALID_LUN            ((tS32)-6)
#define HDR_STATUS_CMD_ONGOING            ((tS32)-7)
#define HDR_STATUS_DEV_NOT_READY          ((tS32)-8)
#define HDR_STATUS_CMD_PARAMETER_WRONG    ((tS32)-9)
#define HDR_STATUS_CMD_NOT_SUPPORTED      ((tS32)-10)
#define HDR_STATUS_INVALID_HANDLER        ((tS32)-11)
#define HDR_STATUS_NO_RESOURCES           ((tS32)-12)
#define HDR_STATUS_DATA_NOT_PRESENT       ((tS32)-14) // USed to indicate that a zero content frame has been received
#define HDR_STATUS_HEADER_RE_TX_FLAG      ((tS32)-15) // Used to indicate re-tx flag raised
#define HDR_STATUS_HEADER_PARITY_ERROR    ((tS32)-16) // Used to indicate that received header is corrupted
#define HDR_STATUS_DATA_CRC_ERROR         ((tS32)-17) // Used to indicate that received data is corrupted
#define HDR_STATUS_HOST_DEVICE_CLASH      ((tS32)-19) // Used to indicate both thinking to rx

#ifdef __cplusplus
}
#endif

#endif // HDR_LLD_H

// End of file
