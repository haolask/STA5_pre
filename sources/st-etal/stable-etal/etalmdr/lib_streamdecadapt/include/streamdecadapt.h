//!
//!  \file 		streamdecadapt.h
//!  \brief 	<i><b>streamdecoder adaptation Header File</b></i>
//!  \details	This is the headerfile for the StreamDecoder adaptation to accordo 5.
//!  \author 	David Pastor
//!  \author 	(original version) David Pastor
//!  \version 	1.0
//!  \date 		21.09.2017
//!  \bug 		Unknown
//!  \warning	None
//!
#if !defined (STREAMDECADAPT_HEADER)
#define STREAMDECADAPT_HEADER

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "target_config.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------

#define OSAL_s32ThreadActivate(_a_) 
#define OSAL_s32ThreadPriority(_a_, _b_) 

//----------------------------------------------------------------------
// Exported variables (declared with extern statement)
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Exported Functions (declared with extern statement)
//----------------------------------------------------------------------
extern tS32 OSALIO_s32RemoveDevice (/*OSAL_tenDevID*/ tU32 DevId);
extern tS32 streamdecadapt_destructor(void);
extern tVoid DABMW_SeamlessInit(tVoid);

#ifdef __cplusplus
}
#endif

#endif
