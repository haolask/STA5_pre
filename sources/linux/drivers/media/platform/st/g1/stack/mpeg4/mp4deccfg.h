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
--  Abstract : Global configurations.
--
------------------------------------------------------------------------------*/

#ifndef _DEC_CFG_H
#define _DEC_CFG_H

/*
 *  Maximum number of macro blocks in one VOP
 */
#define MP4_MIN_WIDTH   48

#define MP4_MIN_HEIGHT  48

#define MP4API_DEC_MBS  8160

/*
 * Size of RLC buffer per macro block
 */
#ifndef _MP4_RLC_BUFFER_SIZE
    #define _MP4_RLC_BUFFER_SIZE 384
#endif


#endif /* already included */
