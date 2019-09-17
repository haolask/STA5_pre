//!
//!  \file 		etalconfig_cmostm.c
//!  \brief 	<i><b> ETAL hardware description for CMOST Main Board </b></i>
//!  \details   Tables describing the hardware capabilities of the CMOST Main Board (6033-380.1)
//!	 \details	This file supports the following configurations:
//!				- only CMOST module (STAR-T or STAR-S)
//!				- CMOST and DAB DCOP modules
//!				- CMOST and HD DCOP modules
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_BOARD_CMOST_MAIN)

#define ETAL_HDRADIO_CMOSTM_I2C_ADDRESS (tU8)0x2E // TODO not validated

/* SPLINT: turn off locally 'Initializer does not define all elements of a declared array' warnings */
/*@ -initallelements @*/

/*!
 * \var		etalFrontendCommandRouting_Tune
 * 			For every band supported by ETAL define to which Frontend the Tune command
 * 			should be routed. The entries must be sorted in the same order as in the
 * 			#EtalBcastStandard enum.
 */
const tU32 etalFrontendCommandRouting_Tune[ETAL_BCAST_STD_NUMBER] =
{
	/* ETAL_BCAST_STD_UNDEF */ deviceUnknown,
	/* ETAL_BCAST_STD_DAB   */ deviceDCOP | deviceCMOST,
	/* ETAL_BCAST_STD_DRM   */ deviceDCOP | deviceCMOST,
	/* ETAL_BCAST_STD_FM    */ deviceCMOST,
	/* ETAL_BCAST_STD_AM    */ deviceCMOST,
	/* ETAL_BCAST_STD_HD_FM */ deviceDCOP | deviceCMOST,
	/* ETAL_BCAST_STD_HD_AM */ deviceDCOP | deviceCMOST
};

/*!
 * \var		etalFrontendCommandRouting_Quality
 * 			For every band supported by ETAL define to which Frontend the Quality command
 * 			should be routed. The entries must be sorted in the same order as in the
 * 			#EtalBcastStandard enum.
 */
const tU32 etalFrontendCommandRouting_Quality[ETAL_BCAST_STD_NUMBER] =
{
	/* ETAL_BCAST_STD_UNDEF */ deviceUnknown,
	/* ETAL_BCAST_STD_DAB   */ deviceDCOP | deviceCMOST,
	/* ETAL_BCAST_STD_DRM   */ deviceDCOP | deviceCMOST,
	/* ETAL_BCAST_STD_FM    */ deviceCMOST,
	/* ETAL_BCAST_STD_AM    */ deviceCMOST,
	/* ETAL_BCAST_STD_HD_FM */ deviceDCOP | deviceCMOST,
	/* ETAL_BCAST_STD_HD_AM */ deviceDCOP | deviceCMOST
};

/*!
 * \var		etalFrontendCommandRouting_BandSpecific
 * 			Same as #etalFrontendCommandRouting_Tune, but refers to commands
 * 			other than Tune.
 * \see		EtalBcastStandard
 */
const EtalDeviceType etalFrontendCommandRouting_BandSpecific[ETAL_BCAST_STD_NUMBER] =
{
	/* ETAL_BCAST_STD_UNDEF */ 0,
	/* ETAL_BCAST_STD_DAB   */ deviceDCOP,
	/* ETAL_BCAST_STD_DRM   */ deviceDCOP,
	/* ETAL_BCAST_STD_FM    */ deviceCMOST,
	/* ETAL_BCAST_STD_AM    */ deviceCMOST,
	/* ETAL_BCAST_STD_HD_FM */ deviceDCOP,
	/* ETAL_BCAST_STD_HD_AM */ deviceDCOP
};

/*!
 * \var		etalFrontendCommandRouting_RDS
 *			Defines where RDS-related commands should be routed.
 */
const EtalDeviceType etalFrontendCommandRouting_RDS = deviceCMOST;

/*
 * These normally would be inside the board-specific definitions but in current
 * configurations they do not depend on the board so we meve them here
 */
