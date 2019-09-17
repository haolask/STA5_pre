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

#ifndef _LLD_EXPORT_H_
#define _LLD_EXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

// LIST OF ALL DEVICES
//-----------------------------------------------------------------------
// low level driver
//-----------------------------------------------------------------------
#include "lld.h"
#include "lld_dma.h"
#include "lld_uart.h"
#include "lld_vic.h"
#include "lld_ssp.h"
#ifdef PLATFORM_IS_ADR3
   #include "lld_gpio_sta660.h"
   #include "lld_i2c_sta660.h"
   #include "lld_esai.h"
   #include "lld_aif.h"
   #include "lld_smu.h"
   #include "lld_spi.h"
   #include "lld_rds.h"
   #include "lld_slink.h"
   #include "lld_mtu.h"   
   #include "lld_sdmmc_ssp_sta66x.h"
#elif defined (PLATFORM_IS_CARTESIOPLUS) || defined (PLATFORM_IS_CARTESIO)
   #include "lld_gpio_sta206x.h"
   #include "lld_i2c_sta206x.h"
   #include "lld_sdi_sta206x.h"
   #include "lld_msp.h"
#elif defined(CONFIG_TARGET_SYS_ACCORDO2) || defined(CONFIG_TARGET_SYS_ACCORDO5)

#else
   #error No correct platform included in OSALIO
#endif

#ifdef __cplusplus
}
#endif

#endif  // _LLD_H

// End of file
