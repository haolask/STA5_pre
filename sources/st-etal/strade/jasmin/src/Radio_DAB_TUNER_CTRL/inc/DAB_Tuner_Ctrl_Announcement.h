/*==================================================================================================
    start of file
==================================================================================================*/
/**************************************************************************************************/
/** \file DAB_Tuner_Ctrl_Announcement.h 														   *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.											   *
*  All rights reserved. Reproduction in whole or part is prohibited								   *
*  without the written permission of the copyright owner.										   *
*																								   *
*  Project              : ST_Radio_Middleware													   *
*  Organization			: Jasmin Infotech Pvt. Ltd.												   *
*  Module				: Radio DAB Tuner Control												   *
*  Description			: This file contains function declarations of DAB Announcement related     *
                          APIs.					                                                   *
*																								   *
*																								   *
***************************************************************************************************/
#ifndef DAB_TUNER_CTRL_ANNOUNCEMENT_
#define DAB_TUNER_CTRL_ANNOUNCEMENT_

/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_main_hsm.h"

/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/

#define DAB_TUNER_CTRL_ALARM_ANNO_SET    					0x0001 /* Macro for checking alarm announcement set condition  */
#define DAB_TUNER_CTRL_ANNO_START_CIF_COUNT_DIFF_LIMIT    	0x0005 /* Macro for checking CIF count difference to identify start or stop announcement */
#define DAB_TUNER_CTRL_ANNO_CLUSTERID_ALL_ZERO			   	0x0000 /* Cluster ID  is  '0000 0000' case */
#define DAB_TUNER_CTRL_ANNO_CLUSTERID_ALL_ONE			   	0x00FF /* Cluster ID  is  '1111 1111' case */

/*--------------------------------------------------------------------------------------------------
    Function declarations
--------------------------------------------------------------------------------------------------*/

void findAnnoServiceInfoentryindatabase(Ts_Service_Info	 *st_Service_Info,Tu8 *u8_Index);
void findMatchedSidAnnoServiceInfoentryindatabase(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu8 *u8_Index);
void findAnnoentryindatabase(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu8 *u8_Index);
void findMatchedSidAnnoentryindatabase(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu8 *u8_Index);
Tbool check_same_service(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_UpdateAnnouncementType(Ts_Anno_Swtch_Info *st_Anno_Swtch_Info,Te_DAB_Tuner_Ctrl_announcement_type *e_announcement_type);
Tu32 FindSID_Anno_SubChID(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 u8_SubChId);
void DAB_Tuner_Ctrl_Update_CurrentTunedAnnoData(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_AnnouncementSwitchingFilter(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_StoreAnnouncementSwitchingNotificationInfo(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_UpdateCurrentSIDAnnoInfo(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_ClearingAnnoDatabases(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
#endif
/*=============================================================================
    end of file
=============================================================================*/