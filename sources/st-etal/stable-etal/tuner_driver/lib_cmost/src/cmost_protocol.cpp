//!
//!  \file		cmost_protocol.cpp
//!  \brief		<i><b> CMOST driver </b></i>
//!  \details	CMOST protocol implementation. The file implements the
//!				protocol described in section 1 "Communication Interface" of
//!				the CMOST API specification ("Application interface - TDA7707 and TDA7708",
//!				version 2.41 1 Apr 2016 or later, filename TDA770X_STAR_API_IF.pdf).
//!				The CMOST permits accesses to its memory or register address space
//!				either through the CMOST Protocol (the access is obtained by sending
//!				a command to the CMOST that interprets it and performs the operation)
//!				or through direct access to the address. The first method requires
//!				the CMOST Firmware to be up and running, the second can be used
//!				before the Firmware has been loaded.
//!
//!				Direct access is not recommended after the CMOST Firmware is
//!				loaded and fully operational.
//!
//!				This file supports both methods.
//!  $Author$
//!  \author	(original version) Maurizio Tonella
//!  $Revision$
//!  $Date$
//!

#include "target_config.h"

#include "osal.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_CMOST)

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)

#elif defined CONFIG_HOST_OS_WIN32
	#include <winsock2.h>
#endif

#include "tunerdriver_internal.h"
#include "tunerdriver.h"
#include "cmost_protocol.h"
#include "cmost_helpers.h"
#include "cmost_crc.h"
#include "cmost_lld.h"
#include "common_trace.h"
#include "cmost_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

///
// DEFINES
///
// CMOST state machine
/*!
 * \def		CMOST_IDLE
 * 			CMOST state machine, idle state identifier
 */
#define CMOST_IDLE                                  0
/*!
 * \def		CMOST_TX_FRAME
 * 			CMOST state machine, frame transmission state identifier
 */
#define CMOST_TX_FRAME                              1
/*!
 * \def		CMOST_RX_FRAME
 * 			CMOST state machine, frame reception state identifier
 */
#define CMOST_RX_FRAME                              2
/*!
 * \def		CMOST_ERROR
 * 			CMOST state machine, error state identifier
 */
#define CMOST_ERROR                                 3
/*!
 * \def		CMOST_PARSE_RESPONSE
 * 			CMOST state machine, response parsing state identifier
 */
#define CMOST_PARSE_RESPONSE                        4
/*!
 * \def		CMOST_RESET_DEVICE
 * 			CMOST state machine, device reset state identifier
 */
#define CMOST_RESET_DEVICE                          5

/*!
 * \def		CMOST_NO_MALLOC_NO_CONCURRENCY
 * 			When defined the CMOST driver does not use dynamic memory allocation
 * 			for the intermediate buffers but rather allocates one static buffer
 * 			of size #CMOST_WRITE_BUF_SIZE.
 * \remark	This works fine as long as there is only one entity using the CMOST
 * 			driver at any time; if concurrency is a requirement the macro must be #undefined
 */
#undef CMOST_NO_MALLOC_NO_CONCURRENCY
//#define CMOST_NO_MALLOC_NO_CONCURRENCY


/*! 
 * \def		CMOST_WRITE_BUF_SIZE
 * 			Size of a temporary (static) buffer used by #CMOST_Write when
 * 			#CMOST_NO_MALLOC_NO_CONCURRENCY is defined to generate the
 * 			CMOST command.
 * \remark	Must be big enough to hold #FW_CHUNK_DATA_SIZE_BYTES_CMOST
 * 			plus a 3 byte header.
 */
#define CMOST_WRITE_BUF_SIZE  ((63 * 4) + 4)

/*!
 * \def		CMOST_CMD_ADDRESS
 * 			Address in the CMOST memory space of the command buffer
 */
#define CMOST_CMD_ADDRESS           (tU32)0x020180
/*!
 * \def		CMOST_READ_WAIT_AFTER_CMD
 * 			wait time between write and read command.
 *			in us
 */
#define CMOST_READ_WAIT_AFTER_CMD   100


///
// Type definitions
///
/*!
 * \struct	CMOST_exchangeDataTy
 * 			Type of a temporary buffer use to store the command generated
 * 			by the #CMOST_Read function to request a read operation
 */
typedef struct
{
	tU8 buffer[CMOST_MAX_COMMAND_LEN];
	tSInt len;
} CMOST_exchangeDataTy;

///
// Local variables
///

///
// Local function prototypes
///
static tS32 CMOST_Read (tVoid *devicehandler, tU8 *buf, tU8 *txbuf, tU32 bytesToRead, tU32 deviceID, tBool isCmdAnswer, tBool directAccess, tBool format24bit);
static tS32 CMOST_Write (tVoid *devicehandler, tU8 *buf, tU32 len, tU32 deviceID, tBool directAccess, tBool format24bit);
static tS32 CMOST_SetHeader (tU8 *buf, tBool isI2c, tU8 currAddr, tBool format24bit, tBool writeAccess, tU32 writeAdr);

///
// Global function definitions
///


/***************************
 *
 * CMOST_ProtocolHandle
 *
 **************************/
