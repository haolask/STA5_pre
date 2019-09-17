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

#ifndef MPEG2DEBUG_H_DEFINED
#define MPEG2DEBUG_H_DEFINED

#ifdef MPEG2DEC_TRACE
#define MPEG2_API_TRC(str)    Mpeg2DecTrace(str)
#else
#define MPEG2_API_TRC(str)
#endif
#define MPEG2DEC_API_DEBUG DEBUG_PRINT

#define MPEG2DEC_DEBUG DEBUG_PRINT

#define MPEG2FLUSH
#endif
