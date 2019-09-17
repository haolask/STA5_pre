//!
//!  \file    dcop_lld.cpp
//!  \brief   <i><b> DCOP LLD for flash protocol</b></i>
//!  \details STECI protocol used by ETAL and by MDR_Protocol
//!  \author  Alberto Saviotti
//!

#include "target_config.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)

#include "osal.h"

#include "common_trace.h"
#include "steci_trace.h"

#include "defines.h"
#include "types.h"
#include "utility.h"

#include "DAB_Protocol.h"
#include "steci_lld.h"
#include "dcop_lld.h"

#if (defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)) && (defined (CONFIG_BOARD_ACCORDO2) || defined (CONFIG_BOARD_ACCORDO5))
	#include "bsp_sta1095evb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

///
// DEFINES
///
#define DWORD     tU32


///
// MACROs
///
#define DCOP_OUTPUT_TEMP_BUFFER_SIZE		9000 // Arbitrary length, big enought

///
// Typedefs
///

///
// Local variables
///
#if 0 // CONFIG_HOST_OS_WIN32
static tU8 DCOP_tempBuffer[DCOP_OUTPUT_TEMP_BUFFER_SIZE];
#endif

///
// Local functions declarations
///
#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
static tSInt DCOP_LINUX_WaitReq_Low (tVoid *deviceHandle, DWORD timeoutValue);
static tSInt DCOP_LINUX_ReadPort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataPtr, tU32 timeoutValue, tU8 *outputBuffer);
static tSInt DCOP_LINUX_WritePort (tVoid *deviceHandle, tU32 size, tU8 *dataPtr, tU8 *outputBuffer);
#endif // CONFIG_HOST_OS_LINUX || CONFIG_HOST_OS_TKERNEL || defined (CONFIG_HOST_OS_FREERTOS)

#if 0 // CONFIG_HOST_OS_WIN32
static tVoid DCOP_WaitIncomingData (tVoid *deviceHandle, tU32 bytesToWaitFor, tS64 timeout);
static tSInt DCOP_FT2232H_WaitReqLow (tVoid *deviceHandle, DWORD timeoutValue);
static tSInt DCOP_FT4222H_WaitReqLow (tVoid *gpioHandle, DWORD timeoutValue);
static tSInt DCOP_FT2232H_ReadPort (tVoid *deviceHandle, tU32 size, tU8 *dataPtr, tU32 timeoutValue, tU8 *outputBuffer);
static tSInt DCOP_FT4222H_ReadPort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataOutBufferPtr, tU32 timeoutValue, tU8 *dataInBufferPtr);
static tSInt DCOP_FT2232H_WritePort (tVoid *deviceHandle, tU32 size, tU8 *dataPtr, tU8 *outputBuffer);
static tSInt DCOP_FT4222H_WritePort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataOutBufferPtr, tU8 *dataInBufferPtr);
#endif

///
// Global functions
///
tSInt DCOP_ReadPort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataPtr, tU32 timeoutValue, tU8 *outputBuffer)
{
	tSInt res = FLASH_STATUS_ERROR_GENERIC;

#if 0 // CONFIG_HOST_OS_WIN32
	if (FTDI_4222H == deviceInfoPtr->deviceMode)
	{
		res = DCOP_FT4222H_ReadPort (deviceInfoPtr, size, dataPtr, timeoutValue, outputBuffer);
	}
	else
	{
		res = DCOP_FT2232H_ReadPort (deviceInfoPtr->deviceHandle, size, dataPtr, timeoutValue, outputBuffer);
	}
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	res = DCOP_LINUX_ReadPort (deviceInfoPtr, size, dataPtr, timeoutValue, outputBuffer);
#endif // CONFIG_HOST_OS_LINUX || CONFIG_HOST_OS_TKERNEL || defined (CONFIG_HOST_OS_FREERTOS)

	return res;
}

