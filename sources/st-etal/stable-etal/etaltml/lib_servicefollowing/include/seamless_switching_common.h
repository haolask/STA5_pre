//!
//!  \file      seamless_switching_common.h
//!  \brief     <i><b> seamless switching implementation </b></i>
//!  \details   This file provides external interface for seamless switching services
//!  \author    Jean-Hugues Perrin
//!  \author    (original version) Jean-Hugues Perrin
//!  \version   1.0
//!  \date      2014.11.04
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SEAMLESS_SWITCHING_COMMON_H_
#define SEAMLESS_SWITCHING_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********************
* DEFINE SECTION
***********************/
#define SD_SEAMLESS_ESTIMATION_CONFIRMATION_REQUESTED	    0x01
#define SD_SEAMLESS_ESTIMATION_CONFIRMATION_NOT_REQUESTED	0x00
#define SD_SEAMLESS_ESTIMATION_DEFAULT_DELAY                8888888
#define SD_SEAMLESS_ESTIMATION_DEFAULT_CONFIDENCE_LEVEL     0
#define SD_SEAMLESS_ESTIMATION_DEFAULT_TS                   0
#define SD_SEAMLESS_ESTIMATION_DEFAULT_RMS2                 1

#if defined(CONFIG_SEAMLESS_SWITCHING_DELAY_SUPPORTED_14S)
		#define SD_DEFAULT_CONFIG_SS_FULL_WINDOWS_START_IN_SAMPLE          ((-14) * 48000)
#elif defined(CONFIG_SEAMLESS_SWITCHING_DELAY_SUPPORTED_12S)
		#define SD_DEFAULT_CONFIG_SS_FULL_WINDOWS_START_IN_SAMPLE          ((-12) * 48000)
#else
		#define SD_DEFAULT_CONFIG_SS_FULL_WINDOWS_START_IN_SAMPLE          ((-10) * 48000)
#endif

#define SD_DEFAULT_CONFIG_SS_FULL_WINDOWS_STOP_IN_SAMPLE           0


// SD RX messages protocol
#define SD_MESSAGE_RX_SEAMLESS_SWITCHING             (tU8)0x01
#define SD_MESSAGE_RX_SEAMLESS_ESTIMATION            (tU8)0x07

// SD TX messages protocol

#define SD_MESSAGE_TX_SEAMLESS_ESTIMATION            (tU8)0x02
#define SD_MESSAGE_TX_SEAMLESS_SWITCHING            (tU8)0x03


/**********************
* STRUCTURE SECTION
***********************/
typedef enum
{
	SD_SEAMLESS_ESTIMATION_STOP_MODE		= 0, // request for on going procedure stop
	SD_SEAMLESS_ESTIMATION_START_MODE_1		= 1, // ie Delay + loundness Estimation
	SD_SEAMLESS_ESTIMATION_START_MODE_2		= 2, // ie Delay only Estimation
	SD_SEAMLESS_ESTIMATION_START_MODE_3		= 3  // ie loundness only Estimation
} SD_SeamlessEstimationModeTy;

typedef enum
{
    SD_SEAMLESS_ESTIMATION_STATUS_NONE                          = 0x00,
    SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS 	 					= 0x01,
    SD_SEAMLESS_ESTIMATION_STATUS_FAILURE 						= 0x02,
    SD_SEAMLESS_ESTIMATION_STATUS_STOPPED 						= 0x04,
    SD_SEAMLESS_ESTIMATION_STATUS_ERROR_ON_DELAY_ESTIMATION		= 0x08,
    SD_SEAMLESS_ESTIMATION_STATUS_ERROR_ON_LOUDNESS_ESTIMATION	= 0x10,
    SD_SEAMLESS_ESTIMATION_STATUS_ERROR_ON_BUFFERING            = 0x20,
    SD_SEAMLESS_ESTIMATION_STATUS_ERROR_ON_SEAMLESS_ESTIMATION  = 0x40
} SD_SeamlessEstimationStatusTy;

