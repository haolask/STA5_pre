/*
 * (C) COPYRIGHT 2011 HANTRO PRODUCTS
 *
 * Please contact: hantro-support@verisilicon.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

/*------------------------------------------------------------------------------
--
--  Description : Sytem Wrapper Layer
--
------------------------------------------------------------------------------*/
#ifndef __DWL_H__
#define __DWL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "basetype.h"
#include "decapicommon.h"
#include "debug.h"

#define DWL_OK                      0
#define DWL_ERROR                  -1

#define DWL_HW_WAIT_OK              DWL_OK
#define DWL_HW_WAIT_ERROR           DWL_ERROR
#define DWL_HW_WAIT_TIMEOUT         1

#define DWL_CLIENT_TYPE_H264_DEC         1U
#define DWL_CLIENT_TYPE_MPEG4_DEC        2U
#define DWL_CLIENT_TYPE_JPEG_DEC         3U
#define DWL_CLIENT_TYPE_PP               4U
#define DWL_CLIENT_TYPE_VC1_DEC          5U
#define DWL_CLIENT_TYPE_MPEG2_DEC        6U
#define DWL_CLIENT_TYPE_VP6_DEC          7U
#define DWL_CLIENT_TYPE_AVS_DEC          8U
#define DWL_CLIENT_TYPE_RV_DEC           9U
#define DWL_CLIENT_TYPE_VP8_DEC          10U

/* define irq registers offset and masks */
#define G1_HW_DEC_IRQ_OFFSET	(1 * 4)
#define G1_HW_PP_IRQ_OFFSET	(60 * 4)
#define G1_HW_DEC_IRQ_MASK	0x100
#define G1_HW_PP_IRQ_MASK	0x100
#define G1_HW_PP_PIPELINE_E_MASK	0x2

#define G1_MPEG2_E         31	/* 1 bit */
#define G1_VC1_E           29	/* 2 bits */
#define G1_JPEG_E          28	/* 1 bit */
#define G1_MPEG4_E         26	/* 2 bits */
#define G1_H264_E          24	/* 2 bits */
#define G1_VP6_E           23	/* 1 bit */
#define G1_PJPEG_E         22	/* 1 bit */
#define G1_REF_BUFF_E      20	/* 1 bit */

#define G1_JPEG_EXT_E          31	/* 1 bit */
#define G1_REF_BUFF_ILACE_E    30	/* 1 bit */
#define G1_MPEG4_CUSTOM_E      29	/* 1 bit */
#define G1_REF_BUFF_DOUBLE_E   28	/* 1 bit */
#define G1_RV_E            26	/* 2 bits */
#define G1_VP7_E           24	/* 1 bit */
#define G1_VP8_E           23	/* 1 bit */
#define G1_AVS_E           22	/* 1 bit */
#define G1_MVC_E           20	/* 2 bits */
#define G1_WEBP_E          19	/* 1 bit */
#define G1_DEC_TILED_L     17	/* 2 bits */
#define G1_DEC_PIC_W_EXT   14	/* 2 bits */
#define G1_EC_E            12	/* 2 bits */
#define G1_STRIDE_E        11	/* 1 bit */
#define G1_FIELD_DPB_E     10	/* 1 bit */

#define G1_CFG_E           24	/* 4 bits */
#define G1_PP_E            16	/* 1 bit */
#define G1_PP_IN_TILED_L   14	/* 2 bits */

#define G1_SORENSONSPARK_E 11	/* 1 bit */

#define G1_H264_FUSE_E          31	/* 1 bit */
#define G1_MPEG4_FUSE_E         30	/* 1 bit */
#define G1_MPEG2_FUSE_E         29	/* 1 bit */
#define G1_SORENSONSPARK_FUSE_E 28	/* 1 bit */
#define G1_JPEG_FUSE_E          27	/* 1 bit */
#define G1_VP6_FUSE_E           26	/* 1 bit */
#define G1_VC1_FUSE_E           25	/* 1 bit */
#define G1_PJPEG_FUSE_E         24	/* 1 bit */
#define G1_CUSTOM_MPEG4_FUSE_E  23	/* 1 bit */
#define G1_RV_FUSE_E            22	/* 1 bit */
#define G1_VP7_FUSE_E           21	/* 1 bit */
#define G1_VP8_FUSE_E           20	/* 1 bit */
#define G1_AVS_FUSE_E           19	/* 1 bit */
#define G1_MVC_FUSE_E           18	/* 1 bit */

#define G1_DEC_MAX_1920_FUSE_E  15	/* 1 bit */
#define G1_DEC_MAX_1280_FUSE_E  14	/* 1 bit */
#define G1_DEC_MAX_720_FUSE_E   13	/* 1 bit */
#define G1_DEC_MAX_352_FUSE_E   12	/* 1 bit */
#define G1_REF_BUFF_FUSE_E       7	/* 1 bit */

