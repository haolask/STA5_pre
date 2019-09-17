//!
//!  \file 		etalinternal_cmost.h
//!  \brief 	<i><b>ETAL communication, private header</b></i>
//!  \details	ETAL internal and external communications, CMOST-specific definitions
//!

/*!
 * \def		ETAL_CHANGE_BAND_DELAY_CMOST
 * 			CMOST_CUT_2_0 requires a delay after the change band command,
 * 			it is specified here in milliseconds.
 * 			Later silicon versionis provide a busy bit in the SCSR0 to be polled:
 * 			in this case the time interval her e specified is the maximum delay.
 * 			The bit has to be polled only for some special commands: change band, tune, AFcheck.
 * \remark	Developers reccommend 100ms minimum, but even with 100ms we still get timeout
 * 			waiting for the busy bit to clear for some automatic seek commands.
 */
#define ETAL_CHANGE_BAND_DELAY_CMOST   100
#define ETAL_SEEK_START_DELAY_CMOST    100
#define ETAL_AF_CHECK_DELAY_CMOST      100 //15
#define ETAL_AF_START_DELAY_CMOST      100 //15


// RDS buffers constraints
#define ETAL_RDS_MAX_BLOCK_IN_BUFFER	16

/*!
 * \def		ETAL_CMOST_SCSR0_BUSY_CHA
 * 			The SCSR0 contains indicates if the CMOST device is busy
 * 			completing a command. This macro accesses the bit which
 * 			gives this indication for the CMOST A channel.
 */
#define ETAL_CMOST_SCSR0_BUSY_CHA(_buf)                 ((_buf) & 0x000001)
/*!
 * \def		ETAL_CMOST_SCSR0_BUSY_CHB
 * 			The SCSR0 contains indicates if the CMOST device is busy
 * 			completing a command. This macro accesses the bit which
 * 			gives this indication for the CMOST B channel.
 */
#define ETAL_CMOST_SCSR0_BUSY_CHB(_buf)                 (((_buf) & 0x001000) >> 12)
/*!
 * \def		ETAL_CMOST_SCSR0_TUNED_STATUS_CHA
 * 			The SCSR0 contains indicates if the CMOST device is tuned
 * 			completing a command. This macro accesses the bit which
 * 			gives this indication for the CMOST A channel.
 */
#define ETAL_CMOST_SCSR0_TUNED_STATUS_CHA(_buf)         ((_buf) & 0x000002)
/*!
 * \def		ETAL_CMOST_SCSR0_TUNED_STATUS_CHB
 * 			The SCSR0 contains indicates if the CMOST device is tuned
 * 			completing a command. This macro accesses the bit which
 * 			gives this indication for the CMOST B channel.
 */
#define ETAL_CMOST_SCSR0_TUNED_STATUS_CHB(_buf)         (((_buf) & 0x002000) >> 13)
/*!
 * \def		ETAL_CMOST_SCSR0_TUNER_BAND_CHA
 * 			The SCSR0 contains indicates the CMOST device selected band
 * 			completing a command. This macro accesses the bit which
 * 			gives this indication for the CMOST A channel.
 */
#define ETAL_CMOST_SCSR0_TUNER_BAND_CHA(_buf)           (((_buf) & 0x000F00) >> 8)
/*!
 * \def		ETAL_CMOST_SCSR0_TUNER_BAND_CHB
 * 			The SCSR0 contains indicates the CMOST device selected band
 * 			completing a command. This macro accesses the bit which
 * 			gives this indication for the CMOST B channel.
 */
#define ETAL_CMOST_SCSR0_TUNER_BAND_CHB(_buf)           (((_buf) & 0xF00000) >> 20)

/*
 * TUNER_Get_Reception_Quality decoding
 */
#define ETAL_CMOST_GET_AMFMMON_RESP_LEN                                 12
#define ETAL_CMOST_GET_AMFMMON_VPA_ON_RESP_LEN                         (ETAL_CMOST_GET_AMFMMON_RESP_LEN + 9)
#define ETAL_CMOST_STAR_DETUNING_FM_MAX                                 50000
#define ETAL_CMOST_STAR_DETUNING_AM_MAX                                 5000
#define ETAL_CMOST_STAR_DEVIATION_FM_MAX                                200000

