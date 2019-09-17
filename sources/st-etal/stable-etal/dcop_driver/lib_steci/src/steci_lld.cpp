//!
//!  \file 		 steci_lld.cpp
//!  \brief 	 <i><b> STECI low level driver </b></i>
//!  \details This module provide helper function to connect to the hardware.
//!           These function shall be reworked for the application hardware.
//!  \author 	Alberto Saviotti
//!

#include "target_config.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)

#include "osal.h"

#include "common_trace.h"
#include "steci_trace.h"
#if (defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)) && (defined (CONFIG_BOARD_ACCORDO2) || defined (CONFIG_BOARD_ACCORDO5))
	#include "bsp_sta1095evb.h"
#endif


#include "steci_helpers.h"
#include "DAB_Protocol.h"
#include "DAB_internal.h"
#include "steci_lld.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0 // CONFIG_HOST_OS_WIN32
static tS32 STECI_FT2232H_SpiWriteSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tVoid *lpBufferPnt, tU32 dwBytesToWrite, tVoid *RxBuffer,
	                                         tBool *nextReqActiveStatusPtr, tU8 *tmpTxDataPtr,
		                                     tBool waitReadyCondition, tBool closeCommunication);

static tS32 STECI_FT4222H_SpiWriteSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tVoid *lpBufferPnt, tU32 dwBytesToWrite, tVoid *RxBuffer,
											 tBool *nextReqActiveStatusPtr, tU8 *tmpTxDataPtr,
											 tBool waitReadyCondition, tBool closeCommunication);
static tVoid STECI_WaitIncomingData (STECI_deviceInfoTy *deviceInfoPtr, DWORD bytesToWaitFor);
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
static tS32 STECI_LINUX_SpiWriteSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tVoid *lpBufferPnt, tU32 dwBytesToWrite, tVoid *RxBuffer,
	                          tBool *nextReqActiveStatusPtr, tU8 *tmpTxDataPtr,
                              tBool waitReadyCondition, tBool closeCommunication);
#endif

static tBool STECI_WaitReqReady (STECI_deviceInfoTy *deviceInfoPtr, tBool levelToWaitFor);

tBool STECI_GetReqLevel (STECI_deviceInfoTy *deviceInfoPtr)
{
    tBool reqLevel = false;
#if 0 // CONFIG_HOST_OS_WIN32
	tU8 tmpRes;

	if (FTDI_4222H == deviceInfoPtr->deviceMode)
	{
		reqLevel = FTDI_FT4222H_GetReqLevel (deviceInfoPtr->deviceSecondaryHandle);
	}
	else
	{
		tmpRes = FTDI_FT2232H_GetReqLevel (deviceInfoPtr->deviceHandle);

		if (0 != tmpRes)
		{
			reqLevel = true;
		}
	}
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
    reqLevel = BSP_SteciReadREQ_MDR();
#endif

    return reqLevel;
}

tBool STECI_ToggleReqLevel (tBool reqLevel)
{
	tBool res;

	(true == reqLevel) ? (res = false) : (res = true);


	// +DEBUG
//	STECI_tracePrintError(TR_CLASS_STECI, "STECI_ToggleReqLevel : reqLevel = %d, res = %d\n", 
//			reqLevel, res);
	
	//-DEBUG

    return res;
}

tS32 STECI_ResetDevice (STECI_deviceInfoTy *deviceInfoPtr)
{
#if 0 // CONFIG_HOST_OS_WIN32
    FT_STATUS res;

    if (FTDI_4222H == deviceInfoPtr->deviceMode)
    {
        // Reset device in SPI mode
        res = FTDI_FT4222H_ResetSPIDevice (deviceInfoPtr->deviceHandle, deviceInfoPtr->deviceSecondaryHandle, false);
    }
    else
    {
        // Init SPI device
        FTDI_FT2232H_InitSPIDevice (deviceInfoPtr->deviceHandle, FTDI_GetNormalSpiSpeed (), false);

    // Reset device in SPI mode
        res = FTDI_FT2232H_ResetSPIDevice (deviceInfoPtr->deviceHandle, false, true);
    }

    if (FT_OK != res)
    {
        return STECI_STATUS_ERROR;
    }
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
    if (BSP_DeviceReset_MDR(deviceInfoPtr->DABDeviceConfigID) != 0)
    {
        return STECI_STATUS_ERROR;
    }
#endif
    return STECI_STATUS_SUCCESS;
}