tSInt DCOP_WritePort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataPtr, tU8 *outputBuffer)
{
    tSInt res = FLASH_STATUS_ERROR_GENERIC;

#if 0 // CONFIG_HOST_OS_WIN32
	if (FTDI_4222H == deviceInfoPtr->deviceMode)
	{
		res = DCOP_FT4222H_WritePort (deviceInfoPtr, size, dataPtr, outputBuffer);
	}
	else
	{
		res = DCOP_FT2232H_WritePort (deviceInfoPtr->deviceHandle, size, dataPtr, outputBuffer);
	}
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	res = DCOP_LINUX_WritePort (deviceInfoPtr->deviceHandle, size, dataPtr, outputBuffer);
#endif

    return res;
}

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
static tSInt DCOP_LINUX_ReadPort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataPtr, tU32 timeoutValue, tU8 *outputBuffer)
{
    tSInt res = FLASH_STATUS_ERROR_GENERIC;

    if (size > 0)
    {
        // Wait REQ line going LOW : the target flags the HOST that answer buffer is filled                 
        if (DCOP_LINUX_WaitReq_Low (deviceInfoPtr->deviceHandle, timeoutValue) == -1) 
        {
			// If REQ never goes low, wait until the timeout then exit
            return FLASH_STATUS_ERROR_GENERIC; //Error or Timeout
        }

        // Lower CS line
        BSP_SteciSetCS_MDR(0);

        // Send data
        STECI_tracePrintComponent(TR_CLASS_STECI, "DCOP_LINUX_ReadPort Tx: %d bytes", size);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
        COMMON_tracePrintBufComponent(TR_CLASS_STECI, (tU8 *)dataPtr, size, NULL);
#endif
        BSP_TransferSpi_MDR(dataPtr, outputBuffer, size);

        // Raise the CS line
        BSP_SteciSetCS_MDR(TRUE);

        if (NULL != outputBuffer && 0 != size) 
        {
            STECI_tracePrintComponent(TR_CLASS_STECI, "DCOP_LINUX_ReadPort Rx: %d bytes", size);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
            COMMON_tracePrintBufComponent(TR_CLASS_STECI, (tU8 *)outputBuffer, size, NULL);
#endif
            res = FLASH_STATUS_OK;
        }
        else
        {
            if (NULL == outputBuffer)
            {
                STECI_tracePrintError(TR_CLASS_STECI, "Read failed, NULL pointer passed");
            }
            else
            {
                STECI_tracePrintError(TR_CLASS_STECI, "Read failed, 0 bytes to read passed");
            }
            res = FLASH_STATUS_ERROR_READ;
        }
    }
    return res;
}

static tSInt DCOP_LINUX_WaitReq_Low (tVoid *deviceHandle, DWORD timeoutValue)
{
    tSInt res = 1;
    DWORD startTime, endTime, elapsedTime;
    static tU8 level_of_GPIOs;
    tBool gpioPollTimeoutReached = false; //set the global timeout variable

    level_of_GPIOs = BSP_SteciReadREQ_MDR();

    startTime = OSAL_ClockGetElapsedTime();

    // Wait for REQ line goes Low, before to attempt next operation
    while ((gpioPollTimeoutReached == false) && (level_of_GPIOs != (tU8)0))
    {
        level_of_GPIOs = BSP_SteciReadREQ_MDR();

        // Read system time
        endTime = OSAL_ClockGetElapsedTime ();
        elapsedTime = endTime - startTime;

        if (elapsedTime > timeoutValue)
        {
            gpioPollTimeoutReached = true;
            res = -1; //timeout error
        }
    }

    return res;
}
#endif // CONFIG_HOST_OS_LINUX || CONFIG_HOST_OS_TKERNEL || defined (CONFIG_HOST_OS_FREERTOS)

#if 0 // CONFIG_HOST_OS_WIN32
static tVoid DCOP_WaitIncomingData (tVoid *deviceHandle, tU32 bytesToWaitFor, tS64 timeout)
{
    tS64 startTime;
    tU32 bytesInTheQueue;

    // Get queue status
    p_FT_GetQueueStatus (deviceHandle, (DWORD *)&bytesInTheQueue);

    // Get current time
    startTime = UTILITY_GetTime ();

    // While until queue is correctly filled
    while (bytesInTheQueue < bytesToWaitFor)
    {
        p_FT_GetQueueStatus (deviceHandle, (DWORD *)&bytesInTheQueue);

        // Check timeout
        if (true == UTILITY_CheckTimeout (startTime, timeout))
        {
            return;
        }
    }

    return;
}