#define ETAL_CMOST_GET_AMFMMON_FM_RF_FIELD_STRENGTH(_d_)                ((((tU32)(_d_) & 0x80) != 0) ? ((tS32)(_d_) - 256) : (tS32)(_d_))
#define ETAL_CMOST_GET_AMFMMON_AM_RF_FIELD_STRENGTH(_d_)                ((((tU32)(_d_) & 0x80) != 0) ? ((tS32)(_d_) - 256) : (tS32)(_d_))
#define ETAL_CMOST_GET_AMFMMON_DAB_RF_FIELD_STRENGTH(_d_)               ((((tU32)(_d_) & 0x80) != 0) ? ((tS32)(_d_) - 256) : (tS32)(_d_))
#define ETAL_CMOST_GET_AMFMMON_FM_BB_FIELD_STRENGTH(_d_)                ((((tU32)(_d_) & 0x80) != 0) ? ((tS32)(_d_) - 256) : (tS32)(_d_))
#define ETAL_CMOST_GET_AMFMMON_AM_BB_FIELD_STRENGTH(_d_)                ((((tU32)(_d_) & 0x80) != 0) ? ((tS32)(_d_) - 256) : (tS32)(_d_))
#define ETAL_CMOST_GET_AMFMMON_DAB_BB_FIELD_STRENGTH(_d_)               ((((tU32)(_d_) & 0x80) != 0) ? ((tS32)(_d_) - 256) : (tS32)(_d_))
#define ETAL_CMOST_GET_AMFMMON_FM_DETUNING(_d_)                         (((tU32)(_d_) * ETAL_CMOST_STAR_DETUNING_FM_MAX) / 0xFF)
#define ETAL_CMOST_GET_AMFMMON_AM_DETUNING(_d_)                         (((tU32)(_d_) * ETAL_CMOST_STAR_DETUNING_AM_MAX) / 0xFF)
#define ETAL_CMOST_GET_AMFMMON_FM_MULTIPATH(_d_)                        (((tU32)(_d_) * 100) / 0xFF)
#define ETAL_CMOST_GET_AMFMMON_FM_MPX_NOISE(_d_)                        (((tU32)(_d_) * 100) / 0xFF)
#define ETAL_CMOST_GET_AMFMMON_FM_ADJACENT_CHANNEL(_d_)                 ((((tU32)(_d_) & 0x80) != 0) ? ((tS32)(_d_) - 256) : (tS32)(_d_))

#define ETAL_CMOST_GET_AMFMMON_AM_ADJACENT_CHANNEL(_d_)                 ((((tU32)(_d_) & 0x80) != 0) ? ((tS32)(_d_) - 256) : (tS32)(_d_))

#define ETAL_CMOST_GET_AMFMMON_FM_DEVIATION(_d_)                        (((((tU32)(_d_) >> 1) & 0x7F) * ETAL_CMOST_STAR_DEVIATION_FM_MAX) / 0x7F)
#define ETAL_CMOST_GET_AMFMMON_AM_MODULATION(_d_)                       (((((tU32)(_d_) >> 1) & 0x7F) * 100) / 0x7F)
#define ETAL_CMOST_GET_AMFMMON_FM_SNR(_d_)                              ((tU32)(_d_))
#define ETAL_CMOST_GET_AMFMMON_FM_COCHANNEL(_d_)                        (((tU32)(_d_) * 100) / 0xFF)
#define ETAL_CMOST_GET_AMFMMON_FM_STEREO_MONO_RECEPTION(_d_)            ((tU32)(_d_) & 0x01)

/* TUNER_Get_AF_Quality decoding */
#define ETAL_CMOST_GET_AFQUALITY_RESP_LEN                               12

/*!
 * \def		ETAL_CMOST_BUSYBIT_WAIT_LOOP
 * 			Number of times ETAL loops polling the CMOST busy bit
 * \see		ETAL_CMOST_BUSYBIT_WAIT_SCHEDULING
 */
#define ETAL_CMOST_BUSYBIT_WAIT_LOOP           5

