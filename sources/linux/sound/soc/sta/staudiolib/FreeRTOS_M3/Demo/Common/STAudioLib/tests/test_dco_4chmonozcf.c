/***********************************************************************************/
/*!
*
*  \file      test_dco_4chmonozcf.c
*
*  \brief     <i><b> STAudioLib test application for DCO </b></i>
*
*  \details
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2015/04/27
*
*  \bug       see Readme.txt
*
*  \warning
*
*  This file is part of <component name> and is dual licensed,
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
* <component_name> is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* <component_name> is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*
*/
/***********************************************************************************/

/*
	Testing DCO_4CHMONOZCF module

			 MUX_4OUT
	         +----+
	 (mute)--|0   |
	XIN0-----|1  0|-----*----------------------- XOUT0 FL
	XIN1-----|2  1|----*|----------------------- XOUT1 FR
	+-----+  |   2|---*||----------------------- XOUT2 RL
	|SINE |--|3  3|--*|||----------------------- XOUT3 RR
	|GEN  |--|4   |  ||||   DCO_4CHMONOZCF
	+-----+  +----+  ||||  +-------------+
	                 |||*--|in0  swZCFout|------ XOUT4 swZCF<<18
	                 ||*---|in1     score|------ XOUT5 score
	                 |*----|in2          |
	                 *-----|in3  hwZCFout|------ XOUT6 (DCO_DAT>>18)&1  (opt.)
DCO_DAT XIN2------------*--|DCODAT       |
	                    |  +-------------+
	                    *----------------------- XOUT7 DCO_DAT (opt.)

 */

#if 1
#include "staudio_test.h"

//select the DSP
#define DSP_SEL				0

#if (DSP_SEL == 0)
#define DMA_XIN 			STA_DMA_DSP0_XIN
#define DMA_XOUT 			STA_DMA_DSP0_XOUT
#define MOD_XIN				STA_XIN0
#define MOD_XOUT			STA_XOUT0
#elif (DSP_SEL == 1)
#define DMA_XIN 			STA_DMA_DSP1_XIN
#define DMA_XOUT 			STA_DMA_DSP1_XOUT
#define MOD_XIN				STA_XIN1
#define MOD_XOUT			STA_XOUT1
#else
#define DMA_XIN 			STA_DMA_DSP2_XIN
#define DMA_XOUT 			STA_DMA_DSP2_XOUT
#define MOD_XIN				STA_XIN2
#define MOD_XOUT			STA_XOUT2
#endif

//GPIO0
#define GPIO_SWZCF_BASE		(GPIO_Typedef*)R4_GPIO0_BASE
#define GPIO_SWZCF_MASK		(1 << 0)

//GPIO2
#define GPIO_HWZCF_BASE		(GPIO_Typedef*)R4_GPIO0_BASE
#define GPIO_HWZCF_MASK		(1 << 2)


//-------------
// SINEWAVE GEN
//-------------
enum SINE_PARAM {SINE_FREQ, SINE_VOL, SINE_BAL};

typedef struct {
	STAModule 	mod;
	s32			ampl[2];
	u32			freq;			//user
	u32			msec;
	enum SINE_PARAM param;		//selected param on rotary
} SINEWAVEGEN;

static const SINEWAVEGEN g_sineInit = {
	.mod 		= 0,
	.ampl		= {200, 200},	//0.2
	.freq		= 150000, 		//1500 Hz (x100)
	.msec		=   1000,  		//1 sec
	.param		= SINE_VOL,
};

static void swg_reset(SINEWAVEGEN* sine)
{
	STAModule mod = sine->mod; 				//bak handle
	*sine = g_sineInit; sine->mod = mod;	//reset

	STA_SetMode(mod, STA_SINE_ON);
}


static void swg_print(SINEWAVEGEN* sine)
{
	switch (sine->param) {
	case SINE_FREQ:	PRINTF("SWG: FRQ: [F= %d Hz], L= %d, R= %d\n",  sine->freq/100, sine->ampl[0], sine->ampl[1]); break;
	case SINE_VOL:	PRINTF("SWG: VOL:  F= %d Hz, [L= %d, R= %d]\n", sine->freq/100, sine->ampl[0], sine->ampl[1]); break;
	case SINE_BAL:  PRINTF("SWG: BAL:  F= %d Hz, [L= %d, R= %d]\n", sine->freq/100, sine->ampl[0], sine->ampl[1]); break;
	}
	return;
}

//-------------
// DCO
//-------------

enum DCO_PARAM {DCO_DLY, DCO_DCO, DCO_THR, DCO_WIN, DCO_CTH, DCO_ATH, DCO_LAST};  //CTH=CalibThres, ATH=AlertThres