#if defined (CONFIG_MODULE_INDEPENDENT)
	#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	const etalBoardInitFunctionPtrTy etalBoardInitFunctionPtr = ETAL_InitCMOSTtoMDRInterface;
	#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	const etalBoardInitFunctionPtrTy etalBoardInitFunctionPtr = ETAL_InitCMOSTtoHDRADIOInterface;
	#else
	const etalBoardInitFunctionPtrTy etalBoardInitFunctionPtr = NULL;
	#endif
#elif defined (CONFIG_MODULE_INTEGRATED)
	#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	const etalBoardInitFunctionPtrTy etalBoardInitFunctionPtr = ETAL_InitCMOSTtoMDRInterface_MTD;
	#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	const etalBoardInitFunctionPtrTy etalBoardInitFunctionPtr = ETAL_InitCMOSTtoHDRADIOInterface_MTD;
	#else
	const etalBoardInitFunctionPtrTy etalBoardInitFunctionPtr = NULL;
	#endif
#endif

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
const etalDCOPTy etalDCOP =
{
	{
		deviceMDR,
		ETAL_BusSPI, // ignored by BSP
		0,
		2
	},
	0
};
#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
const etalDCOPTy etalDCOP =
{
	{
		deviceHD,
#if defined(CONFIG_COMM_HDRADIO_I2C)
		ETAL_BusI2C,
		ETAL_HDRADIO_CMOSTM_I2C_ADDRESS,
#else // CONFIG_COMM_HDRADIO_SPI
		ETAL_BusSPI, // ignored by BSP
		0,
#endif
		2
	},
	0
};
#else
const etalDCOPTy etalDCOP =
{
	{
		deviceUnknown,
		0,
		0,
		0
	},
	0
};
#endif

#if defined (CONFIG_MODULE_INDEPENDENT)
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
/*!
 * \var		etalTuner
 * 			Describes the Tuners present in the system.
 *
 * 			Tuner handles are statically allocated by ETAL and their value
 * 			corresponds to the index in this array.
 */
etalTunerTy etalTuner[ETAL_CAPA_MAX_TUNER] =
{
	{
		{
			deviceSTART,
			ETAL_BusI2C,
			0xC2,
			2
		},
		{
			{
				ETAL_UNDEF_SLOT,
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
				ETAL_BCAST_STD_DAB | ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
				ETAL_DATA_TYPE_AUDIO | ETAL_DATA_TYPE_DCOP_AUDIO | ETAL_DATA_TYPE_DATA_SERVICE | ETAL_DATA_TYPE_TEXTINFO
#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
				ETAL_BCAST_STD_HD_FM | ETAL_BCAST_STD_HD_AM | ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
				ETAL_DATA_TYPE_AUDIO | ETAL_DATA_TYPE_DCOP_AUDIO | ETAL_DATA_TYPE_TEXTINFO
#else
				ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
				ETAL_DATA_TYPE_AUDIO | ETAL_DATA_TYPE_TEXTINFO
#endif
			},
			{
				ETAL_UNDEF_SLOT,
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
				ETAL_BCAST_STD_DAB | ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
				ETAL_DATA_TYPE_DCOP_AUDIO | ETAL_DATA_TYPE_DATA_SERVICE | ETAL_DATA_TYPE_TEXTINFO
#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
				ETAL_BCAST_STD_HD_FM | ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
				ETAL_DATA_TYPE_DCOP_AUDIO | ETAL_DATA_TYPE_TEXTINFO
#else
				ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
				ETAL_DATA_TYPE_DCOP_AUDIO | ETAL_DATA_TYPE_TEXTINFO
#endif
			}
		}
	}
};

#if defined (CONFIG_ETAL_HAVE_ETALTML)
/*!
 * \var		etalPathPreferences
 * 			Path selection management for ETALTML
 * \todo	add information to manage Tuner selection depending on path
 */
etalPathCapabilitiesTy etalPathPreferences =
{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	7, // nb defined path
#else
	6,
#endif
	{
	// path description 
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
		// DAB_1 is supported only on 1 tuner : channel B of STAR, ie FE_HANDLE_2
		{ ETAL_PATH_NAME_DAB_1, 
		ETAL_BCAST_STD_DAB,
		  1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_2, ETAL_INVALID_HANDLE}}
			}
		},
		// DAB_2 is supported only on 1 tuner : channel A of STAR, ie FE_HANDLE_1
		{ ETAL_PATH_NAME_DAB_2,
		ETAL_BCAST_STD_DAB,
		 1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}}
			}
		},
