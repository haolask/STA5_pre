/****************************************************************************/
/*                                                                          */
/* Project:              MDR3 Control application                           */
/* Filename:             main.c                                             */
/* Programmer:           Alessandro Vaghi                                   */
/* Date:                 Jun 18th 2010                                      */
/* CPU:                  PC                                                 */
/* Type:                 C code                                             */
/* Scope:                Main module                                        */
/* Functions:            main                                               */
/*                                                                          */
/****************************************************************************/

#include "osal.h"

#ifdef CONFIG_COMM_DRIVER_EXTERNAL

#include "etaldefs.h"
#include "ipfcomm.h"
#include "TcpIpProtocol.h"
#include "ctrl_app.h"

PortInfo TcpIpPortList [ MAX_PORT_NUMBER ] ;
tS32     ProtocolLayerIndexCMOST = PROTOCOL_LAYER_INDEX_INVALID;
tS32     ProtocolLayerIndexDCOP = PROTOCOL_LAYER_INDEX_INVALID;
static tS32 TotalTcpIpPortNumber;

/*************************************************************************************/
/*                                                                                   */
/* ipforward_init                                                                    */
/*                                                                                   */
/*************************************************************************************/

static tSInt ipforward_init(tChar *IPaddress, tU32 port, tU32 log_mask, tChar *log_filename, tS32 *index)
{
  ConfigParameters ( IPaddress, port, log_mask, log_filename, TotalTcpIpPortNumber ) ;

  OpenLogSession(TotalTcpIpPortNumber) ;

  InterLayerProtocolInit(TotalTcpIpPortNumber) ;

  if (TcpIpProtocolSetup(TotalTcpIpPortNumber) != OSAL_OK)
  {
     return OSAL_ERROR;
  }
  *index = TotalTcpIpPortNumber;
  TotalTcpIpPortNumber++;
  return OSAL_OK;
}

/*************************************************************************************/
/*                                                                                   */
/* ipforwardDeinit                                                                  */
/*                                                                                   */
/*************************************************************************************/

tSInt ipforwardDeinit (tS32 index)
{
	if (index == IPF_DEINIT_ALL)
	{
		if (ProtocolLayerIndexDCOP >= 0)
		{
			TcpIpProtocolEnd(ProtocolLayerIndexDCOP);
			ProtocolLayerIndexDCOP = PROTOCOL_LAYER_INDEX_INVALID;
		}
		if (ProtocolLayerIndexCMOST >= 0)
		{
			TcpIpProtocolEnd(ProtocolLayerIndexCMOST);
			ProtocolLayerIndexCMOST = PROTOCOL_LAYER_INDEX_INVALID;
		}
		TotalTcpIpPortNumber = 0;
	}
	else if (index != PROTOCOL_LAYER_INDEX_INVALID)
	{
		TotalTcpIpPortNumber--;
		if (TotalTcpIpPortNumber < 0)
		{
			TotalTcpIpPortNumber = 0;
		}
		TcpIpProtocolEnd(index);
		if (index == ProtocolLayerIndexCMOST)
		{
			ProtocolLayerIndexCMOST = PROTOCOL_LAYER_INDEX_INVALID;
		}
		else if (index == ProtocolLayerIndexDCOP)
		{
			ProtocolLayerIndexDCOP = PROTOCOL_LAYER_INDEX_INVALID;
		}
	}
	else
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***********************************
 *
 * ipforwardPowerUp
 *
 **********************************/
tVoid ipforwardPowerUp(tVoid)
{
	ProtocolLayerIndexCMOST = PROTOCOL_LAYER_INDEX_INVALID;
	ProtocolLayerIndexDCOP = PROTOCOL_LAYER_INDEX_INVALID;
}


/***********************************
 *
 * ipforwardInit
 *
 **********************************/
tSInt ipforwardInit(tChar *IPaddress, tU32 port, tU32 log_mask, tChar *logfile, tS32 *index)
{
	OSAL_tMSecond start_time, end_time;

	*index = PROTOCOL_LAYER_INDEX_INVALID;
	if (ipforward_init(IPaddress, port, log_mask, logfile, index) != OSAL_OK)
	{
		IPF_tracePrintFatal(TR_CLASS_APP_IPFORWARD, "TCP/IP thread or semaphore creation");
		return OSAL_ERROR_DEVICE_INIT;
	}
	start_time = OSAL_ClockGetElapsedTime();
	end_time = start_time + PROTOCOL_LAYER_CONNECT_TIMEOUT;

	/* wait until the TCP/IP thread connects to the ProtocolLayer */
	while (OSAL_ClockGetElapsedTime() < end_time)
	{
		if (TcpIpProtocolIsConnected(*index))
		{
			break;
		}
		OSAL_s32ThreadWait(PROTOCOL_LAYER_CONNECT_TEST_SCHEDULING);
	}

	if (!TcpIpProtocolIsConnected(*index))
	{
		IPF_tracePrintFatal(TR_CLASS_APP_IPFORWARD, "cannot connect to the Protocol Layer (is the protocol_layer.exe started?)");
		return OSAL_ERROR_DEVICE_NOT_OPEN;
	}
	return OSAL_OK;
}
#endif // CONFIG_COMM_DRIVER_EXTERNAL

