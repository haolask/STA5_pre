/*=============================================================================
start of file
=============================================================================*/
/**************************************************************************************************/
/** \file DAB_HAL_Interface.h																       *
* Copyright (c) 2016, Jasmin Infotech Private Limited.                                             *
*  All rights reserved. Reproduction in whole or part is prohibited                                *
*  without the written permission of the copyright owner.                                          *
*                                                                                                  *
*  Project              :  ST_Radio_Middleware                                                     *
*  Organization			:  Jasmin Infotech Pvt. Ltd.                                               *
*  Module				:  RADIO_DAB_HAL                                                           *
*  Description			:  This file contains declarations of functions that interface DAB Tuner   *
*                          Ctrl with SoC                                                           *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
#ifndef __DAB_HAL_INTERFACE_H__
#define __DAB_HAL_INTERFACE_H__


/** \file */
/** \page DAB_TUNER_CTRL_Messages_top DAB Tuner Control messages package

\subpage DAB_TUNER_CTRL_Messages_Overview
\n
\subpage DAB_TUNER_CTRL_Messages_API_Functions
\n
*/

/**\page DAB_TUNER_CTRL_Messages_Overview Overview   
    \n
     DAB Tuner Control messages package consists of message API's that are processed by SoC.   
    \n\n
*/

/** \page DAB_TUNER_CTRL_Messages_API_Functions API Functions 
    <ul>
        <li> #DabTuner_MsgSndSetTuneTocmd         : Tune command. </li>
        <li> #DabTuner_MsgSndCreateReciver_Cmd       : Create receiver command.</li>
		<li> #DabTuner_MsgSndStartScan_Cmd         : Start scan command. </li>
		<li> #DabTuner_MsgSndSelectService_Cmd         : Select service command. </li>
		<li> #DabTuner_MsgCmd_SelectComponent         : Select component command. </li>
		<li> #DabTuner_MsgCmdDestroyReciver         : Destroy receiver command. </li>
		<li> #DabTuner_MsgSndScan2GetComponentListReq_Cmd         : Get component list command. </li>
		<li> #DabTuner_MsgRcvCreateReceiver         : Receive create receiver reply. </li>
		<li> #DabTuner_MsgRcvStartScan         : Receive start scan reply. </li>
		<li> #DabTuner_MsgRcvScanStatusNot         : Receive scan notification. </li>
		<li> #DabTuner_MsgRcvScan2GetComponentListReply         : Receive get component list reply. </li>
		<li> #DabTuner_MsgRcvSetTuneStatusNot         : Receive set tune status notification . </li>
		<li> #DabTuner_MsgRcvTuneTo         : Receive tune to reply. </li>
		<li> #DabTuner_MsgReply_SelectComponent         : Receive select component reply. </li>
		<li> #DabTuner_MsgRcvUpNot         : Receive up notification. </li>
		<li> #DabTuner_MsgRcvSelectServiceReply         : Receive select service reply. </li>
		<li> #DabTuner_MsgReplyDestroyReceiver         : Receive destroy receiver reply. </li>
		<li> #DabTuner_MsgRcvSetTuneStatus         : Receive set tune status reply. </li>
		<li> #DabTuner_MsgSndVersionRequest_Cmd         : Get version request command. </li>
		<li> #DabTuner_MsgRcvVersionRequest         : Request for tuning to station in station list. </li>
		<li> #DAB_CheckSum         : Checksum calculation. </li>
		<li> #DAB_RxCmdAnalysis         : Checksum validation. </li>
    </ul>
*/
/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_Types.h"
#include "etal_types.h"
#include "etal_api.h"
#include "etalversion.h"
#include "DAB_Tuner_Ctrl_Notify.h"

/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/

#define     DAB_TUNER_CTRL_CREATE_RECEIVER_OPCODE                    0xE0
#define     DAB_TUNER_CTRL_TUNE_CMD_OPCODE                           0xEA
#define     DAB_TUNER_CTRL_SET_TUNE_STATUS_NOTIFICATION_CMD_OPCODE   0x19
#define     DAB_TUNER_CTRL_SET_TUNE_STATUS_NOTIFICATION_OPCODE       0x13
#define     DAB_TUNER_CTRL_SCAN_CMD_OPCODE                           0x16                         
#define     DAB_TUNER_CTRL_INST_HSM_CID                             (0x12)
#define     DABTUNER_SNDCMD_MAX                                      70U
#define     DABTUNER_CMDID_STARTSCAN_CMD                             0x48
#define     UPDOWN                                                   0x01

#define     SYSTEM_ERROR_NOTIFICATION_MAX_STRING_LENGTH 	        (52u)

#define     MAX_LINKAGE_SETS										 5
#define     DAB_MAX_LINKAGE_PARTS		                        	 4u
#define     MAX_ENSEMBLE_SERVICES               					 32  // As per ETAL standard
#define     MAX_COMPONENTS_PER_SERVICE								2
#define     DAB_TUNER_CTRL_OE_SERVICES_MAX_NUM_EID     			    ((Tu8) 10)
#define		MAX_SW_VERSION_LENGTH									(Tu8)48
#define		MAX_HW_VERSION_LENGTH									(Tu8)10

#define		FIC_DATA_BUFFER_MAX_SIZE								128	/* Max 4 FIBS(32bytes) -- 128 bytes*/

#define		TUNER_1													1
#define		QUALITYMONITOR_NOTIFYTIME								1000			//IN  MILLISECONDS

/* As per DAB Standard SLS image maximum size is 50 KB */
#define		MAX_DATA_SERVICE_PAYLOAD_SIZE							51200

#define		DATA_SERVICE_TYPE_SIZE									4  /* bytes */

typedef enum
{
	FOREGROUND_CHANNEL,
	BACKGROUND_CHANNEL
}Te_Frontend_type;

typedef struct
{
	Tu32 pUEId;
	Tu8 pCharset;
	Tchar pLabel[17];
	Tu16 pCharflag;
}EtalEnsembleinfo;

typedef struct
{
	Ts32		RFFieldStrength;
	Ts32		BBFieldStrength;
	Tu32		FicBitErrorRatio;
	Tbool		isValidMscBitErrorRatio;
	Ts32		MscBitErrorRatio;
}Ts_Dabtunerctrl_EtalQualityEntries;
/*--------------------------------------------------------------------------------------------------
type definitions
--------------------------------------------------------------------------------------------------*/
/**
* @brief SystemControl_repl
*/
typedef struct
{
	Tu8	ReplyStatus;

}Ts_SystemControl_repl;

/**
* @brief SystemMonitoring_not
*/
typedef struct
{
	Tu16	MonitorId;
	Tu16	Status;

}Ts_SystemMonitoring_not;

/**
* @brief SystemMonitoring_not
*/
typedef struct
{
	Tu8		ProcessorId;
	Tu8		ErrorType;
	Tu16	Length;
	Tu8		DataByte[SYSTEM_ERROR_NOTIFICATION_MAX_STRING_LENGTH];

}Ts_SystemError_not;

/**
* @brief Structure to update the SPI commands.Structure instance is updated and sent to SoC.
*/
typedef struct
{
	Tu8   data[DABTUNER_SNDCMD_MAX];
	Tu16  len;
	Tu8   padding[2];

}Ts_DabTunerMsg_sCmd;

/**
* @brief Structure to update the major and minor version received while reading the up notifications
		  from SoC.
*/
typedef struct
{
	Tu8			majorver;
	Tu8			minorver;
	Tu8         padding[2];

}Ts_DabTunerMsg_N_UpNot;

/**
* @brief Structure to update receiver reply from SoC for create receiver command.
*/
typedef struct
{
	Tu8			 reply;
	Tu8          rx_handle;
	Tu8          padding[3];
}Ts_DabTunerMsg_R_CreateReceiver;

