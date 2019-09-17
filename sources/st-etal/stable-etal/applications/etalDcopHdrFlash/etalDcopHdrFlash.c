//!
//!  \file 		etalDcopMdrFlash.c
//!  \brief 	<i><b> DCOP HD Radio firmware download application </b></i>
//!  \details   DCOP HD Radio (DCOP-STA680) firmware download application
//!  \author 	David Pastor
//!

/*
 * This application shows how to  
 * download the DCOP HD Radio Firmware or Patches to the DCOP HD Radio device.
 * copy phase1.bin and dcop_fw.bin in same directory than the application
 * and run the application.
 *
 * Tested on Accordo2/Linux board with CMOST STAR-T module and DCOP HD Radio module
 */

#include "target_config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "etal_api.h"

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
#undef CONFIG_DCOP_FLASH_PROGRAM_IN_ROM

/*
 *  Boot mode define
 */
#ifndef CONFIG_DCOP_FLASH_PROGRAM_IN_ROM
#ifdef CONFIG_HOST_OS_TKERNEL
// File name path : /sda = SDCard 0
//#define ETAL_DCOP_FLASH_DUMP_FILENAME       "/sda/tuner/firmware/dcop/flashDump.bin"
//#define ETAL_DCOP_FLASH_BOOTSTRAP_FILENAME  "/sda/tuner/firmware/dcop/phase1.bin"
//#define ETAL_DCOP_FLASH_PROGRAM_FILENAME    "/sda/tuner/firmware/dcop/dcop_fw.bin"
#define ETAL_DCOP_FLASH_DUMP_FILENAME       "/uda/flashDump.bin"
#define ETAL_DCOP_FLASH_BOOTSTRAP_FILENAME  "/uda/phase1.bin"
#define ETAL_DCOP_FLASH_PROGRAM_FILENAME    "/uda/dcop_fw.bin"

#else
#define ETAL_DCOP_FLASH_DUMP_FILENAME       "flashDump.bin"
#define ETAL_DCOP_FLASH_BOOTSTRAP_FILENAME  "phase1.bin"
#define ETAL_DCOP_FLASH_PROGRAM_FILENAME    "dcop_fw.bin"
#endif // CONFIG_HOST_OS_TKERNEL
#else
// include the files
#include	"phase1.bin.h"
#include	"dcop_fw.bin.h"
#endif

#define ETAL_DCOP_BOOT_FILENAME_MAX_SIZE      256

// The FLASH WRITE command has an header of 12 bytes (11 at the begin, 1 at the end). In order
#define FLASH_WRITE_HEADER_SIZE             ((tU32)12)
#define WRITE_FLASH                         ((tU8)0x38)

/*****************************************************************
| types
|----------------------------------------------------------------*/
#ifdef CONFIG_HOST_OS_TKERNEL
typedef struct {
	tBool flashDump;
	tBool flashProgram;
	tBool downloadMemory;
	tChar bootstrap_file[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE];
	tChar program_file[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE];
}etalDcopFlashOptionTy;
#endif

/*****************************************************************
| variables
|----------------------------------------------------------------*/
#ifndef CONFIG_DCOP_FLASH_PROGRAM_IN_ROM
static tChar ETALDcopFlash_bootstrap_filename[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE] = ETAL_DCOP_FLASH_BOOTSTRAP_FILENAME;
static tChar ETALDcopFlash_program_filename[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE] = ETAL_DCOP_FLASH_PROGRAM_FILENAME;
#endif
static tBool ETALDcopFlash_doFlashDump = FALSE;
static tBool ETALDcopFlash_doFlashProgram = FALSE;
static tBool ETALDcopFlash_doDownloadMemory = FALSE;
static tBool ETALDcopMdrFlash_addXloaderHeader = FALSE;
#ifdef CONFIG_ETAL_DCOP_FLASH_FIRMWARE_DL_FILE_MODE
static tChar *ETALDcopFlash_filename[2] = {ETALDcopFlash_bootstrap_filename, ETALDcopFlash_program_filename};
#endif


