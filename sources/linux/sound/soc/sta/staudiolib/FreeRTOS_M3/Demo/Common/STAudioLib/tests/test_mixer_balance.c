/*
 * STAudioLib - test_mixer_balance.c
 *
 * Created on 2013/10/23 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	/!\ THIS TEST DOES NOT USE DSP @48Khz, instead triggers XIN/XOUT swaps manually

	Testing Mixer balance (L/R) and fader (F/B)

	XIN[0] -> L0
	XIN[1] -> R0
	XIN[2] -> L1
	XIN[3] -> R1

	TEST FLOW 1): using 1 Mixer 2ins x 4ch

	    Left +--------+
    L0	--#==| Mixer0 |--XOUT[0]  (FL)
	L1	--#  +--------+
	      #==| Mixer1 |--XOUT[2]  (RL)
	   Right +--------+
	R0	--#==| Mixer2 |--XOUT[1]  (FR)
	R1	--#  +--------+
	      #==| Mixer3 |--XOUT[3]  (RR)
	         +--------+


	TEST FLOW 2): using 2 Mixers 2ins x 2ch

	     +--------+
	L0 --| Front  |--XOUT[0]  (FL)
	L1 --| Mixer0 |
	     +--------+
	R0 --| Front  |--XOUT[1]  (FR)
	R1 --| Mixer1 |
		 +--------+
	     +--------+
	L0 --| Rear   |--XOUT[2]  (RL)
	L1 --| Mixer0 |
	     +--------+
	R0 --| Rear   |--XOUT[3]  (RR)
	R1 --| Mixer1 |
		 +--------+
 */

//TODO: Test saturation cases...
//		Test SetBalanceLimit
//		Test SetBalanceCenter (<-- need a 6ch mixer...)


#if 1
#include "staudio_test.h"


#define FRONT	0
#define REAR	1
#define LEFT	0
#define RIGHT	1

static STAModule g_mixer1;   //mixer
static STAModule g_mixer[2]; //mixer   for F and R

static STAModule g_bal[2];	 //balance for F and R
static STAModule g_fad[2];	 //fader   for L and R



typedef struct {
	s32 bal, fad;
	float out[4];
} tTestCase;

#define a	0.2f
#define s	(a+a)
#define g	0.5f  /*-6 db */

static const float g_in0[4] = {a, a, a, a};

static const tTestCase g_case[9] = {
//	 bal	fad		 FL		FR		RL		RR			volumes
	{  0,	  0,	{s,		s,		s,		s	 }},
	{-60,	  0,	{s*g,	s,		s*g,	s	 }},	// L/2
	{+60,	  0,	{s,		s*g,	s,		s*g	 }},	// R/2
	{  0,	-60,	{s*g,	s*g,	s,		s	 }},	// F/2
	{  0,	+60,	{s,		s,		s*g,	s*g	 }},	// B/2
	{-60,	-60,	{s*g*g,	s*g,	s*g,	s	 }},	// L/2 F/2
	{-60,	+60,	{s*g,	s,		s*g*g,	s*g	 }},	// L/2 B/2
	{+60,  	-60,	{s*g,	s*g*g,	s,		s*g	 }},	// R/2 F/2
	{+60,	+60,	{s,		s*g,	s*g,	s*g*g}},	// R/2 B/2
};

#undef a
#undef s
#undef g



