/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_can_irq.c
* Author             : APG-MID Application Team
* Date First Issued  : 12/09/2013
* Description        : This file provides CAN driver interrupt service routines
********************************************************************************
* History:
* 12/09/2013: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#include "sta_can.h"
#include "sta_canif.h"
#include "sta_canif_cfg.h"
#include "sta_canif_types.h"
#include "sta_can_irq.h"
#include "sta_can_p.h"
#include "sta_uart.h"
#include "trace.h"

// handlers for reception and retransmission
extern ControllerMO_private ControllerMO[MAX_CONFIG_CAN_CONTROLLERS];
extern Can_GlobalConfigData GlobalConfigData;

// ****************************************************************************
// RemoveAllTX_PendingMO_Abort
//
// Desc.: Remove pending MO from controller with confirmation about this action
// 		  to upper layer.  It is called only from ISR-s.
//
// Parameters (in)	:	No. oc CAN Controller.
// Parameters (out)	:	None.
// ****************************************************************************

__inline void RemoveAllTX_PendingMO_Abort(volatile CCan * CAN_ptr, uint8_t CAN_Controller)
{
	uint8_t i;

	for(i = CAN_MO_1; i <= CAN_MO_16; i++)
	{
		if(((CAN_ptr->NEWDATR1 | CAN_ptr->TRR1) & ( 1 << (i - 1))) != 0 )
		{ 	// remove this MO
			// clear transmit MO
			// IF1 is used for transmitting
			CAN_IF_WAIT(1);
			CAN_ptr->IF1CMR.reg = CANC_IF_CLR_IP | CANC_IF_TXR_ND;
			CAN_ptr->IF1CR.reg = (i);
			if (GlobalConfigData.Can_TxCnfAbort != NULL_PTR )
				GlobalConfigData.Can_TxCnfAbort(ControllerMO[CAN_Controller].SwPduHandle[i]);
		}
	}

	for(i = CAN_MO_17; i <= CAN_MO_32; i ++)
	{
		if(((CAN_ptr->NEWDATR2 | CAN_ptr->TRR2) & ( 1 << (i - 17))) != 0)// remove this MO
		{
    			// IF1 is used for transmitting,clear transmit MO
			CAN_IF_WAIT(1);
			CAN_ptr->IF1CMR.reg = CANC_IF_CLR_IP | CANC_IF_TXR_ND;
			CAN_ptr->IF1CR.reg = (i);
			if (GlobalConfigData.Can_TxCnfAbort != NULL_PTR )
				GlobalConfigData.Can_TxCnfAbort(ControllerMO[CAN_Controller].SwPduHandle[i]);
		}
	}
}


// ****************************************************************************
// ReadFIFO_Indicat
//
// Desc.: Reading of the FIFO till end, beginning on item 'Object' from controller with
// confirmation about this action  to upper layer.  It is called only from ISR-s.
//
// Parameters (in)	:	No. of CAN Controller.
//						Begining Address of Message Object
// Parameters (out)	:	None.
// ****************************************************************************
__inline void ReadFIFO_Indicat(volatile CCan * CAN_ptr, uint8_t CAN_Controller, uint8_t Object)
{
	uint8_t CanDlc;	         // length-1 of data
	uint32_t Identifier;	// ID
	uint8_t CanSduPtr[8];	// Can data

	if (Object >= CAN_MO_1 && Object <= CAN_MO_32)// only non 0 MO-s are valid
	{
		// Receive Message - read message contents -> use IF2
		CAN_ptr->IF2CMR.reg = CANC_IF_MASK | CANC_IF_ARB | CANC_IF_CNTRL | CANC_IF_CLR_IP | CANC_IF_DATA_A | CANC_IF_DATA_B | CANC_IF_TXR_ND;
		/* transfer data from message buffer to the message object 'Object' */
		CAN_ptr->IF2CR.reg = Object;
		CAN_IF_WAIT(2);

		/*Copy CAN Message Data*/
		CanDlc = (CAN_ptr->IF2MCR.reg - 1) & 0x0007;
		switch(CanDlc)
		{
		default:
			CanSduPtr[7] = CAN_ptr->IF2DATAB2.bit.data7;
		case 6 :
			CanSduPtr[6] = CAN_ptr->IF2DATAB2.bit.data6;
		case 5 :
			CanSduPtr[5] = CAN_ptr->IF2DATAB1.bit.data5;
		case 4 :
			CanSduPtr[4] = CAN_ptr->IF2DATAB1.bit.data4;
		case 3 :
			CanSduPtr[3] = CAN_ptr->IF2DATAA2.bit.data3;
		case 2 :
			CanSduPtr[2] = CAN_ptr->IF2DATAA2.bit.data2;
		case 1 :
			CanSduPtr[1] = CAN_ptr->IF2DATAA1.bit.data1;
		case 0 :
			CanSduPtr[0] = CAN_ptr->IF2DATAA1.bit.data0;
			break;
		}

		/*Get CAN Message ID*/
		Identifier = (CAN_ptr->IF2AR2.reg & 0x1fff);
		if ((CAN_ptr->IF2AR2.reg & CANC_MO_IF_ID_LONG) != 0)// extended frame
		{
			Identifier <<= 16;
			Identifier |= CAN_ptr->IF2AR1;
		}
		else// standard frame
		{
			Identifier = Identifier >> 2;
		}

		TRACE_INFO("RX CAN ID IS: %d %d\n", Identifier, Object);

		/*Copy to and Notify User Space*/
		if (GlobalConfigData.Can_RxIndication != NULL_PTR )
			GlobalConfigData.Can_RxIndication(ControllerMO[CAN_Controller].HW_Object_ptr[Object]->Handle_number, Identifier, CanDlc + 1, CanSduPtr);
	}
}


