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
--  Description : 
--
------------------------------------------------------------------------------*/

#ifndef REGDRV_H
#define REGDRV_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

#define DEC_8170_IRQ_RDY            0x02
#define DEC_8170_IRQ_BUS            0x04
#define DEC_8170_IRQ_BUFFER         0x08
#define DEC_8170_IRQ_ASO            0x10
#define DEC_8170_IRQ_ERROR          0x20
#define DEC_8170_IRQ_SLICE          0x40
#define DEC_8170_IRQ_TIMEOUT        0x80

#define DEC_8190_IRQ_ABORT          0x01
#define DEC_8190_IRQ_RDY            DEC_8170_IRQ_RDY
#define DEC_8190_IRQ_BUS            DEC_8170_IRQ_BUS
#define DEC_8190_IRQ_BUFFER         DEC_8170_IRQ_BUFFER
#define DEC_8190_IRQ_ASO            DEC_8170_IRQ_ASO
#define DEC_8190_IRQ_ERROR          DEC_8170_IRQ_ERROR
#define DEC_8190_IRQ_SLICE          DEC_8170_IRQ_SLICE
#define DEC_8190_IRQ_TIMEOUT        DEC_8170_IRQ_TIMEOUT

#define DEC_IRQ_DISABLE             0x10
#define DEC_ABORT                   0x20

#define PP_REG_START                60

typedef enum
{
/* include script-generated part */
#include "8170enum.h"
    HWIF_DEC_IRQ_STAT,
    HWIF_PP_IRQ_STAT,
    HWIF_LAST_REG,

    /* aliases */
    HWIF_MPEG4_DC_BASE = HWIF_I4X4_OR_DC_BASE,
    HWIF_INTRA_4X4_BASE = HWIF_I4X4_OR_DC_BASE,
    /* VP6 */
    HWIF_VP6HWGOLDEN_BASE = HWIF_REFER4_BASE,
    HWIF_VP6HWPART1_BASE = HWIF_REFER13_BASE,
    HWIF_VP6HWPART2_BASE = HWIF_RLC_VLC_BASE,
    HWIF_VP6HWPROBTBL_BASE = HWIF_QTABLE_BASE,
    /* progressive JPEG */
    HWIF_PJPEG_COEFF_BUF = HWIF_DIR_MV_BASE,

    /* MVC */
    HWIF_INTER_VIEW_BASE = HWIF_REFER15_BASE

} hwIfName_e;

/*------------------------------------------------------------------------------
    Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

void SetDecRegister(u32 *regBase, u32 id, u32 value);
u32 GetDecRegister(const u32 *regBase, u32 id);

void SetPpRegister(u32 *ppRegBase, u32 id, u32 value);
u32 GetPpRegister(const u32 *ppRegBase, u32 id);

#endif /* #ifndef REGDRV_H */
