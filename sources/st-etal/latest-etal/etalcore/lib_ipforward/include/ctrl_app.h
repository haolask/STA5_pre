/****************************************************************************/
/*                                                                          */
/* Project:              ADR3 Control application                           */
/* Filename:             ctrl_app.h                                         */
/* Programmer:           Alessandro Vaghi                                   */
/* Date:                 Jun 18th 2010                                      */
/* CPU:                  PC                                                 */
/* Type:                 C header file                                      */
/* Scope:                Types definition                                   */
/* Functions:                                                               */
/*                                                                          */
/****************************************************************************/

#define INVALID_TCP_IP_PORT          0xFFFFFFFF
#define INVALID_LUN                  (tU8)0xFF

/********************************************************************/
/* GUI COMMAND DEFINES                                              */
/********************************************************************/

#define PL_TARGET_MESSAGE_SYNC_BYTE                 ((tU8)0x1D)

/* Protocol Layer command IDs */
#define PL_CMDTYPE_COMMAND_CMOST                    0
#define PL_CMDTYPE_READ_CMOST                       1
#define PL_CMDTYPE_WRITE_CMOST                      2
#define PL_CMDTYPE_READDMA_CMOST                    3
#define PL_CMDTYPE_WRITEDMA_CMOST                   4
#define PL_CMDTYPE_GENERIC_CMOST                    5
#define PL_CMDTYPE_BOOT_CMOST                       6
#define PL_CMDTYPE_RESET_CMOST                      7

/* Protocol Layer response len */
#define PL_RESPLEN_BOOT_CMOST                       1
#define PL_RESPLEN_RESET_CMOST                      1

/* Protocol Layer response status code */
#define PL_RESPCODE_OK_CMOST                        ((tU8)0)
#define PL_RESPCODE_RESETOK_CMOST                   ((tU8)7)

/* LOG defines */

#define LOG_MASK_MDR_UTILITY                     0x01
#define LOG_MASK_MDR_INTERLAYER                  0x02
#define LOG_MASK_MDR_USB_FPGA_STATUS             0x04
#define LOG_MASK_MDR_USB_FPGA_LOW_LEVEL          0x08
#define LOG_MASK_MDR_TCP_IP                      0x10
#define LOG_MASK_MDR_UART                        0x20

void LogString ( PortInfo *port, tChar *StringToLog , tU32 LogId ) ; 
void OpenLogSession ( tS32 index ) ;
void CloseLogSession ( FILE *fp ) ;
void CtrlAppMessageHandle ( PortInfo *TcpIpPortPnt ) ;
void PutBELongOnBuffer ( tU8 *Buffer , tU32 LongToPut ) ;
void PutBEWordOnBuffer ( tU8 *Buffer , tU16 WordToPut ) ;
void ConfigParameters ( tChar *IPaddress, tU32 tcp_port, tU32 log_mask, tChar *log_filename, tS32 index );
void InterLayerProtocolInit ( tS32 index ) ;

