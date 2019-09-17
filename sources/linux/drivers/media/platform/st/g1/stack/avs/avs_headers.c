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
--  Abstract :  Header decoding
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "avs_headers.h"
#include "avs_utils.h"
#include "avs_strm.h"
#include "avs_cfg.h"
#include "avs_vlc.h"

/*------------------------------------------------------------------------------
    2. External identifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Module indentifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

   5.1  Function name:
                AvsStrmDec_DecodeSequenceHeader

        Purpose:

        Input:

        Output:

------------------------------------------------------------------------------*/
u32 AvsStrmDec_DecodeSequenceHeader(DecContainer * pDecContainer)
{
    u32 tmp;
    DecHdrs *pHdr;

    ASSERT(pDecContainer);

    AVSDEC_DEBUG(("Decode Sequence Header Start\n"));

    pHdr = pDecContainer->StrmStorage.strmDecReady == FALSE ?
        &pDecContainer->Hdrs : &pDecContainer->tmpHdrs;

    /* profile_id (needs to be checked, affects the fields that need to be
     * read from the stream) */
    tmp = AvsStrmDec_GetBits(pDecContainer, 8);
    if ((tmp != 0x20 && /* JIZHUN_PROFILE_ID */
        tmp != 0x48) || /* GUANGDIAN_PROFILE_ID, Broadcasting profile in avs+ */
        (!pDecContainer->avsPlusSupport && tmp == 0x48)) /* Check HW support for AVS+ */
    {
        AVSDEC_DEBUG(("UNSUPPORTED PROFILE 0x%x\n", tmp));
        pDecContainer->StrmStorage.unsupportedFeaturesPresent = 1;
        return HANTRO_NOK;
    }

    pHdr->profileId = tmp;

    /* level_id */
    tmp = pHdr->levelId = AvsStrmDec_GetBits(pDecContainer, 8);

    tmp = pHdr->progressiveSequence = AvsStrmDec_GetBits(pDecContainer, 1);

    tmp = pHdr->horizontalSize = AvsStrmDec_GetBits(pDecContainer, 14);
    if(!tmp)
        return (HANTRO_NOK);

    tmp = pHdr->verticalSize = AvsStrmDec_GetBits(pDecContainer, 14);
    if(!tmp)
        return (HANTRO_NOK);

    tmp = pHdr->chromaFormat = AvsStrmDec_GetBits(pDecContainer, 2);
    if (pHdr->chromaFormat != 1) /* only 4:2:0 supported */
    {
        AVSDEC_DEBUG(("UNSUPPORTED CHROMA_FORMAT\n"));
        pDecContainer->StrmStorage.unsupportedFeaturesPresent = 1;
        return (HANTRO_NOK);
    }

    /* sample_precision, shall be 8-bit */
    tmp = AvsStrmDec_GetBits(pDecContainer, 3);
    if (tmp != 1)
    {
        pDecContainer->StrmStorage.unsupportedFeaturesPresent = 1;
        return (HANTRO_NOK);
    }

    tmp = pHdr->aspectRatio = AvsStrmDec_GetBits(pDecContainer, 4);
    tmp = pHdr->frameRateCode = AvsStrmDec_GetBits(pDecContainer, 4);

    /* bit_rate_lower */
    tmp = pHdr->bitRateValue = AvsStrmDec_GetBits(pDecContainer, 18);

    /* marker */
    tmp = AvsStrmDec_GetBits(pDecContainer, 1);

    /* bit_rate_upper */
    tmp = AvsStrmDec_GetBits(pDecContainer, 12);
    pHdr->bitRateValue |= tmp << 18;

    tmp = pDecContainer->Hdrs.lowDelay = AvsStrmDec_GetBits(pDecContainer, 1);

    /* marker */
    tmp = AvsStrmDec_GetBits(pDecContainer, 1);

    tmp = pHdr->bbvBufferSize = AvsStrmDec_GetBits(pDecContainer, 18);

    /* reserved_bits, shall be '000', not checked */
    tmp = AvsStrmDec_GetBits(pDecContainer, 3);

    if(pDecContainer->StrmStorage.strmDecReady)
    {
        if (pHdr->horizontalSize != pDecContainer->Hdrs.horizontalSize ||
            pHdr->verticalSize != pDecContainer->Hdrs.verticalSize)
        {
            pDecContainer->ApiStorage.firstHeaders = 1;
            pDecContainer->StrmStorage.strmDecReady = HANTRO_FALSE;
            /* delayed resolution change */
            if (!pDecContainer->StrmStorage.sequenceLowDelay)
            {
                pDecContainer->StrmStorage.newHeadersChangeResolution = 1;
            }
            else
            {
                pDecContainer->Hdrs.horizontalSize = pHdr->horizontalSize;
                pDecContainer->Hdrs.verticalSize = pHdr->verticalSize;
                pDecContainer->Hdrs.aspectRatio = pHdr->aspectRatio;
                pDecContainer->Hdrs.frameRateCode =
                    pHdr->frameRateCode;
                pDecContainer->Hdrs.bitRateValue =
                    pHdr->bitRateValue;
            }
        }

        if (pDecContainer->StrmStorage.sequenceLowDelay &&
            !pDecContainer->Hdrs.lowDelay)
            pDecContainer->StrmStorage.sequenceLowDelay = 0;

    }
    else
        pDecContainer->StrmStorage.sequenceLowDelay =
            pDecContainer->Hdrs.lowDelay;

    pDecContainer->StrmStorage.frameWidth =
        (pDecContainer->Hdrs.horizontalSize + 15) >> 4;

    pDecContainer->StrmStorage.frameHeight =
        (pDecContainer->Hdrs.verticalSize + 15) >> 4;

/*    if(pDecContainer->Hdrs.progressiveSequence)
        pDecContainer->StrmStorage.frameHeight =
            (pDecContainer->Hdrs.verticalSize + 15) >> 4;
    else
        pDecContainer->StrmStorage.frameHeight =
            2 * ((pDecContainer->Hdrs.verticalSize + 31) >> 5);*/

    pDecContainer->StrmStorage.totalMbsInFrame =
        (pDecContainer->StrmStorage.frameWidth *
         pDecContainer->StrmStorage.frameHeight);

    AVSDEC_DEBUG(("Decode Sequence Header Done\n"));

    return (HANTRO_OK);
}

