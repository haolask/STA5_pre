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
--  Abstract : Header file for debug traces
--
------------------------------------------------------------------------------*/

#ifndef MP4DEBUG_H_DEFINED
#define MP4DEBUG_H_DEFINED

#ifdef MP4DEC_TRACE
#define MP4_API_TRC(str)    MP4DecTrace(str)
#else
#define MP4_API_TRC(str)
#endif
#define MP4DEC_API_DEBUG DEBUG_PRINT

#define MP4DEC_DEBUG DEBUG_PRINT

#endif
