/***********************************************************************************/
/*!
*
*  \file      dco.c
*
*  \brief     <i><b> STAudioLib DC Offset Detection Module </b></i>
*
*  \details   Manage the DCO detection implemented in Emerald DSP
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2015/04/22
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


//===========================================================================
// DCO DETECTION
//===========================================================================
static void DCO_InitDspData(tModule* mod);

void DCO_Init(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tDCO* const dco = mod->m_sub;

	u32 i, nin = mod->m_nin-1; //(-1: removing last channel dedicated to DCO_REG)

//	mod->AdjustDspMemSize = NULL;
	mod->InitDspData      = &DCO_InitDspData;
	mod->UpdateDspCoefs   = &DCO_UpdateDspCoefs;
//	mod->m_mode			  = STA_DCO_OFF; //(use dco[ch].m_mode instead because need per channel)
	mod->m_dirtyDspCoefs  = 0xFF;

	FORi(nin) {
		dco[i].m_mode        = (u32)STA_DCO_OFF;
		dco[i].m_scoreWindow = 5000;
		dco[i].m_alertThres  = 4000;
	}

	}
_ret: return;
}

static void DCO_InitDspData(tModule* mod)
{
	if (!mod) {goto _ret;}{

//	tDCO* 						const dco  = mod->m_sub;
//	volatile T_Dco1chData* 		const data = mod->m_dspData;	//ARM addr
//	volatile T_Dco1chParams* 	const coef = mod->m_dspCoefs;	//ARM addr
//	tDspPtr  xdata = (tDspPtr)mod->m_xdata; //DSP addr
//	tDspPtr  ycoef = (tDspPtr)mod->m_ycoef; //DSP addr

	sta_assert(mod->m_dspData && mod->m_dspCoefs);


	//set modes
	DCO_SetMode(mod, 0xFF);

	//set params
	mod->m_dirtyDspCoefs  = 0xFF;
	DCO_UpdateDspCoefs(mod);

	//then DSP calls an init function.
	}
_ret: return;
}


void DCO_SetMode(tModule* mod, u32 chmask)
{
	if (!mod) {goto _ret;}{

	tDCO* 						const dco  = mod->m_sub;
	volatile T_Dco1chData* 		const data = mod->m_dspData;	//ARM addr
	volatile T_Dco1chParams* 	const coef = mod->m_dspCoefs;	//ARM addr

	u32 ch, i, j, nin = mod->m_nin-1; //-1: removing last channel dedicated to DCO_REG
	int scoreCounterInit = 1; //last round for update

	sta_assert(mod->m_dspCoefs);


	FORMASKEDi(nin, chmask)
	{
		//CALIBRATE
		if (dco[i].m_mode & (u32)STA_DCO_CALIBRATE)
		{
			if (!coef[i].calibrate) { //not already calibrating
				//reset the tuners
				DSP_WRITE(&data[i].dlTunerData.delayIncr,	 0);
				DSP_WRITE(&data[i].dlTunerData.scoreCounter, scoreCounterInit); //last round for update
				DSP_WRITE(&data[i].dcTunerData.dcoIncr,		 0);
				DSP_WRITE(&data[i].dcTunerData.scoreCounter, scoreCounterInit); //last round for update
				DSP_WRITE(&data[i].thTunerData.thresIncr,	 0);
				DSP_WRITE(&data[i].thTunerData.scoreCounter, scoreCounterInit); //last round for update

				scoreCounterInit++; //(spread next channel's tuner updates to next samples)

				DSP_WRITE(&coef[i].calibrate, 1);
			}
		}
		else
		{
			DSP_WRITE(&coef[i].calibrate, 0);
		}

		//RUN

		//monozcf case
		ch = (mod->m_type == STA_DCO_4CH_MONOZCF) ? 0 : i;

		if (dco[ch].m_mode & (u32)STA_DCO_RUN)
		{
			if (!DSP_READ(&coef[ch].run)) { //metering not already running
				//reset metering
				DSP_WRITE(&data[ch].meterData.score, 		0);				//reset queriable score
				DSP_WRITE(&data[ch].meterData.scoreTmp, 	0);
				DSP_WRITE(&data[ch].meterData.scoreWindow,	dco[ch].m_scoreWindow);
				DSP_WRITE(&data[ch].meterData.scoreCounter,	dco[ch].m_scoreWindow);
				FORj(4) {
					DSP_WRITE(&data[ch].meterData.statsMatch[j],	0);		//reset queriable stats
					DSP_WRITE(&data[ch].meterData.statsMatchTmp[j],	0);
				}
				DSP_WRITE(&coef[ch].run, 1);
			}
		}
		else
		{
			DSP_WRITE(&coef[ch].run, 0);
			DSP_WRITE(&data[ch].meterData.score, 0); 						//reset queriable score
			FORj(4) {DSP_WRITE(&data[ch].meterData.statsMatch[j], 0);}		//reset queriable stats
		}

		if (mod->m_type == STA_DCO_4CH_MONOZCF) {
			break;} //just apply run to ch0

	}

	}
_ret: return;
}


void DCO_UpdateDspCoefs(tModule* mod)
{
	if (!mod) {goto _ret;}{

	tDCO* 						const dco  = mod->m_sub;
	volatile T_Dco1chData* 		const data = mod->m_dspData;	//ARM addr
	volatile T_Dco1chParams* 	const coef = mod->m_dspCoefs;	//ARM addr

	u32 i, nin = mod->m_nin-1; //(-1: removing last channel dedicated to DCO_REG)


	sta_assert(mod->m_dspCoefs);


	FORMASKEDi(nin, mod->m_dirtyDspCoefs)
	{
		u32 bulk = dco[i].m_fracDelay / 100;

		sta_assert(DSP_READ(&coef[i].calibrate) == 0); //can't update while calibrating

		DSP_WRITE(&coef[i].dcoParams.bulkDelay,		(s32)bulk);
		DSP_WRITE(&coef[i].dcoParams.fracDelay,		FRAC(dco[i].m_fracDelay - 100*bulk) / 100);
		DSP_WRITE(&coef[i].dcoParams.dcOffset,		FRAC1000(dco[i].m_dco));
		DSP_WRITE(&coef[i].dcoParams.zcThreshold,	FRAC1000(dco[i].m_winThres));

		DSP_WRITE(&data[i].meterData.scoreWindow,	dco[i].m_scoreWindow);
		DSP_WRITE(&data[i].meterData.scoreCounter,	dco[i].m_scoreWindow);
		DSP_WRITE(&data[i].meterData.alertThres,	dco[i].m_alertThres);
	}

	mod->m_dirtyDspCoefs = 0;
	}
_ret: return;
}


s32 DCO_GetParam(tModule* mod, u32 ch, STA_DCOParam param)
{
	s32 val = 0;

	if (!mod) {goto _ret;}{

	tDCO*	 					const dco  = mod->m_sub;
	volatile T_Dco1chData* 		const data = mod->m_dspData;	//ARM addr
	volatile T_Dco1chParams* 	const coef = mod->m_dspCoefs;	//ARM addr


	switch (param) {

	case STA_DCO_FRAC_DELAY:  //x100 (step 0.01)
		val = DSP_READ(&coef[ch].dcoParams.bulkDelay) * 100;
		val += FRAC2SCALE(DSP_READ(&coef[ch].dcoParams.fracDelay), 100);
		dco[ch].m_fracDelay = (u32)val;
		break;

	case STA_DCO_WIN_THRES:   //x1000 (step 0.001)
		val = dco[ch].m_winThres = FLT1000(DSP_READ(&coef[ch].dcoParams.zcThreshold)); break;

	case STA_DCO_DCOFFSET:   //x1000 (step 0.001)
		val = dco[ch].m_dco = FLT1000(DSP_READ(&coef[ch].dcoParams.dcOffset)); break;

	case STA_DCO_SCORE:
		val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? DSP_READ(&data[0].meterData.score) : DSP_READ(&data[ch].meterData.score); break;

	case STA_DCO_SCORE_WINDOW: //(get from driver)
		val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? (s32)dco[0].m_scoreWindow : (s32)dco[ch].m_scoreWindow; break;

	case STA_DCO_ALERT_THRES: //(get from driver)
		val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? (s32)dco[0].m_alertThres : (s32)dco[ch].m_alertThres; break;

	case STA_DCO_STATS_MATCH11:
		val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? DSP_READ(&data[0].meterData.statsMatch[3]) : DSP_READ(&data[ch].meterData.statsMatch[3]); break;

	case STA_DCO_STATS_MATCH00:
		val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? DSP_READ(&data[0].meterData.statsMatch[0]) : DSP_READ(&data[ch].meterData.statsMatch[0]); break;

	case STA_DCO_STATS_MISSED:
		val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? DSP_READ(&data[0].meterData.statsMatch[1]) : DSP_READ(&data[ch].meterData.statsMatch[1]); break;

	case STA_DCO_STATS_FALSE:
		val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? DSP_READ(&data[0].meterData.statsMatch[2]) : DSP_READ(&data[ch].meterData.statsMatch[2]); break;

	case STA_DCO_SWZCF:
		val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? DSP_READ(&data[0].dspZCFout) >> 18 : DSP_READ(&coef[ch].dcoParams.swZCF); break;

	case STA_DCO_HWZCF:
		val = (mod->m_type == STA_DCO_4CH_MONOZCF) ? DSP_READ(&coef[0].dcoParams.hwZCF) : DSP_READ(&coef[ch].dcoParams.hwZCF); break;

	default: sta_assert(0);
	}

	}
_ret: return val;
}