// ****************************************************************************
// CAN0_ISR
//
// Desc.: Interrupt Service Routine.
// This function is called in interrupt for CAN 0
//
// Parameters (in)	:	None.
// Parameters (out)	:	None.
// ****************************************************************************
void CAN0_ISR (void)
{
    uint8_t status;   // status register
    uint16_t inter_reg; // interrupt register
    uint32_t rec, tec;

    CAN0.CR.bit.ie = false;

    rec = CAN0.ECR.bit.rec;
    tec = CAN0.ECR.bit.tec;

    if (rec > 0xF || tec > 0xF) {
        TRACE_ERR("%s rec:%x tec:%x\n", __func__, rec, tec);
        return;
    }

    while((inter_reg = CAN0.IDR) != 0)  // some MO is pending for service OR status register was changed
    {
        if(inter_reg & 0x8000)   // Status interrupt, Highest priority
        {
            status = CAN0.SR.reg;	    // read status register
			inter_reg &= 0x3F;

            //***Test if CAN is in BUS OFF***//
            if (status & CANC_BUS_OFF)
            {
                RemoveAllTX_PendingMO_Abort(&CAN0, 0);
                CCanLLD_StartController((volatile CCan *)&CAN0);
                if (GlobalConfigData.Can_ControllerBusOff != NULL_PTR )
                    GlobalConfigData.Can_ControllerBusOff(0);
            }

            //***Test if CAN is in  Error warning or ERROR PASSIVE***//
            if (status & (CANC_ERROR_WARNING | CANC_ERROR_PASSIVE))
                RemoveAllTX_PendingMO_Abort(&CAN0, 0);//!!! CAN_EWARN send notification to CAN Interface

            //***Received packet has been successfully received***//
            if (status & CANC_RECEIVE_OK)
                ReadFIFO_Indicat(&CAN0, 0, (uint8_t)inter_reg);

            //***message was send successfully and acknowledged***//
            if (status & CANC_TRANSMIT_OK)
            {
                inter_reg = CAN0.IDR & 0x3F;
                if (inter_reg >= CAN_MO_1 && inter_reg <= CAN_MO_32)
                {   // clear MO
                    switch (ControllerMO[0].HW_Object_ptr[inter_reg]->Type)
					{
					case TX_Basic:
					case TX_Full:

						// IF1 is used for transmitting,clear transmit MO
						CANC_IF_WAIT(0,1);
						CAN0.IF1CMR.reg = CANC_IF_CLR_IP | CANC_IF_TXR_ND;
						CAN0.IF1CR.reg = inter_reg;
						if (GlobalConfigData.Can_TxCnfOk != NULL_PTR )
							GlobalConfigData.Can_TxCnfOk(ControllerMO[0].SwPduHandle[(inter_reg & 0x003F)]);

					default: break;
					}
                }
            }

            //***LEC ERR***//
            if((status & CANC_LEC_ALL_ERR) != 0)
            {
                inter_reg = CAN0.IDR & 0x3F;  // curious, but in this state inter_reg contains 0,
                RemoveAllTX_PendingMO_Abort(&CAN0, 0);//then we can't decide	which MO wasn't acknowledged
            }
        } // end of checking STATUS register 'inter_reg & 0x8000'

        if(inter_reg >= CAN_MO_1 && inter_reg <= CAN_MO_32) // only non 0 MO-s are valid
        {
            switch(ControllerMO[0].HW_Object_ptr[inter_reg]->Type)
            {
                case TX_Basic:
                case TX_Full:
                    // IF1 is used for transmitting,clear transmit MO
                    CANC_IF_WAIT(0,1);
                    CAN0.IF1CMR.reg = CANC_IF_CLR_IP | CANC_IF_TXR_ND;
                    CAN0.IF1CR.reg = inter_reg;
                    break;

                case RX_Full:
                case RX_Basic:
                    // IF2 is used for receiving,get receive MO
                    ReadFIFO_Indicat(&CAN0, 0, (uint8_t)inter_reg);
                    break;

                default: break;
            }
        }  // end of switch TX/RX message
    }

    CAN0.CR.bit.ie = true;
}

