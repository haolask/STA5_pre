//!
//!  \file 		boot_cmost.c
//!  \brief 	<i><b> CMOST firmware download </b></i>
//!  \details   Firmware download for the CMOST device
//!  \author 	Raffaele Belardi
//!
#include "target_config.h"

#if defined(CONFIG_ETAL_SUPPORT_CMOST)

	#include "osal.h"

#include "tunerdriver.h"
#include "tunerdriver_internal.h"
#include "cmost_protocol.h"
#include "cmost_helpers.h"
#include "cmost_dump.h"
#include "boot_cmost.h"
#include "common_trace.h"
#include "boot_trace.h"

/*
 * Compatibility check for the CMOST parameter file
 * These may be defined in cmost_defs.h
 */
#if defined (COEFF_FORMAT_24BIT_LITTLE_ENDIAN) || defined (COEFF_FORMAT_24BIT_BIG_ENDIAN)
	#error "Only 32 bit format supported for CMOST parameter file"
#endif

/*
 * The firmware files
 */
#ifdef CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED
	#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
		#include BOOT_FIRMWARE_INCLUDE_STAR_T
	#endif
	#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
		#include BOOT_FIRMWARE_INCLUDE_STAR_S
	#endif
	#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
		#include BOOT_FIRMWARE_INCLUDE_DOT_T
	#endif
	#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
		#include BOOT_FIRMWARE_INCLUDE_DOT_S
	#endif
#endif

/*
 * The (optional) parameter files
 */
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	#if defined (BOOT_PARAM_INCLUDE_STAR_T)
		#include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_T
		#include BOOT_PARAM_INCLUDE_STAR_T
	#endif
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	#if defined (BOOT_PARAM_INCLUDE_STAR_S)
		#include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_S
		#include BOOT_PARAM_INCLUDE_STAR_S
	#endif
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	#if defined (BOOT_PARAM_INCLUDE_DOT_T)
		#include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_T
		#include BOOT_PARAM_INCLUDE_DOT_T
	#endif
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	#if defined (BOOT_PARAM_INCLUDE_DOT_S)
		#include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_S
		#include BOOT_PARAM_INCLUDE_DOT_S
	#endif
#endif


/**************************************
 * Macros
 *************************************/

/**************************************
 * Other macros
 *************************************/
/*
 * Macros in this section normally should not be changed directly
 * except when porting the driver to a new platform
 */

/*!
 * \def		FW_CHUNK_DATA_SIZE_BYTES_CMOST
 * 			Size in bytes of the temporary buffer used to format the chunk of
 * 			Firmware data.
 *
 * \remark	Must be multiple of 4
 */
#define FW_CHUNK_DATA_SIZE_BYTES_CMOST   (63 * 4)

/* Reserve some space for the address, it is needed by the CMOST protocol_layer */
#define FW_CHUNK_HEADER_RESERVED_CMOST   3
#define FW_CHUNK_SIZE_BYTES_CMOST       (FW_CHUNK_HEADER_RESERVED_CMOST + FW_CHUNK_DATA_SIZE_BYTES_CMOST)
/* the first two bytes reserved for the size */
#define FW_CHUNK_ARRAY_ROW_SIZE_CMOST   (FW_CHUNK_SIZE_BYTES_CMOST + 2)

#define FW_END_MARKER                   0xFFFFFF

/*
 * WORKAROUND_FOR_CMOST_READ_BUG
 *
 * There's a bug in the CMOST up to at least CUT2.1:
 * when reading from memory the first value returned (4 bytes) is invalid
 * Once the bug will be resolved change this define to 0
 */
#define WORKAROUND_FOR_CMOST_READ_BUG  4

/*
 * BOOT_MANAGE_DSP_RESET
 *
 * Early CMOST FWs did not release the DSPs' reset at the end
 * of the download procedure as a workaround for some issue that
 * required connecting the JTAG emulator.
 * Current builds do it so it is no longer necessary to do it from here.
 *
 * It's possible to verify if the FW manages the DSP reset by looking
 * at the last lines in the .boot.h file, just before the TERMINATION string:
 *
 * // 0x14012 - 0x14013: 8 bytes, 2 words
 * 0x01,0x40,0x12,
 * 0x00,0x08,
 * 0xFF,0xFF,0xFF,0xFF, 0x00,0x00,0x00,0xFF,  
 *
 * If the values written to registers 0x14012 - 0x14013
 * contain only 0xFFs and no 0xFEs then the code is removind
 * the DSPs from reset
 */
#undef BOOT_MANAGE_DSP_RESET

/*
 * BOOT_TUNER_TY_STR_LENGTH
 *
 * Define the number of character for tuner type string
 */
#define BOOT_TUNER_TY_STR_LENGTH (8)

const tChar BootTunerTy_STR[BOOT_TUNER_TY_NB][BOOT_TUNER_TY_STR_LENGTH]=
{
	"UNKNOWN"													/* BOOT_TUNER_UNKNOWN */
	,"STAR-T"													/* BOOT_TUNER_STAR_T */
	,"STAR-S"													/* BOOT_TUNER_STAR_S */
	,"DOT-T"													/* BOOT_TUNER_DOT_T */
	,"DOT-S"													/* BOOT_TUNER_DOT_S */
};

/**************************************
 * Local types
 *************************************/
typedef enum 
{
	Init,
	ReadData,
	ReadAddress,
#ifdef CONFIG_COMM_CMOST_FIRMWARE_FILE
	FirstChar,
	BufferComplete,
	LineComplete,
	FileComplete,
	Complete,
	Error
#endif
} FwReadFSMStateTy;

typedef enum
{
	LoadFirmware,
	LoadParameters
} BootModeTy;

typedef struct
{
	FwReadFSMStateTy state;
#ifdef CONFIG_COMM_CMOST_FIRMWARE_FILE
	tS32 firstAddress;
	tS32 currentAddress;
	tS32 nextLineAddress;
	tS32 offset;
#endif
#if defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED) || defined (CONFIG_COMM_CMOST_FIRMWARE_IMAGE)
	tU32 currentPosition;
	tU32 remainingRowBytes;
	tU32 targetAddress;
	tU8 *firmware;
	tU32 firmwareSize;
#endif
} BootCMOSTParamsTy;

/**************************************
 * Local variables
 *************************************/
/* Ensure 32-bit word alignment for FwChunk_CMOST (not sure if needed) */
static tU8 FwChunk_CMOST[FW_CHUNK_SIZE_BYTES_CMOST] __attribute__ ((aligned (4)));

#ifdef BOOT_READ_BACK_AND_COMPARE_CMOST
tU8 FwTestReadBuf_CMOST[FW_CHUNK_SIZE_BYTES_CMOST + WORKAROUND_FOR_CMOST_READ_BUG];
#endif