/*!
 * \brief		The CMOST communication protocol
 * \details		The function sends a command as described in *pParams* and
 * 				reads the CMOST response.
 * \param[in]	pParams - function parameters (see #CMOST_paramsTy)
 * \param[out]	DataPnt - pointer to buffer where the function stores the data
 * 				          received from the CMOST, depending on the operation.
 * 				          The buffer must be big enough to hold the max
 * 				          response from the CMOST, that is #CMOST_MAX_RESPONSE_LEN
 * \param[out]	BytesNum - number of bytes written to *DataPnt*
 * \return		0 no error
 * \return		1 error
 * \callgraph
 * \callergraph
 * \todo		Manage errors 
 */
tUInt CMOST_ProtocolHandle (tVoid *pParams, tU8 *DataPnt, tS32 *BytesNum)
{
	CMOST_exchangeDataTy readData;
	tSInt status = CMOST_IDLE;
	CMOST_paramsTy *paramPtr = (CMOST_paramsTy *)pParams;
	CMOST_txRxDataTy *txRxDataPtr;
	tBool directAccess = false;
	tBool format24bit = true;
	tU32  bytesToRead = 3;
	tUInt retval = 0;

	// Exchange memory ptr
	txRxDataPtr = &paramPtr->txRxData;

	// init for MISRA
	readData.len = 0;
	
	do
	{
		switch (status)
		{
		case CMOST_IDLE:
			if (0 != txRxDataPtr->RemainingBytesNum)
			{
			    if (txRxDataPtr->mode == CMOST_CMDMODE_RD)
			    {
			        status = CMOST_RX_FRAME;
			    }
				else if (CMOST_CMDMODE_CMD == txRxDataPtr->mode     ||
					     CMOST_CMDMODE_CMD_GEN == txRxDataPtr->mode ||
					     CMOST_CMDMODE_WR == txRxDataPtr->mode      ||
					     CMOST_CMDMODE_RD_DMA == txRxDataPtr->mode  ||
						 CMOST_CMDMODE_WR_DMA == txRxDataPtr->mode  ||
						 CMOST_CMDMODE_W_BOOT == txRxDataPtr->mode)
			    {
					// This status is used for all kind of CMD MODES but CMOST_CMDMODE_RD
			        status = CMOST_TX_FRAME;
			    }
				else
				{
					// An unsupported mode has been received
					status = CMOST_ERROR;
				}
			}
			else if (CMOST_CMDMODE_RESET == txRxDataPtr->mode)
			{
			    status = CMOST_RESET_DEVICE;
			}
			break;

		case CMOST_TX_FRAME:
			// Write			
			if ((txRxDataPtr->mode == CMOST_CMDMODE_WR) ||
	            (txRxDataPtr->mode == CMOST_CMDMODE_W_BOOT))
			{
			    directAccess = true;
			    format24bit = false;
			}
			else
			{
				// This else is taken for: CMOST_CMDMODE_CMD, CMOST_CMDMODE_CMD_GEN, CMOST_CMDMODE_RD_DMA,
				//                         CMOST_CMDMODE_WR_DMA
			    directAccess = false;
			    format24bit = true;
			}
			if (0 != CMOST_Write (paramPtr->devHandler, txRxDataPtr->DataBuffer, txRxDataPtr->RemainingBytesNum,
					paramPtr->deviceID, directAccess, format24bit))
			{
				// TODO: see function header (manage errors)
			    // Error
			}

			// Data is TX remove from the queue
			txRxDataPtr->RemainingBytesNum = 0;

			if (CMOST_CMDMODE_W_BOOT == txRxDataPtr->mode)
			{
			    // In case of boot we just simulate an answer, we do not perform an actual read
			    readData.buffer[0] = (tU8)0;
			    readData.len = 1;
			}
			else
			{
					// we should wait ~100 us for the command to be processed in CMOST
					//
					(void)OSAL_s32ThreadWait_us(CMOST_READ_WAIT_AFTER_CMD);
					
				    // We always read 3 bytes first to understand how many others must be read
				    bytesToRead = CMOST_HEADER_LEN;
				    // Read
				    readData.len = CMOST_Read (paramPtr->devHandler, readData.buffer, txRxDataPtr->DataBuffer, bytesToRead, paramPtr->deviceID, true, directAccess, format24bit);
    	        }

            if (readData.len < 0)
            {
                status = CMOST_ERROR;
			}
            else
            {
				// Enter parse response state
				status = CMOST_PARSE_RESPONSE;
            }
			break;

		case CMOST_RX_FRAME:
			

			if (CMOST_CMDMODE_RD == txRxDataPtr->mode)
			{
			    directAccess = true;
			    format24bit = false; 

			    // Retrieve bytes num to read from payload
			    bytesToRead = 4 * ((((tU32)txRxDataPtr->DataBuffer[4] << 8) & 0xFF00) |
			        (((tU32)txRxDataPtr->DataBuffer[5]) & 0xFF));
			}
			else
			{
			    directAccess = false;
			    format24bit = true;

			    // bytes num to read = payload size
			    bytesToRead = txRxDataPtr->RemainingBytesNum;
			}

			// Read
#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
			/*
			* This mode is used only in a special Firmware download procedure,
			* that is to read back each Firmware chunk after download
			* to ensure it is correct.
			* For this the buffer provided by readData is too small
			*/
			if ((DataPnt != NULL) && (BytesNum != NULL))
			{
				*BytesNum = CMOST_Read (paramPtr->devHandler, DataPnt, txRxDataPtr->DataBuffer, bytesToRead, paramPtr->deviceID, false, directAccess, format24bit);
				(void)OSAL_pvMemorySet ((tVoid *)txRxDataPtr, 0, sizeof (CMOST_txRxDataTy));
				status = CMOST_IDLE;
			}
			else
			{
				status = CMOST_ERROR;
			}
#else
			readData.len = CMOST_Read (paramPtr->devHandler, readData.buffer, txRxDataPtr->DataBuffer, bytesToRead, paramPtr->useI2c, (tU8)txRxDataPtr->devAddress, false, directAccess, format24bit);
            if (readData.len < 0)
            {
                status = CMOST_ERROR;
            }
            else
            {
    			// Enter parse response state
    			status = CMOST_PARSE_RESPONSE;
            }
#endif
			break;

			case CMOST_PARSE_RESPONSE:
				if (readData.len > CMOST_MAX_RESPONSE_LEN)
				{
					retval = 1;
				}
				else
				{
					if ((readData.len > 0) && (DataPnt != NULL))
					{
						(void)OSAL_pvMemoryCopy((tVoid *)DataPnt, (tPCVoid)readData.buffer, (tU32)readData.len);
					}
					if (BytesNum != NULL)
					{
						*BytesNum = readData.len;
					}
				}

				// Remove data, it has been used
				(void)OSAL_pvMemorySet ((tVoid *)txRxDataPtr, 0, sizeof (CMOST_txRxDataTy));

			    // Return to idle state
			    status = CMOST_IDLE;
			break;

			case CMOST_RESET_DEVICE:
			    // Reset device
			    CMOST_ResetDevice(paramPtr->devHandler, paramPtr->deviceID);

	            // Set answer: it is build here, CMOST device is not sending any data
			    readData.buffer[0] = (tU8)7;
			    readData.len = 1;

	            // Enter parse response state (in reset case is just the confirmation, not something from the device)
			    status = CMOST_PARSE_RESPONSE;
			break;

			case CMOST_ERROR:
				// TODO: properly manage error with signaling to caller
				(void)OSAL_pvMemorySet ((tVoid *)txRxDataPtr, 0, sizeof (CMOST_txRxDataTy));
			    // Return to idle state
			    status = CMOST_IDLE;
				retval = 1;
			break;
			
			default:
				break;
		}
#if defined (CONFIG_TRACE_ASYNC)
		/*
		 * This OSAL_s32ThreadWait will introduce some delay when downloading the FW
		 * (measured on A2 with CMOST traces set to TR_LEVEL_COMPONENT, 6.1s without sleep, 6.7s with 1ms sleep)
		 * but aids the etalTrace empty the FIFO in all other situations
		 */
		if (status == CMOST_IDLE)
		{
			OSAL_s32ThreadWait (TUNER_DRIVER_CMOST_THREAD_SCHEDULING);
		}
#endif
	}
	while (status != CMOST_IDLE);

	return retval;
} 