/*!
 * \def		ETAL_CMOST_BUSYBIT_WAIT_SCHEDULING
 * 			While waiting for the Busy bit to clear ETAL polls the SCSR0
 * 			and if the bit is not clear it sleeps for the amount of ms specified
 * 			in this macro.
 *
 * 			If a value too small is used (e.g. 1ms) if traces are enabled the
 * 			burst of messages generated during the busy bit loop overflows the
 * 			trace buffer and messages are lost. To avoid this we calculate the
 * 			wait time from the #ETAL_CHANGE_BAND_DELAY_CMOST so that we loop at
 * 			least #ETAL_CMOST_BUSYBIT_WAIT_LOOP times waiting for the busy bit.
 *
 * 			This is a temporary solution, once it is clear which #ETAL_CHANGE_BAND_DELAY_CMOST
 * 			should be used #ETAL_CMOST_BUSYBIT_WAIT_SCHEDULING can be 
 * 			adjusted accordingly.
 */
#if (ETAL_CHANGE_BAND_DELAY_CMOST < ETAL_CMOST_BUSYBIT_WAIT_LOOP)
	#define ETAL_CMOST_BUSYBIT_WAIT_SCHEDULING 1
#else
	#define ETAL_CMOST_BUSYBIT_WAIT_SCHEDULING (ETAL_CHANGE_BAND_DELAY_CMOST / ETAL_CMOST_BUSYBIT_WAIT_LOOP)
#endif

typedef struct
{
	ETAL_HANDLE           hMonitor;
	EtalBcastQualityContainer qual;
} etalQualityCbTy;

/*!
 * \enum	EtalBBIntfModeTy
 * 			CMOST Baseband Interface Mode, parameter to the
 * 			function #ETAL_directCmdSetBBIf_CMOST.
 * 			It corresponds to bits 3..0 of parameter1
 * 			of the Tuner_Set_BB_IF CMOST command
 */
typedef enum
{
	BBMode0 = 0, /*! Disable digital baseband interface */
	BBMode1 = 1, /*! Enable SAI BB CMOS Level */
	BBMode2 = 2, /*! Enable SDM (Interface to STA660) */
	BBMode3 = 3, /*! Enable JDEC interface (JESD204) */
	BBMode4 = 4, /*! Enable SAI BB LVDS */
	BBMode5 = 5, /*! Enable SAI BB SPLIT Mode (CMOS Level) */
} EtalBBIntfModeTy;

/*!
 * \enum	EtalAudioIntfModeTy
 * 			CMOST Audio Interface Mode, parameter to the
 * 			function #ETAL_directCmdSetBBIf_CMOST.
 * 			It corresponds to bits 19..16 of parameter1
 * 			of the Tuner_Set_BB_IF CMOST command
 */
typedef enum
{
	AIMode0 = 0, /*! No Audio I/F GPIO mapping */
	AIMode1 = 1, /*! - LegacySTA680 mode (no AAA) with BB MUX IQ
					 - STA660 DAB /DRM
					 - Connection to host (e.g. Accordo X) */
	AIMode2 = 2, /*! LegacySTA680 mode (no AAA) with BB MUX IQ
				     and system clock to STA680 from CMOS Tuner */
	AIMode3 = 3, /*! STA680 mode with AAA and BB MUX IQ.
				     System clock to STA680 from CMOS Tuner */
	AIMode4 = 4, /*! Audio Mode for BB I/F Mode 5 */
	AIMode5 = 5, /*! MSR1 test mode */
	AIMode6 = 6  /*! STA680 mode with AAA and BB MUX 
					 IQ, but without system clock */
} EtalAudioIntfModeTy;

/*!
 * \struct	EtalFMMode
 * 			Parameter for the #ETAL_cmdSetFMProc_CMOST API
 */
 typedef enum
{
	ETAL_FM_MODE_SINGLE_TUNER = 0,
	ETAL_FM_MODE_VPA = 1,
} EtalFMMode;

/*!
 * \enum	etalCMOSTTunerMode
 * 			CMOST Tuner Mode, used by the
 * 			function ETAL_paramSetBand_CMOST and 
 * 			used to decode Tuner Mode of SCSR0 register 
 * 			from function ETAL_readStatusRegister_CMOST
 */
