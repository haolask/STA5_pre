/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_rpmsg.c
* Author             : APG-MID Application Team
* Date First Issued  : 03/13/2014
* Description        : This file provides STA RPMSG specific functions.
********************************************************************************
* History:
* 03/13/2014: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Standard includes. */
#include <string.h>
#include <errno.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* Platform includes */
#include "sta_rpmsg.h"
#include "trace.h"
#include "MessageQCopy.h"

/*************************/
/** Defines declaration **/
/*************************/
#define INIT_TIMEOUT    (30000 / portTICK_RATE_MS)

/***************************/
/** Variables declaration **/
/***************************/
static RPMSG_DeviceTy   vRpmsgDev;
static xSemaphoreHandle xSemRpmsg;

xQueueHandle            xQueueRpmsg;
int                     vRpmsgInitDone = INIT_NOT_STARTED;

/*************************/
/** Function definition **/
/*************************/

/*******************************************************************************
* Function Name  : RPMSG_disable_rx
* Description    : Disable remote processor events
* Return         : NONE
*******************************************************************************/
void RPMSG_disable_rx(void)
{
    mbox_stop_rx_msg(vRpmsgDev.MboxChanID);
}

/*******************************************************************************
* Function Name  : RPMSG_enable_rx
* Description    : Enable remote processor events
* Return         : NONE
*******************************************************************************/
void RPMSG_enable_rx(void)
{
    mbox_resume_rx_msg(vRpmsgDev.MboxChanID);
}

/*******************************************************************************
* Function Name  : RPMSG_U32Send
* Description    : Send message to the remote processor through allocated
*                  Mailbox channel.
* Input          : 32 bits data payload to be sent
* Return         : NONE
*******************************************************************************/
void RPMSG_U32Send(uint32_t arg)
{
    struct mbox_msg  MboxMsg;

    MboxMsg.dsize = sizeof(arg);
    MboxMsg.pdata = (uint8_t *)&arg;
    while(mbox_send_msg(vRpmsgDev.MboxChanID, &MboxMsg) == MBOX_BUSY);
}

/*******************************************************************************
* Function Name  : RPMSG_PreInit
* Description    : Initialiazes the semaphore required for RPMSG_Init
*				   synchronization. This pre-init shall be called only once,
*				   before all RPMSG task start running.
* Input          : NONE
* Return         : 0 if no error, not 0 otherwise
*******************************************************************************/
int RPMSG_PreInit()
{
    /* Create a semaphore once, and give it, to let all FreeRTOS that require
	 * RPMSG framework init to synchronize between each others */

	 xSemRpmsg = xSemaphoreCreateBinary();
	 if ( xSemRpmsg == NULL) {
		 return -EFAULT;
	 }

	 xSemaphoreGive(xSemRpmsg);

	 return 0;
}