#define G1_PP_FUSE_E				31	/* 1 bit */
#define G1_PP_DEINTERLACE_FUSE_E   30	/* 1 bit */
#define G1_PP_ALPHA_BLEND_FUSE_E   29	/* 1 bit */
#define G1_PP_MAX_4096_FUSE_E		16	/* 1 bit */
#define G1_PP_MAX_1920_FUSE_E		15	/* 1 bit */
#define G1_PP_MAX_1280_FUSE_E		14	/* 1 bit */
#define G1_PP_MAX_720_FUSE_E		13	/* 1 bit */
#define G1_PP_MAX_352_FUSE_E		12	/* 1 bit */


    /* Linear memory area descriptor */
    typedef struct DWLLinearMem
    {
        u32 *virtualAddress;
        u32 busAddress;
        u32 size;
	const char* name;
    } DWLLinearMem_t;

    /* DWLInitParam is used to pass parameters when initializing the DWL */
    typedef struct DWLInitParam
    {
        u32 clientType;
    } DWLInitParam_t;

    /* Hardware configuration description, same as in top API */
    typedef struct DecHwConfig_ DWLHwConfig_t;

    typedef struct DWLHwFuseStatus
    {
        u32 h264SupportFuse;     /* HW supports h.264 */
        u32 mpeg4SupportFuse;    /* HW supports MPEG-4 */
        u32 mpeg2SupportFuse;    /* HW supports MPEG-2 */
        u32 sorensonSparkSupportFuse;   /* HW supports Sorenson Spark */
        u32 jpegSupportFuse;     /* HW supports JPEG */
        u32 vp6SupportFuse;      /* HW supports VP6 */
        u32 vp7SupportFuse;      /* HW supports VP6 */
        u32 vp8SupportFuse;      /* HW supports VP6 */
        u32 vc1SupportFuse;      /* HW supports VC-1 Simple */
        u32 jpegProgSupportFuse; /* HW supports Progressive JPEG */
        u32 ppSupportFuse;       /* HW supports post-processor */
        u32 ppConfigFuse;        /* HW post-processor functions bitmask */
        u32 maxDecPicWidthFuse;  /* Maximum video decoding width supported  */
        u32 maxPpOutPicWidthFuse; /* Maximum output width of Post-Processor */
        u32 refBufSupportFuse;   /* HW supports reference picture buffering */
        u32 avsSupportFuse;      /* one of the AVS values defined above */
        u32 rvSupportFuse;       /* one of the REAL values defined above */
        u32 mvcSupportFuse;
        u32 customMpeg4SupportFuse; /* Fuse for custom MPEG-4 */
    } DWLHwFuseStatus_t;

/* HW ID retrieving, static implementation */
    u32 DWLReadAsicID(void);

/*Custo */
void DWLSetup(void * pctx);
void DWLSetUserContext(const void *instance, void *userCtx);

/* HW configuration retrieving, static implementation */
    void DWLReadAsicConfig(DWLHwConfig_t *pHwCfg);
    void DWLReadMCAsicConfig(DWLHwConfig_t pHwCfg[MAX_ASIC_CORES]);

   /* Return number of ASIC cores, static implementation */
   u32 DWLReadAsicCoreCount(void);

/* HW fuse retrieving, static implementation */
	 void DWLReadAsicFuseStatus(DWLHwFuseStatus_t * pHwFuseSts);

/* DWL initialization and release */
    const void *DWLInit(DWLInitParam_t * param);
    i32 DWLRelease(const void *instance);

/* HW sharing */
    i32 DWLReserveHw(const void *instance, i32 *coreID);
    i32 DWLReserveHwPipe(const void *instance, i32 *coreID);
    void DWLReleaseHw(const void *instance, i32 coreID);

/* Frame buffers memory */
    i32 DWLMallocRefFrm(const void *instance, u32 size, DWLLinearMem_t * info);
    void DWLFreeRefFrm(const void *instance, DWLLinearMem_t * info);

/* SW/HW shared memory */
    i32 DWLMallocLinear(const void *instance, u32 size, DWLLinearMem_t * info);
    void DWLFreeLinear(const void *instance, DWLLinearMem_t * info);

/* D-Cache coherence */
    void DWLDCacheRangeFlush(const void *instance, DWLLinearMem_t * info);  /* NOT in use */
    void DWLDCacheRangeRefresh(const void *instance, DWLLinearMem_t * info);    /* NOT in use */

/* Register access */
    void DWLWriteReg(const void *instance, i32 coreID, u32 offset, u32 value);
    u32 DWLReadReg(const void *instance, i32 coreID, u32 offset);

    void DWLWriteRegAll(const void *instance, const u32 * table, u32 size); /* NOT in use */
    void DWLReadRegAll(const void *instance, u32 * table, u32 size);    /* NOT in use */

/* HW starting/stopping */
    void DWLEnableHW(const void *instance, i32 coreID, u32 offset, u32 value);
    void DWLDisableHW(const void *instance, i32 coreID, u32 offset, u32 value);

/* HW synchronization */
    i32 DWLWaitHwReady(const void *instance, i32 coreID, u32 timeout);

    typedef void DWLIRQCallbackFn(void *arg, i32 coreID);

    void DWLSetIRQCallback(const void *instance, i32 coreID,
            DWLIRQCallbackFn *pCallbackFn, void* arg);

/* SW/SW shared memory */
    void *DWLmalloc(u32 n);
    void DWLfree(void *p);
    void *DWLcalloc(u32 n, u32 s);
    void *DWLmemcpy(void *d, const void *s, u32 n);
    void *DWLmemset(void *d, i32 c, u32 n);
/* G1 init*/
void * g1_hw_init(void);
void g1_hw_release(void * pctx);


#ifdef __cplusplus
}
#endif

#endif                       /* __DWL_H__ */
