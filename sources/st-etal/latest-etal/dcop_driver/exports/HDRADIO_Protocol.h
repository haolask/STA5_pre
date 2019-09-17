//!
//!  \file 		HDRADIO_Protocol.h
//!  \brief 	<i><b> HDRADIO driver </b></i>
//!  \details   
//!  $Author$
//!  \author 	(original version) Roberto Allevi
//!  $Revision$
//!  $Date$
//!

#ifndef HDR_PROTOCOL_H
#define HDR_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tunerdriver_types.h"

/**************************************
 * Defines and macros
 *************************************/
#define CP_PAYLOAD_INDEX          5
#define CP_OVERHEAD               8

#ifdef CONFIG_COMM_HDRADIO_I2C
#define LM_PHYDRIVER_OVERHEAD     4
#else
#define LM_PHYDRIVER_OVERHEAD     0
#endif // CONFIG_COMM_HDRADIO_I2C

// Logical message header length
#define LM_OVERHEAD						    13

// Logical message maximum payload length
#define LM_PAYLOAD_MAX_LEN					2035

#define LM_FULL_SIZE						(LM_PAYLOAD_MAX_LEN + LM_OVERHEAD + LM_PHYDRIVER_OVERHEAD)

// Logical message start payload data position
#define LM_PAYLOAD_INDEX					12

// The time taken by the HDRADIO to load the FW from flash and execute it
#ifdef CONFIG_HOST_OS_TKERNEL
	#define HDRADIO_FW_LOADING_TIME         4000
#else //!CONFIG_HOST_OS_TKERNEL
	#define HDRADIO_FW_LOADING_TIME         2500
#endif //CONFIG_HOST_OS_TKERNEL

/***********************************
 * Types
 **********************************/
/*!
 * \enum	tyHDRADIOInstanceID
 * 			For HD Radio devices supporting multiple channels,
 * 			defines the channel (instance in HD Radio specification
 * 			terminology).
 * \remark	assumed to fit in one byte, checked in #ETAL_runtimecheck
 */
typedef enum {
	/*! Undefined, indicates un-initialized or error entry */
	INSTANCE_UNDEF = 0x00,
	/*! first instance */
	INSTANCE_1     = 0x01,
	/*! second instance */
	INSTANCE_2     = 0x02
} tyHDRADIOInstanceID;

/*!
 * \enum	HDR_cmdEnumTy
 * 			For HD Radio command type,
 * 			Write commands either have no associated data, or 
 * 			they download data or parameters into the BBP; request commands request data from the BBP.
 * 			These command types are distinguished since they require slightly different actions by the Logical
 * 			Messaging Layer.
 */
typedef enum
{
	HDR_CMD_UNDEF_TYPE = -1,
	HDR_CMD_READ_TYPE = 0x00,
	HDR_CMD_WRITE_TYPE = 0x01
} HDR_cmdEnumTy;

/*!
 * \struct	tyHDRADIODeviceConfiguration
 * 			Describes the HDRADIO device configuration
 */
typedef struct
{
	/*! Communication bus type (SPI or I2C) */
	tyCommunicationBusType 		communicationBusType;
	/*! SPI or I2C bus configuration */
	union
	{
		/*! SPI bus configuration */
		tySpiCommunicationBus	spi;
		/*! I2C bus configuration */
		tyI2cCommunicationBus	i2c;
	} communicationBus;
	/*! GPIO where the HDRADIO DCOP's RSTN line is connected */
	tU32 						GPIO_RESET;
} tyHDRADIODeviceConfiguration;


/**************************************
 * Function prototypes
 *************************************/
tS32 HDRADIO_init(tyHDRADIODeviceConfiguration *deviceConfiguration);
tS32 HDRADIO_setTcpIpAddress(tChar *IPAddress);
tS32 HDRADIO_deinit(tVoid);
tS32 HDRADIO_reset(tVoid);
tS32 HDRADIO_reconfiguration(tyHDRADIODeviceConfiguration *deviceConfiguration);
tS32 HDRADIO_Write_Command(tyHDRADIOInstanceID instId, tU8 *buf, tU32 buf_len);
tS32 HDRADIO_Read_Response(tyHDRADIOInstanceID *pInstId, tU8 *buf, tS32* buf_len);
tS32 HDRADIO_Write_Segment_Command(tyHDRADIOInstanceID instId, tU16 nb_data_segments, tU16 data_segment_nb, tU8 *buf, tU32 buf_len);
tS32 HDRADIO_Read_Segment_Status(tyHDRADIOInstanceID instId, tU8 u8Opcode, tU16 u16NumOfSeg, tU16 u16SegNum, tU8 *buf, tS32 *buf_len);
tS32 HDRADIO_Read_Raw_Data(tU8 *buf, tU32 buf_len);
tS32 HDRADIO_Write_Raw_Data(tU8 *buf, tU32 buf_len);
#ifdef __cplusplus
}
#endif

#endif // HDR_PROTOCOL_H

//EOF
