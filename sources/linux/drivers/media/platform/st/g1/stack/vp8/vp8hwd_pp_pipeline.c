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
--  Description :  VP8 decoder and PP pipeline support
--
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "vp8hwd_pp_pipeline.h"
#include "vp8hwd_container.h"
#include "vp8hwd_debug.h"

#ifdef _TRACE_PP_CTRL
#ifndef TRACE_PP_CTRL
#include <stdio.h>
#define TRACE_PP_CTRL(...)          printf(__VA_ARGS__)
#endif
#else
#define TRACE_PP_CTRL(...)
#endif

/*------------------------------------------------------------------------------
    Function name   : vp8RegisterPP
    Description     :
    Return type     : i32
    Argument        : const void * decInst
    Argument        : const void  *ppInst
    Argument        : (*PPRun)(const void *)
    Argument        : void (*PPEndCallback)(const void *)
------------------------------------------------------------------------------*/
i32 vp8RegisterPP(const void *decInst, const void *ppInst,
                   void (*PPDecStart) (const void *, const DecPpInterface *),
                   void (*PPDecWaitEnd) (const void *),
                   void (*PPConfigQuery) (const void *, DecPpQuery *))
{
    VP8DecContainer_t  *pDecCont;

    pDecCont = (VP8DecContainer_t *) decInst;

    if(decInst == NULL || pDecCont->pp.ppInstance != NULL ||
       ppInst == NULL || PPDecStart == NULL || PPDecWaitEnd == NULL
       || PPConfigQuery == NULL)
    {
        TRACE_PP_CTRL("vp8hwdRegisterPP: Invalid parameter\n");
        return -1;
    }

    if(pDecCont->asicRunning)
    {
        TRACE_PP_CTRL("vp8hwdRegisterPP: Illegal action, asicRunning\n");
        return -2;
    }

    pDecCont->pp.ppInstance = ppInst;
    pDecCont->pp.PPConfigQuery = PPConfigQuery;
    pDecCont->pp.PPDecStart = PPDecStart;
    pDecCont->pp.PPDecWaitEnd = PPDecWaitEnd;

    pDecCont->pp.decPpIf.ppStatus = DECPP_IDLE;

    TRACE_PP_CTRL("vp8hwdRegisterPP: Connected to PP instance 0x%08x\n", (size_t)ppInst);

    return 0;
}

/*------------------------------------------------------------------------------
    Function name   : vpdUnregisterPP
    Description     :
    Return type     : i32
    Argument        : const void * decInst
    Argument        : const void  *ppInst
------------------------------------------------------------------------------*/
i32 vp8UnregisterPP(const void *decInst, const void *ppInst)
{
    VP8DecContainer_t *pDecCont;

    pDecCont = (VP8DecContainer_t *) decInst;

    ASSERT(decInst != NULL && ppInst == pDecCont->pp.ppInstance);

    if(ppInst != pDecCont->pp.ppInstance)
    {
        TRACE_PP_CTRL("vp8hwdUnregisterPP: Invalid parameter\n");
        return -1;
    }

    if(pDecCont->asicRunning)
    {
        TRACE_PP_CTRL("vp8hwdUnregisterPP: Illegal action, asicRunning\n");
        return -2;
    }

    pDecCont->pp.ppInstance = NULL;
    pDecCont->pp.PPConfigQuery = NULL;
    pDecCont->pp.PPDecStart = NULL;
    pDecCont->pp.PPDecWaitEnd = NULL;

    TRACE_PP_CTRL("vp8hwdUnregisterPP: Disconnected from PP instance 0x%08x\n",
                  (size_t)ppInst);

    return 0;
}

/*------------------------------------------------------------------------------
    Function name   : vp8hwdPreparePpRun
    Description     : 
    Return type     : void 
    Argument        : 
------------------------------------------------------------------------------*/
void vp8hwdPreparePpRun(VP8DecContainer_t * pDecCont)
{
    DecPpInterface *decPpIf = &pDecCont->pp.decPpIf;

    if(pDecCont->pp.ppInstance != NULL) /* we have PP connected */
    {
        pDecCont->pp.ppInfo.tiledMode =
            pDecCont->tiledReferenceEnable;
        pDecCont->pp.PPConfigQuery(pDecCont->pp.ppInstance,
                                &pDecCont->pp.ppInfo);

        TRACE_PP_CTRL
            ("vp8hwdPreparePpRun: output picture => PP could run!\n");

        decPpIf->usePipeline = pDecCont->pp.ppInfo.pipelineAccepted & 1;

        /* pipeline accepted, but previous pic needs to be processed
         * (pp config probably changed from one that cannot be run in
         * pipeline to one that can) */
        pDecCont->pendingPicToPp = 0;
        if (decPpIf->usePipeline && pDecCont->outCount)
        {
            decPpIf->usePipeline = 0;
            pDecCont->pendingPicToPp = 1;
        }

        if(decPpIf->usePipeline)
        {
            TRACE_PP_CTRL
                ("vp8hwdPreparePpRun: pipeline=ON => PP will be running\n");
            decPpIf->ppStatus = DECPP_RUNNING;
        }
        /* parallel processing if previous output pic exists */
        else if (pDecCont->outCount)
        {
            TRACE_PP_CTRL
                ("vp8hwdPreparePpRun: pipeline=OFF => PP has to run after DEC\n");
            decPpIf->ppStatus = DECPP_RUNNING;
        }
    }
}
