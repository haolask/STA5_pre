//!
//!  \file      fic_common.h
//!  \brief     <i><b> Header file for FIC common data </b></i>
//!  \details   This file is the FIC common data.
//!  \author    Alberto Saviotti
//!  \author    (original version) Alberto Saviotti
//!  \version   1.0
//!  \date      20111021
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef __FIC_COMMON_H__
#define __FIC_COMMON_H__

// Apps
#define DABMW_LANDSCAPE_SUPPORTED_APP           (tU8)2

// Storage space related definition
#if defined (CONFIG_DABMW_RAM_STORED_ENSEMBLE_NUMBER)
    #define DABMW_FIC_MAX_ENSEMBLE_NUMBER           (tU16)CONFIG_DABMW_RAM_STORED_ENSEMBLE_NUMBER
#else
    #define DABMW_FIC_MAX_ENSEMBLE_NUMBER           (tU16)24
#endif // #if defined (CONFIG_DABMW_RAM_STORED_ENSEMBLE_NUMBER)

#if defined (CONFIG_DABMW_NVM_STORED_ENSEMBLE_NUMBER)
    #define DABMW_FIC_NVM_MAX_ENSEMBLE_NUMBER       (tU16)CONFIG_DABMW_NVM_STORED_ENSEMBLE_NUMBER
#else
    #define DABMW_FIC_NVM_MAX_ENSEMBLE_NUMBER       (tU16)12
#endif // #if defined (CONFIG_DABMW_NVM_STORED_ENSEMBLE_NUMBER)

// We link the number of monitore ensemble to the number of possible tuned ones
#define DABMW_FIC_NVM_MAX_NEXTCONF_ENSEMBLE_NUM ((tU16)DABMW_SOURCE_DABDEVICE_NUMBER)

// Maximum number of services inside a ensemble
#define DABMW_FIC_MAX_SERVICES_NUMBER           (tU8)32

#define DABMW_INVALID_PACKET_ADDRESS            (tU16)0xFFFF
#define DABMW_PADDING_PACKET_ADDRESS            (tU16)0x0
/* padding_packets don't get processed by the packet decoder, so
 * it is safe to use the address 0 for a different purpose
 * that is to identify audio subchannels in the packet decoder
 * (for X-PAD Data Group processing) */
#define DABMW_AUDIO_PACKET_ADDRESS              DABMW_PADDING_PACKET_ADDRESS

#define DABMW_INVALID_SUBCH_ID                  (tU8)0xFF

#define DABMW_INVALID_FEC_SCHEME                0xFF

// Maximum number of sub-channels inside a ensemble
#define DABMW_MAX_SUBCH_NUM                     (tU8)64

// maximum number of service components for service
#define DABMW_FIC_MAX_SC_NUMBER                 (tU16)32 // Here we are on the ensemble level

#define DABMW_FIC_MAX_SC_NUMBER_PER_SERVICE     (tU16)12 // This number is determined by DAB standard ETSI EN 300 401 V1.4.1 (2006-06) pg. 54

#define DABMW_INVALID_SC_ID                     (tU16)0xFFFF

#define DABMW_MAGIC_SC_ID                       (tU16)0x8000

#define DABMW_INVALID_GLOBAL_SERVICE_COMPONENT  (tU8)0xFF
#define DABMW_INVALID_SERVICE_COMPONENT         (tU16)0xFFFF

// Maximum number of sub-channels inside a ensemble
#define DABMW_MAX_FIDC_NUM                      (tU8)8

// Maximum number of packet addresses per subch in packet mode (from (ETSI)
#define DABMW_MAX_SC_PER_SUBCH                  (tU16)1024

// Max size of the Network Packet data (including padding)
#define DABMW_NETWORK_PACKET_DATA_SIZE          91
// Max size of the whole Network Packet including header and CRC bytes
#define DABMW_NETWORK_PACKET_SIZE               (3 + DABMW_NETWORK_PACKET_DATA_SIZE + 2)

// Max size of the data in a Packet mode DataGroup (from ETSI)
// The data group can contain up to 8191 bytes, plus possibly one of
// the two CRC bytes, so 8192.
#define DABMW_DATA_GROUP_DATA_SIZE              (tU16)8192

// Max size of the header of a DataGroup (from ETSI)
#define DABMW_DATA_GROUP_HEADER_MAX_SIZE        (tU16)22

// Max number of DataGroup type (from ETSI, 4 bits)
#define DABMW_PACKET_MAX_DATA_GROUP_TYPE        (tU8)16

// Stored FIG types
#define DABMW_MAX_STORED_FIG_TYPES              (tU8)7

// Labels related definitions
#define DABMW_LABEL_LENGTH                      (tU8)16
#define DABMW_EXTENDED_LABEL_LENGTH             (tU8)64
#define DABMW_LANDSCAPE_MEM_POOL_MAX_EXT_LABEL  32

