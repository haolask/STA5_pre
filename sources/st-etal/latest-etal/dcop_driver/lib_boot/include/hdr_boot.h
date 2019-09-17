//!
//!  \file    hdr_boot.h
//!  \brief   <i><b> HDR boot header </b></i>
//!  \details Interface file for HDR boot functionalities.
//!  \author  Alberto Saviotti
//!

#ifndef HDR_BOOT_H
#define HDR_BOOT_H

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined CONFIG_APP_TUNERDRIVER_LIBRARY)
    ///
    // Defines
    ///
    // FLASH SIZE
#define HD_FLASH_SIZE					    ((tU32)(2 * 1024 * 1024))

    // The FLASH WRITE command has an header of 12 bytes (11 at the begin, 1 at the end). In order
    // to read 2048 bytes of payload we use a buffer containing such payload size plus the header size
#define HD_FLASH_WRITE_HEADER_SIZE			((tU32)12)
#define HD_FLASH_WRITE_PAYLOAD_SIZE			((tU32)2048)


    // The FLASH DUMP command has an header of 28 bytes
#define HD_BOOT_HEADER_SIZE               ((tU32)28)

    // Device maximum allocated space
#define HD_SPI_PROGRAM_BUFFER_SIZE			(HD_FLASH_WRITE_HEADER_SIZE + HD_FLASH_WRITE_PAYLOAD_SIZE)
#define HD_SPI_PHASE1_BUFFER_SIZE		    ((tU32)2048)

    // SPI Flasher Commands Header/Type
#define HD_FLASHER_READY                    ((tU8)0x4A)
#define HD_FLASHER_MEASURE	                ((tU8)0x4B)
#define HD_BULK_ERASE		                ((tU8)0x28) 	
#define HD_SECTOR_ERASE	                    ((tU8)0x29)
#define HD_WRITE_FLASH 	                    ((tU8)0x38)
#define HD_READ_FLASH	                    ((tU8)0x48)
#define HD_SCRC_FLASH		                ((tU8)0x58)

    // SPI Flasher Answer Header
#define HD_BULK_ERASE_OK	                ((tU8)0x21)
#define HD_SECTOR_ERASE_OK                  ((tU8)0x21)
#define HD_WRITE_FLASH_OK                   ((tU8)0x31)
#define HD_READ_FLASH_OK                    ((tU8)0x41)
#define HD_SCRC_FLASH_OK                    ((tU8)0x51)

#define HD_BOOT_DEFAULT_OPTIONS             ((tU32)0x00040100)

    // Default destination address is internal RAM base address
#define HD_BOOT_DEFAULT_DEST                ((tU32)0x10000000)

    // Maximum namber of packets
#define LM_PACKET_NUMBER					1

    // Following defines are related to HDRadio Logical Messaging Layer defined in ch. 5 of
    // "HD Radio Commercial Receiver Baseband Processor Command and Data Interface Definition"
    // Logical message header length
#define LM_FORMAT_FIELDS					12

    // Logical message maximum payload length
#define LM_PAYLOAD_MAX_LEN					2035

    // Logical message header length
#define LM_OVERHEAD						    13

    // The HDRadio message is encapsulated in the following mode:
    // - Command packets are 8 bytes header + data
    // - Logical messages encapsulating command pakects are 2035 payload + 13 header bytes.
    // Total is max 2048 bytes
#define LM_MAX_MESSAGE_LEN				    (LM_PAYLOAD_MAX_LEN + LM_OVERHEAD)

    // Logical message overhead depending on the physical channel
#if (defined CONFIG_COMM_HDRADIO_I2C)
#define LM_PHYDRIVER_OVERHEAD				4
#else
#define LM_PHYDRIVER_OVERHEAD				0
#endif // #if (defined CONFIG_COMM_HDRADIO_I2C)

    // Logical message full size
#define LM_FULL_SIZE						(LM_PAYLOAD_MAX_LEN + LM_OVERHEAD + LM_PHYDRIVER_OVERHEAD)

#define HDR_SPI_CS0         0
#define HDR_SPI_CS1         1

    // Special commands
#define HDR_NORMAL_CMD                                  ((tU8)0x00)
#define HDR_FLASH_CMD									((tU8)0x01)
#define HDR_RESET_CMD									((tU8)0x07)
#define HDR_BOOT_CMD									((tU8)0x08)

