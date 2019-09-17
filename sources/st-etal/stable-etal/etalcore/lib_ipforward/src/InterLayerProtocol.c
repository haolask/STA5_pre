/****************************************************************************/
/*                                                                          */
/* Project:              ADR3 Control application                           */
/* Filename:             InterLayer_Protocol.c                              */
/* Programmer:           Alessandro Vaghi                                   */
/* Date:                 Jun 18th 2010                                      */
/* CPU:                  PC                                                 */
/* Type:                 C code                                             */
/* Scope:                Serial communication functions                     */
/* Functions:                                                               */
/*                                                                          */
/****************************************************************************/

#include "osal.h"

#ifdef CONFIG_COMM_DRIVER_EXTERNAL

#include "etaldefs.h"
#include "ipfcomm.h"
#include "TcpIpProtocol.h"
#include "ctrl_app.h"

#define GUI_PARSE_IDLE                  0
#define GUI_GET_LUN_ID                  1
#define GUI_GET_SPARE_0                 2
#define GUI_GET_SPARE_1                 3
#define GUI_GET_SPARE_2                 4
#define GUI_GET_SPARE_3                 5
#define GUI_GET_DATA_LEN                6
#define GUI_GET_DATA                    7

#define IPF_FIFO_FULL_RETRY 5

static tU8 FromCtrlAppMessageParse ( PortInfo *port, CtrlAppMessageHeaderType *HeaderPnt , tU32* RemainingBytesPnt ) ;

/*************************************************************************************/
/*                                                                                   */
/* void ProtocolLayer_FifoReset ( tProtocolLayerFifo *fifo )                         */
/*                                                                                   */
/*************************************************************************************/
static tVoid ProtocolLayer_FifoReset(tProtocolLayerFifo *fifo)
{
  fifo->writeIndex = (tU8)0;
  fifo->readIndex = (tU8)0;
  OSAL_pvMemorySet((tPVoid)fifo->fifo, 0x00, sizeof(tProtocolLayerFifoElem) * PROTOCOL_LAYER_FIFO_SIZE);
}

/*************************************************************************************/
/*                                                                                   */
/* void ProtocolLayer_FifoPush                                                       */
/*                                                                                   */
/*************************************************************************************/
/*
 * Push a message to the MDRComm to ETAL fifo
 * This is invoked by the ipforward
 */
static tSInt ProtocolLayer_FifoPush(tS32 index, CtrlAppMessageHeaderType *head)
{
	tSInt retval = OSAL_ERROR;
	tProtocolLayerFifo *fifo;
	tProtocolLayerFifoElem *elem;
	PortInfo *port;

	if (index < 0)
	{
		ASSERT_ON_DEBUGGING(0);
	}

	port = &TcpIpPortList[index];
    fifo = &port->ProtocolLayerFifo;
	elem = &fifo->fifo[fifo->writeIndex];

	OSAL_s32SemaphoreWait(port->CommSem, OSAL_C_TIMEOUT_FOREVER);

	if ((fifo->writeIndex != fifo->readIndex) || !(elem->used))
	{
		elem->used = TRUE;
		if (port->ProtocolLayer_DataBufferBytesNum > PROTOCOL_LAYER_INTERNAL_BUFFER_LEN)
		{
			IPF_tracePrintError(TR_CLASS_APP_IPFORWARD, "Received message too long (%d), truncating to %d", port->ProtocolLayer_DataBufferBytesNum, PROTOCOL_LAYER_INTERNAL_BUFFER_LEN);
			port->ProtocolLayer_DataBufferBytesNum = PROTOCOL_LAYER_INTERNAL_BUFFER_LEN;
		}
		elem->len = port->ProtocolLayer_DataBufferBytesNum;
		OSAL_pvMemoryCopy((tPVoid)elem->data, (tPCVoid)port->ProtocolLayer_DataBuffer, port->ProtocolLayer_DataBufferBytesNum);
		OSAL_pvMemoryCopy((tPVoid)&(elem->head), (tPCVoid)head, sizeof(CtrlAppMessageHeaderType));
		fifo->writeIndex += (tU8)1;
		fifo->writeIndex %= PROTOCOL_LAYER_FIFO_SIZE;

		retval =  OSAL_OK;
	}

	OSAL_s32SemaphorePost(port->CommSem);
	return retval;
}

/*************************************************************************************/
/*                                                                                   */
/* void ProtocolLayer_FifoPop                                                        */
/*                                                                                   */
/*************************************************************************************/
/*
 * Pop a message from the ProtocolLayer fifo
 * This is invoked by the ETAL
 */
