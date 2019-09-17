//!
//!  \file    rif_protocol_router.h
//!  \brief   <i><b> Radio interface protocol router functions entry point </b></i>
//!  \details Interface file for upper software layer. State machine is implemented here.
//!  \author  David Pastor
//!

#ifndef RIF_PROTOCOL_ROUTER_H
#define RIF_PROTOCOL_ROUTER_H

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///

#define MSG_ID_RIF_PR_TX_RESP_DABMW     RIF_MAKE_MSG_ID(TASK_ID_RIF_PR, TASK_ID_DABMW, MSG_IDX_RIF_PR_TX_RESP)
#define MSG_ID_RIF_PR_TX_RESP_RIMW      RIF_MAKE_MSG_ID(TASK_ID_RIF_PR, TASK_ID_RIF_RIMW, MSG_IDX_RIF_PR_TX_RESP)

///
// Enums
///

enum 
{
	MSG_IDX_RIF_PR_TX_RESP
};

///
// Types
///
typedef struct
{
	tU32 msg_id;
	tU32 num_bytes;
	tU8 *msg_buf_ptr;
	tU8 lun;
} rif_pr_msg_header_t;

///
// Variables
///
extern rif_msg_queue_t *rif_pr_msg_queue_p;

///
// Exported functions
///
extern tVoid rif_protocol_router_handle(tVoid *pParams);
extern tVoid ForwardRxPacketToProtocolRouter(tU8 lun, tU8 *inDataBuf, tU32 bytesNumInDataBuf);
extern tSInt rif_sendMsgTo_pr(tU32 msg_id, tU8 lun, tU8 *cmd, tU32 clen);

#ifdef __cplusplus
}
#endif

#endif // RIF_PROTOCOL_ROUTER_H

// End of file