typedef enum
{
	ETAL_CMOST_TUNER_MODE_STANDBY = 0x00,
	ETAL_CMOST_TUNER_MODE_FM      = 0x01,
	ETAL_CMOST_TUNER_MODE_WB      = 0x02,
	ETAL_CMOST_TUNER_MODE_DAB3    = 0x03,
	ETAL_CMOST_TUNER_MODE_DABL    = 0x04,
	ETAL_CMOST_TUNER_MODE_AMEU    = 0x05,
	ETAL_CMOST_TUNER_MODE_AMUS    = 0x06,
	ETAL_CMOST_TUNER_MODE_DRM30   = 0x07,
	ETAL_CMOST_TUNER_MODE_DRMP    = 0x08
} etalCMOSTTunerMode;

#ifdef CONFIG_ETAL_SUPPORT_CMOST
tSInt 		ETAL_initCommunication_CMOST(tVoid);
tSInt 		ETAL_initCommunication_SingleCMOST(tU32 deviceID, tBool vI_manageReset);
tSInt       ETAL_deinitCommunication_CMOST(tVoid);
tSInt       ETAL_load_CMOST(ETAL_HANDLE hTuner);
tSInt       ETAL_cmdPing_CMOST(ETAL_HANDLE hReceiver);
tSInt       ETAL_cmdTune_CMOST(ETAL_HANDLE hReceiver, tU32 dwFrequency);
tSInt       ETAL_cmdTuneXTAL_CMOST(ETAL_HANDLE hReceiver);       // only for CONFIG_ETAL_HAVE_XTAL_ALIGNMENT
tSInt       ETAL_cmdDebugSetBBProc_CMOST(ETAL_HANDLE hReceiver); // only for CONFIG_ETAL_HAVE_XTAL_ALIGNMENT
tSInt       ETAL_cmdSelectAudioSource_CMOST(ETAL_HANDLE hTuner, etalStarBlendingModeEnumTy src);
tSInt       ETAL_cmdSelectAudioInterface_CMOST(ETAL_HANDLE hTuner, etalAudioIntfStatusTy intf);
tSInt       ETAL_cmdChangeBand_CMOST(ETAL_HANDLE hGeneric, EtalFrequencyBand band, tU32 bandMin, tU32 bandMax, tU32 step, EtalProcessingFeatures processingFeatures);
tSInt       ETAL_cmdReceiverConfig_CMOST(etalReceiverStatusTy *pstat);
tSInt       ETAL_readStatusRegister_CMOST(ETAL_HANDLE hTuner, tU32 *scsr, tU32 *freq0, tU32 *freq1);
tSInt       ETAL_waitBusyBit_CMOST(ETAL_HANDLE hTuner, tU32 tuner_mask, tU32 max_delay_msec);
tSInt       ETAL_cmdRead_CMOST(ETAL_HANDLE hReceiver, tU32 address, tU8 length, tU8 *response, tU32 *responseLength);
tSInt       ETAL_cmdWrite_CMOST(ETAL_HANDLE hReceiver, tU32 address, tU32 *value, tU8 length);
tSInt 		ETAL_cmdAudioMute_CMOST(ETAL_HANDLE hReceiver, tU8 muteAction);
tSInt 		ETAL_cmdFMStereoMode_CMOST(ETAL_HANDLE hReceiver, tU8 fmStereoMode);
tSInt       ETAL_RDSreset_CMOST(ETAL_HANDLE hReceiver, etalRDSAttr *RDSAttr);
tSInt 		ETAL_cmdSetTunerStandby_CMOST(ETAL_HANDLE hTuner);
tSInt       ETAL_cmdAFCheck_CMOST(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p);
tSInt 		ETAL_cmdAFSwitch_CMOST(ETAL_HANDLE hReceiver, tU32 alternateFrequency);
tSInt 		ETAL_cmdAFStart_CMOST(ETAL_HANDLE hReceiver, etalAFModeTy AFMode, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p);
tSInt 		ETAL_cmdAFEnd_CMOST(ETAL_HANDLE hReceiver, tU32 frequency, EtalBcastQualityContainer* p);
tSInt 		ETAL_cmdGetAFQuality_CMOST(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p);
etalCMOSTTunerMode ETAL_EtalStandard_To_CMOST_Band(EtalBcastStandard std);
#endif

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
tSInt       ETAL_cmdSeekStart_CMOST(ETAL_HANDLE hReceiver, etalSeekDirectionTy dir, tU32 dwAMFMstepOrDiscreteFreq, etalSeekModeTy seekMode, etalSeekAudioTy stayMuted, tBool updateStopFrequency, tBool seekOnDiscreteFreq);
tSInt 		/*@alt void@*/ETAL_cmdSeekEnd_CMOST(ETAL_HANDLE hReceiver, etalSeekAudioTy muted);
tSInt 		ETAL_cmdSeekGetStatus_CMOST(ETAL_HANDLE hReceiver, tBool *pO_seekStoped, tBool *pO_fullCycleReached, tBool *pO_bandBorderCrossed, tU32 *freq, EtalBcastQualityContainer *quality_info);
tSInt       ETAL_cmdGetRDS_CMOST(ETAL_HANDLE hReceiver, tU8 *buf, tU32 *len);
tSInt       ETAL_cmdGetReceptionQuality_CMOST(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p);
tSInt		ETAL_cmdGetChannelQuality_CMOST(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p);
ETAL_STATUS ETAL_cmdStartMonitor_CMOST(ETAL_HANDLE hMonitor);
ETAL_STATUS ETAL_cmdStopMonitor_CMOST(ETAL_HANDLE hMonitor);
tSInt 		ETAL_cmdSetSeekThreshold_CMOST(ETAL_HANDLE hReceiver, EtalSeekThreshold *threshold);

