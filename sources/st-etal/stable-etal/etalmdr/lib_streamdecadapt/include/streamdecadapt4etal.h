//!
//!  \file 		streamdecadapt.h
//!  \brief 	<i><b>streamdecoder4etal adaptation Header File</b></i>
//!  \details	This is the headerfile for the StreamDecoder Etal adaptation to accordo 5.
//!  \author 	David Pastor
//!  \author 	(original version) David Pastor
//!  \version 	1.0
//!  \date 		16.01.2018
//!  \bug 		Unknown
//!  \warning	None
//!
#if !defined (STREAMDECADAPT4ETAL_HEADER)
#define STREAMDECADAPT4ETAL_HEADER

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
// Exported variables (declared with extern statement)
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Exported Functions (declared with extern statement)
//----------------------------------------------------------------------
extern tSInt ETAL_cmdSeamlessEstimation_DABMW(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessEstimationConfigTy *seamlessEstimationConfig);
extern tSInt ETAL_cmdSeamlessSwitching_DABMW(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig);

#ifdef __cplusplus
}
#endif

#endif
