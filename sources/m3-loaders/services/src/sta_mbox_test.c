/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : STA_mbox_test.c
* Author             : APG-MID Application Team
* Date First Issued  : 01/10/2014
* Description        : This file provides mailbox test functions.
********************************************************************************
* History:
* 01/10/2014: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Platform includes */
#include "trace.h"
#include "sta_mbox.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************/
/** Defines declaration **/
/*************************/
#define MBOX_BUFFER_SIZE    20
#define MBOX_MAX_NB_MSG     2

#define MBOX_MSG_TYPE       1
#define MBOX_CLOSE_TYPE     2
#define MBOX_OPEN_TYPE      3
#define OTHER_MSG_TYPE      4

#define MBOX_CLOSE_ORDER    ((uint32_t)0xFFFFFFFF)
#define MBOX_OPEN_ORDER     ((uint32_t)0xFFFFFF00)


/***************************/
/** Variables declaration **/
/***************************/
struct test_msg
{
    int     ID;
	uint16_t		MsgID;
	uint16_t     MsgSize;
	uint8_t		MsgData[MBOX_BUFFER_SIZE];
};

static struct lib_rbuf   vRxBuf[MBOX_MAX_NUMBER];
static struct mbox_msg   vMboxMsg;
static xQueueHandle xQueue;


/*************************/
/** Function definition **/
/*************************/

/*******************************************************************************
* Function Name  : Mbox_RxCallback
* Description    : Call back function called upon Mailbox received message
*                  This function is used in an interrupt service routine
* Input          : user_data: user specific data used as parameter in rxcb
*                  Msg : Message received from the Mailbox channel
* Return         : None
*******************************************************************************/
void Mbox_RxCallback(void *user_data, struct mbox_msg *Msg)
{
    uint32_t         i, NbFreeBuf;
    portBASE_TYPE xHigherPriorityTaskWoken;
    struct test_msg  *RxMsg = lib_rbuf_get(&vRxBuf[(int)user_data], &NbFreeBuf);
    uint32_t         *RxOrder = (uint32_t *)RxMsg->MsgData;

    /* If Rx Ring buffer is full so stop upcoming Rx Msg */
    if (!NbFreeBuf)
        mbox_stop_rx_msg((int)user_data);

    if (!RxMsg) {
        TRACE_ERR("Mbox_RxCallback: Chan[ID%d] RxFiFo full\n", RxMsg->ID);
        return;
    }

    /* Get Rx message from Mailbox IPC */
    RxMsg->ID = (int)user_data;
    RxMsg->MsgSize = Msg->dsize;

    for (i = 0; i < Msg->dsize && i < MBOX_BUFFER_SIZE; i++)
    {
        RxMsg->MsgData[i] = *(Msg->pdata + i);
    }

    if (*RxOrder == MBOX_CLOSE_ORDER)
        RxMsg->MsgID = MBOX_CLOSE_TYPE;
    else if ((*RxOrder & MBOX_OPEN_ORDER) == MBOX_OPEN_ORDER)
    {
        i = *RxOrder & ~MBOX_OPEN_ORDER;
        RxMsg->MsgData[0] = (uint8_t)i;
        RxMsg->MsgID = MBOX_OPEN_TYPE;
    }
    else
        RxMsg->MsgID = MBOX_MSG_TYPE;

    //TRACE_INFO("RX [ID%d]: %s\n", RxMsg->ID, RxMsg->MsgData);

    /* Wake up task waiting for event in xQueue */
    xQueueSendToBackFromISR( xQueue, (void *)&RxMsg, &xHigherPriorityTaskWoken );

	/* Now the buffer is empty we can switch context if necessary. */
	if( xHigherPriorityTaskWoken )
	{
		taskYIELD ();
	}
    return;
}

/*******************************************************************************
* Function Name  : Mbox_ChanInit
* Description    : Initialize mailbox channel to be used
* Input          : chan_name: Mailbox channel name requested
* Return         : Mailbox channel index or MBOX_ERROR(-1)
*******************************************************************************/
int Mbox_ChanInit(char *chan_name, void *user_data)
{
    struct mbox_chan_req  MboxReq;
    int             ChanID;

    /* Set requested Mailbox channel feature */
    MboxReq.chan_name = chan_name;
    MboxReq.user_data = user_data;
    MboxReq.rx_cb = Mbox_RxCallback;

    ChanID = mbox_request_channel(&MboxReq);

    if (ChanID == MBOX_ERROR)
        TRACE_INFO("Mbox_ChanInit: Requested channel %s not found\n", MboxReq.chan_name);
    else
        TRACE_INFO("Mbox_ChanInit: Requested channel %s [id:%d] allocated\n",
                    MboxReq.chan_name, ChanID);

    return ChanID;
}

