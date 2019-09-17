//!
//!  \file 		etalcmd.c
//!  \brief 	<i><b> ETAL command protocol layer </b></i>
//!  \details   The ETAL command protocol layer implements the command protocol
//! 			specific to each device controlled by ETAL (Tuner, DCOP).
//! 
//! 			This file contains the utilities used to decide to which device
//! 			a command should be routed.
//!  $Author$
//!  \author (original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

/*****************************************************************
| Local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*****************************************************************
| prototypes
|----------------------------------------------------------------*/
/***************************
 *
 * ETAL_BcastStandardToIndex
 *
 **************************/
/*!
 * \brief		Returns the index into the Command Routing Tables given the Broadcast Standard
 * \details		
 * \remark		The entries in the Command Routing Tables must follow the order
 * 				specified in this function; this requirement is not enforced
 * 				by the code so care must be take when changing or adding new
 * 				tables.
 * \param[in]	std - the Broadcast Standard
 * \return		The Command Routing Table index, or 0 in case of error.
 * \callgraph
 * \callergraph
 */
static tU32 ETAL_BcastStandardToIndex(EtalBcastStandard std)
{
	switch (std)
	{
		case ETAL_BCAST_STD_DAB:
			return 1;
		case ETAL_BCAST_STD_DRM:
			return 2;
		case ETAL_BCAST_STD_FM:
			return 3;
		case ETAL_BCAST_STD_AM:
			return 4;
		case ETAL_BCAST_STD_HD_FM:
			return 5;
		case ETAL_BCAST_STD_HD_AM:
			return 6;
		case ETAL_BCAST_STD_UNDEF:
		default:
			ASSERT_ON_DEBUGGING(0);
			return 0;
	}
}
/***************************
 *
 * ETAL_cmdRouting_GetTune
 *
 **************************/
/*!
 * \brief		Returns the Routing information for Tune-type commands
 * \param[in]	std - the Broadcast Standard to which the command applies
 * \return		The Command Routing as a combination of #EtalDeviceType values
 * \callgraph
 * \callergraph
 */
static tU32 ETAL_cmdRouting_GetTune(EtalBcastStandard std)
{
	tU32 index;

	index = ETAL_BcastStandardToIndex(std);
	return etalFrontendCommandRouting_Tune[index];
}

/***************************
 *
 * ETAL_cmdRouting_GetQuality
 *
 **************************/
/*!
 * \brief		Returns the Routing information for Tune-type commands
 * \param[in]	std - the Broadcast Standard to which the command applies
 * \return		The Command Routing as a combination of #EtalDeviceType values
 * \callgraph
 * \callergraph
 */
static tU32 ETAL_cmdRouting_GetQuality(EtalBcastStandard std)
{
	tU32 index;

	index = ETAL_BcastStandardToIndex(std);
	return etalFrontendCommandRouting_Quality[index];
}

/***************************
 *
 * ETAL_cmdRouting_GetBandSpecific
 *
 **************************/
/*!
 * \brief		Returns the Routing information for band-specific commands
 * \param[in]	std - the Broadcast Standard to which the command applies
 * \return		The Command Routing as a combination of #EtalDeviceType values
 * \callgraph
 * \callergraph
 */
static tU32 ETAL_cmdRouting_GetBandSpecific(EtalBcastStandard std)
{
	tU32 index;

	index = ETAL_BcastStandardToIndex(std);
	return etalFrontendCommandRouting_BandSpecific[index];
}

/***************************
 *
 * ETAL_cmdRouting_GetRDS
 *
 **************************/
/*!
 * \brief		Returns the Routing information for RDS commands
 * \remark		RDS commands are always processed by the same device,
 * 				independent of the current Broadcast standard
 * 				(if different from FM it is an error).
 * \return		The Command Routing as a combination of #EtalDeviceType values
 * \callgraph
 * \callergraph
 */
static tU32 ETAL_cmdRouting_GetRDS(tVoid)
{
	/*
	 * RDS commands are always processed by the same device, independent of the
	 * current Broadcast standard (if different from FM it is an error)
	 */
	return etalFrontendCommandRouting_RDS;
}

/***************************
 *
 * ETAL_cmdRoutingCheck
 *
 **************************/
/*!
 * \brief		Checks which devices shall process a given command type
 * \details		Returns a bitmap of #EtalDeviceType describing to which devices 
 * 				command of the specified type should be sent. The function uses
 * 				predefined tables and selects the appropriate one based on the
 * 				hReceiver's broadcast standard.
 * \param[in]	hReceiver - handle of the receiver that shall process the command
 *    			            The handle is used to detect the Broadcast standard
 * \param[in]	cmd       - The type of command to be sent to *hReceiver*
 * \return		A bitmap of #EtalDeviceType values describing which devices should
 * 				process the command type, or 0 in case of error (i.e. undefined standard, undefined command type)
 * \callgraph
 * \callergraph
 */
tU32 ETAL_cmdRoutingCheck(ETAL_HANDLE hReceiver, etalCommandTy cmd)
{
	EtalBcastStandard std;

	ASSERT_ON_DEBUGGING(cmd < commandSize);

	if ((cmd == commandBandSpecific) || (cmd == commandTune) || (cmd == commandQuality))
	{
		std = ETAL_receiverGetStandard(hReceiver);
	}
	else if (cmd == commandRDS)
	{
		/* For commandRDS it is implicitly known that the standard is FM, so get to the table directly */
		return ETAL_cmdRouting_GetRDS();
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Undefined command type %d)", cmd);
		return 0;
	}

	switch (std)
	{
		case ETAL_BCAST_STD_DAB:
		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
			if (cmd == commandTune)
			{
				return ETAL_cmdRouting_GetTune(std);
			}
			else if (cmd == commandQuality)
			{
				return ETAL_cmdRouting_GetQuality(std);
			}
			return ETAL_cmdRouting_GetBandSpecific(std);
		default:
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "ETAL_cmdRoutingCheck : Unsupported broadcast standard (%d), receiver %d, cmd %d", std, hReceiver, cmd);
			return 0;
	}

}