tS32 STECI_SetDeviceBootModeAndRestart (STECI_deviceInfoTy *deviceInfoPtr, tBool bootSelLevel)
{
#if 0  // CONFIG_HOST_OS_WIN32
    FT_STATUS res;
    tU32 bootspiSpeed = 6;
    if (FTDI_4222H == deviceInfoPtr->deviceMode)
    {
        // TODO: if we increment the normal speed of SPI then we need to slow down the speed when flashing

        // Reset device in SPI mode
        res = FTDI_FT4222H_ResetSPIDevice (deviceInfoPtr->deviceHandle, deviceInfoPtr->deviceSecondaryHandle, bootSelLevel);
    }
    else
    {
        // Init SPI device
        if (true == bootSelLevel)
        {
            // Reset of device with reduced speed for firmware update
            FTDI_FT2232H_InitSPIDevice (deviceInfoPtr->deviceHandle, bootspiSpeed, false);

            // Reset device in boot mode
            res = FTDI_FT2232H_ResetSPIDevice (deviceInfoPtr->deviceHandle, false, false);
    }
        else
        {
            // Init SPI device with PSI speed as requested by user for normal operations
            FTDI_FT2232H_InitSPIDevice (deviceInfoPtr->deviceHandle, FTDI_GetNormalSpiSpeed (), false);

            // Reset device in normal mode
            res = FTDI_FT2232H_ResetSPIDevice (deviceInfoPtr->deviceHandle, false, true);
        }
    }

    if (FT_OK != res)
    {
        return STECI_STATUS_ERROR;
    }
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
    // restart DCOP
    if (BSP_DeviceInit_MDR(deviceInfoPtr->DABDeviceConfigID) == OSAL_OK)
    {
        /* Set BOOT GPIO */
        BSP_SteciSetBOOT_MDR(bootSelLevel);

#ifdef INCLUDE_INTERACTIVE_DCOP_RESET_CODE
        if (etalTestOptionSkipDCOPReset)
        {
            STECI_tracePrintComponent(TR_CLASS_BSP, "Skipping DCOP reset");
            STECI_tracePrintComponent(TR_CLASS_BSP, "Load DCOP and hit RETURN");
            fgetc(stdin);
        }
        else
        {
            // Reset DCOP device
            if (STECI_ResetDevice(deviceInfoPtr) != OSAL_OK)
            {
                return STECI_STATUS_ERROR;
            }
        }
#else
        if (STECI_ResetDevice(deviceInfoPtr) != OSAL_OK)
        {
            return STECI_STATUS_ERROR;
        }
#endif //INCLUDE_INTERACTIVE_DCOP_RESET_CODE
    }
    else
    {
        return STECI_STATUS_ERROR;
    }
#endif

    return STECI_STATUS_SUCCESS;
}

tS32 STECI_ChipSelectDeassert (STECI_deviceInfoTy *deviceInfoPtr)
{
#if 0 // CONFIG_HOST_OS_WIN32
    FT_STATUS res;

	if (FTDI_4222H == deviceInfoPtr->deviceMode)
	{
		res = FTDI_FT4222H_ChipSelectDeassert (deviceInfoPtr->deviceSecondaryHandle);
	}
	else
	{
		res = FTDI_FT2232H_ChipSelectDeassert (deviceInfoPtr->deviceHandle);
	}

    if (FT_OK != res)
    {
        return STECI_STATUS_ERROR;
    }
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
    BSP_SteciSetCS_MDR(TRUE);
#endif
    return STECI_STATUS_SUCCESS;
}

tS32 STECI_ChipSelectAssert (STECI_deviceInfoTy *deviceInfoPtr)
{
#if 0 // CONFIG_HOST_OS_WIN32
	FT_STATUS res;

	if (FTDI_4222H == deviceInfoPtr->deviceMode)
	{
		res = FTDI_FT4222H_ChipSelectAssert (deviceInfoPtr->deviceSecondaryHandle);
	}
	else
	{
		res = FTDI_FT2232H_ChipSelectAssert (deviceInfoPtr->deviceHandle);
	}	

	if (FT_OK != res)
	{
		return STECI_STATUS_ERROR;
	}
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
    BSP_SteciSetCS_MDR(0);
#endif
	return STECI_STATUS_SUCCESS;
}
    