/*------------------------------------------------------------------------------

   5.2  Function name:
                AvsStrmDec_GenWeightQuantParam

        Purpose:

        Input:

        Output:

------------------------------------------------------------------------------*/
u32 AvsStrmDec_GenWeightQuantParam(DecHdrs *pHdr)
{
    const u32 weightingQuantParamDefault[] = {128, 98, 106, 116, 116, 128};
    const u32 weightingQuantParamBase1[]   = {135, 143, 143, 160, 160, 213};
    const u32 weightingQuantParamBase2[]   = {128, 98, 106, 116, 116, 128};
    u32 *wqP = (u32 *)pHdr->weightingQuantParam;
    u32 i;

    if (pHdr->weightingQuantFlag == 0)
    {
        /* needn't generate this param */
        for(i=0; i<6; i++)
        {
            wqP[i] = 128;
        }
        return 0;
    }

    if(pHdr->weightingQuantParamIndex == 0x0)
    {
        for(i=0; i<6; i++)
        {
            wqP[i] = weightingQuantParamDefault[i];
        }
    }
    else if (pHdr->weightingQuantParamIndex == 0x1)
    {
        for(i=0; i<6; i++)
        {
            wqP[i] = weightingQuantParamBase1[i] +
                    pHdr->weightingQuantParamDelta1[i];
        }
    }
    else if (pHdr->weightingQuantParamIndex == 0x2)
    {
        for(i=0; i<6; i++)
        {
            wqP[i] = weightingQuantParamBase2[i] +
                    pHdr->weightingQuantParamDelta2[i];
        }
    }
    else
    {
        /* shouldn't happen */
        AVSDEC_DEBUG(("AvsStrmDec_GenWeightQuantParam: Something went wrong!\n"));
        for(i=0; i<6; i++)
        {
            wqP[i] = 128;
        }
    }

    return 1;
}

