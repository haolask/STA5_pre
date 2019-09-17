//!
//!  \file 		cmost_dump.c
//!  \brief 	<i><b> CMOST module debug dump </b></i>
//!  \details   Debug dump functions for the CMOST driver
//!  $Author$
//!  \author 	(original version) Raffaele Belardi, Roberto Allevi
//!  $Revision$
//!  $Date$
//!

#include "target_config.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_CMOST)

#include "osal.h"
#include "cmost_dump.h"

#ifdef CMOST_DEBUG_DUMP_TO_FILE
FILE *CMOST_DebugDump_fd;
#endif

/**************************************
 *
 * CMOST_StartDump
 *
 *************************************/
/*!
 * \brief		Starts dumping the CMOST firmware data to a file
 * \details		The function instructs the low level driver (SPI or I2C) of the
 * 				CMOST to write to a file all the firmware data being transferred
 * 				to the CMOST. This is useful only during initial TUNER DRIVER system
 * 				debug, not recommended for normal operation.
 * \remark		For this function to work #CMOST_DEBUG_DUMP_TO_FILE must be defined
 * \see			
 * \callgraph
 * \callergraph
 */
tVoid CMOST_StartDump(tVoid)
{
#ifdef CMOST_DEBUG_DUMP_TO_FILE
	CMOST_DebugDump_fd = fopen(CMOST_DEBUG_DUMP_FILENAME, "w");
#endif
}

/**************************************
 *
 * CMOST_StopDump
 *
 *************************************/
/*!
 * \brief		Stops dumping the CMOST firmware data to a file
 * \remark		For this function to work #CMOST_DEBUG_DUMP_TO_FILE must be defined
 * \see			CMOST_StartDump
 * \callgraph
 * \callergraph
 */
tVoid CMOST_StopDump(tVoid)
{
#ifdef CMOST_DEBUG_DUMP_TO_FILE
	fclose(CMOST_DebugDump_fd);
	CMOST_DebugDump_fd = NULL;
#endif
}


#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_ETAL_SUPPORT_CMOST

//EOF