tSInt ProtocolLayer_FifoPop(tS32 index, tU8 *data, tU32 max_data, tU32 *len, CtrlAppMessageHeaderType *head)
{
	tProtocolLayerFifo *fifo;
	tProtocolLayerFifoElem *elem;
	PortInfo *port;
	tSInt ret = OSAL_OK;

	if (index < 0)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	port = &TcpIpPortList[index];
	OSAL_s32SemaphoreWait(port->CommSem, OSAL_C_TIMEOUT_FOREVER);

    fifo = &port->ProtocolLayerFifo;
	elem = &fifo->fifo[fifo->readIndex];

	if ((fifo->writeIndex == fifo->readIndex) && !(elem->used))
	{
		// fifo empty
		ret = OSAL_ERROR_TIMEOUT_EXPIRED;
	}
	else
	{
		if ((data != NULL ) && ((tU32)elem->len > max_data))
		{
			// would overflow the caller's buffer, abort
			ASSERT_ON_DEBUGGING(0);
			ret = OSAL_ERROR;
		}
		else
		{
			if (data != NULL)
			{
				OSAL_pvMemoryCopy((tPVoid)data, (tPCVoid)elem->data, elem->len);
			}
			if (head != NULL)
			{
				OSAL_pvMemoryCopy((tPVoid)head, (tPCVoid)&(elem->head), sizeof(CtrlAppMessageHeaderType));
			}
			if (len != NULL)
			{
				*len = (tU32)elem->len;
			}
		}
		elem->used = FALSE;
		fifo->readIndex += (tU8)1;
		fifo->readIndex %= PROTOCOL_LAYER_FIFO_SIZE;
	}

	OSAL_s32SemaphorePost(port->CommSem);
	return ret;
}

/*************************************************************************************/
/*                                                                                   */
/* void InterLayerProtocolInit ( tS32 index )                                     */
/*                                                                                   */
/*************************************************************************************/

void InterLayerProtocolInit ( tS32 index )
{
  PortInfo *port = &TcpIpPortList[index];

  if (index < 0)
  {
  	ASSERT_ON_DEBUGGING(0);
	return;
  }

  port->ProtocolLayer_DataBufferBytesNum = 0 ;
  port->FromCtrlAppParseStatus  = GUI_PARSE_IDLE ;
  ProtocolLayer_FifoReset(&port->ProtocolLayerFifo);

}

/********************************************************************************************/
/*                                                                                          */
/* tSInt CheckCtrlAppMessage_CMOST                                                          */
/*                                                                                          */
/********************************************************************************************/
tSInt CheckCtrlAppMessage_CMOST(tU8 cmd, tS32 index, CtrlAppMessageHeaderType *CtrlAppMessageHeader)
{
	PortInfo *port;

	if (index < 0)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	if ((CtrlAppMessageHeader->CtrlAppSync == PL_TARGET_MESSAGE_SYNC_BYTE) &&
		(CtrlAppMessageHeader->CtrlAppLunId == INVALID_LUN) &&
		(CtrlAppMessageHeader->CtrlAppSpare_0 == (tU8)0) &&
		(CtrlAppMessageHeader->CtrlAppSpare_1 == (tU8)0) &&
		(CtrlAppMessageHeader->CtrlAppSpare_2 == (tU8)0) &&
		(CtrlAppMessageHeader->CtrlAppSpare_3 == (tU8)0))
	{
		port = &TcpIpPortList[index];

		switch (cmd)
		{
			case PL_CMDTYPE_COMMAND_CMOST:
			case PL_CMDTYPE_READ_CMOST:
			case PL_CMDTYPE_WRITE_CMOST:
			case PL_CMDTYPE_READDMA_CMOST:
			case PL_CMDTYPE_WRITEDMA_CMOST:
			case PL_CMDTYPE_GENERIC_CMOST:
				// the response is in CMOST protocol format, nothing to check here
				return OSAL_OK;

			case PL_CMDTYPE_BOOT_CMOST:
				if ((port->ProtocolLayer_DataBufferBytesNum == PL_RESPLEN_BOOT_CMOST) &&
					(port->ProtocolLayer_DataBuffer[0] == PL_RESPCODE_OK_CMOST))
				{
					return OSAL_OK;
				}
				break;

			case PL_CMDTYPE_RESET_CMOST:
				if ((port->ProtocolLayer_DataBufferBytesNum == PL_RESPLEN_RESET_CMOST) &&
					(port->ProtocolLayer_DataBuffer[0] == PL_RESPCODE_RESETOK_CMOST))
				{
					return OSAL_OK;
				}
				break;

			default:
				return OSAL_ERROR;
		}
	}
	return OSAL_ERROR;
}

