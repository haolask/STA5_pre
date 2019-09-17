//!
//!  \file    steci_helpers.h
//!  \brief   <i><b> STECI helper functions interface file </b></i>
//!  \details STECI helper functionalities interface file.
//!  \author  Alberto Saviotti
//!

#ifndef STECI_HELPERS_H
#define STECI_HELPERS_H

#include "types.h"
#include "defines.h"
#include "utility.h"

#include "steci_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
tSInt ETAL_InitSteciProtocol(tVoid *arg); // not provided for other OSes
tSInt ETAL_DeinitSteciProtocol(tVoid); // not provided for other OSes
#endif //defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)

#ifdef CONFIG_COMM_DRIVER_EMBEDDED
	tSInt STECI_fifoInit(tVoid);
	tVoid STECI_fifoDeinit(tVoid);
	tVoid STECI_fifoPush(tU8 *buf, tU32 len, tU8 lun);
	tS32 STECI_fifoPop(tU8 *buf, tU32 max_buf, tU32 *len, tU8 *lun);
#endif //CONFIG_COMM_DRIVER_EMBEDDED

#ifdef __cplusplus
}
#endif

#endif // STECI_HELPERS_H

// End of file