static tSInt DCOP_FT2232H_WaitReqLow (tVoid *deviceHandle, DWORD timeoutValue)
{
    tSInt res = 1;
    tS64 startTime, endTime, elapsedTime;
    static tU8 level_of_GPIOs;
    tBool gpioPollTimeoutReached = false; //set the global timeout variable

    level_of_GPIOs = FTDI_FT2232H_GetGPIOLevel (deviceHandle);

	startTime = UTILITY_GetTime ();

    // Wait for REQ line goes Low, before to attempt next operation
    while ((gpioPollTimeoutReached == false) && (((level_of_GPIOs & FTDI_REQ_PIN) >> 5) == 1))
    {
		level_of_GPIOs = FTDI_FT2232H_GetGPIOLevel (deviceHandle);

        // Read system time
		endTime = UTILITY_GetTime ();
        elapsedTime = endTime - startTime;

        if (elapsedTime > timeoutValue)
        {
            gpioPollTimeoutReached = true;

            res = -1; // Timeout error
        }
    }

    return res;
}

static tSInt DCOP_FT4222H_WaitReqLow (tVoid *gpioHandle, DWORD timeoutValue)
{
	tSInt res = 1;
	tS64 startTime, endTime, elapsedTime;
	tBool gpioPollTimeoutReached = false;
	tBool reqLevel;

	reqLevel = FTDI_FT4222H_GetGPIOLevel (gpioHandle, FTDI_FT4222H_REQ);

	startTime = UTILITY_GetTime ();

	// Wait for REQ line goes Low, before to attempt next operation
	while (false == gpioPollTimeoutReached && true == reqLevel)
	{
		reqLevel = FTDI_FT4222H_GetGPIOLevel (gpioHandle, FTDI_FT4222H_REQ);

		// Read system time
		endTime = UTILITY_GetTime ();
		elapsedTime = endTime - startTime;

		if (elapsedTime > timeoutValue)
		{
			gpioPollTimeoutReached = true;

			res = -1; // Timeout error
		}
	}

	return res;
}


static tSInt DCOP_FT2232H_ReadPort (tVoid *deviceHandle, tU32 size, tU8 *dataPtr, tU32 timeoutValue, tU8 *outputBuffer)
{
    FT_STATUS ftStatus;
    tSInt res = FLASH_STATUS_ERROR_GENERIC;
    tU32 dwNumBytesRead = 0;
    tU8 *txBfrPtr = &DCOP_tempBuffer[0];
    tU32 bufferCnt = 0;

    if (size > 0)
    {
        // Wait REQ line going LOW : the target flags the HOST that answer buffer is filled                 
		if (DCOP_FT2232H_WaitReqLow (deviceHandle, timeoutValue) == -1)
        {
			// If REQ never goes low, wait until the timeout then exit
            return FLASH_STATUS_ERROR_GENERIC; //Error or Timeout
        }

        // Lower CS line
        txBfrPtr[bufferCnt++] = 0x80;
        txBfrPtr[bufferCnt++] = FTDI_SET_CS_LOW;
        txBfrPtr[bufferCnt++] = GPIO_DIR;                   // 0 = in / 1 = out, set CSGPIO out, REQGPIO in

        txBfrPtr[bufferCnt++] = 0x30;                       // MPSSE command to Write bytes in from SPI phase 11

        // Send data
        txBfrPtr[bufferCnt++] = (size - 1) & 0xFF;          // LSB Length
        txBfrPtr[bufferCnt++] = ((size - 1) & 0xFF00) >> 8; // MSB Length		

        memcpy ((tVoid *)&txBfrPtr[bufferCnt], (const tVoid *)dataPtr, size);
        bufferCnt += size;

        // Raise the CS line
        txBfrPtr[bufferCnt++] = 0x80;
        txBfrPtr[bufferCnt++] = FTDI_SET_CS_HIGH;
        txBfrPtr[bufferCnt++] = GPIO_DIR;                    // 0 = in / 1 = out, set CSGPIO out, REQGPIO in	

        // Write command
        ftStatus = p_FT_Write (deviceHandle, &txBfrPtr[0], bufferCnt, (LPDWORD)&dwNumBytesRead);

        if (FT_OK != ftStatus)
        {
			res = FLASH_STATUS_ERROR_WRITE;
        }
        else
        {
            // Wait until data is available
            DCOP_WaitIncomingData (deviceHandle, size, DCOP_TIMEOUT_FLASH_ERASE);

            // Read data (check the receive buffer)
            if (0 != size)
            {
                ftStatus = p_FT_Read (deviceHandle, outputBuffer, size, (LPDWORD)&dwNumBytesRead);
            }

            if (FT_OK != ftStatus)
            {
				res = FLASH_STATUS_ERROR_READ;
            }
            else
            {
                res = FLASH_STATUS_OK;
            }
        }
    }

    return res;
}

