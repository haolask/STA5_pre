/****************************************************************************/
/*                                                                          */
/* Project:              ADR3 Control application                           */
/* Filename:             GUI_Protocol.c                                     */
/* Programmer:           Alessandro Vaghi                                   */
/* Date:                 Jun 18th 2010                                      */
/* CPU:                  PC                                                 */
/* Type:                 C code                                             */
/* Scope:                TCP/IP GUI protocol handling funcyions             */
/* Functions:                                                               */
/*                                                                          */
/****************************************************************************/
#include "osal.h"

#ifdef CONFIG_COMM_DRIVER_EXTERNAL

#include <sys/types.h>
#ifndef CONFIG_HOST_OS_WIN32
	/* Win32 provides this in <winsock2.h>, already included */
	#include <netinet/tcp.h>  // for TCP_NODELAY
#endif
#include <fcntl.h>
#include <time.h>

#include "etaldefs.h"
#include "ipfcomm.h"
#include "TcpIpProtocol.h"
#include "ctrl_app.h"

#ifdef CONFIG_HOST_OS_LINUX
	#define SOCKET_ERROR		-1
	#define INVALID_SOCKET		-1
	#define ERROR				-1
	typedef struct sockaddr SOCKADDR;
#endif

/* TCP/IP protocol handling status machine */

#define TCP_IP_PROTOCOL_STARTUP             0
#define TCP_IP_PROTOCOL_LINK                1
#define TCP_IP_PROTOCOL_CONNECT             2
#define TCP_IP_PROTOCOL_GET_DATA            3
#define TCP_IP_PROTOCOL_FORWARD_DATA        4
#define TCP_IP_PROTOCOL_DISCONNECT_HANDLE   5
#define TCP_IP_PROTOCOL_EXIT                6

static void  TcpIpProtocolClose ( PortInfo *PortPnt ) ;
static tS32  TcpIpProtocolConnect ( PortInfo *PortPnt ) ;
static void  TcpIpProtocolHandle ( void* pParams ) ;
static tS32  TcpIpProtocolInit ( PortInfo *PortPnt ) ;
static tS32  TcpIpProtocolLink ( PortInfo *PortPnt ) ;
static tS16  TcpIpProtocolGetData ( PortInfo *PortPnt ) ;

/************************************************************************************************/
/*                                                                                              */
/* tS32 TcpIpProtocolInit ( PortInfo *PortPnt )                                                 */
/*                                                                                              */
/************************************************************************************************/