/**
* @brief Structure to update receiver reply from SoC for version request command.
*/
typedef struct
{
	Tu8			reply;
	Tu8         NoOfCharsSW;
	Tu8         NoOfCharsHW;
	Tu8         SWVersion[256];
	Tu8         HWVersion[256];
	Tu8         padding;
}Ts_DabTunerMsg_R_GetVersion;

 /**
 * @brief Structure to update receiver reply from SoC for version request command.
 */
typedef struct
{
	Tu8         NoOfCharsSW;
    Tu8         NoOfCharsHW;
    Tu8         SWVersion[MAX_SW_VERSION_LENGTH];
	Tu8         HWVersion[MAX_HW_VERSION_LENGTH];
}Ts_DabTunerMsg_R_GetVersion_Reply;

 /**
* @brief Structure to update receiver reply from SoC for set tune status notification command.
*/
typedef struct
{
	Tu8    Reply;
	Tu8    padding[3];

}Ts_DabTunerMsg_R_SetTuneStatus;

/**
* @brief Structure to update receiver reply from SoC for Abort scan command.
*/
typedef struct
{
	Tu8    Reply;
	Tu8    padding[3];

}Ts_AbortScan2Reply;

/**
* @brief Structure to update  receiver reply from SoC for scan command.
*/
typedef struct
{
	Tu8			reply;
	Tu8         padding[3];

}Ts_DabTunerMsg_R_StartScan_Cmd;

/**
* @brief Structure to update service information during scan notification reception from SoC.
*/
typedef struct
{
	Tu32     ServiceId;
	Tu8	   CA_Applied;
	Tu8	   PDFlag;
	Tu8	   CAId;
	Tu8      Protectionlevel;
	Tu8	   RSIindicator;
	Tu8	   Secondaryaudiocompflag;
	Tu8	   Secondarydatacompflag;
	Tu8	   Subchanlid;
	Tu8	   InternTableIdSPTy;
	Tu8	   StatPTy;
	Tu8	   InternTableIdDPTy;
	Tu8	   DynPTy;
	Tu8	   CharSet;
	Tu8	   LabelString[17];
	Tu8	   ShortLabelFlags[2];

	Tu8      padding;
}Ts_t_sat_serviceinfo;

/**
* @brief Structure to update ensemble information during scan notification reception from SoC.
*/
typedef struct
{
	Ts_t_sat_serviceinfo    t_sat_ScanStatusNot[100];
	Tu32                    prevfrequency;
	Tu32		            frequency;
	Tu16		            EnsembleIdentifier;
	Tu8			            DabSignalPresent;
	Tu8			            DABMode;
	Tu8			            EnsembleInfoFlag;
	Tu8			            SIAvailableFlag;
	Tu8			            FreqChangeOnly;
	Tu8			            BERsignificant;
	Ts8			            BERexponent;
	Ts8			            SNRlevel;
	Tu8						RSSI_ValidFlag;
	Ts8						RSSI;
	Tu8						TimeOutFlag;
	Tu8			            ECC;
	Tu8						SINotComplete;
	Tu8			            CharSet;
	Tu8			            num_of_Ser;
	Tu8			            LabelString[17];
	Tu8			            ShortLabelFlags[2];
	Tu8                     padding;

}Ts_DabTunerMsg_R_ScanStatus_Not;

/**
* @brief Structure to update component information during scan notification reception from SoC.
*/
typedef struct
{
	Tu16        ShortLabelFlags;
	Tu16        UAType;
	Tu16        InternalCompId;
	Tu8         Primary;
	Tu8         CA_Applied;
	Tu8         Activated;
	Tu8         CharSet;
	Tu8         Language;
	Tu8         TransportMech;
	Tu8         ComponentType;
	Tu8         LabelString[17];
	Tu8         padding[3];
}Ts_DabTuner_Components;

/**
* @brief Structure to update tune notification received from SoC.
*/
typedef struct
{
	Tu32		Frequency;
	Tu8 		reply;
	Tu8			DABSignalPresent;
	Tu8			MCIAvailableFlag;
	Tu8			MCIandSIAvailableFlag;
	Tu8			DABMode;
	Tu8			FreqChangeOnly;
	Tu8         padding;
}Ts_DabTunerMsg_R_SetTuneStatusNot;

/**
* @brief Structure to update receiver reply from SoC for select component command.
*/
typedef struct
{
	Tu8         reply;
	Tu8         contextid;
	Tu8         padding[2];
}Ts_Select_ComponentReply;

/**
* @brief Structure to update receiver reply from SoC for tune command.
*/
typedef struct
{
	Tu8 		reply;
	Tu8         padding[3];
}Ts_DabTunerMsg_R_TuneTo;

typedef struct
{
	Tu8 Activated;
	Tu16 InternalCompId;

}Ts_SelectServiceComponents;

typedef struct
{
	Ts_SelectServiceComponents SelectserviceComponets[3];
	Tu8 ReceiverHandle;
	Tu8 Opcode;
	Tu8 ReplyStatus;
	Tu8 ContextId;
	Tu8 NrOfSelectedComponents;

}Ts_DabTunerMsg_SelectServiceReply;

/**
* @brief Structure to update receiver reply from SoC for destroy receiver command.
*/
typedef struct
{
	Tu8         reply;
	Tu8         padding[2];

}Ts_DabTunerMsg_R_DestroyReceiver_Cmd;


typedef struct
{
	Tu8    Reply;
	Tu8    padding[3];
}Ts_DabTunerMsg_R_SetAudioStatusReply;


typedef struct
{
	Tu8 AudioSynchronised;
	Tu8 DecodingStatus;

}Ts_DabTunerMsg_R_AudioStatus_notify;

/*************************************Bit Error Rate ************************************/

typedef struct
{
	Tu8    Reply;
	Tu8    padding[3];
}Ts_DabTunerMsg_R_PeriodicalBERQual_reply;

typedef struct
{
	Tu8  FICValidFlag;
	Tu8  SubChannelValidFlag;
	Tu8  FIC_BER_Significant;
    Ts8  FIC_BER_Exponent;
    Tu8 Subchannel_BER_Significant; 
    Ts8  Subchannel_BER_Exponent;	
}Ts_DabTunerMsg_R_PeriodicalBERQual_Notify;

/*=============================================================================
end of file
=============================================================================*/

/*************************************RSSI ************************************/
typedef struct
{
	Tu8    Reply;
	Tu8    padding[3];
}Ts_DabTunerMsg_R_RSSINotifierSettings_reply;

typedef struct
{
	Ts8    TunerLevel;
	Tu8    padding[3];

}Ts_DabTunerMsg_R_RSSI_notify;

/********************************SNR************************************/
typedef struct
{
	Tu8    Reply;
	Tu8    padding[3];
}Ts_DabTunerMsg_R_SetSNRNotifier_reply;

typedef struct
{
	Ts8    SNRLevel;
	Tu8    padding[3];

}Ts_DabTunerMsg_R_SNRNotifier;

/**
* @brief Structure to update receiver reply from SoC for get component list command.
*/
typedef struct
{
	Ts_DabTuner_Components Component[2];
	Tu8                    reply;
	Tu8                    ContextId;
	Tu8                    NoOfComponents;
	Tu8                    padding;

}Ts_DabTunerMsg_GetComponentList_Reply;

typedef struct
{
	Ts_DabTunerMsg_GetComponentList_Reply st_compInfo;
	Tu32                                  ProgServiceId;
	Tu16	                              ShortLabelFlags;
	Tu8	                   CA_Applied;
	Tu8	                   CAId;
	Tu8	                   CharSet;
	Tu8	                   InternTableIdSPTy;
	Tu8	                   StatPTy;
	Tu8	                   InternTableIdDPTy;
	Tu8	                   DynPTy;
	Tu8	                   LabelString[17];

}Ts_CurrEnsemble_serviceinfo;

