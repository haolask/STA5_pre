//!
//!  \file    rif_rimw_protocol.h
//!  \brief   <i><b> Radio interface RIMW protocol functions entry point </b></i>
//!  \details Radio interface RIMW protocol functions.
//!  \author  David Pastor
//!

#ifndef RIF_RIMW_PROTOCOL_H
#define RIF_RIMW_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///

#define MSG_ID_RIF_RIMW_RX_CMD_PR       RIF_MAKE_MSG_ID(TASK_ID_RIF_RIMW, TASK_ID_RIF_PR, MSG_IDX_RIMW_RX_CMD)

#define RIF_RIMW_H1_CMDNUMM_MASK        0x03
#define RIF_RIMW_H2_CMDNUML_MASK        0xFF
#define RIF_RIMW_GET_CMD_NUM(cmd)	(tU16)(((tU16)(((rimw_apiHeaderTy *)cmd)->header_1.fields.cmdNumM & RIF_RIMW_H1_CMDNUMM_MASK) << 8) | (tU16)(((rimw_apiHeaderTy *)cmd)->header_2.fields.cmdNumL & RIF_RIMW_H2_CMDNUML_MASK))

#define RIF_RIMW_STATUS_MASK                       (tU8)0x3F

#define RIF_RIMW_CMD_STATUS_OK                     (tU8)((tU8)0x00 & RIF_RIMW_STATUS_MASK)

// Error list, all errors that results in a stop of an auto-command must be
// inside the following values: 0x01 - 0x1F
#define RIF_RIMW_CMD_STATUS_ERR_RESERVED_0         (tU8)((tU8)0x01 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_RESERVED_1         (tU8)((tU8)0x02 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_RESERVED_2         (tU8)((tU8)0x03 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_CHECKSUM           (tU8)((tU8)0x04 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_TIMEOUT            (tU8)((tU8)0x05 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_WRONG_DATA_BIT     (tU8)((tU8)0x06 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_HEADER0_FORMAT     (tU8)((tU8)0x07 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_QUEUE_FULL         (tU8)((tU8)0x08 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_PARAM_WRONG        (tU8)((tU8)0x09 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_ILLEGAL_CMDNUM     (tU8)((tU8)0x0A & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_RESERVED_CMDNUM    (tU8)((tU8)0x0B & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_DUPLICATED_AUTO    (tU8)((tU8)0x0C & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_PAYLOAD_TOO_LONG   (tU8)((tU8)0x0D & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_UNAVAILABLE_FNCT   (tU8)((tU8)0x0E & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_WRONG_CMD_LENGHT   (tU8)((tU8)0x0F & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_GENERIC_FAILURE    (tU8)((tU8)0x10 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_FNCT_DISABLED      (tU8)((tU8)0x11 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_APP_NOT_SUPPORTED  (tU8)((tU8)0x12 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_WRONG_STOP_BIT     (tU8)((tU8)0x13 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_CMD_IS_NOT_ONGOING (tU8)((tU8)0x14 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_RADIO_IS_NOT_ON    (tU8)((tU8)0x15 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_NO_MEMORY_AVAIL    (tU8)((tU8)0x16 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_NVM_FAILED         (tU8)((tU8)0x17 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_NVM_FILE_OPEN      (tU8)((tU8)0x18 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_NVM_FILE_READ      (tU8)((tU8)0x19 & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_NVM_FILE_WRITE     (tU8)((tU8)0x1A & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_NVM_FILE_CLOSE     (tU8)((tU8)0x1B & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_RESERVED_11        (tU8)((tU8)0x1C & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_RESERVED_12        (tU8)((tU8)0x1D & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_RESERVED_13        (tU8)((tU8)0x1E & RIF_RIMW_STATUS_MASK)
#define RIF_RIMW_CMD_STATUS_ERR_LAST_ERROR         (tU8)((tU8)0x1F & RIF_RIMW_STATUS_MASK)

#define RIF_RIMW_CMD_STATUS_RSP_WITHOUT_PAYLOAD    (tU8)((tU8)0x20 & RIF_RIMW_STATUS_MASK)

#define RIF_RIMW_CMD_STATUS_RSP_NO_DATA_AVAILABLE  (tU8)((tU8)0x21 & RIF_RIMW_STATUS_MASK)

#define RIF_RIMW_CMD_STATUS_NO_RSP_TO_SEND         (tU8)((tU8)0x22 & RIF_RIMW_STATUS_MASK) // It will never be see by the host (command ends)

#define RIF_RIMW_CMD_STATUS_OPERATION_ONGOING      (tU8)((tU8)0x23 & RIF_RIMW_STATUS_MASK) // Operation ongoing

#define RIF_RIMW_CMD_STATUS_OPERATION_FAILED       (tU8)((tU8)0x24 & RIF_RIMW_STATUS_MASK) // Operation failed

#define RIF_RIMW_CMD_STATUS_ONGOING_NO_RSP_TO_SEND (tU8)((tU8)0x25 & RIF_RIMW_STATUS_MASK) // Operation ongoing but avoid to send response (it will never be see by the host)

#define RIF_RIMW_CMD_STATUS_OTHER_CMD_WAITING      (tU8)((tU8)0x26 & RIF_RIMW_STATUS_MASK)

#define RIF_RIMW_CMD_STATUS_LASTOPERATION_STOPPED  (tU8)((tU8)0x27 & RIF_RIMW_STATUS_MASK) // Last operation stopped


#define RIF_RIMW_MSG_AUTO_NOTIF_MAX_SIZE    512

///
// Enums
///

enum
{
	MSG_IDX_RIMW_RX_CMD
};

///
// Types
///
typedef union
{
    struct
    {
        tU8 Stop:           1;
        tU8 Data:           1;
        tU8 Fast:           1;
        tU8 Len:            1;
        tU8 Sto:            1;
        tU8 Auto:           1;
        tU8 Reply:          1;
        tU8 Host:           1;
    } fields;

    tU8 value;
} rimw_apiHeader0Ty;

typedef union
{
	struct
	{
		tU8 cmdNumM:            2;
		tU8 specific_status:    6;
	} fields;

	tU8 value;
} rimw_apiHeader1Ty;

typedef union
{
	struct
	{
		tU8 cmdNumL;
	} fields;

	tU8 value;
} rimw_apiHeader2Ty;

typedef struct
{
	rimw_apiHeader0Ty header_0;

	rimw_apiHeader1Ty header_1;

	rimw_apiHeader2Ty header_2;
} rimw_apiHeaderTy;

typedef struct
{
	rimw_apiHeaderTy header;
	tU8 payload[3];
} rimw_apiCmdTy;

///
// Exported variables
///

extern tU8 rif_rimw_auto_notif[RIF_RIMW_MSG_AUTO_NOTIF_MAX_SIZE];

///
// Exported functions
///

extern tU8 rif_rimw_OSAL_ERROR_to_rimw_error(tSInt osal_error_code);
extern tVoid rif_rimw_protocol_handle(tVoid *pParams);
extern tSInt rif_sendMsgTo_rimw(tU32 msg_id, tU8 *cmd, tU32 clen);
tU8 *rif_rimw_get_cmd_payload_address(tU8 *cmd, tU32 clen);
tU32 rif_rimw_get_cmd_payload_length(tU8 *cmd, tU32 clen);

#ifdef __cplusplus
}
#endif

#endif // RIF_RIMW_PROTOCOL_H

// End of file
