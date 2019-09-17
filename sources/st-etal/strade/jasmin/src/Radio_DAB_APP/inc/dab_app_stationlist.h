/*=============================================================================
    start of file
=============================================================================*/


/************************************************************************************************************/
/** \file dab_app_stationlist.h																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains station list related API's for DAB Application.					*
*																											*
*																											*
*************************************************************************************************************/


#ifndef __DAB_APP_STATIONLIST_H__
#define __DAB_APP_STATIONLIST_H__

/** \file */
/** \page DAB_APP_STATIONLIST_top DAB Application Station List related functions.

\subpage DAB_APP_STATIONLIST_Overview
\n
\subpage DAB_APP_STATIONLIST_Functions
\n
*/

/**\page DAB_APP_STATIONLIST_Overview Overview   
    \n
     DAB Station List related functions, contains functions for creating, sorting station list.  
    \n\n
*/

/** \page DAB_APP_STATIONLIST_Functions Functions 
    <ul>
        <li> #DAB_APP_AddServCompToSTL			: This function adds the valid stations of ensemble to the station list. </li>
        <li> #DAB_APP_CreateStationList			: This function creates the station list .</li>
		<li> #DAB_APP_SortSTL					: This function sotts the station list in alphabetical order. </li>
    </ul>
*/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "cfg_types.h"
/*-----------------------------------------------------------------------------
   Macros
-----------------------------------------------------------------------------*/
#define DAB_APP_ENSEMBLE_FOUND_TWICE 	2u
#define DAB_APP_SERVICE_FOUND_TWICE 	2u
#define DAB_APP_SAME_COMPONENT_FOUND_ONCE 1u
/*-----------------------------------------------------------------------------
Variables
-----------------------------------------------------------------------------*/


/**
 * @brief Structure comprises information regarding label 
 */
typedef enum
{
	DAB_APP_LABEL_INVALID = -1,
	DAB_APP_LABEL_NOT_PRESENT,
	DAB_APP_LABEL_PRESENT
	
}Te_DAB_App_LabelStatus;


/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief 				This function adds the valid stations of ensemble to the station list.
*   \param[in]				curr_freq frequency value of the station
*   \param[in]				curr_EId ensemble value of the station
*   \param[in]				i index value of ensemble information structure
*   \param[in]				curr_ECC Extended country code of the station
*   \param[out]				None
*   \pre					the ast_EnsembleInfo, ast_ServiceInfo, ast_ComponentInfo structures are updated.
*   \details 				This function adds primary and secondary components (stations) to the satation list belongs to the ensemble.
*   \post					Station list got updated with primary and secondary components of the ensemble.\n
*   \errhdl			   		NA
* 
******************************************************************************************************/

void DAB_APP_AddServCompToSTL(Tu32 u32_Cur_Frequency, Tu16 u16_Cur_EId, Tu8 u8_Cur_ECC,  Tu16 u16_EnsembleIndex,Tu32 u32_previousfreq);

/*****************************************************************************************************/
/**	 \brief 				This function creates the station list.
*   \param[in]				none
*   \param[out]				none
*   \pre					the ast_EnsembleInfo, ast_ServiceInfo, ast_ComponentInfo structures are updated.
*   \details 				This is the function selects the ensemble for which service components is to be added in the station list.
*   \post					The DAB station list is updated.\n
*   \errhdl			   		NA
* 
******************************************************************************************************/

Te_RADIO_ReplyStatus DAB_APP_CreateStationList(Te_DAB_App_RequestCmd e_DAB_App_RequestCmd,Tu32 u32_Frequency);

/*****************************************************************************************************/
/**	 \brief 				This function sotts the station list in alphabetical order.
*   \param[in]				none
*   \param[out]				none
*   \pre					The DAB station list is updated.
*   \details 				This function sorts the DAB station list in alphabetic order of the station label.
*   \post					The DAB station list is sorted.\n
*   \errhdl			   		NA
* 
******************************************************************************************************/

void DAB_APP_SortSTL(void);

void DAB_APP_EnsembleSortSTL(void);
void DAB_APP_AddEnsemblesToSTL(Tu16 u16_Cur_EId, Tu16 u16_EnsembleIndex, Tu32 u32_Cur_Frequency);
Ts32 DAB_App_String_comparison(Tu8 *src,Tu8 *dst,Tu8 size);

void DAB_App_Update_TunerCtrlLayer_EnsembleInfoBuffer(void) ;

void DAB_App_Update_TunerCtrlLayer_ServiceInfoBuffer(void) ;

void DAB_App_Update_TunerCtrlLayer_ComponentInfoBuffer(void) ;
void DAB_App_Update_TunerCtrlLayer_MultiplexInfo(void);
#endif /* end of __DAB_APP_STATIONLIST_H__ */

/*=============================================================================
    end of file
=============================================================================*/