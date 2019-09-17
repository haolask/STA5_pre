//!
//!  \file 		etal_index_parameter_TDA7707.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"

#if defined (CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS) || defined (CONFIG_ETAL_HAVE_ALL_API) || \
	defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)

#include "etalinternal.h"
#include "tunerdriver.h"
#include "boot_cmost.h"
#include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_T

const tU32 etalRegisterAddress_TDA7707[ETAL_IDX_CMT_MAX_INTERNAL] =
{
	/* FM foreground */
	TDA7707_tunApp0_fm_wsp_smLevShp,
	TDA7707_tunApp0_fm_wsp_smMpShp,
	TDA7707_tunApp0_fm_wsp_smDistShp,
	TDA7707_tunApp0_fm_wsp_sm,
	TDA7707_tunApp0_fm_wsp_sbLevShp,
	TDA7707_tunApp0_fm_wsp_sbMpShp,
	TDA7707_tunApp0_fm_wsp_sbDistShp,
	TDA7707_tunApp0_fm_wsp_sb,
	TDA7707_tunApp0_fm_wsp_hcLevShp,
	TDA7707_tunApp0_fm_wsp_hcMpShp,
	TDA7707_tunApp0_fm_wsp_hcDistShp,
	TDA7707_tunApp0_fm_wsp_hc,
	TDA7707_tunApp0_fm_wsp_hbLevShp,
	TDA7707_tunApp0_fm_wsp_hbMpShp,
	TDA7707_tunApp0_fm_wsp_hbDistShp,
	TDA7707_tunApp0_fm_wsp_hb,
	TDA7707_tunApp0_fm_wsp_smDistProc,
	TDA7707_tunApp0_fm_wsp_sbDistProc,
	TDA7707_tunApp0_fm_wsp_hcDistProc,
	TDA7707_tunApp0_fm_wsp_hbDistProc,
	TDA7707_tunApp0_fm_qdR_fstLog,
	TDA7707_tunApp0_fm_qdR_fstBB,
	TDA7707_tunApp0_fm_qdR_fstRF,
	TDA7707_tunApp0_fm_qdR_detune,
	TDA7707_tunApp0_fm_qdR_mpxNoise,
	TDA7707_tunApp0_fm_qdR_mp,
	TDA7707_tunApp0_fm_qdR_adj,
	TDA7707_tunApp0_fm_qdR_snr,
	TDA7707_tunApp0_fm_qdR_coChannel,
	TDA7707_tunApp0_fm_qdR_deviation,
	TDA7707_tunApp0_fm_qdR_compressRegQ0,
	TDA7707_tunApp0_fm_qdR_compressRegQ1,
	TDA7707_tunApp0_fm_qdR_compressRegQ2,
	TDA7707_tunApp0_fm_qd_fstLog,
	TDA7707_tunApp0_fm_qd_fstBB,
	TDA7707_tunApp0_fm_qd_fstRF,
	TDA7707_tunApp0_fm_qd_detune,
	TDA7707_tunApp0_fm_qd_detuneFast,
	TDA7707_tunApp0_fm_qd_mpxNoise,
	TDA7707_tunApp0_fm_qd_mp,
	TDA7707_tunApp0_fm_qd_adj,
	TDA7707_tunApp0_fm_qd_snr,
	TDA7707_tunApp0_fm_qd_deviation,
	TDA7707_tunApp0_fm_qd_compressRegQ0,
	TDA7707_tunApp0_fm_qd_compressRegQ1,
	TDA7707_tunApp0_fm_qd_compressRegQ2,
	/* FM background */
	TDA7707_tunApp1_fm_qd_fstLog,
	TDA7707_tunApp1_fm_qd_fstBB,
	TDA7707_tunApp1_fm_qd_fstRF,
	TDA7707_tunApp1_fm_qd_detune,
	TDA7707_tunApp1_fm_qd_detuneFast,
	TDA7707_tunApp1_fm_qd_mpxNoise,
	TDA7707_tunApp1_fm_qd_mp,
	TDA7707_tunApp1_fm_qd_adj,
	TDA7707_tunApp1_fm_qd_snr,
	TDA7707_tunApp1_fm_qd_deviation,
	TDA7707_tunApp1_fm_qd_compressRegQ0,
	TDA7707_tunApp1_fm_qd_compressRegQ1,
	TDA7707_tunApp1_fm_qd_compressRegQ2,
	TDA7707_tunApp0_fm_qdAf_compressRegQ0,
	TDA7707_tunApp0_fm_qdAf_compressRegQ1,
	TDA7707_tunApp0_fm_qdAf_compressRegQ2,
	TDA7707_tunApp1_fm_qdAf_compressRegQ0,
	TDA7707_tunApp1_fm_qdAf_compressRegQ1,
	TDA7707_tunApp1_fm_qdAf_compressRegQ2,
	/* AM foreground */
	TDA7707_tunApp0_am_wsp_smLevWght,
	TDA7707_tunApp0_fm_wsp_smDistWght,
	TDA7707_tunApp0_am_wsp_sm,
	TDA7707_tunApp0_am_wsp_lcLevWght,
#ifndef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
	ETAL_UNDEFINED_ADDRESS, /* TDA7707_tunApp0_fm_wsp_lcDistWght, */
#else
	TDA7707_tunApp0_fm_wsp_lcDistWght,
#endif
	TDA7707_tunApp0_am_wsp_lc,
	TDA7707_tunApp0_am_wsp_hcLevWght,
	TDA7707_tunApp0_fm_wsp_hcDistWght,
	TDA7707_tunApp0_am_wsp_hc,
	TDA7707_tunApp0_am_qd_fstLog,
	TDA7707_tunApp0_am_qd_fstBB,
	TDA7707_tunApp0_am_qd_fstRF,
	TDA7707_tunApp0_am_qd_detune,
	TDA7707_tunApp0_am_qd_adj,
	TDA7707_tunApp0_am_qd_modulation,
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BF) || defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BG)
	ETAL_UNDEFINED_ADDRESS, /* TDA7707_tunApp0_am_qd_compressRegQ0, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7707_tunApp0_am_qd_compressRegQ2, */