/*------------------------------------------------------------------------------

   5.2  Function name:
                AvsStrmDec_DecodeIPictureHeader

        Purpose:

        Input:

        Output:

------------------------------------------------------------------------------*/
u32 AvsStrmDec_DecodeIPictureHeader(DecContainer * pDecContainer)
{
    u32 tmp, val;
    DecHdrs *pHdr;

    ASSERT(pDecContainer);

    AVSDEC_DEBUG(("Decode I Picture Header Start\n"));

    pHdr = &pDecContainer->Hdrs;

    pHdr->picCodingType = IFRAME;

    /* bbv_delay */
    tmp = AvsStrmDec_GetBits(pDecContainer, 16);

    if (pHdr->profileId == 0x48) /* broadcast profile in avs+ */
    {
        /* marker_bit, its value should be 1 */
        tmp = AvsStrmDec_GetBits(pDecContainer, 1);
        /* bbv_delay_extension */
        tmp = AvsStrmDec_GetBits(pDecContainer, 7);
    }

    /* time_code_flag */
    tmp = AvsStrmDec_GetBits(pDecContainer, 1);
    if (tmp)
    {
        /* time_code */
        tmp = AvsStrmDec_GetBits(pDecContainer, 1); /* DropFrameFlag */
        tmp = AvsStrmDec_GetBits(pDecContainer, 5); /* TimeCodeHours */
        pHdr->timeCode.hours = tmp;
        tmp = AvsStrmDec_GetBits(pDecContainer, 6); /* TimeCodeMinutes */
        pHdr->timeCode.minutes = tmp;
        tmp = AvsStrmDec_GetBits(pDecContainer, 6); /* TimeCodeSeconds */
        pHdr->timeCode.seconds = tmp;
        tmp = AvsStrmDec_GetBits(pDecContainer, 6); /* TimeCodePictures */
        pHdr->timeCode.picture = tmp;
    }

    tmp = AvsStrmDec_GetBits(pDecContainer, 1);

    tmp = pHdr->pictureDistance = AvsStrmDec_GetBits(pDecContainer, 8);

    if (pHdr->lowDelay)
        /* bbv_check_times */
        tmp = AvsDecodeExpGolombUnsigned(pDecContainer, &val);

    tmp = pHdr->progressiveFrame = AvsStrmDec_GetBits(pDecContainer, 1);

    if (!tmp)
    {
        tmp = pHdr->pictureStructure = AvsStrmDec_GetBits(pDecContainer, 1);
    }
    else pHdr->pictureStructure = FRAMEPICTURE;

    tmp = pHdr->topFieldFirst = AvsStrmDec_GetBits(pDecContainer, 1);
    tmp = pHdr->repeatFirstField = AvsStrmDec_GetBits(pDecContainer, 1);
    tmp = pHdr->fixedPictureQp = AvsStrmDec_GetBits(pDecContainer, 1);
    tmp = pHdr->pictureQp = AvsStrmDec_GetBits(pDecContainer, 6);

    if (pHdr->progressiveFrame == 0 && pHdr->pictureStructure == 0)
        tmp = pHdr->skipModeFlag = AvsStrmDec_GetBits(pDecContainer, 1);

    /* reserved_bits, shall be '0000', not checked */
    tmp = AvsStrmDec_GetBits(pDecContainer, 4);

    tmp = pHdr->loopFilterDisable = AvsStrmDec_GetBits(pDecContainer, 1);
    if (!tmp)
    {
        tmp = AvsStrmDec_GetBits(pDecContainer, 1);
        if (tmp)
        {
            tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
            pHdr->alphaOffset = (i32)val;
            if (pHdr->alphaOffset < -8 || pHdr->alphaOffset > 8)
                return (HANTRO_NOK);
            tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
            pHdr->betaOffset = (i32)val;
            if (pHdr->betaOffset < -8 || pHdr->betaOffset > 8)
                return (HANTRO_NOK);
        }
    }

    /* for AvsSetRegs() setting reg convenience  */
    pHdr->noForwardReferenceFlag = 0;
    pHdr->pbFieldEnhancedFlag = 0;
    pHdr->weightingQuantFlag = 0;
    pHdr->aecEnable = 0;

    if (pHdr->profileId == 0x48) /* broadcast profile in avs+ */
    {
        /* weighting_quant_flag */
        tmp = AvsStrmDec_GetBits(pDecContainer, 1);
        pHdr->weightingQuantFlag = tmp;
        if(tmp == 0x1)
        {
            u32 i;
            /* reserved_bits, shall be '0', not checked */
            tmp = AvsStrmDec_GetBits(pDecContainer, 1);
            /* chroma_quant_param_disable */
            tmp = AvsStrmDec_GetBits(pDecContainer, 1);
            pHdr->chromaQuantParamDisable = tmp;
            if(tmp == 0x0)
            {
                /* chroma_quant_param_delta_cb */
                tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
                pHdr->chromaQuantParamDeltaCb = (i32)val;
                /* chroma_quant_param_delta_cr */
                tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
                pHdr->chromaQuantParamDeltaCr = (i32)val;
            }

            /* weighting_quant_param_index */
            tmp = AvsStrmDec_GetBits(pDecContainer, 2);
            pHdr->weightingQuantParamIndex = tmp;
            /* weighting_quant_model */
            tmp = AvsStrmDec_GetBits(pDecContainer, 2);
            pHdr->weightingQuantModel = (tmp == 0x3) ? 0 : tmp;

            if(pHdr->weightingQuantParamIndex == 0x1)
            {
                for(i=0; i<6; i++)
                {
                    tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
                    pHdr->weightingQuantParamDelta1[i] = (i32)val;
                }
            }
            else
            {
                for(i=0; i<6; i++)
                {
                    pHdr->weightingQuantParamDelta1[i] = 0;
                }
            }

            if(pHdr->weightingQuantParamIndex == 0x2)
            {
                for(i=0; i<6; i++)
                {
                    tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
                    pHdr->weightingQuantParamDelta2[i] = (i32)val;
                }
            }
            else
            {
                for(i=0; i<6; i++)
                {
                    pHdr->weightingQuantParamDelta2[i] = 0;
                }
            }
        }

        /* generate wqP[m][6] */
        AvsStrmDec_GenWeightQuantParam(pHdr);

        /* aec_enable */
        pHdr->aecEnable = AvsStrmDec_GetBits(pDecContainer, 1);
    }

    return (HANTRO_OK);
}