#ifdef CONFIG_COMM_CMOST_FIRMWARE_FILE
FILE *FwFilep_CMOST;
#endif

/**************************************
 * Function prototypes
 *************************************/

/**************************************
 * Function definitions
 *************************************/
static tVoid BOOT_resetBootParam(BootCMOSTParamsTy *param, tU8 *image, tU32 image_size);
#ifndef CONFIG_APP_TEST_INITIALIZATION
static tSInt BOOT_getFirmwareReferences_CMOST(BootTunerTy tuner_type, tChar **filename, tU8 **array, tU32 *size, tU8 **param_array, tU32 *param_entries);
#endif
static tSInt BOOT_DownloadSendChunk_CMOST(tU32 addr, tS32 size, tU32 deviceID);
#if defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED) || defined (CONFIG_COMM_CMOST_FIRMWARE_IMAGE)
static tS32 BOOT_ReadLineFromArray_CMOST(BootCMOSTParamsTy *param, BootTunerTy tuner_type, tU32 *addr, tS32 *size);
#endif
static tS32 BOOT_DownloadReadLine_CMOST(BootCMOSTParamsTy *param, BootTunerTy tuner_type, tU32 *addr, tS32 *size);
static tSInt BOOT_DownloadParameters_CMOST(BootTunerTy tuner_type, tU32 deviceID);


/**************************************
 *
 * BOOT_resetBootParam
 *
 *************************************/
/*!
 * \brief		Initializes the #BootCMOSTParamsTy variable used for firmware download
 * \remark		For #CONFIG_COMM_CMOST_FIRMWARE_FILE the *image* and *image_size* parameters
 * 				are ignored.
 * \param[in,out] param - the boot state variable to be initialized
 * \param[in]	image - pointer to an array containing the firmware image (see #
 * 				        or NULL if the embedded image is used
 * \param[in]	image_size - size in bytes of the *image* parameter
 * \callgraph
 * \callergraph
 */
static tVoid BOOT_resetBootParam(BootCMOSTParamsTy *param, tU8 *image, tU32 image_size)
{
	(void)OSAL_pvMemorySet((tVoid *)param, 0x00, sizeof(BootCMOSTParamsTy));
#if defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED) || defined (CONFIG_COMM_CMOST_FIRMWARE_IMAGE)
	if (image != NULL)
	{
		param->firmware = image;
		param->firmwareSize = image_size;
		param->state = ReadAddress;
	}
	else
	{
		param->state = Init;
	}
#else // CONFIG_COMM_CMOST_FIRMWARE_FILE
	param->state = Init;
#endif
}

/**************************************
 *
 * BOOT_tunerTypeToString
 *
 *************************************/
tChar *BOOT_tunerTypeToString(BootTunerTy type)
{
tChar *ret;

	switch (type)
	{
		case BOOT_TUNER_STAR_T:
			ret = (tChar*)&BootTunerTy_STR[BOOT_TUNER_STAR_T][0];
			break;

		case BOOT_TUNER_STAR_S:
			ret = (tChar*)&BootTunerTy_STR[BOOT_TUNER_STAR_S][0];
			break;
			
		case BOOT_TUNER_DOT_T:
			ret = (tChar*)&BootTunerTy_STR[BOOT_TUNER_DOT_T][0];
			break;
			
		case BOOT_TUNER_DOT_S:
			ret = (tChar*)&BootTunerTy_STR[BOOT_TUNER_DOT_S][0];
			break;
			
		default:
			ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
			ret = NULL;
			break;
	}

return ret;
}

/**************************************
 *
 * BOOT_getFirmwareReferences_CMOST
 *
 *************************************/
/*!
 * \brief		Provides firmware and parameter references for CMOST download
 * \remark		This function is normally static to this file but since one test
 * 				needs to access the firmware info, in the case of #CONFIG_APP_TEST_INITIALIZATION
 * 				it is made global.
 * \remark		All parameters except *tuner_type* may be NULL, in which case they are ignored
 * \param[in]	tuner_type - the type of tuner for which to return the information
 * \param[out]	filename - name of the file containing the firmware
 * 				           (or NULL in case it is #CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED)
 * \param[out]	array - pointer to the array containing the firmware
 * 				        (or NULL in case it is #CONFIG_COMM_CMOST_FIRMWARE_FILE
 * \param[out]	size - size of the firmware array (or 0 in case it is #CONFIG_COMM_CMOST_FIRMWARE_FILE)
 * \param[out]	param_array - pointer to the array containing the CMOST parameters (DISS, WSP, ...)
 * \param[out]	param_entries - number of entries in param_array
 * \return		OSAL_OK
 * \return		OSAL_ERROR - unknown (or not support not compiled) *tuner_type*
 * \callgraph
 * \callergraph
 */