#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
		// to be updated : for now, HD whatever FE 1 or 2
		{ ETAL_PATH_NAME_HD,
		ETAL_BCAST_STD_HD, /* TODO update PATH to _AM or _FM */
		  2, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}},
				{1, {ETAL_FE_HANDLE_2, ETAL_INVALID_HANDLE}}
			}
		},
#endif
		// FM_FG is supported only on 1 tuner : channel A of STAR, ie FE_HANDLE_1
		// because no audio on channel B
		// But if VPA supported : then 1 & 2.
		{ ETAL_PATH_NAME_FM_FG,
		ETAL_BCAST_STD_FM,
		  2, // nb tuner option
			{
				{2, {ETAL_FE_HANDLE_1, ETAL_FE_HANDLE_2}},
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}}
			}
		},
		// FM_BG  is supported  in theory on 2 tuners : channel A of STAR, ie FE_HANDLE_1 or channel B ie FE_HANDLE_2
		// the VPA is not applicable
		// take channel B as preference...
		// because no audio on channel B
		// But if VPA supported : then 1 & 2.
		{ ETAL_PATH_NAME_FM_BG,
		ETAL_BCAST_STD_FM,
		 2, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_2, ETAL_INVALID_HANDLE}},
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}}
			}
		},
		// AM is supported only on channel A... as 1 path,FG
		{ ETAL_PATH_NAME_AM,
		ETAL_BCAST_STD_AM,
		 1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}}
			}
		},
		// DRM not covered yet
		{ ETAL_PATH_NAME_DRM_1,
		ETAL_BCAST_STD_DRM,
				 1, // nb tuner option
			{
				{1, {ETAL_INVALID_HANDLE, ETAL_INVALID_HANDLE}}
			}
		},
		
		{ ETAL_PATH_NAME_DRM_2,
		ETAL_BCAST_STD_DRM,
				 1, // nb tuner option
			{
				{1, {ETAL_INVALID_HANDLE, ETAL_INVALID_HANDLE}}
			}
		}
		
	}
};
#endif // CONFIG_ETAL_HAVE_ETALTML


#elif defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
/*!
 * \var		etalTuner
 * 			Describes the Tuners present in the system.
 * 			Tuner handles are statically allocated by ETAL and their value
 * 			corresponds to the index in this array.
 */
etalTunerTy etalTuner[ETAL_CAPA_MAX_TUNER] =
{
	{
		{
			deviceSTARS,
			ETAL_BusI2C,
			0xC2,
			1
		},
		{
			{
				ETAL_UNDEF_SLOT,
				ETAL_BCAST_STD_DAB | ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM | ETAL_BCAST_STD_HD_FM | ETAL_BCAST_STD_HD_AM,
				ETAL_DATA_TYPE_AUDIO | ETAL_DATA_TYPE_DATA_SERVICE | ETAL_DATA_TYPE_TEXTINFO
			},
		}
	}
};

#if defined (CONFIG_ETAL_HAVE_ETALTML)
/*!
 * \var		etalPathPreferences
 * 			Path selection management for ETALTML
 * \todo	add information to manage Tuner selection depending on path
 */
etalPathCapabilitiesTy etalPathPreferences =
{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	7, // nb defined path
#else
	6,
#endif
	{
	// path description 
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
		// DAB_1 is supported only on 1 tuner : channel B of STAR, ie FE_HANDLE_2
		{ ETAL_PATH_NAME_DAB_1, 
		ETAL_BCAST_STD_DAB,
		  1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}}
			}
		},
		// DAB_2 is supported only on 1 tuner : channel A of STAR, ie FE_HANDLE_1
		{ ETAL_PATH_NAME_DAB_2,
		ETAL_BCAST_STD_DAB,
		 1, // nb tuner option
			{
				{1, {ETAL_INVALID_HANDLE, ETAL_INVALID_HANDLE}}
			}
		},
#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
		// to be updated : for now, HD whatever FE 1 or 2
		{ ETAL_PATH_NAME_HD,
		ETAL_BCAST_STD_HD, /* TODO update PATH to _AM or _FM */
		  1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}}
			}
		},
