/*!
  @file         dabdev.h
  @brief        <b>Header File for dabdev.c</b>
  @author       CM-AI/PJ-CF42
  @version      0.1
  @date         2009.10.05
  @bug          Unknown
  @warning      None

 Copyright © Robert Bosch Car Multimedia GmbH, 1995-2012
 This code is property of the Robert Bosch Car Multimedia GmbH. Unauthorized
 duplication and disclosure to third parties is prohibited.

*/

#if !defined (OSAL_DAB_DEV_HEADER)
   #define OSAL_DAB_DEV_HEADER

/* buffers should fit to ~100ms */
#define NTF_BUFFER_NUMB_PER_DABDEV           4
#define NTF_BUFFER_SIZE                    256

#define CMD_BUFFER_NUMB_PER_DABDEV           4
#ifdef CONFIG_TARGET_DEV_DABDEVICE_TEST__ENABLE
#define CMD_BUFFER_SIZE                    256
#else
#define CMD_BUFFER_SIZE                     64
#endif

#define FIC_BUFFER_NUMB_PER_DABDEV           8
#define FIC_BUFFER_SIZE                    384+2+3

#define ADD_SUBCHANNEL_BUFFERS               0xfa
#define REMOVE_SUBCHANNEL_BUFFERS            0xec

#define MAX_DAB_DATA_RATE                   384 /* data rate in kbit/s */
#define MAX_DABPLUS_DATA_RATE               192 /* data rate in kbit/s */
#define MAX_DMB_DATA_RATE                   640 /* data rate in kbit/s */


/*** INCLUDES ******************************************************************
 */
#if 0 /* this it not allowed, no OSAL includes in headers! (Maik) */
#include "osal.h"
#include "osioctrl.h"
#include "osio.h"
#include "dispatcher.h"
#include "osutilio.h"
#endif
/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
|defines and macros (scope: global)
|-----------------------------------------------------------------------*/

/************************************************************************
|typedefs and struct defs (scope: global)
|-----------------------------------------------------------------------*/

typedef tS32 (*OSAL_tpfDABDevSetupSubchBufferCallback) ( tPVoid pvArg,
														 tU8    device_id,
			                                             tU8    subch_id,
			                                             tU16   num_bytes,
			                                             tU8    add_remove );

struct OSAL_trAsyncDABDevSetupSubchBuffer_struct
{  /*make LINT happy*/ /*make LINT happy*/ /*make LINT happy*/
 OSAL_tpfDABDevSetupSubchBufferCallback  pCallBack;  /* Pointer to Callback function */
 tPVoid                                  pvArg;      /* argument of callback fct */
};

typedef struct OSAL_trAsyncDABDevSetupSubchBuffer_struct OSAL_trAsyncDABDevSetupSubchBuffer;

/* possible tuner path IDs for the DABDevice */
typedef enum {
    DABDEVICE_TUNER_PATH_1 = 1,
    DABDEVICE_TUNER_PATH_2 = 2,
    DABDEVICE_TUNER_PATH_3 = 3,
    DABDEVICE_TUNER_PATH_4 = 4,
    DABDEVICE_INVALID_TUNER_PATH = 0xff
} tDABDeviceTunerPathID;

/************************************************************************
| variable declaration (scope: global)
|-----------------------------------------------------------------------*/

/************************************************************************
|function prototypes (scope: global)
|-----------------------------------------------------------------------*/

typedef tU32 LLD_DABIRQ_IdTy;

typedef void (*OSAL_irqCallback) ( tVoid * );

#define DABIRQ_0_ID					(LLD_DABIRQ_IdTy)0
#define DABIRQ_1_ID					(LLD_DABIRQ_IdTy)1
#define DABIRQ_2_ID					(LLD_DABIRQ_IdTy)2

/*------------------------------------------------------------------------------
 *    DCSR message types
 *------------------------------------------------------------------------------
 */
#define DABDEV_DCSR_COMMAND(x)        ((0x01 << 6) | (x & 0x3f))
#define DABDEV_DCSR_RESPONSE(x)       ((0x02 << 6) | (x & 0x3f))
#define DABDEV_DCSR_NOTIFICATION(x)   ((0x03 << 6) | (x & 0x3f))


/*------------------------------------------------------------------------------
 *   Constants for DCSR commands
 *------------------------------------------------------------------------------
 */