typedef struct {
	STAModule	mod;
	u32			delay[4];
	s32			dcoff[4];
	u32			winTh[4];
	u32			scoreWin;
	u32			calibTh;
	u32			alertTh;
	s32			steps[DCO_LAST];	//steps for each user params
	bool		showScore;
	bool		calibrateAuto;
	bool		enableMeter;
	bool		dcDetected;
	xTimerHandle monitoringTimer;
	enum DCO_PARAM param; 			//selected param on rotary
} DCO;

static const DCO g_dcoInit = {
	.mod 		= 0,
	.delay		= {2200, 2200, 2200, 2200}, //=22 (x100) <-- set offline calibration or initial conditions
	.dcoff		= {  0,   0,   0,   0}, //=0   (x1000)
	.winTh		= {100, 100, 100, 100}, //=0.1 (x1000)
	.scoreWin	= 5000,
	.calibTh	= 4950,//5000*0.95,
	.alertTh	= 5000*0.80,
	.param		= DCO_DLY,
	.steps		= {100, 10, 10, 100, 100, 100},
	.showScore	   = 1,
	.enableMeter   = 0,
	.calibrateAuto = 0,
};


//---------
// MUX
//---------

static enum SRC {SRC_SINE, SRC_XIN} g_src = SRC_SINE;

static u32 			g_chmask	 = 0x1; //0xF means all unmute

static u8 			g_muxSels[4] = {0, 0, 0, 0};	//all mute <- init from g_src and g_chmask


//---------
// MODULES
//---------
//for user interaction
static SINEWAVEGEN 	g_sine;
static STAModule	g_mux;
static DCO			g_dco;
//static STAModule 	g_gains;

//---------
// actions
//---------

static enum SEL {SEL_SINE, SEL_MUX, SEL_DCO} g_sel = SEL_SINE;

static int			g_playSine		= 1;	//note: DSP sine is not managed by PLAYER


//---------------------------------------------------------
// MUX
//---------------------------------------------------------

/*
#define MUX_IN_MUTE		0
#define MUX_IN_XINL		1
#define MUX_IN_XINR		2
#define MUX_IN_SWGL		3
#define MUX_IN_SWGR		4
*/

//mux depends on src and chmask
static void mux_set(enum SRC src, u32 chmask)
{
	int inL, inR, i;
	int out[4] = {0}; //all mute

	//input
	if (src == SRC_XIN) {
		inL = 1; inR = 2;		//XIN
	} else {
		inL = 3; inR = 4;		//SINE
	}

	//output
	switch (chmask) {

	//single channel => inL
	case 0x0:	break; 			//keep all mute
	case 0x1:	out[0] = inL; break;
	case 0x2:	out[1] = inL; break;
	case 0x4:	out[2] = inL; break;
	case 0x8:	out[3] = inL; break;

	//multi channel => inL and inR (if masked)
	default:	out[0] = (chmask & 0x1) ? inL : 0;
				out[1] = (chmask & 0x2) ? inR : 0;
				out[2] = (chmask & 0x4) ? inL : 0;
				out[3] = (chmask & 0x8) ? inR : 0;
	}

	//apply
	FORi(4) g_muxSels[i] = (u8) out[i];

	STA_MuxSet(g_mux, g_muxSels);

}
//---------------------------------------------------------
// DCO (offline) CALIBRATION
//---------------------------------------------------------
#define PRINT_P		dco->delay[i], dco->dcoff[i], dco->winTh[i]
//#define PRINT_PS	dco->delay[i], dco->dcoff[i], dco->winTh[i], score
//#define PRINT_PSM	dco->delay[i], dco->dcoff[i], dco->winTh[i], score, match1, match0, missed, falsed
#define PRINT_SM	score, dco->scoreWin, scorepercent_int, scorepercent_frac, match1, match0, missed, falsed

#define PR_GETFROMDSP	1
#define PR_ONLYSCORE	2
#define PR_ONLYCALIB	4

