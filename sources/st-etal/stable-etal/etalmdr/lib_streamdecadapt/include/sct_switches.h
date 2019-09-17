/*!
  @file         sct_switches.h
  @brief        <b>Basic compilation switches for ARM software </b>
  @author       CM-AI/PJ-CF42
  @version      0.1 (adapted from DFIRE)
  @date         2009.11.05
  @bug          Unknown
  @warning      None

 Copyright © Robert Bosch Car Multimedia GmbH, 1995-2012
 This code is property of the Robert Bosch Car Multimedia GmbH. Unauthorized
 duplication and disclosure to third parties is prohibited.

*/

#ifndef _SCT_SWITCHES_H_
#define _SCT_SWITCHES_H_

#include "target_config.h"
#ifndef CONFIG_TARGET_OS_IS_LINUX
#include "registers_sta660.h"
#endif
/******************************************************************************
 * DEFINES
 ******************************************************************************/
 /* For verification purpose only: let tuner input #2 be the source for second DAB-IP at tuner id #3 */
#undef FORK_TUNER2_DATA_TO_TUNER3_AT_FEI
/*------------------------------------------------------------------*
 * Environment
 *------------------------------------------------------------------*/
#define   noSIMULATOR_DABDEVICE
#define   noSIMULATE_TUNE
#define   noDATA_RECORDER
#define   noDATA_RECORDER_FFT
#define   noDATA_RECORDER_TCOR
#define   noDAB_TCOR_INIT_0
#define   noDAB_FSY_DELAY

#define ADR3_SW_VERSION 0xffffffffu

/*------------------------------------------------------------------*
 * settings for DAB
 *------------------------------------------------------------------*/

/* the number of DAB IPs to be supported by the SW */
#define NUM_DAB_IP 2
#if NUM_DAB_IP < CONFIG_MAX_NUM_DAB_DEVICES
/* number of DAB-IPs must be at least equal to number of DAB-Devices */
#error number of DAB-IPs must be at least equal to number of DAB-Devices
#endif

/* which DAB-IP shall be the source for audio output to streamdecoder by default? #0 or #1 */
#define DEFAULT_DABIP_ID_FOR_AUDIO_SUBCH_SOURCE 0

/* enable TII analysis */
#define SUPPORT_TII_ANALYSIS
#ifdef SUPPORT_TII_ANALYSIS
/* do it on system controller instead of DSP */
#define TII_ANALYSIS_ON_SCT
#endif

/* support DAB-IPs notch filter NTC? */
#undef SUPPORT_DABIP_NOTCH_FILTER

/* channel decoding version */
/* possible values: */
#define SINGLE_SUBCH_DECODING_INTERN 0
#define FULLSTREAM_DECODING          2
/* now decide : */
#define SUBCH_DECODING_SW_VERSION  FULLSTREAM_DECODING
/* the maximum number of subchannels that can be decoded simultaneously */
#if SUBCH_DECODING_SW_VERSION == FULLSTREAM_DECODING
#define MAX_NUM_DECODABLE_SUBCH  4
#elif SUBCH_DECODING_SW_VERSION == SINGLE_SUBCH_DECODING_INTERN
#define MAX_NUM_DECODABLE_SUBCH  2
#else
 error
#endif

/* the maximum number of CUs that can be decoded simultaneously */
#if SUBCH_DECODING_SW_VERSION == FULLSTREAM_DECODING
#define MAX_NUM_DECODABLE_CU 864
#elif SUBCH_DECODING_SW_VERSION == SINGLE_SUBCH_DECODING_INTERN
#define MAX_NUM_DECODABLE_CU 280
#else
 error not a valid version
#endif

/* DAB-IP can decode max 32 subchannels in parallel
 * (the subchannel decoding parameter list for TDI and VIT consists
 * of up to this number of subchannels) */
#define DABIP_MAX_NUM_DECODABLE_SUBCH 32
/* security check for number of subchannels */
#if MAX_NUM_DECODABLE_SUBCH > DABIP_MAX_NUM_DECODABLE_SUBCH
#error this cannot work
#endif

#ifdef CONFIG_TARGET_SYS_ADR3
	#define noUSE_DAB_CONCEALMENT
	#define USE_DABPLUS_RS_DECODER
	#define USE_DMB_RS_DECODER
#elif ( (1==CONFIG_TARGET_AUDIO_CODEC_CIDANA_DABPLUS_AAC) || \
		(1==CONFIG_TARGET_AUDIO_CODEC_CIDANA_DMB_AAC) )
#define SW_RS_DECODER	/* W.S. use RS decoder in SW e.g in Simulator or target 3b */
#endif

/* send a ms_notify_channel_selection(action == resume) to streamdecoder after retune and re-activating subchannel? */
#define SEND_NOTIFYCHSEL_AFTER_AUDIO_SUBCH_RESTORE

/* what RF frontend do we have to control? */
#define NO_RF_TUNER                  0
#define RF_TUNER_STA610              8

#ifdef CONFIG_DABDEV_STA610
#define SUPPORTED_DAB_RF_TUNER     RF_TUNER_STA610
#endif