#define DABDEV_DCSR_CMD_GET_RECEIVER_CAPABILITY            DABDEV_DCSR_COMMAND(0x01)
#define DABDEV_DCSR_CMD_TUNE                               DABDEV_DCSR_COMMAND(0x02)
#define DABDEV_DCSR_CMD_GET_TII                            DABDEV_DCSR_COMMAND(0x03)
#define DABDEV_DCSR_CMD_SELECT_TII                         DABDEV_DCSR_COMMAND(0x04)
#define DABDEV_DCSR_CMD_GET_PAD                            DABDEV_DCSR_COMMAND(0x05)
#define DABDEV_DCSR_CMD_SELECT_PAD                         DABDEV_DCSR_COMMAND(0x06)
#define DABDEV_DCSR_CMD_GET_FIGS                           DABDEV_DCSR_COMMAND(0x07)
#define DABDEV_DCSR_CMD_SELECT_FIGS                        DABDEV_DCSR_COMMAND(0x08)
#define DABDEV_DCSR_CMD_GET_CHANNEL                        DABDEV_DCSR_COMMAND(0x09)
#define DABDEV_DCSR_CMD_SELECT_CHANNEL                     DABDEV_DCSR_COMMAND(0x0A)
#define DABDEV_DCSR_CMD_GET_SELECTION_STATUS               DABDEV_DCSR_COMMAND(0x0B)
#define DABDEV_DCSR_CMD_SEARCH_FOR_ENSEMBLE                DABDEV_DCSR_COMMAND(0x0C)
#define DABDEV_DCSR_CMD_SET_DRC                            DABDEV_DCSR_COMMAND(0x0D)
#define DABDEV_DCSR_CMD_GET_AUDIO_INFO                     DABDEV_DCSR_COMMAND(0x0E)
#define DABDEV_DCSR_CMD_GET_DAB_STATUS                     DABDEV_DCSR_COMMAND(0x0F)
#define DABDEV_DCSR_CMD_SET_DAB_STATUS_AUTO_NOTIFICATION   DABDEV_DCSR_COMMAND(0x10)
#define DABDEV_DCSR_CMD_GET_ACTIVE_INFO                    DABDEV_DCSR_COMMAND(0x11)
#define DABDEV_DCSR_CMD_MANUFACTURER_SPECIFIC_COMMAND      DABDEV_DCSR_COMMAND(0x20)



/*------------------------------------------------------------------------------
 *   Constants for DCSR responses
 *------------------------------------------------------------------------------
 */
#define DABDEV_DCSR_RESPONSE_ACCEPTED                      DABDEV_DCSR_RESPONSE(0x01)
#define DABDEV_DCSR_RESPONSE_REJECTED                      DABDEV_DCSR_RESPONSE(0x02)
#define DABDEV_DCSR_RESPONSE_INTERIM                       DABDEV_DCSR_RESPONSE(0x03)
#define DABDEV_DCSR_RESPONSE_COMMAND_NOT_IMPLEMENTED       DABDEV_DCSR_RESPONSE(0x04)
#define DABDEV_DCSR_RESPONSE_BUSY                          DABDEV_DCSR_RESPONSE(0x05)
#define DABDEV_DCSR_RESPONSE_SYNTAX_ERROR                  DABDEV_DCSR_RESPONSE(0x06)

/*------------------------------------------------------------------------------
 *   Constants for DCSR notifications
 *------------------------------------------------------------------------------
 */
