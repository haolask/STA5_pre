/****************************************************************************/
/*                                                                          */
/* Project:              ADR3 Control application                           */
/* Filename:             TcpIpProtocol.h                                    */
/* Programmer:           Alessandro Vaghi                                   */
/* Date:                 Jun 19, 2010                                       */
/* CPU:                  ST 10                                              */
/* Type:                 C include file                                     */
/* Scope:                                                                   */
/*                                                                          */
/****************************************************************************/

#define PORT_BUFFER_SIZE          4096 /* Power of 2 */
#define LOG_BUFFER_SIZE           PORT_BUFFER_SIZE /* should be 3xPORT_BUFFER_SIZE but we don't want to print all that stuff */

#define MAX_PORT_NUMBER              2
#define MAX_PORTNAME_LEN            50

typedef struct
{
	tBool used;
	tU8 data[PROTOCOL_LAYER_INTERNAL_BUFFER_LEN];
	tU32 len;
	CtrlAppMessageHeaderType head;
} tProtocolLayerFifoElem;

typedef struct
{
	tProtocolLayerFifoElem fifo[PROTOCOL_LAYER_FIFO_SIZE];
	tU8 writeIndex;
	tU8 readIndex;

} tProtocolLayerFifo;

typedef enum
{
    DEV_PROTOCOL_NONE       = 0,
    DEV_PROTOCOL_CMOST      = 1,
    DEV_PROTOCOL_SSI32      = 2,
    DEV_PROTOCOL_STECI      = 3,
	DEV_PROTOCOL_HDR		= 4,
	DEV_PROTOCOL_MCP		= 5,
    DEV_PROTOCOL_UNKNOWN    = 0xFF
} DEV_protocolEnumTy;

typedef struct
{
#ifdef CONFIG_HOST_OS_LINUX
	int         TcpIpClientServer ;
	struct sockaddr_in TcpIpClientServerParams ;
#else
	WSADATA     TcpIpWsdata ;
   	SOCKET      TcpIpClientServer ;
   	SOCKADDR_IN TcpIpClientServerParams ;
#endif
   	tS32        TcpIpConnect ;
   	tU32        TotalByteCount ;
   	tU16        PortStatus ;
   	tS32        PortIdx ;
   	tBool       ConnectFlag ; 
   	tBool       TerminateFlag ;
   	tU8         PortBuffer [ PORT_BUFFER_SIZE ] ;
   	tU8         TempPortBuffer [ PORT_BUFFER_SIZE ] ;
   	tU16        PortBufferBytesNum ;
   	tU16        PortBufferProd ;
   	tU16        PortBufferCons ;

   	tS32        TcpIpPortNumber ;
   	tChar       PortName [ MAX_PORTNAME_LEN ] ;
   	tProtocolLayerFifo ProtocolLayerFifo;

	tU16        FromCtrlAppParseStatus ;
	tU16        FromCtrlAppDataFieldLen;
	CtrlAppMessageHeaderType CtrlAppMessageHeader;
	tU8         ProtocolLayer_DataBuffer [ PROTOCOL_LAYER_INTERNAL_BUFFER_LEN ] ;
	tU32        ProtocolLayer_DataBufferBytesNum ;

#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
   	FILE       *LogFileFp;
   	tU32        LogMask;
   	tChar       LogFilename [ 100 ];
	tChar       LogBuffer  [ LOG_BUFFER_SIZE ] ;
#endif

	OSAL_tSemHandle  CommSem;
	OSAL_tThreadID   ThreadId;
} PortInfo ;

tBool TcpIpProtocolCheck ( void ) ;
void  TcpIpProtocolEnd ( tS32 index ) ;
void  TcpIpProtocolSendData ( tS32 index , tU8 *DataToSend , tU32 BytesNum ) ;
tSInt TcpIpProtocolSetup ( tS32 index ) ;
tU8   TpcIpProtocolReadBufferData ( PortInfo *PortPnt , tU32 *RemainingBytesPnt ) ;

extern PortInfo             TcpIpPortList [ MAX_PORT_NUMBER ] ;