/********************************************************************************************/
/*                                                                                          */
/* void CtrlAppMessageHandle                                                                */
/*                                                                                          */
/********************************************************************************************/

void CtrlAppMessageHandle ( PortInfo *port )
{
  tU32  BytesInBuffer ;
  tU8 CtrlAppMessageReadyFlag ;
  tU16 retry;
  tSInt retval = OSAL_ERROR; /* avoid splint warning */

  /* Handle a Control Application Incoming message */

  BytesInBuffer = 0 ;

  while ( TRUE )
  {
          /* Read and parse the incoming message */

          CtrlAppMessageReadyFlag = FromCtrlAppMessageParse ( port, &(port->CtrlAppMessageHeader), &BytesInBuffer ) ;

          if ( PL_TARGET_MESSAGE_SYNC_BYTE == CtrlAppMessageReadyFlag )
		  {
            // pass data to ETAL
            retry = IPF_FIFO_FULL_RETRY;
			while (retry > 0)
			{
                retval = ProtocolLayer_FifoPush(port->PortIdx, &(port->CtrlAppMessageHeader));
                if ((retval == OSAL_ERROR) && (--retry > 0))
                {
                    OSAL_s32ThreadWait(5);
                    IPF_tracePrintComponent(TR_CLASS_APP_IPFORWARD, "IPF to ETAL FIFO retry");
                }
                else
                {
                    break;
                }
            }
            if (retval == OSAL_ERROR)
            {
                IPF_tracePrintError(TR_CLASS_APP_IPFORWARD, "IPF to ETAL FIFO full");
                continue;
            }
          }

          if ( 0 == BytesInBuffer )
		  {
               break ; /* No more data to process */
          }

          OSAL_s32ThreadWait ( 1 ) ;

  }
}

/****************************************************************************************************/
/*                                                                                                  */
/* tBool FromCtrlAppMessageParse                                                                    */
/*                                                                                                  */
/****************************************************************************************************/

static tU8 FromCtrlAppMessageParse ( PortInfo *port, CtrlAppMessageHeaderType *HeaderPnt , tU32* RemainingBytesPnt )
{
  tU8 TempByte ;

  while ( 0 != port -> PortBufferBytesNum )
  {
          /* Extract a byte from buffer */

          TempByte = TpcIpProtocolReadBufferData ( port , RemainingBytesPnt ) ;

          switch ( port->FromCtrlAppParseStatus )
		  {
                   case GUI_PARSE_IDLE :
                        if ( PL_TARGET_MESSAGE_SYNC_BYTE == TempByte )
						{

                             port->FromCtrlAppParseStatus = GUI_GET_LUN_ID ;
                             HeaderPnt -> CtrlAppSync       = TempByte ;
                             HeaderPnt -> CtrlAppLunId      = INVALID_LUN ;
                             HeaderPnt -> CtrlAppSpare_0    = (tU8)0 ;
                             HeaderPnt -> CtrlAppSpare_1    = (tU8)0 ;
                             HeaderPnt -> CtrlAppSpare_2    = (tU8)0 ;
                             HeaderPnt -> CtrlAppSpare_3    = (tU8)0 ;
                             HeaderPnt -> CtrlAppCmdDataLen = (tU8)0 ;
                             port->FromCtrlAppDataFieldLen = 0 ;
                        }
                        else {
                             HeaderPnt -> CtrlAppSync = (tU8)0 ;
                        }
                   break ;
                   case GUI_GET_LUN_ID :
                        HeaderPnt -> CtrlAppLunId = TempByte ;
                        port->FromCtrlAppParseStatus = GUI_GET_SPARE_0 ;
                   break ;

                   case GUI_GET_SPARE_0 :
                        HeaderPnt -> CtrlAppSpare_0 = TempByte ;
                        port->FromCtrlAppParseStatus = GUI_GET_SPARE_1 ;
                   break ;

                   case GUI_GET_SPARE_1 :
                        HeaderPnt -> CtrlAppSpare_1 = TempByte ;
                        port->FromCtrlAppParseStatus = GUI_GET_SPARE_2 ;
                   break ;

                   case GUI_GET_SPARE_2 :
                        HeaderPnt -> CtrlAppSpare_2 = TempByte ;
                        port->FromCtrlAppParseStatus = GUI_GET_SPARE_3 ;
                   break ;

                   case GUI_GET_SPARE_3 :
                        HeaderPnt -> CtrlAppSpare_3 = TempByte ;
                        port->FromCtrlAppParseStatus = GUI_GET_DATA_LEN ;
                        port->FromCtrlAppDataFieldLen = (tU16)sizeof ( HeaderPnt -> CtrlAppCmdDataLen ) ; /* Size of ID field */
                   break ;
                   case GUI_GET_DATA_LEN :

                        HeaderPnt -> CtrlAppCmdDataLen = ( HeaderPnt -> CtrlAppCmdDataLen << 8 ) | TempByte ;
                        port->FromCtrlAppDataFieldLen-- ;

                        if ( 0 == port->FromCtrlAppDataFieldLen )
						{

                             /* Data len field end */

                             if ( 0 != HeaderPnt -> CtrlAppCmdDataLen )
							 {
                                  port->FromCtrlAppParseStatus = GUI_GET_DATA ;
                                  port->FromCtrlAppDataFieldLen = HeaderPnt -> CtrlAppCmdDataLen ;
                                  port->ProtocolLayer_DataBufferBytesNum = 0 ;
                             }
                             else {
                                  /* No data expected for the current message: exit */

                                  port->FromCtrlAppParseStatus = GUI_PARSE_IDLE ;

                                  HeaderPnt -> CtrlAppSync = (tU8)0 ;

                                  return (tU8)0 ;
                             }
                        }
                   break ;
                   case GUI_GET_DATA :

                        if ( port->ProtocolLayer_DataBufferBytesNum < (tU16)sizeof ( port->ProtocolLayer_DataBuffer ) )
						{
                             port->ProtocolLayer_DataBuffer [ port->ProtocolLayer_DataBufferBytesNum++ ] = TempByte ;
                        }

                        port->FromCtrlAppDataFieldLen-- ;

                        if ( 0 == port->FromCtrlAppDataFieldLen )
						{

                             /* No more data expected for the current message */

                             port->FromCtrlAppParseStatus = GUI_PARSE_IDLE ;

                             TempByte = HeaderPnt -> CtrlAppSync ;
                             //HeaderPnt -> CtrlAppSync = 0 ;

                             return ( TempByte ) ; /* Message completed */
                        }
                   break ;
          }
  }

  return (tU8)0 ;

}