tS32 STECI_SpiWriteSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tVoid *lpBufferPnt, tU32 dwBytesToWrite, tVoid *RxBuffer,
	                          tBool *nextReqActiveStatusPtr, tU8 *tmpTxDataPtr,
                              tBool waitReadyCondition, tBool closeCommunication)
{
	tS32 dwNumBytesRead;

#if 0 // CONFIG_HOST_OS_WIN32
	if (FTDI_4222H == deviceInfoPtr->deviceMode)
	{
		dwNumBytesRead = STECI_FT4222H_SpiWriteSteciMode (deviceInfoPtr, lpBufferPnt, dwBytesToWrite, RxBuffer,
			                                              nextReqActiveStatusPtr, tmpTxDataPtr, waitReadyCondition, closeCommunication);
	}
	else
	{
		dwNumBytesRead = STECI_FT2232H_SpiWriteSteciMode (deviceInfoPtr, lpBufferPnt, dwBytesToWrite, RxBuffer,
			                                              nextReqActiveStatusPtr, tmpTxDataPtr, waitReadyCondition, closeCommunication);
	}
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	dwNumBytesRead = STECI_LINUX_SpiWriteSteciMode (deviceInfoPtr, lpBufferPnt, dwBytesToWrite, RxBuffer,
		                                              nextReqActiveStatusPtr, tmpTxDataPtr, waitReadyCondition, closeCommunication);
#endif

    return dwNumBytesRead;
}

#if 0 // CONFIG_HOST_OS_WIN32
static tS32 STECI_FT2232H_SpiWriteSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tVoid *lpBufferPnt, tU32 dwBytesToWrite, tVoid *RxBuffer,
	                                         tBool *nextReqActiveStatusPtr, tU8 *tmpTxDataPtr,
	                                         tBool waitReadyCondition, tBool closeCommunication)
{
    tU8 GPIO_val, GPIO_dir;
    tS32 lpBytesWritten = 0;    
    tU32 bufferCnt = 0;
    tS32 dwNumBytesRead = 0;
    DWORD dwNumBytesToRead; 
    FT_STATUS ftStatus;
	tBool reqChanged;

    // Lines status
	GPIO_val = GPIO_NOT_SPI_DI;
	GPIO_dir = GPIO_DIR; // Leave GPIO 1 and MISO as input

    // Set waiting for next active state of the REQ line if this is not the reaction of an already toggled REQ line
	if (true == waitReadyCondition)
	{
		// Lower CS line
		STECI_ChipSelectAssert (deviceInfoPtr);

		// Wait for the requets line to be at the wanted level
		reqChanged = STECI_WaitReqReady (deviceInfoPtr, *nextReqActiveStatusPtr);

		// Check if we exited for timeout, in timeout we return without sending data
		if (false == reqChanged)
		{
			// Set CS into inactive status
			STECI_ChipSelectDeassert (deviceInfoPtr);

			return STECI_STATUS_DEV_NOT_READY;
		}
	}
	else
	{
		// Lower CS line in fast mode
		*(tmpTxDataPtr + bufferCnt) = 0x80;
		bufferCnt++;
		*(tmpTxDataPtr + bufferCnt) = GPIO_val  & (~FTDI_CS_PIN); // 4MSB set level GPIO
		bufferCnt++;
		*(tmpTxDataPtr + bufferCnt) = GPIO_dir;                   // 0 = in / 1 = out, set CSGPIO out, REQGPIO in 
		bufferCnt++;
	}

    // Send the data
	*(tmpTxDataPtr + bufferCnt) = 0x30;                                 // 4 / MPSSE command to Write bytes in from SPI phase 11
	bufferCnt++;
	*(tmpTxDataPtr + bufferCnt) = ((dwBytesToWrite)-1) & 0xFF;          // LSB Length
	bufferCnt++;
	*(tmpTxDataPtr + bufferCnt) = (((dwBytesToWrite)-1) & 0xFF00) >> 8; // MSB Length		
	bufferCnt++;

	memcpy ((void *)(tmpTxDataPtr + bufferCnt), (const void *)lpBufferPnt, dwBytesToWrite);

    bufferCnt += dwBytesToWrite;

    // Raise the CS line
    if (true == closeCommunication)
    {
		*(tmpTxDataPtr + bufferCnt) = 0x80;
		bufferCnt++;
		*(tmpTxDataPtr + bufferCnt) = GPIO_val | FTDI_CS_PIN; // 4MSB set level GPIO
		bufferCnt++;
		*(tmpTxDataPtr + bufferCnt) = GPIO_dir;                              // 0 = in / 1 = out, set CSGPIO out, REQGPIO in
		bufferCnt++;
    }

    // Write command
	ftStatus = p_FT_Write (deviceInfoPtr->deviceHandle, tmpTxDataPtr, bufferCnt, (LPDWORD)&lpBytesWritten);

    // Get queue status: wait until wanted data are available
	STECI_WaitIncomingData (deviceInfoPtr, dwBytesToWrite);


    // Check the receive buffer 
    if (NULL != RxBuffer) 
    {
		dwNumBytesToRead = dwBytesToWrite;

		ftStatus = p_FT_Read (deviceInfoPtr->deviceHandle, (unsigned char *)RxBuffer, dwNumBytesToRead, (LPDWORD)&dwNumBytesRead);
    }
    else
    {
        if (NULL == RxBuffer)
        {
			LogString ("Read failed, NULL pointer passed", LOG_MASK_DEVICE_STATUS);
        }
        else
        {
			LogString ("Read failed, 0 bytes to read passed", LOG_MASK_DEVICE_STATUS);
        }
    }

    return dwNumBytesRead;
}