typedef struct
{
	//Ts_Tuner_Ctrl_BasicEnsembleInfo   st_CurrentEnsembleInfo;
	Ts_CurrEnsemble_serviceinfo        st_serviceinfo[MAX_ENSEMBLE_SERVICES];
	Tu8	u8_ReplyStatus;
	Tu8    u8_NumOfServices;

}Ts_DabTunerMsg_GetCurrEnsembleProgListReply;

typedef struct
{
	Tu8  u8_reply;
	Tu8  padding[3];
}Ts_SearchNext_Reply;

typedef struct
{
	Tu32 Sid[DAB_MAX_LINKAGE_PARTS][MAX_HARDLINK_SID];
	Tu8 idflag;
	Tu8	Activelink;
	Tu8 Hardlink;
	Tu8 Ils;
	Tu16 Lsn;
	Tu8 Idlq[DAB_MAX_LINKAGE_PARTS];
	Tu8 Noofids[DAB_MAX_LINKAGE_PARTS];
	Tu8 OE;
	Tu8 Pd;
	//Ts_basicensembleinfo ensembleinfo;
}Ts_Lsn_Info;

typedef struct
{
	Tu32 u32_SId;                                       /**< Service ID */
	Tu16 au16_EId[DAB_TUNER_CTRL_OE_SERVICES_MAX_NUM_EID];  /**< Ensemble IDs of other ensembles */
	Tu8  u8_numEId;                                    /**< Number of Ensemble IDs */
} Ts_dab_oeServices;

/**
* @brief Structure definition for ensemble information.
*
* This is the data structure definition for collected ensemble information.
*/
typedef struct
{
	Tu16                      EId;                    /**< Ensemble identifier */
	Tu8                       ECC;                    /**< Extended country code */
}Ts_dab_ensembleProperties;

/**
* @brief Ensemble Information:
*        Ensemble Information is stored using the following structure. It is used in context of #SERVICE_DABTUN_tuneEnsemble.
*/
typedef struct
{
	Tu32 frequency;     /**< Frequency in kHz */
	Tu16 EId;           /**< Ensemble Identifier */
	Tu8  ECC;           /**< Extended County Code */
	Tu8  signalQuality; /**< Signal quality 0..100 */
} Ts_dab_ensembleInfo;

typedef struct
{
	Tu8 Audiosyncronised;
	Tu8 Decodingstatus;
	Tu8 ComfortNoiseLevelValidFlag;
	Tu8 AudioQualityvalidflag;
	Tu8 AudioQuality;
	Tu8 AudioLevelValidFlag;
	Tu8 AudioLevel;
	Tu8 HighResolutionNoiseLevel;
	Tu8 ComfortNoiseLevel;
	Tu8 CutOffFrequencyValid;
	Tu16 CutOffFrequency;

}Ts_AudioStatus2_not;


typedef struct
{
	Tu8 Replystatus;
	Tu8 Audiosyncronised;
	Tu8 Decodingstatus;
	Tu8 AudioQualityvalidflag;
	Tu8 AudioQuality;

}Ts_Set_Audio_Status_Notifier_Reply;


typedef struct
{
	Tu8 u8_ReplyStatus;

}Ts_DabTunerMsg_R_SynchReply;

typedef struct
{
	Tu8 TimeSyncLevel;
	Tu8 FrequencySyncLevel;
	Tu8 NullSymbolFound;
	Tu8 ChannelSync;
}Ts_DabTunerMsg_R_SynchNotification;

typedef struct
{
	Tu8		ReplyStatus;
	Tu8		AudioSynchronised;
	Tu8		DecodingStatus;
	Tu8 	AudioQualityValidFlag;
	Tu8		AudioQuality;

}Ts_DabTunerMsg_GetAudioStatus2_reply;

typedef struct
{
	Tu8    Reply;
	Tu8    padding[3];
}Ts_PrepareForBlending_Reply;

typedef struct
{
	Tu8    Reply;
	Tu8    padding[3];
}Ts_ResetBlending_Reply;

typedef struct
{
	Ts32	Delay;
	Tu16	LevelData;
	Tu8		CapturedTime;
	Tu8		CorrelationQuality;
	Tu8		DelayFound;
	Tu8		ConfidenceLevel;
	Tu8		LevelStatus;
	Tu8		CompensateFMDriftDone;

}Ts_PrepareForBlending_Notify;

typedef struct
{
	Tu8		Done;
	Tu8		Source;

}Ts_TimeAlignmentForBlending_Notify;

typedef struct
{
	Tu8		ReplyStatus;

}Ts_StartTimeAlignmentForBlending_repl;

typedef struct
{
	Tu8    Reply;
	Tu8    padding[3];
}Ts_StartBlending_Reply;

typedef struct
{
	Tu8    ReplyStatus;
	Tu8    ECC;
	Tu16   EnsembleIdentifier;
	Tu8    CharSet;
	Tu8    LabelString[17];
	Tu16   ShortLabelFlags;
	Tu8	   EncodingFlag;
	Tu16   ExtendedLabelFlags;
	Tu8    NrOfLabelStringBytes;
	Tu8    ExtendedLabelByte[256];
}Ts_DabTunerGetEnsembleProperties_reply;

typedef struct
{
	Tu8 replyStatus;
	Tu8 ContextId;
}Ts_DabTunerMsg_RegisterSinkToServComp_reply;

typedef struct
{
	Tu8 Runningstatus;
	Tu8 Contenttype;
	Tu8 CharacterSet;
	Tu8 LabelLength;
	Tu8 DLS_LabelByte[MAX_DLS_LENGTH];
}Ts_dab_tuner_ctrl_DLS_data;

/**
* @brief Structure comprises the reply of SetServListChangedNotifier_repl
*/
typedef struct
{
	Tu8 ReplyStatus;

}Ts_DabTunerMsg_SetServListChangedNotifier_repl;

/**
* @brief Structure comprises service information used in Ts_DabTunerMsg_ProgrammeServListChanged_not structure
*/
typedef struct
{
	Tu8 CA_Applied;
	Tu8 CAId;
	Tu8	CharSet;
	Tu8 LabelString[DAB_TUNER_CTRL_MAX_LABEL_LENGTH];
	Tu16 ShortLabelFlags;
	Tu32 ProgServiceId;
	Tu8 InternTableIdSPTy;
	Tu8 StatPTy;
	Tu8 InternTableIdDPTy;
	Tu8 DynPTy;

}Ts_Msg_Service_Info;

/**
* @brief Structure comprises the notification information of ProgrammeServListChanged_not
*/
typedef struct
{
	Tu8 NrOfServices;
	Ts_Msg_Service_Info		Service[MAX_ENSEMBLE_SERVICES];

}Ts_DabTunerMsg_ProgrammeServListChanged_not;

/**
* @brief Structure comprises the reply of SetServPropsChangedNotifier_repl
*/
typedef struct
{
	Tu8 ReplyStatus;

}Ts_DabTunerMsg_SetServPropsChangedNotifier_repl;

/**
* @brief Structure comprises the notification information of ServPropsChanged_not
*/
typedef struct
{
	Tu32 ServiceId;
	Tu8  InternTableIdDPTy;
	Tu8  DynPTy;
	Tu8  CharSet;
	Tu8  LabelString[DAB_TUNER_CTRL_MAX_LABEL_LENGTH];
	Tu16 ShortLabelFlags;
	Tu8  EncodingFlag;
	Tu16 ExtendedLabelFlags;
	Tu8  NrOfLabelStringBytes;
	Tu8	 ExtendedLabelByte[DAB_TUNER_CTRL_MAX_EXTENDED_LABEL_LENGTH];

}Ts_DabTunerMsg_ServPropsChanged_not;

