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
--  Abstract : Application Programming Interface (API) extension
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "h264hwd_container.h"
#include "h264decapi_e.h"

/*------------------------------------------------------------------------------

    Function: H264DecNextChPicture

        Functional description:

        Input:
            decInst     decoder instance.

        Output:
            pOutput     pointer to output structure

        Returns:
            H264DEC_OK            no pictures available for display
            H264DEC_PIC_RDY       picture available for display
            H264DEC_PARAM_ERROR     invalid parameters

------------------------------------------------------------------------------*/
H264DecRet H264DecNextChPicture(H264DecInst decInst,
                              u32 **pOutput, u32 *busAddr)
{
    decContainer_t *pDecCont = (decContainer_t *) decInst;

    if(decInst == NULL || pOutput == NULL || busAddr == NULL)
    {
        return (H264DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        return (H264DEC_NOT_INITIALIZED);
    }

    if (pDecCont->storage.enable2ndChroma && pDecCont->storage.pCh2)
    {
        *pOutput = pDecCont->storage.pCh2;
        *busAddr = pDecCont->storage.bCh2;
        return (H264DEC_PIC_RDY);
    }
    else
    {
        return (H264DEC_OK);
    }

}
