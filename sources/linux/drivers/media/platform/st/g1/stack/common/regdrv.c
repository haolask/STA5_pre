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

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "regdrv.h"

/*------------------------------------------------------------------------------
    External compiler flags
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

#ifdef _ASSERT_USED
#ifndef ASSERT
#include <assert.h>
#define ASSERT(expr) assert(expr)
#endif
#else
#define ASSERT(expr)
#endif

static const u32 regMask[33] = { 0x00000000,
    0x00000001, 0x00000003, 0x00000007, 0x0000000F,
    0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
    0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
    0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
    0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
    0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
    0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
    0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
};

/* { SWREG, BITS, POSITION } */
static const u32 hwDecRegSpec[HWIF_LAST_REG + 1][3] = {
/* include script-generated part */
#include "8170table.h"
/* HWIF_DEC_IRQ_STAT */ {1, 8, 11},
/* HWIF_PP_IRQ_STAT */ {60, 2, 12},
/* dummy entry */ {0, 0, 0}
};

/*------------------------------------------------------------------------------
    Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Function name: SetDecRegister

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
void SetDecRegister(u32 *regBase, u32 id, u32 value)
{
    u32 tmp;

    ASSERT(id < HWIF_LAST_REG);

    tmp = regBase[hwDecRegSpec[id][0]];
    tmp &= ~(regMask[hwDecRegSpec[id][1]] << hwDecRegSpec[id][2]);
    tmp |= (value & regMask[hwDecRegSpec[id][1]]) << hwDecRegSpec[id][2];
    regBase[hwDecRegSpec[id][0]] = tmp;
}

void SetPpRegister(u32 *ppRegBase, u32 id, u32 value)
{
    u32 tmp;

    ASSERT(id < HWIF_LAST_REG);

    /* hw reg spec is based on decoder so we have to adjust the register offset
     * because we are getting PP base register as parameter
     */

    tmp = ppRegBase[hwDecRegSpec[id][0] - PP_REG_START];
    tmp &= ~(regMask[hwDecRegSpec[id][1]] << hwDecRegSpec[id][2]);
    tmp |= (value & regMask[hwDecRegSpec[id][1]]) << hwDecRegSpec[id][2];
    ppRegBase[hwDecRegSpec[id][0] - PP_REG_START] = tmp;
}
/*------------------------------------------------------------------------------

    Function name: GetDecRegister

        Functional description:

        Inputs:

        Outputs:

        Returns: 

------------------------------------------------------------------------------*/
u32 GetDecRegister(const u32 *regBase, u32 id)
{
    u32 tmp;

    ASSERT(id < HWIF_LAST_REG);

    tmp = regBase[hwDecRegSpec[id][0]];
    tmp = tmp >> hwDecRegSpec[id][2];
    tmp &= regMask[hwDecRegSpec[id][1]];
    return (tmp);
}

u32 GetPpRegister(const u32 *ppRegBase, u32 id)
{
    u32 tmp;

    ASSERT(id < HWIF_LAST_REG);

    /* hw reg spec is based on decoder so we have to adjust the register offset
     * because we are getting PP base register as parameter
     */

    tmp = ppRegBase[hwDecRegSpec[id][0] - PP_REG_START];
    tmp = tmp >> hwDecRegSpec[id][2];
    tmp &= regMask[hwDecRegSpec[id][1]];
    return (tmp);
}
