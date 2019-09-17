//!
//!  \file    rif_msg_queue.h
//!  \brief   <i><b> Radio interface message queue function entry point </b></i>
//!  \details Radio interface message queue interface.
//!  \author  David Pastor
//!

#ifndef RIF_MSG_QUEUE_H
#define RIF_MSG_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///

#define RIF_MSG_ID_TO_MASK                           (tU32)0xFF000000
#define RIF_MSG_ID_FROM_MASK                         (tU32)0x00FF0000
#define RIF_MSG_ID_IDX_MASK                          (tU32)0x0000FFFF

#define RIF_MAKE_MSG_ID(to, from, msg_idx)           (tU32)(((to << 24) & RIF_MSG_ID_TO_MASK) | ((from << 16) & RIF_MSG_ID_FROM_MASK) | (msg_idx & RIF_MSG_ID_IDX_MASK))
#define RIF_GET_MSG_ID_IDX(msg_id)                   (tU32)(msg_id & RIF_MSG_ID_IDX_MASK)
#define RIF_GET_MSG_ID_TASK_TO(msg_id)               (tU32)((msg_id & RIF_MSG_ID_TO_MASK) >> 24)
#define RIF_GET_MSG_ID_TASK_FROM(msg_id)             (tU32)((msg_id & RIF_MSG_ID_FROM_MASK) >> 16)


///
// Types
///

/*!
 * \struct	rif_msg_queue_t
 * 			Defines the type of the variable containing the
 * 			private status of the rif msg queue.
 */
typedef struct
{
	/*! The msg queue name, only for debug purposes */
	tChar *name;
	/*! Max number of elements in the queue */
	tU16   queueSize;
	/*! Index of the last location read */
	tS16   readPtr;
	/*! Index of the last location written to */
	tS16   writePtr;
	/*! Semaphore needed by #rif_queueGetLock/#rif_queueReleaseLock */
	OSAL_tSemHandle lock;
	/*! Event needed to wait for message */
	OSAL_tEventHandle event;
} rif_msg_queue_t;

///
// Exported functions
///

extern rif_msg_queue_t *rif_msg_create_queue(tChar *name, tUInt max_messages);
extern tSInt rif_msg_delete_queue(rif_msg_queue_t **msg_queue_p);
extern tSInt rif_msg_send(rif_msg_queue_t *msg_queue_p, tVoid *msg_p);
extern tVoid *rif_msg_receive(rif_msg_queue_t *msg_queue_p);
extern tSInt rif_msg_release(rif_msg_queue_t *msg_queue_p, tVoid *msg_p);

#ifdef __cplusplus
}
#endif

#endif // RIF_MSG_QUEUE_H

// End of file
