//!
//!  \file    hdr_boot.cpp
//!  \brief   <i><b> HDR flash protocol</b></i>
//!  \details HD protocol used by ETAL and by MDR_Protocol
//!  \author  Alberto Saviotti
//!

#include "target_config.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO) && !(defined (CONFIG_APP_TUNERDRIVER_LIBRARY))

#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
#include <stdio.h>
#include <string.h>
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
#include <stdlib.h>

//#warning "before"
#include "types.h"

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
#include "common_helpers.h"

#include "ftdi_mngm.h"
#include "interlayer_protocol.h"
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
#include "osal.h"
#include "etalinternal.h"
#include "hdradio_trace.h"
#endif

#include "HDRADIO_Protocol.h"
#include "hdr_lld.h"
#include "hdr_boot.h"

#ifdef __cplusplus
extern "C" {
#endif

    ///
    // DEFINES
    ///

#define SEGSIZE  242             // 1497   //1024     read 242 bytes at a time
#define HDRFLASHSIZE 1048586     // HDR FLASH size
#define HDR_PHASE_MAX_BYTE_TO_SEND  2044
#define HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND  (HDR_PHASE_MAX_BYTE_TO_SEND - LM_OVERHEAD)
#define HDR_READ_RESP_MAX_LEN       (CP_OVERHEAD + 8)

#if defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
#define HDR_Write(/*tVoid * */_devicehandler_, /*tU8 * */_buf_, /*tU32*/ _len_, /*tBool*/ _useI2c_, /*tU8*/ _spiCS_Phase_, /*tBool*/ _intByteDelayOn_) \
					((HDRADIO_Write_Raw_Data((_buf_), (_len_)) == OSAL_OK)?RES_OK:RES_ERROR_GENERIC)

#define HDR_Read(/*tVoid * */_devicehandler_, /*tU8 * */_buf_, /*tU32*/ _len_, /*tU8*/ _i2cHdAddr_, /*tBool*/ _useI2c_, /*tU8*/ _spiCS_Phase_) \
					HDRADIO_Read_Raw_Data(&(_buf_), (_len_))
#endif // #if defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)

///
// MACROs
///

///
// Typedefs
///

///
// Local variables
///

#if 0
static tBool    HDR_fileLenFieldSet;
static tBool    HDR_entryPointFieldSet;
static tU8      HDR_flashMaxSizeDataBfr[HD_SPI_PROGRAM_BUFFER_SIZE];
#endif


///
// Local functions declarations
///
static tS32 HDRADIO_sendCommand(tyHDRADIOInstanceID instId, tU8 *cmd, tU32 clen, tU8 *resp, tSInt *rlen);
static tSInt HDR_SendPhase1 (tVoid *devicehandler, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr);
static tSInt HDR_SendPhase2 (tVoid *devicehandler, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr);

static tSInt HDR_ExecutePhase3 (tVoid *devicehandler, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr);

static tSInt HDR_Phase2ViaCmd (tVoid *devicehandler, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr);
#if 0
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
static tSInt WriteBinSpitoSTA680Flash (tVoid *devicehandler, tUChar *pagein,  tULong fileSize);
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
static tSInt hdrWriteFlash0x84 (tVoid *devicehandler, tUChar * pagein, tULong fileSize, HDR_instanceIdEnumTy instId);
static tSInt hdrSta680PackCommand (tVoid *devicehandler, tUChar * hdBufferOut, tUChar * hdBufferIn, tULong fileSize, tU8 lmCount, int segmentNb, tBool useI2c);
static tSInt hdrSta680SendPackData (tVoid *devicehandler, tUChar * hdBufferIn, tULong fileSize, tBool useI2c);

static tSInt hdrFirmwareUpdate (tVoid *devicehandler, int hdrInstance, HDR_txRxDataTy *txRxData, tU32 numBytes, tU8 *dataPtr, tBool useI2c, tU8 hdI2cAdr, HDR_spiBusTy spiPhase);
static tSInt sendHdrWriteToFlash0x84 (tVoid *devicehandler, int hdrInstance, HDR_txRxDataTy *txRxData, tU32 numBytes, tU8 *dataPtr, tBool useI2c, tU8 hdI2cAdr, HDR_spiBusTy spiPhase);
static tSInt getHdrStatusOfProgramming0x84 (tVoid *devicehandler, int hdrInstance, HDR_txRxDataTy *txRxData, tU32 numBytes, tU8 *dataPtr, tBool useI2c, tU8 hdI2cAdr, HDR_spiBusTy spiPhase);
static tSInt hdrGetSTA680Answer (tVoid *devicehandler, int hdrInstance, tUChar * hdrBuff, tULong numBytes, tBool useI2c, HDR_spiBusTy spiPhase);
static tSInt hdrPack_SendSTA680Flash (tVoid *devicehandler, int hdrInstance, tUChar * pagein, tULong fileSize, tBool useI2c, HDR_spiBusTy spiPhase);
static tSInt hdr_PackSendWrite (tVoid *devicehandler, int hdrInstance, tUChar * pagein, tULong fileSize, tSInt segNumber, tBool useI2c, HDR_spiBusTy spiPhase);

double PC1Freq = 0.0;
int64 Counter1Start = 0;

void Start1Counter ()
{
    LARGE_INTEGER li;
    if (QueryPerformanceFrequency (&li))
    {
        PC1Freq = double (li.QuadPart) / 1000.0;

        QueryPerformanceCounter (&li);
        Counter1Start = li.QuadPart;
    }
    else
    {
        //To Do
        // QueryPerformanceFrequency failed!;
    }
}

double Get1Counter ()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter (&li);
    return double (li.QuadPart - Counter1Start) / PC1Freq;
}
#endif // 0

///
// Global functions
///

tU16 HDR_ExecuteFlashOperations (tVoid *devicehandler, HDR_flashModeEnumTy mode, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr)
{
    tU16 resultAction = RES_ERROR;
    tSInt res = HD_FLASH_STATUS_ERROR_GENERIC;

    if (HDR_FLASH_OP_MODE_IS_PHASE1 == mode)
    {
        // Send Phase1 sync and download Phase1 file
        res = HDR_SendPhase1 (devicehandler, paramPtr, numBytes, dataPtr);
    }
    else if (HDR_FLASH_OP_MODE_IS_PROGRAM == mode)
    {
        // Call sync2 and download Program Flash function (passing only file name to use)
        res = HDR_SendPhase2 (devicehandler, paramPtr, numBytes, dataPtr);
    }
    else if (HDR_FLASH_OP_MODE_IS_PH2CMD == mode)
    {
        // Call programming Phase2 file via command 0x84
        res = HDR_Phase2ViaCmd (devicehandler, paramPtr, numBytes, dataPtr);
    }

    if (res == HD_FLASH_STATUS_OK)
    {
        resultAction = RES_OK;
    }

    return resultAction;
}

static tSInt HDR_SendPhase1 (tVoid *devicehandler, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr)
{
    tSInt res = HD_FLASH_STATUS_OK;

    tUChar *pagein = NULL;
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    tULong fileSize;
    FILE *pfin = NULL;
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    tU8 hdBuffOut[100];
    int ii;
    tU32 lenBuf = 4;
    HDR_accessModeEnumTy access_mode;
    EtalCbGetImage     cbGetImage;
    tVoid             *pvGetImageContext;
    tU32 returnedByteNum, remainingByteNum;
    tSInt retcgi;

    HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HD Radio send phase1");
    access_mode = (HDR_accessModeEnumTy)paramPtr[3];

    // Open the phase1 and program files before DCOP reset to avoid unexpected delay in flashing sequence.
    // Because the delay between the bytes must not exceed 100 ms.
    if (access_mode == HDR_ACCESS_FILE_MODE)
    {
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
        // Open received file for binary read: the flasher file must already have the header
        // allocated at least as zeroes (it will be calculated again here)
        if ((pfin = OSAL_fopen ((const char*)dataPtr, "rb")) == NULL)
        {
            // Error : File is not accessible nor existing
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 file open fail");
            res = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }
#else
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 access file mode not supported");
        res = RES_ERROR_GENERIC;
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    }
    else if (access_mode == HDR_ACCESS_DATA_MODE)
    {
        if (res == HD_FLASH_STATUS_OK)
        {
            /* get read function callback and context */
			/* Lint deviation - code is due to the protocol itself, noway to change it */
            cbGetImage = (EtalCbGetImage)((dataPtr[0] << 24) | (dataPtr[1] << 16) | (dataPtr[2] << 8) | dataPtr[3]);	//lint !e701 - MISRA 10.5 - code is due to the protocol itself, noway to change it
            pvGetImageContext = (tPVoid)((dataPtr[4] << 24) | (dataPtr[5] << 16) | (dataPtr[6] << 8) | dataPtr[7]);		//lint !e701 - MISRA 10.5 - code is due to the protocol itself, noway to change it
            if (cbGetImage == NULL)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 cbGetImage invalid %p", cbGetImage);
                res = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
            }
        }
        if (res == HD_FLASH_STATUS_OK)
        {
            // Allocation of a dynamic array
            pagein = (tUChar *)OSAL_pvMemoryAllocate (HDR_PHASE_MAX_BYTE_TO_SEND);
            if (pagein == NULL)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 allocation of size %d fail", HDR_PHASE_MAX_BYTE_TO_SEND);
                res = HD_FLASH_STATUS_ERROR_MEMORY;
            }
        }
        if (res == HD_FLASH_STATUS_OK)
        {
            // Call a first time the cbGetImage to open the file before the DCOP Reset
            // because the delay between the bytes must not exceed 100 ms during the DCOP flashing.
            // fopen can take sometimes 180 ms which is too much.
            retcgi = cbGetImage(pvGetImageContext, 0, pagein, &returnedByteNum, &remainingByteNum, TRUE);
            retcgi = cbGetImage(pvGetImageContext, 0, pagein, &returnedByteNum, &remainingByteNum, FALSE);
        }
    }