static void dco_print(int chmask, u32 flags)
{
	DCO* dco = &g_dco;
	s32 score=0, match1=0, match0=0, missed=0, falsed=0;
	bool onlyscore = flags & PR_ONLYSCORE;
	bool onlycalib = flags & PR_ONLYCALIB;
	int scorepercent_int, scorepercent_frac;
	int i;


	FORMASKEDi(4, chmask)
	{
		//get all params from dsp
		if (flags & PR_GETFROMDSP)
		{
			//Params
			if (!onlyscore) {
				dco->delay[i] = STA_DCOGetParam(dco->mod, i, STA_DCO_FRAC_DELAY);
				dco->dcoff[i] = STA_DCOGetParam(dco->mod, i, STA_DCO_DCOFFSET);
				dco->winTh[i] = STA_DCOGetParam(dco->mod, i, STA_DCO_WIN_THRES);
			}
			//Score and stats
			if (onlyscore || dco->enableMeter) {
				score  = STA_DCOGetParam(dco->mod, i, STA_DCO_SCORE);
				match1 = STA_DCOGetParam(dco->mod, i, STA_DCO_STATS_MATCH11);
				match0 = STA_DCOGetParam(dco->mod, i, STA_DCO_STATS_MATCH00);
				missed = STA_DCOGetParam(dco->mod, i, STA_DCO_STATS_MISSED);
				falsed = STA_DCOGetParam(dco->mod, i, STA_DCO_STATS_FALSE);

				scorepercent_int = score * 100   / dco->scoreWin;
				scorepercent_frac= score * 10000 / dco->scoreWin - 100 * scorepercent_int;
			}
		}

		//PRINT
		if (onlyscore)
		{
			#if 0
			if (dco->calibrateAuto) //(inside autocalibration, don't re-print channel)
				PRINTF("score= %d (%d.%d%%)  (1= %d 0= %d m= %d f= %d)\n", PRINT_SM);
			else
				PRINTF("DCO: ch%d: score= %d (%d.%d%%)  (1= %d 0= %d m= %d f= %d)\n", i, PRINT_SM);
			#else
			PRINTF("score= %d/%d (%d.%d%%)  (1= %d 0= %d m= %d f= %d)\n", PRINT_SM);
			#endif
		}
		else if (onlycalib)
		{
			PRINTF("DCO: ch%d: dl= %d  dc= %d  th= %d\n", i, PRINT_P);
		}
		else if (dco->enableMeter)
		{
			switch (dco->param) {
			case DCO_DLY: PRINTF("DCO: ch%d: dl= %d (+/-%d)  score= %d/%d (%d.%d%%) (1= %d 0= %d m= %d f= %d)\n", i, dco->delay[i], dco->steps[DCO_DLY], PRINT_SM); break;
			case DCO_DCO: PRINTF("DCO: ch%d: dc= %d (+/-%d)  score= %d/%d (%d.%d%%) (1= %d 0= %d m= %d f= %d)\n", i, dco->dcoff[i], dco->steps[DCO_DCO], PRINT_SM); break;
			case DCO_THR: PRINTF("DCO: ch%d: th= %d (+/-%d)  score= %d/%d (%d.%d%%) (1= %d 0= %d m= %d f= %d)\n", i, dco->winTh[i], dco->steps[DCO_THR], PRINT_SM); break;
			case DCO_WIN: PRINTF("DCO: ch%d: score win= %d (+/-%d)  score= %d/%d (%d.%d%%) (1= %d 0= %d m= %d f= %d)\n", i, dco->scoreWin, dco->steps[DCO_WIN], PRINT_SM); break;
			case DCO_CTH: PRINTF("DCO: ch%d: calib thr= %d (+/-%d)  score= %d/%d (%d.%d%%) (1= %d 0= %d m= %d f= %d)\n", i, dco->calibTh,  dco->steps[DCO_CTH], PRINT_SM); break;
			case DCO_ATH: PRINTF("DCO: ch%d: alert thr= %d (+/-%d)  score= %d/%d (%d.%d%%) (1= %d 0= %d m= %d f= %d)\n", i, dco->alertTh,  dco->steps[DCO_ATH], PRINT_SM); break;
			default:;
			}
		}
		else
		{
			switch (dco->param) {
			case DCO_DLY: PRINTF("DCO: ch%d: dl= %d (+/-%d)\n", i, dco->delay[i], dco->steps[DCO_DLY]); break;
			case DCO_DCO: PRINTF("DCO: ch%d: dc= %d (+/-%d)\n", i, dco->dcoff[i], dco->steps[DCO_DCO]); break;
			case DCO_THR: PRINTF("DCO: ch%d: th= %d (+/-%d)\n", i, dco->winTh[i], dco->steps[DCO_THR]); break;
			case DCO_WIN: PRINTF("DCO: ch%d: score win= %d (+/-%d)\n", i, dco->scoreWin, dco->steps[DCO_WIN]); break;
			case DCO_CTH: PRINTF("DCO: ch%d: calib thr= %d (+/-%d)\n", i, dco->calibTh,  dco->steps[DCO_CTH]); break;
			case DCO_ATH: PRINTF("DCO: ch%d: alert thr= %d (+/-%d)\n", i, dco->alertTh,  dco->steps[DCO_ATH]); break;
			default:;
			}
		}
	}
}


