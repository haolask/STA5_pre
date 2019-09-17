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
--  Abstract : Stream decoding top header file
--
------------------------------------------------------------------------------*/

#ifndef AVSSTRMDEC_H_DEFINED
#define AVSSTRMDEC_H_DEFINED

#include "avs_container.h"

/* StrmDec_Decode() function return values
 * DEC_RDY -> everything ok but no frames finished
 * DEC_PIC_RDY -> pic finished
 * DEC_PIC_RDY_BUF_NOT_EMPTY -> pic finished but stream buffer not empty
 * DEC_ERROR -> nothing finished, decoder not ready, cannot continue decoding
 *      before some headers received
 * DEC_ERROR_BUF_NOT_EMPTY -> same as above but stream buffer not empty
 * DEC_END_OF_STREAM -> nothing finished, no data left in stream
 * DEC_OUT_OF_BUFFER -> out of rlc buffer
 * DEC_HDRS_RDY -> either vol header decoded or short video source format
 *      determined
 * DEC_HDRS_RDY_BUF_NOT_EMPTY -> same as above but stream buffer not empty
 *
 * Bits in the output have following meaning:
 * 0    ready
 * 1    frame ready
 * 2    buffer not empty
 * 3    error
 * 4    end of stream
 * 5    out of buffer
 * 6    video object sequence end
 * 7    headers ready
 */
enum
{
    DEC_RDY = 0x01,
    DEC_PIC_RDY = 0x03,
    DEC_PIC_RDY_BUF_NOT_EMPTY = 0x07,
    DEC_ERROR = 0x08,
    DEC_ERROR_BUF_NOT_EMPTY = 0x0C,
    DEC_END_OF_STREAM = 0x10,
    DEC_OUT_OF_BUFFER = 0x20,
    DEC_HDRS_RDY = 0x80,
    DEC_HDRS_RDY_BUF_NOT_EMPTY = 0x84,
    DEC_PIC_HDR_RDY = 0x100,
    DEC_PIC_HDR_RDY_ERROR = 0x108,
    DEC_PIC_SUPRISE_B = 0x1000
};

/* function prototypes */
u32 AvsStrmDec_Decode(DecContainer * pDecContainer);

#endif /* #ifndef AVSSTRMDEC_H_DEFINED */
