//!  \file 		rds_af_data.h
//!  \brief 	<i><b> RDS AF manager header </b></i>
//!  \details	RDS AF related management header
//!  \author 	Alberto Saviotti
//!  \author 	(original version) Alberto Saviotti
//!  \version   1.0
//!  \date 		2011.10.06
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef RDS_AF_MNGR_H_
#define RDS_AF_MNGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DABMW_AF_ENTRY_NOT_TO_BE_USED_VALUE         (tU8)0
#define DABMW_AF_ENTRY_FILLER_CODE_VALUE            (tU8)205
#define DABMW_AF_ENTRY_NO_AF_EXISTS_VALUE           (tU8)224
#define DABMW_AF_ENTRY_AF_NUM_BASE_VALUE            (tU8)225
#define DABMW_AF_ENTRY_LFMF_FREQUENCY_FOLLOW_VALUE  (tU8)250

#define DABMW_AF_LIST_BFR_LEN_16BITS                (tU8)(DABMW_AF_LIST_BFR_LEN / 2 + 1) // Add 1 to consider the odd number divided by 2

extern tSInt DABMW_RdsAfDataInit (tVoid);

extern tSInt DABMW_RdsAfDataMngr (tSInt slot, tU32 piVal, tU32 freq);

extern tSInt DABMW_RdsAfDataPush (tSInt slot, tU8 data, tU8 pos);

extern tSInt /*@alt void@*/DABMW_RdsAfDataReInit (tSInt slot);

#ifdef __cplusplus
}
#endif

#endif // RDS_AF_MNGR_H_

// End of file

