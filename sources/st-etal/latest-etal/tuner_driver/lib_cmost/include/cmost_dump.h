//!
//!  \file    cmost_dump.h
//!  \brief 	<i><b> CMOST module debug dump </b></i>
//!  \details   This file contains macros to print messages to the output.
//!				The macros are defined so that if the TR_LEVEL is below
//!				a threshold the trace code is not even included in the build.
//!				The macros map to the OSAL OSALUTIL_s32TracePrintf function.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi, Roberto Allevi
//!  $Revision$
//!  $Date$
//!

#ifndef CMOST_DUMP_H
#define CMOST_DUMP_H

/*!
 * \def		CMOST_DEBUG_DUMP_TO_FILE
 * 			If defined, writes to #CMOST_DEBUG_DUMP_FILENAME all the **data**
 * 			sent to the CMOST during the Firmware download (not the addresses).
 */
#undef CMOST_DEBUG_DUMP_TO_FILE

/*!
 * \def		CMOST_DEBUG_DUMP_FILENAME
 * 			Name of the file where the CMOST data will be dumped to.
 */
#define CMOST_DEBUG_DUMP_FILENAME "cmost_fw.bin"

#ifdef CMOST_DEBUG_DUMP_TO_FILE
	extern FILE *CMOST_DebugDump_fd;
#endif

tVoid CMOST_StartDump(tVoid);
tVoid CMOST_StopDump(tVoid);

#endif // CMOST_DUMP_H

// End of file