static tSInt DCOP_FT4222H_ReadPort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataOutBufferPtr, tU32 timeoutValue, tU8 *dataInBufferPtr)
{
	tSInt res = FLASH_STATUS_ERROR_GENERIC;
	FT_STATUS ftStatus;
	tU16 dwNumBytesRead = 0;
	tU8 *txBfrPtr = &DCOP_tempBuffer[0];
	tU32 bufferCnt = 0;

	if (size > 0)
	{
		// Wait REQ line going LOW : the target flags the HOST that answer buffer is filled                 
		if (DCOP_FT4222H_WaitReqLow (deviceInfoPtr->deviceSecondaryHandle, timeoutValue) == -1)
		{
			// If REQ never goes low, wait until the timeout then exit
			return FLASH_STATUS_ERROR_GENERIC; // Error or Timeout
		}

		// Lower CS line
		STECI_ChipSelectAssert (deviceInfoPtr);

		// Read data
		ftStatus = p_FT4222_SPIMaster_SingleRead (deviceInfoPtr->deviceHandle, (tU8 *)dataInBufferPtr, size, &dwNumBytesRead, true);

		// Check result
		if (FT_OK != ftStatus)
		{
			res = FLASH_STATUS_ERROR_WRITE;
		}
		else
		{
			res = FLASH_STATUS_OK;
		}

		// Raise the CS line
		STECI_ChipSelectDeassert (deviceInfoPtr);
	}

	return res;
}
#endif

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
static tSInt DCOP_LINUX_WritePort (tVoid *deviceHandle, tU32 size, tU8 *dataPtr, tU8 *outputBuffer)
{
    tSInt res = FLASH_STATUS_ERROR_GENERIC;

    // Lower CS line
    BSP_SteciSetCS_MDR(FALSE);

    // Send data
    STECI_tracePrintComponent(TR_CLASS_STECI, "DCOP_LINUX_WritePort Tx: %d bytes", size);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
    COMMON_tracePrintBufComponent(TR_CLASS_STECI, (tU8 *)dataPtr, size, NULL);
#endif
    BSP_TransferSpi_MDR(dataPtr, outputBuffer, size);

    // Raise the CS line
    BSP_SteciSetCS_MDR(TRUE);

    // Check the receive buffer 
    if (NULL != outputBuffer && 0 != size) 
    {
        STECI_tracePrintComponent(TR_CLASS_STECI, "DCOP_LINUX_WritePort Rx: %d bytes", size);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
        COMMON_tracePrintBufComponent(TR_CLASS_STECI, (tU8 *)outputBuffer, size, NULL);
#endif
        res = FLASH_STATUS_OK;
    }
    else
    {
        if (NULL == outputBuffer)
        {
            STECI_tracePrintError(TR_CLASS_STECI, "Read failed, NULL pointer passed");
        }
        else
        {
            STECI_tracePrintError(TR_CLASS_STECI, "Read failed, 0 bytes to read passed");
        }
        res = FLASH_STATUS_ERROR_READ;
    }

    return res;
}
#endif // CONFIG_HOST_OS_LINUX || CONFIG_HOST_OS_TKERNEL || defined (CONFIG_HOST_OS_FREERTOS)