// Errors
#define RES_ERROR_GENERIC						((tS32)-1)
#define RES_OK									0
#define RES_ERROR								1

    ///
    // Enums
    ///
    // Flash specific sub-commands
    typedef enum
    {
        HDR_FLASH_OP_MODE_IS_NONE = 0,
        HDR_FLASH_OP_MODE_IS_PHASE1 = 1,
        HDR_FLASH_OP_MODE_IS_SYNC = 2,
        HDR_FLASH_OP_MODE_IS_PROGRAM = 3,
        HDR_FLASH_OP_MODE_IS_DUMP = 4,
        HDR_FLASH_OP_MODE_IS_CHKFLASH = 5,
        HDR_FLASH_OP_MODE_IS_PH2CMD = 6
    } HDR_flashModeEnumTy;

    typedef enum
    {
        HDR_TARGET_SPI2FLASH = 0,
        HDR_TARGET_SPI2MEM = 1
    } HDR_targetMemEnumTy;

    typedef enum
    {
        HDR_ACCESS_DATA_MODE = 0,
        HDR_ACCESS_FILE_MODE = 1
    } HDR_accessModeEnumTy;


    ///
    // Types
    ///
    typedef enum
    {
        HDR_I2CADDR = 0x2E
    } HDR_addressEnumTy;

    typedef enum
    {
        HDR_SPI_PHASE00 = 0x00,
        HDR_SPI_PHASE01 = 0x01,
        HDR_SPI_PHASE10 = 0x02,
        HDR_SPI_PHASE11 = 0x03
    } HDR_spiPhaseEnumTy;


    typedef struct
    {
        union
        {
            struct
            {
                tU8 spiPhase : 4;
                tU8 spiCs : 4;
            } bitfield;

            tU8 val;
        } h3;
    } HDR_spiBusTy;

    typedef enum
    {
        HDR_INSTANCE_UNDEF = 0x00,
        HDR_INSTANCE_1 = 0x01,
        HDR_INSTANCE_2 = 0x02,
        HDR_INSTANCE_3 = 0x03
    } HDR_instanceIdEnumTy;


    typedef enum
    {
        HDR_NORMAL_MODE = 0,
        HDR_RESET_MODE = 1,
        HDR_BOOT_MODE = 2,
        HDR_FLASH_MODE = 3
    } HDR_cmdModeEnumTy;

    typedef struct
    {
        tU8 opCode;                 // Command ID
        tU32 dataLength;            // 4 bytes LSB first
        tU8 *payloadDataPtr;
        HDR_cmdEnumTy cmdType;
        tU8 reserved;
        tU8 cmdStatus;
    } HDR_logicalMessageTy;

    typedef struct
    {
        tBool dataToBeTx;
        tU8 command;
        tU8 dataBuffer[(LM_MAX_MESSAGE_LEN * LM_PACKET_NUMBER)];
        tU8 rspBuffer[(LM_MAX_MESSAGE_LEN * LM_PACKET_NUMBER)];
        tU16 totalBytesNum;
        tU16 remainingBytesNum;

        HDR_logicalMessageTy hdrLm;
    } HDR_txRxDataTy;

    typedef struct
    {
        tVoid *devHandler;
        tBool useI2c;
        tU8 i2Caddr;
        tU8 HDR_countLM;
        //    tU8 communicationOptions;
        HDR_spiBusTy communicationOptions;
        HDR_addressEnumTy devAddress;
        HDR_instanceIdEnumTy instId;
        HDR_txRxDataTy txRxData;
        tU8 HDR_buf[LM_FULL_SIZE];
    } HDR_paramsTy;

typedef union
{
    HDR_targetMemEnumTy trg_mem;
    tyHDRADIOInstanceID instId;
} HDR_targetMemOrInstanceIDTy;


    ///
    // Exported functions
    ///
    extern tU16 HDR_ExecuteFlashOperations (tVoid *devicehandler, HDR_flashModeEnumTy mode, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr);
    extern void Start1Counter (void);
    extern double Get1Counter (void);

#ifdef __cplusplus
}
#endif
#endif
#endif // HDR_BOOT_H

// End of file hdr_boot.h