#ifdef CONFIG_COMM_DRIVER_EMBEDDED
    if (res == HD_FLASH_STATUS_OK)
    {
        // Reset the DCOP
        if (HDRADIO_reset() == OSAL_OK)
        {
            // SYNC sequence need to be send within 10 ms after the reset
            (void)OSAL_s32ThreadWait(3); // working in range [1; 15+]
        }
        else
        {
            res = HD_FLASH_STATUS_ERROR_GENERIC;
        }
    }
#endif // #ifdef CONFIG_COMM_DRIVER_EMBEDDED

    if (res == HD_FLASH_STATUS_OK)
    {
        // SYNC PHASE 1
        // Start ....

        // Sync msg = 0x01 0x01 0x01 0x01

        hdBuffOut[1] = (tU8)0x01;

        // send 64 times the Sync msg (minimum requested by Ibiquity is totally 64 data bytes)
#ifdef	ETAL_BSP_DEBUG_SPI_RW_WORD
		hdBuffOut[2] = (tU8)0x01;
		hdBuffOut[3] = (tU8)0x01;
		hdBuffOut[4] = (tU8)0x01;

		for (ii = 0; ii < 128 / 4; ii++)
		{
			if (HDRADIO_Write_Raw_Data(&hdBuffOut[1], 4) != OSAL_OK)
           	{
#else  // ETAL_BSP_DEBUG_SPI_RW_WORD

#ifdef CONFIG_COMM_HDRADIO_SPI
        for (ii = 0; ii < 128; ii++)
#else
        for (ii = 0; ii < 82; ii++)
#endif
        {
            if (HDRADIO_Write_Raw_Data(&hdBuffOut[1], 1) != OSAL_OK)
            {
#endif // ETAL_BSP_DEBUG_SPI_RW_WORD

                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 SYNC ko");
#if !defined (CONFIG_HOST_OS_TKERNEL)
				res = RES_ERROR_GENERIC;
#endif
            }


            /* During the SYNC sequence only it is highly recommended to place small inter */
            /* character delay of 2 to 5 ms to ensure robust synchronization */
            (void)OSAL_s32ThreadWait(3);
        }
    }

    if (res == HD_FLASH_STATUS_OK)
    {
        // SYNC executed

        // Start speed sequence ...
        hdBuffOut[66] = (tU8)0x7A;
        hdBuffOut[67] = (tU8)0x7A;
        hdBuffOut[68] = (tU8)0x7A;
        hdBuffOut[69] = (tU8)0x63;

        hdBuffOut[71] = (tU8)0x00;
        hdBuffOut[72] = (tU8)0x00;
        hdBuffOut[73] = (tU8)0x00;
        hdBuffOut[74] = (tU8)0x00;

        if (HDRADIO_Write_Raw_Data(&hdBuffOut[66], lenBuf) != OSAL_OK)
        {
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 START send ko");
            res = RES_ERROR_GENERIC;
        }
    }

    if (res == HD_FLASH_STATUS_OK)
    {
        if (HDRADIO_Write_Raw_Data(&hdBuffOut[71], lenBuf) != OSAL_OK)
        {
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 SPEED send ko");
            res = RES_ERROR_GENERIC;
        }
        // Speed Sequence executed
    }

    if (res == HD_FLASH_STATUS_OK)
    {
        if (access_mode == HDR_ACCESS_FILE_MODE)
        {
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
            if (pfin != NULL)
            {
                // Get the size of file in bytes
                if (res == HD_FLASH_STATUS_OK)
                {
                    if (OSAL_fseek (pfin, 0, SEEK_END) != 0)
                    {
                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 file seek fail");
                        OSAL_fclose (pfin);
                        res = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
                    }
                }
                if (res == HD_FLASH_STATUS_OK)
                {
                    fileSize = OSAL_ftell (pfin);
                    if (OSAL_fseek (pfin, 0, SEEK_SET) != 0)
                    {
                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 file seek fail");
                        OSAL_fclose (pfin);
                        res = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
                    }
                }

                if (res == HD_FLASH_STATUS_OK)
                {
                    // Allocation of a dynamic array
                    pagein = (tUChar *)OSAL_pvMemoryAllocate (HDR_PHASE_MAX_BYTE_TO_SEND);
                    if (NULL == pagein)
                    {
                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 allocation of size %d fail", fileSize);
                        res = HD_FLASH_STATUS_ERROR_MEMORY;
                    }
                }
                if (HD_FLASH_STATUS_OK == res)
                {
                    // Start Write Phase1
                    do
                    {
                        if ((retcgi = OSAL_fread(pagein, 1, HDR_PHASE_MAX_BYTE_TO_SEND, pfin)) != HDR_PHASE_MAX_BYTE_TO_SEND)
                        {
                            if (OSAL_ferror(pfin) != 0)
                            {
                                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 file read fail");
                                res = HD_FLASH_STATUS_ERROR_READ;
                            }
                            if ((OSAL_feof(pfin) != 0) || (OSAL_ferror(pfin) != 0))
                            {
                                OSAL_clearerr(pfin);
                            }
                        }
                        if (retcgi > 0)
                        {
                            returnedByteNum = (tU32)retcgi;
                        }
                        else
                        {
                            returnedByteNum = (tU32)0;
                        }
                        HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "retcgi %d, returnedByteNum %d, fileSize %d\n", retcgi, returnedByteNum, fileSize);
                        if ((res == HD_FLASH_STATUS_OK) && (returnedByteNum > 0))
                        {
                            if (HDRADIO_Write_Raw_Data(&pagein[0], returnedByteNum) != OSAL_OK)
                            {
                                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 write block size %d fail", returnedByteNum);
                                res = RES_ERROR_GENERIC;
                            }
                        }
                    } while((retcgi > 0) && (returnedByteNum == HDR_PHASE_MAX_BYTE_TO_SEND) && (res == HD_FLASH_STATUS_OK));
                }
                // Close file
                OSAL_fclose (pfin);
                pfin = NULL;
            }

            // if (res == HD_FLASH_STATUS_OK)
            // {
                // Start Write Phase1 to Flash ...
                // res = WriteBinSpitoSTA680Flash (devicehandler, &pagein[0], fileSize);
            // }
#else
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 access file mode not supported");
            res = RES_ERROR_GENERIC;
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
        }
        else if (access_mode == HDR_ACCESS_DATA_MODE)
        {
            if (res == HD_FLASH_STATUS_OK)
            {
                if (numBytes < 8)
                {
                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 payload length invalid %d", numBytes);
                    res = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
                }
            }
            if (res == HD_FLASH_STATUS_OK)
            {
                // Start Write Phase1
                do
                {
                    returnedByteNum = 0;
                    retcgi = cbGetImage(pvGetImageContext, HDR_PHASE_MAX_BYTE_TO_SEND, pagein, &returnedByteNum, &remainingByteNum, TRUE); //lint !e644 - MISRA 9.1 - cbGetImage correctly initialize from message
                    HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "retcgi %d, returnedByteNum %d, remainingByteNum %d\n", retcgi, returnedByteNum, remainingByteNum);
                    if ((retcgi == 0) && (returnedByteNum > 0))
                    {
                        if (HDRADIO_Write_Raw_Data(&pagein[0], returnedByteNum) != OSAL_OK)
                        {
                            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH1 write block size %d fail", returnedByteNum);
                            res = RES_ERROR_GENERIC;
                        }
                    }
                } while((retcgi == 0) && (returnedByteNum == HDR_PHASE_MAX_BYTE_TO_SEND) && (res == HD_FLASH_STATUS_OK));
            }
        }
    }

#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    if (pfin != NULL)
    {
        // Close file
        OSAL_fclose (pfin);
        pfin = NULL;
    }
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)

    // Free allocated resources
    if (NULL != pagein)
    {
        OSAL_vMemoryFree ((tPVoid)pagein);
    }
    return res;
}

