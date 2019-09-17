/****************************************************************************/
/*                                                                          */
/* Project:              ADR3 Control application                           */
/* Filename:             Utility.c                                          */
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

#include <fcntl.h>
#include <time.h>

#include "etaldefs.h"
#include "ipfcomm.h"
#include "TcpIpProtocol.h"
#include "ctrl_app.h"

#define PORT_NAME			"PROTOCOL LAYER"

/*************************************************************************************/
/*                                                                                   */
/* void ConfigParameters                                                             */
/*                                                                                   */
/*************************************************************************************/

void ConfigParameters ( tChar *IPaddress, tU32 tcp_port, tU32 log_mask, tChar *log_filename, tS32 index )
{
  PortInfo *PortInfoPnt ;

  if (index < 0)
  {
  	ASSERT_ON_DEBUGGING(0);
  	return;
  }

  PortInfoPnt = &TcpIpPortList[index];
  OSAL_pvMemorySet ( (tVoid *)PortInfoPnt , 0 , sizeof ( PortInfo ) ) ; // Invalidate data

#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
  PortInfoPnt->LogMask = log_mask ;
  strncpy ( PortInfoPnt->LogFilename , log_filename , sizeof ( PortInfoPnt->LogFilename ) ) ;
  PortInfoPnt->LogMask |= LOG_MASK_MDR_UTILITY  ;
#endif

#ifdef CONFIG_HOST_OS_LINUX
  PortInfoPnt -> TcpIpClientServerParams . sin_addr . s_addr = inet_addr ( IPaddress ) ;
#else
  PortInfoPnt -> TcpIpClientServerParams . sin_addr . S_un . S_addr = inet_addr ( IPaddress ) ;
#endif

  strncpy ( PortInfoPnt -> PortName , PORT_NAME , MAX_PORTNAME_LEN - 1 );
  PortInfoPnt -> TcpIpPortNumber  = (tS32)tcp_port ;

}

/*************************************************************************************/
/*                                                                                   */
/* tU32 GetBELongFromBuffer ( tU8 *Buffer )                                          */
/*                                                                                   */
/*************************************************************************************/
#if 0
static tU32 GetBELongFromBuffer ( tU8 *Buffer )
{
  tU32 LongToGet ;

  LongToGet = 0 ;

  LongToGet = *Buffer++ ;
  LongToGet = ( LongToGet << 8 ) + *Buffer++ ;
  LongToGet = ( LongToGet << 8 ) + *Buffer++ ;
  LongToGet = ( LongToGet << 8 ) + *Buffer ;

  return ( LongToGet ) ;

}
#endif
/*************************************************************************************/
/*                                                                                   */
/* tU16 GetBEWordFromBuffer ( tU8 *Buffer )                                          */
/*                                                                                   */
/*************************************************************************************/
#if 0
static tU16 GetBEWordFromBuffer ( tU8 *Buffer )
{
  tU16 WordToGet ;

  WordToGet = 0 ;

  WordToGet = *Buffer++ ;
  WordToGet = ( WordToGet << 8 ) + *Buffer ;

  return ( WordToGet ) ;

}
#endif
/*************************************************************************************/
/*                                                                                   */
/* void PutBELongOnBuffer ( tU8 *Buffer , tU32 LongToPut )                           */
/*                                                                                   */
/*************************************************************************************/

void PutBELongOnBuffer ( tU8 *Buffer , tU32 LongToPut )
{
  *Buffer++ = (tU8) ( LongToPut >> 24 ) ;
  *Buffer++ = (tU8) ( LongToPut >> 16 ) ;
  *Buffer++ = (tU8) ( LongToPut >> 8 ) ;
  *Buffer   = (tU8)LongToPut ;

}

/*************************************************************************************/
/*                                                                                   */
/* void PutBEWordOnBuffer ( tU8 *Buffer , tU16 WordToPut )                           */
/*                                                                                   */
/*************************************************************************************/

void PutBEWordOnBuffer ( tU8 *Buffer , tU16 WordToPut )
{
  *Buffer++ = (tU8) ( WordToPut >> 8 ) ;
  *Buffer   = (tU8)WordToPut ;

}

/************************************************************************************************/
/*                                                                                              */
/* void OpenLogSession ( tS32 index )                                                           */
/*                                                                                              */
/************************************************************************************************/

void OpenLogSession ( tS32 index )
{
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
  time_t DateAndTime ;
  FILE *LogFileFp;
  PortInfo *PortPnt = &TcpIpPortList [ index ];

  if (index < 0)
  {
  	ASSERT_ON_DEBUGGING(0);
  	return;
  }
  LogFileFp = fopen ( PortPnt->LogFilename , "w" ) ;

  if ( NULL != LogFileFp )
  {
    PortPnt->LogFileFp = LogFileFp;
    DateAndTime = time ( NULL ) ;
    sprintf ( PortPnt->LogBuffer , "MDR PROTOCOL APPLICATION - %s" , ctime ( &DateAndTime ) ) ;
    LogString ( PortPnt, PortPnt->LogBuffer, LOG_MASK_MDR_INTERLAYER ) ;
  }
  else {
    IPF_tracePrintError(TR_CLASS_APP_IPFORWARD, "Log file [%s] open error" , PortPnt->LogFilename ) ;
  }
#endif
}

/************************************************************************************************/
/*                                                                                              */
/* void CloseLogSession ( FILE *LogFileFp )                                                                */
/*                                                                                              */
/************************************************************************************************/

void CloseLogSession ( FILE *LogFileFp )
{
#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
  if ( NULL != LogFileFp )
  {
       fclose ( LogFileFp ) ;
       LogFileFp = NULL ;
  }

  IPF_tracePrintComponent(TR_CLASS_APP_IPFORWARD, "Log file closed" ) ;
#endif
}

/************************************************************************************************/
/*                                                                                              */
/* void LogString ( PortInfo *port, char *StringToLog , tU32 LogId )                                            */
/*                                                                                              */
/************************************************************************************************/

void LogString ( PortInfo *port, char *StringToLog , tU32 LogId )
{
  IPF_tracePrintComponent(TR_CLASS_APP_IPFORWARD, "%s", StringToLog);

#ifdef CONFIG_TRACE_ENABLE_IPFORWARD_LOGFILE
  if ( 0 != ( LogId & port->LogMask ) )
  {
       tU32 Time ;

#ifdef CONFIG_HOST_OS_WIN32
       Time = (tU32)clock() ;
#else
       {
	   struct timespec tp;
	   clock_gettime(CLOCK_REALTIME, &tp);
	   Time = tp.tv_sec;
	   }
#endif
 
       if ( NULL != port->LogFileFp )
	   {
            fprintf ( port->LogFileFp , "\nT: %.10d - %.8X - %s" , Time , LogId , StringToLog ) ;
       }
  }
#endif

}

#endif // CONFIG_COMM_DRIVER_EXTERNAL