/*------------------------------------------------------------------------------

   5.2  Function name:
                AvsStrmDec_DecodePBPictureHeader

        Purpose:

        Input:

        Output:

------------------------------------------------------------------------------*/
u32 AvsStrmDec_DecodePBPictureHeader(DecContainer * pDecContainer)
{
    u32 tmp, val;
    DecHdrs *pHdr;

    ASSERT(pDecContainer);

    AVSDEC_DEBUG(("Decode PB Picture Header Start\n"));

    pHdr = &pDecContainer->Hdrs;

    /* bbv_delay */
    tmp = AvsStrmDec_GetBits(pDecContainer, 16);

    if (pHdr->profileId == 0x48) /* broadcast profile in avs+ */
    {
        /* marker_bit, its value should be 1 */
        tmp = AvsStrmDec_GetBits(pDecContainer, 1);
        /* bbv_delay_extension */
        tmp = AvsStrmDec_GetBits(pDecContainer, 7);
    }

    tmp = pHdr->picCodingType = AvsStrmDec_GetBits(pDecContainer, 2)+1;
    if (tmp != PFRAME && tmp != BFRAME)
        return (HANTRO_NOK);

    tmp = pHdr->pictureDistance = AvsStrmDec_GetBits(pDecContainer, 8);

    if (pHdr->lowDelay)
        /* bbv_check_times */
        tmp = AvsDecodeExpGolombUnsigned(pDecContainer, &val);

    tmp = pHdr->progressiveFrame = AvsStrmDec_GetBits(pDecContainer, 1);

    if (!tmp)
    {
        tmp = pHdr->pictureStructure = AvsStrmDec_GetBits(pDecContainer, 1);
        if (tmp == 0)
            tmp = pHdr->advancedPredModeDisable =
                AvsStrmDec_GetBits(pDecContainer, 1);
    }
    else pHdr->pictureStructure = FRAMEPICTURE;

    tmp = pHdr->topFieldFirst = AvsStrmDec_GetBits(pDecContainer, 1);
    tmp = pHdr->repeatFirstField = AvsStrmDec_GetBits(pDecContainer, 1);
    tmp = pHdr->fixedPictureQp = AvsStrmDec_GetBits(pDecContainer, 1);
    tmp = pHdr->pictureQp = AvsStrmDec_GetBits(pDecContainer, 6);

    if (!(pHdr->picCodingType == BFRAME && pHdr->pictureStructure == 1))
    {
        tmp = pHdr->pictureReferenceFlag = AvsStrmDec_GetBits(pDecContainer, 1);
    }

    if (pHdr->profileId == 0x48)
    {
        /* no_forward_reference_flag */
        pHdr->noForwardReferenceFlag = AvsStrmDec_GetBits(pDecContainer, 1);
        /* pb_field_enhanced_flag */
        pHdr->pbFieldEnhancedFlag = AvsStrmDec_GetBits(pDecContainer, 1);
    }
    else
    {
        /* no_forward_reference_flag */
        pHdr->noForwardReferenceFlag = AvsStrmDec_GetBits(pDecContainer, 1);
        /* TODO AVS should be confirmed? */
        pHdr->noForwardReferenceFlag = 0;
        /* pb_field_enhanced_flag */
        pHdr->pbFieldEnhancedFlag = AvsStrmDec_GetBits(pDecContainer, 1);
        pHdr->pbFieldEnhancedFlag = 0;
    }
    /* reserved_bits, shall be '00', not checked */
    tmp = AvsStrmDec_GetBits(pDecContainer, 2);

    tmp = pHdr->skipModeFlag = AvsStrmDec_GetBits(pDecContainer, 1);

    tmp = pHdr->loopFilterDisable = AvsStrmDec_GetBits(pDecContainer, 1);
    if (!tmp)
    {
        tmp = AvsStrmDec_GetBits(pDecContainer, 1);
        if (tmp)
        {
            tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
            pHdr->alphaOffset = (i32)val;
            if (pHdr->alphaOffset < -8 || pHdr->alphaOffset > 8)
                return (HANTRO_NOK);
            tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
            pHdr->betaOffset = (i32)val;
            if (pHdr->betaOffset < -8 || pHdr->betaOffset > 8)
                return (HANTRO_NOK);
        }
    }

    pHdr->weightingQuantFlag = 0;
    pHdr->aecEnable = 0;

    if (pHdr->profileId == 0x48) /* broadcast profile in avs+ */
    {
        /* weighting_quant_flag */
        tmp = AvsStrmDec_GetBits(pDecContainer, 1);
        pHdr->weightingQuantFlag = tmp;
        if (tmp == 0x1)
        {
            u32 i;
            /* reserved_bits, shall be '0', not checked */
            tmp = AvsStrmDec_GetBits(pDecContainer, 1);
            /* chroma_quant_param_disable */
            tmp = AvsStrmDec_GetBits(pDecContainer, 1);
            pHdr->chromaQuantParamDisable = tmp;
            if (tmp == 0x0)
            {
                /* chroma_quant_param_delta_cb */
                tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
                pHdr->chromaQuantParamDeltaCb = (i32)val;
                /* chroma_quant_param_delta_cr */
                tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
                pHdr->chromaQuantParamDeltaCr = (i32)val;
            }

            /* weighting_quant_param_index */
            tmp = AvsStrmDec_GetBits(pDecContainer, 2);
            pHdr->weightingQuantParamIndex = tmp;
            /* weighting_quant_model */
            tmp = AvsStrmDec_GetBits(pDecContainer, 2);
            pHdr->weightingQuantModel = (tmp == 0x3) ? 0 : tmp;

            if (pHdr->weightingQuantParamIndex == 0x1)
            {
                for(i=0; i<6; i++)
                {
                    tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
                    pHdr->weightingQuantParamDelta1[i] = (i32)val;
                }
            }

            if (pHdr->weightingQuantParamIndex == 0x2)
            {
                for(i=0; i<6; i++)
                {
                    tmp = AvsDecodeExpGolombSigned(pDecContainer, (i32*)&val);
                    pHdr->weightingQuantParamDelta2[i] = (i32)val;
                }
            }
        }

        /* generate wqP[m][6] */
        AvsStrmDec_GenWeightQuantParam(pHdr);

        /* aec_enable */
        pHdr->aecEnable = AvsStrmDec_GetBits(pDecContainer, 1);
    }

    return (HANTRO_OK);
}

