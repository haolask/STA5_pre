/*=============================================================================
    start of file
=============================================================================*/


/************************************************************************************************************/
/** \file file dab_app.h 																				    *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains function declaration for handling DAB main and instance HSM's.	*
*																											*
*																											*
*************************************************************************************************************/

#ifndef __DAB_APP_H__
#define __DAB_APP_H__


/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include "hsm_api.h"



//#define PC_TEST


/*-----------------------------------------------------------------------------
    function declarations intern 
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief 				Brief description of the function.
*   \param[in]				pst_msg message to be handled by the component.
*   \param[out]				None
*   \pre					Component is registered and used HSM is initialized.
*   \details 				This API shuts down the software component SC_DAB_APP. This de-initializes DAB Application resources.\n
*							DAB Application triggers Shut-down request to Tuner Ctrl.
*   \post					DAB Application component is stopped.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

/*===========================================================================*/
/** 
 * @brief Brief description of the function. 
 *
 * @param[in] pst_msg message to be handled by the component
 *
 * @pre [mandatory] Component is registered and used HSM is initialized 
 *
 * @post [mandatory] If the message is not set to pending the messaged is now 
 * freed by the system layer. Only if a massage is set to pending a reference 
 * can be saved.
 *
 * @errhdl [mandatory] none
 */
/*===========================================================================*/
void DAB_APP_MSG_HandleMsg(Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/** 
 * @brief This function will handle message for the specific instance
 *
 * @param[in, out] pst_me Pointer to the hsm object
 * @param[in, out] pst_msg Pointer to the msg. This message can be modified. 
 *   A result code has to be set if message is handled in this state
 *
 * @pre [mandatory] HSM is initialized and msg is not NULL
 *
 * @description [mandatory] this function processes the messages based on the instance
 *
 * @post [mandatory] message is handled for the particular instance
 *
 * @errhdl [mandatory] none
 */
/*===========================================================================*/
void DAB_APP_INST_HSM_HandleMessage(Ts_Sys_Msg *pst_msg);

/*===========================================================================*/
/** 
 * @brief This function will handle component intialization
 *
 * @param[in, out] None
 *
 * @pre [mandatory]Framework is intialized
 *
 * @description [mandatory] this function processes the intialization of DAB application
 *
 * @post [mandatory] SC_DAB_APP is intialized
 *
 * @errhdl [mandatory] none
 */
/*===========================================================================*/
void DAB_App_Component_Init();

void DAB_App_UpdateChannelName(Tu32 u32_Frequency, Tu8 *pu8_ChannelName) ;

void DAB_App_Updatefrequency(Tu8 *au8_ChannelName,Tu32 *u32_Frequency);

#endif /* __DAB_APP_H__ */

/*=============================================================================
    end of file
=============================================================================*/