//---------------------------------------------------------
static void BuildFlow_1()
{
	const int nin  = 2 * 2/*insets*/;
	const int nout = 4;
	int i;

	STA_Reset();

	//MODULES
	//add 1 Mixer 2ins_4ch
	g_mixer1 = STA_AddModule(STA_DSP0, STA_MIXE_2INS_NCH); 	//2 channels by default. All linear gains = 0 by default
	STA_SetNumChannels(g_mixer1, nout);  					//add more output channels (sub-mixers)

	//add 2 balances
	g_bal[FRONT] = STA_AddModule(STA_DSP0, STA_MIXE_BALANCE); 	//for Front
	g_bal[REAR]  = STA_AddModule(STA_DSP0, STA_MIXE_BALANCE); 	//for Back
	STA_MixerBindTo(g_mixer1, 0, g_bal[FRONT], STA_LEFT);
	STA_MixerBindTo(g_mixer1, 1, g_bal[REAR],  STA_LEFT);
	STA_MixerBindTo(g_mixer1, 2, g_bal[FRONT], STA_RIGHT);
	STA_MixerBindTo(g_mixer1, 3, g_bal[REAR],  STA_RIGHT);

	//add 2 Faders
	g_fad[LEFT]  = STA_AddModule(STA_DSP0, STA_MIXE_FADER);		//for Left
	g_fad[RIGHT] = STA_AddModule(STA_DSP0, STA_MIXE_FADER);		//for Right
	STA_MixerBindTo(g_mixer1, 0, g_fad[LEFT],  STA_FRONT);
	STA_MixerBindTo(g_mixer1, 1, g_fad[LEFT],  STA_REAR);
	STA_MixerBindTo(g_mixer1, 2, g_fad[RIGHT], STA_FRONT);
	STA_MixerBindTo(g_mixer1, 3, g_fad[RIGHT], STA_REAR);

	//CONNECTIONS
	FORi(nin)  STA_Connect(STA_XIN0,  i,  g_mixer1,   i);	//ins
	STA_Connect(g_mixer1,   0,  STA_XOUT0, 0);	//out
	STA_Connect(g_mixer1,   1,  STA_XOUT0, 2);	//out
	STA_Connect(g_mixer1,   2,  STA_XOUT0, 1);	//out
	STA_Connect(g_mixer1,   3,  STA_XOUT0, 3);	//out

	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//START
	STA_Play();
	SLEEP(5); 		//wait 5ms for DSP initialisation

	//POST DSP-INIT

	//Since all linear gains are 0 by default, we need to set some gains.
	//Each mixer applies x InGains x Softmute x volume
	STA_MixerSetInGains(g_mixer1, 0xFFFF, 0xFFFF, 0/*db*/);	//set all in_g = 1
	STA_MixerSetInGain(g_mixer1, 0, 0/*db*/);	//set ch 0 in_g = 1
	STA_MixerSetVolume(g_mixer1, 0xFFFF, 0/*db*/);			//set all out volume_g = 1
#ifdef MIXER_VER_1
	STA_MixerSetMute(g_mixer1, 0xFFFF, FALSE, FALSE, 1/*don't update depth*/);
#endif

//	STA_MixerSetBalanceLimit(g_bal[0], );
//	STA_MixerSetBalanceCenter(g_bal[0], );
}
//---------------------------------------------------------
static void BuildFlow_2()
{
	STA_Reset();

	//MODULES

	//add 2 Mixers 2ins_2ch
	g_mixer[FRONT] = STA_AddModule(STA_DSP0, STA_MIXE_2INS_NCH);//2 channels by default. All linear gains = 0 by default
	g_mixer[REAR]  = STA_AddModule(STA_DSP0, STA_MIXE_2INS_NCH);//2 channels by default. All linear gains = 0 by default
	STA_SetNumChannels(g_mixer[FRONT], 2);  					//add more output channels (sub-mixers)
	STA_SetNumChannels(g_mixer[REAR],  2);  					//add more output channels (sub-mixers)

	//add 2 balances
	g_bal[FRONT] = STA_AddModule(STA_DSP0, STA_MIXE_BALANCE); 	//for Front
	g_bal[REAR]  = STA_AddModule(STA_DSP0, STA_MIXE_BALANCE); 	//for Back
	STA_MixerBindTo(g_mixer[FRONT], 0, g_bal[FRONT], STA_LEFT);
	STA_MixerBindTo(g_mixer[FRONT], 1, g_bal[FRONT], STA_RIGHT);
	STA_MixerBindTo(g_mixer[REAR],  0, g_bal[REAR],  STA_LEFT);
	STA_MixerBindTo(g_mixer[REAR],  1, g_bal[REAR],  STA_RIGHT);

	//add 2 Faders
	g_fad[LEFT]  = STA_AddModule(STA_DSP0, STA_MIXE_FADER);		//for Left
	g_fad[RIGHT] = STA_AddModule(STA_DSP0, STA_MIXE_FADER);		//for Right
	STA_MixerBindTo(g_mixer[FRONT], 0, g_fad[LEFT],  STA_FRONT);
	STA_MixerBindTo(g_mixer[FRONT], 1, g_fad[RIGHT], STA_FRONT);
	STA_MixerBindTo(g_mixer[REAR],  0, g_fad[LEFT],  STA_REAR);
	STA_MixerBindTo(g_mixer[REAR],  1, g_fad[RIGHT], STA_REAR);

	//CONNECTIONS

	//Front MIXER inputs
	STA_Connect(STA_XIN0,  0,  g_mixer[FRONT],  0); //L0
	STA_Connect(STA_XIN0,  2,  g_mixer[FRONT],  1); //L1
	STA_Connect(STA_XIN0,  1,  g_mixer[FRONT],  2); //R0
	STA_Connect(STA_XIN0,  3,  g_mixer[FRONT],  3); //R1

	//Rear MIXER inputs
	STA_Connect(STA_XIN0,  0,  g_mixer[REAR],   0); //L0
	STA_Connect(STA_XIN0,  2,  g_mixer[REAR],   1); //L1
	STA_Connect(STA_XIN0,  1,  g_mixer[REAR],   2); //R0
	STA_Connect(STA_XIN0,  3,  g_mixer[REAR],   3); //R1

	//XOUT
	STA_Connect(g_mixer[FRONT],  0,  STA_XOUT0, 0);
	STA_Connect(g_mixer[FRONT],  1,  STA_XOUT0, 1);
	STA_Connect(g_mixer[REAR],   0,  STA_XOUT0, 2);
	STA_Connect(g_mixer[REAR],   1,  STA_XOUT0, 3);

	//BUILD
	STA_BuildFlow();
	ASSERT(STA_GetError() == STA_NO_ERROR);

	//START
	STA_Play();
	SLEEP(5); 		//wait 5ms for DSP initialisation

	//POST DSP-INIT

	//Since all linear gains are 0 by default, we need to set some gains.
	//Each mixer applies x InGains x Softmute x volume
	//FRONT MIXER
	STA_MixerSetInGains(g_mixer[FRONT], 0xFFFF, 0xFFFF, 0/*db*/);	//set all in_g = 1
	STA_MixerSetInGain(g_mixer[FRONT], 0, 0/*db*/);	//set ch 0 in_g = 1
	STA_MixerSetVolume(g_mixer[FRONT], 0xFFFF, 0/*db*/);			//set all out volume_g = 1
#ifdef MIXER_VER_1
	STA_MixerSetMute(g_mixer[FRONT], 0xFFFF, FALSE, FALSE, 1/*don't update depth*/);
#endif

	//REAR MIXER
	STA_MixerSetInGains(g_mixer[REAR], 0xFFFF, 0xFFFF, 0/*db*/);	//set all in_g = 1
	STA_MixerSetInGain(g_mixer[REAR], 0, 0/*db*/);	//set ch 0 in_g = 1
	STA_MixerSetVolume(g_mixer[REAR], 0xFFFF, 0/*db*/);			//set all out volume_g = 1
#ifdef MIXER_VER_1
	STA_MixerSetMute(g_mixer[REAR], 0xFFFF, FALSE, FALSE, 1/*don't update depth*/);
#endif

//	STA_MixerSetBalanceLimit(g_bal[0], );
//	STA_MixerSetBalanceCenter(g_bal[0], );
}

//---------------------------------------------------------
//Test Mixer Balance and Fader
//
static void Test_BalanceFader(void)
{
	int i,j;

	//loop through all the test cases j
	FORj(9)
	{
		//set balance / fade
		if (j>0) {	//skip setting of default balance/fader to test them.
		STA_MixerSetBalance(g_bal[FRONT], g_case[j].bal);
		STA_MixerSetBalance(g_bal[REAR],  g_case[j].bal);
		STA_MixerSetBalance(g_fad[LEFT],  g_case[j].fad);
		STA_MixerSetBalance(g_fad[RIGHT], g_case[j].fad);
		}

		//send audio input
		FORi(4) XIN[i] = FRAC(g_in0[i]);
		SWAP_DP(3);

		//check output
		FORi(4)
			CHECKF(FLT(XOUT[i]), g_case[j].out[i]);
	}

}
//---------------------------------------------------------

void TestMixerBalance(void)
{
	PRINTF("TestMixerBalance... ");


	BuildFlow_1();
	Test_BalanceFader();

	BuildFlow_2();
	Test_BalanceFader();

	//stop and clean
	STA_Reset();
	PRINT_OK_FAILED;
}


#endif //0