#else
	TDA7707_tunApp0_am_qd_compressRegQ0,
	TDA7707_tunApp0_am_qd_compressRegQ2,
#endif
	/* AM background */
	TDA7707_tunApp1_am_qd_fstLog,
	TDA7707_tunApp1_am_qd_fstBB,
	TDA7707_tunApp1_am_qd_fstRF,
	TDA7707_tunApp1_am_qd_detune,
	TDA7707_tunApp1_am_qd_adj,
	TDA7707_tunApp1_am_qd_modulation,
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BF) || defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BG)
	ETAL_UNDEFINED_ADDRESS, /* TDA7707_tunApp1_am_qd_compressRegQ0, */
	ETAL_UNDEFINED_ADDRESS, /* TDA7707_tunApp1_am_qd_compressRegQ2, */
#else
	TDA7707_tunApp1_am_qd_compressRegQ0,
	TDA7707_tunApp1_am_qd_compressRegQ2,
#endif

	/* start of internal indexes */

	/* XTAL alignment */
	TDA7707_tunApp0_tm_outSwitch,
	TDA7707_tunApp0_tm_iqShift,
	TDA7707_bbpX_detFlags,
	TDA7707_bbpX_y1High	,
	TDA7707_systemConfig_tuneDetCompCoeff,
	/* etal_get_version command */
	TDA7707_mainY_st_version_info__0__,
	/* service following */
	TDA7707_tunApp0_fm_qd_quality,
	TDA7707_tunApp0_fm_qdAf_quality,
	TDA7707_tunApp1_fm_qd_quality,
	TDA7707_tunApp1_fm_qdAf_quality,
};

#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR && CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL
#endif // CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS || CONFIG_ETAL_HAVE_ALL_API || CONFIG_ETAL_HAVE_XTAL_ALIGNMENT || CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
