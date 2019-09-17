//!
//!  \file      service_following_external_radio.h
//!  \brief     <i><b> Service following implementation : external interface definition => radio part </b></i>
//!  \details   This file provides functionalities for external interface : radio  part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RADIO_H
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RADIO_H

/*
*********************
* DEFINE SECTION
**********************
*/




/*
*********************
* STRUCTURE SECTION
**********************
*/
	

/*
*********************
* VARIABLE SECTION
**********************
*/



/*
*********************
* FUNCTIONS SECTION
**********************
*/

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RADIO_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_TuneServiceIdCallback(DABMW_SF_mwAppTy vI_app, tU32 vI_tunedFreq, tBool vp_PIconfirmedStatus);

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_TuneFrequency (DABMW_SF_mwAppTy vI_app, tU32 vI_frequency);

GLOBAL DABMW_SF_mwAppTy DABMW_ServiceFollowing_ExtInt_GetAppFromEtalPath(EtalPathName vI_etalPath);
GLOBAL EtalPathName DABMW_ServiceFollowing_ExtInt_GetEtalPath(DABMW_SF_mwAppTy vI_app);


#undef GLOBAL


#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RADIO_H