static tSInt HDR_SendPhase2 (tVoid *devicehandler, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr)
{
    tSInt res = HD_FLASH_STATUS_OK;

    tUChar *pagein = NULL;
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    tULong fileSize;
    FILE *pfin;
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    tU8 hdBuffOut[100];
    tU8 hdBuffIn[HDR_READ_RESP_MAX_LEN], *p_hdBuffIn = hdBuffIn;
    int ii;
    tU32 lenBuf = 4;
    tBool first_memblock;
    HDR_accessModeEnumTy access_mode;
    EtalCbGetImage     cbGetImage;
    tVoid             *pvGetImageContext;
    tU32 returnedByteNum, remainingByteNum;
    tSInt retcgi, offset;
    HDR_targetMemEnumTy trg_mem;
    tS32 ret_read;
    EtalDeviceDesc deviceDescription;
	tSInt ret = HD_FLASH_STATUS_OK;

#ifdef CONFIG_COMM_HDRADIO_SPI
    tyHDRADIODeviceConfiguration HDDeviceConfig;

    ETAL_getDeviceConfig_HDRADIO(&HDDeviceConfig);

    /* change to SPI11 mode if needed */
    if (HDDeviceConfig.communicationBusType == BusSPI)
    {
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_SPI_CPHA1_CPOL1
        HDDeviceConfig.communicationBus.spi.mode = SPI_CPHA1_CPOL1;
#else
        HDDeviceConfig.communicationBus.spi.mode = SPI_CPHA0_CPOL0;	/* this spi mode change should never happen because already set in ETAL_initCommunication_HDRADIO */
#endif /* CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_SPI_CPHA1_CPOL1 */
        HDDeviceConfig.communicationBus.spi.speed = HDRADIO_ACCORDO2_SPI_SPEED_LOAD_PHASE1;
        (void)HDRADIO_reconfiguration(&HDDeviceConfig);
    }
#endif

    HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HD Radio send phase2");
    // Get the target memory
    trg_mem = (HDR_targetMemEnumTy)paramPtr[2];
    // Get the access mode
    access_mode = (HDR_accessModeEnumTy)paramPtr[3];
    // Get the I2C / SPI mode
    ETAL_getDeviceDescription_HDRADIO(&deviceDescription);

    // start PHASE 2, SYNC

    hdBuffOut[1] = (tU8)0x01;

    // send 64 times the Sync msg (minimum requested by Ibiquity is totally 64 data bytes)
#ifdef CONFIG_HOST_OS_TKERNEL
	for (ii = 0; ii < 82; ii++)
#else
	for (ii = 0; ii < 66; ii++)
#endif
    {
        if (HDRADIO_Write_Raw_Data(&hdBuffOut[1], 1) != OSAL_OK)
        {
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SYNC ko");
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }
        /* During the SYNC sequence only it is highly recommended to place small inter */
        /* character delay of 2 to 5 ms to ensure robust synchronization */
        (void)OSAL_s32ThreadWait(3);
    }
    // SYNC executed

    // Start speed sequence ...
    hdBuffOut[66] = (tU8)0x7A;
    hdBuffOut[67] = (tU8)0x7A;
    hdBuffOut[68] = (tU8)0x7A;
    hdBuffOut[69] = (tU8)0x63;

    hdBuffOut[71] = (tU8)0x00;
    hdBuffOut[72] = (tU8)0x00;
    hdBuffOut[73] = (tU8)0x00;
    hdBuffOut[74] = (tU8)0x00;

    hdBuffOut[76] = (tU8)0x71;
    hdBuffOut[77] = (tU8)0x71;
    hdBuffOut[78] = (tU8)0x71;
    if (trg_mem == HDR_TARGET_SPI2FLASH)
    {
        hdBuffOut[79] = (tU8)0x6D;
    }
    else if(trg_mem == HDR_TARGET_SPI2MEM)
    {
        hdBuffOut[79] = (tU8)0x79;
    }
    else
    {
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 invalid target memory %d", trg_mem);
        res = RES_ERROR_GENERIC;
        ret = res;
        goto exit;
    }

    //Sleep (2)
    if (HDRADIO_Write_Raw_Data(&hdBuffOut[76], lenBuf) != OSAL_OK)
    {
        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 START send ko");
        res = RES_ERROR_GENERIC;
        ret = res;
        goto exit;
    }

    (void)OSAL_s32ThreadWait(2);

    hdBuffIn[0] = hdBuffIn[1] = hdBuffIn[2] = hdBuffIn[3] = (tU8)0;

    if (true == (deviceDescription.m_busType == ETAL_BusI2C))
    {
        ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 2);
        if ((ret_read != OSAL_OK) || ((tU8)0x01 != hdBuffIn[0]) || ((tU8)0x62 != hdBuffIn[1]))
        {
            // Wrong acknowledge value
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SYNC ACK i2c ko");
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }
        else
        {
            if (HDRADIO_Write_Raw_Data(&hdBuffOut[71], lenBuf) != OSAL_OK)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SPEED send ko");
                res = RES_ERROR_GENERIC;
                ret = res;
                goto exit;
            }
            res = RES_OK;
            // SYNC executed, ack ok \n";
        }
    }
    else
    {
        if (HDRADIO_Read_Raw_Data(p_hdBuffIn, 4) != OSAL_OK)
        {
            ret = RES_ERROR_GENERIC;
            goto exit;
        }
        if ((tU8)0x62 != hdBuffIn[0])
        {
            // Wrong acknowledge value
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SYNC ACK spi ko");
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }
        else
        {
            res = RES_OK;
            // SYNC executed, ack ok \n";
        }
    }

#ifdef CONFIG_COMM_HDRADIO_SPI
    if (res == RES_OK)
    {
        /* change SPI clock speed for firmware load */
        if ((HDDeviceConfig.communicationBusType == BusSPI) &&
            (((trg_mem == HDR_TARGET_SPI2FLASH) && (HDDeviceConfig.communicationBus.spi.speed != HDRADIO_ACCORDO2_SPI_SPEED_FLASH)) ||
            ((trg_mem == HDR_TARGET_SPI2MEM) && (HDDeviceConfig.communicationBus.spi.speed != HDRADIO_ACCORDO2_SPI_SPEED_LOAD_SDRAM)))
            )
        {
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_SPI_CPHA1_CPOL1
            HDDeviceConfig.communicationBus.spi.mode = SPI_CPHA1_CPOL1;
#else
            HDDeviceConfig.communicationBus.spi.mode = SPI_CPHA0_CPOL0;
#endif /* CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_SPI_CPHA1_CPOL1 */
            if (trg_mem == HDR_TARGET_SPI2FLASH)
            {
                HDDeviceConfig.communicationBus.spi.speed = HDRADIO_ACCORDO2_SPI_SPEED_FLASH;
            }
            else if(trg_mem == HDR_TARGET_SPI2MEM)
            {
                HDDeviceConfig.communicationBus.spi.speed = HDRADIO_ACCORDO2_SPI_SPEED_LOAD_SDRAM;
            }
            (void)HDRADIO_reconfiguration(&HDDeviceConfig);
        }
    }
#endif

    if (access_mode == HDR_ACCESS_FILE_MODE)
    {
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
        // Open received file for binary read: the flasher file must already have the header
        // allocated at least as zeroes (it will be calculated again here)
        if ((pfin = OSAL_fopen ((const char*)dataPtr, "rb")) == NULL)
        {
            // Error : File is not accessible nor existing
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 file open fail");
            ret = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
            goto exit;
        }
        else
        {
            // Get the size of file in bytes
            if (OSAL_fseek (pfin, 0, SEEK_END) != 0)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 file seek fail");
                OSAL_fclose (pfin);
                ret = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
                goto exit;
            }
            fileSize = OSAL_ftell (pfin);
            if (OSAL_fseek (pfin, 0, SEEK_SET) != 0)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 file seek fail");
                OSAL_fclose (pfin);
                ret = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
                goto exit;
            }

            if (trg_mem == HDR_TARGET_SPI2FLASH)
            {
                // Send SIZE message
                if (fileSize == 1048576)
                {
                    hdBuffOut[74] = (tU8)1;
                }
                else if (fileSize == 2097152)
                {
                    hdBuffOut[74] = (tU8)2;
                }
                else if (fileSize == 4194304)
                {
                    hdBuffOut[74] = (tU8)4;
                }
                else
                {
                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 invalid file SIZE %d, expect 1 MB, 2 MB or 4 MB", fileSize);
                    res = RES_ERROR_GENERIC;
                }

                if (true == (deviceDescription.m_busType == ETAL_BusI2C))
                {
                    if (HDRADIO_Write_Raw_Data(&hdBuffOut[71], lenBuf) != OSAL_OK)
                    {
                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SIZE send ko");
                        res = RES_ERROR_GENERIC;
                    }

                    hdBuffIn[0] = hdBuffIn[1] = hdBuffIn[2] = hdBuffIn[3] = (tU8)0;
                    ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 2);
                    offset = 0;

                    if ((ret_read != OSAL_OK) || ((tU8)0x01 != hdBuffIn[0]) || ((tU8)0x70 != hdBuffIn[1]))
                    {
                        // Wrong acknowledge value
                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SIZE ACK ko");
                        res = RES_ERROR_GENERIC;
                        ret = res;
                        goto exit;
                    }
                }
                else
                {
                    if (HDRADIO_Write_Raw_Data(&hdBuffOut[71], lenBuf) != OSAL_OK)
                    {
                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SIZE send ko");
                        res = RES_ERROR_GENERIC;
                    }

                    hdBuffIn[0] = hdBuffOut[74];
                    ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 1);
                    offset = 1;

                    // SPI expected answer 0x70 will be read at transmission of first byte of PHASE 2 file
                    if ((ret_read != OSAL_OK) || ((tU8)0x70 != hdBuffIn[0]))
                    {
                        // Wrong acknowledge value
                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SIZE ACK ko");
                        res = RES_ERROR_GENERIC;
                        ret = res;
                        goto exit;
                    }
                }
            }

            if (res == HD_FLASH_STATUS_OK)
            {
                // Allocation of a dynamic array
                pagein = (tUChar *)OSAL_pvMemoryAllocate (HDR_PHASE_MAX_BYTE_TO_SEND);
                if (pagein == NULL)
                {
                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 allocation of size %d fail", HDR_PHASE_MAX_BYTE_TO_SEND);
                    res = HD_FLASH_STATUS_ERROR_MEMORY;
                }
            }
            if (res == HD_FLASH_STATUS_OK)
            {
                // Start Write Phase2
                do
                {
                    // Get firmware block
                    if ((retcgi = OSAL_fread (&pagein[0], 1, HDR_PHASE_MAX_BYTE_TO_SEND, pfin)) != HDR_PHASE_MAX_BYTE_TO_SEND)
                    {
                        if (OSAL_ferror(pfin) != 0)
                        {
                            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 file read fail");
                            res = HD_FLASH_STATUS_ERROR_READ;
                        }
                        if ((OSAL_feof(pfin) != 0) || (OSAL_ferror(pfin) != 0))
                        {
                            OSAL_clearerr(pfin);
                        }
                    }
                    if (retcgi > 0)
                    {
                        returnedByteNum = (tU32)retcgi;
                    }
                    else
                    {
                        returnedByteNum = (tU32)0;
                    }
                    HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "retcgi %d, returnedByteNum %d, fileSize %d\n", retcgi, returnedByteNum, fileSize);
                    if ((res == HD_FLASH_STATUS_OK) && (returnedByteNum > 0))
                    {
                        // Download firmware memory block
                        if (HDRADIO_Write_Raw_Data(&pagein[offset], (returnedByteNum - offset)) != OSAL_OK)
                        {
                            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 write block size %d fail", returnedByteNum);
                            res = RES_ERROR_GENERIC;
                        }
                    }
                    offset = 0;
                } while((retcgi > 0) && (returnedByteNum == HDR_PHASE_MAX_BYTE_TO_SEND) && (res == HD_FLASH_STATUS_OK));
            }
        }

        // Close file
        OSAL_fclose (pfin);

        // if (res == HD_FLASH_STATUS_OK)
        // {

            // Start Write Phase2 to Flash ...
            // res = WriteBinSpitoSTA680Flash (devicehandler, &pagein[0], fileSize);
        // }
