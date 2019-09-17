//!
//!  \file 		tunerdriver.h
//!  \brief 	<i><b> CMOST driver API layer </b></i>
//!  \details   External interface implementation for CMOST driver.
//!				Details on these functions can be found also in the TUNER_DRIVER_Specification.pdf
//!				document.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#ifndef TUNERDRIVER_H
#define TUNERDRIVER_H

#include "tunerdriver_types.h"


/**************************************
 * Types
 *************************************/
 /*!
  * \enum	BootTunerTy
  * 		CMOST types known to the TUNER DRIVER boot download code.
  * 		This type is used as parameter to #TUNERDRIVER_download_CMOST
  * 		but note that ETAL or TUNER DRIVER must be built
  * 		with support for the requested CMOST type otherwise
  * 		the call will fail.
  */
typedef enum
{
	/*! Undefined, reserved for uninitialized or error code */
	BOOT_TUNER_UNKNOWN,
	/*! STAR flavour, dual channel */
	BOOT_TUNER_STAR_T,
	/*! STAR flavour, single channel */
	BOOT_TUNER_STAR_S,
	/*! DOT flavour, dual channel */
	BOOT_TUNER_DOT_T,
	/*! DOT flavour, single channel */
	BOOT_TUNER_DOT_S,

	/*! Reserved for trace usage */
	BOOT_TUNER_TY_NB
} BootTunerTy;

/**************************************
 * Functions
 *************************************/
tSInt TUNERDRIVER_system_init(tVoid);
tSInt TUNERDRIVER_setTcpIpAddress(tChar *IPAddress);
tSInt TUNERDRIVER_system_deinit(tVoid);
tSInt TUNERDRIVER_init(tU32 deviceID, tyCMOSTDeviceConfiguration *deviceConfiguration);
tSInt TUNERDRIVER_deinit(tU32 deviceID);
tSInt TUNERDRIVER_reset_CMOST(tU32 deviceID);
tSInt TUNERDRIVER_download_CMOST(tU32 deviceID, BootTunerTy boot_type, tU8 *image, tU32 image_size, tBool load_default_params);
tSInt TUNERDRIVER_testRead_CMOST(tU32 deviceID);
tSInt TUNERDRIVER_readRaw32_CMOST(tU32 deviceID, tU32 address, tU8 *resp);
tSInt TUNERDRIVER_writeRaw32_CMOST(tU32 deviceID, tU32 address, tU32 value);
tSInt TUNERDRIVER_writeRawBlock_CMOST(tU32 deviceID, tU32 address, tU8 *buf, tU32 size);
tSInt TUNERDRIVER_sendCommand_CMOST(tU32 deviceID, tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);

#endif // TUNERDRIVER_H