/*****************************************************************
| functions prototypes
|----------------------------------------------------------------*/
tVoid etalDcopFlash_ShowUsage(char *prgName);
int etalDcopFlash_parseParameters(int argc, char **argv);
static int etalDcopFlash_initialize_and_FlashDcop(tBool doFlashDump, tBool doFlashProgram, tBool doDownloadMemory, tBool addXloaderHeader);
int etalDcopFlash_GetImageCb (void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap);
void etalDcopFlash_PutImageCb (void *pvContext, tU8* block, tU32 providedByteNum, tU32 remainingByteNum);
#ifdef CONFIG_HOST_OS_TKERNEL
int etalDcopHdrFlash_EntryPoint(etalDcopFlashOptionTy *pI_etalDcopFlashOption);
#endif

/*****************************************************************
| functions
|----------------------------------------------------------------*/

/***************************
 *
 * userNotificationHandler
 *
 **************************/
static void userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus)
{
	printf("Unexpected event %d\n", dwEvent);
}

/***************************
 *
 * ConvertBaseUInt2Bin
 *
 **************************/
 #if 0
/* converted parameter 3 type to tS32 (from size_t) because a size_t may be unsigned
 * and thus the first comparison below causes a warning */
static tU32 ConvertBaseUInt2Bin(unsigned int baseUInt, unsigned char *baseBin, int baseLen, unsigned char LittleEndian)
{
	unsigned int i, baseLen_unsign;

	if (baseLen < 0)
	{
		assert(0);
		baseLen_unsign = 0;
	}
	else
	{
		baseLen_unsign = (tU32)baseLen;
	}
	for (i = 0; i<baseLen_unsign; i++)
	{
		if (LittleEndian)
		{
			baseBin[baseLen_unsign - i - 1] = (unsigned char)(baseUInt >> (8 * i));
		}
		else
		{
			baseBin[i] = (unsigned char)(baseUInt >> (8 * i));
		}
	}

	return (baseLen_unsign - i); 
}

/***************************
 *
 * DCOP_ComputeChecksum
 *
 **************************/
static tVoid DCOP_ComputeChecksum (unsigned char *pData, unsigned int len)
{
	unsigned int temp = 0, ckm = 0;

	for (ckm = 0; ckm < (len - 1); ckm++)
	{
		temp += (unsigned int)pData[ckm];
	}

	pData[ckm] = (unsigned char)(temp & 0xFF);
}
#endif

#ifndef CONFIG_DCOP_FLASH_PROGRAM_IN_ROM
/***************************
 *
 * etalDcopFlash_GetImageCb
 *
 **************************/
int etalDcopFlash_GetImageCb(void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap)
{
	static FILE *fhbs = NULL, *fhpg = NULL;
	static long file_bs_size = 0, file_pg_size = 0;
	FILE **fhgi;
	long file_position = 0, *file_size = NULL;
	size_t retfread;
	int do_get_file_size = 0;

	/* open file bootstrap or program firmware if not already open */
	if (isBootstrap != 0)
	{
		fhgi = &fhbs;
		file_size = &file_bs_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(ETALDcopFlash_bootstrap_filename, "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, ETALDcopFlash_bootstrap_filename);
			return errno;
		}
	}
	else
	{
		fhgi = &fhpg;
		file_size = &file_pg_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(ETALDcopFlash_program_filename, "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, ETALDcopFlash_program_filename);
			return errno;
		}
	}

	if (do_get_file_size == 1)
	{
		/* read size of file */
		if (fseek(*fhgi, 0, SEEK_END) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_END) %d\n", errno);
			return errno;
		}
		if ((*file_size = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell end of file %d\n", errno);
			return errno;
		}
		if (fseek(*fhgi, 0, SEEK_SET) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_SET) %d\n", errno);
			return errno;
		}
	}

	/* set remaining bytes number */
	if (remainingByteNum != NULL)
	{
		if ((file_position = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell %d\n", errno);
			return errno;
		}
		*remainingByteNum = (*file_size - file_position);
	}

	/* read requestedByteNum bytes in file */
	retfread = fread(block, 1, requestedByteNum, *fhgi);
	*returnedByteNum = (tU32) retfread;
	if (*returnedByteNum != requestedByteNum)
	{
		if (ferror(*fhgi) != 0)
		{
			/* error reading file */
			if (isBootstrap != 0)
			{
				printf("Error %d reading file %s\n", errno, ETALDcopFlash_bootstrap_filename);
			}
			else
			{
				printf("Error %d reading file %s\n", errno, ETALDcopFlash_program_filename);
			}
			clearerr(*fhgi);
			fclose(*fhgi);
			*fhgi = NULL;
			return -1;
		}
	}

	/* Close file if EOF */
	if (feof(*fhgi) != 0)
	{
		/* DCOP bootstrap or flash program successful */
		clearerr(*fhgi);
		fclose(*fhgi);
		*fhgi = NULL;
	}
	return 0;
}
#else // DCOP_FLASH_IN_ROM
/***************************
 *
 * etalDcopFlash_GetImageCb
 *
 **************************/
