//!
//!  \file		cmost_lld.cpp
//!  \brief 	<i><b> CMOST driver </b></i>
//!  \details	Low level driver for the CMOST device. For non-CONFIG_HOST_OS_WIN32 builds
//!				the functions here contained are mostly empty wrappers around BSP functions.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi, Roberto Allevi, Alberto Saviotti
//!  $Revision$
//!  $Date$
//!


#include "target_config.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_CMOST)

	#include "osal.h"

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL)
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include "bsp_sta1095evb.h"
#elif defined (CONFIG_HOST_OS_FREERTOS)
	#include <unistd.h>
	#include <stdio.h>
	#include "bsp_sta1095evb.h"
#elif (defined CONFIG_HOST_OS_WIN32)
	#include <winsock2.h>
	#include "types.h"
	#include "ftdi_mngm.h"
	#include "utility.h"
#endif //#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)

#include "cmost_lld.h"
#include "common_trace.h"
#include "cmost_trace.h"
#include "tunerdriver_internal.h"
#include "tunerdriver.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \def		FRAME_SIZE
 *			Maximum frame size for a single CMOST transmission
 */
#define FRAME_SIZE						1024

#if defined (CONFIG_HOST_OS_WIN32)
	static DWORD CMOST_SpiWriteSeq (tVoid *ftHandle, LPVOID lpBufferPnt, DWORD dwBytesToWrite, LPVOID RxBuffer, 
                                tU8 curspiCSline, tBool commkeptActive);

	static tS32 CMOST_ReadData (tVoid *devicehandler, tU8 * destinationBufferPnt, tS32 bytesToRead, tBool useI2c,
								tU8 i2cAdr, tBool i2CStartOn, tBool i2CStopOn);

	static tU32 CMOST_WriteData (tVoid *devicehandler, tU8 * sourceBufferPnt, tU32 bytesToWrite, tBool useI2c, tU8 i2cAdr);
	static DWORD CMOST_I2cWriteWithAckCheck (tVoid *handle, LPVOID lpBufferPnt, DWORD dwBytesToWrite, LPVOID RxBuffer);
	static DWORD CMOST_I2cWriteWithoutAckCheck (tVoid *handle, LPVOID lpBufferPnt, DWORD dwBytesToWrite);
	static DWORD CMOST_I2cReadWithAckCheck (tVoid * handle, LPVOID lpBufferPnt, DWORD dwBytesToRead, LPVOID RxBuffer);
	static DWORD CMOST_I2cReadWithoutAckCheck (tVoid * handle, LPVOID lpBufferPnt, DWORD dwBytesToRead, LPVOID RxBuffer, 
                                           tBool i2CStartOn, tBool i2CStopOn);

	static tVoid CMOST_WaitIncomingData (tVoid *ftHandle, DWORD bytesToWaitFor);
#else
	static tSInt CMOST_SpiWriteSeq (tU32 deviceID, tU8* lpBufferPnt, tU16 dwBytesToWrite, tU8*  RxBuffer, tU16 dwBytesToRead, tBool commkeptActive);
#endif // CONFIG_HOST_OS_TKERNEL


/***************************
 *
 * CMOST_ReadRaw
 *
 **************************/
/*!
 * \brief		Reads a buffer from the CMOST through I2C or SPI bus
 * \details		This function can be called before the CMOST firmware has been started.
 * \remark		Must be called after #BSP_BusConfig_CMOST
 * \param[in]	devicehandler_win32 - only for CONFIG_HOST_OS_WIN32, ignored in other builds
 * \param[out]	buf - pointer to the buffer where the function stores the data read.
 * 				      Must be large enough to hold the *len* bytes.
 * \param[in]	len - number of bytes to read
 * \param[in]	useI2c - TRUE if the device is on the I2C bus, FALSE if it is on the SPI bus
 * \param[in]	i2cAdr - the device's physical address
 * \param[in]	i2CStartOn_win32 - only for CONFIG_HOST_OS_WIN32, ignored in other builds
 * \param[in]	i2CStopOn_win32 - only for CONFIG_HOST_OS_WIN32, ignored in other builds
 * \return		>=0 - success, the number of bytes read
 * \return		-1 - read failure or system not configured
 * \callgraph
 * \callergraph
 */