/**
* @brief Structure to update receiver reply from SoC for Announcement Switching command.
*/
typedef struct
{
	Tu8			 reply;
	Tu8          padding[2];
}Ts_DabTunerMsg_R_AnnouncementSwitching;

typedef struct
{
	Tu8		ReplyStatus;
	Tu8		AudioSynchronised;
	Tu8		DecodingStatus;
	Ts32	RFFieldStrength;
	Ts32	BBFieldStrength;
	Tu32	FicBitErrorRatio;
	Tbool	isValidMscBitErrorRatio;
	Ts32	MscBitErrorRatio;
}Ts_GetAudioStatus_repl;

typedef struct
{
	Tu8	ReplyStatus;
	Tu8	TimeSyncLevel;
	Tu8	FrequencySyncLevel;
	Tu8 NullSymbolFound;
	Tu8 ChannelSync;

}Ts_GetSynchronisationState_repl;

typedef struct
{
	Tu32	BitRate;
	Tu8		ReplyStatus;
	Tu8		SBRFlag;
	Tu8 	SampleRate;
	Tu8 	SurroundFlag;
	Tu8 	AudioMode;
	Tu8 	SourceEncoding;

}Ts_GetAudioProperties_repl;

typedef struct
{

	Tu8 ReplyStatus;
	Tu8 TrackingInstanceIDValidFlag;
	Tu8	TrackingInstanceID;
	Tu8	rfa;

}Ts_Set_DriftTracking_Reply;

typedef struct
{
	Ts32 Delay;
	Tu16	LevelData;
	Tu8 Source1;
	Tu8 Source2;
	Tu8 TrackingInstanceID;
	Tu8 CorrelationQuality;
	Tu8	DelayFound;
	Tu8	ConfidenceLevel;
	Tu8	LevelStatus;
	Tu8	CompensateFMDriftDone;

}Ts_Drift_Tracking_Notification;

typedef struct
{
 
 Tu8				u8_ReplyStatus;
 Tu16   			u16_AudioFadeTimeIn;
 Tu8 				u8_AudioFadeTimeOut;
 Tbool  			b_HighResolutionNoiseLevel ;
 Tu8				u8_ComfortNoiseLevel ;
 Tbool	 			b_ComfortNoiseOnOffFlag;
 Tbool				b_AdvancedConcealmentParamsFlag;
 Tbool				b_MPEG_DABFlag;
 Tbool				b_AAC_DABPFlag ;
 Tbool				b_AAC_TDMBFlag;
 Tbool				b_MPEGHighCutConcealmentEnable;
 Tbool				b_SubbandErrorConcealmentEnable;
 Tbool				b_MPEGSoftMuteActivate;
 Tu8				u8_MPEGErrorConcealmentSetting;
 Tu8				u8_EcNumFrameRepeat;
 Tu8				u8_EcNumHeaderRepeat;
 Tu8				u8_EcNumScFRepeat;
 Tu8				u8_Theta0;
 Tu8				u8_Theta1;
 Tu8 				u8_BirdieSuppressionMode;
 Tbool				b_AAC_DABPHighCutConcealmentEnable;
 Tbool				b_AAC_DABPSoftMuteActivate;
 Tu8				u8_AAC_DABPErrorConcealmentSetting;
 Tbool				b_AAC_TDMBHighCutConcealmentEnable;
 Tbool				b_AAC_TDMBSoftMuteActivate;
 Tu8				u8_AAC_TDMBErrorConcealmentSetting;
 Tbool				b_ActivateForcedMuteMechanism;
 Tbool				b_AttackReleaseSettings;
 Tu16				u16_ErrorConcealmentAttackSetting;
 Tu16				u16_ErrorConcealmentReleaseSetting;
 Tu8				u8_QualityThresholdMute_Prep;
 Tu8				u8_QualityThresholdDeMute_Prep;
 Tu8				u8_MutePreparationTime; 
 //Tu64				u64_rfa;  /*check*/
 //Tu8				u8_rfa;

}Ts_DabTunerMsg_GetAudioErrorConcealment2SettingsReply;

typedef struct 
{
   Tu8         Reply;
   Tu8         padding[2];

}Ts_DabTunerMsg_R_AudioErrorConcealment2_repl;

typedef struct 
{
	Ts32	s32_CurrentTimeStretched ;
	Tu8 	u8_rfa ;	
	Tu8		u8_TimeStretchStatus ;
	
}Ts_StartTimeStretch_not;

typedef struct 
{
	Tu8		ReplyStatus ;
	
}Ts_StartTimeStretch_repl;

typedef struct
{
	Tu32 buffer_size;
	Tu8 buf[100];
}Ts_DabData;