/***************************
 *
 * CMOST_SendMessageStart
 *
 **************************/
/*!
 * \brief		Formats the parameter structure used by #CMOST_ProtocolHandle
 * \param[out]	memoryPtr - pointer to the structure filled by the function,
 * 							to be passed to #CMOST_ProtocolHandle.
 * 				            See #CMOST_paramsTy for the format
 * \param[in]	DataToSend - pointer to the buffer containing the data for the command
 * \param[in]	type - requested driver operation mode. See #CMOST_cmdModeEnumTy
 * \param[in]	accessSize - the operation type. See #CMOST_accessSizeEnumTy
 * \param[in]	busAddress - the device address. See #CMOST_addressEnumTy
 * \param[in]	busOptions - the communication options. See #CMOST_paramsTy
 * \param[in]	BytesNumber - size of the transfer
 * \return		0 - success
 * \callgraph
 * \callergraph
 */
tS32 CMOST_SendMessageStart(tVoid *memoryPtr, tU8 *DataToSend, tU8 type, tU8 accessSize, tU8 busAddress, tU8 busOptions, tU32 BytesNumber)
{
	CMOST_paramsTy *txRxDataPtr = (CMOST_paramsTy *)memoryPtr;    
	tS32 ret;

	if (0)
	{
		ret = -1;
	}
	else
	{
		txRxDataPtr->txRxData.DataBuffer = DataToSend;

		txRxDataPtr->txRxData.TotalBytesNum = 0;
		txRxDataPtr->txRxData.RemainingBytesNum = 0;
		txRxDataPtr->txRxData.paramSize = (CMOST_accessSizeEnumTy)accessSize;
		txRxDataPtr->txRxData.devAddress = (CMOST_addressEnumTy)busAddress;
		txRxDataPtr->txRxData.mode = (CMOST_cmdModeEnumTy)type;
		txRxDataPtr->communicationOptions  = busOptions;

		if (CMOST_CMDMODE_RESET != txRxDataPtr->txRxData.mode)
		{
			txRxDataPtr->txRxData.TotalBytesNum = BytesNumber;
			txRxDataPtr->txRxData.RemainingBytesNum = BytesNumber;
		}

		ret = 0;
	}

	return ret;
}