#define DABDEV_DCSR_NOTIFY_RECEIVER_CAPABILITY             DABDEV_DCSR_NOTIFICATION(0x01)
#define DABDEV_DCSR_NOTIFY_TII                             DABDEV_DCSR_NOTIFICATION(0x03)
#define DABDEV_DCSR_NOTIFY_PAD                             DABDEV_DCSR_NOTIFICATION(0x05)
#define DABDEV_DCSR_NOTIFY_FIG                             DABDEV_DCSR_NOTIFICATION(0x07)
#define DABDEV_DCSR_NOTIFY_CHANNEL                         DABDEV_DCSR_NOTIFICATION(0x09)
#define DABDEV_DCSR_NOTIFY_SELECTION_STATUS                DABDEV_DCSR_NOTIFICATION(0x0B)
#define DABDEV_DCSR_NOTIFY_SEARCH_FOR_ENSEMBLE             DABDEV_DCSR_NOTIFICATION(0x0C)
#define DABDEV_DCSR_NOTIFY_AUDIO_INFO                      DABDEV_DCSR_NOTIFICATION(0x0E)
#define DABDEV_DCSR_NOTIFY_DAB_STATUS                      DABDEV_DCSR_NOTIFICATION(0x0F)
#define DABDEV_DCSR_NOTIFY_ACTIVE_INFO                     DABDEV_DCSR_NOTIFICATION(0x11)
#define DABDEV_DCSR_NOTIFY_SERVICE_FOLLOWING               DABDEV_DCSR_NOTIFICATION(0x12)
#define DABDEV_DCSR_MANUFACTURER_SPECIFIC_NOTIFICATION     DABDEV_DCSR_NOTIFICATION(0x20)
#define DABDEV_DCSR_NOTIFY_ERROR_MESSAGE                   DABDEV_DCSR_NOTIFICATION(0x30)


/*------------------------------------------------------------------------------
 *   Constants for DCSR manufacturer specific commands
 *------------------------------------------------------------------------------
 */
#define DABDEV_DCSR_MS_CMD_MONITORING                      0x01
#define DABDEV_DCSR_MS_CMD_CONFIG_TRACKING                 0x01
#define DABDEV_DCSR_MS_CMD_SET_CPI_REG                     0x04
#define DABDEV_DCSR_MS_CMD_GET_CPI_REG                     0x05
#define DABDEV_DCSR_MS_CMD_SET_CONFIG_REG                  0x06
#define DABDEV_DCSR_MS_CMD_GET_CONFIG_REG                  0x07
#define DABDEV_DCSR_MS_CMD_DUMP_DATA                       0x09
#define DABDEV_DCSR_MS_NOTIFY_DUMP_DATA                    DABDEV_DCSR_MS_CMD_DUMP_DATA
#define DABDEV_DCSR_MS_CMD_GET_SUBCH_PARAMS                0x0b
#define DABDEV_DCSR_MS_CMD_SNOOP_DAB                       0x14
#define DABDEV_DCSR_MS_CMD_GET_BER                         0x17
#define DABDEV_DCSR_MS_CMD_SET_CONCEALMENT_LEVEL           0x1b
#define DABDEV_DCSR_MS_CMD_STA610                          0x33
#define DABDEV_DCSR_MS_CMD_GET_VERSION                     0x50
#define DABDEV_DCSR_MS_CMD_GET_PROD_DATA                   0x51
#define DABDEV_DCSR_MS_CMD_GET_AUDIO_STATUS                0x52
#define DABDEV_DCSR_MS_CMD_AUTO_NOTIFICATION               0x54
#define DABDEV_DCSR_MS_CMD_CLIENTTESTMODE				   0x55
#define DABDEV_DCSR_MS_CMD_ADJUST_AUDIO_SAMPLING_RATE	   0x60
#define DABDEV_DCSR_MS_CMD_ADJUST_AUDIO_SAMPLING_RATE_FM   0x61
#define DABDEV_DCSR_MS_CMD_NOP                             0xff
//#define DABDEV_DCSR_MS_CMD_DEVICE_ID_0                     0xa0
//#define DABDEV_DCSR_MS_CMD_DEVICE_ID_1                     0xa1
#define DABDEV_DCSR_MS_CMD_SET_TUNER_PATH                  0xb0
#define DABDEV_DCSR_MS_CMD_GET_STM_FW_VERSION              0xFE
/* W.S. 2011-07-11 add some workaround to enable FM tuning with DMON */
#define DABDEV_DCSR_MS_CMD_TUNE_FM			               0x11
#define DABDEV_DCSR_MS_CMD_SNOOP_FM                        0x12
#define DABDEV_DCSR_MS_CMD_SELECT_RDS_FM_INFO              0x13
#define DABDEV_DCSR_MS_CMD_SEARCH_FM                       0x15
/*------------------------------------------------------------------------------
 *   Constants for DCSR manufacturer specific notifications
 *------------------------------------------------------------------------------
 */