// Specific FIGs defines
#define DABMW_FIG_0_EXT_6_MAX_NUM_IDS           ((tU8)12)

// FIG 0/9
// Extended Field is max 25 bytes, in the worst case
// (Number of Services == 1, P/D == 0) each Sub-field is 4 bytes
#define DABMW_FIG_0_EXT_9_SUPPORTED_SUB_FIELDS  6
// Per spec at most 3 services per Sub-field
#define DABMW_FIG_0_EXT_9_SUPPORTED_SID         3

// FIG 0/11
// Each SubId is 5 bit long; SubIdList is max 23 bytes long
// so the max number of SubId in each TransmitterGroup is
// (23 * 8) / 5 = 36
#define DABMW_SUBID_NUM                         36
// The minimum length of the Transmitter Group is 3 bytes
// (when "Length of SubId list" = 1).
// The max length for the TII List is 25 bytes.
// Thus the max number of Transmitter Groups per Geographical Area is
// (25 / 3 ) = 8
#define DABMW_TRANSMITTER_GROUP_NUM             8

// FIGs
#define DABMW_FIG_TYPE_0                        ((tU8)0)
#define DABMW_FIG_TYPE_1                        ((tU8)1)
#define DABMW_FIG_TYPE_2                        ((tU8)2)
#define DABMW_FIG_TYPE_5                        ((tU8)5)
#define DABMW_FIG_TYPE_6                        ((tU8)6)

// FIG extensions
#define DABMW_FIG_EXT_0                         ((tU8)0)
#define DABMW_FIG_EXT_1                         ((tU8)1)
#define DABMW_FIG_EXT_2                         ((tU8)2)
#define DABMW_FIG_EXT_3                         ((tU8)3)
#define DABMW_FIG_EXT_4                         ((tU8)4)
#define DABMW_FIG_EXT_5                         ((tU8)5)
#define DABMW_FIG_EXT_6                         ((tU8)6)
#define DABMW_FIG_EXT_8                         ((tU8)8)
#define DABMW_FIG_EXT_9                         ((tU8)9)
#define DABMW_FIG_EXT_10                        ((tU8)10)
#define DABMW_FIG_EXT_11                        ((tU8)11)
#define DABMW_FIG_EXT_13                        ((tU8)13)
#define DABMW_FIG_EXT_14                        ((tU8)14)
#define DABMW_FIG_EXT_16                        ((tU8)16)
#define DABMW_FIG_EXT_17                        ((tU8)17)
#define DABMW_FIG_EXT_18                        ((tU8)18)
#define DABMW_FIG_EXT_19                        ((tU8)19)
#define DABMW_FIG_EXT_21                        ((tU8)21)
#define DABMW_FIG_EXT_22                        ((tU8)22)
#define DABMW_FIG_EXT_24                        ((tU8)24)
#define DABMW_FIG_EXT_25                        ((tU8)25)
#define DABMW_FIG_EXT_27                        ((tU8)27)
#define DABMW_FIG_EXT_28                        ((tU8)28)
#define DABMW_FIG_EXT_31                        ((tU8)31)

// Sub-channel data decoding table (used by FIG 0/1)
#define DABMW_INVALID                           ((tU8)0xFF)

// Regional definition (FIG 0/11)
#define DABMW_SUBID_LEN_MAX_NUMBER              ((tU8)15)
#define DABMW_TII_LIST_LEN_MAX_NUMBER           ((tU8)425)

// Application definition (FIG 0/13)
#define DABMW_USER_APP_MAX_PER_ENSEMBLE         32
#define DABMW_USER_APP_MAX_PER_SC               4
// should be 21 per spec
#define DABMW_USER_APPLICATION_MAX_DATA         8
/* UserApplication types from FIG 0/13 */
#define DABMW_TS101756_USER_APP_NOT_USED        0x01
#define DABMW_TS101756_USER_APP_SLS             0x02
#define DABMW_TS101756_USER_APP_BWS             0x03
#define DABMW_TS101756_USER_APP_TPEG            0x04
#define DABMW_TS101756_USER_APP_EPG             0x07
#define DABMW_TS101756_USER_APP_JOURNALINE     0x44a

// Cluster definition (FIG 0/18)
#define DABMW_CLUSTER_MAX_NUMBER                ((tU8)0x17)

// SubId array definition (FIG 0/22)
#define DABMW_TII_NUM_SUB_FIELD                 4

// OE EnsembleId array (FIG 0/24)
#define DABMW_EID_MAX_NUMBER                    ((tU8)0xC)

// Program Identification definition (FIG 0/27)
#define DABMW_PI_MAX_NUMBER                    ((tU8)0xC)