#ifndef CONFIG_APP_TEST_INITIALIZATION
static
#endif
tSInt BOOT_getFirmwareReferences_CMOST(BootTunerTy tuner_type, tChar **filename, tU8 **array, tU32 *size, tU8 **param_array, tU32 *param_entries)
{
	tChar *local_filename_to_convert = NULL;
	tU8 *local_array = NULL;
	tU32 local_size = 0;
	tU8* local_param_array = NULL;
	tU32 local_param_entries = 0;
	tSInt ret = OSAL_OK;
	
	switch (tuner_type)
	{
		case BOOT_TUNER_STAR_T:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	#if defined (CONFIG_COMM_CMOST_FIRMWARE_FILE)
			local_filename_to_convert = BOOT_FIRMWARE_BINARY_STAR_T;
	#elif defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED)
			local_array = (tU8 *)BOOT_ARRAY_STAR_T;
			local_size = sizeof(BOOT_ARRAY_STAR_T);
	#endif
	#if defined (BOOT_PARAM_INCLUDE_STAR_T)
			local_param_array = (tU8 *)BOOT_PARAM_ARRAY_STAR_T;
			local_param_entries = sizeof(BOOT_PARAM_ARRAY_STAR_T) / sizeof(tCoeffInit);
	#endif
#endif
			break;

		case BOOT_TUNER_STAR_S:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	#if defined (CONFIG_COMM_CMOST_FIRMWARE_FILE)
			local_filename_to_convert = BOOT_FIRMWARE_BINARY_STAR_S;
	#elif defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED)
			local_array = (tU8 *)BOOT_ARRAY_STAR_S;
			local_size = sizeof(BOOT_ARRAY_STAR_S);
	#endif
	#if defined (BOOT_PARAM_INCLUDE_STAR_S)
			local_param_array = (tU8 *)BOOT_PARAM_ARRAY_STAR_S;
			local_param_entries = sizeof(BOOT_PARAM_ARRAY_STAR_S) / sizeof(tCoeffInit);
	#endif
#endif
			break;

		case BOOT_TUNER_DOT_T:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	#if defined (CONFIG_COMM_CMOST_FIRMWARE_FILE)
			local_filename_to_convert = BOOT_FIRMWARE_BINARY_DOT_T;
	#elif defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED)
			local_array = (tU8 *)BOOT_ARRAY_DOT_T;
			local_size = sizeof(BOOT_ARRAY_DOT_T);
	#endif
	#if defined (BOOT_PARAM_INCLUDE_DOT_T)
			local_param_array = (tU8 *)BOOT_PARAM_ARRAY_DOT_T;
			local_param_entries = sizeof(BOOT_PARAM_ARRAY_DOT_T) / sizeof(tCoeffInit);
	#endif
#endif
			break;

		case BOOT_TUNER_DOT_S:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	#if defined (CONFIG_COMM_CMOST_FIRMWARE_FILE)
			local_filename_to_convert = BOOT_FIRMWARE_BINARY_DOT_S;
	#elif defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED)
			local_array = (tU8 *)BOOT_ARRAY_DOT_S;
			local_size = sizeof(BOOT_ARRAY_DOT_S);
	#endif
	#if defined (BOOT_PARAM_INCLUDE_DOT_S)
			local_param_array = (tU8 *)BOOT_PARAM_ARRAY_DOT_S;
			local_param_entries = sizeof(BOOT_PARAM_ARRAY_DOT_S) / sizeof(tCoeffInit);
	#endif
#endif
			break;

		default:
			ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
			ret = OSAL_ERROR;
			break;
	}
	if (ret == OSAL_ERROR)
	{
		goto exit;
	}
	if (filename)
	{
		*filename = local_filename_to_convert;
	}
	if (array)
	{
		*array = local_array;
	}
	if (size)
	{
		*size = local_size;
	}
	if (param_array)
	{
		*param_array = local_param_array;
	}
	if (param_entries)
	{
		*param_entries = local_param_entries;
	}

exit:
	return ret;
}

#ifdef CONFIG_COMM_CMOST_FIRMWARE_FILE
/**************************************
 *
 * BOOT_OpenFirmwareFile_CMOST
 *
 *************************************/
static FILE *BOOT_OpenFirmwareFile_CMOST(BootTunerTy tuner_type)
{
	tChar *filename;
	FILE *fd;

	if (BOOT_getFirmwareReferences_CMOST(tuner_type, &filename, NULL, NULL, NULL, NULL) == OSAL_ERROR)
	{
		return NULL;
	}
	if (OSAL_u32StringLength(filename) == 0)
	{
		return NULL;
	}
	fd = fopen(filename, "r");
	if (fd == NULL)
	{
		BOOT_tracePrintError(TR_CLASS_BOOT, "opening the firmware file (%s): %s", filename, strerror(errno));
	}
	return fd;
}

/**************************************
 *
 * BOOT_CloseFirmwareFile_CMOST
 *
 *************************************/
static tSInt BOOT_CloseFirmwareFile_CMOST(FILE *fd)
{
	if (!fclose(fd))
	{
		return OSAL_OK;
	}
	BOOT_tracePrintSysmin(TR_CLASS_BOOT, "Firmware close error: %s", strerror(errno));
	return OSAL_ERROR;
}
#endif // CONFIG_COMM_CMOST_FIRMWARE_FILE


#if defined(CONFIG_TRACE_CLASS_BOOT) && (CONFIG_TRACE_CLASS_BOOT >= TR_LEVEL_SYSTEM_MIN)
#ifdef CONFIG_COMM_CMOST_FIRMWARE_FILE
/**************************************
 *
 * BOOT_State2Ascii_CMOST
 *
 *************************************/
static const tChar *BOOT_State2Ascii_CMOST(FwReadFSMStateTy st)
{
	switch (st)
	{
		case Init:
			return "Init";
		case FirstChar:
			return "FirstChar";
		case ReadAddress:
			return "ReadAddress";
		case ReadData:
			return "ReadData";
		case BufferComplete:
			return "BufferComplete";
		case LineComplete:
			return "LineComplete";
		case FileComplete:
			return "FileComplete";
		case Complete:
			return "Complete";
		case Error:
			return "Error";
		default:
			return "Unknown";
	}
}
#endif // CONFIG_COMM_CMOST_FIRMWARE_FILE
#endif

#ifdef BOOT_READ_BACK_AND_COMPARE_CMOST
/**************************************
 *
 * BOOT_CheckReadBug_CMOST
 *
 *************************************/
/*
 * The CMOST read bug afects only reads from memory.
 * This function tries to guess if the read is from memory
 * or from register, based only on the size of the read, and
 * returns the number of bytes to skip in case
 *
 * This simple implementation does not cope correctly with the write
 * of the last chunk of memory which may be smaller than FW_CHUNK_DATA_SIZE_BYTES_CMOST
 * so expect some mismatch errors in the output of the comparison
 */
static tU8 BOOT_CheckReadBug_CMOST(tS32 size)
{
#if (WORKAROUND_FOR_CMOST_READ_BUG != 0)
	if (size < FW_CHUNK_DATA_SIZE_BYTES_CMOST)
	{
		return 0;
	}
	else
	{
		return WORKAROUND_FOR_CMOST_READ_BUG;
	}
#else
	return 0;
#endif
}

/**************************************
 *
 * BOOT_ReadRawBlock_CMOST
 *
 *************************************/