#endif
		// FM_FG is supported only on 1 tuner : channel A of STAR, ie FE_HANDLE_1
		// because no audio on channel B
		// But if VPA supported : then 1 & 2.
		{ ETAL_PATH_NAME_FM_FG,
		ETAL_BCAST_STD_FM,
		  1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}}
			}
		},
		// FM_BG  is supported  in theory on 2 tuners : channel A of STAR, ie FE_HANDLE_1 or channel B ie FE_HANDLE_2
		// the VPA is not applicable
		// take channel B as preference...
		// because no audio on channel B
		// But if VPA supported : then 1 & 2.
		{ ETAL_PATH_NAME_FM_BG,
		ETAL_BCAST_STD_FM,
		 1, // nb tuner option
			{
				{1, {ETAL_INVALID_HANDLE, ETAL_INVALID_HANDLE}},
			}
		},
		// AM is supported only on channel A... as 1 path,FG
		{ ETAL_PATH_NAME_AM,
		ETAL_BCAST_STD_AM,
		 1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}}
			}
		},
		// DRM not covered yet
		{ ETAL_PATH_NAME_DRM_1,
		ETAL_BCAST_STD_DRM,
				 1, // nb tuner option
			{
				{1, {ETAL_INVALID_HANDLE, ETAL_INVALID_HANDLE}}
			}
		},
		
		{ ETAL_PATH_NAME_DRM_2,
		ETAL_BCAST_STD_DRM,
				 1, // nb tuner option
			{
				{1, {ETAL_INVALID_HANDLE, ETAL_INVALID_HANDLE}}
			}
		}
		
	}
};
#endif // CONFIG_ETAL_HAVE_ETALTML
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR && CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL

#elif defined (CONFIG_MODULE_INTEGRATED)
/*!
 * \var		etalTuner
 * 			Describes the Tuners present in the system.
 *
 * 			Tuner handles are statically allocated by ETAL and their value
 * 			corresponds to the index in this array.
 *			NOTE : the tuners devices should be defined in the appropriate order 
 *			1st = Tuner 1 = the tuner which is the master and handle the analog audio
 *			2nd = Tuner 2 = 2nd tuner, a second TDA7707
 *
 *			This defines the MTD board v2.1, with 2xTDA7707
 */
etalTunerTy etalTuner[ETAL_CAPA_MAX_TUNER] =
{
	{
		{
			deviceSTART,
#ifdef CONFIG_COMM_CMOST_I2C
			ETAL_BusI2C,
			CMOST_T1_ACCORDO2_I2C_ADDRESS,
#else
			ETAL_BusSPI,
			0, // not used in SPI
#endif
			2
		},
		{
			{
				ETAL_UNDEF_SLOT,
				/* TODO tuner1 provides also DAB */
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
				ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM | ETAL_BCAST_STD_HD_FM |  ETAL_BCAST_STD_HD_AM,
#else
				ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
#endif
				/* TODO missing DCOP_AUDIO data type */
				ETAL_DATA_TYPE_AUDIO | ETAL_DATA_TYPE_TEXTINFO
			},
			{
				ETAL_UNDEF_SLOT,
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
				ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM | ETAL_BCAST_STD_HD_FM,
#else
				ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
#endif
				ETAL_DATA_TYPE_TEXTINFO
			}
		}
	},
	{
		{
			deviceSTART,
#ifdef CONFIG_COMM_CMOST_I2C
			ETAL_BusI2C,
			CMOST_T2_ACCORDO2_I2C_ADDRESS,
#else
			ETAL_BusSPI,
			0, // not used in SPI
#endif
			2
		},
		{
			{
				ETAL_UNDEF_SLOT,
				/* TODO wrong, this is based on the MTD v1 board, in V2 BB is from tuner1 */
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
				ETAL_BCAST_STD_DAB | ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
#else
				ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
#endif
#if defined(CONFIG_ETAL_SUPPORT_DCOP_MDR) || defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
				ETAL_DATA_TYPE_AUDIO | ETAL_DATA_TYPE_DCOP_AUDIO | ETAL_DATA_TYPE_DATA_SERVICE | ETAL_DATA_TYPE_TEXTINFO
#else
				ETAL_DATA_TYPE_AUDIO | ETAL_DATA_TYPE_DATA_SERVICE | ETAL_DATA_TYPE_TEXTINFO
#endif
			},
			{
				ETAL_UNDEF_SLOT,
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
				ETAL_BCAST_STD_DAB | ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
				ETAL_DATA_TYPE_DCOP_AUDIO | ETAL_DATA_TYPE_DATA_SERVICE | ETAL_DATA_TYPE_TEXTINFO
#elif defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
				ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM | ETAL_BCAST_STD_HD_FM,
				ETAL_DATA_TYPE_DCOP_AUDIO | ETAL_DATA_TYPE_TEXTINFO
#else
				ETAL_BCAST_STD_FM | ETAL_BCAST_STD_AM,
				ETAL_DATA_TYPE_TEXTINFO
#endif
			}
		}
	}
};

