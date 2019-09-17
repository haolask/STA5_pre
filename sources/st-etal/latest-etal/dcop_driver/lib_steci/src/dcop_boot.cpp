//!
//!  \file    dcop_boot.cpp
//!  \brief   <i><b> DCOP flash protocol</b></i>
//!  \details STECI protocol used by ETAL and by MDR_Protocol
//!  \author  Alberto Saviotti
//!

#include "target_config.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)

#include "osal.h"

#include "defines.h"
#include "types.h"
#include "utility.h"

#include "common_trace.h"
#include "steci_trace.h"

#include "DAB_Protocol.h"
#include "steci_lld.h"
#include "dcop_lld.h"
#include "dcop_boot.h"

#ifdef __cplusplus
extern "C" {
#endif

///
// DEFINES
///

///
// MACROs
///

///
// Typedefs
///

///
// Local variables
///
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
#if 0

//  Ethernet FCS 32 bit CRC algorithm ==================================
//  First, the polynomial itself and its table of feedback terms.  The
//  polynomial is
//  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
//
// ======================================================================
static const tU32 crc32_tab[] =
{
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
    0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
    0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
    0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
    0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
    0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
    0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
    0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
    0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
    0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
    0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
    0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
    0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
    0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
    0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
    0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
    0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
    0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
    0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
    0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
    0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
    0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
    0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
    0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
    0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
    0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
    0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
    0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
    0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
    0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
    0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
    0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
    0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
    0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
    0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
    0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
    0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
    0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
    0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
    0x2d02ef8dL
};

static tBool DCOP_fileLenFieldSet;
static tBool DCOP_entryPointFieldSet;
#endif
#endif // #ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
static tU8 DCOP_flashMaxSizeDataBfr[SPI_DEVICE_BUFFER_SIZE];

tU8 DCOP_fwSectionsNb;
DCOP_SectionDescriptionTy *DCOP_fwSectionsTable;

///
// Local functions declarations
///
static tSInt DCOP_Loader_GO (STECI_deviceInfoTy *deviceInfoPtr, tUInt address);
static tSInt DCOP_CheckSPILoader (STECI_deviceInfoTy *deviceInfoPtr);
static tSInt DCOP_CheckFlasher (STECI_deviceInfoTy *deviceInfoPtr);
static tSInt DCOP_FlashErase (STECI_deviceInfoTy *deviceInfoPtr);
static tSInt DCOP_ProgramCheckResult (STECI_deviceInfoTy *deviceInfoPtr, tU8 typeAction);
static tSInt DCOP_TxDataFlashProgram (STECI_deviceInfoTy *deviceInfoPtr, tU32 numBytes, tU8 *dataPtr, tU32 chunkSize, tBool checkResult);
static tSInt DCOP_FlashProgram (STECI_deviceInfoTy *deviceInfoPtr, tU32 numBytes, tU8 *dataPtr);
static tVoid DCOP_ReadSectionsFile(tU8* sectFileName);
static tSInt DCOP_WriteMemory (STECI_deviceInfoTy *deviceInfoPtr, tUInt address, tChar *dataPtr);
static tSInt DCOP_FlashDump (STECI_deviceInfoTy *deviceInfoPtr, tU16 numBytes, tU8 *dataPtr);
static tSInt DCOP_SendBootstrap (STECI_deviceInfoTy *deviceInfoPtr, tU32 numBytes, tU8 *dataPtr);
static tVoid DCOP_ComputeChecksum (tUChar *pData, tU32 len);
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
#if 0
static tVoid DCOP_BuildHeader (tU8 *headerBfrPtr, tU32 options, tU32 destAddress, tU32 fileLen, tU32 entryPoint);
static tBool DCOP_CalculateCrc32 (tU8 *headerBfrPtr, tU32 crc32val, tUChar *pData, tU32 len);
#endif
#endif // #ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE

///
// Global functions
///
tU16 DCOP_ExecuteFlashOperations (STECI_deviceInfoTy *deviceInfoPtr, DCOP_flashModeEnumTy mode, DCOP_targetMemEnumTy targetM,
                                    DCOP_accessModeEnumTy accessmode, tU16 numBytes, tU8 *dataPtr)
{
    tU16 resultAction = 1;
    tSInt res = FLASH_STATUS_ERROR_GENERIC;
    tU8 i;

    if (STECI_FLASH_OP_MODE_IS_FLASHER_CHK == mode)
    {
        if (STECI_TARGET_SPI2MEM == targetM)
        {
        	// Call the check for SPI loader
			res = DCOP_CheckSPILoader (deviceInfoPtr);
        }
        else if (STECI_TARGET_SPI2FLASH == targetM)
        {
			// Call the Check Flasher
			res = DCOP_CheckFlasher (deviceInfoPtr);
        }
    }
    else if (STECI_FLASH_OP_MODE_IS_BOOTSTRAP == mode)
    {
        if (STECI_TARGET_SPI2MEM == targetM)
        {
            // TODO: currently not supported
        }
        else if (STECI_TARGET_SPI2FLASH == targetM)
        {
            if (STECI_ACCESS_FILE_MODE == accessmode)
            {
                // Send bootstrap (passing only file name to use)
				res = DCOP_SendBootstrap (deviceInfoPtr, numBytes, dataPtr);
            }
            else
            {
                // Send bootstrap (passing Data already prepared by caller)
				res = DCOP_TxDataFlashProgram (deviceInfoPtr, numBytes, dataPtr, SPI_BOOTSTRAP_BUFFER_SIZE, false);
            }
        }
    }
    else if (STECI_FLASH_OP_MODE_IS_ERASE == mode)
	{
		// Call the erase function
		res = DCOP_FlashErase (deviceInfoPtr);
	}
	else if (STECI_FLASH_OP_MODE_IS_PROGRAM == mode)
	{
        if (STECI_TARGET_SPI2MEM == targetM)
        {
            DCOP_ReadSectionsFile(dataPtr);
            for (i=0;i<DCOP_fwSectionsNb;i++)
            {
                res = DCOP_WriteMemory(deviceInfoPtr, DCOP_fwSectionsTable[i].load_address, DCOP_fwSectionsTable[i].section);
                if (FLASH_STATUS_OK != res)
                {
                    //ERROR
                    // this will be freed when leaving the loop 
                    // DCOP_fwSectionsTable;
                    break;
                }
            }
            free(DCOP_fwSectionsTable);
            if (FLASH_STATUS_OK == res)
            {
                res = DCOP_Loader_GO(deviceInfoPtr, 0x00000000);
            }
        }
        else if (STECI_TARGET_SPI2FLASH == targetM)
        {
            if (STECI_ACCESS_FILE_MODE == accessmode)
            {
                // Call Program Flash function (passing only file name to use)
				res = DCOP_FlashProgram (deviceInfoPtr, numBytes, dataPtr);
            }
            else
            {
                // Call Program Flash function (passing Data already prepared by caller)
				res = DCOP_TxDataFlashProgram (deviceInfoPtr, numBytes, dataPtr, SPI_DEVICE_BUFFER_SIZE, true);
            }
        }
    }
	else if (STECI_FLASH_OP_MODE_IS_DUMP == mode)
	{
		// Call the dump flash function
		res = DCOP_FlashDump (deviceInfoPtr, numBytes, dataPtr);
	}

    if (res == FLASH_STATUS_OK)
    {
        resultAction = 0;
    }

    return resultAction;
}

static tSInt DCOP_Loader_GO (STECI_deviceInfoTy *deviceInfoPtr, tUInt address)
{
    tU8 cmdBuffer[LOADER_GO_CMD_LENGTH] = { 0x00, 0x08, LOADER_GO, 0x00, 0x00, 0x00, 0x00, 0x00 };
    tSInt res = FLASH_STATUS_ERROR_GENERIC;
    tU16 size;

    size = ((tU16)cmdBuffer[0] << (tU16)8) | (tU16)cmdBuffer[1];

	ConvertBaseUInt2Bin (address, &cmdBuffer[3], 4, true);

	DCOP_ComputeChecksum(&(cmdBuffer[0]), LOADER_GO_CMD_LENGTH);

	if ((res = DCOP_WritePort (deviceInfoPtr, size, &cmdBuffer[0], &DCOP_flashMaxSizeDataBfr[0])) == FLASH_STATUS_OK)
	{
		res = DCOP_ReadPort (deviceInfoPtr, LOADER_GO_RSP_LENGTH, &DCOP_flashMaxSizeDataBfr[0], DCOP_TIMEOUT_FLASH_READ, &DCOP_flashMaxSizeDataBfr[0]);

	    // Check if answer is good and timeout was not reached
	    if ((DCOP_flashMaxSizeDataBfr[2] != LOADER_GO_OK) || (FLASH_STATUS_OK != res))
    	{
	        res = FLASH_STATUS_ERROR_GENERIC;
	    }
	    else
	    {
	    	STECI_tracePrintComponent(TR_CLASS_STECI, "DCOP_Loader_GO OK");
	        res = FLASH_STATUS_OK;
	    }
	}
    return res;
}

static tSInt DCOP_CheckSPILoader (STECI_deviceInfoTy *deviceInfoPtr)
{
    tU8 cmdBuffer[CHKLOADER_CMD_LENGTH] = { 0x00, 0x04, LOADER_READY, 0x00 };
    tSInt res = FLASH_STATUS_ERROR_BOOTSTRAP;
    tU16 size;

    size = ((tU16)cmdBuffer[0] << (tU16)8) | (tU16)cmdBuffer[1];

	DCOP_ComputeChecksum(&(cmdBuffer[0]), CHKLOADER_CMD_LENGTH);

	if ((res = DCOP_WritePort (deviceInfoPtr, size, &cmdBuffer[0], &DCOP_flashMaxSizeDataBfr[0])) == FLASH_STATUS_OK)
	{
		res = DCOP_ReadPort (deviceInfoPtr, CHKLOADER_RSP_LENGTH, &DCOP_flashMaxSizeDataBfr[0], DCOP_TIMEOUT_FLASH_READ, &DCOP_flashMaxSizeDataBfr[0]);

	    // Check if answer is good and timeout was not reached
	    if (((DCOP_flashMaxSizeDataBfr[3] != LOADER_READY_OK1) && (DCOP_flashMaxSizeDataBfr[3] != LOADER_READY_OK2)) || (FLASH_STATUS_OK != res))
	    {	    
	        res = FLASH_STATUS_ERROR_BOOTSTRAP;
	    }
	    else
	    {
   			STECI_tracePrintComponent(TR_CLASS_STECI, "DCOP_CheckSPILoader OK: target boot mode = 0x%x", DCOP_flashMaxSizeDataBfr[2]);
	        res = FLASH_STATUS_OK;
	    }
	}
    return res;
}

static tSInt DCOP_CheckFlasher (STECI_deviceInfoTy *deviceInfoPtr)
{
    #define CHKFLASH_RSP_LENGTH				6

    tU8 cmdBuffer[4] = { 0x00, 0x04, 0x4A, 0x4E };
    tSInt res = FLASH_STATUS_ERROR_ERASE;
    tU16 size;

    // Erase total time can take up to 30/40 second (typical)
    size = ((tU16)cmdBuffer[0] << (tU16)8) | (tU16)cmdBuffer[1];

    // Write the command
    cmdBuffer[0] = (tU8)0x00;
    cmdBuffer[1] = (tU8)0x04;
    cmdBuffer[2] = (tU8)0x4A;
    cmdBuffer[3] = (tU8)0x4E;
	if ((res = DCOP_WritePort (deviceInfoPtr, size, &cmdBuffer[0], &DCOP_flashMaxSizeDataBfr[0])) == FLASH_STATUS_OK)
	{

	    // Read response
	    cmdBuffer[0] = (tU8)0x00;
	    cmdBuffer[1] = (tU8)0x00;
	    cmdBuffer[2] = (tU8)0x00;
	    cmdBuffer[3] = (tU8)0x00;

		res = DCOP_ReadPort (deviceInfoPtr, size + 2, &DCOP_flashMaxSizeDataBfr[0], DCOP_TIMEOUT_FLASH_READ, &DCOP_flashMaxSizeDataBfr[0]);

	    // Check if answer is good and timeout was not reached
	    if (((DCOP_flashMaxSizeDataBfr[2] != (tU8)0xF8) && (DCOP_flashMaxSizeDataBfr[3] != (tU8)0xCA) && (DCOP_flashMaxSizeDataBfr[5] != (tU8)0xC8)) || (FLASH_STATUS_OK != res))
	    {
	        res = FLASH_STATUS_ERROR_ERASE;
	    }
	    else
	    {
	        res = FLASH_STATUS_OK;
	    }
	}
    return res;
}

static tSInt DCOP_FlashErase (STECI_deviceInfoTy *deviceInfoPtr)
{
    #define FLASH_BULKERASE_RSP_LENGTH				4

    // Erase total time can take up to 30/40 second (typical)
	tU8 cmdBuffer[4] = {0x00, 0x04, 0x28, 0x2C};
	tSInt res = FLASH_STATUS_ERROR_ERASE;
	tU16 size;

	size = ((tU16)cmdBuffer[0] << (tU16)8) | (tU16)cmdBuffer[1];

	// Write the command
    if ((res = DCOP_WritePort (deviceInfoPtr, size, &cmdBuffer[0], &DCOP_flashMaxSizeDataBfr[0])) == FLASH_STATUS_OK)
    {
		// Set dummy bytes for next read operation
		cmdBuffer[0] = (tU8)0x00;
		cmdBuffer[1] = (tU8)0x00;
		cmdBuffer[2] = (tU8)0x00;
		cmdBuffer[3] = (tU8)0x00;
		
		// Check REQ line status and read response
		res = DCOP_ReadPort (deviceInfoPtr, size, &DCOP_flashMaxSizeDataBfr[0], DCOP_TIMEOUT_FLASH_BULKERASE, &DCOP_flashMaxSizeDataBfr[0]);

		// Check if answer is good and timeout was not reached
		if ((DCOP_flashMaxSizeDataBfr[2] != BULK_ERASE_OK) || (DCOP_flashMaxSizeDataBfr[3] != (tU8)0x25) || (FLASH_STATUS_OK != res))
		{
			res = FLASH_STATUS_ERROR_ERASE;
		}
		else
		{
			res = FLASH_STATUS_OK;
		}
	}
	return res;
}

static tSInt DCOP_ProgramCheckResult (STECI_deviceInfoTy *deviceInfoPtr, tU8 typeAction)
{
    #define FLASH_PROGRAM_FLASH_RSP_LENGTH				4

    tU8 cmdBuffer[4] = { 0x00, 0x00, 0x00, 0x00 };
	tSInt res = FLASH_STATUS_ERROR_PROGRAM;
	tU8 checkByte2, checkByte3;

    // Read response
	res = DCOP_ReadPort (deviceInfoPtr, FLASH_PROGRAM_FLASH_RSP_LENGTH, &DCOP_flashMaxSizeDataBfr[0], DCOP_TIMEOUT_FLASH_READ, &cmdBuffer[0]);

	if (FLASH_STATUS_OK == res)
	{
		// Check if answer is good and timeout was not reached
		if (typeAction == WRITE_FLASH)
		{
			checkByte2 = WRITE_FLASH_OK;
			checkByte3 = (tU8)0x35;
		}
		else if (typeAction == READ_FLASH)
		{
			checkByte2 = READ_FLASH_OK;
			checkByte3 = (tU8)0x45;
		}

		if ((cmdBuffer[0] == (tU8)0x00) && (cmdBuffer[1] == (tU8)0x04) && (cmdBuffer[2] == checkByte2) && (cmdBuffer[3] == checkByte3))
		{
			res = FLASH_STATUS_OK; // Operation has been concluded well
		}
	}

    return res;
}

static tSInt DCOP_TxDataFlashProgram (STECI_deviceInfoTy *deviceInfoPtr, tU32 numBytes, tU8 *dataPtr, tU32 chunkSize, tBool checkResult)
{
	tSInt res = FLASH_STATUS_ERROR_PROGRAM;
	tU32 bytesToSend = numBytes;

	while (numBytes > 0)
	{
		if (numBytes > chunkSize)
		{
			bytesToSend = chunkSize;
		}
		else
		{
			bytesToSend = numBytes;
		}

		if (bytesToSend > 0)
		{
			res = DCOP_WritePort (deviceInfoPtr, bytesToSend, dataPtr, &DCOP_flashMaxSizeDataBfr[0]);

			if (FLASH_STATUS_OK == res)
			{
				numBytes -= bytesToSend;
				dataPtr += bytesToSend;

				if (true == checkResult)
				{
					res = DCOP_ProgramCheckResult (deviceInfoPtr, WRITE_FLASH);

					if (FLASH_STATUS_OK != res)
					{
						// Error
						break;
					}
				}
			}
			else
			{
				// Error            
				break;
			}
		}
	}

	return res;
}

static tSInt DCOP_SendBootstrap (STECI_deviceInfoTy *deviceInfoPtr, tU32 numBytes, tU8 *dataPtr)
{
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
    tUChar *pagein;
	tU32 payloadSize;
    tULong fileSize;
	tSLong fileSize_sign;
    FILE *pfin;
    tSInt res = FLASH_STATUS_OK;
	tU8 headerBuffer[DCOP_BOOT_HEADER_SIZE];

    // Open received file for binary read: the flasher file must already have the header
    // allocated at least as zeroes (it will be calculated again here)
    if ((pfin = OSAL_fopen ((const char*)dataPtr, "rb")) == NULL)
    {
        // Error : File is not accessible nor existing
		return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
    }
    else
    {
        // Get the size of file in bytes
        if (OSAL_fseek (pfin, 0, SEEK_END) != 0)
        {
            fclose (pfin);
            return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }

        fileSize_sign = OSAL_ftell (pfin);
		if (fileSize_sign >= 0)
		{
			fileSize = (tULong)fileSize_sign;
		}
		else
		{
			ASSERT_ON_DEBUGGING(0);
			fileSize = 0;
		}
        if (OSAL_fseek (pfin, 0, SEEK_SET) != 0)
        {
            fclose (pfin);
            return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }

        // Allocation of a dinamic array   size = (flashSize+12+2)
        pagein = (tUChar *)OSAL_pvMemoryAllocate ((tU32)fileSize);

		if (NULL != pagein)
		{
	        // Fill the pagein buffer starting from the first data payload bytes (byte 11) with the file bytes
	        if (OSAL_fread (pagein, 1, fileSize, pfin) != fileSize)
	        {
	            res = FLASH_STATUS_ERROR_READ;
	        }
		}
		else
		{
			res = FLASH_STATUS_ERROR_MEMORY;
		}
    }

    // Close file
    OSAL_fclose (pfin);

    if (res == FLASH_STATUS_OK)
    {
	        // Calculate payload size (no header)
	        payloadSize = (tU32)(fileSize - DCOP_BOOT_HEADER_SIZE);

			// Header is present in the file generated at compile time
			// use it
#if 0
		    // Calculate CRC and get header
		   	DCOP_BuildHeader (&headerBuffer[0], DCOP_BOOT_DEFAULT_OPTIONS, DCOP_BOOT_DEFAULT_DEST, payloadSize, 0);

		   	// Calculate CRC
		   	if (!DCOP_CalculateCrc32 (&headerBuffer[0], 0, (pagein + DCOP_BOOT_HEADER_SIZE), payloadSize))
		   	{
		   		// TODO what to do in case of header error?
		   		ASSERT_ON_DEBUGGING(0);
		   	}   	
#endif
			// take the header present in the file already
			OSAL_pvMemoryCopy((tVoid *)&headerBuffer[0], (tVoid *)pagein, DCOP_BOOT_HEADER_SIZE);

	    	// Copy header
	    	OSAL_pvMemoryCopy ((tVoid *)pagein, (tPCVoid)&headerBuffer[0], DCOP_BOOT_HEADER_SIZE);

	        // Copy data got from file
	        OSAL_pvMemoryCopy ((tVoid *)(pagein + DCOP_BOOT_HEADER_SIZE), (tPCVoid)(pagein + DCOP_BOOT_HEADER_SIZE), payloadSize);

        // Send data to SPI
        res = DCOP_TxDataFlashProgram (deviceInfoPtr, (tU32)fileSize, pagein, SPI_BOOTSTRAP_BUFFER_SIZE, false);
    }

    // Free allocated resources
    if (NULL != pagein)
    {
        free (pagein);
    }
#else
    tSInt res = FLASH_STATUS_ERROR_SOURCENOTAVAIL;
#endif // #ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE

    return res;
}

static tSInt DCOP_FlashProgram (STECI_deviceInfoTy *deviceInfoPtr, tU32 numBytes, tU8 *dataPtr)
{
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
    tUChar *prg_frame;
    tU32 cnt, indexMAX;
    tU32 address = 0;
    tU32 fractSIZE;   
    FILE *pfin;
	tULong flashSize;
	tSLong flashSize_sign;
    tSInt res = FLASH_STATUS_OK;

    // Take input file name and open for binary read
    if ((pfin = OSAL_fopen ((const char*)dataPtr, "rb")) == NULL)
    {
        // File is not accessible nor existing 
		return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
    }
    else
    {
        // Get the size of file in bytes
        if (OSAL_fseek (pfin, 0, SEEK_END) != 0)
        {
            OSAL_fclose (pfin);
		    return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }
        flashSize_sign = OSAL_ftell (pfin);
		if (flashSize_sign >= 0)
		{
			flashSize = (tULong)flashSize_sign;
		}
		else
		{
			ASSERT_ON_DEBUGGING(0);
			flashSize = 0;
		}
        if (OSAL_fseek (pfin, 0, SEEK_SET) != 0)
        {
            OSAL_fclose (pfin);
		    return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }
    }

    // Max buffer size is SPI_DEVICE_BUFFER_SIZE and we need to take in account 12 header bytes
	indexMAX = (tU32)(flashSize / FLASH_WRITE_PAYLOAD_SIZE);
	fractSIZE = (tU32)(flashSize % FLASH_WRITE_PAYLOAD_SIZE);

    // Allocate dynamic structure containing command program frame
    prg_frame = (tUChar *)OSAL_pvMemoryAllocate (SPI_DEVICE_BUFFER_SIZE);
	if (prg_frame == NULL)
	{
		return FLASH_STATUS_ERROR_MEMORY;
	}

    // Write Command Header: command length + command ID
	ConvertBaseUInt2Bin (SPI_DEVICE_BUFFER_SIZE, &prg_frame[0], 2, true);
    prg_frame[2] = WRITE_FLASH;

    // Packets to transmit: each packet is FLASH_SLOT_SIZE wide
    for (cnt = 0; cnt < indexMAX; cnt++)
    {
        // Calculate actual start address
		address = cnt * FLASH_WRITE_PAYLOAD_SIZE;

        ConvertBaseUInt2Bin (address, &prg_frame[3], 4, true);
 
        //Calculate Write offset/lenght fields
		ConvertBaseUInt2Bin (FLASH_WRITE_PAYLOAD_SIZE, &prg_frame[7], 4, true);

        // Write Data Payload and Checksum, embedd the data payload
		if (OSAL_fread ((tVoid *)&prg_frame[11], 1, FLASH_WRITE_PAYLOAD_SIZE, pfin) != FLASH_WRITE_PAYLOAD_SIZE)
		{
            res = FLASH_STATUS_ERROR_READ;
            break;
		}

        // Refresh the Checksum for frame command
		DCOP_ComputeChecksum (&prg_frame[0], SPI_DEVICE_BUFFER_SIZE);

		// Program flash
		res = DCOP_TxDataFlashProgram (deviceInfoPtr, SPI_DEVICE_BUFFER_SIZE, prg_frame, SPI_DEVICE_BUFFER_SIZE, true);

		if (FLASH_STATUS_OK != res)
		{
			// On error we do not go further
			break;
		}
    }

    // Residual bytes to transmit (if no error has beenfound)
	if (fractSIZE != 0 && FLASH_STATUS_OK == res)
    {
        // Command Length
		ConvertBaseUInt2Bin ((fractSIZE + FLASH_WRITE_HEADER_SIZE), &prg_frame[0], 2, true);

        // Re-calculate actual start address
		address += FLASH_WRITE_PAYLOAD_SIZE;
        ConvertBaseUInt2Bin (address, &prg_frame[3], 4, true);

        // Write offset/lenght fields
        ConvertBaseUInt2Bin (fractSIZE, &prg_frame[7], 4, true);

        // Write Data Payload and Checksum, embedd the data payload
		if (OSAL_fread ((tVoid *)&prg_frame[11], 1, fractSIZE, pfin) == fractSIZE)
		{
            // Refresh the Checksum for frame command
    		DCOP_ComputeChecksum (&prg_frame[0], (fractSIZE + FLASH_WRITE_HEADER_SIZE));

    		// Program flash
    		res = DCOP_TxDataFlashProgram (deviceInfoPtr, (fractSIZE + FLASH_WRITE_HEADER_SIZE), prg_frame, fractSIZE, true);
    	}
        else
		{
            res = FLASH_STATUS_ERROR_READ;
		}
    }

    // Close file
    OSAL_fclose (pfin);
    OSAL_vMemoryFree((tVoid *)prg_frame);
#else
    tSInt res = FLASH_STATUS_ERROR_SOURCENOTAVAIL;
#endif // #ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE

    return res;
}

static tVoid DCOP_ReadSectionsFile(tU8* sectFileName)
{
	FILE * pFile;
	char line[DCOP_FW_SECTION_NAME_LENGTH];
	tU8 i;

	pFile = fopen ((const char *)sectFileName, "r");
	if (pFile != NULL)
	{
	    if ( fgets (line , sizeof(line) , pFile) != NULL )
	    {
		    DCOP_fwSectionsNb = atoi(line);
        }
        DCOP_fwSectionsTable = (DCOP_SectionDescriptionTy *) malloc(sizeof(DCOP_SectionDescriptionTy)*DCOP_fwSectionsNb);
        for (i=0;i<DCOP_fwSectionsNb;i++)
        {
            if ( fgets (line , sizeof(line) , pFile) != NULL )
            {
                memset(DCOP_fwSectionsTable[i].section, '\0', DCOP_FW_SECTION_NAME_LENGTH);
                strncpy(DCOP_fwSectionsTable[i].section, line, strlen(line)-1);
                if ( fgets (line , sizeof(line) , pFile) != NULL )
                {
                    DCOP_fwSectionsTable[i].load_address = strtoul(line, NULL, 0);
                }
                else
                {
            	    break;
                }
            }
            else
            {
                break;
            }
       	}
        fclose (pFile);
	}
}

static tSInt DCOP_WriteMemory (STECI_deviceInfoTy *deviceInfoPtr, tUInt WR_address, tChar *dataPtr)
{
    tUChar *prg_frame;
    tU32 cnt, indexMAX;
    tU32 address = 0;
    tU32 fractSIZE;
    FILE *pfin;
	tULong flashSize;
	tSLong flashSize_sign;

    tSInt res = FLASH_STATUS_ERROR_WRITE;
	tUInt baseAddress = 0x00000000;

	baseAddress = WR_address;

    // Take input file name and open for binary read
    if ((pfin = OSAL_fopen ((const char*)dataPtr, "rb")) == NULL)
    {
        // File is not accessible nor existing
		return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
    }
    else
    {
        // Get the size of file in bytes
        if (OSAL_fseek (pfin, 0, SEEK_END) != 0)
        {
            OSAL_fclose (pfin);
		    return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }
        flashSize_sign = OSAL_ftell (pfin);
		if (flashSize_sign >= 0)
		{
			flashSize = (tULong)flashSize_sign;
		}
		else
		{
			ASSERT_ON_DEBUGGING(0);
			flashSize = 0;
		}
        if (OSAL_fseek (pfin, 0, SEEK_SET) != 0)
        {
            OSAL_fclose (pfin);
		    return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }
    }

    // Max buffer size is SPI_DEVICE_BUFFER_SIZE and we need to take in account 12 header bytes
	indexMAX = (tU32)(flashSize / FLASH_WRITE_PAYLOAD_SIZE);
	fractSIZE = (tU32)(flashSize % FLASH_WRITE_PAYLOAD_SIZE);

    // Allocate dynamic structure containing command program frame
    prg_frame = (tUChar *)OSAL_pvMemoryAllocate (SPI_DEVICE_BUFFER_SIZE);
	if (prg_frame == NULL)
	{
		return FLASH_STATUS_ERROR_MEMORY;
	}

    // Write Command Header: command length + command ID
	ConvertBaseUInt2Bin (SPI_DEVICE_BUFFER_SIZE, &prg_frame[0], 2, true);
    prg_frame[2] = WRITE_MEMORY;

    // Packets to transmit: each packet is FLASH_SLOT_SIZE wide
    for (cnt = 0; cnt < indexMAX; cnt++)
    {
        // Calculate actual start address
		address = baseAddress + (cnt * FLASH_WRITE_PAYLOAD_SIZE);

        ConvertBaseUInt2Bin (address, &prg_frame[3], 4, true);

        //Calculate Write offset/length fields
		ConvertBaseUInt2Bin (FLASH_WRITE_PAYLOAD_SIZE, &prg_frame[7], 4, true);

        // Write Data Payload and Checksum, embedd the data payload
		if (OSAL_fread ((tVoid *)&prg_frame[11], 1, FLASH_WRITE_PAYLOAD_SIZE, pfin) != FLASH_WRITE_PAYLOAD_SIZE)
		{
            res = FLASH_STATUS_ERROR_READ;
            break;
		}

        // Refresh the Checksum for frame command
		DCOP_ComputeChecksum (&prg_frame[0], SPI_DEVICE_BUFFER_SIZE);

		if ((res = DCOP_WritePort(deviceInfoPtr, SPI_DEVICE_BUFFER_SIZE, prg_frame, &DCOP_flashMaxSizeDataBfr[0])) == FLASH_STATUS_OK)
		{
			res = DCOP_ReadPort(deviceInfoPtr, WRMEM_RSP_LENGTH, &DCOP_flashMaxSizeDataBfr[0], DCOP_TIMEOUT_FLASH_READ, &DCOP_flashMaxSizeDataBfr[0]);

			// Check if answer is good and timeout was not reached
			if ((DCOP_flashMaxSizeDataBfr[2] != WRITE_MEMORY_OK) || (FLASH_STATUS_OK != res))
			{
				res = FLASH_STATUS_ERROR_WRITE;
			}
			else
			{
				STECI_tracePrintComponent(TR_CLASS_STECI, "DCOP_WriteMemory OK");
				res = FLASH_STATUS_OK;
			}
		}

		if (FLASH_STATUS_OK != res)
		{
			// On error we do not go further
			break;
		}
    }

    // Residual bytes to transmit (if no error has been found or if file is smaller than a packet )
	if ((fractSIZE != 0 && FLASH_STATUS_OK == res) || (fractSIZE != 0 && cnt == 0))
    {
        // Command Length
		ConvertBaseUInt2Bin ((fractSIZE + FLASH_WRITE_HEADER_SIZE), &prg_frame[0], 2, true);

        // Re-calculate actual start address
		address = baseAddress + (cnt * FLASH_WRITE_PAYLOAD_SIZE);
        ConvertBaseUInt2Bin (address, &prg_frame[3], 4, true);

        // Write offset/length fields
        ConvertBaseUInt2Bin (fractSIZE, &prg_frame[7], 4, true);

        // Write Data Payload and Checksum, embed the data payload
		if (OSAL_fread ((tVoid *)&prg_frame[11], 1, fractSIZE, pfin) == fractSIZE)
		{
            // Refresh the Checksum for frame command
            DCOP_ComputeChecksum (&prg_frame[0], (fractSIZE + FLASH_WRITE_HEADER_SIZE));

			if ((res = DCOP_WritePort(deviceInfoPtr, (fractSIZE + FLASH_WRITE_HEADER_SIZE), prg_frame, &DCOP_flashMaxSizeDataBfr[0])) == FLASH_STATUS_OK)
			{
				res = DCOP_ReadPort(deviceInfoPtr, WRMEM_RSP_LENGTH, &DCOP_flashMaxSizeDataBfr[0], DCOP_TIMEOUT_FLASH_READ, &DCOP_flashMaxSizeDataBfr[0]);

				// Check if answer is good and timeout was not reached
				if ((DCOP_flashMaxSizeDataBfr[2] != WRITE_MEMORY_OK) || (FLASH_STATUS_OK != res))
				{
					res = FLASH_STATUS_ERROR_WRITE;
				}
				else
				{
					STECI_tracePrintSystem(TR_CLASS_STECI, "Last DCOP_WriteMemory OK");
					res = FLASH_STATUS_OK;
				}
			}
        }
        else
        {
            res = FLASH_STATUS_ERROR_READ;
        }
    }

    // Close file
    OSAL_fclose (pfin);
    OSAL_vMemoryFree((tVoid *)prg_frame);

    return res;
}


static tVoid DCOP_ComputeChecksum (tUChar *pData, tU32 len)
{
    tU32 temp = 0;
    tU32 ckm = 0;

    for (ckm = 0; ckm < (len - 1); ckm++)
    {
        temp += (tU32)*(pData + ckm);
    }

    *(pData + ckm) = temp;
}

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
#if 0
// Build default header: it doe not have CRC and file length properly set
static void DCOP_BuildHeader (tU8 *headerBfrPtr, tU32 options, tU32 destAddress, tU32 fileLen, tU32 entryPoint)
{
	tSInt cnt = 0;

    DCOP_fileLenFieldSet = false;
    DCOP_entryPointFieldSet = false;
    
    // Identifier 8 Bytes
	*(headerBfrPtr + cnt) = (tU8)0xF4; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0x01; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0xD5; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0xBC; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0x73; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0x40; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0x98; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0x83; cnt++;

    // Options (4 Bytes)
	*(headerBfrPtr + cnt) = (tU8)(options >> 0) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(options >> 8) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(options >> 16) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(options >> 24) & 0xFF; cnt++;

    // Destination address
	*(headerBfrPtr + cnt) = (tU8)(destAddress >> 0) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(destAddress >> 8) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(destAddress >> 16) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(destAddress >> 24) & 0xFF; cnt++;

    // CRC
	*(headerBfrPtr + cnt) = (tU8)0x00; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0x00; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0x00; cnt++;
	*(headerBfrPtr + cnt) = (tU8)0x00; cnt++;

    // Code size
	*(headerBfrPtr + cnt) = (tU8)(fileLen >> 0) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(fileLen >> 8) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(fileLen >> 16) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(fileLen >> 24) & 0xFF; cnt++;

    // Entry point
	*(headerBfrPtr + cnt) = (tU8)(entryPoint >> 0) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(entryPoint >> 8) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(entryPoint >> 16) & 0xFF; cnt++;
	*(headerBfrPtr + cnt) = (tU8)(entryPoint >> 24) & 0xFF;

    // We keep track that some field shall be set prior to calculate CRC
    DCOP_fileLenFieldSet = true;
    DCOP_entryPointFieldSet = true;
}

// Return a 32-bit CRC of the contents of the buffer accumulating the
// result from a previous CRC calculation. This uses the Ethernet FCS algorithm
static tBool DCOP_CalculateCrc32 (tU8 *headerBfrPtr, tU32 crc32val, tUChar *pData, tU32 len)
{
    tU32 cnt;

    // We check if the user has properly set all fields prior to CRC calculation
    if (false == DCOP_fileLenFieldSet || false == DCOP_entryPointFieldSet)
    {
        return false;
    }

    // Check if a valid pointer has been passed
    if (NULL == pData)
    {
        return false;
    }

    // Initialize CRC value
	crc32val = crc32val ^ (tU32)0xFFFFFFFF;

    // Calculate CRC on header part
    for (cnt = 0; cnt < 8; cnt++)
    {
		crc32val = crc32_tab[(crc32val ^ *(headerBfrPtr + 20 + cnt)) & 0xFF] ^ (crc32val >> 8);
    }

    // Calculate CRC on payload part
    for (cnt = 0; cnt < len; cnt++)
    {
        crc32val = crc32_tab[(crc32val ^ pData[cnt]) & 0xFF] ^ (crc32val >> 8);
    }

    // Set crc
    crc32val = crc32val ^ (tU32)0xFFFFFFFF;

    // Insert the CRC inside header
	*(headerBfrPtr + 16) = (tU8)(crc32val >> 0) & 0xFF;
	*(headerBfrPtr + 17) = (tU8)(crc32val >> 8) & 0xFF;
	*(headerBfrPtr + 18) = (tU8)(crc32val >> 16) & 0xFF;
	*(headerBfrPtr + 19) = (tU8)(crc32val >> 24) & 0xFF;

    return true;
}
#endif

#endif // #ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
static tSInt DCOP_FlashDump (STECI_deviceInfoTy *deviceInfoPtr, tU16 numBytes, tU8 *dataPtr)
{
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
	#define DUMP_BUFFER_NUMBER                  (FLASH_SIZE / FLASH_DUMP_RSP_PAYLOAD_SIZE)

	// Last byte is sum of previous bytes and it is calculated below by DCOP_ComputeChecksum()
	tU8 readDump_cmd[FLASH_DUMP_CMD_HEADER_SIZE] = {0x00, 0x0C, READ_FLASH, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00};
	tSInt res = FLASH_STATUS_ERROR_DUMP;
    FILE *pfout;
    tU32 cnt;
    tU32 cmd_length = 0;
	tU8 *rxBufferPtr;
    tU32 base_addr;

	// Try opening output file
	if (NULL == (pfout = OSAL_fopen ((const char*)dataPtr, "wb+")))
    {
		return FLASH_STATUS_ERROR_SOURCENOTAVAIL;
    }

	// Allocate dynamic structure containing command program frame
	rxBufferPtr = (tU8 *)OSAL_pvMemoryAllocate (DCOP_DUMP_BUFFER_SIZE);

	if (NULL == rxBufferPtr)
	{
		fclose (pfout);
		return FLASH_STATUS_ERROR_DUMP;
	}

    // Get file destination address
    base_addr = ConvertBaseBin2UInt (&readDump_cmd[3], 4, true);
	ConvertBaseUInt2Bin (FLASH_DUMP_RSP_PAYLOAD_SIZE, &readDump_cmd[7], 4, true);

    // Dump loop
    for (cnt = 0; cnt < DUMP_BUFFER_NUMBER; cnt++)
    {
		cmd_length = (((tU32)readDump_cmd[0] << 8)) | (((tU32)readDump_cmd[1]) << 0);

        // Refresh the Checksum for frame command
        DCOP_ComputeChecksum (&readDump_cmd[0], cmd_length);

        if ((res = DCOP_WritePort (deviceInfoPtr, cmd_length, &readDump_cmd[0], &DCOP_flashMaxSizeDataBfr[0])) != FLASH_STATUS_OK)
        {
            break;
        }

		res = DCOP_ReadPort (deviceInfoPtr, DCOP_DUMP_BUFFER_SIZE, rxBufferPtr, DCOP_TIMEOUT_FLASH_READ, &DCOP_flashMaxSizeDataBfr[0]);

        if (FLASH_STATUS_OK == res)
        {
			// Write to file
			if (OSAL_fwrite (&DCOP_flashMaxSizeDataBfr[3], 1, FLASH_DUMP_RSP_PAYLOAD_SIZE, pfout) != FLASH_DUMP_RSP_PAYLOAD_SIZE)
			{
                res = FLASH_STATUS_ERROR_WRITE;
                break;
			}
        }
        else
        {
            break;
        }

		base_addr += FLASH_DUMP_RSP_PAYLOAD_SIZE;  // Steps to the next address

        // Update start address
        ConvertBaseUInt2Bin (base_addr, &readDump_cmd[3], 4, true);
    }

	// Close resources
	OSAL_fclose (pfout);

	if (NULL != rxBufferPtr)
	{
		free (rxBufferPtr);
	}
#else
    tSInt res = FLASH_STATUS_ERROR_SOURCENOTAVAIL;
#endif // #ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE

    return res;
}

#ifdef __cplusplus
}
#endif

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_ETAL_SUPPORT_DCOP_MDR

// End of file