static void dco_timerCallback(xTimerHandle xTimer)
{
	static bool s_dcDetected = 0;

	//check the score
	s32 score = STA_DCOGetParam(g_dco.mod, 1<<0, STA_DCO_SCORE); //monozcf -> read score from ch0

	g_dco.dcDetected = (score < g_dco.alertTh);

	//PRINT if dcDetected changed
	if (g_dco.dcDetected != s_dcDetected)
	{
		if (g_dco.dcDetected)
			PRINTF("DCO: DC offset detected !!\n");
		else
			PRINTF("DCO: DC offset removed !!\n");

		dco_print(1<<0, PR_GETFROMDSP | PR_ONLYSCORE); //(onlyscore)

		s_dcDetected = g_dco.dcDetected;
		return;
	}


	if (g_dco.showScore)
		dco_print(1<<0, PR_GETFROMDSP | PR_ONLYSCORE); //(onlyscore)

}

static void dco_FsCallback(void)
{
	//static int s_tmp = 0;
	//s_tmp ^= 1;

	//forward swZCF and hwZCF to gpios...
	u32 swzcf = STA_DCOGetParam(g_dco.mod, 1<<0, STA_DCO_SWZCF);
	u32 hwzcf = STA_DCOGetParam(g_dco.mod, 1<<0, STA_DCO_HWZCF);

	GPIO_Set(GPIO_SWZCF_BASE, GPIO_SWZCF_MASK, swzcf ? GPIO_SETDATA : GPIO_CLRDATA);
	GPIO_Set(GPIO_HWZCF_BASE, GPIO_HWZCF_MASK, hwzcf ? GPIO_SETDATA : GPIO_CLRDATA);
}


static void dco_toggleMetering(bool enable)
{
	long xHigherPriorityTaskWoken; //used for timer APIs from ISR

	if (enable)
	{
		//enable dco meter
		STA_DCOSetMode(g_dco.mod, 0xF, STA_DCO_RUN);

		//enable dco timer
		if (xTimerStartFromISR(g_dco.monitoringTimer, &xHigherPriorityTaskWoken) != pdPASS)
			PRINTF("toggleMeter(): timer failed!\n");

		//enable fs irq
		STA_SetFSyncISR(dco_FsCallback);
	}
	else
	{
		//disable fs irq
		STA_SetFSyncISR(NULL);

		//disable dco timer
		xTimerStopFromISR(g_dco.monitoringTimer, &xHigherPriorityTaskWoken);

		//disable dco meter
		STA_DCOSetMode(g_dco.mod, 0xF, STA_DCO_OFF);
	}
}

static void dco_reset(u32 chmask)
{
	DCO* dco = &g_dco;
	STAModule hdco = g_dco.mod; 			//bak handle
	int i;

	//create a timer for dco score pulling
	if (!dco->monitoringTimer) {
		dco->monitoringTimer = xTimerCreate((signed char*)"dcoTimer", 1000/portTICK_RATE_MS, TRUE, (void*)2, dco_timerCallback); //1sec, autorelead, ID, callback
		if (!dco->monitoringTimer)
			PRINTF("DCO: xTimerCreate() failed\n");
	}

//	*dco = g_dcoInit; dco->mod = hdco;	//reset to inital values

	PRINTF("DCO reset to initial calibration\n");

	dco->calibrateAuto	= 0;
	dco->dcDetected		= 0;
	dco->enableMeter	= g_dcoInit.enableMeter;
	dco->showScore		= g_dcoInit.showScore;
	dco->scoreWin 		= g_dcoInit.scoreWin;
	dco->calibTh  		= g_dcoInit.calibTh;
	dco->alertTh  		= g_dcoInit.alertTh;
	FORi(DCO_LAST) dco->steps[i] = g_dcoInit.steps[i];


	//stop DCO calibration first
	dco_toggleMetering(0);

	//init common params
	STA_DCOSetParam(hdco, 0xF, STA_DCO_SCORE_WINDOW, dco->scoreWin);
	STA_DCOSetParam(hdco, 0xF, STA_DCO_ALERT_THRES,  dco->alertTh);

	//init the dco params
	#if 0
	STA_DCOSetParam(hdco, 0xF, STA_DCO_DCOFFSET,     0);
	STA_DCOSetParam(hdco, 0xF, STA_DCO_FRAC_DELAY,   0);
	STA_DCOSetParam(hdco, 0xF, STA_DCO_WIN_THRES,    0);
	#else
	FORMASKEDi(4, chmask) {
		dco->dcoff[i] = g_dcoInit.dcoff[i];		//reset
		dco->delay[i] = g_dcoInit.delay[i];
		dco->winTh[i] = g_dcoInit.winTh[i];
		STA_DCOSetParam(hdco, 1<<i, STA_DCO_DCOFFSET,     dco->dcoff[i]);
		STA_DCOSetParam(hdco, 1<<i, STA_DCO_FRAC_DELAY,   dco->delay[i]);
		STA_DCOSetParam(hdco, 1<<i, STA_DCO_WIN_THRES,    dco->winTh[i]);
	}
	#endif

	dco_print(chmask, PR_GETFROMDSP | PR_ONLYCALIB);

	//restore metering
	if (dco->enableMeter)
		dco_toggleMetering(1);
}


