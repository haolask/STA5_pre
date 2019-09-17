//!
//!  \file 		 steci_lld.h
//!  \brief 	 <i><b> STECI low level driver interface </b></i>
//!  \details Interface file for low level driver functionalities.
//!           Functionality provided in this module are usually to be rewritten for the 
//!           specific hardware.
//!  \author 	Alberto Saviotti
//!

#ifndef STECI_LLD_H
#define STECI_LLD_H

#ifdef __cplusplus
extern "C" {
#endif

extern tBool STECI_GetReqLevel (STECI_deviceInfoTy *deviceInfoPtr);

extern tBool STECI_ToggleReqLevel (tBool reqLevel);

extern tS32 STECI_ChipSelectAssert (STECI_deviceInfoTy *deviceInfoPtr);

extern tS32 /*@alt void@*/STECI_ChipSelectDeassert (STECI_deviceInfoTy *deviceInfoPtr);

extern tS32 STECI_SpiWriteSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tVoid *lpBufferPnt, tU32 dwBytesToWrite, tVoid *RxBuffer,
                                     tBool *nextReqActiveStatusPtr, tU8 *tmpTxDataPtr,
                                     tBool waitReadyCondition, tBool closeCommunication);

extern tS32 /*@alt void@*/STECI_ResetDevice (STECI_deviceInfoTy *deviceInfoPtr);

extern tS32 STECI_SetDeviceBootModeAndRestart (STECI_deviceInfoTy *deviceInfoPtr, tBool bootSelLevel);

#ifdef __cplusplus
}
#endif

#endif // STECI_LLD_H

// End of file