typedef enum
{
    SD_SEAMLESS_SWITCHING_STATUS_SUCCESSFUL					    = 0x00,
    SD_SEAMLESS_SWITCHING_STATUS_SUCCESSFUL_WITH_DEFAULT_VALUE	= 0x01,
    SD_SEAMLESS_SWITCHING_STATUS_SUCCESSFUL_APPROXIMATED		= 0x02,
    SD_SEAMLESS_SWITCHING_STATUS_FAILURE					   	= 0x03
} SD_SeamlessSwitchingStatusTy;

typedef enum
{
    SD_SEAMLESS_PROVIDER_IS_DAB,
    SD_SEAMLESS_PROVIDER_IS_FM,
    SD_SEAMLESS_PROVIDER_IS_NONE
} SD_SeamlessEstimationDataProviderTy;

typedef enum
{
    SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB,
    SD_SEAMLESS_SWITCHING_SWITCH_TO_FM,
    SD_SEAMLESS_SWITCHING_SWITCH_TO_EARLY_FM
} SD_SeamlessSwitchingSystemToSwitchTy;

/***********************/
/* SEAMLESS ESTIMATION */
/***********************/
typedef struct
{
	tU8 msg_id;							    // MSG_ID
	SD_SeamlessEstimationModeTy mode;	    // Mode (in Enum)
	tS32 start_position_in_samples;			// Start position in samples
    tS32 stop_position_in_samples;		    // Stop position in samples
	tU8 downSampling; 						// Down sampling factor applied for delay estimation.
	tU8 loudness_duration;					// Loudness duration in seconds
} SD_msg_SeamlessEstimationRequest;


typedef struct
{
	tU8 msg_id;												// MSG_ID
	SD_SeamlessEstimationStatusTy status;					// Report status
	SD_SeamlessEstimationDataProviderTy provider_type;  	// Audio reference for delay estimation
#ifndef CONFIG_TARGET_SYS_ACCORDO5
	tU8 dummy1;
#endif
	tS32 absolute_estimated_delay_in_samples;				// Absolute delay estimate in samples
	tS32 delay_estimate;									// Delay estimate in samples
	tU32 timestamp_FAS;										// Time stamp on FAS for the delay estimate
	tU32 timestamp_SAS;										// Time stamp on SAS for the delay estimate
	tU32 average_RMS2_FAS;									// Average of squared RMS on FAS
	tU32 average_RMS2_SAS;									// Average of squared RMS on SAS
	tU32 confidence_level;									// Confidence level of delay estimate
} SD_msg_SeamlessEstimationResponse;


/**********************/
/* SEAMLESS SWITCHING */
/**********************/
typedef struct
{
	tU8 msg_id;												// MSG_ID
	SD_SeamlessSwitchingSystemToSwitchTy systemToSwitch;	// System to switch
	SD_SeamlessEstimationDataProviderTy provider_type;  	// Audio reference for delay estimation
	tBool confirmationRequested;							// Request or not seamless estimation reconfirmation
	tS32 absolute_estimated_delay_in_samples;				// Audio reference for delay estimation
	tS32 delay_estimate;									// Delay estimate in samples
	tU32 timestamp_FAS;										// Time stamp on FAS for the delay estimate
	tU32 timestamp_SAS;										// Time stamp on SAS for the delay estimate
	tU32 average_RMS2_FAS;									// Average of squared RMS on FAS
	tU32 average_RMS2_SAS;									// Average of squared RMS on SAS
} SD_msg_SeamlessSwitchingRequest;

typedef struct
{
	tU8 msg_id;								// MSG_ID
	SD_SeamlessSwitchingStatusTy status;	// Report status
	tS32 delay;								// Absolute delay estimate in samples
	tU32 confidence_level;					// Confidence level of delay estimate
	tU32 average_RMS2_FAS;					// Average of squared RMS on FAS
	tU32 average_RMS2_SAS;					// Average of squared RMS on SAS
} SD_msg_SeamlessSwitchingResponse;

#ifdef __cplusplus
}
#endif

#endif // SEAMLESS_SWITCHING_COMMON_H_

// End of file