#if 0 // CONFIG_HOST_OS_WIN32
static tSInt DCOP_FT2232H_WritePort (tVoid *deviceHandle, tU32 size, tU8 *dataPtr, tU8 *outputBuffer)
{
    tSInt res = FLASH_STATUS_ERROR_GENERIC;
    FT_STATUS ftStatus;
    tU32 lpBytesWritten = 0;
    tU8 *txBfrPtr = &DCOP_tempBuffer[0];
    tU32 bufferCnt = 0;
    tU32 dwNumBytesRead = 0;

    // Lower CS line
    txBfrPtr[bufferCnt++] = 0x80;
    txBfrPtr[bufferCnt++] = FTDI_SET_CS_LOW;
    txBfrPtr[bufferCnt++] = GPIO_DIR;                              // 0 = in / 1 = out, set CSGPIO out, REQGPIO in

    txBfrPtr[bufferCnt++] = 0x30;                                  // 4 / MPSSE command to Write bytes in from SPI phase 11

    // Send data
    txBfrPtr[bufferCnt++] = (size - 1) & 0xFF;          // LSB Length
    txBfrPtr[bufferCnt++] = ((size - 1) & 0xFF00) >> 8; // MSB Length		

    memcpy ((tVoid *)&txBfrPtr[bufferCnt], (const tVoid *)dataPtr, size); bufferCnt += size;

    // Raise the CS line
    txBfrPtr[bufferCnt++] = 0x80;
    txBfrPtr[bufferCnt++] = FTDI_SET_CS_HIGH;
    txBfrPtr[bufferCnt++] = GPIO_DIR;                              // 0 = in / 1 = out, set CSGPIO out, REQGPIO in	

    // Write command
    ftStatus = p_FT_Write (deviceHandle, &txBfrPtr[0], bufferCnt, (LPDWORD)&lpBytesWritten);

    if (FT_OK != ftStatus)
    {
		res = FLASH_STATUS_ERROR_WRITE;
    }
    else
    {
        // Wait until data is available
		DCOP_WaitIncomingData (deviceHandle, size, DCOP_TIMEOUT_FLASH_WRITE);

        // Read data (check the receive buffer)
        if (0 != size)
        {
            ftStatus = p_FT_Read (deviceHandle, outputBuffer, size, (LPDWORD)&dwNumBytesRead);
        }

        if (FT_OK != ftStatus)
        {
			res = FLASH_STATUS_ERROR_READ;
        }
        else
        {
            res = FLASH_STATUS_OK;
        }
    }

    return res;
}

static tSInt DCOP_FT4222H_WritePort (STECI_deviceInfoTy *deviceInfoPtr, tU32 size, tU8 *dataOutBufferPtr, tU8 *dataInBufferPtr)
{
	tSInt res = FLASH_STATUS_ERROR_GENERIC;
	FT4222_STATUS ftStatus;
	tU16 dwNumBytesRead = 0;
	tS32 lpBytesWritten = 0;

	// Lower CS line
	STECI_ChipSelectAssert (deviceInfoPtr);

	// Send data
	ftStatus = p_FT4222_SPIMaster_SingleReadWrite (deviceInfoPtr->deviceHandle, (tU8 *)dataInBufferPtr, dataOutBufferPtr,
		                                           size, &dwNumBytesRead, true);

	if (FT4222_OK == ftStatus)
	{
		res = FLASH_STATUS_OK;
	}

	// Raise the CS line
	STECI_ChipSelectDeassert (deviceInfoPtr);

	return res;
}

#endif

#ifdef __cplusplus
}
#endif

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_ETAL_SUPPORT_DCOP_MDR

// End of file