tVoid       ETAL_getParameter_CMOST(tU8 *cmd, tU32 *p);
tSInt       ETAL_cmdSetFMProc_CMOST(ETAL_HANDLE hReceiver, EtalFMMode fmMode);
tSInt       ETAL_cmdDebugSetDISS_CMOST(ETAL_HANDLE hReceiver, etalChannelTy tuner_channel, EtalDISSMode mode, tU8 filter_index);
tSInt       ETAL_cmdDebugGetWSPStatus_CMOST(ETAL_HANDLE hReceiver, EtalWSPStatus *WSPStatus);
#endif

tSInt       ETAL_directCmdPing_CMOST(ETAL_HANDLE hTuner);
tSInt       ETAL_directCmdWrite_CMOST(ETAL_HANDLE hTuner, tU32 address, tU32 value);
tSInt       ETAL_directCmdRead_CMOST(ETAL_HANDLE hTuner, tU32 address, tU32 value, tU8 *resp, tU32 *rlen);
tSInt       ETAL_directCmdSetBBIf_CMOST(ETAL_HANDLE hTuner, EtalBBIntfModeTy bb_mode, EtalAudioIntfModeTy ai_mode);
tSInt		ETAL_directCmdSetAudioIf_CMOST(ETAL_HANDLE hTuner, etalAudioIntfStatusTy intf);
tSInt       ETAL_directCmdFMStereoMode_CMOST(ETAL_HANDLE hTuner, tU8 fmStereoMode);
tSInt       ETAL_directCmdAudioMute_CMOST(ETAL_HANDLE hTuner, tU8 muteAction);
#if (defined CONFIG_ETAL_SUPPORT_DCOP)
tSInt       ETAL_directCmdSetHDBlender_CMOST(ETAL_HANDLE hTuner, tU8 mode);
tSInt       ETAL_directCmdSetGPIOforAudioInput_CMOST(ETAL_HANDLE hTuner);
tSInt       ETAL_directCmdConfigSAIForDCOP_CMOST(ETAL_HANDLE hTuner);
tSInt       ETAL_directCmdSetSAI_BB_CMOST(ETAL_HANDLE hTuner);
tSInt       ETAL_directCmdConfigSAI_WS_MUX_CMOST(ETAL_HANDLE hTuner);
tSInt       ETAL_directCmdChangeBandTune_CMOST(ETAL_HANDLE hTuner);
#endif
tSInt 		ETAL_CmdTunerConfHDBlend_CMOST(ETAL_HANDLE hReceiver, tU32 vI_AD_GainStep, tU32 vI_DA_GainStep, tS8 vI_AV_Gain, tS8 vI_DV_Gain, tU8 vI_CdNo);

tSInt       ETAL_sendCommandTo_CMOST(ETAL_HANDLE hGeneric, tU8 *cmd, tU32 clen, tU16 *cstat, tU8 *resp, tU32 *rlen);

