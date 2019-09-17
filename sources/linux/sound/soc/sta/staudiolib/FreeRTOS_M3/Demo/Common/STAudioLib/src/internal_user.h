/***********************************************************************************/
/*!
*
*  \file      internal_user.h
*
*  \brief     <i><b> STAudioLib internal unser functions </b></i>
*
*  \details   Internal functions for conversions....
*
*
*  \author    Douvenot Olivier
*
*  \author    (original version) Douvenot Olivier
*
*  \version
*
*  \date      2017/06/02
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
* Copyright (c) 2017 STMicroelectronics - All Rights Reserved
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

#ifndef INTERNAL_USER_H_
#define INTERNAL_USER_H_


//---Internal Headers -----------------------------------------


//---- DSP MACROS --------------------------------------------------------------


#define DSP_MEMSET				(_DSP_memset)
#define DSP_MEMCPY				(_DSP_memcpy)

//lint -emacro(960, DSP_READ, DSP_WRITE)  inhibits misra2004 11.5 "cast away const/volatile from ptr" (required to get address offset)

#ifdef LINUX_OS
#define DSP_READ 				(dsp_readl)
#define DSP_WRITE(addr, value)	dsp_writel((volatile void *)addr, (u32)(value))
#define DSP_ENABLE_CLOCK(core)	dsp_clken(g_drv.m_dev, core)
#define DSP_DISABLE_CLOCK(core)	dsp_clkdis(g_drv.m_dev, core)
#define DSP_START(core)			dsp_start(g_drv.m_dev, core)
#define DSP_STOP(core)			dsp_stop(g_drv.m_dev, core)
#define DSP_CLOCK_IS_ENABLED(core) dsp_clk_is_en(g_drv.m_dev, core)
#define XIN_DMABUS_CLEAR(core) dsp_xin_dmabus_clear(g_drv.m_dev, core)
#define XOUT_DMABUS_CLEAR(core)
#else
#define DSP_READ(addr)			(*(const volatile s32*)(const volatile void*)(addr))
#define DSP_WRITE(addr, value)	(*(volatile s32*)(volatile void*)(addr) = (s32)(value))
#define DSP_ENABLE_CLOCK(core)	(AUDCR->ACO_CR.BIT.DSP_EnableClk |= (1u << (u32)(core)))
#define DSP_DISABLE_CLOCK(core)	(AUDCR->ACO_CR.BIT.DSP_EnableClk &= ~(1u << (u32)(core)))
#define DSP_START(core)			(AUDCR->ACO_CR.BIT.DSP_Start |= (1u << (u32)(core)))
#define DSP_STOP(core)			(AUDCR->ACO_CR.BIT.DSP_Start &= ~(1u << (u32)(core)))
#define DSP_CLOCK_IS_ENABLED(core) (AUDCR->ACO_CR.BIT.DSP_EnableClk & (1u << (u32)(core)))
#define XIN_DMABUS_CLEAR(core)	(_DSP_memset((volatile void*)XIN_DMABUS_ADDR(core), 0, (STA_XIN_SIZE-1) * 4))
#define XOUT_DMABUS_CLEAR(core)	(_DSP_memset((volatile void*)XOUT_DMABUS_ADDR(core), 0, STA_XOUT_SIZE * 4))
#endif

//DSP specific memset/memcpy (32bit access + "read-after-write" hack)
void 		_DSP_memset(volatile void* ptr, int value, unsigned int num);
void 		_DSP_memcpy(volatile void* dst, const void* src, unsigned int num);
#ifdef LINUX_OS
u32  		dsp_readl(const volatile void *addr);
int  		dsp_writel(volatile void *addr, u32 value);
int 		dsp_clken(u32 *dev, int core);
int 		dsp_clkdis(u32 *dev, int core);
int 		dsp_clk_is_en(u32 *dev, int core);
int 		dsp_start(u32 *dev, int core);
int 		dsp_stop(u32 *dev, int core);
int			dsp_xin_dmabus_clear(u32 *dev, int core);
#endif

#endif /* INTERNAL_USER_H_ */