//assumed src and mux already set for the given channel
//and all other channels are muted.
static void dco_calibrateChannel_offline(u32 ch)
{
	u32 chm		= 1<<ch;
	u32 score   = 0;
	u32 timeleft = 2000; //2 sec

	PRINTF("Calibrating channel %d...  ", ch);

	//set initial values to DCO params
	#if 0
	STA_DCOSetMode(g_dco.mod, 0xF, STA_DCO_OFF);  //stop before set params
	STA_DCOSetParam(g_dco.mod, chm, STA_DCO_DCOFFSET,   0);
	STA_DCOSetParam(g_dco.mod, chm, STA_DCO_FRAC_DELAY, 0);
	STA_DCOSetParam(g_dco.mod, chm, STA_DCO_WIN_THRES,  0);
	#endif


	//START calibration
	STA_DCOSetMode(g_dco.mod, chm, STA_DCO_CALIBRATE | STA_DCO_RUN);


	while (timeleft && score < g_dco.calibTh) {
		SLEEP(100);	timeleft -= 100;
		score = STA_DCOGetParam(g_dco.mod, ch, STA_DCO_SCORE); //(get from ch0 because monozcf)
	}

	//PRINT
	PRINTF((timeleft > 0) ? "ok: " : "failed: ");
	dco_print(chm, PR_GETFROMDSP | PR_ONLYSCORE);

	//STOP calibration (? keep meter on for final print of stats ?)
	STA_DCOSetMode(g_dco.mod, chm, STA_DCO_RUN);

}


static void dco_calibrate_offline(u32 chmask)
{
	int i;

	PRINTF("DCO offline calibration:\n");

	//stop DCO on all channels
	dco_toggleMetering(0);


	//SINEWAVE ON (60 sec)
	STA_SinewaveGenPlay(g_sine.mod, g_sine.freq, 60000, g_sine.ampl[0], g_sine.ampl[1]); //play

	//Calibrate each channel consecutively
	FORMASKEDi(4, chmask)
	{
		//MUX
		//unmute all but ch, and select sinewave_L
		u8 muxSels[4] = {0}; 		//all muted
		muxSels[i]    = 3;			//sine_L
		STA_MuxSet(g_mux, muxSels);

		dco_calibrateChannel_offline(i);
	}

	//PRINT
	dco_print(0xF, PR_GETFROMDSP | PR_ONLYCALIB); //print all

	//SINEWAVE OFF
	STA_SinewaveGenPlay(g_sine.mod, g_sine.freq, 0, g_sine.ampl[0], g_sine.ampl[1]); //stop

	//restore MUX
	STA_MuxSet(g_mux, g_muxSels);

	//restore DCO mode
	dco_toggleMetering(g_dco.enableMeter);


	g_dco.calibrateAuto = 0;
}


//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void dsp_flow(void)
{
	STAModule xin, xout, mux, sin, dco;


	STA_Reset();

	//SINEWAVE
	g_sine.mod = STA_AddModule(STA_DSP0, STA_SINE_2CH);
	swg_reset(&g_sine);

	//MUX
	g_mux = STA_AddModule(STA_DSP0, STA_MUX_4OUT);
	mux_set(g_src, g_chmask);

	//GAINS
//	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); //2 gains by default

	//DCO
	g_dco.mod = STA_AddModule(STA_DSP0, STA_DCO_4CH_MONOZCF);
	dco_reset(0xF); //reset all ch


	//CONNECTIONS
	xin  = MOD_XIN;
	xout = MOD_XOUT;
	sin  = g_sine.mod;
	dco  = g_dco.mod;
	mux  = g_mux;

	//MUX inputs
	//note: mux_in0 left unconnected (mute)
	STA_Connect(xin, 0, mux, 1);
	STA_Connect(xin, 1, mux, 2);
	STA_Connect(sin, 0, mux, 3);
	STA_Connect(sin, 1, mux, 4);

	//DCO inputs
	STA_Connect(mux, 0, dco, 0);
	STA_Connect(mux, 1, dco, 1);
	STA_Connect(mux, 2, dco, 2);
	STA_Connect(mux, 3, dco, 3);
	STA_Connect(xin, 2, dco, 4); //DCODAT

	//output
	STA_Connect(mux, 0, xout, 0); //FL
	STA_Connect(mux, 1, xout, 1); //FR
	STA_Connect(mux, 2, xout, 2); //RL
	STA_Connect(mux, 3, xout, 3); //RR
	STA_Connect(dco, 0, xout, 4); //ZCFout
	STA_Connect(dco, 1, xout, 5); //DCOscore
	STA_Connect(dco, 2, xout, 6); //ZCFin


	//TMP
//	STA_Connect(sin, 0, xout, 0); //FL
//	STA_Connect(sin, 1, xout, 1); //FR

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);
}


