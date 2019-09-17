//!
//!  \file 		etalruntimecheck.c
//!  \brief 	<i><b> ETAL build run-time checks </b></i>
//!  \details   Run-time checks aiming to validate the ETAL build.
//!  \details	**None of the functions here contained should be included in a release build.**
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	#include "ipfcomm.h"
#endif

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	#include "tunerdriver.h"
	#include "cmost_helpers.h"
#endif

#if defined (CONFIG_DEBUG_ETAL_CHECKS)

/***********************************
 *
 * Defines and macros
 *
 **********************************/

/***********************************
 *
 * Local types
 *
 **********************************/

/***********************************
 *
 * Local variables
 *
 **********************************/

#if defined (CONFIG_DEBUG_TYPEDEF_CHECK)
#ifdef CONFIG_ETAL_SUPPORT_CMOST
static tVoid ETAL_buildcheck_CMOST_accessSizeEnumTy(tVoid)
{
	CMOST_accessSizeEnumTy var = 0;

	switch (var)
	{
		case CMOST_ACCESSSIZE_32BITS:
		case CMOST_ACCESSSIZE_24BITS:
			break;
	}
	if ((CMOST_ACCESSSIZE_32BITS > 255) ||
		(CMOST_ACCESSSIZE_24BITS > 255))
	{
		ASSERT_ON_DEBUGGING(0);
	}
}

static tVoid ETAL_buildcheck_CMOST_cmdModeEnumTy(tVoid)
{
	CMOST_cmdModeEnumTy var = 0;

	switch (var)
	{
		case CMOST_CMDMODE_CMD:
		case CMOST_CMDMODE_RD:
		case CMOST_CMDMODE_WR:
		case CMOST_CMDMODE_RD_DMA:
		case CMOST_CMDMODE_WR_DMA:
		case CMOST_CMDMODE_CMD_GEN:
		case CMOST_CMDMODE_W_BOOT:
		case CMOST_CMDMODE_RESET:
		case CMOST_CMDMODE_IDLE:
			break;
	}
	if ((CMOST_CMDMODE_CMD > 255) ||
		(CMOST_CMDMODE_RD > 255) ||
		(CMOST_CMDMODE_WR > 255) ||
		(CMOST_CMDMODE_RD_DMA > 255) ||
		(CMOST_CMDMODE_WR_DMA > 255) ||
		(CMOST_CMDMODE_CMD_GEN > 255) ||
		(CMOST_CMDMODE_W_BOOT > 255) ||
		(CMOST_CMDMODE_RESET > 255) ||
		(CMOST_CMDMODE_IDLE > 255))
	{
		ASSERT_ON_DEBUGGING(0);
	}
}