/*******************************************************************************
* Function Name  : RPMSG_Init
* Description    : RPMSG framework initialisation procedure.
*                  M3 FreeRTOS RPMSG framework shares resources with Linux
*                  like Vring buffers. These shared resources are allocated in
*                  shared memory by the Linux RPMSG framework and gathered in
*                  a resource table.
*                  So the Init procedure on the M3 remote processor must
*                  be synchronised with the Linux RPMSG init in order to
*                  fetch this resource table for init completion.
*                  Init procedure is the following:
*                  - Request a Mailbox channel dedicated to RPMSG
*                  - Synchronisation with HOST Linux:
*                    - wait for RP_MSG_INIT_REQUEST to get resource table address
*                      and Call MessageQCopy_init() to complete init procedure
*                      based on the input parameters fetched in the resource table.
*                    - wait for RP_MSG_HOST_READY to before completing init
*                  RPMSG Init procedure has to only be done once so if the init
*                  procedure is already started by another task, a waiting point
*                  allow to wait for its completion before going further.
* Input          : NONE
* Return         : 0 if no error, not 0 otherwise
*******************************************************************************/
int RPMSG_Init()
{
    struct mbox_chan_req  MboxReq;
    uint32_t RxMsg[2];
    int ChanID, HostSynchro = false;

    /** RPMSG Init procedure has to only be done once **/
    if (vRpmsgInitDone == INIT_DONE)
        return 0;

    /** RPMSG Init procedure is already started by another task **/
    /** So wait for its completion **/
    if (vRpmsgInitDone == INIT_ONGOING)
    {
        TRACE_INFO("RPMSG_Init: Wait for RPMSG Init completion\n");

        /* Wait for RPMSG Init is completed by another task */
        if( xSemaphoreTake( xSemRpmsg, INIT_TIMEOUT ) == pdTRUE )
        {
            TRACE_INFO("RPMSG_Init: RPMSG Init done\n");

            /* Release the semaphore in case of another task expects it */
            xSemaphoreGive(xSemRpmsg);
            return 0;
        }
        else
        {
            /* RPMSG Init timeout occurs so ERROR suspected */
            TRACE_ERR("RPMSG_Init: RPMSG Init timeout\n");
            return -EFAULT;
        }
    }

    xSemaphoreTake(xSemRpmsg, portMAX_DELAY);

    /** 1st task requiring RPMSG so start RPMSG Init procedure **/
    vRpmsgInitDone = INIT_ONGOING;
    /**
     * Request Mailbox channel allocated to RPMSG *
     **/
    /* Set requested Mailbox channel feature */
    MboxReq.chan_name = "rpmsg";
    MboxReq.user_data = NULL;
    MboxReq.rx_cb = VirtQueue_isr;

    ChanID = mbox_request_channel(&MboxReq);
    if (ChanID == MBOX_ERROR)
    {
        TRACE_ERR("RPMSG_Init: Requested channel %s not found\n", MboxReq.chan_name);
        return -EFAULT;
    }
    else
        TRACE_INFO("RPMSG_Init: Requested channel %s [id:%d] allocated\n",
                    MboxReq.chan_name, ChanID);

    vRpmsgDev.MboxChanID = ChanID;

    /**
     * Init procedure synchonisation with Linux RPMSG init
     **/
    /* Create a queue capable of containing 10 rxMsg, which are
     * arrays of 2 (uint32_t) */
    xQueueRpmsg = xQueueCreate( 10, sizeof( RxMsg ) );



    TRACE_INFO("RPMSG_Init: Wait for Linux HOST RPMSG synchronisation\n");

    /* Synchronisation procedure with HOST */
    while (HostSynchro == false)
    {
        /* Wait for external HOST event */
        if (xQueueReceive(xQueueRpmsg, RxMsg, portMAX_DELAY))
        {
            /* Get Rx message from VirtQueue_isr
             * RxMsg[0] contains the acutal message code
             * RxMsg[1] may contain an optional parameter
             */
            switch (RxMsg[0])
            {
                /** Step 1: RP init resquest from HOST **/
                case RP_MSG_INIT_REQUEST:
                    /* Fetch resource table address */
                    vRpmsgDev.RscTable = (struct resource_table *)(RxMsg[1]);
                    TRACE_INFO("RPMSG_Init: Get Resource Table address [0x%x]\n", RxMsg[1]);

                    /** Init VirtQueues based on resource table parameters **/
                    if (MessageQCopy_init(vRpmsgDev.RscTable) != MessageQCopy_S_SUCCESS) {
                        TRACE_ERR("RPMSG_Init: MessageQCopy_init failed !\n");
                        HostSynchro = true;
                    }
                    break;

                /** Step 2: HOST ready to receive message through Virtio VRING **/
                case RP_MSG_HOST_READY:
                    /* Complete init procedure */
                    TRACE_INFO("RPMSG_Init: RP_MSG_HOST_READY, init done\n");
                    vRpmsgInitDone = INIT_DONE;
                    HostSynchro = true;

                    break;
                default:
                    TRACE_ERR("RPMSG_Init: Unexpected Rx message[0x%x]\n", RxMsg[0]);
                    HostSynchro = true;
                    break;
            }
        }
        else
        {
            /* TIMEOUT error: Init procedure failed */
            TRACE_ERR("RPMSG_Init: Timeout error\n");
            HostSynchro = true;
        }
    }

    /**
     * Init procedure end: Acknowledge the RPMSG Init procedure
     **/
    if (vRpmsgInitDone == INIT_DONE) {
        /* Wake up task waiting for Rpmsg init completion */
        xSemaphoreGive(xSemRpmsg);

        RPMSG_U32Send(RP_MSG_INIT_ACK);
        TRACE_INFO("RPMSG_Init: Successfull RPMSG init procedure\n");
        return 0;
    }
    else {
        RPMSG_U32Send(RP_MSG_INIT_FAILED);
        TRACE_ERR("RPMSG_Init: RPMSG init procedure failure\n");
        return -EFAULT;
    }
}

// End of file