#else
        res = RES_ERROR_GENERIC;
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    }
    else if (access_mode == HDR_ACCESS_DATA_MODE)
    {
        if (numBytes < 8)
        {
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 payload length invalid %d", numBytes);
            res = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }
        if (res == HD_FLASH_STATUS_OK)
        {
            /* get read function callback and context */
			/* Lint deviation - code is due to the protocol itself, noway to change it */
            cbGetImage = (EtalCbGetImage)((dataPtr[0] << 24) | (dataPtr[1] << 16) | (dataPtr[2] << 8) | dataPtr[3]);	//lint !e701 - MISRA 10.5 - code is due to the protocol itself, noway to change it
            pvGetImageContext = (tPVoid)((dataPtr[4] << 24) | (dataPtr[5] << 16) | (dataPtr[6] << 8) | dataPtr[7]);		//lint !e701 - MISRA 10.5 - code is due to the protocol itself, noway to change it
            if (cbGetImage == NULL)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 cbGetImage invalid %p", cbGetImage);
                res = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
            }
        }
        if (res == HD_FLASH_STATUS_OK)
        {
            // Allocation of a dynamic array
            pagein = (tUChar *)OSAL_pvMemoryAllocate (HDR_PHASE_MAX_BYTE_TO_SEND);
            if (pagein == NULL)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 allocation of size %d fail", HDR_PHASE_MAX_BYTE_TO_SEND);
                res = HD_FLASH_STATUS_ERROR_MEMORY;
            }
        }
        if (res == HD_FLASH_STATUS_OK)
        {
            // Start Write Phase2
            first_memblock = TRUE;
            do
            {
                // Get firmware block
                returnedByteNum = 0;
                retcgi = cbGetImage(pvGetImageContext, HDR_PHASE_MAX_BYTE_TO_SEND, pagein, &returnedByteNum, &remainingByteNum, FALSE);  //lint !e644 - MISRA 9.1 - cbGetImage correctly initialize from message
                HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "retcgi %d, returnedByteNum %d, remainingByteNum %d\n", retcgi, returnedByteNum, remainingByteNum);
                if ((retcgi == 0) && (returnedByteNum > 0))
                {
                    offset = 0;
                    if (first_memblock == TRUE)
                    {
                        if (trg_mem == HDR_TARGET_SPI2FLASH)
                        {
                            // Send SIZE message
                            if (remainingByteNum == 1048576)
                            {
                                hdBuffOut[74] = (tU8)1;
                            }
                            else if (remainingByteNum == 2097152)
                            {
                                hdBuffOut[74] = (tU8)2;
                            }
                            else if (remainingByteNum == 4194304)
                            {
                                hdBuffOut[74] = (tU8)4;
                            }
                            else
                            {
                                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 invalid file SIZE %d, expect 1 MB, 2 MB or 4 MB", remainingByteNum);
                                res = RES_ERROR_GENERIC;
                            }

                            if (true == (deviceDescription.m_busType == ETAL_BusI2C))
                            {
                                if (HDRADIO_Write_Raw_Data(&hdBuffOut[71], lenBuf) != OSAL_OK)
                                {
                                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SIZE send ko");
                                    res = RES_ERROR_GENERIC;
                                }

                                hdBuffIn[0] = hdBuffIn[1] = hdBuffIn[2] = hdBuffIn[3] = (tU8)0;
                                ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 2);

                                if ((ret_read != OSAL_OK) || ((tU8)0x01 != hdBuffIn[0]) || ((tU8)0x70 != hdBuffIn[1]))
                                {
                                    // Wrong acknowledge value
                                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SIZE ACK ko");
                                    res = RES_ERROR_GENERIC;
                                    ret = res;
                                    goto exit;
                                }
                            }
                            else
                            {
                                if (HDRADIO_Write_Raw_Data(&hdBuffOut[71], lenBuf) != OSAL_OK)
                                {
                                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SIZE send ko");
                                    res = RES_ERROR_GENERIC;
                                }

                                hdBuffIn[0] = hdBuffOut[74];

                                ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 1);

                                offset = 1;

                                // SPI expected answer 0x70 will be read at transmission of first byte of PHASE 2 file
                                if ((ret_read != OSAL_OK) || ((tU8)0x70 != hdBuffIn[0]))
                                {
                                    // Wrong acknowledge value
                                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 SIZE ACK ko");
                                    res = RES_ERROR_GENERIC;
                                    ret = res;
                                    goto exit;
                                }

								// align on 4 bytes border
								//
								if (HDRADIO_Write_Raw_Data(&pagein[offset], (4 - offset)) != OSAL_OK)
			                    {
			                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 write block size %d fail", returnedByteNum);
			                        res = RES_ERROR_GENERIC;
			                    }
								offset = 4;
                            }
                        }
                        first_memblock = FALSE;
                    }

                    // Download firmware memory block
                    if (HDRADIO_Write_Raw_Data(&pagein[offset], (returnedByteNum - offset)) != OSAL_OK)
                    {
                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2 write block size %d fail", (returnedByteNum - offset));
                        res = RES_ERROR_GENERIC;
                    }
                }
            } while((retcgi == 0) && (returnedByteNum == HDR_PHASE_MAX_BYTE_TO_SEND) && (res == HD_FLASH_STATUS_OK));
        }
    }

    // Free allocated resources
    if (NULL != pagein)
    {
        OSAL_vMemoryFree ((tVoid *)pagein);
    }
    if ((res == HD_FLASH_STATUS_OK) && (trg_mem == HDR_TARGET_SPI2FLASH))
    {
        res = HDR_ExecutePhase3 (devicehandler, paramPtr, numBytes, dataPtr);
    }
    else if ((res == HD_FLASH_STATUS_OK) && (trg_mem == HDR_TARGET_SPI2MEM))
    {
        // Normal communication with the HD processor can start in approximately 0.5 sec
        (void)OSAL_s32ThreadWait(500);
    }

#ifdef CONFIG_COMM_HDRADIO_SPI
    /* change SPI clock speed for normal mode */
    if ((HDDeviceConfig.communicationBusType == BusSPI) && (
#if defined(CONFIG_COMM_HDRADIO_SPI_CPHA1_CPOL1)
        (HDDeviceConfig.communicationBus.spi.mode != SPI_CPHA1_CPOL1) ||
#elif defined(CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0)
        (HDDeviceConfig.communicationBus.spi.mode != SPI_CPHA0_CPOL0) ||
#endif
        (HDDeviceConfig.communicationBus.spi.speed != HDRADIO_ACCORDO2_SPI_SPEED)))
    {
#if defined(CONFIG_COMM_HDRADIO_SPI_CPHA1_CPOL1)
        HDDeviceConfig.communicationBus.spi.mode = SPI_CPHA1_CPOL1;
#elif defined(CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0)
        HDDeviceConfig.communicationBus.spi.mode = SPI_CPHA0_CPOL0;
#endif /* CONFIG_COMM_HDRADIO_SPI_CPHA1_CPOL1 || CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0 */
        HDDeviceConfig.communicationBus.spi.speed = HDRADIO_ACCORDO2_SPI_SPEED;
        (void)HDRADIO_reconfiguration(&HDDeviceConfig);
    }
#endif

	ret = res;

exit:
    return ret;
}