#define DABDEV_DCSR_MS_NOTIFY_MONITORING                        DABDEV_DCSR_MS_CMD_MONITORING
#define DABDEV_DCSR_MS_NOTIFY_TRACKING                          0x01
#define DABDEV_DCSR_MS_NOTIFY_CPI_REG                           DABDEV_DCSR_MS_CMD_GET_CPI_REG
#define DABDEV_DCSR_MS_NOTIFY_CONFIG_REG                        0x07
#define DABDEV_DCSR_MS_NOTIFY_SUBCH_PARAMS                      0x0b
#define DABDEV_DCSR_MS_NOTIFY_SNOOP_DAB                         DABDEV_DCSR_MS_CMD_SNOOP_DAB
#define DABDEV_DCSR_MS_NOTIFY_GET_BER                           0x18
#define DABDEV_DCSR_MS_NOTIFY_AUDIO_STATUS                      DABDEV_DCSR_MS_CMD_GET_AUDIO_STATUS
#define DABDEV_DCSR_MS_NOTIFY_AUDIO_STATUS_DAB_DABPLUS_DMB      DABDEV_DCSR_MS_CMD_GET_AUDIO_STATUS
#define DABDEV_DCSR_MS_NOTIFY_STA610                            0x35

#define DABDEV_DCSR_MS_NOTIFY_VERSION                           0x50
#define DABDEV_DCSR_MS_NOTIFY_PROD_DATA                         0x51
#define DABDEV_DCSR_MS_NOTIFY_CLIENTTESTMODE					0x55
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS             0x61
#define DABDEV_DCSR_MS_NOTIFY_SNOOP_INPUT_POWER                 0x90
#define DABDEV_DCSR_MS_NOTIFY_ERROR_MESSAGE                     0xf0
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_SELECTION					0xf1
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL                           0xf2

#define DABDEV_DCSR_MS_NOTIFY_STM_FW_VERSION                    0xFE
/* W.S. 2011-07-11 add some workaround to enable FM tuning with DMON */
#define DABDEV_DCSR_MS_NOTIFY_TUNE_FM                           DABDEV_DCSR_MS_CMD_TUNE_FM
#define DABDEV_DCSR_MS_NOTIFY_SNOOP_FM                          DABDEV_DCSR_MS_CMD_SNOOP_FM
#define DABDEV_DCSR_MS_NOTIFY_RDS_FM_INFO                       DABDEV_DCSR_MS_CMD_SELECT_RDS_FM_INFO
#define DABDEV_DCSR_MS_NOTIFY_SEARCH_FM                         DABDEV_DCSR_MS_CMD_SEARCH_FM
#define DABDEV_DCSR_MS_NOTIFY_FM_STATUS							0x16

/*------------------------------------------------------------------------------
 *   Constants for DCSR interfaces
 *------------------------------------------------------------------------------
 */
#define MAX_NUM_OUTPUTINTERFACES					8

#define INTERFACE_PROTOCOL_DEFAULT                  0
#define INTERFACE_PROTOCOL_RDI_LOW                  1
#define INTERFACE_PROTOCOL_RDI_HIGH_NORM            2
#define INTERFACE_PROTOCOL_RDI_HIGH_EXT             3
#define INTERFACE_PROTOCOL_SPDIF                    4
#define INTERFACE_PROTOCOL_AES_EBU                  5
#define INTERFACE_PROTOCOL_I2S                      6
#define INTERFACE_PROTOCOL_DCSR                     7

#define INTERFACE_ID_DEFAULT                        0
#define INTERFACE_ID_AUDIO_SPEAKER                  1
#define INTERFACE_ID_HEADPHONE                      2
#define INTERFACE_ID_AUDIO_LINE_FIXED               3
#define INTERFACE_ID_AUDIO_LINE_VARIABLE            4
#define INTERFACE_ID_AUDIO_DIGITAL_OPTICAL          5
#define INTERFACE_ID_AUDIO_DIGITAL_ELECTRICAL       6
#define INTERFACE_ID_IEEE1394                       7
#define INTERFACE_ID_RF_INPUT                       8
#define INTERFACE_ID_VIDEO_RGB                      9
#define INTERFACE_ID_VIDEO_FBAS                    10
#define INTERFACE_ID_S_VIDEO                       11
#define INTERFACE_ID_RS232                         12
#define INTERFACE_ID_D2B                           13
#define INTERFACE_ID_IEEE1284                      14
#define INTERFACE_ID_IEC958                        15
#define INTERFACE_ID_SPI                           16
#define INTERFACE_ID_I2C                           17
#define INTERFACE_ID_USB                           18
#define INTERFACE_ID_IRDA                          19