int etalDcopFlash_GetImageCb(void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap)
{
	static long file_bs_size = 0, file_pg_size = 0;
	static long file_bs_position = 0, file_pg_position = 0;
	long *file_position, file_size = 0;
	const unsigned char *pl_bin;

	/* open file bootstrap or program firmware if not already open */
	if (isBootstrap != 0)
	{
		pl_bin = phase1_bin;
		file_bs_size = PHASE1_BIN_SIZE;
		file_size = file_bs_size;
		file_position = &file_bs_position;
	}
	else
	{
		pl_bin = dcop_fw_bin;
		file_pg_size = DCOP_FW_BIN_SIZE;
		file_size = file_pg_size;
		file_position = &file_pg_position;
	}


	/* set remaining bytes number */
	if (remainingByteNum != NULL)
	{
		*remainingByteNum = (file_size - *file_position);
	}

	/* read requestedByteNum bytes in file */
	if (requestedByteNum >= *remainingByteNum)
	{
		*returnedByteNum = *remainingByteNum;
	}
	else
	{
		*returnedByteNum = requestedByteNum;
	}

	// copy the data
	// from current position, requested byte
	if (NULL != block)
	{
		memcpy(block, (pl_bin + *file_position), *returnedByteNum);
	}
	else
	{
		return -1;
	}


	*file_position += *returnedByteNum;

	/* Close file if EOF */
	if (*file_position == file_size)
	{
		*file_position = 0;
	}

	
	return 0;
}

#endif

#ifndef CONFIG_DCOP_FLASH_PROGRAM_IN_ROM

/***************************
 *
 * etalDcopFlash_PutImageCb
 *
 **************************/
void etalDcopFlash_PutImageCb (void *pvContext, tU8* block, tU32 providedByteNum, tU32 remainingByteNum)
{
	static FILE *fhpu = NULL;
	size_t nb_byte_written;

	/* open file dump if not already open */
	if (fhpu == NULL)
	{
		fhpu = fopen(ETAL_DCOP_FLASH_PROGRAM_FILENAME, "w");
	}
	if (fhpu == NULL)
	{
		printf("Error %d opening file %s\n", errno, ETAL_DCOP_FLASH_PROGRAM_FILENAME);
	}
	else
	{
		/* write dump file */
		nb_byte_written = fwrite(block, 1, providedByteNum, fhpu);
		if (nb_byte_written != (size_t)providedByteNum)
		{
			/* error writting dump file */
			printf("Error %d writing file %s\n", errno, ETAL_DCOP_FLASH_PROGRAM_FILENAME);
			fclose(fhpu);
			fhpu = NULL;
		}
		else if (nb_byte_written == remainingByteNum)
		{
			/* DCOP dump successful */
			fclose(fhpu);
			fhpu = NULL;
		}
	}
}
#else
// not supported for now
/***************************
 *
 * etalDcopFlash_PutImageCb
 *
 **************************/
void etalDcopFlash_PutImageCb (void *pvContext, tU8* block, tU32 providedByteNum, tU32 remainingByteNum)
{
	return;
}

#endif

/***************************
 *
 * etalDcopFlash_initialize_and_FlashDcop
 *
 **************************/