/*******************************************************************************
* Function Name  : MboxTestTask
* Description    : Mailbox Test task routine
* Input          : None
* Return         : None
*******************************************************************************/
void MboxTestTask()
{
    struct test_msg  *RxMsg;
    int         MboxID[MBOX_MAX_NUMBER];
    char        mbox_name[20];
    int         i, ret, status = 0;

    TRACE_INFO("FreeRTOS Mailbox test task\n");

    /* Create a queue capable of containing up to 3 rxMsg pointers per mailboxes */
	xQueue = xQueueCreate( MBOX_MAX_NB_MSG * MBOX_MAX_NUMBER, sizeof( RxMsg ) );

    /** Mailbox channel initialisation **/
    for (i = 0; i < MBOX_MAX_NUMBER; i++)
    {
        sprintf(mbox_name, "mbox-test-%02d", i);
        ret = Mbox_ChanInit(mbox_name, (void *)i);

		if (ret == MBOX_ERROR)
			continue;

		MboxID[i] = ret;

        /* Allocate ring buffer for Rx mailbox messages */
        ret = lib_rbuf_alloc(&vRxBuf[i], sizeof(struct test_msg), MBOX_MAX_NB_MSG);
        if (ret != 0)
        {
            int j;

			TRACE_ERR("\nMboxTestTask: failed to allocate memory\n");

            for (j = 0; j < i; j++)
                lib_rbuf_free(&vRxBuf[j]);

            status = -1;
            break;
        }
    }

    while(1)
    {
        /* Task init failed so do nothing */
        if (xQueue == 0 || status != 0)
        {
            TRACE_ERR("\n\rMboxTestTask: xQueue[%d] and Mbox[%s] init failed\n",
                        xQueue, mbox_name);
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        /* Task is ready: Wait for external event */
        if (xQueueReceive(xQueue, &RxMsg, portMAX_DELAY))
        {
            switch (RxMsg->MsgID)
            {
                case MBOX_MSG_TYPE:
                    /* For test purpose loop back received message to A7 */
                    vMboxMsg.dsize = RxMsg->MsgSize;
                    vMboxMsg.pdata = RxMsg->MsgData;
                    mbox_os_wait_send_msg(MboxID[RxMsg->ID], &vMboxMsg);

                    //TRACE_INFO("TX [ID%d]: %s\n", RxMsg->ID, vMboxMsg.pdata);

                    /* Free Rx Buffer */
                    lib_rbuf_put(&vRxBuf[RxMsg->ID], RxMsg);

                    /* One Rx buffer has been released so Rx msg on MBOX */
                    /* can be re-enabled in case it has been disabled */
                    /* due to Ring Buffer full */
                    mbox_resume_rx_msg(MboxID[RxMsg->ID]);
                break;

                case MBOX_CLOSE_TYPE:
                    /* Close mailbox channel */
                    mbox_free_channel(MboxID[RxMsg->ID]);
                    lib_rbuf_free(&vRxBuf[RxMsg->ID]);
                break;

                case MBOX_OPEN_TYPE:
                    /* Open specified mailbox channel */
                    i = RxMsg->MsgData[0];
                    sprintf(mbox_name, "mbox-test-%02d", i);
                    ret = Mbox_ChanInit(mbox_name, (void *)i);
                    if (ret != MBOX_ERROR &&
                        lib_rbuf_alloc(&vRxBuf[i], sizeof(struct test_msg), MBOX_MAX_NB_MSG) == 0)
                        MboxID[i] = ret;
                break;

                case OTHER_MSG_TYPE:
                default:
                    TRACE_ERR("MboxTestTask: Wrong Rx MsgID [%d]\n", RxMsg->MsgID);
                break;
            }
        }
    }
    return;
}

#ifdef __cplusplus
}
#endif

// End of file
