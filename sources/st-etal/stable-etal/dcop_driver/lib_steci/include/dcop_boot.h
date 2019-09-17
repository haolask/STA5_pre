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
// FLASH SIZE
#define FLASH_SIZE							((tU32)(2 * 1024 * 1024))

// The FLASH WRITE command has an header of 12 bytes (11 at the begin, 1 at the end). In order
// to read 2048 bytes of payload we use a buffer containing such payload size plus the header size
#define FLASH_WRITE_HEADER_SIZE				((tU32)12)
#define FLASH_WRITE_PAYLOAD_SIZE			((tU32)2048)

// The FLASH DUMP command has an header of 12 bytes contiguous without any payload, meanwhile the 
// FLASH DUMP RESPONSE has an header of 3 bytes at the begin
#define FLASH_DUMP_CMD_HEADER_SIZE			((tU32)12)
#define FLASH_DUMP_CMD_PAYLOAD_SIZE			((tU32)0)

#define FLASH_DUMP_RSP_HEADER_SIZE			((tU32)3)
#define FLASH_DUMP_RSP_PAYLOAD_SIZE			((tU32)2048)

// The FLASH DUMP command has an header of 28 bytes
#define DCOP_BOOT_HEADER_SIZE               ((tU32)28)

// Device maximum allocated space
#define DCOP_DUMP_BUFFER_SIZE               (FLASH_DUMP_RSP_HEADER_SIZE + FLASH_DUMP_RSP_PAYLOAD_SIZE)
#define SPI_DEVICE_BUFFER_SIZE			    (FLASH_WRITE_HEADER_SIZE + FLASH_WRITE_PAYLOAD_SIZE)
#define SPI_BOOTSTRAP_BUFFER_SIZE		    ((tU32)2048)

// SPI Flasher Commands Header/Type
#define FLASHER_READY                       ((tU8)0x4A)
#define FLASHER_MEASURE	                    ((tU8)0x4B)
#define BULK_ERASE		                    ((tU8)0x28) 	
#define SECTOR_ERASE	                    ((tU8)0x29)
#define WRITE_FLASH 	                    ((tU8)0x38)
#define READ_FLASH	                        ((tU8)0x48)
#define SCRC_FLASH		                    ((tU8)0x58)

// SPI Flasher Answer Header
#define BULK_ERASE_OK	                    ((tU8)0x21)
#define SECTOR_ERASE_OK                     ((tU8)0x21)
#define WRITE_FLASH_OK                      ((tU8)0x31)
#define READ_FLASH_OK                       ((tU8)0x41)
#define SCRC_FLASH_OK                       ((tU8)0x51)

// SPI Loader Commands
#define BULK_FORMAT                         ((tU8)0x68)
#define SECTOR_FORMAT                       ((tU8)0x69)
#define WRITE_MEMORY                        ((tU8)0x78)
#define READ_MEMORY                         ((tU8)0x88)
#define LOADER_READY                        ((tU8)0x7A)
#define LOADER_GO                           ((tU8)0x7C)

// SPI Loader Answer
#define WRITE_MEMORY_OK                     ((tU8)0x71)
#define WRITE_MEMORY_KO                     ((tU8)0x70)
#define LOADER_READY_OK1                    ((tU8)0xCA)
#define LOADER_READY_OK2                    ((tU8)0xBA)
#define LOADER_GO_OK                        ((tU8)0x7F)
#define LOADER_GO_KO                        ((tU8)0x70)

// SPI Loader other parameters
#define CHKLOADER_CMD_LENGTH                4
#define CHKLOADER_RSP_LENGTH                6
#define WRMEM_RSP_LENGTH                    4
#define LOADER_GO_CMD_LENGTH                8
#define LOADER_GO_RSP_LENGTH                4

#define DCOP_BOOT_DEFAULT_OPTIONS           ((tU32)0x00040100)

// Default destination address is internal RAM base address
#define DCOP_BOOT_DEFAULT_DEST              ((tU32)0x10000000)

#define DCOP_FW_SECTION_NAME_LENGTH         128

///
// Types
///

typedef struct
{
	tChar   section[DCOP_FW_SECTION_NAME_LENGTH];
	tU32  load_address;
} DCOP_SectionDescriptionTy;

///
// Exported functions
///
extern tU16 DCOP_ExecuteFlashOperations (STECI_deviceInfoTy *deviceInfoPtr, DCOP_flashModeEnumTy mode, DCOP_targetMemEnumTy targetM,
                        DCOP_accessModeEnumTy accessmode, tU16 numBytes, tU8 *dataPtr);

#ifdef __cplusplus
}
#endif

// End of file