typedef struct
{
	Te_RADIO_EtalDataServiceType e_Header;
	Tu8					u8_Payload[MAX_DATA_SERVICE_PAYLOAD_SIZE];
	Tu32				u32_PayloadSize;
}Ts_DAB_DataServiceRaw;
/*--------------------------------------------------------------------------------------------------
Function declarations
--------------------------------------------------------------------------------------------------*/
/**************************************************************************************************/
/**	 \brief                 API for sending tune command to SoC.
*   \param[in]				u8_Trigger_Options
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune select service request from upper layer.
*   \details                DAB Tuner Control forms SPI message for tune command and sends to SoC
							via this API.
*   \post-condition			Tune to command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_MsgSndSetTuneTocmd(Tu8 u8_Trigger_Options);

/**************************************************************************************************/
/**	 \brief                 API for sending create receiver command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives start up request from upper layer.
*   \details                When DAB Tuner Control receives start up request from upper layer, it
                            forms the SPI message for create receiver command and sends to SoC
							via this API.
*   \post-condition			Create receiver command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_MsgSndCreateReciver_Cmd(void);

/**************************************************************************************************/
/**	 \brief                 API for sending scan command to SoC.
*   \param[in]				sScanInput
*   \param[out]				None
*   \pre-condition			DAB tuner control receives scan request from upper layer.
*   \details                When DAB Tuner Control receives scan request from upper layer, it
							forms the SPI message for scan command and sends to SoC
							via this API.
*   \post-condition			Scan command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
ETAL_STATUS DabTunerCtrl_StartScan2_cmd(Tbool b_Scanstarted, Te_DAB_Tuner_Ctrl_RequestCmd e_RequestCmd);

/**************************************************************************************************/
/**	 \brief                 API for sending select service command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives select service request from upper layer.
*   \details                DAB Tuner Control forms SPI message for select service command and
                            sends to SoC via this API.  
*   \post-condition			Select service command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
ETAL_STATUS DabTuner_MsgSndSelectService_Cmd(Tu32 u32_Sid);

/**************************************************************************************************/
/**	 \brief                 API for sending select component command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives select service request from upper layer.
*   \details                DAB Tuner Control forms SPI message for select component command and
							sends to SoC via this API.
*   \post-condition			Select component command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
ETAL_STATUS DabTuner_MsgCmd_SelectComponent(Tu16 u16InternalCompId, Tu32 u32Sid);

/**************************************************************************************************/
/**	 \brief                 API for sending destroy receiver command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives shut down request from upper layer.
*   \details                DAB Tuner Control forms SPI message for destroy receiver command and
							sends to SoC via this API.
*   \post-condition			Destroy receiver command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_MsgCmdDestroyReciver(void);

/**************************************************************************************************/
/**	 \brief                 API for sending get component list command for a particular service to SoC.
*   \param[in]				Sid
*   \param[out]				None
*   \pre-condition			DAB tuner control receives scan request from upper layer.
*   \details                DAB Tuner Control forms SPI message for component list command and
							sends to SoC via this API.
*   \post-condition			Get component list command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
ETAL_STATUS DabTuner_MsgSndScan2GetComponentListReq_Cmd(Tu32 Sid);

/**************************************************************************************************/
/**	 \brief                 API for receiving create receiver reply from SoC.
*   \param[in]				msgdata
*   \param[out]				DAB_Tuner_Ctrl_CreateRcvrReply
*   \pre-condition			Create receiver command is sent to SoC by DAB Tuner Control.
*   \details                DAB Tuner Control extracts create receiver reply message received from
							SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with reply message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
							this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvCreateReceiver(Ts_DabTunerMsg_R_CreateReceiver * DAB_Tuner_Ctrl_CreateRcvrReply, char *msgdata);

/**************************************************************************************************/
/**	 \brief                 API for receiving scan reply from SoC.
*   \param[in]				msgdata
*   \param[out]				DAB_Tuner_Ctrl_ScanReply
*   \pre-condition			Scan command is sent to SoC by DAB Tuner Control.
*   \details                DAB Tuner Control extracts scan reply message received from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with reply message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvStartScan(Ts_DabTunerMsg_R_StartScan_Cmd * DAB_Tuner_Ctrl_ScanReply, char *msgdata);

/**************************************************************************************************/
/**	 \brief                 API for receiving scan status notification from SoC.
*   \param[in]				msg
*   \param[out]				DAB_Tuner_Ctrl_ScanNotification
*   \pre-condition			Scan reply is received by DAB Tuner Control.
*   \details                DAB Tuner Control extracts scan notification message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvScanStatusNot(Ts_DabTunerMsg_R_ScanStatus_Not * DAB_Tuner_Ctrl_ScanNotification);


/**************************************************************************************************/
/**	 \brief                 API for receiving get component list request reply from SoC.
*   \param[in]				msgdata
*   \param[out]				DAB_Tuner_Ctrl_GetComponent_Reply
*   \pre-condition			Scan reply is message received from SoC.
*   \details                DAB Tuner Control extracts scan notification message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvScan2GetComponentListReply(Ts_DabTunerMsg_GetComponentList_Reply * DAB_Tuner_Ctrl_GetComponent_Reply);


/**************************************************************************************************/
/**	 \brief                 API for receiving set tune status notification message from SoC.
*   \param[in]				msg
*   \param[out]				msgdata
*   \pre-condition			Tune status notification message is received from SoC.
*   \details                DAB Tuner Control extracts tune notification message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with tune notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvSetTuneStatusNot(Ts_DabTunerMsg_R_SetTuneStatusNot *msgdata, char *msg);

/**************************************************************************************************/
/**	 \brief                 API for receiving set tune reply message from SoC.
*   \param[in]				msg
*   \param[out]				msgdata
*   \pre-condition			Tune reply message is received from SoC.
*   \details                DAB Tuner Control extracts reply message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with tune reply message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvTuneTo(Ts_DabTunerMsg_R_TuneTo *msgdata, char *msg);

/**************************************************************************************************/
/**	 \brief                 API for receiving set tune reply message from SoC.
*   \param[in]				msg
*   \param[out]				msgdata
*   \pre-condition			Tune reply message is received from SoC.
*   \details                DAB Tuner Control extracts reply message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with tune reply message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/

/**************************************************************************************************/
/**	 \brief                 API for receiving select component reply message from SoC.
*   \param[in]				msg
*   \param[out]				msgdata
*   \pre-condition			Select component reply message is received from SoC.
*   \details                DAB Tuner Control extracts select component reply message received from
							SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with tune reply message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
void DabTuner_MsgReply_SelectComponent(Ts_Select_ComponentReply * msgdata, char *msg);

/**************************************************************************************************/
/**	 \brief                 API for receiving up notification message from SoC.
*   \param[in]				msg
*   \param[out]				msgdata
*   \pre-condition			Up notification message is received from SoC.
*   \details                DAB Tuner Control extracts up notification message received from
							SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with up notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvUpNot(Ts_DabTunerMsg_N_UpNot * DAB_TunerRcvUpNotification, char * msg);

Tbool DabTuner_SystemError_not(Ts_SystemError_not *pst_SystemError_not, Ts8 *pu8_Msg);

/**************************************************************************************************/
/**	 \brief                 API for receiving select service reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				selectServiceReply
*   \pre-condition			Select service reply message is received from SoC.
*   \details                DAB Tuner Control extracts select service reply message received from
							SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with select service reply message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvSelectServiceReply(Ts_DabTunerMsg_SelectServiceReply * selectServiceReply, char *msgdata);

/**************************************************************************************************/
/**	 \brief                 API for receiving destroy receiver reply message from SoC.
*   \param[in]				msg
*   \param[out]				msgdata
*   \pre-condition			Destroy receiver reply message is received from SoC.
*   \details                DAB Tuner Control extracts destroy receiver reply message received from
							SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with destroy receiver reply message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
void DabTuner_MsgReplyDestroyReceiver(Ts_DabTunerMsg_R_DestroyReceiver_Cmd *msgdata, char *msg);


/**************************************************************************************************/
/**	 \brief                 API for receiving set tune status notification reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				DAB_Tuner_Ctrl_CreateRcvrReply
*   \pre-condition			Set tune status notification reply message is received from SoC.
*   \details                DAB Tuner Control extracts set tune status notification reply message
							received from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with set tune status notification
							reply message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvSetTuneStatus(Ts_DabTunerMsg_R_SetTuneStatus * DAB_Tuner_Ctrl_CreateRcvrReply, char *msgdata);

/**************************************************************************************************/
/**	 \brief                 API for sending get version request command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives start up request from upper layer.
*   \details                When DAB Tuner Control receives start up request from upper layer, it
							forms the SPI message for et version request command and sends to SoC
							via this API.
*   \post-condition			Get version request command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_MsgSndVersionRequest_Cmd(void);

/**************************************************************************************************/
/**	 \brief                 API for receiving get version request reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				DAB_Tuner_Ctrl_GetVersionReply
*   \pre-condition			Get version request reply message is received from SoC.
*   \details                DAB Tuner Control extracts get version request reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with get version request reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvVersionRequest(Ts_DabTunerMsg_R_GetVersion * DAB_Tuner_Ctrl_GetVersionReply, char *msgdata);

/**************************************************************************************************/
/**	 \brief                 API for calculating checksum for SPI messages.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			SPI command message is formed.
*   \details                DAB Tuner Control calculates checksum for any SPI command message via
                            this API.
*   \post-condition			Checksum for SoC SPI command message is calculated.
*   \ErrorHandling    		N/A.
*
***************************************************************************************************/
void DAB_CheckSum(void);

