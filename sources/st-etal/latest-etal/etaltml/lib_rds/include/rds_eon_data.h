//!  \file      rds_eon_data.h
//!  \brief     <i><b> RDS EON manager header </b></i>
//!  \details   RDS EON related management header
//!  \author    Alberto Saviotti
//!  \author    (original version) Alberto Saviotti
//!  \version   1.0
//!  \date      2012.02.02
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef RDS_EON_MNGR_H_
#define RDS_EON_MNGR_H_

#ifdef __cplusplus
extern "C" {
#endif

extern tSInt DABMW_RdsEonDataMngr (tSInt slot,
                                   tU32 piVal, tU8 realGroup,
                                   tBool availBlock_A, tU32 block_A, 
                                   tBool availBlock_B, tU32 block_B, 
                                   tBool availBlock_C, tU32 block_C, 
                                   tBool availBlock_Cp, tU32 block_Cp, 
                                   tBool availBlock_D, tU32 block_D);

extern tVoid DABMW_RdsEonCleanUpData (tSInt slot);

#ifdef __cplusplus
}
#endif

#endif // RDS_EON_MNGR_H_

// End of file

