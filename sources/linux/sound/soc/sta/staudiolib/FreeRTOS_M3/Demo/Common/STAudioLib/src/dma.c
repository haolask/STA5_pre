/***********************************************************************************/
/*!
*
*  \file      dma.c
*
*  \brief     <i><b> STAudioLib DMABUS manager </b></i>
*
*  \details   Manages the DMABUS, internal Sound Subsystem bus
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2013/04/22
*
*  \bug       see Readme.txt
*
*  \warning
*
*  This file is part of STAudioLib and is dual licensed,
*  ST Proprietary License or GNU General Public License version 2.
*
********************************************************************************
*
* Copyright (c) 2014 STMicroelectronics - All Rights Reserved
* Reproduction and Communication of this document is strictly prohibited
* unless specifically authorized in writing by STMicroelectronics.
* FOR MORE INFORMATION PLEASE READ CAREFULLY THE LICENSE AGREEMENT LOCATED
* IN THE ROOT DIRECTORY OF THIS SOFTWARE PACKAGE.
*
********************************************************************************
*
* ALTERNATIVELY, this software may be distributed under the terms of the
* GNU General Public License ("GPL") version 2, in which case the following
* provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* STAudioLib is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* STAudioLib is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*
*/
/***********************************************************************************/

#include "internal.h"



void DMAB_Reset(tDMABUS* dma)
{
	sta_assert(dma);
	if (!dma) {goto _ret;}

	DMA_RUN(0);							//force stop
	DMA_GATING(1);						//?

	_STA_memset32(DMA_TX_BASE, 0, STA_DMA_SIZE*4);
	dma->m_numt = 0;
	dma->m_pTx  = DMA_TX_BASE; //mainly to indicates that the DMA was reseted properly

_ret: return;
}
//----------------------------------------------------------------------------

//from CUT2
//#define DMA_TYPE_FSYNC	0xF