static int etalDcopFlash_initialize_and_FlashDcop(tBool doFlashDump, tBool doFlashProgram, tBool doDownloadMemory, tBool addXloaderHeader)
{
	int ret;
	EtalHardwareAttr init_params;

	/*
	 * Initialize ETAL
	 */

	memset(&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = userNotificationHandler;
	init_params.m_CountryVariant = ETAL_COUNTRY_VARIANT_EU;
	if ((doFlashDump != FALSE) || (doFlashProgram != FALSE) || (doDownloadMemory != FALSE))
	{
		init_params.m_DCOPAttr.m_doFlashDump = doFlashDump;
		init_params.m_DCOPAttr.m_doFlashProgram = doFlashProgram;
		init_params.m_DCOPAttr.m_doDownload = doDownloadMemory;
#ifndef CONFIG_ETAL_DCOP_FLASH_FIRMWARE_DL_FILE_MODE
		if (addXloaderHeader == FALSE)
		{
			init_params.m_DCOPAttr.m_cbGetImage = etalDcopFlash_GetImageCb;
		}
		else
		{
			/* used only for DCOP STA660 */
			init_params.m_DCOPAttr.m_cbGetImage = NULL;
		}
		init_params.m_DCOPAttr.m_cbPutImage = etalDcopFlash_PutImageCb;
#else
		init_params.m_DCOPAttr.m_pvGetImageContext = ETALDcopFlash_filename;
#endif // #ifndef CONFIG_ETAL_DCOP_FLASH_FIRMWARE_DL_FILE_MODE
	}

#ifndef TUNER_MTD_MODULE	
		init_params.m_tunerAttr[1].m_isDisabled = TRUE;
#endif

	printf("call: etal_initialize\n");

	/* initialize and download DCOP firmware file dcop_fw.bin */
	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_initialize (%d)\n", ret);
		return 1;
	}

	return 0;
}

/***************************
 *
 * etalDcopFlash_ShowUsage
 *
 **************************/
tVoid etalDcopFlash_ShowUsage(char *prgName)
{
#ifndef	CONFIG_DCOP_FLASH_PROGRAM_IN_ROM
	printf("Usage: %s [-options] [phase1_filename] [dcop_firmware_filename]\n", prgName);
	printf("use %s file for phase1 file and dcop_fw.bin file for DCOP STA680 \nfirmware download.\n", ETAL_DCOP_FLASH_BOOTSTRAP_FILENAME);
	printf("phase1 and dcop firmware filenames can optionally be defined with \ncommand line.\n");
#else
	printf("Usage: %s [-options]\n", prgName);
#endif
#if 0
	printf("\t-d  dump DCOP STA660 serial flash in %s\n", ETAL_DCOP_FLASH_DUMP_FILENAME);
#endif
	printf("\t-h  show this help\n");
	printf("\t-m  download DCOP STA680 firmware in volatile memory\n");
	printf("\t-p  flash DCOP firmware in serial flash\n");
#if 0
	printf("\t-x  add xloader frame header to DCOP STA660 firmware\n");
#endif
}

#ifndef CONFIG_HOST_OS_TKERNEL
/***************************
 *
 * etalDcopFlash_parseParameters
 *
 **************************/
int etalDcopFlash_parseParameters(int argc, char **argv)
{
	tBool cmdFound = TRUE, bootstrapFilenameFound = FALSE, programFilenameFound = FALSE;
	int i;

	ETALDcopFlash_doFlashDump = FALSE;
	ETALDcopFlash_doFlashProgram = TRUE;
	ETALDcopFlash_doDownloadMemory = FALSE;
	ETALDcopMdrFlash_addXloaderHeader = FALSE;
	for(i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			ETALDcopFlash_doFlashDump = FALSE;
			ETALDcopFlash_doFlashProgram = FALSE;
			ETALDcopFlash_doDownloadMemory = FALSE;
			ETALDcopMdrFlash_addXloaderHeader = FALSE;
			break;
		}
	}

	for(i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			cmdFound = FALSE;
#if 0
			if ((argv[i][1] == 'd') || (argv[i][2] == 'd'))
			{
				ETALDcopFlash_doFlashDump = TRUE;
				cmdFound = TRUE;
			}
#endif
			if ((argv[i][1] == 'p') || (argv[i][2] == 'p'))
			{
				if (ETALDcopFlash_doDownloadMemory == TRUE)
				{
					etalDcopFlash_ShowUsage(argv[0]);
					return 1;
				}
				ETALDcopFlash_doFlashProgram = TRUE;
				cmdFound = TRUE;
			}
			if ((argv[i][1] == 'm') || (argv[i][2] == 'm'))
			{
				if (ETALDcopFlash_doFlashProgram == TRUE)
				{
					etalDcopFlash_ShowUsage(argv[0]);
					return 1;
				}
				ETALDcopFlash_doDownloadMemory = TRUE;
				cmdFound = TRUE;
			}
#if 0
			if ((argv[i][1] == 'x') || (argv[i][2] == 'x'))
			{
				ETALDcopMdrFlash_addXloaderHeader = TRUE;
				cmdFound = TRUE;
			}
#endif
			if (((argv[i][1] == 'h') || (argv[i][2] == 'h')) || (cmdFound == FALSE))
			{
				etalDcopFlash_ShowUsage(argv[0]);
				cmdFound = TRUE;
				return 1;
			}
		}
		else
		{
#ifndef	CONFIG_DCOP_FLASH_PROGRAM_IN_ROM

			if (bootstrapFilenameFound == FALSE)
			{
				strncpy(ETALDcopFlash_bootstrap_filename, argv[i], ETAL_DCOP_BOOT_FILENAME_MAX_SIZE);
				bootstrapFilenameFound = TRUE;
			}
			else if (programFilenameFound == FALSE)
			{
				strncpy(ETALDcopFlash_program_filename, argv[i], ETAL_DCOP_BOOT_FILENAME_MAX_SIZE);
				programFilenameFound = TRUE;
			}
			else
#endif
			{
				etalDcopFlash_ShowUsage(argv[0]);
				return 1;
			}
		}
	}

	return 0;
}
#endif // CONFIG_HOST_OS_TKERNEL

