//!
//!  \file       dabdevadapt.h
//!  \brief      <i><b>dabdev audio raw input Header File</b></i>
//!  \details    This is the headerfile for the dabdev audio raw input handling
//!  \author     David Pastor
//!  \author     (original version) David Pastor
//!  \version    1.0
//!  \date       11.10.2017
//!  \bug        Unknown
//!  \warning    None
//!

#ifndef DABDEVADAPT_HEADER
#define DABDEVADAPT_HEADER

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

extern tS32 add_api_ms_notify_channel_selection(DABMW_mwAppTy app, uint8 output_interface_id, uint8 action_onoff, uint8 mode, uint8 subch_id, uint16 subch_size);
extern tVoid DABDEV_CbDataPath_dab_audio_raw(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);

#ifdef __cplusplus
}
#endif

#endif

// End of file