static tS32 TcpIpProtocolInit ( PortInfo *PortPnt )
{
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
  OSAL_s32NPrintFormat ( PortPnt->LogBuffer , LOG_BUFFER_SIZE, "TcpIpProtocolInit - Port: %d - Client" , PortPnt -> TcpIpPortNumber ) ;
  LogString ( PortPnt, PortPnt->LogBuffer , LOG_MASK_MDR_TCP_IP ) ;
#endif

#ifdef CONFIG_HOST_OS_WIN32
  if ( NO_ERROR != WSAStartup ( MAKEWORD ( 2 , 2 ) , &PortPnt -> TcpIpWsdata ) )
  {
       LogString ( PortPnt, (char *) "TcpIpProtocolInit - socket init error" , LOG_MASK_MDR_TCP_IP ) ;
       WSACleanup() ;
       return ( -1 ) ;
  }
#endif

  PortPnt -> TcpIpClientServer = socket ( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;

  if ( INVALID_SOCKET == PortPnt -> TcpIpClientServer )
  {
       LogString ( PortPnt, (char *) "TcpIpProtocolInit - invalid socket" , LOG_MASK_MDR_TCP_IP ) ;
#ifdef CONFIG_HOST_OS_WIN32
       WSACleanup() ;
#endif
       return ( -1 ) ;
  }

  /* Define server parameters */

  PortPnt -> TcpIpClientServerParams . sin_family = AF_INET ;

  PortPnt -> TcpIpClientServerParams . sin_port = htons ( PortPnt -> TcpIpPortNumber ) ;

  return ( 0 ) ;

}

/************************************************************************************************/
/*                                                                                              */
/* tS32 TcpIpProtocolLink ( PortInfo *PortPnt )                                                 */
/*                                                                                              */
/************************************************************************************************/

static tS32 TcpIpProtocolLink ( PortInfo *PortPnt )
{
	tSInt yesFlag = 1;

  if (setsockopt(PortPnt->TcpIpClientServer, IPPROTO_TCP, TCP_NODELAY, (char *)&yesFlag, sizeof(int)) < 0)
  {
     LogString ( PortPnt, (char *) "TcpIpProtocolInit - setsockopt error" , LOG_MASK_MDR_TCP_IP ) ;
	 return -1;
  }

  return ( 0 ) ;

}

/************************************************************************************************/
/*                                                                                              */
/* void TcpIpProtocolClose ( PortInfo *PortPnt )                                                 */
/*                                                                                              */
/************************************************************************************************/

static void TcpIpProtocolClose ( PortInfo *PortPnt )
{
  if ( -1 != PortPnt -> TcpIpPortNumber )
  {
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
  OSAL_s32NPrintFormat ( PortPnt->LogBuffer , LOG_BUFFER_SIZE, "TcpIpProtocolClose - Port: %d" , PortPnt -> TcpIpPortNumber ) ;
  LogString ( PortPnt, PortPnt->LogBuffer , LOG_MASK_MDR_TCP_IP ) ;
#endif

  PortPnt -> TerminateFlag = TRUE ;

  PortPnt -> ConnectFlag = FALSE ;

#ifdef CONFIG_HOST_OS_LINUX
  shutdown(PortPnt -> TcpIpClientServer, SHUT_RDWR);
#else
  shutdown(PortPnt -> TcpIpClientServer, SD_BOTH);
  closesocket ( PortPnt -> TcpIpClientServer ) ;
  WSACleanup() ;
#endif
  }

}

/************************************************************************************************/
/*                                                                                              */
/* void TcpIpProtocolEnd ( void )                                                               */
/*                                                                                              */
/************************************************************************************************/

void TcpIpProtocolEnd ( tS32 PortIdx )
{
  PortInfo *PortInfoPnt ;
  char name[ETAL_THREAD_BASE_NAME_MAX_LEN];

  if (PortIdx <  0)
  {
	return;
  }
  PortInfoPnt = &TcpIpPortList [ PortIdx ] ;
  if ( -1 != TcpIpPortList [ PortIdx ] . TcpIpPortNumber )
  {
     TcpIpProtocolClose ( &TcpIpPortList [ PortIdx ] );
     OSAL_s32ThreadWait ( 10 ) ;
     (LINT_IGNORE_RET) OSAL_s32ThreadDelete(TcpIpPortList [ PortIdx ] . ThreadId);
     if (OSAL_s32NPrintFormat(name, ETAL_THREAD_BASE_NAME_MAX_LEN, "Sem_Ipf%d", PortInfoPnt -> TcpIpPortNumber) < 0)
     {
        return;
     }
     if (OSAL_s32SemaphoreClose(PortInfoPnt->CommSem) == OSAL_ERROR)
     {
        return;
     }
	 else
	 {
        (LINT_IGNORE_RET) OSAL_s32SemaphoreDelete(name);
	 }
   }
}

/************************************************************************************************/
/*                                                                                              */
/* tSInt TcpIpProtocolSetup ( tS32 PortIdx )                                                    */
/*                                                                                              */
/************************************************************************************************/

tSInt TcpIpProtocolSetup ( tS32 PortIdx )
{
  PortInfo *PortInfoPnt ;
  OSAL_trThreadAttribute thread_attr;
  char name[ETAL_THREAD_BASE_NAME_MAX_LEN];

  ASSERT_ON_DEBUGGING(PortIdx >= 0);

  PortInfoPnt = &TcpIpPortList [ PortIdx ] ;

  if (OSAL_s32NPrintFormat(name, ETAL_THREAD_BASE_NAME_MAX_LEN, "Sem_Ipf%d", PortInfoPnt -> TcpIpPortNumber) < 0)
  {
     return OSAL_ERROR;
  }
  if (OSAL_s32SemaphoreCreate(name, &PortInfoPnt->CommSem, 1) == OSAL_ERROR)
  {
     return OSAL_ERROR;
  }

  PortInfoPnt->PortStatus       = TCP_IP_PROTOCOL_STARTUP ;
  PortInfoPnt->PortIdx          = PortIdx ;

  if ( PortInfoPnt -> TcpIpPortNumber <= 0x10000)
  {
	if (OSAL_s32NPrintFormat(name, ETAL_THREAD_BASE_NAME_MAX_LEN, "%s%d", ETAL_IPFORWARD_THREAD_BASE_NAME, PortInfoPnt->TcpIpPortNumber) < 0)
    {
       return OSAL_ERROR;
    }

	thread_attr.szName = name;
	thread_attr.u32Priority = ETAL_IPFORWARD_THREAD_PRIORITY ;
	thread_attr.s32StackSize = ETAL_IPFORWARD_STACK_SIZE;
	thread_attr.pfEntry = TcpIpProtocolHandle;
	thread_attr.pvArg = (tPVoid)PortInfoPnt;
	if ((PortInfoPnt->ThreadId = OSAL_ThreadSpawn(&thread_attr)) == OSAL_ERROR)
	{
		return OSAL_ERROR;
	}
  }

  return OSAL_OK;

}

/*************************************************************************************/
/*                                                                                   */
/* tS32 TcpIpProtocolConnect ( PortInfo *PortPnt )                                   */
/*                                                                                   */
/*************************************************************************************/

static tS32 TcpIpProtocolConnect ( PortInfo *PortPnt )
{
  /* The current port is a CLIENT: connect to a server */

  PortPnt -> TcpIpConnect = connect ( PortPnt -> TcpIpClientServer ,
                                           (SOCKADDR*)&PortPnt -> TcpIpClientServerParams ,
                                           sizeof ( PortPnt -> TcpIpClientServerParams ) ) ;

 if ( SOCKET_ERROR != PortPnt -> TcpIpConnect )
 {
       return ( 0 ) ;
 }

  return ( -1 ) ;
}

/****************************************************************************************************/
/*                                                                                                  */
/* void TcpIpProtocolGetData                                                                        */
/*                                                                                                  */
/****************************************************************************************************/

static tS16 TcpIpProtocolGetData ( PortInfo *PortPnt )
{
  tS16 ByteCount ;
  tS16 Retco ;
  tU8 *SrcBufferPnt ;
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
  char TempLogBuffer [ 20 ] ;
  int TcpIpLogBuffer_used;
#endif

  ByteCount = recv ( PortPnt -> TcpIpClientServer , (char *)PortPnt->TempPortBuffer , PORT_BUFFER_SIZE , 0 ) ;

  Retco = ByteCount ;

  if ( ByteCount < 0 )
  {
       Retco = -1 ;
       IPF_tracePrintSysmin(TR_CLASS_APP_IPFORWARD, "recv returned no data" ) ;
  }
  else if ( 0 != ByteCount )
  {
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
       OSAL_s32NPrintFormat ( PortPnt->LogBuffer , LOG_BUFFER_SIZE, "Rx %d bytes from %d port " , ByteCount , PortPnt -> TcpIpPortNumber ) ;
       TcpIpLogBuffer_used = strlen(PortPnt->LogBuffer);
#endif

       /* GUI port: store data into the relevant circular buffer */

       SrcBufferPnt = PortPnt->TempPortBuffer  ;

       while ( ByteCount-- > 0 )
	   {

               if ( PortPnt -> PortBufferBytesNum < PORT_BUFFER_SIZE )
			   {

#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
                    OSAL_s32NPrintFormat ( TempLogBuffer , 20, " %.2X" , *SrcBufferPnt ) ;
					if (TcpIpLogBuffer_used + 3 < LOG_BUFFER_SIZE)
					{
                        strcat ( PortPnt->LogBuffer , TempLogBuffer ) ;
                        TcpIpLogBuffer_used += 3;
                    }
#endif

                    PortPnt -> PortBuffer [ PortPnt -> PortBufferProd++ ] = *SrcBufferPnt++ ;
                    PortPnt -> PortBufferProd &= ( PORT_BUFFER_SIZE - 1 ) ; /* ok if power of 2 */

                    PortPnt -> PortBufferBytesNum++ ;
                    PortPnt -> TotalByteCount++ ;
               }
       }
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
       LogString ( PortPnt, PortPnt->LogBuffer , LOG_MASK_MDR_TCP_IP ) ;
       PortPnt->LogBuffer[0] = '\0';
#endif
       IPF_tracePrintComponent(TR_CLASS_APP_IPFORWARD, "Rx %d bytes from %d port ", Retco , PortPnt->TcpIpPortNumber) ;
  }

  return ( Retco ) ;

}

/************************************************************************************************/
/*                                                                                              */
/* void TcpIpProtocolSendData ( tS32 TcpIpInterfaceIdx , tU8 *DataToSend , tU32 BytesNum )      */
/*                                                                                              */
/************************************************************************************************/

void TcpIpProtocolSendData ( tS32 TcpIpInterfaceIdx , tU8 *DataToSend , tU32 BytesNum )
{
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
  char TempLogBuffer [ 20 ] ;
  int TcpIpLogBuffer_used = 0;
#endif
  PortInfo *PortPnt = &TcpIpPortList [ TcpIpInterfaceIdx ] ;

  if (TcpIpInterfaceIdx < 0)
  {
  	ASSERT_ON_DEBUGGING(0);
	return;
  }

  if ( TRUE == PortPnt->ConnectFlag )
  {
       if (send ( PortPnt->TcpIpClientServer , (char *)DataToSend , BytesNum , 0 ) < 0)
       {
           IPF_tracePrintError(TR_CLASS_APP_IPFORWARD, "Sending data to TCP port (%s)", strerror(errno));
       }
       else
       {
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
		   sprintf ( PortPnt->LogBuffer , "Tx %d bytes to %d port:" , BytesNum , PortPnt->TcpIpPortNumber ) ;
		   TcpIpLogBuffer_used = strlen(PortPnt->LogBuffer);

		   while ( (0 != BytesNum--) &&  (TcpIpLogBuffer_used + 3 < LOG_BUFFER_SIZE))
		   {
				   OSAL_s32NPrintFormat ( TempLogBuffer , 20, " %.2X" , *DataToSend++ ) ;
				   strcat ( PortPnt->LogBuffer , TempLogBuffer ) ;
				   TcpIpLogBuffer_used += 3;
		   }

		   LogString ( PortPnt, PortPnt->LogBuffer , LOG_MASK_MDR_TCP_IP ) ;
		   PortPnt->LogBuffer[0] = '\0';
#endif
		   IPF_tracePrintComponent(TR_CLASS_APP_IPFORWARD, "Tx %d bytes to %d port ", BytesNum, PortPnt->TcpIpPortNumber) ;
       }
  }

}

/********************************************************************************************/
/*                                                                                          */
/* tU8 TpcIpProtocolReadBufferData                                                          */
/*                                                                                          */
/********************************************************************************************/

tU8 TpcIpProtocolReadBufferData ( PortInfo *PortPnt, tU32 *RemainingBytesPnt )
{
  tU8 TempByte ;

  if ( 0 == PortPnt -> PortBufferBytesNum )
  {
       if ( NULL != RemainingBytesPnt )
	   {
            *RemainingBytesPnt = 0 ;
       }

       return ( (tU8)0 ) ;
  }
  else {
       TempByte = PortPnt -> PortBuffer [ PortPnt -> PortBufferCons++ ] ;
       PortPnt -> PortBufferCons &= ( PORT_BUFFER_SIZE - 1 ) ; /* ok if power of 2 */
       PortPnt -> PortBufferBytesNum-- ;

       if ( NULL != RemainingBytesPnt )
	   {
            *RemainingBytesPnt = PortPnt -> PortBufferBytesNum ;
       }

       return ( TempByte ) ;
  }

}

/************************************************************************************************/
/*                                                                                              */
/* void TcpIpProtocolHandle ( void )                                                            */
/*                                                                                              */
/************************************************************************************************/

#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid TcpIpProtocolHandle(tSInt stacd, tPVoid pParams)
#else
static void TcpIpProtocolHandle ( void* pParams )
#endif
{
  PortInfo *TcpIpPortPnt ;
  tS16  GetDataRetco ;

  TcpIpPortPnt = (PortInfo *)pParams ;

  TcpIpPortPnt -> TerminateFlag = FALSE ;

  while ( TRUE )
  {

          if ( TRUE == TcpIpPortPnt -> TerminateFlag )
		  {
               break ;
          }

          switch ( TcpIpPortPnt -> PortStatus )
		  {

                   case TCP_IP_PROTOCOL_STARTUP :

                        /* Initialize TCP/IP port */

                        if ( 0 == TcpIpProtocolInit ( TcpIpPortPnt ) )
						{
                             TcpIpPortPnt -> PortStatus = TCP_IP_PROTOCOL_LINK ;
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
                             OSAL_s32NPrintFormat ( TcpIpPortPnt->LogBuffer , LOG_BUFFER_SIZE, "TCP_IP_PROTOCOL_STARTUP exit" );
                             LogString ( TcpIpPortPnt, TcpIpPortPnt->LogBuffer , LOG_MASK_MDR_TCP_IP ) ;
#endif
                        }
                        else {
                             OSAL_s32ThreadWait ( 1000 ) ;
                        }
                   break ;

                   case TCP_IP_PROTOCOL_LINK :
                        if ( 0 == TcpIpProtocolLink ( TcpIpPortPnt ) )
						{
                             TcpIpPortPnt -> PortStatus = TCP_IP_PROTOCOL_CONNECT ;
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
                             OSAL_s32NPrintFormat ( TcpIpPortPnt->LogBuffer , LOG_BUFFER_SIZE, "TCP_IP_PROTOCOL_LINK exit" );
                             LogString ( TcpIpPortPnt, TcpIpPortPnt->LogBuffer , LOG_MASK_MDR_TCP_IP ) ;
#endif
                        }
                        else {
                             OSAL_s32ThreadWait ( 1000 ) ;
                        }

                   break ;

                   case TCP_IP_PROTOCOL_CONNECT :

                        if ( 0 == TcpIpProtocolConnect ( TcpIpPortPnt ) )
						{

#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
                             if ( 0 != TcpIpPortPnt -> PortName [ 0 ] )
							 {
                                  OSAL_s32NPrintFormat ( TcpIpPortPnt->LogBuffer , LOG_BUFFER_SIZE, "Port %s [%d] connection established" ,
                                            TcpIpPortPnt -> PortName , TcpIpPortPnt -> TcpIpPortNumber ) ;
                             }
                             else {
                                  OSAL_s32NPrintFormat ( TcpIpPortPnt->LogBuffer , LOG_BUFFER_SIZE, "%d port connection established" , TcpIpPortPnt -> TcpIpPortNumber ) ;
                             }

                             LogString ( TcpIpPortPnt, TcpIpPortPnt->LogBuffer , LOG_MASK_MDR_TCP_IP ) ;
#endif

                             TcpIpPortPnt -> PortStatus = TCP_IP_PROTOCOL_GET_DATA ;
                             TcpIpPortPnt -> ConnectFlag = TRUE ;
                        }
                        else {
                             TcpIpPortPnt -> PortStatus = TCP_IP_PROTOCOL_STARTUP ;
                             OSAL_s32ThreadWait ( 1000 ) ;
                        }
                   break ;

                   case TCP_IP_PROTOCOL_GET_DATA :

                        GetDataRetco = TcpIpProtocolGetData ( TcpIpPortPnt ) ;

                        if ( -1 == GetDataRetco )
						{

#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
                             OSAL_s32NPrintFormat ( TcpIpPortPnt->LogBuffer , LOG_BUFFER_SIZE, "%s connection lost" , TcpIpPortPnt -> PortName ) ;
                             LogString ( TcpIpPortPnt, TcpIpPortPnt->LogBuffer , LOG_MASK_MDR_TCP_IP ) ;
#endif
                             TcpIpPortPnt -> PortStatus = TCP_IP_PROTOCOL_LINK ;

                             OSAL_s32ThreadWait ( 1000 ) ;
                        }
                        else if ( GetDataRetco > 0 )
						{

#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
                             if ( 0 != TcpIpPortPnt -> PortName [ 0 ] )
							 {
                                  OSAL_s32NPrintFormat ( TcpIpPortPnt->LogBuffer , LOG_BUFFER_SIZE, "Data from port %s [%d]" , TcpIpPortPnt -> PortName ,
                                            TcpIpPortPnt -> TcpIpPortNumber ) ;
                             }
                             else {
                                  OSAL_s32NPrintFormat ( TcpIpPortPnt->LogBuffer , LOG_BUFFER_SIZE, "Data from port %d" , TcpIpPortPnt -> TcpIpPortNumber ) ;
                             }

                             LogString ( TcpIpPortPnt, TcpIpPortPnt->LogBuffer , LOG_MASK_MDR_TCP_IP ) ;
#endif

                             CtrlAppMessageHandle ( TcpIpPortPnt ) ;
                        }
                        else {
                             OSAL_s32ThreadWait ( 10 ) ;
                        }
                   break ;

                   case TCP_IP_PROTOCOL_EXIT :
                        LogString ( TcpIpPortPnt, (char *)"TCP/IP Protocol exit" , LOG_MASK_MDR_TCP_IP ) ;

                        TcpIpProtocolClose ( TcpIpPortPnt ) ;
                        return ;
          }
  }

}

/************************************************************************************************/
/*                                                                                              */
/* tBool TcpIpProtocolIsConnected                                                               */
/*                                                                                              */
/************************************************************************************************/
tBool TcpIpProtocolIsConnected ( tS32 index )
{
	if (index < 0)
	{
		//ASSERT_ON_DEBUGGING(0);
		return FALSE;
	}
	return TcpIpPortList[index].ConnectFlag;
}

#endif // CONFIG_COMM_DRIVER_EXTERNAL

