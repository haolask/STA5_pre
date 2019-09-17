//!
//!  \file		cmost_protocol.h
//!  \brief		<i><b> CMOST driver </b></i>
//!  \details	CMOST protocol
//!  $Author$
//!  \author	(original version) Maurizio Tonella
//!  $Revision$
//!  $Date$
//!

#ifndef CMOST_PROTOCOL_H
#define CMOST_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

///
// Exported functions
///
#ifdef CONFIG_COMM_DRIVER_EMBEDDED
tUInt CMOST_ProtocolHandle (tVoid *pParams, tU8 *DataPnt, tS32 *BytesNum);
#else
	#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	tUInt CMOST_ProtocolHandle (tVoid *pParams);
	#else
	tUInt __stdcall CMOST_ProtocolHandle (tVoid *pParams);
	#endif
#endif

extern tS32 CMOST_SendMessageStart(tVoid *memoryPtr, tU8 *DataToSend, tU8 type, tU8 accessSize, tU8 busAddress, tU8 busOptions, tU32 BytesNumber);

#ifdef __cplusplus
}
#endif

#endif // CMOST_PROTOCOL_H

// End of file
