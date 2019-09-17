//!
//!  \file 		hdradio_internal.h
//!  \brief 	<i><b> Internal interface for HDRADIO DCOP driver. </b></i>
//!  \details   Internal interface for HDRADIO DCOP driver.
//!  $Author$
//!  \author 	(original version) Jean-Hugues Perrin
//!  $Revision$
//!  $Date$
//!
#ifndef HDRADIO_INTERNAL_H
#define HDRADIO_INTERNAL_H

/***********************************
 *
 * Macros
 *
 **********************************/
#define HDRADIO_DCOP_ID_0			0

#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	#define HDRADIO_EXTERNAL_DRIVER_TYPE_COMMAND    (tU8)0
	#define HDRADIO_EXTERNAL_DRIVER_TYPE_RESET      (tU8)7
	#define HDRADIO_EXTERNAL_DRIVER_RESERVED        (tU8)0
#endif

/***********************************
 *
 * Prototype
 *
 **********************************/
tyCommunicationBusType HDRADIO_GetBusType(tU32 deviceID);
tU32 HDRADIO_GetGPIOReset(tU32 deviceID);
tVoid *HDRADIO_GetCommunicationBusConfig(tU32 deviceID);

#endif // HDRADIO_INTERNAL_H