static tS32 BOOT_ReadRawBlock_CMOST(tU32 addr, tU32 size, tU32 deviceID)
{
	BOOT_tracePrintComponent(TR_CLASS_BOOT, "Read %u bytes from address 0x%.6x", size, addr);
	{
		tU8 w_buf[6];
		tU32 w_len = 0;
		tU32 len;
		tU8 read_bug;
		CMOST_paramsTy param;

		read_bug = BOOT_CheckReadBug_CMOST(size);

		// format is <address*3, sizeinwords*3>
		w_buf[w_len++] = ((addr & 0x0F0000) >> 16);
		w_buf[w_len++] = ((addr & 0x00FF00) >>  8);
		w_buf[w_len++] = ((addr & 0x0000FF) >>  0);
		w_buf[w_len++] = 0;
		w_buf[w_len++] = (((size + read_bug) / 4) & 0xFF00) >> 8;
		w_buf[w_len++] = (((size + read_bug) / 4) & 0xFF);

		if (CMOST_helperSendMessage(&param, w_buf, w_len, CMOST_CMDMODE_RD, CMOST_ACCESSSIZE_32BITS, deviceID, FwTestReadBuf_CMOST, &len) != OSAL_OK)
			{
				BOOT_tracePrintError(TR_CLASS_BOOT, "Reading RAW data from the I2C: %s", strerror(errno));
				return -1;
			}
		if (len < size + read_bug)
		{
			BOOT_tracePrintError(TR_CLASS_BOOT, "Truncated read of RAW data from the I2C: %u instead of %d", len, size + read_bug);
		}

// this is really BSP, not a typo
#if defined(CONFIG_TRACE_CLASS_BSP) && (CONFIG_TRACE_CLASS_BSP >= TR_LEVEL_COMPONENT)
		{
			tU32 i;

			BOOT_tracePrintComponent(TR_CLASS_BOOT, "R %.6x", addr);
			for (i = 0; i < size + read_bug; i += 4)
			{
				COMMON_tracePrintBufComponent(TR_CLASS_BOOT, FwTestReadBuf_CMOST + i, 4, NULL);
			}
		}
#endif
		return (tS32)len;
	}
	return -1;
}

/**************************************
 *
 * BOOT_CompareBlock_CMOST
 *
 *************************************/
static tS32 BOOT_CompareBlock_CMOST(tU32 size)
{
	tU32 i;
	tU8 read_bug;

	read_bug = BOOT_CheckReadBug_CMOST(size);

	for (i = read_bug; i < size + read_bug; i++)
	{
		if (FwTestReadBuf_CMOST[i] != FwChunk_CMOST[i - read_bug + FW_CHUNK_HEADER_RESERVED_CMOST])
		{
			BOOT_tracePrintError(TR_CLASS_BOOT, "Mismatch at position %u (sent %.2X, recv %.2X)", i, FwChunk_CMOST[i - read_bug + FW_CHUNK_HEADER_RESERVED_CMOST], FwTestReadBuf_CMOST[i]);
			return 1;
		}
	}
	return 0;
}
#endif // BOOT_READ_BACK_AND_COMPARE_CMOST

/**************************************
 *
 * BOOT_DownloadSendChunk_CMOST
 *
 *************************************/
/*
 * Writes a block of Firmware data to the CMOST using Raw mode
 * over the I2C or SPI bus.
 *
 * Uses the global FwChunk_CMOST and assumes that the first 
 * FW_CHUNK_HEADER_RESERVED_CMOST locations have been reserved
 * to insert the Raw I2C or SPI header, thus FW data starts from 
 * (FwChunk_CMOST + FW_CHUNK_HEADER_RESERVED_CMOST)
 *
 * Parameters:
 *   addr - the address of the first location of the CMOST to write
 *   size - number of data bytes in FwChunk_CMOST. Must be less than
 *          FW_CHUNK_DATA_SIZE_BYTES_CMOST
 *   device_address - 
 *          the physical address of the device (only some values are leval, see CMOST_addressEnumTy)
 *   useI2C -
 *          TRUE if the device is on the I2C bus, FALSE if it is on the SPI bus
 *
 * Returns:
 *   OSAL_OK    - success
 *   OSAL_ERROR_DEVICE_NOT_OPEN - Device communication error
 *   OSAL_ERROR_INVALID_PARAM - parameter error
 */
static tSInt BOOT_DownloadSendChunk_CMOST(tU32 addr, tS32 size, tU32 deviceID)
{
	tSInt ret = OSAL_OK;
	
	if (size == 0)
	{
		ret = OSAL_OK;
		goto exit;
	}
	if (size < 0)
	{
		ret = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}
	if (size > FW_CHUNK_DATA_SIZE_BYTES_CMOST)
	{
		BOOT_tracePrintError(TR_CLASS_BOOT, "Bad parameter to BOOT_DownloadSendChunk_CMOST: size %d, limit %d", size, FW_CHUNK_DATA_SIZE_BYTES_CMOST);
#if defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED) || defined (CONFIG_COMM_CMOST_FIRMWARE_IMAGE)
		BOOT_tracePrintError(TR_CLASS_BOOT, "Possibly the firmware include file was generated for a different FW_CHUNK_DATA_SIZE_BYTES_CMOST");
#endif
		ret = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}
	if ((addr & 0xF00000) != 0)
	{
		BOOT_tracePrintSysmin(TR_CLASS_BOOT, "Bad parameter to BOOT_DownloadSendChunk_CMOST: addr %x, limit 0x0FFFFF", addr);
	}

	BOOT_tracePrintComponent(TR_CLASS_BOOT, "Writing %d Firmware bytes at address 0x%.6x", size, addr);

	if (TUNERDRIVER_writeRawBlock_CMOST(deviceID, addr, FwChunk_CMOST, (tU32)size) != OSAL_OK)
	{
		ret = OSAL_ERROR_DEVICE_NOT_OPEN;
		goto exit;
	}

#ifdef BOOT_READ_BACK_AND_COMPARE_CMOST
	/* read back and compare */
	BOOT_ReadRawBlock_CMOST(addr, size, deviceID);
	BOOT_CompareBlock_CMOST(size);
#endif

exit:
	return ret;
}


#ifdef CONFIG_COMM_CMOST_FIRMWARE_FILE
/**************************************
 *
 * BOOT_ReadStringFromFile_CMOST
 *
 *************************************/