static void aud_routing(int nch)
{
	//DMABUS
	STA_DMAReset();

	//inputs
	STA_DMAAddTransfer(STA_DMA_FIFO_OUT, 	DMA_XIN, nch);	//FIFO2 to XIN
	if (nch == 1)
		STA_DMAAddTransfer(DMA_XIN, 		DMA_XIN+1, 1); 	//R (copy from L)

	STA_DMAAddTransfer(STA_DMA_DCO_DATA, 	DMA_XIN+2, 1);	//DCO_DATA to XIN

	//outputs
	STA_DMAAddTransfer(DMA_XOUT,  			STA_DMA_SAI3_TX1,    2);	//XOUT(FrontLR) to SAI3
	STA_DMAAddTransfer(DMA_XOUT+2, 			STA_DMA_SAI3_TX2,    2);	//XOUT(RearLR)  to SAI3
//	STA_DMAAddTransfer(DMA_XOUT+4,			??				,    2);	//XOUT(ZCFin,ZCFout) to ??
//	STA_DMAAddTransfer(DMA_XOUT+6,			??				,    1);	//XOUT(DCOscore)  to ??

	//swap trigger
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	DMA_XIN+127, 1);	//swap trigger
}

//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------

static void toggleSrc(void)
{
	g_src ^= 1;

	PRINTF("SRC: %s\n", g_src == SRC_SINE ? "SINEWAVE GEN" : "XIN");

	mux_set(g_src, g_chmask);
}

static void toggleChannel(u32 ch)
{
	if (ch < 4)
	{
		u32 chmask = 1 << ch;

		g_chmask = (g_chmask & chmask)  ? g_chmask & ~chmask  //mute channel
										: g_chmask |  chmask; //unmute channel
		mux_set(g_src, g_chmask);
	}

	PRINTF("MUX: %d %d %d %d\n", g_muxSels[0], g_muxSels[1], g_muxSels[2], g_muxSels[3]);

}

static void toggleSel(void)
{
//	const char* str[] = {"SINEWAVE GEN", "MUX", "DCO"};

	g_sel = (g_sel == SEL_DCO) ? SEL_SINE : g_sel+1;

//	PRINTF("SEL = %s\n", str[g_sel]);

	//more PRINTF
	switch (g_sel) {
	case SEL_SINE: swg_print(&g_sine); break;
	case SEL_MUX:  toggleChannel((u32)-1); break;  //just for print
	case SEL_DCO:  dco_print(g_chmask, PR_GETFROMDSP); break;
	}
}

static void toggleMeter(void)
{
	g_dco.enableMeter ^= 1;

	PRINTF("DCO runtime metering %s\n", g_dco.enableMeter ? "ON" : "OFF");

	dco_toggleMetering(g_dco.enableMeter);
}

static void toggleShowScore(void)
{
	g_dco.showScore ^= 1;

	PRINTF("DCO: %s score\n",

	g_dco.showScore?"show":"hide");
}

static void toggleCalibrate(void)
{
	g_dco.calibrateAuto ^= 1;

	if (g_dco.calibrateAuto)
		g_sel = SEL_DCO;

	else {
		//STOP Calibration
		PRINTF("DCO calibration stopped\n");
		dco_toggleMetering(g_dco.enableMeter);
	}
}

static void togglePlay(void)
{
	//XIN
	if (g_src == SRC_XIN)
		PLY_SetState(g_player.m_state == PLY_PLAY ? PLY_PAUSE : PLY_PLAY);

	//SINE
	else
	{
		g_playSine ^= 1;

		if (!g_playSine)
			STA_SinewaveGenPlay(g_sine.mod, g_sine.freq, 0, g_sine.ampl[0], g_sine.ampl[1]); //stop

		PRINTF("SWG: %s\n", g_playSine ? "PLAY" : "STOP");
	}

	#if 0
	//START / STOP DMABUS
	if (g_player.m_state == PLY_PLAY || g_playSine)
		STA_DMAStart();
	else
		STA_DMAStop();
	#endif


}


