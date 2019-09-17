// _____________________________________________________________________________
//| FILE:         lld.h
//| PROJECT:      ADR3 - STA660
//| SW-COMPONENT:
//|_____________________________________________________________________________
//| DESCRIPTION:  Include all basic interfaces for the LLD
//|_____________________________________________________________________________
//| COPYRIGHT:    (c) 2009 STMicroelectronics, Agrate (ITALY)
//| HISTORY:
//| Date      | Modification               | Author
//|_____________________________________________________________________________
//| 20090706  | Initial revision           | A. Saviotti
//|_____________________________________________________________________________

//!
//!  \file 		lld.h
//!  \brief 	<i><b> Include all basic interfaces for the LLD </b></i>
//!  \author 	Alberto Saviotti
//!  \author 	(original version) Luca Pesenti
//!  \version 	1.0
//!  \date 		2009.07.06
//!  \bug 		Unknown
//!  \warning 	This header shall be included by all LLDs
//!

#ifndef _LLD_H_
#define _LLD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "defines.h"

#if defined (PLATFORM_IS_STA662)
#include "api_sta662.h"
#else
#include "api.h"
#endif

#include "macro.h"

#if defined (PLATFORM_IS_CARTESIO)
	#include "mapping_sta2062.h"
	#include "registers_sta206x.h"
#elif defined (PLATFORM_IS_CARTESIOPLUS)
	#include "mapping_sta2065.h"
	#include "registers_sta206x.h"
#elif (defined (PLATFORM_IS_ADR3) || defined(CONFIG_TARGET_SYS_ACCORDO2) || defined(CONFIG_TARGET_SYS_ACCORDO5))
	#include "mapping_sta660.h"
	#include "registers_sta660.h"
#endif // #if defined (PLATFORM_IS_CARTESIO)

#ifdef __cplusplus
}
#endif

#endif  // _LLD_H

// End of file