// FI (frequency information) list definition (FIG 0/21)
#define DABMW_FI_LIST_MAX_NUMBER            8
#define DABMW_FREQ_LIST_MAX_NUMBER          7

// RM (Range & Modulation) types
typedef enum
{
    DABMW_DAB_ENSEMBLE    = 0,
    DABMW_RFU_1           = 1,
    DABMW_RFU_2           = 2,
    DABMW_RFU_3           = 3,
    DABMW_RFU_4           = 4,
    DABMW_RFU_5           = 5,
    DABMW_DRM             = 6,
    DABMW_RFU_7           = 7,
    DABMW_FM_W_RDS        = 8,
    DABMW_FM_WOUT_RDS     = 9,
    DABMW_AM_LWS          = 10,
    DABMW_RFU_11         = 11,
    DABMW_AM_SWS         = 12,
    DABMW_RFU_13         = 13,
    DABMW_AMSS           = 14,
    DABMW_RFU_15         = 15
} DABMW_rmTy;

// FIDC related
#define DABMW_FIDC_TCID(x)                      ((tU8)((x >> 3) & (tU8)0x07))
#define DABMW_FIDC_EXTENSION(x)                 ((tU8)((x >> 0) & (tU8)0x07))

// ID types
typedef enum
{
    DABMW_ID_IS_NO_TYPE                         = 0,
    DABMW_ID_IS_SID_TYPE                        = 1,
    DABMW_ID_IS_SCID_TYPE                       = 2,
    DABMW_ID_IS_SUBCHID_TYPE                    = 3,
    DABMW_ID_IS_FIDCID_TYPE                     = 4,
    DABMW_ID_IS_SCIDS_TYPE                      = 5,
    DABMW_ID_IS_RDS_PI_CODE_TYPE                = 6,
    DABMW_ID_IS_AMFM_NO_RDS_TYPE                = 7,
    DABMW_ID_IS_DATA_SID_TYPE                   = 8,
	DABMW_ID_IS_DRM_AMSS_TYPE                   = 9
} DABMW_idTy;

// Components types
typedef enum
{
    DABMW_COMPONENTTYPE_IS_NO_TYPE              = 0,
    DABMW_COMPONENTTYPE_IS_MSC_STREAM_AUDIO     = 1,
    DABMW_COMPONENTTYPE_IS_MSC_STREAM_DATA      = 2,
    DABMW_COMPONENTTYPE_IS_FIDC                 = 3,
    DABMW_COMPONENTTYPE_IS_MSC_PACKET_DATA      = 4
} DABMW_componentTypeTy;

// FIG 5 related defines
#define DABMW_PAGING_MESSAGE_MAX_LENGTH         30
#define DABMW_PAGING_FIFO_SIZE                  4
#define DABMW_TMC_SERVICE_MAX_NUMBER            8
#define DABMW_TMC_MESSAGE_MAX_LENGTH            5
#define DABMW_TMC_FIFO_SIZE                     4
#define DABMW_EWS_MESSAGE_MAX_LENGTH            30
#define DABMW_EWS_FIFO_SIZE                     4


// DSCTy table (ref. ETSI TS 101 756 v1.4.1 (2009-07)
typedef enum
{
    DABMW_COMPONENT_TYPE_UNSPECIFIED_DATA__FOREGROUND_SOUND = 0,
    DABMW_COMPONENT_TYPE_TMC__BACKGROUND_SOUND              = 1,
    DABMW_COMPONENT_TYPE_EWS__MULTICHANNEL_AUDIO            = 2,        
    DABMW_COMPONENT_TYPE_ITTS                               = 3,
    DABMW_COMPONENT_TYPE_PAGING                             = 4,
    DABMW_COMPONENT_TYPE_TDC                                = 5,
    DABMW_COMPONENT_TYPE_MPEG2TS                            = 24,
    DABMW_COMPONENT_TYPE_EMBEDDED_IP_PACKET                 = 59,
    DABMW_COMPONENT_TYPE_MOT                                = 60,
    DABMW_COMPONENT_TYPE_PROPRIETARY_NODSCTY                = 61,
    DABMW_COMPONENT_TYPE_NOT_USED_62                        = 62,
    DABMW_COMPONENT_TYPE_NOT_USED_63                        = 63,
    // EPR CHANGE : enum 63 is DABPLUS
    DABMW_COMPONENT_TYPE_DAB_PLUS                           = 63
} DABMW_componentEnumTy;

// Generic Fig macros
#define DABMW_FIC_DECODER_FIG_DATA(_buf_)       (tPU8)&((_buf_)[1])

// FIG 1/3 macro
#define DABMW_NUM_REGION_LABEL_PER_ENSEMBLE     32

#endif // #ifndef __FIC_COMMON_H__

// End of file