#define INTERFACE_ID_MEM_API                      126

/******************************************************************************
 * TYPE DEFINITIONS
 ******************************************************************************/

//#define INTERFACEREF_RF_IN   0x01
#ifndef INTERFACEREF_AUD_OUT
#define INTERFACEREF_AUD_OUT 0x02
#endif
/* #define INTERFACEREF_RDI_OUT 0x00 */
//#define INTERFACEREF_MEM_OUT 0x03

typedef enum       /* The possible reasons for a 'notify_dab_status' message */
{
  DABDEV_REASON_GET_DAB_STATUS         = 0,
  DABDEV_REASON_TUNE                   = 1,
  DABDEV_REASON_SEARCH_FOR_ENSEMBLE    = 2,
  DABDEV_REASON_SYNC                   = 4,
  DABDEV_REASON_RECONF                 = 5,
  DABDEV_REASON_BER_FIC                = 6,
  DABDEV_REASON_MUTE                   = 7,
  DABDEV_REASON_TUNE_CANCELLED         = 8
} DabdevNotifyDabStatusReason;

#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS_LEN  (2+1+1+10)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__channel_type(_buf_)               ((((tU8*)(_buf_))[4]>>4)&0xf)
/* #define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__rfu1(_buf_)                    ((((tU8*)(_buf_))[4])&0xf) */
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__output_interface_reference(_buf_) ((((tU8*)(_buf_))[5]))
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__subch_id(_buf_)                   ((((tU8*)(_buf_))[6]))
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__conc_level(_buf_)                 ((((tU8*)(_buf_))[7]>>4)&0x0f)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__equalizer_level(_buf_)            ((((tU8*)(_buf_))[7]>>1)&0x07)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__mute_flag(_buf_)                  ((((tU8*)(_buf_))[7])&0x01)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__header_crc_err_cnt(_buf_)         ((((tU8*)(_buf_))[8]>>4)&0xf)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__scf_crc_err_cnt(_buf_)            ((((tU8*)(_buf_))[8])&0xf)
/* #define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__rfu2(_buf_)                    ((((tU8*)(_buf_))[9]>>4)&0xf) */
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__ms_flag(_buf_)                    ((((tU8*)(_buf_))[9]>>3)&0x1)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__audio_mode(_buf_)                 ((((tU8*)(_buf_))[9])&0x7)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__core_audio_coding(_buf_)          ((((tU8*)(_buf_))[10]>>5)&0x7)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__drc_status(_buf_)                 ((((tU8*)(_buf_))[10]>>3)&0x3)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__drc_scaling(_buf_)                ((((tU8*)(_buf_))[10])&0x7)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__audio_bitrate(_buf_)              (((((tU8*)(_buf_))[11])<<1)|(((((tU8*)(_buf_))[12])>>7)&0x01))
/* #define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__rfu3(_buf_)                    ((((tU8*)(_buf_))[12]>>5)&0x3) */
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__original_flag(_buf_)              ((((tU8*)(_buf_))[12]>>4)&0x1)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__copyright_flag(_buf_)             ((((tU8*)(_buf_))[12]>>3)&0x1)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__dac_rate(_buf_)                   ((((tU8*)(_buf_))[12])&0x7)
/* #define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__rfu4(_buf_)                    ((((tU8*)(_buf_))[13]>>6)&0x3) */
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__sbr_flag(_buf_)                   ((((tU8*)(_buf_))[13]>>5)&0x1)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__aac_channel_mode(_buf_)           ((((tU8*)(_buf_))[13]>>4)&0x1)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__ps_flag(_buf_)                    ((((tU8*)(_buf_))[13]>>3)&0x1)
#define DABDEV_DCSR_MS_NOTIFY_STREAM_DECODER_STATUS__mpeg_surround_config(_buf_)       ((((tU8*)(_buf_))[13])&0x7)

#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_SELECTION_LEN  (2+1+1+5)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_SELECTION__OUTIFACE_ID(_buf_)   ((tU8*)(_buf_))[4]
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_SELECTION__ONOFF(_buf_)         ((((tU8*)(_buf_))[5])&0x03)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_SELECTION__ERR_CONC_CTRL(_buf_) (((((tU8*)(_buf_))[5])&0x0c)>>2)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_SELECTION__MODE(_buf_)          (((((tU8*)(_buf_))[5])&0xf0)>>4)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_SELECTION__SUBCH_ID(_buf_)      ((tU8*)(_buf_))[6]
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_SELECTION__SUBCH_SIZE(_buf_)    ((((tU8*)(_buf_))[7]<<8)|(((tU8*)(_buf_))[8]<<0))

