//!
//!  \file 		etal_index_parameter_TDA7708.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"

#if defined (CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS) || defined (CONFIG_ETAL_HAVE_ALL_API) || \
	defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)

#include "etalinternal.h"
#include "tunerdriver.h"
#include "boot_cmost.h"
#include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_S

const tU32 etalRegisterAddress_TDA7708[ETAL_IDX_CMT_MAX_INTERNAL] =
{
	TDA7708_tunApp0_fm_wsp_smLevShp,
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp0_fm_wsp_smMpSh, */
	TDA7708_tunApp0_fm_wsp_smDistShp,
	TDA7708_tunApp0_fm_wsp_sm,
	TDA7708_tunApp0_fm_wsp_sbLevShp,
	TDA7708_tunApp0_fm_wsp_sbMpShp,
	TDA7708_tunApp0_fm_wsp_sbDistShp,
	TDA7708_tunApp0_fm_wsp_sb,
	TDA7708_tunApp0_fm_wsp_hcLevShp,
	TDA7708_tunApp0_fm_wsp_hcMpShp,
	TDA7708_tunApp0_fm_wsp_hcDistShp,
	TDA7708_tunApp0_fm_wsp_hc,
	TDA7708_tunApp0_fm_wsp_hbLevShp,
	TDA7708_tunApp0_fm_wsp_hbMpShp,
	TDA7708_tunApp0_fm_wsp_hbDistShp,
	TDA7708_tunApp0_fm_wsp_hb,
	TDA7708_tunApp0_fm_wsp_smDistProc,
	TDA7708_tunApp0_fm_wsp_sbDistProc,
	TDA7708_tunApp0_fm_wsp_hcDistProc,
	TDA7708_tunApp0_fm_wsp_hbDistProc,
	TDA7708_tunApp0_fm_qdR_fstLog,
	TDA7708_tunApp0_fm_qdR_fstBB,
	TDA7708_tunApp0_fm_qdR_fstRF,
	TDA7708_tunApp0_fm_qdR_detune,
	TDA7708_tunApp0_fm_qdR_mpxNoise,
	TDA7708_tunApp0_fm_qdR_mp,
	TDA7708_tunApp0_fm_qdR_adj,
	TDA7708_tunApp0_fm_qdR_snr,
	TDA7708_tunApp0_fm_qdR_coChannel,
	TDA7708_tunApp0_fm_qdR_deviation,
	TDA7708_tunApp0_fm_qdR_compressRegQ0,
	TDA7708_tunApp0_fm_qdR_compressRegQ1,
	TDA7708_tunApp0_fm_qdR_compressRegQ2,
	TDA7708_tunApp0_fm_qd_fstLog,
	TDA7708_tunApp0_fm_qd_fstBB,
	TDA7708_tunApp0_fm_qd_fstRF,
	TDA7708_tunApp0_fm_qd_detune,
	TDA7708_tunApp0_fm_qd_detuneFast,
	TDA7708_tunApp0_fm_qd_mpxNoise,
	TDA7708_tunApp0_fm_qd_mp,
	TDA7708_tunApp0_fm_qd_adj,
	TDA7708_tunApp0_fm_qd_snr,
	TDA7708_tunApp0_fm_qd_deviation,
	TDA7708_tunApp0_fm_qd_compressRegQ0,
	TDA7708_tunApp0_fm_qd_compressRegQ1,
	TDA7708_tunApp0_fm_qd_compressRegQ2,
	/* FM background */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_fstLog, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_fstBB, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_fstRF, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_detune, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_detuneFast, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_mpxNoise, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_mp, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_adj, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_snr, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_deviation, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_compressRegQ0, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_compressRegQ1, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_compressRegQ2, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp0_fm_qdAf_compressRegQ0, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp0_fm_qdAf_compressRegQ1, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp0_fm_qdAf_compressRegQ2, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qdAf_compressRegQ0, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qdAf_compressRegQ1, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qdAf_compressRegQ2, */
	/* AM foreground */
	TDA7708_tunApp0_am_wsp_smLevWght,
	TDA7708_tunApp0_fm_wsp_smDistWght,
	TDA7708_tunApp0_am_wsp_sm,
	TDA7708_tunApp0_am_wsp_lcLevWght,
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp0_fm_wsp_lcDistWght, */
	TDA7708_tunApp0_am_wsp_lc,
	TDA7708_tunApp0_am_wsp_hcLevWght,
	TDA7708_tunApp0_fm_wsp_hcDistWght,
	TDA7708_tunApp0_am_wsp_hc,
	TDA7708_tunApp0_am_qd_fstLog,
	TDA7708_tunApp0_am_qd_fstBB,
	TDA7708_tunApp0_am_qd_fstRF,
	TDA7708_tunApp0_am_qd_detune,
	TDA7708_tunApp0_am_qd_adj,
	TDA7708_tunApp0_am_qd_modulation,
	TDA7708_tunApp0_am_qd_compressRegQ0,
	TDA7708_tunApp0_am_qd_compressRegQ2,
	/* AM background */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_am_qd_fstLog, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_am_qd_fstBB, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_am_qd_fstRF, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_am_qd_detune, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_am_qd_adj, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_am_qd_modulation, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_am_qd_compressRegQ0, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_am_qd_compressRegQ2 */

	/* start of internal indexes */

	/* XTAL alignment */
	TDA7708_tunApp0_tm_outSwitch,
	TDA7708_tunApp0_tm_iqShift,
	TDA7708_bbpX_detFlags,
	TDA7708_bbpX_y1High,
	TDA7708_systemConfig_tuneDetCompCoeff,
	/* etal_get_version command */
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_BC
	TDA7708_mainY_st_version_info__0__,
#else
	TDA7708_mainY_st_version_info__0___no1,
#endif
	/* service following */
	TDA7708_tunApp0_fm_qd_quality,
	TDA7708_tunApp0_fm_qdAf_quality,
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qd_quality, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7708_tunApp1_fm_qdAf_quality, */
};

#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR && CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL
#endif // CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS || CONFIG_ETAL_HAVE_ALL_API || CONFIG_ETAL_HAVE_XTAL_ALIGNMENT || CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