static void keyCallback(int key, bool down)
{
	/*
	- key mapping (SEL = SINE)
	[SRC]        [    ]	    [SEL]  [   ]  [   ] [   ]
							[RST]  [   ]  [FRE] [VOL]  <- SINE
	[PLAY/PAUS]  [STOP]		[RST]  [   ]  [CAL] [DCO]  <- DCO: Calibrate, meterOn


	- key mapping (SEL = MUX)
	[SRC]        [    ]	    [SEL]  [   ]  [CH0] [CH1]  <- MUX front
							[   ]  [   ]  [CH2] [CH3]  <- MUX rear
	[PLAY/PAUS]  [STOP] 	[RST]  [   ]  [CAL] [DCO]  <- DCO Calibrate, meterOn


	- key mapping (SEL = DCO)
	[SRC]        [    ]	    [SEL]  [   ]  [PAR-] [PAR+] <-
							[SCR]  [   ]  [STP-] [STP+] <- SCR=show score
	[PLAY/PAUS]  [STOP] 	[RST]  [   ]  [CAL ] [DCO]  <- DCO Reset, Calibrate, meterOn
	*/

	//always active keys
	switch(key) {
	//SRC
	case 0x11:  toggleSrc(); return;

	//PLAY/PAUSE
	case 0x13:  togglePlay(); return;

	//STOP
	case 0x14:  PLY_SetState(PLY_STOP); g_playSine = 0; return;

	//SEL
	case 0x21:	toggleSel(); return;

	//DCO
	case 0x41:  dco_reset(g_chmask); return;
	case 0x42:  toggleShowScore(); return;
	case 0x43:  toggleCalibrate(); return;
	case 0x44:  toggleMeter(); return;
	}

	//MUX
	if (g_sel == SEL_MUX) {
		switch(key) {
		case 0x23:  toggleChannel(0); return;
		case 0x24:  toggleChannel(1); return;
		case 0x33:  toggleChannel(2); return;
		case 0x34:  toggleChannel(3); return;
		}
	}

	//SINEWAVE GEN
	if (g_sel == SEL_SINE) {
		switch(key) {
		case 0x31:  swg_reset(&g_sine); PRINTF("SWG: Reset\n"); break;
	//	case 0x32:  g_sine.onoff ^= 1; STA_SetMode(g_sine.mod, g_sine.onoff); PRINTF("SINEWAVE GEN: %s\n", g_sine.onoff ? "ON":"OFF"); break;
		case 0x33:  g_sine.param = SINE_FREQ;  break;
		case 0x34:  g_sine.param = SINE_VOL;   break;
	//	case 0x34:  g_sine.param = SINE_BAL;   break;
		}
		swg_print(&g_sine);
		return;
	}

	//DCO
	if (g_sel == SEL_DCO) {
		switch(key) {
		case 0x23:  g_dco.param = (g_dco.param > 0         ) ? g_dco.param-1 : DCO_LAST-1; break;
		case 0x24:  g_dco.param = (g_dco.param < DCO_LAST-1) ? g_dco.param+1 : 0; break;
		case 0x31:  toggleShowScore(); return;
		case 0x33:  g_dco.steps[g_dco.param] = MAX(1, g_dco.steps[g_dco.param]/10); break;
		case 0x34:  g_dco.steps[g_dco.param] *= 10; break;
		}
		dco_print(g_chmask, PR_GETFROMDSP);
		return;
	}

}