tS32 CMOST_ReadRaw (tVoid *devicehandler_win32, tU8 *buf, tU32 len, tU32 deviceID, tBool i2CStartOn_win32, tBool i2CStopOn_win32)
{
	tS32 ret = -1;

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	ret = BSP_Read_CMOST(deviceID, buf, len);

	if (ret < 0)
	{
		ret = -1;
	}

	return ret;
#elif (defined CONFIG_HOST_OS_WIN32)
	tBool useI2c;
	tU8 i2cAdr;

	useI2c = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ? TRUE : FALSE;
	i2cAdr = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ?
			((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress :
			deviceID; // not used in SPI case

	ret = CMOST_ReadData (devicehandler_win32, buf, len, useI2c, i2cAdr, i2CStartOn_win32, i2CStopOn_win32);

	if (ret < 0)
	{
		return -1;
	}
	return ret;
#endif // #if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
}

/***************************
 *
 * CMOST_WriteRaw
 *
 **************************/
/*!
 * \brief		Writes a Raw buffer to the CMOST through I2C or SPI bus to an arbitrary address
 * \details		This function can be called before the CMOST firmware has been started.
 * \remark		Must be called after #BSP_BusConfig_CMOST
 * \remark		If #CMOST_DEBUG_DUMP_TO_FILE is defined the function also dumps the data
 * 				(not the addresses) to a file.
 * \param[in]	devicehandler_win32 - only for CONFIG_HOST_OS_WIN32, ignored in other builds
 * \param[in]	buf - address of a buffer containing the data to be written
 * \param[in]	len - size of the *buf* in bytes
 * \param[in]	useI2c - TRUE if the device is on the I2C bus, FALSE if it is on the SPI bus
 * \param[in]	i2cAdr - the device's physical address
 * \return		0 - success
 * \return		-1 - write failure or system not configured
 * \callgraph
 * \callergraph
 */
tS32 CMOST_WriteRaw (tVoid *devicehandler_win32, tU8 *buf, tU32 len, tU32 deviceID)
{
	tS32 ret = -1;

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	ret = BSP_Write_CMOST(deviceID, buf, len);
	if (ret < 0)
	{
		ret = -1;
		goto exit;
	}
	ret = 0;

	CMOST_tracePrintComponent(TR_CLASS_CMOST,"CMOST_WriteRaw wrting %d bytes\n", len);

#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_COMPONENT)
	COMMON_tracePrintBufComponent(TR_CLASS_CMOST, buf, len, NULL);
#endif

#if (defined CMOST_DEBUG_DUMP_TO_FILE)
	/* skip the address bytes */

	if (TUNERDRIVER_GetBusType(deviceID) == BusI2C)
	{
		if ((CMOST_DebugDump_fd != NULL) && (fwrite(buf+3, 1, len-3, CMOST_DebugDump_fd) < 0))
		{
			CMOST_tracePrintError(TR_CLASS_CMOST, "writing dump to file: %s", strerror(errno));
		}
	}
	else
	{
		if ((CMOST_DebugDump_fd != NULL) && (fwrite(buf+4, 1, len-4, CMOST_DebugDump_fd) < 0))
		{
			CMOST_tracePrintError(TR_CLASS_CMOST, "writing dump to file: %s", strerror(errno));
		}
	}
#endif // #if (defined CMOST_DEBUG_DUMP_TO_FILE)
#elif (defined CONFIG_HOST_OS_WIN32)
	tBool useI2c;
	tU8 i2cAdr;

	useI2c = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ? TRUE : FALSE;
	i2cAdr = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ?
			((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress :
			0xFF; // not used in SPI

	if (CMOST_WriteData (devicehandler_win32, buf, len, useI2c, i2cAdr) == len)
	{
		ret = 0;
	}
#endif // #if (defined CONFIG_HOST_OS_LINUX)

exit:
	return ret;
}

/***************************
 *
 * CMOST_ResetDevice
 *
 **************************/
/*!
 * \brief		Issues a hardware reset to the CMOST device
 * \param[in]	devicehandler_win32 - only for CONFIG_HOST_OS_WIN32, ignored in other builds
 * \param[in]	useI2c_win32 - TRUE if the device is on the I2C bus, FALSE if it is on the SPI bus
 * \param[in]	i2cAdr_win32 - the device's physical address
 * \callgraph
 * \callergraph
 */
tVoid CMOST_ResetDevice (tVoid *devicehandler_win32, tU32 deviceID)
{
#if defined (CONFIG_HOST_OS_WIN32)
	tBool useI2c_win32;
	tU8 i2cAdr_win32;

	useI2c_win32 = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ? TRUE : FALSE;
	i2cAdr_win32 = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ?
			((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress :
			deviceID; // not used in SPI case

    FT_STATUS ftStatus;

    if (true == useI2c_win32)
    {
        ftStatus = FTDI_ResetI2CDevice (devicehandler_win32, i2cAdr_win32);
    }
    else
    {
        ftStatus = FTDI_ResetSPIDevice (devicehandler_win32);
    }
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	(LINT_IGNORE_RET)BSP_DeviceReset_CMOST(deviceID);
#endif
}



// Function for full duplex data exchange
tU32 CMOST_WriteReadData (tVoid *devicehandler, tU8 *sourceBufferPnt, tU16 bytesToWrite, tU8 *destinationBufferPnt, tU16 bytesToRead, tU32 deviceID, tBool commkeptActive)
{
	tU32 readenBytes = 0;
#if defined (CONFIG_HOST_OS_WIN32)
	tBool useI2c;
	tU8 i2cAdr;

	useI2c = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ? TRUE : FALSE;
	i2cAdr = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ?
			((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress :
			deviceID; // not used in SPI case
	
	if (false == useI2c)
	{
		readenBytes = CMOST_SpiWriteSeq ((FT_HANDLE)devicehandler, (tU8 *)sourceBufferPnt, bytesToWrite, destinationBufferPnt, i2cAdr, commkeptActive);
	}
	else
	{
		// not used in I2C
	}
#elif defined(CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)

	if (TUNERDRIVER_GetBusType(deviceID) == BusSPI)
	{
		readenBytes = CMOST_SpiWriteSeq(deviceID, (tU8 *)sourceBufferPnt, bytesToWrite, destinationBufferPnt, bytesToRead, commkeptActive);
	}
	else
	{
		// not used in I2C
		CMOST_tracePrintError(TR_CLASS_CMOST,"invalid configuration for device ID %d", deviceID);
	}

#endif
	return readenBytes;
}

#if defined (CONFIG_HOST_OS_WIN32)

//Write the specified buffer of size = bytesToWrite.
//returns writtenBytes (Return values of 0 means error)
static tU32 CMOST_WriteData (tVoid *devicehandler, tU8 *sourceBufferPnt, tU32 bytesToWrite, tBool useI2c, tU8 i2cAdr) 
{
	#define RX_TMP_BUFFER_SIZE			16192
    
	tU32 writtenBytes = 0;
    tVoid *rxBfrPtr;

    if (true == useI2c)
    {
        writtenBytes = CMOST_I2cWriteWithoutAckCheck ((FT_HANDLE)devicehandler, (tU8 *)sourceBufferPnt, bytesToWrite);

        if (writtenBytes != bytesToWrite)
        {
            // Error
            return 0;
        }
    }
	else
	{
		rxBfrPtr = malloc (RX_TMP_BUFFER_SIZE);

		if (NULL != rxBfrPtr)
		{
			writtenBytes = CMOST_SpiWriteSeq ((FT_HANDLE)devicehandler, (tU8 *)sourceBufferPnt, bytesToWrite, rxBfrPtr, i2cAdr, false);

			// Free resources
			free (rxBfrPtr);

			if (writtenBytes != bytesToWrite)
			{
				// Error
				return 0;
			}
		}
    }

    return writtenBytes;
}

// Read the specified amount of bytes to the specified buffer address. 
// Return value of 0 means error.
static tS32 CMOST_ReadData (tVoid *devicehandler, tU8 *destinationBufferPnt, tS32 bytesToRead, tBool useI2c, 
                            tU8 i2cAdr, tBool i2CStartOn, tBool i2CStopOn)
{
    tU32 readenBytes = 0;
    tBool isAcktested = true;
	tU8 *txBfrPtr;

	// Only I2C is using this mode
	if (false == useI2c)
	{
		return -1;
	}

	// Allocate space for the buffer
	txBfrPtr = (tU8 *)malloc ((bytesToRead + 1));

	// Sanity check
	if (NULL == txBfrPtr)
	{
		return -1;
	}

	// Set all to 0xFF
	OSAL_pvMemorySet (txBfrPtr, 0xFF, (bytesToRead + 1));

    if (true == i2CStartOn)
    {
		*txBfrPtr = (i2cAdr | 0x01);
    }

	// Send read request
	readenBytes = CMOST_I2cReadWithoutAckCheck ((FT_HANDLE)devicehandler, txBfrPtr, bytesToRead,
                                                (tU8 *)destinationBufferPnt, i2CStartOn, i2CStopOn);

	// Free resources
	if (NULL != txBfrPtr)
	{
		free (txBfrPtr);
	}

    return readenBytes;
}

static DWORD CMOST_SpiWriteSeq (tVoid *ftHandle, LPVOID lpBufferPnt, DWORD dwBytesToWrite, LPVOID RxBuffer, tU8 curspiCSline, tBool commkeptActive)
{
	#define GPIO_VAL_ALL_ONES		((tU8)0xFF)	
	#define GPIO_DIR				((tU8)0xDB)	// Leave GPIO 1 and MISO as input

	FT_STATUS ftStatus = FT_OTHER_ERROR;  // Status defined in D2XX to indicate operation result
	DWORD lpBytesWritten = 0;
	DWORD dwNumBytesRead = 0;
	DWORD dwNumBytesToRead = 0;
	tU8 *txBfrPtr;
	tU32 bufferCnt = 0;
	DWORD resultByteNum;
	tU8 *srcDataPtr = (tU8 *)lpBufferPnt;
	DWORD txByteNum;
	tBool errorDetected = false;
	tBool doStartOnce = true;
	DWORD incrementalWrittenBytesNum;
	tU32 startAddress;
	tU8 *tmpRxBufferPtr = (tU8 *)RxBuffer;
	tU32 cursize = 0;

	txByteNum = dwBytesToWrite;
	resultByteNum = dwBytesToWrite;
	incrementalWrittenBytesNum = 0;

	// We need to malloc a relevant amount of space depending on the incoming data, but we
	// cap @ FRAME_SIZE so we can use this as maximum allocation space
//    txBfrPtr = (tU8 *)malloc (FRAME_SIZE + 10 + 3);
    txBfrPtr = (tU8 *)malloc (FRAME_SIZE + 10 + 7);

	// Sanity check
	if (NULL == txBfrPtr)
	{
		return 0;
	}

	while (txByteNum > 0)
	{
		// We do not want to transmit more than 1024 data bytes @ once
		if (txByteNum > FRAME_SIZE)
		{
			if (true == doStartOnce)
			{
				txByteNum = FRAME_SIZE + 4;
			}
			else
			{
				txByteNum = FRAME_SIZE;
			}
		}

		// Lower CS line
		txBfrPtr[bufferCnt++] = 0x80;

		if (curspiCSline == 0)    //if selected CS line 
		{
			txBfrPtr[bufferCnt++] = GPIO_VAL_ALL_ONES  & (~(0x10 << FTDI_GPIO_CS)); // set CS line Low
		}
		else     //selected CS2 line
		{
			txBfrPtr[bufferCnt++] = GPIO_VAL_ALL_ONES  & (~(0x10 << FTDI_GPIO_CS2)); // set CS2 line Low
		}

		txBfrPtr[bufferCnt++] = GPIO_DIR;                              // 0 = in / 1 = out, set CSGPIO out, REQGPIO in

		// Send the data   only phase 11 for the moment
		txBfrPtr[bufferCnt++] = 0x30;                                  // 4 / MPSSE command to Write bytes in from SPI phase 11

		if (false == doStartOnce)
		{
			startAddress += (FRAME_SIZE / 4);
			txBfrPtr[bufferCnt++] = ((txByteNum + 4) - 1) & 0xFF;          // LSB Length
			txBfrPtr[bufferCnt++] = (((txByteNum + 4) - 1) & 0xFF00) >> 8; // MSB Length		
			txBfrPtr[bufferCnt++] = 0x98;
			txBfrPtr[bufferCnt++] = (startAddress >> 16) & 0xFF;
			txBfrPtr[bufferCnt++] = (startAddress >> 8) & 0xFF;
			txBfrPtr[bufferCnt++] = (startAddress >> 0) & 0xFF;
		}
		else
		{
			txBfrPtr[bufferCnt++] = (txByteNum - 1) & 0xFF;          // LSB Length
			txBfrPtr[bufferCnt++] = ((txByteNum - 1) & 0xFF00) >> 8; // MSB Length		
		}

		memcpy ((void *)&txBfrPtr[bufferCnt], (const void *)srcDataPtr, txByteNum); srcDataPtr += txByteNum;

		if (true == doStartOnce)
		{
			startAddress = ((*(&txBfrPtr[bufferCnt] + 1) << 16) & (tU32)0x00FF0000) |
				((*(&txBfrPtr[bufferCnt] + 2) << 8)  & (tU32)0x0000FF00) |
				((*(&txBfrPtr[bufferCnt] + 3) << 0)  & (tU32)0x000000FF);

			doStartOnce = false;
		}

		bufferCnt += txByteNum;

		if (false == commkeptActive)
		{
			// Raise the CS line
			txBfrPtr[bufferCnt++] = 0x80;

			if (curspiCSline == 0)    //if selected CS line 
			{
				txBfrPtr[bufferCnt++] = GPIO_VAL_ALL_ONES | (0x10 << FTDI_GPIO_CS);     // set CS line High
			}
			else
			{
				txBfrPtr[bufferCnt++] = GPIO_VAL_ALL_ONES | (0x10 << FTDI_GPIO_CS2);     // set CS2 line High
			}

			txBfrPtr[bufferCnt++] = GPIO_DIR;                              // 0 = in / 1 = out, set CSGPIO out, REQGPIO in	
		}

		// Write command
		ftStatus = p_FT_Write (ftHandle, &txBfrPtr[0], bufferCnt, (LPDWORD)&lpBytesWritten);

		// Wait until data is available
		CMOST_WaitIncomingData (ftHandle, txByteNum);

        dwNumBytesToRead = txByteNum;

		// Check the receive buffer 
		if (NULL != tmpRxBufferPtr && 0 != dwNumBytesToRead)
		{
			ftStatus = p_FT_Read (ftHandle, tmpRxBufferPtr, dwNumBytesToRead, (LPDWORD)&dwNumBytesRead);
		}

		// Restart from output buffer start
		bufferCnt = 0;

		if (true == commkeptActive)
		{
			if (*(tmpRxBufferPtr + 6) != 0)
			{
				// we need to read also the checksum
				// 
				cursize = (3 * (*(tmpRxBufferPtr + 6))) & 0x3FF + CMOST_CRC_LEN ;

				// Send the data   only phase 11 for the moment
				txBfrPtr[bufferCnt++] = 0x30;                           // 4 / MPSSE command to Write bytes in from SPI phase 11
				txBfrPtr[bufferCnt++] = (cursize - 1) & 0xFF;           // LSB Length
				txBfrPtr[bufferCnt++] = ((cursize - 1) & 0xFF00) >> 8; // MSB Length

				for (tU16 cnt = 0; cnt < cursize; cnt++)
				{
					txBfrPtr[bufferCnt++] = 0xFF;
				}

				resultByteNum += cursize;
			}

			// Raise the CS line ----------------------------------------------------------------
			txBfrPtr[bufferCnt++] = 0x80;

			if (curspiCSline == 0)    //if selected CS line 
			{
				txBfrPtr[bufferCnt++] = GPIO_VAL_ALL_ONES | (0x10 << FTDI_GPIO_CS);     // set CS line High
			}
			else
			{
				txBfrPtr[bufferCnt++] = GPIO_VAL_ALL_ONES | (0x10 << FTDI_GPIO_CS2);     // set CS2 line High
			}

			txBfrPtr[bufferCnt++] = GPIO_DIR;

			// Write command
			ftStatus = p_FT_Write (ftHandle, &txBfrPtr[0], bufferCnt, (LPDWORD)&lpBytesWritten);

			// Wait until data is available
			CMOST_WaitIncomingData (ftHandle, cursize);

            dwNumBytesToRead = cursize;

			// Check the receive buffer 
			if (NULL != tmpRxBufferPtr && 0 != dwNumBytesToRead)
			{
				ftStatus = p_FT_Read (ftHandle, (LPVOID)(tmpRxBufferPtr + 7), dwNumBytesToRead, (LPDWORD)&dwNumBytesRead);
			}
		}

		if (FT_OK != ftStatus)
		{
			errorDetected = true;
		}

		incrementalWrittenBytesNum += txByteNum;

		// Now we subtract the TX data num
		dwBytesToWrite -= txByteNum;

		// Restart TX
		txByteNum = dwBytesToWrite;
	}

	if (FT_OK != ftStatus)
	{
		errorDetected = true;
	}

	if (true == errorDetected)
	{
		resultByteNum = 0;
	}

	// Flush and reset
	FTDI_PurgeDevice (ftHandle);

	// Free resources
	if (NULL != txBfrPtr)
	{
		free (txBfrPtr);
	}

    return resultByteNum;
}

static DWORD CMOST_I2cWriteWithAckCheck (tVoid *handle, LPVOID lpBufferPnt, DWORD dwBytesToWrite, LPVOID rxBufferPtr)
{
	#define GPIO_VAL				((tU8)0xF3)	
	#define GPIO_DIR				((tU8)0xDB)	// Leave GPIO 1 and MISO as input

    FT_STATUS ftStatus;  // Status defined in D2XX to indicate operation result
    DWORD lpBytesWritten = 0;
	tU32 bufferCnt = 0;
	tU8 *txBfrPtr;
	tU8 *tmpRxBufferPtr = (tU8 *)rxBufferPtr;
	tU32 dwCount;
    DWORD dwNumBytesRead;
    DWORD dwNumBytesSent;
    DWORD resultByteNum;
    tU8 *dataPtr = (tU8 *)lpBufferPnt;
	tBool errorDetected = false;
   
    resultByteNum = dwBytesToWrite;

	// We need to malloc a relevant amount of space depending on the incoming data, but we
	// cap @ FRAME_SIZE so we can use this as maximum allocation space
	txBfrPtr = (tU8 *)malloc ((12 + 12 + 3 + (10 * dwBytesToWrite)));

	// Sanity check
	if (NULL == txBfrPtr)
	{
		return 0;
	}

    // Send I2C start seq  1) set SDA, SCL high 2) set SDA low, SCL high 3) set SDA, SCL low ------------------------------
    for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum start hold time =600ns)
    {
		txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF3;     // Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
		txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
    }

    for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum start setup time = 600ns)
    {
		txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF1;     // Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
    }

	txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
	txBfrPtr[bufferCnt++] = 0xF0;     // Set SDA, SCL low, WP disabled by SK, DO, GPIOL0 at bit 0
	txBfrPtr[bufferCnt++] = GPIO_DIR; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0

    for (dwCount = 0; dwCount < dwBytesToWrite; dwCount++) // send all the bytes of the buffer
    {
        // Send I2C Data Byte and check ACK bit (return true if data successfully sent and ACK bit ok. -------------------------- 
        // Return false if error during sending data or ACK bit can't be received
		txBfrPtr[bufferCnt++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT; //Clock data byte out on Clock Edge MSB first
		txBfrPtr[bufferCnt++] = 0x00;
		txBfrPtr[bufferCnt++] = 0x00; //Data length of 0x0000 means 1 byte data to clock out
		txBfrPtr[bufferCnt++] = *dataPtr; dataPtr++; //tmpBuffer[dwCount]; //Add data to be sent
            
        //Get Acknowledge bit from EEPROM
		txBfrPtr[bufferCnt++] = 0x80; // Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xFE;  // 0xF0; //Set SCL low, WP disabled by SK, GPIOL0 at bit 0
		txBfrPtr[bufferCnt++] = GPIO_DIR & (~(0x02));  // 0x11; //Set SK, GPIOL0 pins as output with bit 1, DO and other pins as input with bit 0
		txBfrPtr[bufferCnt++] = MSB_RISING_EDGE_CLOCK_BIT_IN; //Command to scan in ACK bit , clock Edge MSB first
		txBfrPtr[bufferCnt++] = 0x00; // Length of 0x0 means to scan in 1 bit
		txBfrPtr[bufferCnt++] = 0x87; // Send answer back immediate command
            
        // Send out data
		ftStatus = p_FT_Write (handle, txBfrPtr, bufferCnt, &dwNumBytesSent);

        if (FT_OK != ftStatus)
        {
            errorDetected = true;
        }
                
        // Restart from output buffer start
        bufferCnt = 0; 

        //Check if ACK bit received, may need to read more times to get ACK bit or fail if timeout
		ftStatus = p_FT_Read (handle, (LPVOID)tmpRxBufferPtr, 1, &dwNumBytesRead); //Read one byte from device receive buffer

        if (FT_OK != ftStatus)
        {
            errorDetected = true;
        }
                
        if ((ftStatus != FT_OK) || (dwNumBytesRead == 0))
        {
            return 0;  // Error, can't get the ACK bit from the I2C peripheral 
        }
		else if (((*tmpRxBufferPtr & BYTE (0x01)) != BYTE (0x00))) //Check ACK bit 0 on data byte read out
        {
            return 0; // Error, can't get the ACK bit from EEPROM
        }

		txBfrPtr[0] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[1] = 0xF2;     // Set SDA high, SCL low, WP disabled by SK at bit '0', DO, GPIOL0 at bit '1'
		txBfrPtr[2] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0

        // Write command
		ftStatus = p_FT_Write (handle, &txBfrPtr[0], bufferCnt, (LPDWORD)&lpBytesWritten);

        if (FT_OK != ftStatus)
        {
            errorDetected = true;
        }
    }

    // Restart from output buffer begin
    bufferCnt = 0;

    // Send I2C STOP seq 1) set SDA Low, SCL Hi 2) set SDA, SCL Hi 3) set SDA, SCL as input to tristate I2C bus ---------
    for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum setup time =600ns )
    {
		txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF1;     // Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
    }

    for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum hold time =600ns)
    {
		txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF3;     // Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
		txBfrPtr[bufferCnt++] = GPIO_DIR; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
    }

    // Tristate the SCL, SDA pins
	txBfrPtr[bufferCnt++] = 0x80;                  // Command to set directions of lower 8 pins and force value on bits set as output		
	txBfrPtr[bufferCnt++] = 0xF3;                  // 0xF0 //Set WP disabled by GPIOL0 at bit 0
	txBfrPtr[bufferCnt++] = GPIO_DIR & (~(0x03));  //0x10 Set GPIOL0 pins as output with bit 1, SK, DO and other pins as input with bit 0
 
    // Write command
	ftStatus = p_FT_Write (handle, &txBfrPtr[0], bufferCnt, (LPDWORD)&lpBytesWritten);

    if (FT_OK != ftStatus)
    {
        errorDetected = true;
    }

    if (true == errorDetected)
    {
        resultByteNum = 0;
    }

    return resultByteNum;
}

static DWORD CMOST_I2cWriteWithoutAckCheck (tVoid *handle, LPVOID lpBufferPnt, DWORD dwBytesToWrite)
{
	#define GPIO_VAL				((tU8)0xF3)	
	#define GPIO_DIR				((tU8)0xDB)	// Leave GPIO 1 and MISO as input

    FT_STATUS ftStatus;  // Status defined in D2XX to indicate operation result
    DWORD lpBytesWritten = 0;
	tU32 bufferCnt = 0;
	tU8 *txBfrPtr;
	tU32 dwCount;
    DWORD resultByteNum;
    tU8 *dataPtr = (tU8 *)lpBufferPnt;
    tU32 txByteNum;
	tBool errorDetected = false;
	tBool doStartOnce = true;
    DWORD dwNumBytesToRead;
    DWORD incrementalWrittenBytesNum;

    txByteNum = dwBytesToWrite;
    resultByteNum = dwBytesToWrite;

    incrementalWrittenBytesNum = 0;
    dwNumBytesToRead = 0;

	// We need to malloc a relevant amount of space depending on the incoming data, but we
	// cap @ FRAME_SIZE so we can use this as maximum allocation space
	if (txByteNum > FRAME_SIZE)
	{
		txByteNum = FRAME_SIZE;
	}

	// Allocate memory
	txBfrPtr = (tU8 *)malloc (((txByteNum * 13) + 27));

	// Sanity check
	if (NULL == txBfrPtr)
	{
		return 0;
	}

    while (txByteNum > 0)
    {
        if (true == doStartOnce)
        {
            // Send I2C start seq  1) set SDA, SCL high 2) set SDA low, SCL high 3) set SDA, SCL low ------------------------------
            for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum start hold time =600ns)
            {
				txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
				txBfrPtr[bufferCnt++] = 0xF3;     // Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
				txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
            }

            for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum start setup time = 600ns)
            {
				txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
				txBfrPtr[bufferCnt++] = 0xF1;     // Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
				txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
            }

			txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF0;     // Set SDA, SCL low, WP disabled by SK, DO, GPIOL0 at bit 0
			txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0

            doStartOnce = false;
        }

        for (dwCount = 0; dwCount < txByteNum; dwCount++) // send all the bytes of the buffer
        {
            // Send I2C Data Byte and check ACK bit (return true if data successfully sent and ACK bit ok. -------------------------- 
            // Return false if error during sending data or ACK bit can't be received
			txBfrPtr[bufferCnt++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT; //Clock data byte out on –ve Clock Edge MSB first
			txBfrPtr[bufferCnt++] = 0x00;
			txBfrPtr[bufferCnt++] = 0x00;                // Data length of 0x0000 means 1 byte data to clock out
			txBfrPtr[bufferCnt++] = *dataPtr; dataPtr++; // tmpBuffer[dwCount]; //Add data to be sent

            //Generate 9^ clock (Acknowledge bit)
			txBfrPtr[bufferCnt++] = 0x80;                         // Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xFE;                         //Set SCL low, WP disabled by SK, GPIOL0 at bit 0
			txBfrPtr[bufferCnt++] = GPIO_DIR & (~(0x02));         // Set SK, GPIOL0 pins as output with bit 1, DO and other pins as input with bit 0
			txBfrPtr[bufferCnt++] = MSB_RISING_EDGE_CLOCK_BIT_IN; // Command to scan in ACK bit , clock Edge MSB first
			txBfrPtr[bufferCnt++] = 0x00;                         // Length of 0x0 means to scan in 1 bit
			txBfrPtr[bufferCnt++] = 0x87;                         // Send answer back immediate command

            // In case the ack is not required for each byte we write here
			txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF2;     // Set SDA high, SCL low, WP disabled by SK at bit '0', DO, GPIOL0 at bit '1'
			txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
        }

        // Write command
		ftStatus = p_FT_Write (handle, &txBfrPtr[0], bufferCnt, (LPDWORD)&lpBytesWritten);

        // Restart from output buffer start
        bufferCnt = 0;

        if (FT_OK != ftStatus)
        {
            errorDetected = true;
        }
        
        incrementalWrittenBytesNum += txByteNum;

		// Wait until data is available
		CMOST_WaitIncomingData (handle, incrementalWrittenBytesNum);

        // Now we subtract the TX data num
        dwBytesToWrite -= txByteNum;

        // Restart TX
        txByteNum = dwBytesToWrite;

		// We do not want to transmit more than 256 of data @ once
		if (txByteNum > FRAME_SIZE)
		{
			txByteNum = FRAME_SIZE;
		}
    }

    // Restart from output buffer begin
    bufferCnt = 0;

    // Send I2C STOP seq 1) set SDA Low, SCL Hi 2) set SDA, SCL Hi 3) set SDA, SCL as input to tristate I2C bus
    for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum setup time =600ns )
    {
		txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF1;     // Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
		txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
    }

    for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum hold time =600ns)
    {
		txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF3;     // Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
		txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
    }

    // Tristate the SCL, SDA pins
	txBfrPtr[bufferCnt++] = 0x80;                  // Command to set directions of lower 8 pins and force value on bits set as output		
	txBfrPtr[bufferCnt++] = 0xF3;                  // 0xF0 //Set WP disabled by GPIOL0 at bit 0
	txBfrPtr[bufferCnt++] = GPIO_DIR & (~(0x03));  // 0x10 Set GPIOL0 pins as output with bit 1, SK, DO and other pins as input with bit 0

    // Write command
	ftStatus = p_FT_Write (handle, &txBfrPtr[0], bufferCnt, (LPDWORD)&lpBytesWritten);

    if (FT_OK != ftStatus)
    {
        errorDetected = true;
    }

    if (true == errorDetected)
    {
        resultByteNum = 0;
    }

    // Flush and reset
    FTDI_PurgeDevice (handle);

	// Free resources
	if (NULL != txBfrPtr)
	{
		free (txBfrPtr);
	}

    return resultByteNum;
}

static DWORD CMOST_I2cReadWithAckCheck (tVoid * handle, LPVOID lpBufferPnt, DWORD dwBytesToRead, LPVOID RxBuffer)
{
	#define GPIO_VAL				((tU8)0xF3)	
	#define GPIO_DIR				((tU8)0xDB)	// Leave GPIO 1 and MISO as input

    FT_STATUS ftStatus;  // Status defined in D2XX to indicate operation result
    DWORD lpBytesReaden = 0;
    DWORD lpBytesWritten = 0;
    DWORD dwNumBytesRead;
    DWORD dwNumBytesToRead;
    DWORD dwNumBytesSent;
	tU32 bufferCnt = 0;
	tU8 *txBfrPtr;
	tU8 *tmpBfrPtr;
	tU8 rxBuff[16];
	tU32 dwCount;

	// We need to malloc a relevant amount of space, this shall be optimized
	txBfrPtr = (tU8 *)malloc (TX_BUFFER_TOTAL_LEN_BYTES);
	tmpBfrPtr = (tU8 *)malloc (TX_BUFFER_TOTAL_LEN_BYTES);

	// Sanity check
	if (NULL == txBfrPtr || NULL == tmpBfrPtr)
	{
		if (NULL != txBfrPtr)
		{
			free (txBfrPtr);
		}

		if (NULL != tmpBfrPtr)
		{
			free (tmpBfrPtr);
		}

		return 0;
	}

    if (dwBytesToRead > 0)
    {
		///
		// I2C start seq  1) set SDA, SCL high 2) set SDA low, SCL high 3) set SDA, SCL low
		///
        for (dwCount = 0; dwCount < 4; dwCount++)		// Repeat 4 times commands (minimum start hold time =600ns)
        {
			txBfrPtr[bufferCnt++] = 0x80;				// Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF3;				// Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
			txBfrPtr[bufferCnt++] = GPIO_DIR;			// Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
        }

        for (dwCount = 0; dwCount < 4; dwCount++)		// Repeat 4 times commands (minimum start setup time = 600ns)
        {
			txBfrPtr[bufferCnt++] = 0x80;				// Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF1;				// Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
			txBfrPtr[bufferCnt++] = GPIO_DIR;			// Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
        }

		txBfrPtr[bufferCnt++] = 0x80;					// Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF0;					// Set SDA, SCL low, WP disabled by SK, DO, GPIOL0 at bit 0
		txBfrPtr[bufferCnt++] = GPIO_DIR;				// Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0

		///
        // Send I2C address Byte (read mode) and check ACK bit (return true if data successfully sent and ACK bit ok. 
        // Return false if error during sending data or ACK bit can't be received
		///
		memcpy ((void *)&tmpBfrPtr[0], (const void *)lpBufferPnt, dwBytesToRead);

		txBfrPtr[bufferCnt++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT; // 0x11 Clock data byte out on –ve Clock Edge MSB first
		txBfrPtr[bufferCnt++] = 0x00;
		txBfrPtr[bufferCnt++] = 0x00;							// Data length of 0x0000 means 1 byte data to clock out
		txBfrPtr[bufferCnt++] = tmpBfrPtr[0];					// Add chip address +1 value (read mode I2C chip address)

        // Get Acknowledge bit from EEPROM
		txBfrPtr[bufferCnt++] = 0x80;							// Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF0;							// Set SCL low, WP disabled by SK, GPIOL0 at bit 0
		txBfrPtr[bufferCnt++] = GPIO_DIR & (~(0x02));			// 0x11; // Set SK, GPIOL0 pins as output with bit 1, DO and other pins as input with bit 0
		txBfrPtr[bufferCnt++] = MSB_RISING_EDGE_CLOCK_BIT_IN;	// 0x22  Command to scan in ACK bit , clock Edge MSB first
		txBfrPtr[bufferCnt++] = 0x00;							// Length of 0x0 means to scan in 1 bit
		txBfrPtr[bufferCnt++] = 0x87;							// Send answer back immediate command

		ftStatus = p_FT_Write (handle, txBfrPtr, bufferCnt, &dwNumBytesSent); //Send off the commands
        bufferCnt = 0;											// Clear output buffer

        // Check if ACK bit received, may need to read more times to get ACK bit or fail if timeout
        ftStatus = p_FT_Read (handle, rxBuff, 1, &dwNumBytesRead); // Read one byte from device receive buffer

        if ((ftStatus != FT_OK) || (dwNumBytesRead == 0))
        {
			// Free resources
			if (NULL != txBfrPtr)
			{
				free (txBfrPtr);
			}

			if (NULL != tmpBfrPtr)
			{
				free (tmpBfrPtr);
			}

            return 0;  // Error, can't get the ACK bit from the I2C peripheral 
        }
        else if (((rxBuff[0] & BYTE (0x01)) != BYTE (0x00))) //Check ACK bit 0 on data byte read out
        {
			// Free resources
			if (NULL != txBfrPtr)
			{
				free (txBfrPtr);
			}

			if (NULL != tmpBfrPtr)
			{
				free (tmpBfrPtr);
			}

            return 0; // Error, can't get the ACK bit from EEPROM
        }

		txBfrPtr[bufferCnt++] = 0x80;						// Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF0;						// Set SDA low, SCL low, WP disabled by SK at bit '0', DO, GPIOL0 at bit '1'
		txBfrPtr[bufferCnt++] = GPIO_DIR;					// Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0

        for (dwCount = 0; dwCount < dwBytesToRead; dwCount++) // send all the bytes of the buffer
        {
            // Read the data from I2C peripheral with no ACK bit check ----------------------
			txBfrPtr[bufferCnt++] = 0x80;                 // Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF0;                 // Set SCL low, WP disabled by SK, GPIOL0 at bit 
			txBfrPtr[bufferCnt++] = GPIO_DIR & (~(0x02)); // 0x11; // Set SK, GPIOL0 pins as output with bit , DO and other pins as input with bit 
   
			txBfrPtr[bufferCnt++] = MSB_RISING_EDGE_CLOCK_BYTE_IN; // 0x20 Command to clock data byte in on +ve Clock Edge MSB first
			txBfrPtr[bufferCnt++] = 0x00;
			txBfrPtr[bufferCnt++] = 0x00;                 // Data length of 0x0000 means 1 byte data to clock in

            if (dwCount < (dwBytesToRead - 1))
            {
                // Master send ack bit with SDA forced low to slave device
				txBfrPtr[bufferCnt++] = 0x80;             // Command to set directions of lower 8 pins and force value on bits set as output
				txBfrPtr[bufferCnt++] = 0xF0;             // Set SDA, SCL low, WP disabled by SK, DO, GPIOL0 at bit 0			
				txBfrPtr[bufferCnt++] = GPIO_DIR;         // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
            }

			txBfrPtr[bufferCnt++] = MSB_RISING_EDGE_CLOCK_BIT_IN; // 0x22 Command to scan in acknowledge bit , +ve clock Edge MSB first
			txBfrPtr[bufferCnt++] = 0x00;                         // Length of 0 means to scan in 1 bit
			txBfrPtr[bufferCnt++] = 0x87;                         // Send answer back immediate command

			ftStatus = p_FT_Write (handle, txBfrPtr, bufferCnt, &dwNumBytesSent); //Send off the commands
            bufferCnt = 0; // Clear output buffer

			// Wait until data is available
			CMOST_WaitIncomingData (handle, 2);

            // Read two bytes from device receive buffer, first byte is data read from EEPROM, second byte is ACK bit
            ftStatus = p_FT_Read (handle, rxBuff, 2, &dwNumBytesRead);

			tmpBfrPtr[dwCount] = rxBuff[0]; // Return the data read from EEPROM

			txBfrPtr[bufferCnt++] = 0x80;		// Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF2;		// Set SDA high, SCL low, WP disabled by SK at bit '0', DO, GPIOL0 at bit '1'
			txBfrPtr[bufferCnt++] = GPIO_DIR;	// Set SK,DO,GPIOL0 pins as output with bit , other pins as input with bit 
        }

		txBfrPtr[bufferCnt++] = 0x80;			// Command to set directions of lower 8 pins and force value on bits set as output
		txBfrPtr[bufferCnt++] = 0xF0;			// Set SDA low, SCL low, WP disabled by SK at bit '0', DO, GPIOL0 at bit '1'
		txBfrPtr[bufferCnt++] = GPIO_DIR;		// 0x13  //Set SK,DO,GPIOL0 pins as output with bit , other pins as input with bit 

		///
		// I2C STOP seq 1) set SDA Low, SCL Hi 2) set SDA, SCL Hi 3) set SDA, SCL as input to tristate I2C bus
		///
        for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum setup time =600ns )
        {
			txBfrPtr[bufferCnt++] = 0x80;		// Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF1;		// Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
			txBfrPtr[bufferCnt++] = GPIO_DIR;	// Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
        }

        for (dwCount = 0; dwCount<4; dwCount++) // Repeat 4 times commands (minimum hold time =600ns)
        {
			txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF3;     // Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
			txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
        }

		ftStatus = p_FT_Write (handle, txBfrPtr, bufferCnt, &dwNumBytesSent); // Send off the commands
        bufferCnt = 0; // Clear output buffer

		memcpy ((tVoid *)RxBuffer, (tVoid *)&tmpBfrPtr[0], dwBytesToRead);
        lpBytesReaden = dwBytesToRead;
    }
    else
    {
		// Get queue status
		p_FT_GetQueueStatus (handle, &dwNumBytesToRead);

        // Check the receive buffer 
        if (NULL != RxBuffer && 0 != dwNumBytesToRead)
        {
            ftStatus = p_FT_Read (handle, (tU8 *)RxBuffer, dwNumBytesToRead, (LPDWORD)&dwNumBytesRead);
        }

        ftStatus = p_FT_Purge (handle, FT_PURGE_RX | FT_PURGE_TX);
    }

	// Free resources
	if (NULL != txBfrPtr)
	{
		free (txBfrPtr);
	}

	if (NULL != tmpBfrPtr)
	{
		free (tmpBfrPtr);
	}

    return lpBytesReaden;
}

static DWORD CMOST_I2cReadWithoutAckCheck (tVoid * handle, LPVOID lpBufferPnt, DWORD dwBytesToRead, LPVOID RxBuffer, tBool i2CStartOn, tBool i2CStopOn)
{
	#define GPIO_VAL				((tU8)0xF3)	
	#define GPIO_DIR				((tU8)0xDB)	// Leave GPIO 1 and MISO as input

    FT_STATUS ftStatus;  //Status defined in D2XX to indicate operation result
    DWORD lpBytesReaden = 0;
    DWORD lpBytesWritten = 0;
    DWORD dwNumBytesRead;
    DWORD dwNumBytesToRead;
    DWORD dwNumBytesSent;
	tU32 bufferCnt = 0;
	tU8 *txBfrPtr;
	tU8 *tmpBfrPtr;
    tU8 rxBuff[16];
    tU32 dwCount;

	// We need to malloc a relevant amount of space, this shall be optimized
	txBfrPtr = (tU8 *)malloc (TX_BUFFER_TOTAL_LEN_BYTES);
	tmpBfrPtr = (tU8 *)malloc (TX_BUFFER_TOTAL_LEN_BYTES);

	// Sanity check
	if (NULL == txBfrPtr || NULL == tmpBfrPtr)
	{
		if (NULL != txBfrPtr)
		{
			free (txBfrPtr);
		}

		if (NULL != tmpBfrPtr)
		{
			free (tmpBfrPtr);
		}

		return 0;
	}

    if (dwBytesToRead > 0)
    {
        if (true == i2CStartOn)
        {
            // send I2C start seq  1) set SDA, SCL high 2) set SDA low, SCL high 3) set SDA, SCL low
            for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum start hold time =600ns)
            {
				txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
				txBfrPtr[bufferCnt++] = 0xF3;     // Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
				txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
            }

            for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum start setup time = 600ns)
            {
				txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
				txBfrPtr[bufferCnt++] = 0xF1;     // Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
				txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
            }

			txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF0;     // Set SDA, SCL low, WP disabled by SK, DO, GPIOL0 at bit 0
			txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0

            // Send I2C address Byte (read mode) and check ACK bit (return true if data successfully sent and ACK bit ok.
            // Return false if error during sending data or ACK bit can't be received
			memcpy ((void *)&tmpBfrPtr[0], (const void *)lpBufferPnt, dwBytesToRead);

			txBfrPtr[bufferCnt++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT; // 0x11 Clock data byte out on Clock Edge MSB first
			txBfrPtr[bufferCnt++] = 0x00;
			txBfrPtr[bufferCnt++] = 0x00; //Data length of 0x0000 means 1 byte data to clock out
			txBfrPtr[bufferCnt++] = tmpBfrPtr[0]; //Add chip address +1 value (read mode I2C chip address)

            //Get Acknowledge bit from EEPROM
			txBfrPtr[bufferCnt++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF0; //Set SCL low, WP disabled by SK, GPIOL0 at bit 0
			txBfrPtr[bufferCnt++] = GPIO_DIR & (~(0x02));    //0x11; //Set SK, GPIOL0 pins as output with bit 1, DO and other pins as input with bit 0
			txBfrPtr[bufferCnt++] = MSB_RISING_EDGE_CLOCK_BIT_IN; //0x22  Command to scan in ACK bit , clock Edge MSB first
			txBfrPtr[bufferCnt++] = 0x00; //Length of 0x0 means to scan in 1 bit
			txBfrPtr[bufferCnt++] = 0x87; //Send answer back immediate command

            //test ack of chipaddr+1 
			ftStatus = p_FT_Write (handle, txBfrPtr, bufferCnt, &dwNumBytesSent); //Send off the commands
            bufferCnt = 0; //Clear output buffer

            //Check if ACK bit received, may need to read more times to get ACK bit or fail if timeout
            ftStatus = p_FT_Read (handle, rxBuff, 1, &dwNumBytesRead); //Read one byte from device receive buffer

            if ((ftStatus != FT_OK) || (dwNumBytesRead == 0))
            {
                return 0;  //Error, can't get the ACK bit from the I2C peripheral 
            }
            else if (((rxBuff[0] & BYTE(0x01)) != BYTE(0x00))) //Check ACK bit 0 on data byte read out
            {
                return 0; /*Error, can't get the ACK bit from EEPROM */
            }

			txBfrPtr[bufferCnt++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF0; //Set SDA low, SCL low, WP disabled by SK at bit '0', DO, GPIOL0 at bit '1'
			txBfrPtr[bufferCnt++] = GPIO_DIR; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
        }

        //	dwBytesToRead =1;
        for (dwCount = 0; dwCount < dwBytesToRead; dwCount++) // send all the bytes of the buffer
        {
            // Read the data from I2C peripheral with no ACK bit check ----------------------
			txBfrPtr[bufferCnt++] = 0x80; //Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF0; //Set SCL low, WP disabled by SK, GPIOL0 at bit 
			txBfrPtr[bufferCnt++] = GPIO_DIR & (~(0x02));   //0x11; //Set SK, GPIOL0 pins as output with bit , DO and other pins as input with bit 
            
			txBfrPtr[bufferCnt++] = MSB_RISING_EDGE_CLOCK_BYTE_IN; // 0x20 Command to clock data byte in on +ve Clock Edge MSB first
			txBfrPtr[bufferCnt++] = 0x00;
			txBfrPtr[bufferCnt++] = 0x00; //Data length of 0x0000 means 1 byte data to clock in

            if (dwCount < (dwBytesToRead - 1))
            {
                // Master send ack bit with SDA forced low to slave device
				txBfrPtr[bufferCnt++] = 0x80;     // Command to set directions of lower 8 pins and force value on bits set as output
				txBfrPtr[bufferCnt++] = 0xF0;     // Set SDA, SCL low, WP disabled by SK, DO, GPIOL0 at bit 0			
				txBfrPtr[bufferCnt++] = GPIO_DIR; // Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
            }

			txBfrPtr[bufferCnt++] = MSB_RISING_EDGE_CLOCK_BIT_IN; // 0x22 Command to scan in acknowledge bit , +ve clock Edge MSB first            
			txBfrPtr[bufferCnt++] = 0x00;                         //Length of 0 means to scan in 1 bit
			txBfrPtr[bufferCnt++] = 0x87;                         // Send answer back immediate command

			ftStatus = p_FT_Write (handle, txBfrPtr, bufferCnt, &dwNumBytesSent); //Send off the commands
            bufferCnt = 0; //Clear output buffer
            
			// Wait until data is available
			CMOST_WaitIncomingData (handle, 2);

            // Read one byte from device receive buffer
            //	ftStatus = p_FT_Read(ftHandle, rxBuff, 1, &dwNumBytesRead);
            // Read two bytes from device receive buffer, first byte is dummy data, second byte is data ACK byte value
            ftStatus = p_FT_Read (handle, rxBuff, 2, &dwNumBytesRead);

			tmpBfrPtr[dwCount] = rxBuff[0]; //Return the data read from device

			txBfrPtr[bufferCnt++] = 0x80;      //Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF2;      //Set SDA high, SCL low, WP disabled by SK at bit '0', DO, GPIOL0 at bit '1'
			txBfrPtr[bufferCnt++] = GPIO_DIR;  //0x13  //Set SK,DO,GPIOL0 pins as output with bit , other pins as input with bit 
        }

        if (true == i2CStopOn) 
        {
			txBfrPtr[bufferCnt++] = 0x80;	  //Command to set directions of lower 8 pins and force value on bits set as output
			txBfrPtr[bufferCnt++] = 0xF0;     //Set SDA low, SCL low, WP disabled by SK at bit '0', DO, GPIOL0 at bit '1'
			txBfrPtr[bufferCnt++] = GPIO_DIR; //0x13  //Set SK,DO,GPIOL0 pins as output with bit , other pins as input with bit 

            // send I2C STOP seq 1) set SDA Low, SCL Hi 2) set SDA, SCL Hi 3) set SDA, SCL as input to tristate I2C bus ---------
            for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum setup time =600ns )
            {
				txBfrPtr[bufferCnt++] = 0x80;     //Command to set directions of lower 8 pins and force value on bits set as output
				txBfrPtr[bufferCnt++] = 0xF1;     //Set SDA low, SCL high, WP disabled by SK at bit 1, DO, GPIOL0 at bit 0
				txBfrPtr[bufferCnt++] = GPIO_DIR; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
            }

            for (dwCount = 0; dwCount < 4; dwCount++) // Repeat 4 times commands (minimum hold time =600ns)
            {
				txBfrPtr[bufferCnt++] = 0x80;     //Command to set directions of lower 8 pins and force value on bits set as output
				txBfrPtr[bufferCnt++] = 0xF3;     //Set SDA, SCL high, WP disabled by SK, DO at bit 1, GPIOL0 at bit 0
				txBfrPtr[bufferCnt++] = GPIO_DIR; //Set SK,DO,GPIOL0 pins as output with bit 1, other pins as input with bit 0
            }
        }
            
		ftStatus = p_FT_Write (handle, txBfrPtr, bufferCnt, &dwNumBytesSent); //Send off the commands
        
        bufferCnt = 0; //Clear output buffer

		memcpy ((tVoid *)RxBuffer, (tVoid *)&tmpBfrPtr[0], dwBytesToRead);
        lpBytesReaden = dwBytesToRead;
   }
    else
    {
		// Get queue status
		p_FT_GetQueueStatus (handle, &dwNumBytesToRead);

        // Check the receive buffer 
        if (NULL != RxBuffer && 0 != dwNumBytesToRead)
        {
            ftStatus = p_FT_Read (handle, (unsigned char *)RxBuffer, dwNumBytesToRead, (LPDWORD)&dwNumBytesRead);
        }

        ftStatus = p_FT_Purge (handle, FT_PURGE_RX | FT_PURGE_TX);
        if (FT_OK != ftStatus)
        {
        }
    }

	// Free resources
	if (NULL != txBfrPtr)
	{
		free (txBfrPtr);
	}

	if (NULL != tmpBfrPtr)
	{
		free (tmpBfrPtr);
	}

    return lpBytesReaden;
}

static tVoid CMOST_WaitIncomingData (tVoid *ftHandle, DWORD bytesToWaitFor)
{
	tS64 startTime;
	DWORD bytesInTheQueue;

	// Get queue status
	p_FT_GetQueueStatus (ftHandle, &bytesInTheQueue);

	// Get current time
	startTime = UTILITY_GetTime ();

	// While until queue is correctly filled
	while (bytesInTheQueue < bytesToWaitFor)
	{
		p_FT_GetQueueStatus (ftHandle, &bytesInTheQueue);

		// Check timeout
		if (true == UTILITY_CheckTimeout (startTime, MAX_TIMEOUT_WHILECYCLE))
		{
			return;
		}
	}

	return;
}

#else // HOST LINUX or TKERNEL
static tSInt CMOST_SpiWriteSeq (tU32 deviceID, tU8* lpBufferPnt, tU16 dwBytesToWrite, tU8*  RxBuffer, tU16 dwBytesToRead, tBool commkeptActive)
{
#define ETAL_ENHANCE_TRUE_SPI
	tSInt dwNumBytesRead = 0;
	tSInt dwNumBytesToReadOnSPI = 0;
	tSInt resultByteNum = 0;
	tBool errorDetected = false;
	// Correction to manage correct buffer allocation handling.
	//
	//	tU8 *tmpRxBufferPtr = (tU8 *)RxBuffer;
	tU8 *tmpRxBufferPtr;
	tSInt ret;
	
	// not that in theory ; the bytetowrite is set to SPI HEADER : 4
	//
#ifdef ETAL_ENHANCE_TRUE_SPI
	tU16 dwRealBytesToRead;
#endif 

// in the CMOST header, the len is at position 2 in the array (the last byte of cmost header)
#define ETAL_CMOST_LEN_BYTE_POSITION (CMOST_HEADER_LEN - 1)

	if(((tySpiCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->GPIO_CS == ETAL_CS_TRUE_SPI)
	{
#ifdef ETAL_ENHANCE_TRUE_SPI
		// enhancement for TRUE SPI
		// read in 2 steps 
		// 1 : read the CMOST HEADER
		// 2 : read the accurate len
		// as an optimisation : step 1 read HEADER + (CRC len) bytes, so that if no parameter it avoids further read request
		//
		if (FALSE == commkeptActive) // basically it means an answer payload is expected
		{
			dwNumBytesToReadOnSPI = dwBytesToRead + dwBytesToWrite;
		}
		else
		{
			// in true SPI : we need to keep the driver
			// open to read a full response
			// without knowing a priori the length
			dwNumBytesToReadOnSPI = CMOST_HEADER_LEN + CMOST_CRC_LEN + dwBytesToWrite;
		}

		// we need to allocate, we do  know the size
		//		
		tmpRxBufferPtr = (tU8 *) malloc(dwNumBytesToReadOnSPI);
		if (NULL == tmpRxBufferPtr)
		{
			// error in allocation
			ASSERT_ON_DEBUGGING(0);
			ret = 0;
			errorDetected = true;
			goto exit;			
		}

		dwNumBytesRead = BSP_WriteRead_CMOST_SPI(deviceID, (tU8 *)lpBufferPnt, dwBytesToWrite, tmpRxBufferPtr, dwNumBytesToReadOnSPI, FALSE);
		
		// dw NumBytes Read is the number of bytes read on SPI
		// because we read at least as much as write : the 1st 'writes bytes are to be removed'
		// resultByteNum is the useful byte num
		//
		resultByteNum = dwNumBytesRead - dwBytesToWrite;


		if (TRUE == commkeptActive) // basically it means an answer payload is expected
		{
			// what is received is (linked to SPI protocol)
			// CMOST_PHY_HEADER_LEN_SPI (ie dwBytesToWrite) 1st byte => non valid
			// usefull read data starts at CMOST_PHY_HEADER_LEN_SPI
			// starts by CMOST header : len in 3 position.
			//
			
			
			// tmp check addition 
			if (dwBytesToWrite != CMOST_PHY_HEADER_LEN_SPI)
			{
				// this is not correct
				// error in allocation
				ASSERT_ON_DEBUGGING(0);
				ret = 0;
				errorDetected = true;
				goto exit;
			}
			
			// the header provides the real lenght to be read
			dwRealBytesToRead = CMOST_HEADER_LEN + (((*(tmpRxBufferPtr + CMOST_PHY_HEADER_LEN_SPI + ETAL_CMOST_LEN_BYTE_POSITION) ) * 3)) + CMOST_CRC_LEN;

			if (dwRealBytesToRead == (CMOST_HEADER_LEN + CMOST_CRC_LEN))
			{
			 	// this means that the param len is null
			 	// the frame is header + CRC only
			 	//
			 	// nothing to do 
			 	// 
			 	resultByteNum = CMOST_HEADER_LEN + CMOST_CRC_LEN;
			}
			else if (dwRealBytesToRead < CMOST_MAX_RESPONSE_LEN)
			{
				// the parameter len is correct, we need to redo a read with correct len now we know it
				dwNumBytesToReadOnSPI = dwRealBytesToRead + dwBytesToWrite;

				// free and discard prior read data and allocation : we restart full read
				//
				free(tmpRxBufferPtr);
				// we need to allocate, we do  know the size
				//		
				tmpRxBufferPtr = (tU8 *) malloc(dwNumBytesToReadOnSPI);
				if (NULL == tmpRxBufferPtr)
				{
					// error in allocation
					ASSERT_ON_DEBUGGING(0);
					ret = 0;
					errorDetected = true;
					goto exit;						
				}
				
				dwNumBytesRead = BSP_WriteRead_CMOST_SPI(deviceID, (tU8 *)lpBufferPnt, dwBytesToWrite, tmpRxBufferPtr, dwNumBytesToReadOnSPI, FALSE);
				
				// dw NumBytes Read is the number of bytes read on SPI
				// because we read at least as much as write : the 1st 'writes bytes are to be removed'
				// resultByteNum is the useful byte num
				//
				resultByteNum = dwNumBytesRead - dwBytesToWrite;

				
			}
			else
			{
				// there is an error on the reading 
				// 
				// add a warning and set the max len
				CMOST_tracePrintError(TR_CLASS_CMOST, "unexpected len in read result : %d", dwRealBytesToRead);
				
				resultByteNum = CMOST_HEADER_LEN + CMOST_CRC_LEN;
			}
		}
		else
		{
			// resultByteNum is already correct
			//
			//resultByteNum = dwNumBytesRead;
		}
			
		// copy and free the buffer
		// Real byte read 
		//
		// copy the header in destination 
		memcpy(RxBuffer, (tmpRxBufferPtr+dwBytesToWrite), resultByteNum);
		
		//
		free(tmpRxBufferPtr);


	// get further read request : 
	// the 1st part of the answer has been read : the header data
	// this contains the len : number of parameters 
	// each parameter is on 24 bits, i.e. 3 bytes
	// this is the 7th byte in answer
	// we need also to add the checksum size

#else
		if (FALSE == commkeptActive) // basically it means an answer payload is expected
		{
			dwNumBytesToReadOnSPI = dwBytesToRead + dwBytesToWrite;
		}
		else
		{
			// in true SPI : we need to keep the driver
			// open to read a full response
			// without knowing a priori the length
			dwNumBytesToReadOnSPI = CMOST_MAX_RESPONSE_LEN + dwBytesToWrite;
		}

		// we need to allocate, we do  know the size
		//		
		tmpRxBufferPtr = (tU8 *) malloc(dwNumBytesToReadOnSPI);
		if (NULL == tmpRxBufferPtr)
		{
			// error in allocation
			ASSERT_ON_DEBUGGING(0);
			ret = 0;
			goto exit;
				
		}

		dwNumBytesRead = BSP_WriteRead_CMOST_SPI(deviceID, (tU8 *)lpBufferPnt, dwBytesToWrite, tmpRxBufferPtr, dwNumBytesToReadOnSPI, FALSE);
		
		// dw NumBytes Read is the number of bytes read on SPI
		// because we read at least as much as write : the 1st 'writes bytes are to be removed'
		// resultByteNum is the useful byte num
		//
		resultByteNum = dwNumBytesRead - dwBytesToWrite;

		if (TRUE == commkeptActive) // basically it means an answer payload is expected
		{
			// what is received is (linked to SPI protocol)
			// CMOST_PHY_HEADER_LEN_SPI (ie dwBytesToWrite) 1st byte => non valid
			// usefull read data starts at CMOST_PHY_HEADER_LEN_SPI
			// starts by CMOST header : len in 3 position.
			//
			
			// tmp check addition 
			if (dwBytesToWrite != CMOST_PHY_HEADER_LEN_SPI)
			{
				// this is not correct
				// error in allocation
				ASSERT_ON_DEBUGGING(0);
				ret = 0;
				goto exit;
			}
			
			// the header provides the real lenght to be read
			dwNumBytesRead = (((*(tmpRxBufferPtr + CMOST_PHY_HEADER_LEN_SPI + ETAL_CMOST_LEN_BYTE_POSITION) ) * 3)) + CMOST_CRC_LEN;
			// add the 4 bytes of SPI FF FF FF FF
			//
			resultByteNum = CMOST_HEADER_LEN + dwNumBytesRead;

			// We should check that the data contents remain valid !! 
			// else in case or error on SPI read, we may have the len field invalid... 
			//
			if (resultByteNum < CMOST_MAX_RESPONSE_LEN)
			{
				// all seems fine
			}
			else
			{
				// there is an error on the reading 
				// 
				// add a warning and set the max len
				CMOST_tracePrintError(TR_CLASS_CMOST, "unexpected len in read result : %d", resultByteNum);
				
				resultByteNum = CMOST_MAX_RESPONSE_LEN;
			}
		
		}
		else
		{
			// resultByteNum is already correct
			//
			//resultByteNum = dwNumBytesRead;
		}

		// copy and free the buffer
		// Real byte read 
		//
		// copy the header in destination 
		memcpy(RxBuffer, (tmpRxBufferPtr+dwBytesToWrite), resultByteNum);

		//
		free(tmpRxBufferPtr);
#endif // ETAL_ENHANCE_TRUE_SPI

	}
	else
	{
		// we are not on TRUE SPI
		// however, we could, like in TRUE SPI, read in 2 steps.
		// 2 methods : 
		// 1 = same as true SPI => 
		// 1a start SPI communication by CS control
		// 1b read 6 bytes (header + 3)		
		// 1c finish SPI communication by CS control
		// 1d check len
		// 1e if len = 0 means we wait header + CRC only, => reading is complete
		// 1f if len != 0  start SPI communication by CS control
		// 1g bis now we know the len : read in 1 shot all ie header + len + 3
		// 1h finish SPI communication by CS control
		
		// 2 = in step without interupting the CS
		// 1a start SPI communication by CS control
		// 1b read 3 bytes (header )		
		// 1c check len
		// 1d if len = 0 means we wait header + CRC only, => reading is complete
		// 1e if len != 0  start SPI communication by CS control
		// 1f bis now we know the len : read in 1 shot all ie header + len + 3
		// 1g finish SPI communication by CS control

#ifdef ETAL_ENHANCE_TRUE_SPI
		// enhancement for TRUE SPI
		// read in 2 steps 
		// 1 : read the CMOST HEADER
		// 2 : read the accurate len
		// as an optimisation : step 1 read HEADER + (CRC len) bytes, so that if no parameter it avoids further read request
		//
		if (FALSE == commkeptActive) // basically it means an answer payload is expected
		{
			dwNumBytesToReadOnSPI = dwBytesToRead + dwBytesToWrite;
		}
		else
		{
			// in true SPI : we need to keep the driver
			// open to read a full response
			// without knowing a priori the length
			dwNumBytesToReadOnSPI = CMOST_HEADER_LEN + CMOST_CRC_LEN + dwBytesToWrite;
		}
	
		// we need to allocate, we do  know the size
		//		
		tmpRxBufferPtr = (tU8 *) malloc(dwNumBytesToReadOnSPI);
		if (NULL == tmpRxBufferPtr)
		{
			// error in allocation
			ASSERT_ON_DEBUGGING(0);
			errorDetected = true;
			goto exit;
		}

			
		dwNumBytesRead = BSP_WriteRead_CMOST_SPI(deviceID, (tU8 *)lpBufferPnt, dwBytesToWrite, tmpRxBufferPtr, dwNumBytesToReadOnSPI, TRUE);
			
		// dw NumBytes Read is the number of bytes read on SPI
		// because we read at least as much as write : the 1st 'writes bytes are to be removed'
		// resultByteNum is the useful byte num
		//
		resultByteNum = dwNumBytesRead - dwBytesToWrite;
		
		
		if (TRUE == commkeptActive) // basically it means an answer payload is expected
		{
			// what is received is (linked to SPI protocol)
			// CMOST_PHY_HEADER_LEN_SPI (ie dwBytesToWrite) 1st byte => non valid
			// usefull read data starts at CMOST_PHY_HEADER_LEN_SPI
			// starts by CMOST header : len in 3 position.
			//
					
			// tmp check addition 
			if (dwBytesToWrite != CMOST_PHY_HEADER_LEN_SPI)
			{
				// this is not correct
				// error in allocation
				ASSERT_ON_DEBUGGING(0);
				errorDetected = true;
				goto exit;
			}
					
			// the header provides the real lenght to be read
			dwRealBytesToRead = CMOST_HEADER_LEN + (((*(tmpRxBufferPtr + CMOST_PHY_HEADER_LEN_SPI + ETAL_CMOST_LEN_BYTE_POSITION) ) * 3)) + CMOST_CRC_LEN;
		
			if (dwRealBytesToRead == (CMOST_HEADER_LEN + CMOST_CRC_LEN))
			{
				// this means that the param len is null
				// the frame is header + CRC only
				//
				// nothing to do 
				// 
				resultByteNum = CMOST_HEADER_LEN + CMOST_CRC_LEN;
			}
			else if (dwRealBytesToRead < CMOST_MAX_RESPONSE_LEN)
			{
				// the parameter len is correct, we need to redo a read with correct len now we know it
				dwNumBytesToReadOnSPI = dwRealBytesToRead + dwBytesToWrite;
				// free and discard prior read data and allocation : we restart full read
				//
				free(tmpRxBufferPtr);
				
				// we need to allocate, we do  know the size
				//		
				tmpRxBufferPtr = (tU8 *) malloc(dwNumBytesToReadOnSPI);
				if (NULL == tmpRxBufferPtr)
				{
					// error in allocation
					ASSERT_ON_DEBUGGING(0);
					errorDetected = true;
					goto exit;
				}
						
				dwNumBytesRead = BSP_WriteRead_CMOST_SPI(deviceID, (tU8 *)lpBufferPnt, dwBytesToWrite, tmpRxBufferPtr, dwNumBytesToReadOnSPI, TRUE);
				
				// dw NumBytes Read is the number of bytes read on SPI
				// because we read at least as much as write : the 1st 'writes bytes are to be removed'
				// resultByteNum is the useful byte num
				//
				resultByteNum = dwNumBytesRead - dwBytesToWrite;
			}
			else
			{
				// there is an error on the reading 
				// 
				// add a warning and set the max len
				CMOST_tracePrintError(TR_CLASS_CMOST, "unexpected len in read result : %d", dwRealBytesToRead);
						
				resultByteNum = CMOST_HEADER_LEN + CMOST_CRC_LEN;
			}
		}
		else
		{
			// resultByteNum is already correct
			//
			//resultByteNum = dwNumBytesRead;
		}

					
		// copy and free the buffer
		// Real byte read 
		//
		// copy the header in destination 
		memcpy(RxBuffer, (tmpRxBufferPtr+dwBytesToWrite), resultByteNum);
				
		free(tmpRxBufferPtr);
		
		
		// get further read request : 
		// the 1st part of the answer has been read : the header data
		// this contains the len : number of parameters 
		// each parameter is on 24 bits, i.e. 3 bytes
		// this is the 7th byte in answer
		// we need also to add the checksum size
		
#else

		// We are not on TRUE SPI
		// let's read in 2 steps : header 1st will give the len and then payload
		//
		
		// on the SPI :
		// set 1st byte to read : 'header'
		dwNumBytesToReadOnSPI = dwBytesToRead + dwBytesToWrite;
			
		// we should allocate if required
		tmpRxBufferPtr = (tU8 *) malloc(dwNumBytesToReadOnSPI);
		if (NULL == tmpRxBufferPtr)
		{
			// error in allocation
			ASSERT_ON_DEBUGGING(0);
			ret = 0;
			errorDetected = true;
			goto exit;

		}

		
		// Lower CS line
		(void)BSP_SetCS_CMOST_SPI(deviceID, FALSE);

		// Read to get the header
		//
		
		dwNumBytesRead = BSP_WriteRead_CMOST_SPI(deviceID, (tU8 *)lpBufferPnt, dwBytesToWrite, tmpRxBufferPtr, dwNumBytesToReadOnSPI, FALSE);

		// dw NumBytes Read is the number of bytes read on SPI
		// because we read at least as much as write : the 1st 'writes bytes are to be removed'
		// resultByteNum is the useful byte num
		//
		resultByteNum = dwNumBytesRead - dwBytesToWrite;


		if (FALSE == commkeptActive) // basically it means an answer payload is expected
		{
			// this is the end, just raise the CS line
			// Lower CS line
			(void)BSP_SetCS_CMOST_SPI(deviceID, TRUE);

			// copy and free the buffer
			// Real byte read 
			//
			// copy the header in destination 
			memcpy(RxBuffer, (tmpRxBufferPtr+dwBytesToWrite), resultByteNum);
			
			//
			free(tmpRxBufferPtr);
		}
		else
		{

			// what is received is (linked to SPI protocol)
			// CMOST_PHY_HEADER_LEN_SPI (ie dwBytesToWrite) 1st byte => non valid
			// usefull read data starts at CMOST_PHY_HEADER_LEN_SPI
			// starts by CMOST header : len in 3 position.
			//
			

			// get further read request : 
			// the 1st part of the answer has been read : the header data
			// this contains the len : number of parameters 
			// each parameter is on 24 bits, i.e. 3 bytes
			// this is the 7th byte in answer
			// we need also to add the checksum size
			dwNumBytesToReadOnSPI = (((*(tmpRxBufferPtr + CMOST_PHY_HEADER_LEN_SPI + ETAL_CMOST_LEN_BYTE_POSITION) ) * 3)) + CMOST_CRC_LEN;

			// copy the header in destination 
			memcpy(RxBuffer, (tmpRxBufferPtr+dwBytesToWrite), resultByteNum);

			// and free the initial allocation
			free(tmpRxBufferPtr);

			// check that reamaining byte to read are available and valid
			// note that dwNumBytesToReadOnSPI is at least CMOST_CRC_LEN
			if (dwNumBytesToReadOnSPI < (CMOST_PARAMETER_LEN + CMOST_CRC_LEN))// this is the len completing answer which is expected, ie the number of parameters
			{

				// the number of parameters seems correct
				//
				
				// allocate the buffer to read again
				tmpRxBufferPtr = (tU8 *) malloc(dwNumBytesToReadOnSPI);
				if (NULL == tmpRxBufferPtr)
				{
					// error in allocation
					ASSERT_ON_DEBUGGING(0);
					ret = 0;
					errorDetected = true;
					goto exit;
						
				}

				dwNumBytesRead = BSP_WriteRead_CMOST_SPI(deviceID, (tU8 *)NULL, 0, tmpRxBufferPtr, dwNumBytesToReadOnSPI, FALSE);	  

				// Real byte read 
				//
				// copy the header in destination 
				memcpy(RxBuffer+resultByteNum, (tmpRxBufferPtr), dwNumBytesRead);
				
				resultByteNum += dwNumBytesRead;

				// free the buffer
				free(tmpRxBufferPtr);
			}
			else 
			{
				// the read of parameter is not correct
				// add a warning and set the max len
				CMOST_tracePrintError(TR_CLASS_CMOST, "unexpected len in param len calculation : %d", dwNumBytesToReadOnSPI);			
			}
					
			// Raise the CS line ----------------------------------------------------------------
			(void)BSP_SetCS_CMOST_SPI(deviceID, TRUE);
		}

#endif // ENAHNCE METHOD
	}

exit:

	if (true == errorDetected)
	{
		resultByteNum = 0;
	}

	ret = resultByteNum;


    return ret;
}
#endif // CONFIG_HOST_OS_WIN32

#ifdef __cplusplus
}
#endif

#endif // #if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_COMM_CMOST)

// End of file
