/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/* Copyright (c) 2016, Jasmin Infotech Private Limited.
*  All rights reserved. Reproduction in whole or part is prohibited
*  without the written permission of the copyright owner.
*
*  Project              : ICommon.h
*  Organization			: Jasmin Infotech Pvt. Ltd.
*  Module				: HMI IF
*  Description			: This file contains the structure definition and function declaration respective to HMI layer
*
*
**********************************************************************************************************/
/** \file ICommon.h 
	 <b> Brief </b>	 This file contains the structure definition for Data Object, Object List and Base Module. \n
*********************************************************************************************************/
#ifndef	_ICOMMON_H_
#define _ICOMMON_H_

/** \file */

/** \page HMI_IF_COMMON_top HMI IF Common Package 

\subpage	HMI_IF_COMMON_Overview 
\n
\subpage	HMI_IF_COMMON_APIs
\n

*/

/** \page HMI_IF_COMMON_Overview Overview
HMI IF Common Package consists of: \n
 IBaseModule		:  This is the common interface which is used to get the module information of observer/listener, to add the listener and to initialize/ De-initialize the modules.\n
 HMIIF_IDataObject	:  Interface is used to access data. \n
 IObjectList		:  Interface is used to access data collections such as array, linked list etc.\n

*/

/** \page HMI_IF_COMMON_APIs HMI IF Common API's

<b> The below APIs are currently not used and it will return either TRUE/FALSE by default. </b>

 <ul>
 	<li>  #Radio_Default_IDataObject_GetId : This function is used to get the Id of data object. </li>
	<li>  #Radio_Default_IDataObject_Get : This function is used to get the values of data object id's. </li>
	<li>  #Radio_Default_IDataObject_Set : This function is used to set the data object Id. </li>
	<li>  #Radio_Default_IDataObject_Clone : This function is used to clone the Data object. </li>
	<li>  #Radio_Default_IDataObject_Delete : This function is used to delete the Data object. </li>
	<li>  #Radio_Default_IObjectList_GetAt : This function is used to get the object List. </li>
	<li>  #Radio_Default_IObjectList_GetCount : This function is used to get the count in object list. </li>
	<li>  #Radio_Default_IObjectList_Get : This function is used to get the default object list. </li>
	<li>  #Radio_Default_IObjectList_Set : This function is used to set the object lists. </li>
	<li>  #Radio_Default_IObjectList_Clone : This function is used to clone the object list. </li>
	<li>  #Radio_Default_IObjectList_Delete : This function is used to delete the object list. </li>
	<li>  #Radio_Default_IBaseModule_GetModuleInfo : This function is used to get the module information for base module. </li>
	<li>  #Radio_Default_IBaseModule_AddListener : This function registers the callback function for base modules. </li>
	<li>  #Radio_Default_IBaseModule_Init : This function is used to initialize the base module. </li>
	<li>  #_Radio_ModuleRun : This function is used to run the base module. </li>
	<li>  #_Radio_ModuleStop : This function is used to stop the base module. </li>
</ul> 

*/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "cfg_types.h"

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/


typedef struct HMIIF_IDataObject
{
       void* thiss;
       INT (*GetId)(void* thiss);
       BOOL (*Get)(void* thiss, INT nSubId, DWORD* pdwData);
}HMIIF_IDataObject;

typedef struct IBaseModule
{
       BOOL (*GetModuleInfo)(HMIIF_IDataObject** ppInfo);
       BOOL (*AddListener)(struct IBaseModule* pListener);
	   BOOL (*Init)(void);
	   BOOL (*DeInit)(void);
       BOOL (*Run)(HMIIF_IDataObject* pParameter);
       BOOL (*Stop)(HMIIF_IDataObject* pParameter);
       BOOL (*Get)(INT nId, DWORD* pdwData);
       BOOL (*Set)(INT nId, DWORD dwData);
}IBaseModule;

typedef struct IObjectList
{
 	void* thiss;
 	HMIIF_IDataObject* (*GetAt)(void*thiss,INT nIndex);
 	INT (*GetCount)(void*thiss);
}IObjectList;

typedef void (*PFN_NOTIFY)(HMIIF_IDataObject*);
//IDataObject ID definition of module infomation

enum ModuleInfoSubID
{
	MODULE_INFO_DOSID_NAME,
	MODULE_INFO_DOSID_VERSION,
	MODULE_INFO_DOSID_DATE,
	MODULE_INFO_DOSID_NOTIFY
};


#endif

/*=============================================================================
    end of file
=============================================================================*/