static tVoid ETAL_buildcheck_CMOST_addressEnumTy(tVoid)
{
	CMOST_addressEnumTy var = 0;

	switch (var)
	{
		case CMOST_SPI_CS1:
		case CMOST_SPI_CS2:
		case CMOST_SPI_CS3:
		case CMOST_I2CADDR_C0:
		case CMOST_I2CADDR_C2:
		case CMOST_I2CADDR_C8:
			break;
	}
	if ((CMOST_SPI_CS1 > 255) ||
		(CMOST_SPI_CS2 > 255) ||
		(CMOST_SPI_CS3 > 255) ||
		(CMOST_I2CADDR_C0 > 255) ||
		(CMOST_I2CADDR_C2 > 255) ||
		(CMOST_I2CADDR_C8 > 255))
	{
		ASSERT_ON_DEBUGGING(0);
	}
}
#endif // CONFIG_ETAL_SUPPORT_CMOST

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
static tVoid ETAL_buildcheck_DABMWDataServiceType(tVoid)
{
	DABMWDataServiceType var = 0;

	switch (var)
	{
		case DABMW_DATACHANNEL_DECODED_TYPE_UNDEF:
		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_RAW:
		case DABMW_DATACHANNEL_DECODED_TYPE_SLS:
		case DABMW_DATACHANNEL_DECODED_TYPE_SLS_XPAD:
		case DABMW_DATACHANNEL_DECODED_TYPE_BWS_RAW:
		case DABMW_DATACHANNEL_DECODED_TYPE_TPEG_RAW:
		case DABMW_DATACHANNEL_DECODED_TYPE_TPEG_SNI:
		case DABMW_DATACHANNEL_DECODED_TYPE_SLI:
		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_BIN:
		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_SRV:
		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_PRG :
		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_LOGO:
		case DABMW_DATACHANNEL_DECODED_TYPE_JML_OBJ:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_OBJ:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_TM:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_QUALITY:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_FAC:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_SDC:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_PCMAUDIO:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_MDI:
		case DABMW_DATACHANNEL_DECODED_TYPE_DEBUGDUMP:
			break;
	}
	if ((DABMW_DATACHANNEL_DECODED_TYPE_UNDEF > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_EPG_RAW > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_SLS > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_SLS_XPAD > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_BWS_RAW > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_TPEG_RAW > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_TPEG_SNI > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_SLI > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_EPG_BIN > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_EPG_SRV > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_EPG_PRG > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_EPG_LOGO > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_JML_OBJ > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_DRM_OBJ > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_DRM_TM > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_DRM_QUALITY > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_DRM_FAC > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_DRM_SDC > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_DRM_PCMAUDIO > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_DRM_MDI > 255) ||
		(DABMW_DATACHANNEL_DECODED_TYPE_DEBUGDUMP > 255))
	{
		ASSERT_ON_DEBUGGING(0);
	}
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
static tVoid ETAL_buildcheck_tyHDRADIOInstanceID(tVoid)
{
	tyHDRADIOInstanceID var = 0;

	switch (var)
	{
		case INSTANCE_UNDEF:
		case INSTANCE_1:
		case INSTANCE_2:
			break;
	}
	if ((INSTANCE_UNDEF > 255) ||
		(INSTANCE_1 > 255) ||
		(INSTANCE_2 > 255))
	{
		ASSERT_ON_DEBUGGING(0);
	}
}
#endif // ETAL_buildcheck_tyHDRADIOInstanceID

/***********************************
 *
 * ETAL_buildcheck_typedefs
 *
 **********************************/
/*!
 * \brief		Ensure some typedef fit in one byte
 * \details		To reduce the number of run-time checks, ETAL makes assumptions
 * 				on the size of some objects and uses a plain 'C' language
 * 				cast operator to convert them to a tU8. This function checks that
 * 				the assumption is true.
 *
 *				For each typedef to be checked the function:
 *				- defines a variable of that type and initializes it to 0 to avoid compiler warnings;
 *				- uses the variable in a 'switch' statement that does nothing but lists every value
 *				of the typedef, with no 'default' label. This is done to ensure that
 *				the compiler will issue a warning if a value is added to the
 *				typedef without updating this function (but note that this works with the
 *				reference GCC compiler and the appropriate compiler flags, results with other
 *				compilers may vary);
 *				- finally it checks if all the possible variable values are within the byte bound.
 *
 * \remark		This function needs to be included in the build only when new types
 * 				are added. By no means it should be included in a release build.
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_buildcheck_typedefs(tVoid)
{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	ETAL_buildcheck_CMOST_accessSizeEnumTy();
	ETAL_buildcheck_CMOST_cmdModeEnumTy();
	ETAL_buildcheck_CMOST_addressEnumTy();
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	ETAL_buildcheck_DABMWDataServiceType();
	ASSERT_ON_DEBUGGING(ETAL_MAX_FILTERS <= 255);
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	ETAL_buildcheck_tyHDRADIOInstanceID();
#endif
}
#endif // CONFIG_DEBUG_TYPEDEF_CHECK

/***********************************
 *
 * ETAL_runtimecheck
 *
 **********************************/
/*!
 * \brief		Perform several run-time checks
 * \details		To reduce the number of run-time checks ETAL makes several assumptions
 * 				that are checked in this function.
 * \remark		By no means this function should be included in a release build.
 * \see			ETAL_buildcheck_typedefs, ETAL_configValidate
 * \callgraph
 * \callergraph
 */
tVoid ETAL_runtimecheck(tVoid)
{
#if defined (CONFIG_DEBUG_TYPEDEF_CHECK)
	ETAL_buildcheck_typedefs();
#endif
#if defined (CONFIG_DEBUG_CONFIG_VALIDATE_CHECK)
	ETAL_configValidate();
#endif
}

#endif // CONFIG_DEBUG_ETAL_CHECKS