//returns last pos
u32 DMAB_SetTransfers(tDMABUS* dma, u16 srcAddr, u16 dstAddr, u32 nch, u32 dmaPos, u32 ttype)
{
	u32 srcShift;
	u32 src, dst;
	u32 i;

	sta_assert(dma);
	if (!dma) {goto _ret;}

	//CUT1 DMABUS_TCM format
	if ((g_drv.m_cutVersion & 0xFFF) < 0x200) {
		src 	 = srcAddr;
		dst 	 = dstAddr;
		srcShift = 16;
		ttype    = 0;	//CUT1: force ttype = FSYNC 48Khz
	}
	//CUT2 DMABUS_TCM (new) format
	else {
		src  	 = ((srcAddr & 0xF000) >> 2) | (srcAddr & 0x03FF);
		dst  	 = ((dstAddr & 0xF000) >> 2) | (dstAddr & 0x03FF);
		srcShift = 14;
//		ttype    = (ttype & 0xF) << 28;
	}

	for (i = 0; i < nch; i++) {
		dma->m_pTx[dmaPos++] = ttype | src << srcShift | dst;

		if (srcAddr != STA_DMA_FIFO_OUT) {
			src++;}
		if (dstAddr != STA_DMA_FIFO_IN) {
			dst++;}
	}

_ret: return dmaPos;
}
//----------------------------------------------------------------------------
STA_ErrorCode DMAB_CheckAddress(u16 src, u16 dst, u32 numCh)
{

#if 0  //disabled to save time and memory....

	//check stereo source
	//(note: all left channels have even address except for ADC, that we check in the default)
	switch (src & 0xFFFE) {
	case STA_DMA_LPF0_DI:
	case STA_DMA_LPF1_DI:
	case STA_DMA_LPF2_DI:
	case STA_DMA_LPF3_DI:
	case STA_DMA_LPF4_DI:
	case STA_DMA_LPF5_DI:
	case STA_DMA_SRC0_DO:
	case STA_DMA_SRC1_DO:
	case STA_DMA_SRC2_DO:
	case STA_DMA_SRC3_DO:
	case STA_DMA_SRC4_DO:
	case STA_DMA_SRC5_DO:
	case STA_DMA_SAI1_RX1:
	case STA_DMA_SAI2_RX1:
	case STA_DMA_SAI2_RX2:
	case STA_DMA_SAI2_RX3:
	case STA_DMA_SAI2_RX4:
	case STA_DMA_SAI3_RX1:
	case STA_DMA_SAI3_RX2:
	case STA_DMA_SAI3_RX3:
	case STA_DMA_SAI3_RX4:
	case STA_DMA_SAI4_RX1:
	case STA_DMA_SAI4_RX2:
	case STA_DMA_SAI4_RX3:
	case STA_DMA_SAI4_RX4: if (numCh > 2 || (numCh == 2 && (src & 1))) return STA_INVALID_CHANNEL; break;

	case STA_DMA_FIFO_OUT: if (numCh > 1) return STA_INVALID_CHANNEL; break;

	default: //XOUT
		if ( (src < STA_DMA_DSP0_XOUT || src+numCh > STA_DMA_DSP0_XOUT + STA_XOUT_SIZE) &&
			 (src < STA_DMA_DSP1_XOUT || src+numCh > STA_DMA_DSP1_XOUT + STA_XOUT_SIZE) &&
			 (src < STA_DMA_DSP2_XOUT || src+numCh > STA_DMA_DSP2_XOUT + STA_XOUT_SIZE) &&
			 (src < STA_DMA_ADC_DATA  || src+numCh > STA_DMA_ADC_GPIN) )
			return STA_INVALID_DMA_SRC_ADDR; break;
	}

	//check stereo dest
	switch (dst & 0xFFFE) {
	case STA_DMA_LPF0_DI:
	case STA_DMA_LPF1_DI:
	case STA_DMA_LPF2_DI:
	case STA_DMA_LPF3_DI:
	case STA_DMA_LPF4_DI:
	case STA_DMA_LPF5_DI:
	case STA_DMA_SAI2_TX1:
	case STA_DMA_SAI3_TX1:
	case STA_DMA_SAI3_TX2:
	case STA_DMA_SAI3_TX3:
	case STA_DMA_SAI3_TX4:
	case STA_DMA_SAI4_TX1:
	case STA_DMA_SAI4_TX2:
	case STA_DMA_SAI4_TX3:
	case STA_DMA_SAI4_TX4: if (numCh > 2 || (numCh == 2 && (dst & 1))) return STA_INVALID_CHANNEL; break;

	case STA_DMA_FIFO_IN:  if (numCh > 1) return STA_INVALID_CHANNEL; break;

	default: //XIN
		if ( (dst < STA_DMA_DSP0_XIN || dst+numCh > STA_DMA_DSP0_XIN + STA_XIN_SIZE) &&
			 (dst < STA_DMA_DSP1_XIN || dst+numCh > STA_DMA_DSP1_XIN + STA_XIN_SIZE) &&
			 (dst < STA_DMA_DSP2_XIN || dst+numCh > STA_DMA_DSP2_XIN + STA_XIN_SIZE))
			return STA_INVALID_DMA_DST_ADDR; break;
	}
#else
	(void) src; (void) dst; (void) numCh;
#endif

	return STA_NO_ERROR;
}

STA_ErrorCode DMAB_CheckType(u32 type)
{
	STA_ErrorCode ret = STA_NO_ERROR;

	switch (type)
	{
	case STA_DMA_FSCK1_SLOT0:
	case STA_DMA_FSCK1_SLOT1:
	case STA_DMA_FSCK1_SLOT2:
	case STA_DMA_FSCK1_SLOT3:
	case STA_DMA_FSCK1_SLOT4:
	case STA_DMA_FSCK1_SLOT5:
	case STA_DMA_FSCK2_SLOT0:
	case STA_DMA_FSCK2_SLOT1:
	case STA_DMA_FSCK2_SLOT2:
	case STA_DMA_FSCK2_SLOT3:
	case STA_DMA_FSCK2_SLOT4:
	case STA_DMA_FSCK2_SLOT5:
	case STA_DMA_FSYNC:
		break;
	default:
		ret = STA_INVALID_DMA_TYPE;
		break;
	}
	return ret;
}