static void rtxCallback(int steps)
{
	int i;

	//MUX (=> change sel to SINE)
	if (g_sel == SEL_MUX)
		g_sel = SEL_SINE;


	//SINEWAVE GEN
	if (g_sel == SEL_SINE)
	{
		static int s_ampL = 50; //used for balance (set as half)
		static int s_ampR = 50;

		switch(g_sine.param) {
		case SINE_FREQ: g_sine.freq += steps * 1000; // +/- 10 Hz
						break;
		case SINE_VOL:  g_sine.ampl[0] += steps * 10;  s_ampL = g_sine.ampl[0];
						g_sine.ampl[1] += steps * 10;  s_ampR = g_sine.ampl[1];
						break;
		case SINE_BAL:  s_ampL -= steps * 5; if (s_ampL <= 100) g_sine.ampl[0] = s_ampL;
						s_ampR += steps * 5; if (s_ampR <= 100) g_sine.ampl[1] = s_ampR;
						break;
		}
		swg_print(&g_sine);
		STA_SinewaveGenPlay(g_sine.mod, g_sine.freq, 1000, g_sine.ampl[0], g_sine.ampl[1]); //just a beep
		return;
	}

	//DCO
	if (g_sel == SEL_DCO)
	{
		if (g_dco.calibrateAuto)
			toggleCalibrate();  //stop auto calibration

		FORMASKEDi(4, g_chmask)
		{
			switch(g_dco.param) {
			case DCO_DLY: g_dco.delay[i] += steps * g_dco.steps[DCO_DLY];
						  g_dco.delay[i] = MAX((s32)g_dco.delay[i], 0);
						  STA_DCOSetParam(g_dco.mod, 1<<i, STA_DCO_FRAC_DELAY, g_dco.delay[i]);
						  break;

			case DCO_DCO: g_dco.dcoff[i] += steps * g_dco.steps[DCO_DCO];
						  STA_DCOSetParam(g_dco.mod, 1<<i, STA_DCO_DCOFFSET, g_dco.dcoff[i]);
						  break;

			case DCO_THR: g_dco.winTh[i] += steps * g_dco.steps[DCO_THR];
						  g_dco.winTh[i] = MAX((s32)g_dco.winTh[i], 0);
						  STA_DCOSetParam(g_dco.mod, 1<<i, STA_DCO_WIN_THRES, g_dco.winTh[i]);
						  break;

			case DCO_WIN: g_dco.scoreWin += steps * g_dco.steps[DCO_WIN];
						  g_dco.scoreWin = MAX((s32)g_dco.scoreWin, 0);
						  STA_DCOSetParam(g_dco.mod, 1<<i, STA_DCO_SCORE_WINDOW, g_dco.scoreWin);
						  break;

			case DCO_ATH: g_dco.alertTh += steps * g_dco.steps[DCO_ATH];
						  g_dco.alertTh = MAX((s32)g_dco.alertTh, 0);
						  STA_DCOSetParam(g_dco.mod, 1<<i, STA_DCO_ALERT_THRES, g_dco.alertTh);
						  break;

			case DCO_CTH: g_dco.calibTh += steps * g_dco.steps[DCO_CTH];
						  g_dco.calibTh = MAX((s32)g_dco.calibTh, 0);
						  break;
			default: ASSERT(0);
			}
		}
		dco_print(g_chmask, PR_GETFROMDSP);
		return;
	}
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestDCO_4chmonozcf(void)
{
	int nch = 2;


	PRINTF("TestDCO_4chmonozcf... \n");

	/////////////////////////////////////////////////////
	//For "patched" A2EVB for DCO testing:
	//
	//TDA powerAmpIC 'DCOFF' connected to GPIO12 (monoZCF case)
	//(ok for cut1 or cut2)

	//GPIO for DCOFF input
	//DCOFF connected to gpio12 (R4_GPIO0_BASE = 0x48120000) (on "patched" A2EVB for DCO testing)
	GPIO_Set((GPIO_Typedef*)R4_GPIO0_BASE, 1<<12, GPIO_NOALT | GPIO_IN | GPIO_PULLEN | GPIO_NOTRIG);

	//DCO_CR
	//DCO_DAT = [ch5 ch4 ch3 ch2 ch1 ch0 xxxx]  monozcf in ch0
	//       bit: 23  22  21  20  19  18 17-0
	//map DCO_DAT[ch0] to GPIO12 (on "patched" A2EVB for DCO testing)
	AUDCR->DCO_CR0.REG = 0;
	AUDCR->DCO_CR1.REG = 0;
	AUDCR->DCO_CR0.BIT.MUX0SEL = 12; //gpio12
	/////////////////////////////////////////////////////////


	//GPIO for swZCF and hwZCF out
	GPIO_Set(GPIO_SWZCF_BASE, GPIO_SWZCF_MASK, GPIO_NOALT | GPIO_OUT | GPIO_NOTRIG | GPIO_SETDATA);
	GPIO_Set(GPIO_HWZCF_BASE, GPIO_HWZCF_MASK, GPIO_NOALT | GPIO_OUT | GPIO_NOTRIG | GPIO_SETDATA);


	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);


	//audio init
	dsp_flow();
	aud_routing(nch);

	//start dsp
	STA_Play();
	SLEEP(100);					//wait 100ms for DSP initialisation

	//post initialisation

	//TMP debug
	/*
	{int i;
	FORi(4) {
		u32 delay = STA_DCOGetParam(g_dco.mod, i, STA_DCO_FRAC_DELAY);
		s32 dcoff = STA_DCOGetParam(g_dco.mod, i, STA_DCO_DCOFFSET);
		u32 winTh = STA_DCOGetParam(g_dco.mod, i, STA_DCO_WIN_THRES);
		}
	}
	*/

	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_16VAL | SAI_RL | SAI_EN;

	STA_DMAStart();	//start DMABUS

	//mini player
	PLY_Init();
	PLY_Set_PCM_48Khz_16bit_FIFO2(g_songAddr + 11, g_songSize - 11*4, nch);

	if (g_src == SRC_XIN)
		PLY_SetState(PLY_PLAY);	//start DMA


	while (1)
	{

		if (g_dco.calibrateAuto)
			dco_calibrate_offline(g_chmask);  //returns after calibration


		if (g_playSine)
			STA_SinewaveGenPlay(g_sine.mod, g_sine.freq, g_sine.msec, g_sine.ampl[0], g_sine.ampl[1]);


		SLEEP(g_sine.msec - 10); //avoid sticky loop
	}

	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}

#endif //0
