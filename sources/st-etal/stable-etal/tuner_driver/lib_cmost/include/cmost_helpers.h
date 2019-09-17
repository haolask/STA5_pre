//!
//!  \file		cmost_helpers.h
//!  \brief		<i><b> CMOST driver </b></i>
//!  \details	CRC helper functionalities
//!  $Author$
//!  \author	(original version) Raffaele Belardi, Roberto Allevi
//!  $Revision$
//!  $Date$
//!

#ifndef CMOST_HELPERS_H
#define CMOST_HELPERS_H


#ifdef __cplusplus
extern "C" {
#endif


/**************************************
 * Types
 *************************************/
/*!
 * \enum	CMOST_accessSizeEnumTy
 * 			Access size for CMOST driver (function #CMOST_helperSendMessage)
 * \remark	assumed to fit in one byte, checked in #ETAL_runtimecheck
 */
typedef enum
{
	/*! Use 32 bit access size, typically used only for Firmware or Patch download */
    CMOST_ACCESSSIZE_32BITS = 0,
	/*! Use 24 bit access size, normal mode */
    CMOST_ACCESSSIZE_24BITS = 1
} CMOST_accessSizeEnumTy;

/*!
 * \enum	CMOST_cmdModeEnumTy
 * 			Mode of operation for the CMOST driver (function #CMOST_helperSendMessage)
 * 			Normally only the following modes are used:
 * 			- CMOST_CMDMODE_RD
 * 			- CMOST_CMDMODE_WR
 * 			- CMOST_CMDMODE_CMD_GEN
 * 			- CMOST_CMDMODE_W_BOOT
 * 			- CMOST_CMDMODE_RESET
 *
 * \remark	assumed to fit one byte, checked in #ETAL_runtimecheck
 */
typedef enum
{
    CMOST_CMDMODE_CMD     = 0,
	/*!
	  Reads one or more words from the CMOST memory using the raw protocol.
	  - *access_size* must be #CMOST_ACCESSSIZE_32BITS
	  - *buf* format is:
	    + buf + 0 = address MSB
	    + buf + 1 = address mid byte
	    + buf + 2 = address LSB
	    + buf + 3 = size in 32-bit words MSB
	    + buf + 4 = size in 32-bit words mid byte
	    + buf + 5 = size in 32-bit words LSB

	     'address' is the address in the CMOST address space starting from which the 'size'
	     words will be read

	     'size' is the number of 32-bit words to read from the CMOST address space

	  - *buf_len* must be set to 6.
	  - *resp* must point to a buffer at least 'size' * 4 bytes big
	 */
    CMOST_CMDMODE_RD      = 1,
	/*!
	  Sends a data buffer to the CMOST device using the raw protocol
	  that is prior to the availability of the CMOST command interpreter.
	  - *access_size* must be #CMOST_ACCESSSIZE_32BITS
	  - *buf* format is:
	    + buf + 0 = address MSB
	    + buf + 1 = address mid byte
	    + buf + 2 = address LSB
		
		followed by an integer number 'n' of firmware 32-bit words

		'address' is the address in the CMOST address space where the chunk of firmware
		will be written to.
	  - *buf_len* must be set to 'n', i.e. the number of 32-bit firmware words included in *buf*
	  - *resp* must point to a buffer at least 1 byte big

	  The response produced by the driver is a dummy response in this case and can
	  be safely ignored.
	 */
    CMOST_CMDMODE_WR      = 2,
    CMOST_CMDMODE_RD_DMA  = 3,
    CMOST_CMDMODE_WR_DMA  = 4,
	/*!
	  Sends a command to the CMOST using the command interpreter protocol and
	  reads the response.
	  - *access_size* must be #CMOST_ACCESSSIZE_24BITS
	  - *buf* format is:
	   + buf + 0 = command header MSB
	   + buf + 1 = command header mid byte
	   + buf + 2 = command header LSB

	    (optionally:)

	   + buf + 3 = first parameter MSB
	   + buf + 4 = first parameter mid byte
	   + buf + 5 = first parameter LSB
	   + ...
	 
	    'command header' is the CMOST protocol header formed by 'flags, command ID, number of parameters'
	    (for details refer to the CMOST API documentation).

	    'number of parameters' is command-dependent, but each parameter must be 3-bytes long.
	    The 3-byte checksum is added by the CMOST driver.

	  - *buf_len* depends on the command size.
	  - *resp* must point to a buffer capable of holding the largest answer from CMOST,
	    which is 93 bytes (3 command header bytes plus 30 parameters, 3 bytes each)
	 */
    CMOST_CMDMODE_CMD_GEN = 5,
	/*!
	  Sends a data buffer to the CMOST device using the raw protocol
	  that is prior to the availability of the CMOST command interpreter.
	  - *access_size* must be #CMOST_ACCESSSIZE_32BITS
	  - *buf* format is:
	    +  buf + 0 = address MSB
		+  buf + 1 = address mid byte
		+  buf + 2 = address LSB
	     followed by an integer number 'n' of firmware 32-bit words

	     'address' is the address in the CMOST address space where the chunk of firmware
	     will be written to.

	  - *buf_len* must be set to 'n', i.e. the number of 32-bit firmware words included in *buf*
	  - *resp* must point to a buffer at least 1 byte big

	  The response produced by the driver is a dummy response in this case and can
	  be safely ignored.
	 */
    CMOST_CMDMODE_W_BOOT  = 6,
	/*!
	  Resets the CMOST device.
	  - *access_size* must be set to #CMOST_ACCESSSIZE_24BITS
	  - *buf* is not used but must be allocated, a 1-byte buffer is sufficient
	  - *buf_len* is set to 1
	  - *resp* must point to a buffer at least 1 byte big
	 
	  The response produced by the driver is a dummy response in this case and can
	  be safely ignored.
	 */
    CMOST_CMDMODE_RESET   = 7,
    CMOST_CMDMODE_IDLE    = 0xFF
} CMOST_cmdModeEnumTy;

/*!
 * \def		CMOST_addressEnumTy
 * 			List of known addresses for CMOST driver, both for
 * 			SPI and I2C. Wether the addresses listed here are
 * 			actually used in a system is application-dependent.
 * \remark	assumed to fit one byte, checked in ETAL_runtimecheck
 */
typedef enum
{
	/*! SPI chip select 1 */
	CMOST_SPI_CS1 = 1,
	/*! SPI chip select 2 */
	CMOST_SPI_CS2 = 2,
	/*! SPI chip select 3 */
	CMOST_SPI_CS3 = 3,
	/*! I2C address 1 */
	CMOST_I2CADDR_C0 = 0xC0,
	/*! I2C address 2 */
	CMOST_I2CADDR_C2 = 0xC2,
	/*! I2C address 3 */
	CMOST_I2CADDR_C8 = 0xC8
} CMOST_addressEnumTy;

typedef struct
{
    tU8  *DataBuffer;
    tU32 TotalBytesNum;
    tU32 RemainingBytesNum;
    CMOST_accessSizeEnumTy paramSize;
    CMOST_cmdModeEnumTy mode;
	tU8 devAddress;
} CMOST_txRxDataTy;

// Parameter structure
typedef struct
{
    tVoid *devHandler;
    tU32 deviceID;
    tBool useI2c;
	tU8   i2Caddr;             // If use i2C  possible addresses are 0xC2, 0xC8, 0xC0 
					           // If use SPI possible addresses are 0x00 default CS1,  0x01 CS2,  0x02 CS3	
    tU8 communicationOptions;  // in SPI mode bit0 =1 keeps CS active (low) at the end of transmitted frame
    CMOST_txRxDataTy txRxData;
} CMOST_paramsTy;

tSInt CMOST_helperSendMessage(CMOST_paramsTy *param, tU8 *buf, tU32 buf_len, CMOST_cmdModeEnumTy mode, CMOST_accessSizeEnumTy access_size, tU32 deviceID, tU8 *resp, tU32 *resp_lenp);


#ifdef __cplusplus
}
#endif

#endif // CMOST_HELPERS_H

// End of file