//HDR Programming final checks of correct execution
static tSInt HDR_ExecutePhase3 (tVoid *devicehandler, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr)
{
    tS32 res, ret_read;
    tU8 hdBuffIn[HDR_READ_RESP_MAX_LEN], *p_hdBuffIn = hdBuffIn;
    int cnt_reads;
    EtalDeviceDesc deviceDescription;
		tSInt ret;

    HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "HD Radio send phase3");
    ETAL_getDeviceDescription_HDRADIO(&deviceDescription);

    cnt_reads = 0;
    hdBuffIn[0] = (tU8)0;
    hdBuffIn[1] = (tU8)0;
    ret_read = OSAL_OK;

    if (true == (deviceDescription.m_busType == ETAL_BusI2C))
    {
        while (((ret_read != OSAL_OK) || ((tU8)0x68 != hdBuffIn[1]) || ((tU8)0x00 == hdBuffIn[0])) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
            (void)OSAL_s32ThreadWait(20);
#endif
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 2);
            cnt_reads++;
        }
        if ((cnt_reads == 30) || (ret_read != OSAL_OK) || ((tU8)0x68 != hdBuffIn[1]))
        {
            // Wrong ack of the sent file
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 file reception ack failure");
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }
        else
        {
            // Speed seq executed, correct ack
            HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 file reception ack ok");
            res = RES_OK;
        }

        cnt_reads = 0;
        hdBuffIn[0] = hdBuffIn[1] = (tU8)0;
        ret_read = OSAL_OK;
        while (((ret_read != OSAL_OK) || ((((tU8)0x69 != hdBuffIn[1]) && ((tU8)0x49 != hdBuffIn[1])) || ((tU8)0x00 == hdBuffIn[0]))) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
            (void)OSAL_s32ThreadWait(50);
#endif
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 2);
            cnt_reads++;
        }

        if ((cnt_reads == 30) || (ret_read != OSAL_OK) || ((tU8)0x49 == hdBuffIn[1]))
        {
            // CRC failure
            if ((cnt_reads == 30) && ((ret_read != OSAL_OK) || ((ret_read != OSAL_OK) && ((tU8)0x49 != hdBuffIn[1]))))
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 crc failure timeout");
            }
            else
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 crc failure");
            }
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 crc check ok");


        // CRC passed .. Init PHASE 3 erasing memory
        hdBuffIn[0] = hdBuffIn[1] = (tU8)0;
        cnt_reads = 0;
        ret_read = OSAL_OK;
        while (((ret_read != OSAL_OK) || ((tU8)0x67 != hdBuffIn[1]) || ((tU8)0x00 == hdBuffIn[0])) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
            (void)OSAL_s32ThreadWait(1000);
#endif
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 2);
            cnt_reads++;
        }
        if (cnt_reads == 30)
        {
            // " Erase procedure failure =" + QString::number (hdBuffIn[1], 16))''
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 Erase failure");
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }

        // Erase completed
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 flash erase ok");

        cnt_reads = 0;
        hdBuffIn[0] = hdBuffIn[1] = (tU8)0;
        ret_read = OSAL_OK;
        while (((ret_read != OSAL_OK) || (((tU8)0x6F != hdBuffIn[1]) && ((tU8)0x4F != hdBuffIn[1])) || ((tU8)0x00 == hdBuffIn[0])) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
            (void)OSAL_s32ThreadWait(200);
#endif
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 2);
            cnt_reads++;
        }
        if ((cnt_reads == 30) || (ret_read != OSAL_OK) || ((tU8)0x4F == hdBuffIn[1]))
        {
            // " Flash not blanked! " + QString::number (hdBuffIn[1], 16));
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 flash not blanked");
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }
        // Flash Blank
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 flash blanked ok");

        cnt_reads = 0;
        hdBuffIn[0] = hdBuffIn[1] = (tU8)0;
        ret_read = OSAL_OK;
        while (((ret_read != OSAL_OK) || (((tU8)0x6D != hdBuffIn[1]) && ((tU8)0x4D != hdBuffIn[1])) || ((tU8)0x00 == hdBuffIn[0])) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
            (void)OSAL_s32ThreadWait(1000);
#endif
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn/*&hdBuffIn[0]*/, 2);
            cnt_reads++;
        }
        if ((cnt_reads == 30) || (ret_read != OSAL_OK) || ((tU8)0x4D == hdBuffIn[1]))
        {
            // Flash programming FAILURE
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 flash programming failure");
            res = RES_ERROR_GENERIC;
            ret = -1;
            goto exit;
        }
        else
        {
            HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 flash programming ok");
            res = RES_OK;
        }
        ret = res;
        goto exit;
        // Flash programming SUCCESSFUL
    }
    else
    {    // SPI 00 ------------------------------------------------------------------
        ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 1);
        while (((ret_read != OSAL_OK) || ((tU8)0x68 != hdBuffIn[0])) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
            (void)OSAL_s32ThreadWait(20);
#endif
            hdBuffIn[0] = (tU8)0;
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 1);
            cnt_reads++;
        }
        if ((cnt_reads == 30) || (ret_read != OSAL_OK) || ((tU8)0x68 != hdBuffIn[0]))
        {
            // Wrong ack of the sent file
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 file reception ack failure");
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }
        else
        {
            // Speed seq executed, correct ack
            HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 file reception ack ok");
            res = RES_OK;
        }

        cnt_reads = 0;
        hdBuffIn[0] =  (tU8)0;
        ret_read = OSAL_OK;
        while (((ret_read != OSAL_OK) || (((tU8)0x69 != hdBuffIn[0]) && ((tU8)0x49 != hdBuffIn[0]))) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
            (void)OSAL_s32ThreadWait(50);
#endif
            hdBuffIn[0] =  (tU8)0;
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 1);
            cnt_reads++;
        }

        if ((cnt_reads == 30) || (ret_read != OSAL_OK) || ((tU8)0x49 == hdBuffIn[0]))
        {
            // CRC failure
            if ((cnt_reads == 30) && ((ret_read != OSAL_OK) || ((ret_read == OSAL_OK) && ((tU8)0x49 != hdBuffIn[0]))))
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 crc failure timeout");
            }
            else
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 crc failure");
            }
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 crc check ok");


        //CRC passed .. Init PHASE 3 erasing memory
        hdBuffIn[0] = (tU8)0;
        cnt_reads = 0;
        ret_read = OSAL_OK;
        while (((ret_read != OSAL_OK) || ((tU8)0x67 != hdBuffIn[0])) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
            (void)OSAL_s32ThreadWait(1000);
#endif
            hdBuffIn[0] = (tU8)0;
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 1);
            cnt_reads++;
        }
        if (cnt_reads == 30)
        {
            // Erase procedure failure
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 erase failure");
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }

        // Erase completed
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 flash erase ok");

        cnt_reads = 0;
        hdBuffIn[0] = (tU8)0;
        ret_read = OSAL_OK;
        while (((ret_read != OSAL_OK) || (((tU8)0x6F != hdBuffIn[0]) && ((tU8)0x4F != hdBuffIn[0]))) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
            (void)OSAL_s32ThreadWait(200);
#endif
            hdBuffIn[0] = (tU8)0;
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 1);
            cnt_reads++;
        }
        if ((cnt_reads == 30) || ((tU8)0x4F == hdBuffIn[0]))
        {
            // Flash not blanked
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 flash not blanked");
            res = RES_ERROR_GENERIC;
            ret = res;
            goto exit;
        }
        // " Flash Blank ...";
        HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 flash blanked ok");

        cnt_reads = 0;
        hdBuffIn[0] = (tU8)0;
        ret_read = OSAL_OK;
        while (((ret_read != OSAL_OK) || (((tU8)0x6D != hdBuffIn[0]) && ((tU8)0x4D != hdBuffIn[0]))) && (cnt_reads < 30))
        {
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            Sleep (1000);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32) || defined (CONFIG_HOST_OS_FREERTOS)
            (void)OSAL_s32ThreadWait(1000);
#endif
            hdBuffIn[0] = (tU8)0;
            ret_read = HDRADIO_Read_Raw_Data(p_hdBuffIn, 1);
            cnt_reads++;
        }
        if ((cnt_reads == 30) || (ret_read != OSAL_OK) || ((tU8)0x4D == hdBuffIn[0]))
        {
            //" Flash programming FAILURE
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH3 flash programming failure");
            res = RES_ERROR_GENERIC;
            ret = -1;
            goto exit;
        }
        else
        {
            HDRADIO_tracePrintSystem(TR_CLASS_HDRADIO, "PH3 flash programming ok");
            res = RES_OK;
        }
        ret = res;
        goto exit;
        //" Flash programming SUCCESSFUL ";

    }

exit:
	return ret;
}

