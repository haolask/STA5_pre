/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
/*  **********************
    ***** DISCLAIMER *****
    **********************
    "Long File Names" is an intelluectual property of
    "Microsoft Corporation". In this module LFN can be used exclusively when
    propedeutic for flashing WinCE6.0 artifacts.
    LFN is used  for copying Storage Based Image auxialiary files.
 */

#include "string.h"
#include "diskio.h"
#include "sta_common.h"
#include "sta_sdmmc.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/*-----------------------------------------------------------------------*/

#define ATA		0
#define MMC		1
#define USB		2

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{

    switch (drv)
    {
    	case 0:
            dfs_read_sector(0, buff, sector, count);
            break;

    	case 1:
    		memcpy(buff, (uint8_t *)(RAM_DISK_START_ADDRESS + sector * 512), count * 512);
    		break;

    	case 2:
    	case 3:
    		return RES_PARERR;
    		break;
    	default:
    		return RES_PARERR;
    }

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/* The FatFs module will issue multiple sector transfer request
/  (count > 1) to the disk I/O layer. The disk function should process
/  the multiple sector transfer properly Do. not translate it into
/  multiple single sector transfers to the media, or the data read/write
/  performance may be drasticaly decreased. */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{

    switch (drv)
    {
    	case 0:
			dfs_write_sector(0, (uint8_t *)buff, sector, count);
    		break;

    	case 1:
    		memcpy((uint8_t *)(RAM_DISK_START_ADDRESS + sector * 512), buff, count * 512);
    		break;

    	case 2:
    	case 3:
    		return RES_PARERR;
    		break;
    	default:
    		return RES_PARERR;
    }

	return RES_OK;
}
#endif /* _READONLY */


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{

	switch (ctrl)
	{
	case CTRL_SYNC:
 		// Make sure that the disk drive has finished pending write process.
 		// When the disk I/O module has a write back cache, flush the dirty sector immediately. This command is not required in read-only configuration.
		return RES_OK;
		break;

	case GET_SECTOR_SIZE:
		// Returns sector size of the drive into the WORD variable pointed
		// by Buffer. This command is not required in single sector size
		// configuration, _MAX_SS is 512.
		*(WORD*)buff=(WORD)512;
		return RES_OK;
		break;

	case GET_SECTOR_COUNT:
		// Returns total sectors on the drive into the DWORD variable pointed
		// by Buffer. This command is used by only f_mkfs function to determine
		// the volume size to be created.


	    switch (drv)
	    {
	    	case 0:
	    		*(DWORD *)buff= 0x80000;
	    		break;

	    	case 1:
	    		*(DWORD *)buff= 0x20000;
	    		break;

	    	case 2:
	    		return RES_PARERR;
	    		break;

	    	case 3:
	    		return RES_PARERR;

	    	default:
	    		return RES_PARERR;
		}


//		UM_SendDebugMsg (1,"GET_SECTOR_COUNT %d\n",*(DWORD *)buff);
		return RES_OK;
		break;

	case GET_BLOCK_SIZE:
		// Returns erase block size of the flash memory in unit of sector into
		// the DWORD variable pointed by Buffer. This command is used by only
		// f_mkfs function and it attempts to align data area to the erase block
		// boundary. The allowable value is 1 to 32768 in power of 2. Return 1 if
		// the erase block size is unknown or disk devices
		*(DWORD*)buff=(DWORD)1;
		return RES_OK;

	default:
		break;
	}

	return RES_PARERR;
}


/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

DWORD get_fattime (void)
{

	return 0;

	/* Pack date and time into a DWORD variable */
/*	return 	  ((DWORD)(tm.wYear - 1980) << 25)
			| ((DWORD)tm.wMonth << 21)
			| ((DWORD)tm.wDay << 16)
			| (WORD)(tm.wHour << 11)
			| (WORD)(tm.wMinute << 5)
			| (WORD)(tm.wSecond >> 1);*/
}
