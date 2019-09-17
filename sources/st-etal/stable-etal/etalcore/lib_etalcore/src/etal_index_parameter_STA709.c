//!
//!  \file 		etal_index_parameter_STA709.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"

#if defined (CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS) || defined (CONFIG_ETAL_HAVE_ALL_API) || \
	defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING) 
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)

#include "etalinternal.h"
#include "tunerdriver.h"
#include "boot_cmost.h"
#include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_S

const tU32 etalRegisterAddress_STA709[] =
{
/* 
 * the firmware included in ETAL for STA709 is old (1.1.0) and 
 * uses the old naming convention (CMT_ instead of STA709_)
 * TODO need to update the STA709 firmware and the following array
 */

	/*
	 * Quality and WSP is not available on the DOT
	 * thus this array contains only the internal indexes
	 * It is necessary to scale down the array index before
	 * accessing this array
	 */

	/* XTAL alignment */
	CMT_tunApp0_tm_outSwitch,
	CMT_tunApp0_tm_iqShift,
	CMT_bbpX_detFlags,
	CMT_bbpX_y1High,
	CMT_systemConfig_tuneDetCompCoeff,
	/* etal_get_version command */
	CMT_mainY_st_version_info__0__,
	/* service following not available on DOT */
	ETAL_UNDEFINED_ADDRESS, /* STA709_tunApp0_fm_qd_quality, */
	ETAL_UNDEFINED_ADDRESS, /* STA709_tunApp0_fm_qdAf_quality, */
	ETAL_UNDEFINED_ADDRESS, /* STA709_tunApp1_fm_qd_quality, */
	ETAL_UNDEFINED_ADDRESS, /* STA709_tunApp1_fm_qdAf_quality, */
};

#endif // CONFIG_ETAL_SUPPORT_CMOST_DOT && CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL
#endif // CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS || CONFIG_ETAL_HAVE_ALL_API || CONFIG_ETAL_HAVE_XTAL_ALIGNMENT || CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