static tSInt HDR_Phase2ViaCmd (tVoid *devicehandler, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr)
{
    tSInt res = HD_FLASH_STATUS_OK;

    tUChar *pagein = NULL;
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    tULong fileSize = 0;
    FILE *pfin;
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    tU8 hdBuffIn[HDR_READ_RESP_MAX_LEN], *p_hdBuffIn = hdBuffIn;
    tBool first_memblock;
    HDR_accessModeEnumTy access_mode;
    EtalCbGetImage     cbGetImage;
    tVoid             *pvGetImageContext;
    tU32 requestedByteNum, returnedByteNum, remainingByteNum, pageinoffset;
    tSInt retcgi, resplen, *rlen = &resplen;
    tU16 nb_data_segments = 0, data_segment_nb = 0;
    tyHDRADIOInstanceID instId, instIdRes;
    etalToHDRADIOStatusTy cstat;
    tS8 ctype = (tS8)UNDEF_TYPE;

    // Get the access mode
    access_mode = (HDR_accessModeEnumTy)paramPtr[3];
    // Get instace Id
    instId = (tyHDRADIOInstanceID)paramPtr[2];
    instIdRes = instId;

    if (access_mode == HDR_ACCESS_FILE_MODE)
    {
#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
        // Open received file for binary read: the flasher file must already have the header
        // allocated at least as zeroes (it will be calculated again here)
        if ((pfin = OSAL_fopen ((const char*)dataPtr, "rb")) == NULL)
        {
            // Error : File is not accessible nor existing
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd open of file fail %s", dataPtr);
            return HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }
        else
        {
            // Get the size of file in bytes
            if (OSAL_fseek (pfin, 0, SEEK_END) != 0)
            {
                OSAL_fclose (pfin);
                return HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
            }
            fileSize = OSAL_ftell (pfin);
            if (OSAL_fseek (pfin, 0, SEEK_SET) != 0)
            {
                OSAL_fclose (pfin);
                return HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
            }

            if (res == HD_FLASH_STATUS_OK)
            {
                // Allocation of a dynamic array
                pagein = (tUChar *)OSAL_pvMemoryAllocate (HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND + CP_OVERHEAD);
                if (pagein == NULL)
                {
                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd allocation of size %d fail", HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND + CP_OVERHEAD);
                    res = HD_FLASH_STATUS_ERROR_MEMORY;
                }
            }

            if (res == HD_FLASH_STATUS_OK)
            {
                // Create Write_Pgm_To_Flash message
                pagein[0] = 0x84;                   // opcode
                pagein[CP_PAYLOAD_INDEX] = 0x04;    // function code Write_Pgm_To_Flash
                first_memblock = TRUE;
                do
                {
                    // Get Pgm block
                    if (first_memblock == TRUE)
                    {
                        requestedByteNum = HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND - 1;
                        pageinoffset = 1;
                    }
                    else
                    {
                        requestedByteNum = HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND;
                        pageinoffset = 0;
                    }
                    if ((retcgi = OSAL_fread (&(pagein[CP_PAYLOAD_INDEX + pageinoffset]), 1, requestedByteNum, pfin)) != (tSInt)requestedByteNum)
                    {
                        if (OSAL_ferror(pfin) != 0)
                        {
                            res = HD_FLASH_STATUS_ERROR_READ;
                        }
                        if ((OSAL_feof(pfin) != 0) || (OSAL_ferror(pfin) != 0))
                        {
                            OSAL_clearerr(pfin);
                        }
                    }
                    if (retcgi > 0)
                    {
                        returnedByteNum = (tU32)retcgi;
                    }
                    else
                    {
                        returnedByteNum = (tU32)0;
                    }
                    if (res == HD_FLASH_STATUS_OK)
                    {
                        if (first_memblock == TRUE)
                        {
                            nb_data_segments = (((fileSize - returnedByteNum) + (HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND - 1)) / HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND) + 1;
                            data_segment_nb = 0;
                            first_memblock = FALSE;
                        }

                        // add length
                        pagein[1] = (((returnedByteNum + pageinoffset)      ) & 0xFF);
                        pagein[2] = (((returnedByteNum + pageinoffset) >> 8 ) & 0xFF);
                        pagein[3] = (((returnedByteNum + pageinoffset) >> 16) & 0xFF);
                        pagein[4] = (((returnedByteNum + pageinoffset) >> 24) & 0xFF);

                        // add Command Type and Command Status
                        pagein[CP_PAYLOAD_INDEX + (returnedByteNum + pageinoffset)] = (tU8)HDR_CMD_WRITE_TYPE;   // Command Type
                        pagein[CP_PAYLOAD_INDEX + (returnedByteNum + pageinoffset) + 1] = (tU8)0;        // Reserved
                        pagein[CP_PAYLOAD_INDEX + (returnedByteNum + pageinoffset) + 2] = (tU8)0;        // Command Status

                        // Send Write_Pgm_To_Flash to download firmware memory block
                        if (HDRADIO_Write_Segment_Command(instId, nb_data_segments, data_segment_nb, pagein, (returnedByteNum + pageinoffset + CP_OVERHEAD)) == -1)
                        {
                            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd write Write_Pgm_To_Flash fail seg %d %d  len %d", nb_data_segments, data_segment_nb, fileSize);
                            res = HD_FLASH_STATUS_ERROR_WRITE;
                        }
                        else
                        {
                            // Read response
                            if (HDRADIO_Read_Segment_Status(instId, (tU8)0x84, nb_data_segments, data_segment_nb, p_hdBuffIn, rlen) == -1)
                            {
                                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd read Write_Pgm_To_Flash status fail seg %d %d  len %d", nb_data_segments, data_segment_nb, remainingByteNum);
                                res = HD_FLASH_STATUS_ERROR_READ;
                            }
                        }
                    }
                    if (res == HD_FLASH_STATUS_OK)
                    {
                        // check response length, status type and instanceId
                        if(*rlen >= CP_OVERHEAD)
                        {
                            cstat.B = p_hdBuffIn[*rlen - 1];
                            ctype = (tS8)p_hdBuffIn[*rlen - 3];
	                        if((res != HD_FLASH_STATUS_OK) || ((cstat.bf.Success != (tU8)1) || (ctype != (tS8)HDR_CMD_READ_TYPE) || (instId != instIdRes)))
	                        {
	                            ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "PH2viaCmd Error in the HD Response");
	                            res = RES_ERROR_GENERIC;
	                        }
                        }
                        else
                        {
                            res = RES_ERROR_GENERIC;
                        }
                        
                        if (*rlen > HDR_READ_RESP_MAX_LEN)
                        {
                            ASSERT_ON_DEBUGGING(0);
                            res = RES_ERROR_GENERIC;
                        }

                        // check response return data
                        if (((data_segment_nb < (nb_data_segments - 1)) && (*rlen != CP_OVERHEAD)) || ((data_segment_nb == (nb_data_segments - 1)) && 
                            ((*rlen != (CP_OVERHEAD + 1)) || ((tU8)0x04 != hdBuffIn[CP_PAYLOAD_INDEX]))))
                        {
                            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd wrong response to Write_Pgm_To_Flash invalid %02x %02x", *rlen, hdBuffIn[CP_PAYLOAD_INDEX]);
                            res = RES_ERROR_GENERIC;
                        }
                    }
                    data_segment_nb++;
                } while((data_segment_nb < nb_data_segments) && (res == HD_FLASH_STATUS_OK));
            }
            else
            {
                res = HD_FLASH_STATUS_ERROR_MEMORY;
            }
        }

        // Close file
        OSAL_fclose (pfin);
#else
        res = HD_FLASH_STATUS_ERROR_GENERIC;
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE)
    }
    else if (access_mode == HDR_ACCESS_DATA_MODE)
    {
        if (numBytes < 8)
        {
            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd payload length invalid %d", numBytes);
            res = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
        }
        if (res == HD_FLASH_STATUS_OK)
        {
            /* get read function callback and context */
			/* Lint deviation - code is due to the protocol itself, noway to change it */
            cbGetImage = (EtalCbGetImage)((dataPtr[0] << 24) | (dataPtr[1] << 16) | (dataPtr[2] << 8) | dataPtr[3]);	//lint !e701 - MISRA 10.5 - code is due to the protocol itself, noway to change it
            pvGetImageContext = (tPVoid)((dataPtr[4] << 24) | (dataPtr[5] << 16) | (dataPtr[6] << 8) | dataPtr[7]);		//lint !e701 - MISRA 10.5 - code is due to the protocol itself, noway to change it
            if (cbGetImage == NULL)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd cbGetImage invalid %p", cbGetImage);
                res = HD_FLASH_STATUS_ERROR_SOURCENOTAVAIL;
            }
        }
        if (res == HD_FLASH_STATUS_OK)
        {
            // Allocation of a dynamic array
            pagein = (tUChar *)OSAL_pvMemoryAllocate (HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND + CP_OVERHEAD);
            if (pagein == NULL)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd allocation of size %d fail", HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND + CP_OVERHEAD);
                res = HD_FLASH_STATUS_ERROR_MEMORY;
            }
        }
        if (res == HD_FLASH_STATUS_OK)
        {
            // Create Write_Pgm_To_Flash message
            pagein[0] = (tU8)0x84;                   // opcode
            pagein[CP_PAYLOAD_INDEX] = (tU8)0x04;    // function code Write_Pgm_To_Flash
            first_memblock = TRUE;
            do
            {
                // Get Pgm block
                returnedByteNum = 0;
                if (first_memblock == TRUE)
                {
                    requestedByteNum = HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND - 1;
                    pageinoffset = 1;
                }
                else
                {
                    requestedByteNum = HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND;
                    pageinoffset = 0;
                }
                retcgi = cbGetImage(pvGetImageContext, requestedByteNum, &(pagein[CP_PAYLOAD_INDEX + pageinoffset]), &returnedByteNum, &remainingByteNum, FALSE);  //lint !e644 - MISRA 9.1 - cbGetImage correctly initialize from message
                HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "retcgi %d, returnedByteNum %d, remainingByteNum %d\n", retcgi, returnedByteNum, remainingByteNum);
                if ((retcgi == 0) && (returnedByteNum != 0))
                {
                    if (first_memblock == TRUE)
                    {
                        nb_data_segments = (((remainingByteNum - returnedByteNum) + (HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND - 1)) / HDR_PHASE2VIACMD_MAX_BYTE_TO_SEND) + 1;
                        data_segment_nb = 0;
                        first_memblock = FALSE;
                    }

                    // add length
                    pagein[1] = (tU8)(((returnedByteNum + pageinoffset)      ) & 0xFF);
                    pagein[2] = (tU8)(((returnedByteNum + pageinoffset) >> 8 ) & 0xFF);
                    pagein[3] = (tU8)(((returnedByteNum + pageinoffset) >> 16) & 0xFF);
                    pagein[4] = (tU8)(((returnedByteNum + pageinoffset) >> 24) & 0xFF);

                    // add Command Type and Command Status
                    pagein[CP_PAYLOAD_INDEX + (returnedByteNum + pageinoffset)] = (tU8)HDR_CMD_WRITE_TYPE;   // Command Type
                    pagein[CP_PAYLOAD_INDEX + (returnedByteNum + pageinoffset) + 1] = (tU8)0;        // Reserved
                    pagein[CP_PAYLOAD_INDEX + (returnedByteNum + pageinoffset) + 2] = (tU8)0;        // Command Status

                    // Send Write_Pgm_To_Flash to download firmware memory block
                    if (HDRADIO_Write_Segment_Command(instId, nb_data_segments, data_segment_nb, pagein, (returnedByteNum + pageinoffset + CP_OVERHEAD)) == -1) 
                    {
                        HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd write Write_Pgm_To_Flash fail seg %d %d  len %d", nb_data_segments, data_segment_nb, remainingByteNum);
                        res = HD_FLASH_STATUS_ERROR_WRITE;
                    }
                    else
                    {
                        // Read response
                        if (HDRADIO_Read_Segment_Status(instId, (tU8)0x84, nb_data_segments, data_segment_nb, p_hdBuffIn, rlen) == -1)
                        {
                            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd read Write_Pgm_To_Flash status fail seg %d %d  len %d", nb_data_segments, data_segment_nb, remainingByteNum);
                            res = HD_FLASH_STATUS_ERROR_READ;
                        }
                    }
                    if (res == HD_FLASH_STATUS_OK)
                    {
                        // check response length, status type and instanceId
                        if(*rlen >= CP_OVERHEAD)
                        {
                            cstat.B = p_hdBuffIn[*rlen - 1];
                            ctype = (tS8)p_hdBuffIn[*rlen - 3];
	                        if((res != HD_FLASH_STATUS_OK) || ((cstat.bf.Success != (tU8)1) || (ctype != (tS8)HDR_CMD_READ_TYPE) || (instId != instIdRes)))
	                        {
	                            ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "PH2viaCmd Error in the HD Response");
	                            res = RES_ERROR_GENERIC;
	                        }
                        }
                        else
                        {
                            res = RES_ERROR_GENERIC;
                        }
                        
                        if (*rlen > HDR_READ_RESP_MAX_LEN)
                        {
                            ASSERT_ON_DEBUGGING(0);
                            res = RES_ERROR_GENERIC;
                        }
                        
                        // check response return data
                        if (((data_segment_nb < (nb_data_segments - 1)) && (*rlen != CP_OVERHEAD)) || ((data_segment_nb == (nb_data_segments - 1)) && 
                            ((*rlen != (CP_OVERHEAD + 1)) || ((tU8)0x04 != hdBuffIn[CP_PAYLOAD_INDEX]))))
                        {
                            HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd wrong response to Write_Pgm_To_Flash invalid %02x %02x", *rlen, hdBuffIn[CP_PAYLOAD_INDEX]);
                            res = RES_ERROR_GENERIC;
                        }
                    }
                    data_segment_nb++;
                }
            } while((retcgi == 0) && (returnedByteNum == requestedByteNum) && 
                    (data_segment_nb < nb_data_segments) && (res == HD_FLASH_STATUS_OK));
        }
    }

    // Polling of Pgm_Write_Status
    if (res == HD_FLASH_STATUS_OK)
    {
        // Create Pgm_Write_Status message
        pagein[0] = (tU8)0x84;                   // opcode
        pagein[CP_PAYLOAD_INDEX] = (tU8)0x05;    // function code Pgm_Write_Status

        // add length
        pagein[1] = (tU8)((1      ) & 0xFF);
        pagein[2] = 0;		// MISRA - pagein[2] = (tU8)((1 >> 8 ) & 0xFF);
        pagein[3] = 0;		// MISRA - pagein[3] = (tU8)((1 >> 16) & 0xFF);
        pagein[4] = 0;		// MISRA - pagein[4] = (tU8)((1 >> 24) & 0xFF);

        // add Command Type and Command Status
        pagein[CP_PAYLOAD_INDEX + 1] = (tU8)HDR_CMD_WRITE_TYPE;   // Command Type
        pagein[CP_PAYLOAD_INDEX + 1 + 1] = (tU8)0;        // Reserved
        pagein[CP_PAYLOAD_INDEX + 1 + 2] = (tU8)0;        // Command Status

        do
        {
            // Send Pgm_Write_Status message
            if (HDRADIO_sendCommand(instId, pagein, (1 + CP_OVERHEAD), p_hdBuffIn, rlen) != OSAL_OK)
            {
                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd write Pgm_Write_Status fail");
                res = HD_FLASH_STATUS_ERROR_WRITE;
            }
			else
			{
	            // check response return data
	            if ((*rlen != (CP_OVERHEAD + 2)) || ((tU8)0x05 != hdBuffIn[CP_PAYLOAD_INDEX]))
	            {
	                HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd wrong response to Pgm_Write_Status invalid %02x %02x", hdBuffIn[0], hdBuffIn[1]);
	                res = RES_ERROR_GENERIC;
	                if (*rlen > HDR_READ_RESP_MAX_LEN)
	                {
	                    ASSERT_ON_DEBUGGING(0); // memory corruption
	                }
	            }
	            else
	            {
	                // return data byte 1: Program Write Status 
	                // 0x00: Reserved, 0x01: Image Successfully Saved, 0x02: Image Verification Failed (CRC check does not match)
	                // 0x03: Idle BBP not in programming mode, 0x04: Erasing Flash, 0x05: Programming Flash, 
	                // 0x06: Verifying Flash Image, 0x07: Flash Image and NVM security status mismatch, 
	                // 0x08: Warning, attempting to update multiple-tuner flash image with single tuner flash image, 
	                // 0x09: Warning, attempting to update single tuner flash image with multipletuner flash image   
	                if (((tU8)0x01 != hdBuffIn[CP_PAYLOAD_INDEX + 1]) && ((tU8)0x04 != hdBuffIn[CP_PAYLOAD_INDEX + 1]) 
	                    && ((tU8)0x05 != hdBuffIn[CP_PAYLOAD_INDEX + 1]) && ((tU8)0x06 != hdBuffIn[CP_PAYLOAD_INDEX + 1]))
	                {
	                    HDRADIO_tracePrintError(TR_CLASS_HDRADIO, "PH2viaCmd response to Pgm_Write_Status bad status %d", hdBuffIn[CP_PAYLOAD_INDEX + 1]);
	                    res = RES_ERROR_GENERIC;
	                }
	                else if ((tU8)0x01 != hdBuffIn[CP_PAYLOAD_INDEX + 1])
	                {
	                    // wait 1s before next Pgm_Write_Status polling
	                    (void)OSAL_s32ThreadWait(1000);
	                }
	            }
			}
        } while ((res == HD_FLASH_STATUS_OK) && ((tU8)0x01 != hdBuffIn[CP_PAYLOAD_INDEX + 1]));
    }

    // Free allocated resources
    if (NULL != pagein)
    {
        OSAL_vMemoryFree ((tVoid *)pagein);
    }

    return res;
}

