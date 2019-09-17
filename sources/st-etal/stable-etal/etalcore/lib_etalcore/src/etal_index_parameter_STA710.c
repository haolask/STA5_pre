//!
//!  \file 		etal_index_parameter_STA710.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"

#if defined (CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS) || defined (CONFIG_ETAL_HAVE_ALL_API) || \
	defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)

#include "etalinternal.h"
#include "tunerdriver.h"
#include "boot_cmost.h"
#include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_T

const tU32 etalRegisterAddress_STA710[] =
{
	/*
	 * Quality and WSP is not available on the DOT
	 * thus this array contains only the internal indexes
	 * It is necessary to scale down the array index before
	 * accessing this array
	 */

	/* XTAL alignment */
	STA710_tunApp0_tm_outSwitch,
	STA710_tunApp0_tm_iqShift,
	STA710_bbpX_detFlags,
	STA710_bbpX_y1High,
	STA710_systemConfig_tuneDetCompCoeff,
	/* etal_get_version command */
	STA710_mainY_st_version_info__0__,
	/* service following, quality not available on DOT  */
	ETAL_UNDEFINED_ADDRESS, /* IDX_CMT_tunApp0_fm_qd_quality, */
	ETAL_UNDEFINED_ADDRESS, /* IDX_CMT_tunApp0_fm_qdAf_quality, */
	ETAL_UNDEFINED_ADDRESS, /* IDX_CMT_tunApp1_fm_qd_quality, */
	ETAL_UNDEFINED_ADDRESS, /* IDX_CMT_tunApp1_fm_qdAf_quality */
};

#endif // CONFIG_ETAL_SUPPORT_CMOST_DOT && CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL
#endif // CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS || CONFIG_ETAL_HAVE_ALL_API || CONFIG_ETAL_HAVE_XTAL_ALIGNMENT || CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