#if 0
 /* old format */
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__HEADER_LEN (2+1+1+22)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_LEN(_subch_size_)  (DABDEV_DCSR_MS_NOTIFY_CHANNEL__HEADER_LEN+_subch_size_)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE(_buf_)    ((((tU8*)(_buf_))[4]>>4)&0xf)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__RFU(_buf_)    ((((tU8*)(_buf_))[4]>>3)&0x1)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__DRC_SCALING(_buf_)    ((((tU8*)(_buf_))[4]>>0)&0x7)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__SUBCH_ID(_buf_)    ((tU8*)(_buf_))[5]
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__SUB_ID(_buf_) ((((tU8*)(_buf_))[6]<<8)|(((tU8*)(_buf_))[7]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_FLAGS(_buf_) ((((tU8*)(_buf_))[8]<<8)|(((tU8*)(_buf_))[9]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_CNT(_buf_) ((((tU8*)(_buf_))[10]<<8)|(((tU8*)(_buf_))[11]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_RECONF(_buf_) ((((tU8*)(_buf_))[12]<<8)|(((tU8*)(_buf_))[13]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_CONF(_buf_) ((((tU8*)(_buf_))[14]<<8)|(((tU8*)(_buf_))[15]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__FIC_BER(_buf_) ((((tU8*)(_buf_))[16]<<8)|(((tU8*)(_buf_))[17]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__MSC_BER(_buf_) ((((tU8*)(_buf_))[18]<<8)|(((tU8*)(_buf_))[19]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__FEC_BER(_buf_) ((((tU8*)(_buf_))[20]<<8)|(((tU8*)(_buf_))[21]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__RFU2(_buf_) ((((tU8*)(_buf_))[22]<<8)|(((tU8*)(_buf_))[23]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_DATA_SIZE(_buf_) ((((tU8*)(_buf_))[24]<<8)|(((tU8*)(_buf_))[25]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_DATA(_buf_) (&((_buf_)[DABDEV_DCSR_MS_NOTIFY_CHANNEL__HEADER_LEN]))

#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__WR__CHANNEL_TYPE(_buf_,_value_) \
	(((tU8*)(_buf_))[4])=(((_value_)<<4)&0xf0)|((((tU8*)(_buf_))[4])&0x0f)

#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__WR__CHANNEL_DATA_SIZE(_buf_,_value_) \
	(((tU8*)(_buf_))[24])=((_value_)>>8); \
	(((tU8*)(_buf_))[25])=((_value_)>>0);
#else
/* new format */
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__HEADER_LEN (2+1+1+32)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL_LEN(_subch_size_)  (DABDEV_DCSR_MS_NOTIFY_CHANNEL__HEADER_LEN+_subch_size_)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE(_buf_)    ((((tU8*)(_buf_))[4]>>4)&0xf)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__RFU(_buf_)    ((((tU8*)(_buf_))[4]>>3)&0x1)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__DRC_SCALING(_buf_)    ((((tU8*)(_buf_))[4]>>0)&0x7)
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__SUBCH_ID(_buf_)    ((tU8*)(_buf_))[5]
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__SUB_ID(_buf_) ((((tU8*)(_buf_))[6]<<8)|(((tU8*)(_buf_))[7]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_FLAGS(_buf_) ((((tU8*)(_buf_))[8]<<8)|(((tU8*)(_buf_))[9]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_CNT(_buf_) ((((tU8*)(_buf_))[10]<<8)|(((tU8*)(_buf_))[11]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_RECONF(_buf_) ((((tU8*)(_buf_))[12]<<8)|(((tU8*)(_buf_))[13]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_CONF(_buf_) ((((tU8*)(_buf_))[14]<<8)|(((tU8*)(_buf_))[15]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__FIC_BER(_buf_) ((((tU8*)(_buf_))[16]<<8)|(((tU8*)(_buf_))[17]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__MSC_BER(_buf_) ((((tU8*)(_buf_))[18]<<8)|(((tU8*)(_buf_))[19]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__FEC_BER(_buf_) ((((tU8*)(_buf_))[20]<<8)|(((tU8*)(_buf_))[21]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__RFU2(_buf_) ((((tU8*)(_buf_))[22]<<8)|(((tU8*)(_buf_))[23]<<0))
     /* DABDEV_DCSR_MS_NOTIFY_CHANNEL__CONCEALMENT_LEVEL ... */
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_DATA_SIZE(_buf_) ((((tU8*)(_buf_))[34]<<8)|(((tU8*)(_buf_))[35]<<0))
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_DATA(_buf_) (&((_buf_)[DABDEV_DCSR_MS_NOTIFY_CHANNEL__HEADER_LEN]))

#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__WR__CHANNEL_TYPE(_buf_,_value_) \
    (((tU8*)(_buf_))[4])=(((_value_)<<4)&0xf0)|((((tU8*)(_buf_))[4])&0x0f)

#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__WR__CHANNEL_DATA_SIZE(_buf_,_value_) \
    (((tU8*)(_buf_))[34])=((_value_)>>8); \
    (((tU8*)(_buf_))[35])=((_value_)>>0);
#endif

#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE__DAB_SUBCHANNEL		 1
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE__DABPLUS_SUPERFRAME	 2
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE__DAB_MPEG2TS         3
#define DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE__PAD_FRAME		     4

#define MSC_BUFFER_NUMB_PER_CHANNEL       16                              /*16 buffers per subchannel, this value must be 2^n*/
#define MSC_BUFFER_NUMB_PER_DABDEV        (MSC_BUFFER_NUMB_PER_CHANNEL*4) /*16 buffer for 4 channels each*/
#define MSC_BUFFER_SIZE                   ((384*3)+DABDEV_DCSR_MS_NOTIFY_CHANNEL__HEADER_LEN)

#ifdef CONFIG_SOURCE_OF_PAD_STREAMDECODER
#define PAD_BUFFER_NUMB_PER_DABDEV           (8*4) /*8 buffer for 4 channels each*/
#define PAD_BUFFER_SIZE                    ((198)+DABDEV_DCSR_MS_NOTIFY_CHANNEL__HEADER_LEN)
#endif

int LLD_DABIRQCallback(void * param);
tS32 LLD_vInitDABIrq( tU32 * result, tU32 * value, LLD_DABIRQ_IdTy DABIRQ_ID );
tS32 LLD_s32DABIRQ_SetCallback( LLD_DABIRQ_IdTy DABIRQ_ID , OSAL_irqCallback DABIRQfunc, tVoid * Param );
tS32 LDD_s32DABIRQ_InitAllDABIrqs( void );

tVoid DABDEV_vIOHardwareInit(tVoid);
tVoid DABDEV_vIOInit(tVoid);

void  dabdev_send_notification(tU8 device_id, tPCU8 ps8BufferPar, tU32 u32LengthPar);
void dabdev_dcsr_response_ready(tU8 device_id);

uint8* dabdev_get_mem_blk_ptr(tU8 req_id, tU8 subchid, tU32* mem_size);
uint8* dabdev_lock_mem_blk_ptr(tU8 req_id, tU8* buffer_ptr);
uint8* dabdev_unlock_mem_blk_ptr(tU8 req_id, tU8* buffer_ptr);

void dabdev_data_transport_ready(tU8 req_id, tU8 subchid, tU8 cif_diff, tU16 num_bytes, uint8* buffer);

uint8* dabdev_get_FIC_mem_blk_ptr(tU8 device_id, tU32* mem_size);
void dabdev_FIC_data_transport_ready(tU8 device_id, tU8 cif_diff, tU16 num_bytes, uint8* buffer);

tS32 dabdev_please_provide_buffers_soon(tU8 device_id, tU8 subch_id, tU16 num_bytes); //, tU8 num_buffers);
tS32 dabdev_dont_need_buffers_anymore(tU8 device_id, tU8 subch_id);

#ifdef CONFIG_TARGET_DEV_DABDEVICE_TEST__ENABLE
void DABDEV_handle_DCSR_MS_CMD_ADJUST_AUDIO_SAMPLING_RATE( tU8 out_interface_id, tS8 fs_increment, tU8 reset_flag );
#endif

#ifdef __cplusplus
}
#endif

#else
#error dabdev.h included several times
#endif