static tS32 STECI_FT4222H_SpiWriteSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tVoid *lpBufferPnt, tU32 dwBytesToWrite, tVoid *RxBuffer,
	                                         tBool *nextReqActiveStatusPtr, tU8 *tmpTxDataPtr,
	                                         tBool waitReadyCondition, tBool closeCommunication)
{
	tSInt bufferCnt = 0;
	tU16 dwNumBytesRead = 0;
	tBool reqChanged;
	FT4222_STATUS ftStatus;
	tS32 lpBytesWritten = 0;

	// Set waiting for next active state of the REQ line if this is not the reaction of an already toggled REQ line
	if (true == waitReadyCondition)
	{
		// Lower CS line
		STECI_ChipSelectAssert (deviceInfoPtr);

		// Wait for the requets line to be at the wanted level
		reqChanged = STECI_WaitReqReady (deviceInfoPtr, *nextReqActiveStatusPtr);

		// Check if we exited for timeout, in timeout we return without sending data
		if (false == reqChanged)
		{
			// Raise CS line
			STECI_ChipSelectDeassert (deviceInfoPtr);

			return STECI_STATUS_DEV_NOT_READY;
		}
	}
	else
	{
		// Lower CS line in fast mode
		STECI_ChipSelectAssert (deviceInfoPtr);
	}

	// Copy data into the TX buffer
	memcpy ((void *)(tmpTxDataPtr + bufferCnt), (const void *)lpBufferPnt, dwBytesToWrite);
	bufferCnt += dwBytesToWrite;

	// Send the data
	ftStatus = p_FT4222_SPIMaster_SingleReadWrite (deviceInfoPtr->deviceHandle, (unsigned char *)RxBuffer, tmpTxDataPtr, 
		                                           bufferCnt, &dwNumBytesRead, true);

	// Raise the CS line
	if (true == closeCommunication)
	{
		STECI_ChipSelectDeassert (deviceInfoPtr);
	}

	return dwNumBytesRead;
}