static tSInt BOOT_ReadStringFromFile_CMOST(tChar *buf, tS32 size, FwReadFSMStateTy *next_state)
{
	tS32 ret;

	ret = fread(buf, 1, size, FwFilep_CMOST);
	if (ret != size)
	{
		if (feof(FwFilep_CMOST))
		{
			*next_state = FileComplete;
			return OSAL_ERROR;
		}
		else
		{
			BOOT_tracePrintError(TR_CLASS_BOOT, "In BOOT_ReadStringFromFile_CMOST (requested %d bytes, got %d)", size, ret);
			*next_state = Error;
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}

/**************************************
 *
 * BOOT_IsEOL_CMOST
 *
 *************************************/
static tS32 BOOT_IsEOL_CMOST(tChar c)
{
	return (c == '\n') || (c == '\r');
}

/**************************************
 *
 * BOOT_SkipFileToEndOfLine_CMOST
 *
 *************************************/
static tSInt BOOT_SkipFileToEndOfLine_CMOST(FwReadFSMStateTy *next_state)
{
	tChar data;

	/* search the first EOL character */
	while (1)
	{
		if (BOOT_ReadStringFromFile_CMOST(&data, 1, next_state) != OSAL_OK)
		{
			/* changes state */
			return OSAL_ERROR;
		}
		if (BOOT_IsEOL_CMOST(data))
		{
			break;
		}
	}

	/* consume all EOL characters */
	while (1)
	{
		if (BOOT_ReadStringFromFile_CMOST(&data, 1, next_state) != OSAL_OK)
		{
			/* changes state */
			return OSAL_ERROR;
		}
		if (!BOOT_IsEOL_CMOST(data))
		{
			break;
		}
	}
	/* push back the last character */
	fseek(FwFilep_CMOST, -1, SEEK_CUR);
	return OSAL_OK;
}

/**************************************
 *
 * BOOT_ConvertDigit_CMOST
 *
 *************************************/
static tSInt BOOT_ConvertDigit_CMOST(tChar buf, tS32 *val)
{		
	if ((buf >= '0') && (buf <= '9'))
	{
		*val = buf - '0';
	}
	else if ((buf >= 'A') && (buf <= 'F'))
	{
		*val = buf - 'A' + 0xA;
	}
	else if ((buf >= 'a') && (buf <= 'f'))
	{
		*val = buf - 'a' + 0xA;
	}
	else
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/**************************************
 *
 * BOOT_Convert_CMOST
 *
 *************************************/
static tSInt BOOT_Convert_CMOST(tChar *buf, tS32 *val, tS32 size)
{
	tS32 i;
	tS32 hi, lo;

	*val = 0;
	for (i = 0; i < size; i += 2)
	{
		if (BOOT_ConvertDigit_CMOST(buf[i], &hi) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (BOOT_ConvertDigit_CMOST(buf[i + 1], &lo) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		*val = (*val << 8) | (hi * 16 + lo);
	}
	return OSAL_OK;
}

/**************************************
 *
 * BOOT_ConvertAddress_CMOST
 *
 *************************************/
static tSInt BOOT_ConvertAddress_CMOST(tChar *buf, tS32 *addr)
{
	return BOOT_Convert_CMOST(buf, addr, 6);
}

/**************************************
 *
 * BOOT_ConvertData_CMOST
 *
 *************************************/
static tSInt BOOT_ConvertData_CMOST(tChar *buf, tS32 *data)
{
	return BOOT_Convert_CMOST(buf, data, 8);
}

/**************************************
 *
 * BOOT_ReadLineFromFile_CMOST
 *
 *************************************/
/*
 * 
 */
/*!
 * \brief		Read a CMOST firmware file written in the original (<filename>.boot) format
 *
 *\details		Supported file format:
 *				Each line contains the following data
 *				  W addr data*N
 *				where
 *				  the first character is a capital 'W',
 *				  addr is a 6-digit hex number
 *				  data*N is a sequence of N 8-digit hex numbers, separated by a space
 *				Lines starting with '/' are treated as comments and ignored.
 *				Example:
 *				   // TDA7707 OM FW version: 3.8.0
 *				   W 014001 FE7FFCFF FE7FFCFF
 *				   W 014010 FFFFFFFF 000000FF FEFEFEFE 000000FE
 *
 *				The function must fill #FwChunk_CMOST so that it contains at most 
 *				#FW_CHUNK_DATA_SIZE_BYTES_CMOST of data and the data starts from
 *				offset #FW_CHUNK_HEADER_RESERVED_CMOST
 * \param[in,out] param - the function status
 * \param[out]	addr - pointer to a location where the function writes
 *        		       the CMOST address of the first data contained in
 *				       the #FwChunk_CMOST
 * \param[out]	size - pointer to location where the function writes
 *				       the numbef of valid bytes of #FwChunk_CMOST
 * \return		 0 - success
 *				 1 - success, end of file
 *				-1 - error, <addr> and <size> should be ignored
 * \callgraph
 * \callergraph
 */
static tS32 BOOT_ReadLineFromFile_CMOST(BootCMOSTParamsTy *param, tU32 *addr, tS32 *size)
{
	tChar data[10];
	tBool exit_flag = 0;
	tBool end_of_file = 0;
	tS32 curr_data = 0;

	while (!exit_flag)
	{
		//BOOT_tracePrintComponent(TR_CLASS_BOOT, "Current state %s", BOOT_State2Ascii_CMOST(state));
		switch (param->state)
		{
			case Init:
				if (FwFilep_CMOST == NULL)
				{
					BOOT_tracePrintError(TR_CLASS_BOOT, "Firmware file not open");
					param->state = Error;
					break;
				}
				param->state = FirstChar;
				param->firstAddress = 0x0;
				param->currentAddress = 0x0;
				param->nextLineAddress = 0x0;
				param->offset = FW_CHUNK_HEADER_RESERVED_CMOST;
				break;

			case FirstChar:
				if (BOOT_ReadStringFromFile_CMOST(&data[0], 1, &param->state) != OSAL_OK)
				{
					/* changes state */
					break;
				}
				if (data[0] == '/')
				{
					/* no need to check the return value, this call changes the state if required */
					BOOT_SkipFileToEndOfLine_CMOST(&param->state);
					break;
				}
				else if (BOOT_IsEOL_CMOST(data[0]))
				{
					BOOT_tracePrintSysmin(TR_CLASS_BOOT, "EOL in %s", BOOT_State2Ascii_CMOST(param->state));
					break;
				}
				else if ((data[0] != 'W') && (data[0] != 'w'))
				{
					BOOT_tracePrintError(TR_CLASS_BOOT, "Firmware file unrecognized first character (\'%c\', 0x%.2x)", data[0], data[0]);
					param->state = Error;
					break;
				}
				param->state = ReadAddress;
				break;


			case ReadAddress:
				/* address is prefixed by a single ' ', read it now and skip it later */
				if (BOOT_ReadStringFromFile_CMOST(&data[0], 7, &param->state) != OSAL_OK)
				{
					if (param->state == FileComplete)
					{
						BOOT_tracePrintSysmin(TR_CLASS_BOOT, "end of Firmware file while parsing a line");
					}
					break;
				}
#if defined(CONFIG_TRACE_CLASS_BOOT) && (CONFIG_TRACE_CLASS_BOOT >= TR_LEVEL_COMPONENT)
				data[7] = '\0';
#endif
				BOOT_tracePrintComponent(TR_CLASS_BOOT, "Parsing new Firmware line starting with '%s'", data);
				if (data[0] != ' ')
				{
					param->state = LineComplete;
					break;
				}
				if (BOOT_ConvertAddress_CMOST(&data[1], &param->firstAddress) != OSAL_OK)
				{
					param->state = Error;
					break;
				}
				param->currentAddress = param->firstAddress;
				param->nextLineAddress = param->firstAddress;
				param->state = ReadData;
				break;

			case ReadData:
				if (param->offset >= FW_CHUNK_SIZE_BYTES_CMOST)
				{
					param->state = BufferComplete;
					break;
				}
				if (BOOT_ReadStringFromFile_CMOST(&data[0], 1, &param->state) != OSAL_OK)
				{
					break;
				}
				if (data[0] != ' ')
				{
					if (BOOT_SkipFileToEndOfLine_CMOST(&param->state) != OSAL_OK)
					{
						break;
					}
					param->state = LineComplete;
					break;
				}

				if (BOOT_ReadStringFromFile_CMOST(&data[0], 8, &param->state) != OSAL_OK)
				{
					break;
				}
				if (BOOT_ConvertData_CMOST(&data[0], &curr_data) != OSAL_OK)
				{
					param->state = Error;
					break;
				}
				*(FwChunk_CMOST + param->offset + 0) = (curr_data & 0xFF000000) >> 24;
				*(FwChunk_CMOST + param->offset + 1) = (curr_data & 0x00FF0000) >> 16;
				*(FwChunk_CMOST + param->offset + 2) = (curr_data & 0x0000FF00) >>  8;
				*(FwChunk_CMOST + param->offset + 3) = (curr_data & 0x000000FF) >>  0;
				param->offset += 4;
				param->currentAddress += 1; // CMOST addresses are for 32-bit words
				break;

			case BufferComplete:
				param->state = ReadData;
				exit_flag = 1;
				break;
				
			case LineComplete:
				param->state = FirstChar;
				exit_flag = 1;
				BOOT_tracePrintComponent(TR_CLASS_BOOT, "Firmware line, total %d bytes at addr 0x%.6x", (param->currentAddress - param->firstAddress) * 4, param->firstAddress);
				break;

			case FileComplete:
				param->state = Complete;
				end_of_file = 1;
				exit_flag = 1;
#if defined(CONFIG_TRACE_CLASS_BOOT) && (CONFIG_TRACE_CLASS_BOOT >= TR_LEVEL_COMPONENT)
				if (param->offset > FW_CHUNK_HEADER_RESERVED_CMOST)
				{
					BOOT_tracePrintComponent(TR_CLASS_BOOT, "Firmware line, total %d bytes at addr 0x%.6x", (param->currentAddress - param->firstAddress) * 4, param->firstAddress);
				}
				BOOT_tracePrintComponent(TR_CLASS_BOOT, "Firmware file completed");
#endif
				break;

			case Complete:
				param->state = Init; // for debug, used during continous loops
				end_of_file = 1;
				exit_flag = 1;
				break;

			case Error:
				BOOT_tracePrintError(TR_CLASS_BOOT, "Reading Firmware file");
				exit_flag = 1;
		}
	}
	if (param->state != Error)
	{
		*size = param->offset - FW_CHUNK_HEADER_RESERVED_CMOST;
		*addr = param->nextLineAddress;
		BOOT_tracePrintComponent(TR_CLASS_BOOT, "Firmware chunk complete (addr 0x%.8x, size %d, bytes from start %d)", *addr, *size, (param->currentAddress - param->firstAddress) * 4);
#if defined(CONFIG_TRACE_CLASS_BOOT) && (CONFIG_TRACE_CLASS_BOOT >= TR_LEVEL_COMPONENT)
		/* keep under ifdef for the BOOT_State2Ascii_CMOST */
		BOOT_tracePrintComponent(TR_CLASS_BOOT, "Next state %s", BOOT_State2Ascii_CMOST(param->state));
#endif
		param->nextLineAddress = param->currentAddress;
		param->offset = FW_CHUNK_HEADER_RESERVED_CMOST;
		return end_of_file;
	}
	BOOT_tracePrintError(TR_CLASS_BOOT, "Parsing the Firmware file");
	return -1;
}
#endif // CONFIG_COMM_CMOST_FIRMWARE_FILE

#if defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED) || defined (CONFIG_COMM_CMOST_FIRMWARE_IMAGE)
/**************************************
 *
 * BOOT_ReadLineFromArray_CMOST
 *
 *************************************/
/*!
 * \brief		Read a CMOST firmware array as defined in the .boot.h format
 * \details		The format of the firmware array is:
 * 				ADDRESS                                                      [3 bytes], 
 *				NUMBER OF BYTES to be initialized starting from the ADDRESS  [2 bytes],
 *				DATA - each word consists 4 bytes stored                     [number of bytes]
 *				ADDRESS                                                      [3 bytes], 
 *				NUMBER OF BYTES to be initialized starting from the ADDRESS  [2 bytes],
 *				DATA - each word consists 4 bytes                            [number of bytes]
 *				...
 *				TERMINATION: 0xFFFFFF                                        [3 bytes]
 *
 * 				The image chunk is stored to a global variable named #FwChunk_CMOST; the function
 * 				parameter only contain the address of the CMOST location where the image chunk
 * 				should be written (*addr*) and its size (*size*).
 *
 * 				The function tries to copy #FW_CHUNK_DATA_SIZE_BYTES_CMOST bytes
 * 				at every invocation except the last one.
 * \param[in,out] param - state variable to be initialized before first invocation
 * 				          of this function through #BOOT_resetBootParam and then
 * 				          used by this function to store its state
 * \param[in]	tuner_type - the type of CMOST; ignored if the firmware is read from an
 * 				             array passed through the *image* and *image_size* parameters of
 * 				             #BOOT_Download_CMOST
 * \param[in]	addr - pointer to a location where the function stores the address of the CMOST
 * 				       where the chunk of data contained in #FwChunk_CMOST must be written
 * \param[in]	size - pointer to a location where the function stores the size of the chunk
 * 				       of data written to #FwChunk_CMOST
 * \return		-1 error
 * \return		 1 no error, end of file or array	
 * \return		 0 no error, more data to process
 * \callgraph
 * \callergraph
 */
static tS32 BOOT_ReadLineFromArray_CMOST(BootCMOSTParamsTy *param, BootTunerTy tuner_type, tU32 *addr, tS32 *size)
{
	tS32 ret = -1;
	tBool exit_flag = 0;

	while(!exit_flag)
	{
		switch (param->state)
		{
			case Init:
				if (BOOT_getFirmwareReferences_CMOST(tuner_type, NULL, &param->firmware, &param->firmwareSize, NULL, NULL) == OSAL_ERROR)
				{
					// programming error, ASSERT already hit
					ret = -1;
					exit_flag = 1;
					break;
				}
				if (param->firmware == NULL)
				{
					BOOT_tracePrintError(TR_CLASS_BOOT, "Support for %s not built in", BOOT_tunerTypeToString(tuner_type));
					ret = -1;
					exit_flag = 1;
					break;
				}
				param->currentPosition = 0;
				param->state = ReadAddress;
				break;
	
			case ReadAddress:
	
				if (param->currentPosition >= param->firmwareSize)
				{
					ret = -1;
					exit_flag = 1;
					break;
				}
				param->targetAddress  = (tU32)(*(param->firmware + param->currentPosition++)) << 16;
				param->targetAddress |= (tU32)(*(param->firmware + param->currentPosition++)) <<  8;
				param->targetAddress |= (tU32)(*(param->firmware + param->currentPosition++)) <<  0;
				if (param->targetAddress == FW_END_MARKER)
				{
					*size = 0;
					*addr = 0;
					param->state = Init; // for debug, used during continous loops
					ret = 1;
					exit_flag = 1;
					break;
				}
				param->remainingRowBytes  = (tU32)(*(param->firmware + param->currentPosition++)) <<  8;
				param->remainingRowBytes |= (tU32)(*(param->firmware + param->currentPosition++)) <<  0;
				param->state = ReadData;
				break;
	
			case ReadData:
				*addr = param->targetAddress;
				if (param->remainingRowBytes > FW_CHUNK_DATA_SIZE_BYTES_CMOST)
				{
					*size = FW_CHUNK_DATA_SIZE_BYTES_CMOST;
				}
				else
				{
					/* end of the row reached */
					*size = (tS32)param->remainingRowBytes;
					param->state = ReadAddress;
				}
				(void)OSAL_pvMemoryCopy((tVoid *)(FwChunk_CMOST + FW_CHUNK_HEADER_RESERVED_CMOST), (tPCVoid)(param->firmware + param->currentPosition), (tU32)(*size));
				param->remainingRowBytes -= *size;
				param->currentPosition += *size;
				param->targetAddress += (tU32)(*size) >> 2; // CMOST Addresses are for 32-bit words
				ret = 0;
				exit_flag = 1;
				break;
	
			default:
				ret = -1;
				exit_flag = 1;
				break;
		}
	}
	
	return ret;
	
}
#endif

/**************************************
 *
 * BOOT_DownloadReadLine_CMOST
 *
 *************************************/
/*!
 * \brief		Entry point to read firmware data from file or memory
 * \details		Read one 'line' of CMOST FW initialization data from 
 *				file or memory and splits it into one or more FwChunk_CMOST.
 *
 *				The function should hide the origin of the FW data, 
 *				be it read from memory or from a file.
 *
 *				For file format see #BOOT_ReadLineFromFile_CMOST
 *				For memory array format see #BOOT_ReadLineFromArray_CMOST
 * \param[in,out] param - the function status
 * \param[in]	tuner_type - the tuner type (only used for #CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED)
 * \param[out]	addr - pointer to a location where the function writes
 *        		       the CMOST address of the first data contained in
 *				       the #FwChunk_CMOST
 * \param[out]	size - pointer to location where the function writes
 *				       the numbef of valid bytes of #FwChunk_CMOST
 * \return		 0 - success
 *				 1 - success, end of file
 *				-1 - error, <addr> and <size> should be ignored
 * \callgraph
 * \callergraph
 */

static tS32 BOOT_DownloadReadLine_CMOST(BootCMOSTParamsTy *param, BootTunerTy tuner_type, tU32 *addr, tS32 *size)
{
#ifdef CONFIG_COMM_CMOST_FIRMWARE_FILE
	(tVoid) tuner_type; // lint
	return BOOT_ReadLineFromFile_CMOST(param, addr, size);
#elif defined (CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED) || defined (CONFIG_COMM_CMOST_FIRMWARE_IMAGE)
	return BOOT_ReadLineFromArray_CMOST(param, tuner_type, addr, size);
#endif
}

/**************************************
 *
 * BOOT_DownloadParameters_CMOST
 *
 *************************************/
/*
 * Reads a global array containing the parameters
 * and sends it to the device.
 * The parameter array is composed of (address, value) entries and is
 * defined in boot_cmost.h.
 *
 * The array may be undefined, in this case this function has no effect
 */
static tSInt BOOT_DownloadParameters_CMOST(BootTunerTy tuner_type, tU32 deviceID)
{
	tU8 *param_array;
	tU32 param_array_entries;
	tSInt ret = OSAL_OK;

#if defined (BOOT_PARAM_INCLUDE_STAR_T) || \
	defined (BOOT_PARAM_INCLUDE_STAR_S) || \
	defined (BOOT_PARAM_INCLUDE_DOT_T)  || \
	defined (BOOT_PARAM_INCLUDE_DOT_S)
	tU8 cmd[] = {0x00, 0x1F, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	tU8 resp_buf[CMOST_MAX_RESPONSE_LEN]; /* avoid static to protect against task switch corruption */
	tU32 len;
	tU32 i;
	tU32 address, value;
#endif

	if (BOOT_getFirmwareReferences_CMOST(tuner_type, NULL, NULL, NULL, &param_array, &param_array_entries) == OSAL_ERROR)
	{
		ret = OSAL_ERROR;
		goto exit;
	}
		
	if (param_array_entries > 0)
	{
	    if (param_array == NULL)
	    {
	        ret = OSAL_ERROR;
	        goto exit;
	    }
#if defined (BOOT_PARAM_INCLUDE_STAR_T) || \
	defined (BOOT_PARAM_INCLUDE_STAR_S) || \
	defined (BOOT_PARAM_INCLUDE_DOT_T)  || \
	defined (BOOT_PARAM_INCLUDE_DOT_S)
		BOOT_tracePrintComponent(TR_CLASS_BOOT, "Downloading embedded parameters to device ID %d",
			deviceID);

		for (i = 0; i < param_array_entries; i++)
		{
			/* this code assumes the paramter array is in 32-bit format */
			address = (((tCoeffInit *)param_array) + i)->coeffAddr;
			value =   (((tCoeffInit *)param_array) + i)->coeffVal;

			cmd[3] = (tU8)((address & 0x00FF0000) >> 16);
			cmd[4] = (tU8)((address & 0x0000FF00) >>  8);
			cmd[5] = (tU8)((address & 0x000000FF) >>  0);

			cmd[6] = (tU8)((value & 0x00FF0000) >> 16);
			cmd[7] = (tU8)((value & 0x0000FF00) >>  8);
			cmd[8] = (tU8)((value & 0x000000FF) >>  0);

			if (TUNERDRIVER_sendCommand_CMOST(deviceID, cmd, sizeof(cmd), resp_buf, &len) != OSAL_OK)
			{
				ret = OSAL_ERROR;
				goto exit;
			}
		}
#endif
	}
	else
	{
		BOOT_tracePrintComponent(TR_CLASS_BOOT, "Skipping embedded parameters download to device id %d",
			deviceID);
	}

exit:
	return ret;
}

/**************************************
 *
 * BOOT_Download_CMOST
 *
 *************************************/
/*!
 * \brief		Downloads the Firmware and patches to a CMOST
 * \details		Writes the data contained in a .boot file or from
 * 				a global array to the CMOST using Raw writes.
 *
 *				Optionally (if #BOOT_MANAGE_DSP_RESET is defined)
 *				removes the DSPs from reset via Raw write into a CMOST register
 *				   @14012 FFFFFFFF
 *				   @14013 000000FF
 *
 *				Verifies correct download by reading one control location (magic number)
 *				   @20180 = 0xAFFE4200
 * \remark		
 * \param[in]	tuner_type - Tuner type from which the image to download is derived
 * \param[in]	device_address - physical address of the device. Not all values are accepted,
 * 				                 see #CMOST_addressEnumTy
 * \param[in]	useI2C - TRUE if the device is on the I2C bus, FALSE if it is on the SPI bus
 * \param[in]	image - if NULL it is ignored (and also *image_size*)
 *                      if non-NULL it points to an array containing a CMOST firmware that the
 *                      function uses to initialize the CMOST instead of the .boot file or the
 *                      built-in array. The format of the array is the same as the one
 *                      documented for #BOOT_ReadLineFromArray_CMOST
 * \param[in]	image_size - size in bytes of the *image* array
 * \param[in]	load_default_params - if TRUE the driver loads the parameters embedded in ETAL
 * 				                      to the Tuner, after firmware download an start
 * \return		OSAL_OK    - success
 * \return		OSAL_ERROR - the CMOST did not respond the expected magic number
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - Device communication error
 * \return		OSAL_ERROR_INVALID_PARAM - Invalid parameter
 * \callgraph
 * \callergraph
 */
tSInt BOOT_Download_CMOST(BootTunerTy tuner_type, tU32 deviceID, tU8 *image, tU32 image_size, tBool load_default_params)
{
	tSInt retosal;
	tSInt ret_func;
	tU32 addr;
	tS32 size;
	tS32 ret;
	tU32 loop;
	tBool complete = FALSE;
	tU8 bufr[4];
	BootCMOSTParamsTy param; /* the state variable for BOOT_DownloadReadLine_CMOST */

#ifdef CONFIG_COMM_CMOST_FIRMWARE_FILE
	FwFilep_CMOST = BOOT_OpenFirmwareFile_CMOST(tuner_type);
	if (FwFilep_CMOST == NULL)
	{
		ret_func = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}
#endif

#ifdef CMOST_DEBUG_DUMP_TO_FILE
	CMOST_StartDump();
#endif

	BOOT_tracePrintSysmin(TR_CLASS_BOOT, "Starting %s Firmware download, device ID %d, busI2C=%d", BOOT_tunerTypeToString(tuner_type),
			deviceID,
			(TUNERDRIVER_GetBusType(deviceID) == BusI2C));

	/* read the Firmware or Patches and send them to the CMOST */

	BOOT_resetBootParam(&param, image, image_size);
	while (TRUE)
	{
		ret = BOOT_DownloadReadLine_CMOST(&param, tuner_type, &addr, &size);
		if (ret < 0)
		{
			ret_func = OSAL_ERROR_INVALID_PARAM;
			goto exit;
		}
		retosal = BOOT_DownloadSendChunk_CMOST(addr, size, deviceID);
		if (retosal != OSAL_OK)
		{
			ret_func = retosal;
			goto exit;
		}
		if (ret == 1)
		{
			break;
		}

	}

#ifdef CMOST_DEBUG_DUMP_TO_FILE
	CMOST_StopDump();
#endif

#ifdef BOOT_MANAGE_DSP_RESET
	/*
	 * Remove the DSP from reset.
	 * Not needed with recent CMOST images since the image itself
	 * writes to the DSP reset register as last instruction
	 */

	BOOT_tracePrintComponent(TR_CLASS_BOOT, "Removing DSP reset");
	{
		if (TUNERDRIVER_writeRaw32_CMOST(deviceID, 0x014012, 0xFFFFFFFF) != OSAL_OK)
		{
			ret_func = OSAL_ERROR_DEVICE_NOT_OPEN;
			goto exit;
		}
		if (TUNERDRIVER_writeRaw32_CMOST(deviceID, 0x014013, 0x000000FF) != OSAL_OK)
		{
			ret_func = OSAL_ERROR_DEVICE_NOT_OPEN;
			goto exit;
		}
	}
#endif // BOOT_MANAGE_DSP_RESET

	/* test magic location at address 0x020180 */

	loop = 0;
	while (loop < 5)
	{
		(void)OSAL_s32ThreadWait(100);
		BOOT_tracePrintComponent(TR_CLASS_BOOT, "Testing magic location");
		if (TUNERDRIVER_readRaw32_CMOST(deviceID, 0x020180, bufr) != OSAL_OK)
		{
//			BOOT_tracePrintError(TR_CLASS_BOOT, "TUNERDRIVER_readRaw32_CMOST failure");
			ret_func = OSAL_ERROR_DEVICE_NOT_OPEN;
			goto exit;
		}
		if ((bufr[0] == (tU8)0xAF) && (bufr[1] == (tU8)0xFE) && (bufr[2] == (tU8)0x42) && (bufr[3] == (tU8)0x00))
		{
			complete = TRUE;
			break;
		}
		else
		{
//			BOOT_tracePrintError(TR_CLASS_BOOT, "Magic Location failure count = %d", loop);
		}
		loop++;
	}

#ifdef CONFIG_COMM_CMOST_FIRMWARE_FILE
	if (NULL != FwFilep_CMOST)
	{
		BOOT_CloseFirmwareFile_CMOST(FwFilep_CMOST);
		FwFilep_CMOST = NULL;
	}
#endif

	if (complete)
	{
		BOOT_tracePrintSysmin(TR_CLASS_BOOT, "Firmware download complete!");
		if (load_default_params)
		{
			if (BOOT_DownloadParameters_CMOST(tuner_type, deviceID) != OSAL_OK)
			{
				BOOT_tracePrintSysmin(TR_CLASS_BOOT, "Parameter download error, continuing anyway");
			}
		}
		ret_func = OSAL_OK;
		goto exit;
	}
	else
	{
		BOOT_tracePrintError(TR_CLASS_BOOT, "Magic Location failure");
	}

	ret_func = OSAL_ERROR;

exit:
	return ret_func;
}

#endif // CONFIG_ETAL_SUPPORT_CMOST

//EOF

	
