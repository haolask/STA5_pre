//!
//!  \file         audioin.h
//!  \brief     <i><b>/dev/audioin device Header File</b></i>
//!  \details    This is the headerfile for the OSAL /dev/audioin device
//!  \author     David Pastor
//!  \author     (original version) David Pastor
//!  \version     1.0
//!  \date         03.10.2017
//!  \bug         Unknown
//!  \warning    None
//!

#ifndef AUDIOIN_HEADER
#define AUDIOIN_HEADER

/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------------------
// includes
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Defines and Macro
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// typedefs
//----------------------------------------------------------------------



//----------------------------------------------------------------------
// Variable Declaration
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// function prototypes
//----------------------------------------------------------------------

extern tVoid AUDIO_IOIN_vIOInit(void);
extern tVoid AUDIO_IOIN_vIODeInit(void);

extern tS32 AudioIOIN_s32IOOpen(OSAL_tenDevID id, tCString coszName,OSAL_tenAccess enAccess, OSAL_tIODescriptor * pfd);
extern tS32 AudioIOIN_s32IOClose(OSAL_tIODescriptor fd);
extern tS32 AudioIOIN_s32IOControl(OSAL_tIODescriptor fd, tS32 s32fun, tS32 s32arg);


#ifdef __cplusplus
}
#endif

#endif

// End of file