/***************************
 *
 * main
 *
 **************************/
/*
 * Returns:
 * 1 error(s)
 * 0 success
 */
#ifdef CONFIG_HOST_OS_TKERNEL
int etalDcopHdrFlash_EntryPoint(etalDcopFlashOptionTy *pI_etalDcopFlashOption)
{
	ETALDcopFlash_doFlashDump = pI_etalDcopFlashOption->flashDump;
	ETALDcopFlash_doFlashProgram = pI_etalDcopFlashOption->flashProgram;
	ETALDcopFlash_doDownloadMemory = pI_etalDcopFlashOption->downloadMemory;
#ifndef	CONFIG_DCOP_FLASH_PROGRAM_IN_ROM
	strncpy(ETALDcopFlash_program_filename, pI_etalDcopFlashOption->program_file, ETAL_DCOP_BOOT_FILENAME_MAX_SIZE);
	strncpy(ETALDcopFlash_bootstrap_filename, pI_etalDcopFlashOption->bootstrap_file, ETAL_DCOP_BOOT_FILENAME_MAX_SIZE);
#endif // CONFIG_DCOP_FLASH_PROGRAM_IN_ROM
#else
int main(int argc, char **argv)
{
	if (etalDcopFlash_parseParameters(argc, argv))
	{
		return 1;
	}
#endif /* !CONFIG_HOST_OS_TKERNEL */

	if (etalDcopFlash_initialize_and_FlashDcop(ETALDcopFlash_doFlashDump, ETALDcopFlash_doFlashProgram, ETALDcopFlash_doDownloadMemory, ETALDcopMdrFlash_addXloaderHeader))
	{
		printf("Error initializing the DCOP\n");
		return 1;
	}


	// now all is fine, deinit
	
	/*
	 * Final cleanup
	 */

	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_deinitialize\n");
	}
	
	return 0;
}

#ifdef CONFIG_HOST_OS_TKERNEL
// vI_downloadInMemory = FALSE : indicate this is to flash the DCOP
// vI_downloadInMemory = TRUE : indicate this is to load dynamically the DCOP RAM
int etalDcopHdrFlash_Start(tBool vI_downloadInMemory)
{
	int vl_ret = 0;

	etalDcopFlashOptionTy vl_etalDcopFlashOption;

	// Let's try with hard-coded parameters
	vl_etalDcopFlashOption.flashDump = FALSE;
	if (FALSE == vI_downloadInMemory)
	{
		vl_etalDcopFlashOption.flashProgram = TRUE;
		vl_etalDcopFlashOption.downloadMemory = FALSE;
	}
	else
	{
		vl_etalDcopFlashOption.flashProgram = FALSE;
		vl_etalDcopFlashOption.downloadMemory = TRUE;
	}
	
#ifndef CONFIG_DCOP_FLASH_PROGRAM_IN_ROM
	strncpy(vl_etalDcopFlashOption.bootstrap_file, ETAL_DCOP_FLASH_BOOTSTRAP_FILENAME, ETAL_DCOP_BOOT_FILENAME_MAX_SIZE);
	strncpy(vl_etalDcopFlashOption.program_file, ETAL_DCOP_FLASH_PROGRAM_FILENAME, ETAL_DCOP_BOOT_FILENAME_MAX_SIZE);
#endif // CONFIG_DCOP_FLASH_PROGRAM_IN_ROM
	vl_ret = etalDcopHdrFlash_EntryPoint(&vl_etalDcopFlashOption);

	return vl_ret;
}
#endif

#endif // #ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

