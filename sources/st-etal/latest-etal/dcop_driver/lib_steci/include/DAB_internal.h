//!
//!  \file 		dab_internal.h
//!  \brief 	<i><b> Internal interface for DAB DCOP driver. </b></i>
//!  \details   Internal interface for DAB DCOP driver.
//!  $Author$
//!  \author 	(original version) Jean-Hugues Perrin
//!  $Revision$
//!  $Date$
//!
#ifndef DAB_INTERNAL_H
#define DAB_INTERNAL_H

/***********************************
 *
 * Macros
 *
 **********************************/
#define MAX_DAB_DCOP_DEVICE                     1
#define DAB_DCOP_ID_0                           0

#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	#define DAB_EXTERNAL_DRIVER_TYPE_COMMAND    (tU8)0
	#define DAB_EXTERNAL_DRIVER_TYPE_RESET      (tU8)7
	#define DAB_EXTERNAL_DRIVER_RESERVED        (tU8)0
#endif

/***********************************
 *
 * Prototype
 *
 **********************************/
tyCommunicationBusType DAB_GetBusType(tU32 deviceID);
tU32 DAB_GetGPIOReset(tU32 deviceID);
tU32 DAB_GetGPIOReq(tU32 deviceID);
tU32 DAB_GetGPIOBoot(tU32 deviceID);
tVoid *DAB_GetCommunicationBusConfig(tU32 deviceID);

#endif // DAB_INTERNAL_H
