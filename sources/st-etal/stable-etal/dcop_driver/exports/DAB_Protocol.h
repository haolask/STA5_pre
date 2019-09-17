//!
//!  \file 		DAB_Protocol.h
//!  \brief 	<i><b> DCOP DAB driver </b></i>
//!  \details   
//!  $Author$
//!  \author 	(original version) Jean-Hugues Perrin
//!  $Revision$
//!  $Date$
//!

#ifndef DAB_PROTOCOL_H
#define DAB_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tunerdriver_types.h"

#define DAB_FW_LOADING_TIME         1500


	
//
// If defined implements a minimum interval different than zero between two transmissions host to device. Used with IMG's peripherals in FPGA
//
//#define STECI_LIMITING_TX_TIMEFRAME
#undef STECI_LIMITING_TX_TIMEFRAME
	
//
// If defined checks the undefined MISO data for high level. Used with IMG's peripherals in FPGA
//
//#define STECI_MISO_HIGH_LEVEL_UNDEFINED_DATA
#undef STECI_MISO_HIGH_LEVEL_UNDEFINED_DATA
	
//
// May be used for boot protocol, disable to align PC and Accordo2 (ETAL) versions
//
//#define STECI_USE_LARGE_PAYLOAD
#undef STECI_USE_LARGE_PAYLOAD
	
//
// If defined uses a fixed frame size regardless the dimension of message to transfer. Used for example with IMG's peripherals in FPGA
//
//#define STECI_FIXED_PAYLOAD_LENGTH
#undef STECI_FIXED_PAYLOAD_LENGTH
	
// STECI related defines
// Clash management
#ifdef STECI_LIMITING_TX_TIMEFRAME
#define STECI_TX_MIN_TIMEFRAME_MS           49
#endif
	
// Data bytes
#ifdef STECI_MISO_HIGH_LEVEL_UNDEFINED_DATA
	#define STECI_MISO_UNDEFINED_DATA_BYTE      0xFF
#else
	#define STECI_MISO_UNDEFINED_DATA_BYTE      0x00
#endif
	
// Frame format
#define STECI_HEADER_LENGTH                 4
	
#ifdef STECI_USE_LARGE_PAYLOAD
	#define STECI_MAX_PAYLOAD_LENGTH            16380
#elif (defined STECI_FIXED_PAYLOAD_LENGTH)
	#define STECI_MAX_PAYLOAD_LENGTH            60
#else
	#define STECI_MAX_PAYLOAD_LENGTH            4088
#endif
	
#define STECI_MAX_PACKET_LENGTH             (STECI_HEADER_LENGTH + STECI_MAX_PAYLOAD_LENGTH)
	
#if (defined STECI_FIXED_PAYLOAD_LENGTH) && (! defined STECI_USE_LARGE_PAYLOAD)
#define STECI_PAYLOAD_MODULO                STECI_MAX_PAYLOAD_LENGTH
#define STECI_PACKET_MODULO                 STECI_MAX_PACKET_LENGTH // (It must be ^2)
#else
#define STECI_PAYLOAD_MODULO                4
#define STECI_PACKET_MODULO                 4 // (It must be ^2)
#endif

#define STECI_MAX_MESSAGE_LEN               ((tU32)(STECI_MAX_PACKET_LENGTH * 2))


// Commands
#define STECI_NORMAL_CMD                                ((tU8)0x00)
#define STECI_FLASH_CMD                                 ((tU8)0x01)
#define STECI_RESET_CMD                                 ((tU8)0x07)
#define STECI_BOOT_CMD                                  ((tU8)0x08)

///
// Enums
///

// Flash specific sub-commands
typedef enum
{
    STECI_BOOT_MODE_NORMAL    = 0,
    STECI_BOOT_MODE_FLASH     = 2
} DCOP_bootModeEnumTy;

// Flash specific sub-commands
typedef enum
{
    STECI_FLASH_OP_MODE_IS_NONE    = 0,
    STECI_FLASH_OP_MODE_IS_BOOTSTRAP = 1,
    STECI_FLASH_OP_MODE_IS_ERASE = 2,
    STECI_FLASH_OP_MODE_IS_PROGRAM = 3,
    STECI_FLASH_OP_MODE_IS_DUMP    = 4,
    STECI_FLASH_OP_MODE_IS_FLASHER_CHK = 5
} DCOP_flashModeEnumTy;

typedef enum
{
    STECI_TARGET_SPI2FLASH = 0,
    STECI_TARGET_SPI2MEM = 1
} DCOP_targetMemEnumTy;

typedef enum
{
    STECI_ACCESS_DATA_MODE = 0,
    STECI_ACCESS_FILE_MODE = 1
} DCOP_accessModeEnumTy;

/*!
 * \struct  tyDABDeviceConfiguration
 *          Describes the DAB DCOP device configuration
 */
typedef struct
{
    /*! Communication bus type (SPI or I2C) */
    tyCommunicationBusType      communicationBusType;
    /*! SPI or I2C bus configuration */
    union
    {
        /*! SPI bus configuration */
        tySpiCommunicationBus   spi;
        /*! I2C bus configuration */
        tyI2cCommunicationBus   i2c;
    } communicationBus;
    /*! GPIO where the DAB DCOP's RSTN line is connected */
    tU32                        GPIO_RESET;
    /* The GPIO connected to the SPI0's REQ line */
    tU32                        GPIO_REQ;
    /* The GPIO connected to the SPI0's BOOT line */
    tU32                        GPIO_BOOT;
    /* Indicate if the device is in boot mode */
    tBool                       isBootMode;
} tyDABDeviceConfiguration;

typedef struct
{
    tVoid *deviceHandle;
    tVoid *deviceSecondaryHandle;

    tU8 deviceMode;
    tU32 DABDeviceConfigID;
} STECI_deviceInfoTy;

/**************************************
 * Function prototypes
 *************************************/
tS32 DAB_Init(tyDABDeviceConfiguration *deviceConfiguration);
tS32 DAB_Driver_Init(tyDABDeviceConfiguration *deviceConfiguration);
tS32 DAB_setTcpIpAddress(tChar *IPAddress);
tS32 DAB_Deinit(tVoid);
tS32 DAB_Reset(tVoid);
tS32 DAB_StartupReset(tyDABDeviceConfiguration *deviceConfiguration);
tS32 DAB_Reconfiguration(tyDABDeviceConfiguration *deviceConfiguration);
tS32 DAB_Send_Message(tU8 LunId, tU8 *DataToSend,
        tU8 specific0, tU8 specific1, tU8 specific2, tU8 specific3, tU16 BytesNumber);
tS32 DAB_Get_Response(tU8 *buf, tU32 max_buf, tU32 *len, tU8 *lun);

#ifdef __cplusplus
}
#endif

#endif // DAB_PROTOCOL_H

//EOF