/***************************
 *
 * CMOST_Read
 *
 **************************/
/*!
 * \brief		Reads data from the CMOST using the CMOST protocol or a direct access
 * \details		The function creates the read command header (through #CMOST_SetHeader).
 * 				The header address field contains:
 * 				- the #CMOST_CMD_ADDRESS address for accesses through the CMOST protocol
 * 				- the address of the location to be accessed for direct memory accesses.
 * 				In the latter case the address of the CMOST location must be specified
 * 				in the first three bytes of the data buffer *txbuf*.
 *
 * 				A read operation is actually composed of an I2C write on (specifying
 * 				the direct address to be accessed, or the address of the CMOST command
 * 				buffer) followed by an I2C read operation, fetching the actual data.
 *
 * 				CMOST responses are formatted as the commands: a three-byte header
 * 				followed by a variable number of parameters, each one three bytes long.
 * 				The final three bytes contain the checksum.
 *
 * 				The I2C read is performed in two steps: first the function reads
 * 				the response header (the *bytesToRead* bytes containing the CMOST 'status',
 * 				'command Id' and 'command length' fields), then uses the 'command length'
 * 				field to calculate the number of bytes to read and finally reads the
 * 				response parameters. *bytesToRead* is dependent on the CMOST communication
 * 				protocol and should be normally set to #CMOST_HEADER_LEN.
 *
 * 				The function then calculates the checksum on the received response
 * 				(#CMOST_CalculateCrc) and compares it with the one contained in
 * 				the response itself.
 *
 * 				An exception to the above is when the function is used in direct access mode,
 * 				typically for the #CMOST_CMDMODE_RD operational mode of #CMOST_ProtocolHandle,
 * 				which is used only for debug (see #BOOT_READ_BACK_AND_COMPARE_CMOST). In this
 * 				case the function performs only one read operation and *bytesToRead*
 * 				is actually the number of bytes to read.
 *
 * \remark		The function is prepared for but does not support SPI bus.
 * \param[in]	devicehandler - unused for non-CONFIG_HOST_OS_WIN32 builds
 * \param[out]	buf - pointer to a buffer filled by the function with the data
 * 				      read from the CMOST. Must be big enough to hold the
 * 				      requested data plus the 3-bytes command header and the 3-bytes checksum.
 * 				      The max size for this buffer is #CMOST_MAX_RESPONSE_LEN.
 * \param[in]	txbuf - pointer to a data buffer containing the address to be accessed;
 * 				        used only for *directAccess* set to TRUE
 * \param[in]	bytesToRead - the number of bytes to read in the first step
 * \param[in]	useI2c - TRUE if the device is mapped on the I2C bus, FALSE if mapped on the SPI
 * \param[in]	i2cAdr - the device address
 * \param[in]	isCmdAnswer - if TRUE the function reads the first three bytes and interprets
 * 				              them as command header, then reads the variable number of
 * 				              parameters. If FALSE it only reads the number of bytes specified
 * 				              in *bytesToRead*. The latter is used only for special cases,
 * 				              i.e. to read back the firmware for sanity control.
 *
 * 				              Normally set to TRUE.
 * \param[in]	directAccess - if TRUE use the direct access method, not the CMOST Protocol.
 * 				               if FALSE uses the CMOST Protocol access method (normal case).
 * \param[in]	format24bit - if TRUE reads 24-bit words (normal case); if FALSE reads
 * 				              32 bit words
 * \return		The number of bytes received or -1 in case of error
 * \callgraph
 * \callergraph
 * \todo		Add SPI support. NOTE: the function strips the CMOST command header
 * 				from the data returned in *buf* for the SPI case. Since this is different
 * 				from the I2C behavior it will probably confuse the upper layers once
 * 				the SPI support is in place.
 */