/**************************************************************************************************/
/**	 \brief                 API for validating SPI messages from SoC.
*   \param[in]				readata
*   \param[out]				None
*   \pre-condition			SPI message from SoC is read.
*   \details                DAB Tuner Control validates the checksum value received from SoC via
                            this API.
*   \post-condition			SPI message received from SoC is validated.
*   \ErrorHandling    		N/A.
*
***************************************************************************************************/
Tbool DAB_Tuner_Ctrl_RxCmdAnalysis(Tu8 *buffer);
/**************************************************************************************************/
/**	 \brief                 API for sending set audio status notifier command to SoC.
*   \param[in]				Trigger
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for audio status notifier command and sends to SoC
							via this API.
*   \post-condition			Set audio status notifier command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_MsgSndSetAudioStatusNotifier_cmd(Tu8 Trigger);
/**************************************************************************************************/
/**	 \brief                 API for receiving set audio status notifier reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_SetAudioStatusReply
*   \pre-condition			Set audio status notifier reply message is received from SoC.
*   \details                DAB Tuner Control extracts set audio status notifier reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with audio status notifier reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvSetAudioStatusNotifier_reply(Ts_DabTunerMsg_R_SetAudioStatusReply * SetAudioStatus, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for receiving set audio status notification message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_AudioStatus_notify
*   \pre-condition			Set audio status notification message is received from SoC.
*   \details                DAB Tuner Control extracts notification message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with audio status notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
**************************************************************************************************/
Tbool DabTuner_MsgRcvSetAudioStatusNotifier(Ts_DabTunerMsg_R_AudioStatus_notify * AudioStatus_notify, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for sending set periodical BER quality notifier command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for audio status notifier command and sends to SoC
							via this API.
*   \post-condition			Set periodical BER quality notifier command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd(Tu8 Trigger);
/**************************************************************************************************/
/**	 \brief                 API for receiving set periodical BER quality notifier reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_PeriodicalBERQual_reply
*   \pre-condition			Set periodical BER quality notifier reply message is received from SoC.
*   \details                DAB Tuner Control extracts set periodical BER quality notifier reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with periodical BER quality notifier reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvSetPeriodicalBERQualityNotifier_reply(Ts_DabTunerMsg_R_PeriodicalBERQual_reply *PeriodicalBERQual_reply, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for receiving set periodical BER quality notification message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_PeriodicalBERQual_Notify
*   \pre-condition			Set periodical BER quality notification message is received from SoC.
*   \details                DAB Tuner Control extracts notification message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with periodical BER quality notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
**************************************************************************************************/
Tbool DabTuner_MsgRcvSetPeriodicalBERQualityNotifier(Ts_DabTunerMsg_R_PeriodicalBERQual_Notify *BERQual_Notify, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for sending set RSSI notifier settings command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for RSSI notifier settings command and sends to SoC
							via this API.
*   \post-condition			set RSSI notifier settings notifier command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_MsgSndSetRSSINotifierSettings_cmd(Tu8 Trigger);
/**************************************************************************************************/
/**	 \brief                 API for receiving set RSSI notifier settings reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_RSSINotifierSettings_reply
*   \pre-condition			Set RSSI notifier settings reply message is received from SoC.
*   \details                DAB Tuner Control extracts set RSSI notifier settings reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with RSSI notifier settings reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvSetRSSINotifierSettings_reply(Ts_DabTunerMsg_R_RSSINotifierSettings_reply *RSSINotifier, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for receiving set RSSI notifier settings notification message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_RSSI_notify
*   \pre-condition			Set RSSI notifier settings notification message is received from SoC.
*   \details                DAB Tuner Control extracts notification message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with RSSI notifier settings notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
**************************************************************************************************/
Tbool DabTuner_MsgRcvRSSI_notify(Ts_DabTunerMsg_R_RSSI_notify *RSSINotifier, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for sending set SNR notifier settings command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for SNR notifier settings command and sends to SoC
							via this API.
*   \post-condition			set SNR notifier settings notifier command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_MsgSndSetSNRNotifierSettings_cmd(Tu8 Trigger);
/**************************************************************************************************/
/**	 \brief                 API for receiving set SNR notifier settings reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_SetSNRNotifier_reply
*   \pre-condition			Set SNR notifier settings reply message is received from SoC.
*   \details                DAB Tuner Control extracts set SNR notifier settings reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with SNR notifier settings reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgRcvSetSNRNotifierSettings_reply(Ts_DabTunerMsg_R_SetSNRNotifier_reply *SNRNotifier_reply, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for receiving set SNR notifier settings notification message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_SNRNotifier
*   \pre-condition			Set SNR notifier settings notification message is received from SoC.
*   \details                DAB Tuner Control extracts notification message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with SNR notifier settings notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
**************************************************************************************************/
Tbool DabTuner_MsgRcvSNRNotifier(Ts_DabTunerMsg_R_SNRNotifier *SNRNotifier, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for sending get current ensemble programme service list command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for get current ensemble programme service list command and sends to SoC
							via this API.
*   \post-condition			get current ensemble programme service list command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
ETAL_STATUS DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd(void);
/**************************************************************************************************/
/**	 \brief                 API for sending set synchronisation notifier command to SoC.
*   \param[in]				Trigger
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for set synchronisation notifier command and sends to SoC
							via this API.
*   \post-condition			Set synchronisation notifier command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_SetSynchronisationNotifier_cmd(Tu8 Trigger);
/**************************************************************************************************/
/**	 \brief                 API for receiving set synchronisation notifier reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_SynchReply
*   \pre-condition			Set synchronisation notifier reply message is received from SoC.
*   \details                DAB Tuner Control extracts set synchronisation notifier reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with set synchronisation notifier reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
void DabTuner_SetSynchronisationNotifier_reply(Ts_DabTunerMsg_R_SynchReply *SynchReply, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for receiving set synchronisation notification message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_R_SynchNotification
*   \pre-condition			Set synchronisation notification message is received from SoC.
*   \details                DAB Tuner Control extracts notification message received from SoC via
                            this API.
*   \post-condition			DAB Tuner Control structure is updated with set synchronisation notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
**************************************************************************************************/
void DabTuner_MsgRcvSetSynchronisationNotification(Ts_DabTunerMsg_R_SynchNotification *SynchNotify, char *msgdata);
/**************************************************************************************************/
/**	 \brief                 API for sending get audio status2 command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for get audio status2 command and sends to SoC
							via this API.
*   \post-condition			Get audio status2 command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_Get_Audio_Status2_Req(void);
/**************************************************************************************************/
/**	 \brief                 API for receiving get audio status2 reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_GetAudioStatus2_reply
*   \pre-condition			Get audio status2 reply message is received from SoC.
*   \details                DAB Tuner Control extracts get audio status2 reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with get audio status2 reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
void DabTuner_Get_Audio_Status2_Res(Ts_DabTunerMsg_GetAudioStatus2_reply *pst_GetAudioStatus2_reply, char* msgdata);
/**************************************************************************************************/
/**	 \brief                 API for sending get ensemble properties command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
forms the SPI message for get ensemble properties command and sends to SoC
via this API.
*   \post-condition			Get ensemble properties command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
ETAL_STATUS DabTuner_MsgSndGetEnsembleProperties(void);
/**************************************************************************************************/
/**	 \brief                 API for receiving get ensemble properties reply message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_DabTunerGetEnsembleProperties_reply
*   \pre-condition			Get ensemble properties reply message is received from SoC.
*   \details                DAB Tuner Control extracts get ensemble properties reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with get ensemble properties reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
void DabTuner_MsgReplyGetEnsembleProperties(Ts_DabTunerGetEnsembleProperties_reply * msgdata);
/**************************************************************************************************/
/**	 \brief                 API for sending register sink to service component command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for register sink to service component command and sends to SoC
							via this API.
*   \post-condition			Register sink to service component command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DAB_Tuner_Ctrl_RegisterSinkToServComp_cmd(void);
/**************************************************************************************************/
/**	 \brief                 API for receiving register sink to service reply message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_DabTunerMsg_RegisterSinkToServComp_reply
*   \pre-condition			Register sink to service component reply message is received from SoC.
*   \details                DAB Tuner Control extracts receiving register sink to service reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with receiving register sink to service reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
void DAB_Tuner_Ctrl_RegisterSinkToServComp_reply(Ts_DabTunerMsg_RegisterSinkToServComp_reply *msgdata, char *msg);
/**************************************************************************************************/
/**	 \brief                 API for receiving XPAD DLS data reply message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_dab_tuner_ctrl_DLS_data
*   \pre-condition			XPAD DLS data reply message is received from SoC.
*   \details                DAB Tuner Control extracts receiving XPAD DLS data reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with receiving XPAD DLS data  reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
void DabTunerMsg_XPAD_DLS_Data(Ts_dab_tuner_ctrl_DLS_data *msgdata, char *msg);
/**************************************************************************************************/
/**	 \brief                 API for receiving get search next reply message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_SearchNext_Reply
*   \pre-condition			search next reply message is received from SoC.
*   \details                DAB Tuner Control extracts search next reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with search next reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
                            this API is invoked.
*
***************************************************************************************************/
Tbool DabTuner_MsgReply_SearchNext(Ts_SearchNext_Reply * msgdata, char *msg);
/**************************************************************************************************/
/**	 \brief                 API for sending tune to command to SoC.
*   \param[in]				u32Freq
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for tune to command and sends to SoC
							via this API.
*   \post-condition			Tune to command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
ETAL_STATUS DabTuner_MsgSndTuneTo_Cmd(Tu32 u32Freq);
/**************************************************************************************************/
/**	 \brief                 API for sending abort scan command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives scan request from upper layer.
*   \details                When DAB Tuner Control receives abort scan up request from upper layer, it
							forms the SPI message for abort scan command and sends to SoC
							via this API.
*   \post-condition			Abort scan command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
ETAL_STATUS DabTuner_AbortScan2_Cmd(Te_DAB_Tuner_Ctrl_RequestCmd e_RequestCmd);
void DabTuner_AbortSearch_cmd(void);
/**************************************************************************************************/
/**	 \brief                 API for receiving get current ensemble programme service list reply message from SoC.
*   \param[in]				msgdata
*   \param[out]				Ts_DabTunerMsg_GetCurrEnsembleProgListReply
*   \pre-condition			Get current ensemble program service list reply message is received from SoC.
*   \details                DAB Tuner Control extracts get current ensemble program service list reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with get current ensemble program service list reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
							this API is invoked.
*
***************************************************************************************************/
void DabTuner_MsgRcvGetCurrEnsembleProgrammeServiceListReply(Ts_DabTunerMsg_GetCurrEnsembleProgListReply* st_GetCurrEnsembleProgListReply);
/**************************************************************************************************/
/**	 \brief                 API for sending search next command to SoC.
*   \param[in]				startfrequency
*   \param[in]				Te_RADIO_DirectionType
*   \param[out]				None
*   \pre-condition			DAB tuner control receives seek request from upper layer.
*   \details                When DAB Tuner Control receives seek request from upper layer, it
							forms the SPI message for seek command and sends to SoC
							via this API.
*   \post-condition			Search next command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_MsgSndSearchNext_cmd(Te_RADIO_DirectionType e_direction, Tu32 startfrequency);
/**************************************************************************************************/
/**	 \brief                 API for sending set FIG filter command to SoC.
*   \param[in]				filterid
*   \param[in]				muxid
*   \param[in]				u16_extension
*   \param[out]				None
*   \pre-condition			DAB tuner control receives data notification from upper layer.
*   \details                When DAB Tuner Control receives data notification from upper layer, it
							forms the SPI message for SetFIG_filter command and sends to SoC
							via this API.
*   \post-condition			SetFIG_filter command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_SetFIG_filter_command(Tu16 u16_extension, Tu8 muxid, Tu8 filterid);
/**************************************************************************************************/
/**	 \brief                 API for receiving get audio status notification message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_AudioStatus2_not
*   \pre-condition			Get audio status notification message is received from SoC.
*   \details                DAB Tuner Control extracts notification message received from SoC via
							this API.
*   \post-condition			DAB Tuner Control structure is updated with get audio status notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
							this API is invoked.
*
**************************************************************************************************/
Tbool DabTuner_Get_Audio_Status_Notification(Ts_AudioStatus2_not *pst_AudioStatus2_not, Ts8 *pu8_Msg);
/**************************************************************************************************/
/**	 \brief                 API for sending set audio status notifier command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives tune request from upper layer.
*   \details                When DAB Tuner Control receives tune request from upper layer, it
							forms the SPI message for set audio status notifier command and sends to SoC
							via this API.
*   \post-condition			Set audio status notifier command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_Set_Audio_Status_Notifier_Cmd(void);
/**************************************************************************************************/
/**	 \brief                 API for receiving set audio status notifier reply message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_Set_Audio_Status_Notifier_Reply
*   \pre-condition			Set audio status notifier reply message is received from SoC.
*   \details                DAB Tuner Control extracts set audio status notifier reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with set audio status notifier reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
							this API is invoked.
*
***************************************************************************************************/
void DabTuner_Set_Audio_Status_Notifier_Reply(Ts_Set_Audio_Status_Notifier_Reply *st_Set_Audio_Status_reply, char*  msg);
/**************************************************************************************************/
/**	 \brief                 API for sending reset blending command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives prepare for blending from upper layer.
*   \details                When DAB Tuner Control receives prepare for blending request from upper layer, it
							forms the SPI message for Reset Blending command and sends to SoC
							via this API.
*   \post-condition			Reset Blending command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_ResetBlending_cmd(void);
/**************************************************************************************************/
/**	 \brief                 API for sending prepare for blending command to SoC.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control receives prepare for blending from upper layer.
*   \details                When DAB Tuner Control receives prepare for blending request from upper layer, it
							forms the SPI message for prepare for Blending command and sends to SoC
							via this API.
*   \post-condition			PrepareForBlending command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_PrepareForBlending_cmd(void);
/**************************************************************************************************/
/**	 \brief                 API for receiving prepare for blending reply message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_PrepareForBlending_Reply
*   \pre-condition			Prepare for blending reply message is received from SoC.
*   \details                DAB Tuner Control extracts prepare for blending reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with prepare for blending reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
							this API is invoked.
*
***************************************************************************************************/
void DabTuner_PrepareForBlending_Reply(Ts_PrepareForBlending_Reply *msgdata, char *msg);
/**************************************************************************************************/
/**	 \brief                 API for receiving prepare for blending notification message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_PrepareForBlending_Notify
*   \pre-condition			PrepareForBlending notification message is received from SoC.
*   \details                DAB Tuner Control extracts notification message received from SoC via
							this API.
*   \post-condition			DAB Tuner Control structure is updated with PrepareForBlending notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
							this API is invoked.
*
**************************************************************************************************/
Tbool DabTuner_PrepareForBlending_Notify(Ts_PrepareForBlending_Notify *pst_PrepareForBlending_not, Ts8 *pu8_Msg);
/**************************************************************************************************/
/**	 \brief                 API for sending StartTimeAlignmentForBlending command to SoC.
*   \param[in]				Delay
*   \param[out]				None
*   \pre-condition			DAB tuner control receives StartTimeAlignmentForBlending from upper layer.
*   \details                When DAB Tuner Control receives StartTimeAlignmentForBlending request from upper layer, it
							forms the SPI message for StartTimeAlignmentForBlending_cmd command and sends to SoC
							via this API.
*   \post-condition			StartTimeAlignmentForBlending command is sent to SoC.
*   \ErrorHandling    		Checksum is calculated and appended with the SPI message and then sent to
							SoC.SoC would then validate this message through checksum value.
*
***************************************************************************************************/
void DabTuner_StartTimeAlignmentForBlending_cmd(Ts32 Delay);
/**************************************************************************************************/
/**	 \brief                 API for receiving StartTimeAlignmentForBlending reply message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_TimeAlignmentForBlending_Notify
*   \pre-condition			StartTimeAlignmentForBlending reply message is received from SoC.
*   \details                DAB Tuner Control extracts StartTimeAlignmentForBlending reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with StartTimeAlignmentForBlending reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
							this API is invoked.
*
***************************************************************************************************/
void DabTuner_StartTimeAlignmentForBlending_Reply(Ts_StartTimeAlignmentForBlending_repl *msgdata, char *msg);
/**************************************************************************************************/
/**	 \brief                 API for receiving TimeAlignmentForBlending_Notify notification message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_TimeAlignmentForBlending_Notify
*   \pre-condition			TimeAlignmentForBlending_Notify notification message is received from SoC.
*   \details                DAB Tuner Control extracts notification message received from SoC via
							this API.
*   \post-condition			DAB Tuner Control structure is updated with Ts_TimeAlignmentForBlending_Notify notification message from
							SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
							this API is invoked.
*
**************************************************************************************************/
Tbool DabTuner_TimeAlignmentForBlending_Notify(Ts_TimeAlignmentForBlending_Notify *pst_TimeAlignmentForBlending_not, Ts8 *pu8_Msg);
/**************************************************************************************************/
/**	 \brief                 API for receiving Start blending reply message from SoC.
*   \param[in]				msg
*   \param[out]				Ts_StartBlending_Reply
*   \pre-condition			StartBlending_Reply reply message is received from SoC.
*   \details                DAB Tuner Control extracts StartBlending_Reply reply message received
							from SoC via this API.
*   \post-condition			DAB Tuner Control structure is updated with Ts_StartBlending_Reply reply
							message from SoC.
*   \ErrorHandling    		Checksum is calculated before reading any data from SoC, only then
							this API is invoked.
*
***************************************************************************************************/
void DabTuner_StartBlending_Reply(Ts_StartBlending_Reply *msgdata, char *msg);
void DabTuner_StartBlending_Without_delay(Tu8 Target);

void DabTunerCtrl_SetServListChangedNotifier_cmd(Tu8 u8_status);
void DabTunerCtrl_SetServListChangedNotifier_repl(Ts_DabTunerMsg_SetServListChangedNotifier_repl *ServListChangedNotifierRply, char *msg);
void DabTunerCtrl_ProgrammeServListChanged_not(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *ProgrammeServListChangedNot, Tu8 *msg);
void DabTunerCtrl_SetServPropsChangedNotifier_cmd(void);
void DabTunerCtrl_SetServPropsChangedNotifier_repl(Ts_DabTunerMsg_SetServPropsChangedNotifier_repl *ServPropsChangedNotifier_repl, char *msg);
void DabTunerCtrl_ServPropsChanged_not(Ts_DabTunerMsg_ServPropsChanged_not *ServPropsChanged_not, char *msg);
void DabTuner_MsgSndAnnouncementSwitching_cmd(Tu8 subchannelid);
void DabTuner_MsgSndAnnouncementStop_cmd(void);
Tbool DabTuner_MsgRcvAnnouncementSwitching(Ts_DabTunerMsg_R_AnnouncementSwitching *DAB_Tuner_Ctrl_AnnouncementSwitchingReply, char *msgdata);
void DabTuner_StartBlending_for_implicit(Tu8 u8_BlendTarget);
void DabTuner_StartBlending_cmd(Tu8 u8_BlendTarget,Tu16	u16_LevelData);
ETAL_STATUS DabTunerCtrl_GetAudioStatus_req(void);
void DabTunerCtrl_GetAudioStatus_repl(Ts_GetAudioStatus_repl *pst_GetAudioStatus_repl);
Tbool DabTunerCtrl_GetSynchronisationState_repl(Ts_GetSynchronisationState_repl *pst_GetSynchronisationState_repl, Ts8 *pu8_Msg);
void DabTunerCtrl_SystemControl_cmd(Tu16 u16_ActionId, Tu16 u16_Parameter);
Tbool DabTunerCtrl_SystemControl_repl(Ts_SystemControl_repl *pst_SystemControl_repl, Ts8 *pu8_Msg);
Tbool DabTunerCtrl_SystemMonitoring_not(Ts_SystemMonitoring_not *pst_SystemMonitoring_not, Ts8 *pu8_Msg);
void DabTunerCtrl_GetAudioProperties_req(void);
Tbool DabTunerCtrl_GetAudioProperties_repl(Ts_GetAudioProperties_repl *pst_GetAudioProperties_repl, Ts8 *pu8_Msg);
Tbool DabTuner_MsgRcv_AbortScan2Reply(Ts_AbortScan2Reply * msgdata, char *msg);
void DabTuner_ResetBlending_Reply(Ts_ResetBlending_Reply *msgdata, char *msg);
void DabTuner_Drift_Tracking_Not(Ts_Drift_Tracking_Notification *st_Drift_Tracking_Notification, char *msg);
void DabTuner_Set_Drift_Tracking_Reply(Ts_Set_DriftTracking_Reply *st_DriftTacking_Reply, char *msg);
void DabTuner_MsgSndGetAudioErrorConcealment2Settings_Cmd(void);
Tbool DabTuner_MsgRcvGetAudioErrorConcealment2SettingsReply(Ts_DabTunerMsg_GetAudioErrorConcealment2SettingsReply* st_GetAudioErrorConcealment2SettingsReply,char *msgdata);
void DabTuner_SetAudioErrorConcealment2_cmd(void);
Tbool DabTuner_MsgReplySetAudioErrorConcealment2_repl(Ts_DabTunerMsg_R_AudioErrorConcealment2_repl *msgdata, char *msg);
void DabTuner_StartTimeStretch_cmd(Tu32 u32_TimeToStretch);
void DabTuner_CancelStartTimeStretch_cmd(Tu32 u32_TimeToStretch);
Tbool DabTuner_StartTimeStretch_repl(Ts_StartTimeStretch_repl *pst_StartTimeStretch_repl, Ts8 *pu8_Msg);
Tbool DabTuner_StartTimeStretch_not(Ts_StartTimeStretch_not *pst_StartTimeStretch_not, Ts8 *pu8_Msg);
void DabTuner_MsgSndSetRSSINotifier_cmd(Tu8 Trigger);
void DabTuner_MsgSndSetRSSINotifier(Tu8 Trigger);
void DabTunerCtrl_GetSynchronisationState_req(void);
void DabTuner_MsgSndSetPeriodicalBERQualityNotifier(Tu8 Trigger);
void DabTuner_MsgSndDeSelectService_Cmd(Tu32 u32_Sid);
void DabTuner_SetFIG0_21_filter_command(Tu16 u16_extension, Tu8 muxid, Tu8 filterid);
ETAL_STATUS DabTunerCtrl_Config_Receiver_cmd(Te_Frontend_type e_Frontend_type);
ETAL_STATUS DabTunerCtrl_Config_Datapath_cmd(Te_Frontend_type e_Frontend_type);
ETAL_STATUS DabTunerCtrl_Config_FICData_cmd(Te_Frontend_type e_Frontend_type);
ETAL_STATUS DabTunerCtrl_Audio_Source_Select_cmd(void);
ETAL_STATUS DabTunerCtrl_Destroy_Datapath_cmd(Te_Frontend_type e_Frontend_type);
ETAL_STATUS DabTunerCtrl_Destroy_FICData_cmd(Te_Frontend_type e_Frontend_type);
ETAL_STATUS DabTunerCtrl_Destroy_Receiver_cmd(Te_Frontend_type e_Frontend_type);
ETAL_STATUS DabTuner_Msg_GetCurrEnsemble_cmd(void);
Tbool DabTuner_GetServListpropertiesReply(Ts_CurrEnsemble_serviceinfo * ETAL_DAB_Tnr_Ctrl_GetServListinfo_Reply);
ETAL_STATUS DabTunerCtrl_Enable_Data_Service_cmd(void);
ETAL_STATUS DabTunerCtrl_Disable_Data_Service_cmd(void);
ETAL_STATUS DabTunerCtrl_Enable_FICData_cmd(void);
void DABDataService_cbFunc(Tu8* pBuffer, Tu32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
void DABFICData_cbFunc(Tu8* pBuffer, Tu32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
ETAL_STATUS DabTuner_AutoSeekStart_Cmd(Te_RADIO_DirectionType e_SeekDirection, Tbool b_UpdateStopFreq);
ETAL_STATUS DabTuner_AbortAutoSeek_Cmd(void);
ETAL_STATUS  DabTuner_Config_FGQualityMonitor(Te_Frontend_type e_Frontend_type);
ETAL_STATUS  DabTuner_Destroy_FGQualityMonitor(void);
void DAB_Quality_Monitor_Callback(EtalBcastQualityContainer* pQuality, void* vpContext);
#endif
/*==================================================================================================
end of file
==================================================================================================*/