/**************************************
 *
 * HDRADIO_sendCommand
 *
 *************************************/
/*
 * Sends a command to the HDRADIO and read response
 *
 * Parameters:
 *  instId - HD instance number
 *  cmd    - pointer to the command buffer
 *  clen   - size of the command buffer in bytes
 *  resp   - pointer of the response buffer
 *  rlen   - size of the response buffer in bytes
 *
 * The format of buf is:
 * 0            - opcode
 * 1 to 4       - data length (N)
 * 5 to (5+N-1) - command payload starting with the function code byte
 * 5 + N        - operation (0 for request, 1 for write)
 * 5 + N+1      - reserved (0)
 * 5 + N+2      - command status
 *
 * The function creates one or more LM packets, fragmenting the payload
 * if required.
 *
 * Returns:
 *  OSAL_OK    - success
 *  OSAL_ERROR - write/read failure or system not configured
 *  OSAL_ERROR_INVALID_PARAM - invalid parameter
 *
 */
static tS32 HDRADIO_sendCommand(tyHDRADIOInstanceID instId, tU8 *cmd, tU32 clen, tU8 *resp, tSInt *rlen)
{
	tS32 ret = OSAL_OK;
	tyHDRADIOInstanceID instIdRes = INSTANCE_UNDEF;
	etalToHDRADIOStatusTy cstat;
	tS8 ctype = (tS8)UNDEF_TYPE;

	// set rlen to 0 
	// if leaving in error, in addition to the error, the len will be 0
	*rlen = 0;
	
	/* Check parameters */
	if (((instId != INSTANCE_1) && (instId != INSTANCE_2)) || (cmd == NULL) || (resp == NULL))
	{
		ASSERT_ON_DEBUGGING(0);
		ret = OSAL_ERROR_INVALID_PARAM;
	}

	if (ret == OSAL_OK)
	{
		/* Write command */
		if (HDRADIO_Write_Command(instId, cmd, clen) == -1)
		{
			ASSERT_ON_DEBUGGING(0);
			ret = OSAL_ERROR;
		}
		else
		{
			/* Read response */
			if (HDRADIO_Read_Response(&instIdRes, resp, rlen) != OSAL_OK)
			{
				ret = OSAL_ERROR;
			}
		}
	}

	if (ret == OSAL_OK)
	{
		/* Check response length, status type and instanceId */
		if(*rlen >= CP_OVERHEAD)
		{
			cstat.B = (resp)[*rlen - 1];
			ctype = (tS8)(resp)[*rlen - 3];
			if((ret != OSAL_OK) || ((cstat.bf.Success != (tU8)1) || (ctype != (tS8)HDR_CMD_READ_TYPE) || (instId != instIdRes)))
			{
				ret = OSAL_ERROR;
			}	
		}
		else
		{
			ret = OSAL_ERROR;
		}

	}

	return ret;
}