#if defined (CONFIG_ETAL_HAVE_ETALTML)
/*!
 * \var		etalPathPreferences
 * 			Path selection management for ETALTML
 * \todo	add information to manage Tuner selection depending on path
 */
etalPathCapabilitiesTy etalPathPreferences =
{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	5, 
#elif defined CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	6, // nb defined path
#else
	3,	// should not happen on an MTD... 
#endif
	{
	// path description 
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		// DAB_1 is supported only on 1 tuner : channel A of 2nd STAR, ie FE_HANDLE_3
		{ ETAL_PATH_NAME_DAB_1, 
		ETAL_BCAST_STD_DAB,
		  1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_3, ETAL_INVALID_HANDLE}}
			}
		},
		// DAB_2 is supported only on 1 tuner : channel B of 2nd STAR, ie FE_HANDLE_4
		{ ETAL_PATH_NAME_DAB_2,
		ETAL_BCAST_STD_DAB,
		 1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_4, ETAL_INVALID_HANDLE}}
			}
		},
#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
		// to be updated : for now, HD whatever FE 1 or 2
		{ ETAL_PATH_NAME_FM_HD_FG,
		ETAL_BCAST_STD_HD_FM,
		  1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}},
			}
		},
			// to be updated : for now, HD whatever FE 1 or 2
		{ ETAL_PATH_NAME_FM_HD_BG,
		ETAL_BCAST_STD_HD_FM,
		  1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_2, ETAL_INVALID_HANDLE}},
			}
		},
		// to be updated : for now, HD whatever FE 1 or 2
		{ ETAL_PATH_NAME_AM_HD,
		ETAL_BCAST_STD_HD_AM,
		  1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}},
			}
		},
#endif

		// FM_FG is supported on both tuners : channel A of STAR, ie FE_HANDLE_1 
		// because no audio on channel B, no audio on tuner 2
		// But if VPA supported : then 1 & 2 (or 3 & 4)
		{ ETAL_PATH_NAME_FM_FG,
		ETAL_BCAST_STD_FM,
		 1, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}}
			}
		},
		// FM_BG  is supported  in theory on 2 tuners : channel A of STAR, ie FE_HANDLE_1 or 3, or channel B, ie FE_HANDLE_2 or 4
		// the VPA is not applicable
		// take channel B as preference...
		// because no audio on channel B
		// But if VPA supported : then 1 & 2 (or 3 & 4)
		{ ETAL_PATH_NAME_FM_BG,
		ETAL_BCAST_STD_FM,
		 4, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_2, ETAL_INVALID_HANDLE}},
				{1, {ETAL_FE_HANDLE_4, ETAL_INVALID_HANDLE}},
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}},
				{1, {ETAL_FE_HANDLE_3, ETAL_INVALID_HANDLE}}
			}
		},
		// AM is supported only on channel A... as 1 path,FG
		{ ETAL_PATH_NAME_AM,
		ETAL_BCAST_STD_AM,
		 2, // nb tuner option
			{
				{1, {ETAL_FE_HANDLE_1, ETAL_INVALID_HANDLE}},
				{1, {ETAL_FE_HANDLE_3, ETAL_INVALID_HANDLE}}
			}
		},
	}
};
#endif // CONFIG_ETAL_HAVE_ETALTML



#else
	// TODO DOT + MDR should be supported
	#error "Unsupported tuner configuration for CMOST Main Board"
#endif
#endif
