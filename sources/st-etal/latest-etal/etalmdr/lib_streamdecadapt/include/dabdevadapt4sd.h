//!
//!  \file       dabdevadapt4sd.h
//!  \brief      <i><b>dabdev audio raw input Header File</b></i>
//!  \details    This is the headerfile for the dabdev audio raw input handling
//!  \author     David Pastor
//!  \author     (original version) David Pastor
//!  \version    1.0
//!  \date       17.10.2017
//!  \bug        Unknown
//!  \warning    None
//!

#ifndef DABDEVADAPT4SD_HEADER
#define DABDEVADAPT4SD_HEADER

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------------------
// Defines
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Typedef / Enums
//----------------------------------------------------------------------



//----------------------------------------------------------------------
// Exported variables (declared with extern statement)
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Exported Functions (declared with extern statement)
//----------------------------------------------------------------------

extern tBool DABMW_Check_SendAudioToAudioChannel(tS32 fd_dev);
extern tBool DABMW_Check_PlayAudio(tVoid);

#ifdef __cplusplus
}
#endif

#endif

// End of file