/********************************************************************************************************/
/*                                                                                                      */
/* void ForwardPacketToCtrlAppPort                                                                      */
/*                                                                                                      */
/********************************************************************************************************/

/* TODO this function has the same name as a function implemented in the interlayer_protocol.cpp 
 * file of the STECI repository but different parameters.
 * IT IS NOT COMPATIBLE WITH THAT FUNCTION
 * If this function is called from CONFIG_COMM_DRIVER_PROTOCOL_LAYER code it will not compile
 */
void ForwardPacketToCtrlAppPort ( tS32 index, tU8 *DataPnt , tU32 BytesNum , tU8 SyncByte , tU8 LunId, tU8 h0, tU8 h1, tU8 h2, tU8 h3)
{
  tU8 *OutBufPnt, *OutBuffer;
  PortInfo *port;

  if ((index < 0) || (BytesNum > PROTOCOL_LAYER_INTERNAL_BUFFER_LEN))
  {
      ASSERT_ON_DEBUGGING(0);
	  return;
  }

  port = &TcpIpPortList[index];
  OutBuffer = port->ProtocolLayer_DataBuffer; // was using PROTOCOL_LAYER_TO_DEVICE_BUFFER_LEN bytes
  OutBufPnt = OutBuffer;

  *OutBufPnt = SyncByte ;
  OutBufPnt += 1 ;
  *OutBufPnt = LunId ; ;
  OutBufPnt += 1 ;

  *OutBufPnt = h0;
  OutBufPnt += 1;
  *OutBufPnt = h1;
  OutBufPnt += 1;
  *OutBufPnt = h2;
  OutBufPnt += 1;
  *OutBufPnt = h3;
  OutBufPnt += 1;

  PutBEWordOnBuffer ( OutBufPnt , (tU16)BytesNum ) ;
  OutBufPnt += 2 ;

  if ((BytesNum > 0) && (DataPnt != NULL))
  {
      OSAL_pvMemoryCopy((tVoid *)OutBufPnt, (tPCVoid)DataPnt, BytesNum);
      OutBufPnt += BytesNum;
  }

  TcpIpProtocolSendData ( index , OutBuffer , (tU32)( OutBufPnt - OutBuffer ) ) ;
}

#endif // CONFIG_COMM_DRIVER_EXTERNAL