/*------------------------------------------------------------------------------

   5.1  Function name:
                AvsStrmDec_DecodeExtensionHeader

        Purpose:
                Decodes AVS extension headers

        Input:
                pointer to DecContainer

        Output:
                status (OK/NOK/END_OF_STREAM/... enum in .h file!)

------------------------------------------------------------------------------*/
u32 AvsStrmDec_DecodeExtensionHeader(DecContainer * pDecContainer)
{
    u32 extensionStartCode;
    u32 status = HANTRO_OK;

    /* get extension header ID */
    extensionStartCode = AvsStrmDec_GetBits(pDecContainer, 4);

    switch (extensionStartCode)
    {
        case SC_SEQ_DISPLAY_EXT:
            /* sequence display extension header */
            status = AvsStrmDec_DecodeSeqDisplayExtHeader(pDecContainer);
            break;

        default:
            break;
    }

    return (status);
}

/*------------------------------------------------------------------------------

   5.2  Function name:
                AvsStrmDec_DecodeSeqDisplayExtHeader

        Purpose:

        Input:

        Output:

------------------------------------------------------------------------------*/
u32 AvsStrmDec_DecodeSeqDisplayExtHeader(DecContainer * pDecContainer)
{
    u32 tmp;

    ASSERT(pDecContainer);

    AVSDEC_DEBUG(("Decode Sequence Display Extension Header Start\n"));

    tmp = pDecContainer->Hdrs.videoFormat =
        AvsStrmDec_GetBits(pDecContainer, 3);
    tmp = pDecContainer->Hdrs.sampleRange =
        AvsStrmDec_GetBits(pDecContainer, 1);
    tmp = pDecContainer->Hdrs.colorDescription =
        AvsStrmDec_GetBits(pDecContainer, 1);

    if(pDecContainer->Hdrs.colorDescription)
    {
        tmp = pDecContainer->Hdrs.colorPrimaries =
            AvsStrmDec_GetBits(pDecContainer, 8);
        tmp = pDecContainer->Hdrs.transferCharacteristics =
            AvsStrmDec_GetBits(pDecContainer, 8);
        tmp = pDecContainer->Hdrs.matrixCoefficients =
            AvsStrmDec_GetBits(pDecContainer, 8);
    }

    tmp = pDecContainer->Hdrs.displayHorizontalSize =
        AvsStrmDec_GetBits(pDecContainer, 14);

    /* marker bit ==> flush */
    tmp = AvsStrmDec_FlushBits(pDecContainer, 1);

    tmp = pDecContainer->Hdrs.displayVerticalSize =
        AvsStrmDec_GetBits(pDecContainer, 14);

    /* reserved_bits */
    tmp = AvsStrmDec_GetBits(pDecContainer, 2);

    if(tmp == END_OF_STREAM)
        return (END_OF_STREAM);

    AVSDEC_DEBUG(("Decode Sequence Display Extension Header Done\n"));

    return (HANTRO_OK);
}