static tVoid STECI_WaitIncomingData (STECI_deviceInfoTy *deviceInfoPtr, DWORD bytesToWaitFor)
{
	tS64 startTime;
	DWORD bytesInTheQueue;

	// Get queue status
	p_FT_GetQueueStatus (deviceInfoPtr->deviceHandle, &bytesInTheQueue);

	// Get current time
	startTime = UTILITY_GetTime ();

	// While until queue is correctly filled
	while (bytesInTheQueue < bytesToWaitFor)
	{
		p_FT_GetQueueStatus (deviceInfoPtr->deviceHandle, &bytesInTheQueue);

		// Check timeout
		if (true == UTILITY_CheckTimeout (startTime, MAX_TIMEOUT_WHILECYCLE))
		{
			return;
		}
	}

	return;
}
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
static tS32 STECI_LINUX_SpiWriteSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tVoid *lpBufferPnt, tU32 dwBytesToWrite, tVoid *RxBuffer,
	                          tBool *nextReqActiveStatusPtr, tU8 *tmpTxDataPtr,
                              tBool waitReadyCondition, tBool closeCommunication)
{
	tU32 dwNumBytesRead = 0;
	tBool reqChanged;

	// Lower CS line
	BSP_SteciSetCS_MDR(0);

	// Set waiting for next active state of the REQ line if this is not the reaction of an already toggled REQ line
	if (true == waitReadyCondition)
	{
		// Lower CS line
		(LINT_IGNORE_RET) STECI_ChipSelectAssert (deviceInfoPtr);

		// Wait for the requets line to be at the wanted level
		reqChanged = STECI_WaitReqReady (deviceInfoPtr, *nextReqActiveStatusPtr);

		// Check if we exited for timeout, in timeout we return without sending data
		if (false == reqChanged)
		{
			// Set CS into inactive status
			(LINT_IGNORE_RET) STECI_ChipSelectDeassert (deviceInfoPtr);

			STECI_tracePrintComponent(TR_CLASS_STECI, "STECI_LINUX_SpiWriteSteciMode STECI_STATUS_DEV_NOT_READY");
			return STECI_STATUS_DEV_NOT_READY;
		}
	}
	else
	{
		// Lower CS line in fast mode
		(LINT_IGNORE_RET) STECI_ChipSelectAssert (deviceInfoPtr);
	}

	BSP_TransferSpi_MDR((tU8 *)lpBufferPnt, (tU8 *)RxBuffer, dwBytesToWrite);

	// Raise the CS line
	if (true == closeCommunication)
	{
		BSP_SteciSetCS_MDR(TRUE);
/*		// Send the data
		STECI_tracePrintSysmin(TR_CLASS_STECI, "Tx: %d bytes", dwBytesToWrite);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_STECI, (tU8 *)lpBufferPnt, dwBytesToWrite, "Tx: ");
#endif
		COMMON_tracePrintBufError(TR_CLASS_STECI, (tU8 *)lpBufferPnt, dwBytesToWrite, "Tx: ");
*/
		dwNumBytesRead = dwBytesToWrite;
		// Check the receive buffer 
		if (NULL != RxBuffer && 0 != dwNumBytesRead) 
		{
			STECI_tracePrintComponent(TR_CLASS_STECI, "Rx: %d bytes", dwNumBytesRead);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
			COMMON_tracePrintBufComponent(TR_CLASS_STECI, (tU8 *)RxBuffer, dwNumBytesRead, "Rx: ");
#endif
		}
	}

	// Send the data
	STECI_tracePrintComponent(TR_CLASS_STECI, "Tx: %d bytes", dwBytesToWrite);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
	COMMON_tracePrintBufComponent(TR_CLASS_STECI, (tU8 *)lpBufferPnt, dwBytesToWrite, "Tx: ");
#endif

	dwNumBytesRead = dwBytesToWrite;
	// Check the receive buffer 
	if (NULL != RxBuffer && 0 != dwNumBytesRead) 
	{
		STECI_tracePrintComponent(TR_CLASS_STECI, "Rx: %d bytes", dwNumBytesRead);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_STECI, (tU8 *)RxBuffer, dwNumBytesRead, "Rx: ");
#endif
	}
	else
	{
		if (NULL == RxBuffer)
		{
            STECI_tracePrintError(TR_CLASS_STECI, "Read failed, NULL pointer passed");
		}
		else
		{
            STECI_tracePrintError(TR_CLASS_STECI, "Read failed, 0 bytes to read passed");
		}
	}

	return (tS32)dwNumBytesRead;
}
#endif

static tBool STECI_WaitReqReady (STECI_deviceInfoTy *deviceInfoPtr, tBool levelToWaitFor)
{
	#define WAIT_REQ_TIMEOUT					118
	
	tS64 startTime;
	tBool currentLevel;

	// Get current time
	startTime = UTILITY_GetTime ();

	// While until queue is correctly filled
	do
	{
		// Check timeout
		if (true == UTILITY_CheckTimeout (startTime, WAIT_REQ_TIMEOUT))
		{
			return false;
		}

		// Check level
		currentLevel = STECI_GetReqLevel (deviceInfoPtr);
	} while (levelToWaitFor != currentLevel);

	return true;
}

#ifdef __cplusplus
}
#endif

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_ETAL_SUPPORT_DCOP_MDR

// End of file
