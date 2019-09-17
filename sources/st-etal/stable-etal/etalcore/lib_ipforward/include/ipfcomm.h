//!
//!  \file 		ipfcomm.h
//!  \brief 	<i><b> IPFORWARD communication primitives </b></i>
//!  \details   Definitions needed to support internal communication between IPForward thread and ETAL thread
//!  \author 	Raffaele Belardi
//!

#define PROTOCOL_LAYER_INTERNAL_BUFFER_LEN        ETAL_MAX_RESPONSE_LEN
#define PROTOCOL_LAYER_TO_DEVICE_BUFFER_LEN       1024 
/*
 * Max time to wait for the Protocol Layer to come up
 * during connection setup
 * Units: msec
 */
#define PROTOCOL_LAYER_CONNECT_TIMEOUT            1000
/*
 * How often to check for the Protocol Layer connection
 * during connection setup
 * Units: msec
 */
#define PROTOCOL_LAYER_CONNECT_TEST_SCHEDULING      10

/* IPForward definitions specific for the communication to DCOP */
#define TCP_IP_CLIENT_PORT_NUMBER_DCOP            23000
#define LOG_MASK_DCOP                             0x1F
#define LOG_FILENAME_DCOP          (tChar *)"DCOP_protocol_log.txt"

/* IPForward definitions specific for the communication to CMOST */
#define TCP_IP_CLIENT_PORT_NUMBER_CMOST           24000
#define LOG_MASK_CMOST                            0x1F
#define LOG_FILENAME_CMOST         (tChar *)"CMOST_protocol_log.txt"

#define PROTOCOL_LAYER_INDEX_INVALID  (-1)
#define IPF_DEINIT_ALL                (-2)

/*!
 * \def		IPF_tracePrintFatal
 * 			Wrapper for #OSALUTIL_s32TracePrintf. It maps to OSALUTIL_s32TracePrintf if
 * 			the trace level is TR_LEVEL_FATAL or greater, to null operation otherwise.
 * \remark	To be used only for TR_CLASS_APP_IPFORWARD
 */
#if defined(CONFIG_TRACE_CLASS_IPFORWARD) && (CONFIG_TRACE_CLASS_IPFORWARD >= TR_LEVEL_FATAL)
	#define IPF_tracePrintFatal(mclass, ...) do { OSALUTIL_s32TracePrintf(0, TR_LEVEL_FATAL, mclass, __VA_ARGS__); } while (0)
#else
	#define IPF_tracePrintFatal(mclass, ...) do { } while (0)
#endif

/*!
 * \def		IPF_tracePrintError
 * 			Wrapper for #OSALUTIL_s32TracePrintf. It maps to OSALUTIL_s32TracePrintf if
 * 			the trace level is TR_LEVEL_ERRORS or greater, to null operation otherwise.
 * \remark	To be used only for TR_CLASS_APP_IPFORWARD
 */
#if defined(CONFIG_TRACE_CLASS_IPFORWARD) && (CONFIG_TRACE_CLASS_IPFORWARD >= TR_LEVEL_ERRORS)
	#define IPF_tracePrintError(mclass, ...) do { OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, mclass, __VA_ARGS__); } while (0)
#else
	#define IPF_tracePrintError(mclass, ...) do { } while (0)
#endif

/*!
 * \def		IPF_tracePrintSysmin
 * 			Wrapper for #OSALUTIL_s32TracePrintf. It maps to OSALUTIL_s32TracePrintf if
 * 			the trace level is TR_LEVEL_SYSTEM_MIN or greater, to null operation otherwise.
 * \remark	To be used only for TR_CLASS_APP_IPFORWARD
 */
#if defined(CONFIG_TRACE_CLASS_IPFORWARD) && (CONFIG_TRACE_CLASS_IPFORWARD >= TR_LEVEL_SYSTEM_MIN)
	#define IPF_tracePrintSysmin(mclass, ...) do { OSALUTIL_s32TracePrintf(0, TR_LEVEL_SYSTEM_MIN, mclass, __VA_ARGS__); } while (0)
#else
	#define IPF_tracePrintSysmin(mclass, ...) do { } while (0)
#endif

/*!
 * \def		IPF_tracePrintComponent
 * 			Wrapper for #OSALUTIL_s32TracePrintf. It maps to OSALUTIL_s32TracePrintf if
 * 			the trace level is TR_LEVEL_COMPONENT or greater, to null operation otherwise.
 * \remark	To be used only for TR_CLASS_APP_IPFORWARD
 */
#if defined(CONFIG_TRACE_CLASS_IPFORWARD) && (CONFIG_TRACE_CLASS_IPFORWARD >= TR_LEVEL_COMPONENT)
	#define IPF_tracePrintComponent(mclass, ...) do { OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, mclass, __VA_ARGS__); } while (0)
#else
	#define IPF_tracePrintComponent(mclass, ...) do { } while (0)
#endif

extern tS32 ProtocolLayerIndexCMOST;
extern tS32 ProtocolLayerIndexDCOP;

typedef struct {
        tU8   CtrlAppSync;
        tU8   CtrlAppLunId;
        tU8   CtrlAppSpare_0;
        tU8   CtrlAppSpare_1;
        tU8   CtrlAppSpare_2;
        tU8   CtrlAppSpare_3;
        tU16  CtrlAppCmdDataLen;
} CtrlAppMessageHeaderType; 

tVoid ForwardPacketToCtrlAppPort (tS32 index, tU8 *DataPnt, tU32 BytesNum, tU8 SyncByte, tU8 LunId, tU8 h0, tU8 h1, tU8 h2, tU8 h3);
tSInt ProtocolLayer_FifoPop(tS32 index, tU8 *data, tU32 max_data, tU32 *len, CtrlAppMessageHeaderType *head);
tVoid ipforwardPowerUp(tVoid);
tSInt ipforwardInit(tChar *IPaddress, tU32 port, tU32 log_mask, tChar *logfile, tS32 *index);
tSInt ipforwardDeinit (tS32 index);
tBool TcpIpProtocolIsConnected (tS32 index);
tSInt CheckCtrlAppMessage_CMOST(tU8 cmd, tS32 index, CtrlAppMessageHeaderType *CtrlAppMessageHeader);