static tS32 CMOST_Read (tVoid *devicehandler, tU8 *buf, tU8 *txbuf, tU32 bytesToRead, tU32 deviceID, tBool isCmdAnswer, tBool directAccess, tBool format24bit)
{
	tU32 u32BytesRead;
	tS32 s32BytesRead;
	tU8 dataBfrPtr[CMOST_PHY_HEADER_LEN_MAX];
	tU32 len;
	tU32 writeAddress;
	tBool i2CStartOn = true;
	tBool i2CStopOn = true;
	tU32 u32Checksum;
	tBool useI2c;
	tU8 i2cAdr;
#ifdef CONFIG_HOST_OS_WIN32
	tU32 cnt;
#endif
	tS32 ret = 0;

	useI2c = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ? TRUE : FALSE;
	i2cAdr = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ?
			((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress :
			deviceID; // not used in SPI case

	// Set header
	if (false == directAccess)
	{
		// Build header
		writeAddress = 0;
	}
	else
	{
		// Build header
		writeAddress = (((tU32)*txbuf << 16) & 0x00FF0000) | (((tU32)*(txbuf + 1) << 8) & 0x0000FF00) | (((tU32)*(txbuf + 2) << 0) & 0x000000FF);
	}

// Correction to SPI interface
// the particular management of SPI interface which requires extrat space for SPI
// should be handle in SPI bsp itself not here
//
#ifdef CONFIG_HOST_OS_WIN32
	if (false == useI2c)
	{
	    bytesToRead += 4;
	}
#endif
	// Allocate space for the buffer (header + payload)
	(void)OSAL_pvMemorySet ((tVoid *)dataBfrPtr, 0xFF, CMOST_PHY_HEADER_LEN_MAX);

	// Set the CMOST PHY header
	if (CMOST_SetHeader (dataBfrPtr, useI2c, i2cAdr, format24bit, false, writeAddress) > 4)
	{
		ASSERT_ON_DEBUGGING(0);
		ret = -1;
		goto exit;
	}

	// even for a read, it is interesting to know what we request
	CMOST_tracePrintComponent(TR_CLASS_CMOST, "Read Command from host:");
#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_COMPONENT)
#ifdef CONFIG_HOST_OS_WIN32
	COMMON_tracePrintBufComponent(TR_CLASS_CMOST, dataBfrPtr, (CMOST_HEADER_LEN + 1), NULL);
#else
	if(useI2c == TRUE)
	{
		COMMON_tracePrintBufComponent(TR_CLASS_CMOST, dataBfrPtr, CMOST_PHY_HEADER_LEN_I2C, NULL);
	}
	else
	{
		COMMON_tracePrintBufComponent(TR_CLASS_CMOST, dataBfrPtr, CMOST_PHY_HEADER_LEN_SPI, NULL);
	}
#endif
#endif

	if (false == useI2c)
	{
#ifdef CONFIG_HOST_OS_WIN32
		u32BytesRead = CMOST_WriteReadData (devicehandler, (tU8*)dataBfrPtr, (CMOST_HEADER_LEN + 1), (tU8*)buf, bytesToRead, deviceID, isCmdAnswer);
#else
		u32BytesRead = CMOST_WriteReadData (devicehandler, (tU8*)dataBfrPtr, CMOST_PHY_HEADER_LEN_SPI, (tU8*)buf, bytesToRead, deviceID, isCmdAnswer);
#endif
		if ((u32BytesRead != bytesToRead) && (false == isCmdAnswer))
		{
			ret = -1;
			goto exit;
		}
		// Correction to SPI interface
		// the particular management of SPI interface which requires extrat space for SPI
		// should be handle in SPI bsp itself not here
		//


#ifdef CONFIG_HOST_OS_WIN32
		// Discharge first 4 elements of  and decrement by 4 the length
		for (cnt = 0; cnt < u32BytesRead; cnt++)
		{
			*(buf + cnt) = *(buf + cnt + 4);
		}
		u32BytesRead -= 4;
#endif

		// verify the checksum if an answer is read
		if (true == isCmdAnswer)
		{
				// check bytes Read : it should be > HEADER+CRC_LEN
				// if 
				if (u32BytesRead < (CMOST_HEADER_LEN + CMOST_CRC_LEN))	
				{
					// This is not correct
					CMOST_tracePrintError(TR_CLASS_CMOST,"CMOST response len error : %d expected >= 6", u32BytesRead);
					ASSERT_ON_DEBUGGING(0);
					ret = -1;
					goto exit;
				}
					
				u32Checksum = CMOST_CalculateCrc(buf, u32BytesRead - CMOST_CRC_LEN);

				if (buf[u32BytesRead-CMOST_CRC_LEN] != (tU8)((u32Checksum >> 16) & 0xFF) || 
					buf[u32BytesRead-(CMOST_CRC_LEN-1)] != (tU8)((u32Checksum >> 8) & 0xFF) || 
					buf[u32BytesRead-(CMOST_CRC_LEN-2)] != (tU8)(u32Checksum & 0xFF))
				{
					/* Checksum Error in receiving: the flag is set in the command header.
					It could even be already set if reporting an error in transmission */
					ret = -1;
					goto exit;
				}

				if (buf[1] != txbuf[1])
				{
					/* Response command number error: 
					the command number in response is different from command. */
					ret = -1;
					goto exit;
				}
		}

		CMOST_tracePrintComponent(TR_CLASS_CMOST, "Answer from CMOST:");
#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_CMOST, buf, u32BytesRead, NULL);
#endif
	}
	else
	{
#ifdef CONFIG_HOST_OS_WIN32
	    if (CMOST_WriteRaw (devicehandler, (tU8*)dataBfrPtr, (CMOST_HEADER_LEN + 1), deviceID) != 0)
		{
			ret = -1;
			goto exit;
		}
#else
	    if (CMOST_WriteRaw (devicehandler, (tU8*)dataBfrPtr, CMOST_PHY_HEADER_LEN_I2C, deviceID) != 0)
		{
			ret = -1;
			goto exit;
		}
#endif

		// if (true == isCmdAnswer) 
		//    if read (buf[2] == 0) tx I2C stop and finish
		//    else stopI2c is not sent and next read continue without sending I2C start and chipaddr+1
		//  else (false == isCmdAnswer)  then
		//  tx I2C stop anyway and finish
		if (true == isCmdAnswer)
		{
			i2CStopOn = false;
		}
		else
		{
			i2CStopOn = true;
		}

		s32BytesRead = CMOST_ReadRaw (devicehandler, (tU8*)buf, bytesToRead, deviceID, i2CStartOn, i2CStopOn);

		if ((s32BytesRead < 0) || (s32BytesRead != (tS32)bytesToRead))
		{
			ret = -1;
			goto exit;
		}
		u32BytesRead = (tU32)s32BytesRead;

		CMOST_tracePrintComponent(TR_CLASS_CMOST, "Answer from CMOST:");
#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_CMOST, buf, 3, NULL);
#endif

		if (true == isCmdAnswer)
		{
			i2CStartOn = true;
			i2CStopOn = true;

			// 2 steps reading : 
			// 1 : read the header only ==> this should be the value of bytesToRead 
			// and the result of prior read
			// 2 : read parameters and checksum : 
			//
			

			// buf[CMOST_HEADER_LEN - 1] ie buf[2] stores "OutLen" value.
			// If != 0, the verification answer
			// is extended with Parameters and Checksum
			// 
			if (buf[(CMOST_HEADER_LEN - 1)] != (tU8)0)
			{
			    len = (tU32)buf[CMOST_HEADER_LEN - 1] * 3 + CMOST_CRC_LEN;

				// check the len is in the accepted range
				//
				if (len < (CMOST_PARAMETER_LEN + CMOST_CRC_LEN))
				{
					// all is fine

				    // "len = buf[2]*3+3" is the number of bytes for 24bitWords of Parameters + Checksum
				    u32BytesRead += CMOST_ReadRaw (devicehandler, (tU8*)&buf[3], len, deviceID, i2CStartOn, i2CStopOn);

				    // "len+3" keeps into account the bytes of the header already buffered
				    if (u32BytesRead != (len + 3))
				    {
				        ret = -1;
				        goto exit;
				    }

					u32Checksum = CMOST_CalculateCrc(buf, u32BytesRead - 3);

					if (buf[u32BytesRead-3] != (tU8)((u32Checksum >> 16) & 0xFF) || 
						buf[u32BytesRead-2] != (tU8)((u32Checksum >> 8) & 0xFF) || 
						buf[u32BytesRead-1] != (tU8)(u32Checksum & 0xFF))
					{
						/* Checksum Error in receiving: the flag is set in the command header.
						It could even be already set if reporting an error in transmission */
						CMOST_tracePrintError(TR_CLASS_CMOST,"CMOST_Read I2C / CRC ERROR\n");
						ret = -1;
						goto exit;
					}


#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_COMPONENT)
				COMMON_tracePrintBufComponent(TR_CLASS_CMOST, buf + 3, len - 3, NULL); // the header bytes were already printed
#endif
				}
				else
				{
					// len read is out of range : 
					// return an error
					CMOST_tracePrintError(TR_CLASS_CMOST,"CMOST_Read I2C / read cmost_param_len is out of range : %d\n", buf[CMOST_HEADER_LEN - 1]);
					ret = -1;
					goto exit;
				}

			}

			if (buf[1] != txbuf[1])
			{
				/* Response command number error: 
				the command number in response is different from command. */
				ret = -1;
				goto exit;
			}
		}
	}

	ret = (tS32)u32BytesRead;

exit:	
	return ret;
}

/***************************
 *
 * CMOST_Write
 *
 **************************/
/*!
 * \brief		Writes data to the CMOST using the CMOST protocol or a direct access
 * \details		The function creates the write command header (through #CMOST_SetHeader).
 * 				The header address field contains:
 * 				- the #CMOST_CMD_ADDRESS address for accesses through the CMOST protocol
 * 				- the address of the location to be accessed for direct memory accesses.
 * 				In the latter case the address of the CMOST location must be specified
 * 				in the first three bytes of the data buffer *buf*. These address bytes
 * 				will be discarded when generating the CMOST command.
 *
 * 				The function calculates the checksum on the *buf* data (#CMOST_CalculateCrc)
 * 				and appends it to the CMOST command.
 * \remark		
 * \param[in]	devicehandler - unused for non-CONFIG_HOST_OS_WIN32 builds
 * \param[in]	buf - pointer to the buffer of data to be sent to the CMOST; in case
 * 				      of direct memory access the first 3 bytes of the buffer must
 * 				      contain the CMOST address to be written
 * \param[in]	len - size of the *buf*. In case of direct memory access this number
 * 				      must include the 3-byte address field
 * \param[in]	useI2c - TRUE if the device is mapped on the I2C bus, FALSE if mapped on the SPI
 * \param[in]	i2cAdr - the device address
 * \param[in]	directAccess - if TRUE use the direct access method, not the CMOST Protocol.
 * 				               if FALSE uses the CMOST Protocol access method (normal case).
 * \param[in]	format24bit - if TRUE writes 24-bit words (normal case); if FALSE writes
 * 				              32 bit words
 * \return		0 - no error
 * \return		-1 - communication error
 * \return		-2 - memory allocation error
 * \callgraph
 * \callergraph
 */
static tS32 CMOST_Write (tVoid *devicehandler, tU8 *buf, tU32 len, tU32 deviceID, tBool directAccess, tBool format24bit)
{
	tU32 u32BufferCnt = 0;
	tU32 u32Checksum, CMOST_PhyHeaderLength;
	tU8 *dataBfrPtr = NULL;
	tS32 ret = 0;
	tBool useI2c;
	tU8 i2cAdr;

#ifdef CMOST_NO_MALLOC_NO_CONCURRENCY
	/*
	 * dataBfrPtrStatic is static because it could potentially
	 * be large (depending on the firmware chunk size used during
	 * the patch or firmware download).
	 * This implies that this function does not support concurrent calls
	 * for firmware download.
	 */
	static tU8 dataBfrPtrStatic[CMOST_WRITE_BUF_SIZE]; 
#endif
	/*
	 * dataBfrPtrHeap is allocated on the heap to support concurrent
	 * function invocations (on different devices)
	 */
//	tU8 dataBfrPtrHeap[CMOST_MAX_COMMAND_LEN];
	tU32 writeAddress;

	useI2c = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ? TRUE : FALSE;
	i2cAdr = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ?
			((tyI2cCommunicationBus*)TUNERDRIVER_GetCommunicationBusConfig(deviceID))->deviceAddress :
			deviceID; // not use in SPI case.
			
#ifdef CONFIG_HOST_OS_WIN32
	CMOST_PhyHeaderLength = CMOST_HEADER_LEN + 1;
#else
	CMOST_PhyHeaderLength = (TUNERDRIVER_GetBusType(deviceID) == BusI2C) ? CMOST_PHY_HEADER_LEN_I2C : CMOST_PHY_HEADER_LEN_SPI;
#endif


	CMOST_tracePrintComponent(TR_CLASS_CMOST, "Command from host:");
#if defined(CONFIG_TRACE_CLASS_CMOST) && (CONFIG_TRACE_CLASS_CMOST >= TR_LEVEL_COMPONENT)
	COMMON_tracePrintBufComponent(TR_CLASS_CMOST, buf, len, NULL);
#endif

	
	// Set header
	if (false == directAccess)
	{

		if (len >= CMOST_PARAMETER_LEN)
		{
			ASSERT_ON_DEBUGGING(0);
			ret = -1;
			goto exit;
		}
		else
		{
			dataBfrPtr = (tU8 *)OSAL_pvMemoryAllocate (CMOST_PhyHeaderLength + len + CMOST_CRC_LEN);
		
			if (dataBfrPtr == NULL)
			{
				ret = -2;
				goto exit;
			}
		}

		// Calculate CRC
		u32Checksum = CMOST_CalculateCrc (buf, len);

		// Build header
		writeAddress = 0;

		u32BufferCnt += CMOST_SetHeader (dataBfrPtr, useI2c, i2cAdr, format24bit, true, writeAddress);

		// Set the payload
		(void)OSAL_pvMemoryCopy ((tVoid *)(dataBfrPtr + u32BufferCnt), (const tVoid *)buf, len);
		u32BufferCnt += len;

		// The third Command Protocol element consists of 24 bits of the
		// Checksum added to the command processing message
		*(dataBfrPtr + u32BufferCnt) = (tU8)((u32Checksum >> 16) & 0xFF); u32BufferCnt++;
		*(dataBfrPtr + u32BufferCnt) = (tU8)((u32Checksum >> 8) & 0xFF); u32BufferCnt++;
		*(dataBfrPtr + u32BufferCnt) = (tU8)(u32Checksum & 0xFF); u32BufferCnt++;
	}
	else
	{
#ifdef CMOST_NO_MALLOC_NO_CONCURRENCY
		/*
		 * the first three bytes of buf contain the destination
		 * address and do not get copied into dataBfrPtr
		 */
		if ((len - 3) > (CMOST_WRITE_BUF_SIZE - CMOST_PhyHeaderLength))
		{
			ASSERT_ON_DEBUGGING(0);
			ret = -1;
			goto exit;
		}
		else
		{
			dataBfrPtr = dataBfrPtrStatic;
		}
#else
		// Allocate space for the buffer (header + payload)
		dataBfrPtr = (tU8 *)OSAL_pvMemoryAllocate (CMOST_PhyHeaderLength + len);
		if (dataBfrPtr == NULL)
		{
			ret = -2;
			goto exit;
		}
#endif

		// Build header
		writeAddress = (((tU32)*buf << 16) & 0x00FF0000) | (((tU32)*(buf + 1) << 8) & 0x0000FF00) | (((tU32)*(buf + 2) << 0) & 0x000000FF);

		// Adjust first byte
		u32BufferCnt += CMOST_SetHeader (dataBfrPtr, useI2c, i2cAdr, format24bit, true, writeAddress);

		// Set the payload
#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
		/*
		 * The first 3 bytes of <buf> contain the firmware address,
		 * so we must skip it from the copy (it is managed by SetHeader).
		 * The non-Linux implementation does the same because CMOST_SetHeader always
		 * returns 4 but it is not logically correct
		 */
		(void)OSAL_pvMemoryCopy ((tVoid *)(dataBfrPtr + u32BufferCnt), (const tVoid *)(buf + 3), len-3);
#else
		/*
		 * correct but logically wrong (see above)
		 */
		(void)OSAL_pvMemoryCopy ((tVoid *)(dataBfrPtr + u32BufferCnt), (const tVoid *)(buf + u32BufferCnt - 1), len-3);
#endif
		u32BufferCnt += (len-3);
	}

	// The full command processing message Len is assigned to the argument
	len = u32BufferCnt;

	// "CmdProcBuf" to be actually written
	if (0 != CMOST_WriteRaw (devicehandler, (tU8 *)dataBfrPtr, len, deviceID))
	{
		ret = -1;
	}

#ifdef CMOST_NO_MALLOC_NO_CONCURRENCY
	if (false == directAccess)
	{
		OSAL_vMemoryFree ((tVoid *)dataBfrPtr);
	}
#else
	// Free resources
	// in both case it has been dynamically allocated
	OSAL_vMemoryFree ((tVoid *)dataBfrPtr);
#endif

exit:
	return ret;
}

/***************************
 *
 * CMOST_SetHeader
 *
 **************************/
/*!
 * \brief		Creates the CMOST protocol Physical Layer header
 * \details		This function creates the header described in Section 1.1 "Physical Layer"
 * 				of the CMOST specification. For normal operation (that is
 * 				an access to the CMOST memory using the CMOST protocol
 * 				the *writeAdr* must be set to zero: the function will use
 * 				the CMOST command buffer address (#CMOST_CMD_ADDRESS).
 * 				For direct memory access operation (that is an access to the
 * 				CMOST memory or register address space without using the CMOST
 * 				protocol) the *writeAdr* must be set to non-zero, the function
 * 				will use this address as the operation address.
 *
 * 				The physical layer header is composed of 4 bytes for SPI:
 *
 * 				- bit31     = read (0) / write (1) access
 * 				- bits30-29 = 32 (00) / 24 (11) bit transfer
 * 				- bit 28    = single (0) / burst (1) access mode
 * 				- bit 27    = no increment (0) / auto increment (1) mode
 * 				- bits26-18 = unused
 * 				- bits17-0  = peripheral address in the CMOST memory space
 *
 * 				and 3 bytes for I2C:
 *
 * 				- bit 23    = read (0) / write (1) access
 * 				- bit 22    = no increment (0) / auto increment (1) mode
 * 				- bit 21    = single (0) / burst (1) access mode
 * 				- bit 20    = 32 (00) / 24 (11) bit transfer
 * 				- bits19-18 = unused
 * 				- bits17-0  = peripheral address in the CMOST memory space
 *
 * \param[out]	buf
 * \param[in]	isI2c - TRUE if the CMOST device is on I2C bus, FALSE if it is on SPI
 * \param[in]	currAddr_win32 - the I2C address; this parameter is unused and ignored
 * 				           on non WIN32 builds because the address is managed
 * 				           directly by the kernel driver.
 * \param[in]	format24bit - if TRUE use a 24 bit access, if FALSE use a 32 bit access
 * \param[in]	writeAccess - if TRUE use a write access, if FALSE use a read access
 * \param[in]	writeAdr - the address of the operation (in the CMOST memory space)
 * 				           for a direct access operation. If set to 0 the function
 * 				           uses the CMOST command buffer address (#CMOST_CMD_ADDRESS)
 * 				           for a CMOST protocol operation.
 * \return		the size of the header (3 for I2C, 4 for SPI)
 * \callgraph
 * \callergraph
 * \todo		Change parameter isI2C from tBool to tU8 to contain the device address
 */
static tS32 CMOST_SetHeader (tU8 *buf, tBool isI2c, tU8 currAddr_win32, tBool format24bit, tBool writeAccess, tU32 writeAdr)
{
	tSInt counter = 0;
	tU32 adrToUse;

	if (0 == writeAdr)
	{
		adrToUse = CMOST_CMD_ADDRESS;
	}
	else
	{
		adrToUse = writeAdr;
	}

	if (true == isI2c)
	{
#ifdef CONFIG_HOST_OS_WIN32
		// force i2CAddr as first byte (default = 0xC2)
		*(buf + counter) = currAddr_win32; counter++;
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
		/*
		 * In Linux the I2C address is added by the kernel
		 * driver
		 * The <buf> allocated in Linux does not have space
		 * for this additional byte (see CMOST_DRIVER_HEADER_LEN definition)
		 */
#endif
		if (true == format24bit)
		{
			*(buf + counter) = (tU8)0x70;
		}
		else
		{
			*(buf + counter) = (tU8)0x60;
		}

		if (true == writeAccess)
		{
			*(buf + counter) |= 0x80;
		}

		*(buf + counter) |= (tU8)((adrToUse >> 16) & 0xFF);
		counter++;
		*(buf + counter) = (tU8)((adrToUse >> 8)   & 0xFF); 
		counter++;
		*(buf + counter) = (tU8)((adrToUse >> 0)   & 0xFF); 
		counter++;
	}
	else
	{ 
		// SPI management
		if (true == format24bit)
		{
			*(buf + counter) = (tU8)0x78;
		}
		else
		{
			*(buf + counter) = (tU8)0x18; 
		}
		if (true == writeAccess)
		{
			*(buf + counter) |= 0x80; 
		}
		counter++;

		*(buf + counter) = (tU8)((adrToUse >> 16) &  0xFF); counter++;
		*(buf + counter) = (tU8)((adrToUse >> 8)   & 0xFF); counter++;
		*(buf + counter) = (tU8)((adrToUse >> 0)   & 0xFF); counter++;
	}

	return counter;
}

#ifdef __cplusplus
}
#endif

#endif // #if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_COMM_CMOST)

// End of file