#if 0
static tSInt hdrWriteFlash0x84 (tVoid *devicehandler, tUChar * pagein, tULong fileSize, HDR_instanceIdEnumTy instId)
{

    tULong chunkPtr = 0;
    tSInt res = HD_FLASH_STATUS_OK;
    tULong nread = SEGSIZE;
    tU8 hdBuffOut[2050];
    tU8 hdBuffIn[2050];
    tUInt SegNb, jj, offset, cnt;
    tULong segSize;
    tU8 lmCount = 0;
    tSInt singlePerc = (HDRFLASHSIZE / SEGSIZE) / 100 + 1;
    tSInt cntPercent = 0;
    tBool sendBackSegStatus = true;
    tBool useI2c;
    tU32 hdI2cAdr;
    EtalDeviceDesc deviceDescription;

    ETAL_getDeviceDescription_HDRADIO(&deviceDescription);
    hdI2cAdr = deviceDescription.m_busAddress;
    useI2c = (deviceDescription.m_busType == ETAL_BusI2C) ? TRUE : FALSE;

    SegNb = 0;
    hdBuffOut[0] = 0x84;  //command ID
    hdBuffOut[3] = 0x04;  //sub command code
    hdBuffOut[2] = instId;    //Tuner INSTANCE

    hdBuffOut[1] = pagein[0];   //I2C Address


    while (chunkPtr < fileSize)
    {
        if ((fileSize - chunkPtr) < nread)
        {
            nread = fileSize - chunkPtr;
            sendBackSegStatus = false;
        }

//        if ((0 == SegNb) && (true == useI2c))
        if (0 == SegNb)
        {
            jj = 4;
            offset = 1;
        }
        else
        {
            jj = 3;
            offset = 0;
        }

        // Copy slots of size read
        for (cnt = 0; cnt< (nread-offset); cnt++)
        {
            hdBuffOut[cnt + jj] = pagein[chunkPtr+cnt + offset];
        }
        segSize = nread + 13;
        res = hdrSta680PackCommand (devicehandler, hdBuffOut, hdBuffIn, segSize, lmCount, SegNb, useI2c);
        if (res != HD_FLASH_STATUS_OK)
        {
            return res;
        }

        res = hdrSta680SendPackData (devicehandler, hdBuffIn, segSize+1, useI2c);

        if (res >= 0)
        {
            res = HD_FLASH_STATUS_OK;
        }
        if (res != HD_FLASH_STATUS_OK)
        {
            return res;
        }

        if (cntPercent >= singlePerc)
        {
            cntPercent = 0;
        }

        if ((0 == cntPercent) && (true == sendBackSegStatus))
        {

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            sprintf (LogBufferPointer (), "written Segment number %u", SegNb);
            LogStringAndFlush (LogBufferPointer (), LOG_MASK_TCP_IP);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
			HDRADIO_tracePrintComponent(TR_CLASS_HDRADIO, "written Segment number %u", SegNb);
#endif

            hdBuffIn[0] = 0x2e;
            hdBuffIn[1] = 0x55;
            hdBuffIn[2] = 0x00;
            hdBuffIn[3] = (SegNb >> 8) & 0xFF;
            hdBuffIn[4] = SegNb & 0xFF;
            hdBuffIn[5] = 0x00;
            hdBuffIn[6] = 0x84;
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
            tU8 spareByteArray[4] = { 0, 1, 0, 0 };
            ForwardPacketToCtrlAppPort (devicehandler, hdBuffIn, 7, INVALID_LUN, INVALID_PORT_NUMBER, &spareByteArray[0]);
#endif // #if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
        }

        cntPercent++;
        lmCount = (SegNb) & 0xFF;
        chunkPtr = chunkPtr + nread;
        SegNb++;
    }
    return res;


}

static tSInt hdrSta680PackCommand (tVoid *devicehandler, tUChar * hdBufferOut, tUChar * hdBufferIn, tULong fileSize, tU8 lmCount, int segmentNb, tBool useI2c)
{
    tSInt res = HD_FLASH_STATUS_OK;
    tUInt cnt;
    hdBufferIn[0] = hdBufferOut[1];
    hdBufferIn[1] = 0xA5;
    hdBufferIn[2] = 0xA5;
    // Logic Message Count
    hdBufferIn[3] = lmCount;
    hdBufferIn[4] = hdBufferOut[2];
    hdBufferIn[7] = hdBufferOut[0];  // commandID
    hdBufferIn[13] = hdBufferOut[3];  // subCommand
//    for (cnt = 14; cnt < fileSize; cnt++)
    for (cnt = 14; cnt < (fileSize); cnt++)
    {
        hdBufferIn[cnt] = hdBufferOut[cnt - 10];
    }

    hdBufferIn[8] = 0x00;

    if ((hdBufferIn[7] == 0x84) && ((hdBufferIn[13] == 0x04) || (segmentNb > 0)))
    {
        //number of data segments set to 0x00 0x00
        hdBufferIn[9] = (HDRFLASHSIZE / SEGSIZE) & 0xFF;         //  0xED;
        hdBufferIn[10] = ((HDRFLASHSIZE / SEGSIZE)>>8) & 0xFF;   //  0x10;
        hdBufferIn[12] = (segmentNb & 0xFF00) / 0x100;
        hdBufferIn[11] = segmentNb & 0xFF;
    }
    else
    {
        //number of data segments set to 0x00 0x00
        hdBufferIn[9] = 0x00;
        hdBufferIn[10] = 0x00;
        hdBufferIn[12] = 0x00;
        hdBufferIn[11] = 0x00;
    }
    // Logic Message length
    hdBufferIn[6] = (tUChar)((fileSize & 0xFF00) / 0x100);
    hdBufferIn[5] = (tUChar)(fileSize  & 0xFF);

    hdBufferIn[fileSize] = 0;
    for (cnt = 1; cnt <(fileSize); cnt++)
    {
        hdBufferIn[fileSize] = (hdBufferIn[fileSize] + hdBufferIn[cnt]) & 0xFF;
    }
    return res;
}

static tSInt hdrSta680SendPackData (tVoid *devicehandler, tUChar * hdBufferIn, tULong fileSize, tBool useI2c)
{
    tU8 rxBuffer[512], *p_hdBuffIn = rxBuffer;
    tSInt res = HD_FLASH_STATUS_OK, cnt;
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
    tU8 i2cHdAddr = hdBufferIn[0];
#endif // #if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
    tSInt  startPtr = 0;

    if (false == useI2c)
    {
        startPtr = 1;
    }

    for (cnt = 0; cnt < 256; cnt++)
    {
        rxBuffer[cnt] = 0;
    }

    if ((res = HDR_Write (devicehandler, &hdBufferIn[startPtr], fileSize-startPtr, useI2c, spiPhasehdBuffOut.h3.val, true)) <= 0)
    {
        res = RES_ERROR_GENERIC;
        return res;
    }

    // Read the response
    res = HDR_Read (devicehandler, p_hdBuffIn/*rxBuffer*/, 1,
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
    				i2cHdAddr
#else
					0
#endif
    				, useI2c, spiPhasehdBuffOut.h3.val);
    if (res != OSAL_OK)
    {
        res = RES_ERROR_GENERIC;
        return res;
    }

    if (rxBuffer[0] > 0)
    {
        res = HDR_Read (devicehandler, p_hdBuffIn/*rxBuffer*/, 1+rxBuffer[0],
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
						i2cHdAddr
#else
					0
#endif
						, useI2c, spiPhasehdBuffOut.h3.val);
    }
    if (res != OSAL_OK)
    {
        res = RES_ERROR_GENERIC;
        return res;
    }


    return res;
}
#endif

#ifdef __cplusplus
}
#endif

#endif // #if (defined CONFIG_COMM_DRIVER_EMBEDDED) && (defined CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

// End of file HDR_boot.cpp