#define FRI_MODE_SERIAL               20
#define FRI_MODE_BIPHASE              21
#define FRI_MODE_SIGNED_ADC           22
#define FRI_MODE_UNSIGNED_ADC         23
#define ETP_INPUT                     24 /*=BASEBAND*/
#define FRI_MODE_CF730                26
#define FRI_MODE_STA610               27

#if ((SUPPORTED_DAB_RF_TUNER==RF_TUNER_STA610)||( defined( CONFIG_TARGET_DEV_AMFM_AUD__ENABLE ) ))
#define DAB_FRI_INPUT_MODE FRI_MODE_STA610
#endif

#if (SUPPORTED_DAB_RF_TUNER == RF_TUNER_STA610)
// The AGC (or linear AGC) can be enabled only on CA silicon, on BA the
// stepped AGC shall remain enabled by default
#if defined (CONFIG_TARGET_SYS_STA662_Bx)
/* shall the SW-AGC for gain control of baseband signal be performed in DABDEVICE
 * (not needed anymore if linear AGC is performed within lib_tuner) */
#define SUPPORT_DABDEV_STA610_SWAGC

/* Workaround for DC offset compensation weakness: switch for activating the
 * masking out of small BERs. This masks the BERs caused by imperfect DC offset
 * compensation in STA662's FEI.*/
#define ENABLE_MASK_OUT_SMALL_BER_FROM_DCOFFSET_COMPENS
#else
/* shall the SW-AGC for gain control of baseband signal be performed in DABDEVICE
 * (not needed anymore if linear AGC is performed within lib_tuner) */
#undef SUPPORT_DABDEV_STA610_SWAGC

#if (!defined SUPPORT_DABDEV_STA610_SWAGC) && (!defined CONFIG_TARGET_TUNERAGC_DAB_WIDEBAND_FST)
#define USE_MAGNITUDE_MONITOR_FOR_LIN_AGC
#endif

#endif /* #if (SUPPORTED_DAB_RF_TUNER == RF_TUNER_STA610) */

/* Workaround for DC offset compensation weakness: switch for activating the
 * masking out of small BERs. This masks the BERs caused by imperfect DC offset
 * compensation in STA662's FEI.*/
#undef ENABLE_MASK_OUT_SMALL_BER_FROM_DCOFFSET_COMPENS
#endif // #if defined (TARGET_SYS_STA662_Bx)

/* Support guessing the channel coding mode DAB/DAB+/DMB for subchannels
 * FIG 0/2 signalling  ("service-less" subchannels) */
#define SUPPORT_GUESS_SUBCHANNEL_CODING_MODE

/*------------------------------------------------------------------*
 * API
 *------------------------------------------------------------------*/
/* support old notify_audio_status or new notify_audio_status_dab_dabplus_dmb */
//#undef API_SUPPORT_AUDIO_STATUS_DAB_DABPLUS_DMB
#define API_SUPPORT_AUDIO_STATUS_DAB_DABPLUS_DMB


/* some manufacturer specific API functions */
#define SUPPORT_API_TRACKING_NETTO_BER	/* has to be activated net BER calulation for all-"0" data channels */
#define noNETTO_BER_MODE_NEW			/* has to be activated for DMON net BER indication, has to be deactivated for CF41 Tools*/
#define SUPPORT_API_MS_CMD_GET_VERSION
#define noSUPPORT_API_MS_CMD_GET_PROD_DATA

/* which modules uses which general pupose tracking? */
#define NO_GP_TRACKING -1
#define GP_TRACKING_0   0
#define GP_TRACKING_1   4
#define GP_TRACKING_2   8
#define GP_TRACKING_3  12
/* assign GP tracking to modules: */
#define GP_TRACKING_SRC_CTRL  NO_GP_TRACKING
#define GP_TRACKING_FFT_CTRL  NO_GP_TRACKING
#define GP_TRACKING_CONC_CTRL NO_GP_TRACKING

/*------------------------------------------------------------------*
 * FIB decoding
 *------------------------------------------------------------------*/

/* what to to with printf? */

#include <stdio.h>
#if(0)
#   define printf0(a) printf(a)
#   define printf1(a,b) printf(a,b)
#   define printf2(a,b,c) printf(a,b,c)
#   define printf3(a,b,c,d) printf(a,b,c,d)
#   define printf4(a,b,c,d,e) printf(a,b,c,d,e)
#   define printf5(a,b,c,d,e,f) printf(a,b,c,d,e,f)
#   define printf6(a,b,c,d,e,f,g) printf(a,b,c,d,e,f,g)
#   define printf7(a,b,c,d,e,f,g,h) printf(a,b,c,d,e,f,g,h)
#else
#   define printf0(a) ;
#   define printf1(a,b) ;
#   define printf2(a,b,c);
#   define printf3(a,b,c,d);
#   define printf4(a,b,c,d,e);
#   define printf5(a,b,c,d,e,f);
#   define printf6(a,b,c,d,e,f,g);
#   define printf7(a,b,c,d,e,f,g,h);
 #endif
/************************************************
 * Combination of switches
 ************************************************/

/************************************************
 * Error checking for switches
 ************************************************/

/********************
 * API Error checking
 *********************/

#endif /* #ifndef _SCT_SWITCHES_H_ */

