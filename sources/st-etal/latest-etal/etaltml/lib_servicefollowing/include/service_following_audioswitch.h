//!
//!  \file      service_following_mainloopl.h
//!  \brief     <i><b> This header file contains internal functions and variable for service following main loop  </b></i>
//!  \details   This header contains declarations related to service following feature
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2013.10.07
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_AUDIOSWITCH_H_
#define SERVICE_FOLLOWING_AUDIOSWITCH_H_

#ifdef __cplusplus
extern "C" {
#endif

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

/* CONST
*/


/* variables are belonging to SERVICE_FOLLOWING_AUDIOSWITCH_C
*/


/*
*********************
* FUNCTIONS SECTION
**********************
*/
#ifndef SERVICE_FOLLOWING_AUDIOSWITCH_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif


/* Procedure which cares of the switch between original and AF
* assuming everything is calculated and set previously
*/
GLOBAL tVoid DABMW_ServiceFollowing_SwitchToAF(tVoid);

/* procedure to request the FM switch 
*/
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmAFSwitchRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency);


// Local function for audio port switch path tracking
GLOBAL tSInt DABMW_SF_SetCurrentAudioPortUser (DABMW_SF_mwAppTy app, tBool isInternalTune);

#undef GLOBAL


#ifdef __cplusplus
}
#endif

#endif // SERVICE_FOLLOWING_AUDIOSWITCH_H_

// End of file

