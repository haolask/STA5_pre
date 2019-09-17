//!
//!  \file 		rds_data.h
//!  \brief 	<i><b> RDS data manager header </b></i>
//!  \details	RDS data related management header
//!  \author 	Alberto Saviotti
//!  \author 	(original version) Alberto Saviotti
//!  \version   1.0
//!  \date 		2011.09.20
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef RDS_DATA_H_
#define RDS_DATA_H_

#ifdef __cplusplus
extern "C" {
#endif

// Default values for data threshold
#define DABMW_RDS_DEFAULT_QC_THR                        (tU8)6
#define DABMW_RDS_DEFAULT_CP_THR                        (tU8)1
#define DABMW_RDS_DEFAULT_CRITDATA_THR                  (tU8)1
// define a max PI value criteria
// then, when an erronous data is received, threshold is decrease by 1
// this will defne the max erronous value accepted..
// 
#define DABMW_RDS_DEFAULT_NB_PI_ERROR					((tU8)1)

// RDS Data Groups
#define DABMW_GROUP_0A                                  (tU8)0
#define DABMW_GROUP_0B                                  (tU8)1
#define DABMW_GROUP_1A                                  (tU8)2
#define DABMW_GROUP_1B                                  (tU8)3
#define DABMW_GROUP_2A                                  (tU8)4
#define DABMW_GROUP_2B                                  (tU8)5
#define DABMW_GROUP_3A                                  (tU8)6
#define DABMW_GROUP_3B                                  (tU8)7
#define DABMW_GROUP_4A                                  (tU8)8
#define DABMW_GROUP_4B                                  (tU8)9
#define DABMW_GROUP_5A                                  (tU8)10
#define DABMW_GROUP_5B                                  (tU8)11
#define DABMW_GROUP_6A                                  (tU8)12
#define DABMW_GROUP_6B                                  (tU8)13
#define DABMW_GROUP_7A                                  (tU8)14
#define DABMW_GROUP_7B                                  (tU8)15
#define DABMW_GROUP_8A                                  (tU8)16
#define DABMW_GROUP_8B                                  (tU8)17
#define DABMW_GROUP_9A                                  (tU8)18
#define DABMW_GROUP_9B                                  (tU8)19
#define DABMW_GROUP_10A                                 (tU8)20
#define DABMW_GROUP_10B                                 (tU8)21
#define DABMW_GROUP_11A                                 (tU8)22
#define DABMW_GROUP_11B                                 (tU8)23
#define DABMW_GROUP_12A                                 (tU8)24
#define DABMW_GROUP_12B                                 (tU8)25
#define DABMW_GROUP_13A                                 (tU8)26
#define DABMW_GROUP_13B                                 (tU8)27
#define DABMW_GROUP_14A                                 (tU8)28
#define DABMW_GROUP_14B                                 (tU8)29
#define DABMW_GROUP_15A                                 (tU8)30
#define DABMW_GROUP_15B                                 (tU8)31

// Define invalid value for PI
#define DABMW_INVALID_RDS_PI_VALUE                      (tU32)0xFFFFFFFF

// Lengths in bytes
#define DABMW_RDS_PS_LENGTH                             (tU8)8
#define DABMW_RDS_PI_LENGTH                             (tU8)2
#define DABMW_RDS_PTY_LENGTH                            (tU8)(1 + 8 + 16) // 1 byte data, 8 bytes short display string, 16 bytes long display string
#define DABMW_RDS_TPTA_LENGTH                           (tU8)1
#define DABMW_RDS_PTYTPTAMS_LENGTH                      (tU8)2
#define DABMW_RDS_TIME_LENGTH                           (tU8)6
#define DABMW_RDS_RT_LENGTH                             (tU8)64
#define DABMW_RDS_DI_LENGTH                             (tU8)1
#define DABMW_RDS_RT_MAX_LEN_BYTES                      (tU8)64
#define DABMW_EON_BUFFER_SIZE                           (tU8)15

extern tVoid DABMW_RdsMain (DABMW_storageSourceTy source, tU32 data, tU32 error_ratio);

extern tSInt DABMW_RdsDataInit (tVoid);

extern tSInt DABMW_RdsGetSlotFromSource (DABMW_storageSourceTy source);

extern tSInt DABMW_RdsDataSetup (DABMW_storageSourceTy source, tU8 criticalDataThr);

extern tSInt DABMW_RdsGetPs (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet);

extern tSInt DABMW_RdsGetPi (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet);

extern tSInt DABMW_RdsCheckPi (tSInt slot, tPU32 piPtr);

extern tSInt DABMW_RdsGetPty (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet);

extern tSInt DABMW_RdsGetTpTa (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet);

extern tSInt DABMW_RdsGetPtyTpTaMs (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet);

extern tSInt DABMW_RdsGetTime (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet);

extern tSInt DABMW_RdsGetRt (tSInt slot, tPU8 dataPtr, tBool forcedGet);

extern tSInt DABMW_RdsGetDi (tSInt slot, tPU8 dataPtr, tBool forcedGet);

extern tSInt DABMW_RdsGetAf(tSInt slot, ETAL_HANDLE hReceiver, tPU8 dataPtr, tBool forcedGet, tU32 piVal, tU32 baseFreq, 
                                        tBool mode, tU32 maxNumberRetrieved);

extern tVoid DABMW_RdsDataEvent (DABMW_storageSourceTy source, DABMW_eventTy event);

extern tU32 DABMW_RdsGetErrorRatio (DABMW_storageSourceTy source);

extern tSInt DABMW_RdsDataSetupPiDetectionMode (DABMW_storageSourceTy source, tBool vI_Isfast_PI_detection);

extern tSInt DABMW_RdsDataSetupIncreasePiMatchingAF (DABMW_storageSourceTy source, tU32 vI_expected_PI);

extern tSInt DABMW_RdsForceGetPiData (tSInt slot, DABMW_storageStatusEnumTy *pPIStatus, tU32 *pCurrentPI, tU32 * pLastPI, tU32 * pBackupPI);

extern tSInt DABMW_RdsForceGetTPTAData (tSInt slot, tVoid ** ppData);

extern tSInt DABMW_RdsForceGetF_RDSStation (tSInt slot, tBool * pData);

extern tSInt DABMW_RdsForceGetPTYData (tSInt slot, tVoid** ppData);

extern tVoid DABMW_CleanUpTPTA (tSInt slot);

#ifdef __cplusplus
}
#endif

#endif // RDS_DATA_H_

// End of file
