/*
 * STAudioLib - test_mixer4x4.c
 *
 * Created on 2015/03/10 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing Mixer balance (L/R) and fader (R/F)

					MIXER_4INS_4CH

	                              BalR BalF
	            INSET_L            |    |  FaderL
					 |--  FL0  \   |    | /
	                 |-- (mute) \__|___|0>____0__ XOUT[0] (FL)
	SWGF_L (FL0)  _0_|--  L1    /  |   /|
	SWGR_L (RL0)  _1_|--  beep /   |  / |
	SWG1_L (L1)   _2_|             | /  |
	SWG2_L (beep) _3_|-- (mute)\   |/   |
	                 |--  RL0   \_|1>___|_____1__ XOUT[1] (RL)
	                 |--  L1    /  |    |
	                 |--  beep /   |    |
								   |    |
	            INSET_R            |    |  FaderR
	                 |--  FR0  \   |    | /
	                 |-- (mute) \__|___|2>____2__ XOUT[2] (FR)
	SWGF_R (FR0)  _4_|--  R1    /  |   /
	SWGR_R (RR0)  _5_|--  beep /   |  /
	SWG1_R (R1)   _6_|             | /
	SWG2_R (beep) _7_|-- (mute)\   |/
	                 |--  RR0   \_|3>_________3__ XOUT[3] (RR)
	                 |--  R1    /
	                 |--  beep /

 */


#if 1
#include "staudio_test.h"


//SINEWAVES
static STAModule g_sineF, g_sineR, g_sine1, g_sine2;

//MIXER
static STAModule g_mixer;

//BALANCE/FADER
static STAModule g_balanceF, g_balanceR;	//balance for Front and Rear
static STAModule g_faderL,    g_faderR;		//fader   for Left and Right

#define MUTE	-1200
const s16 g_mixer_input_gains_ch0[4] = { 0,   MUTE, -60, -240 };
const s16 g_mixer_input_gains_ch1[4] = { MUTE,   0, -60, -240 };
const s16 g_mixer_input_gains_ch2[4] = { 0,   MUTE, -60, -240 };
const s16 g_mixer_input_gains_ch3[4] = { MUTE,   0, -60, -240 };

//---------------------------------------------------------
static void BuildFlow()
{
	//SINEWAVES
	g_sineF = STA_AddModule(STA_DSP0, STA_SINE_2CH);
	g_sineR = STA_AddModule(STA_DSP0, STA_SINE_2CH);
	g_sine1 = STA_AddModule(STA_DSP0, STA_SINE_2CH);
	g_sine2 = STA_AddModule(STA_DSP0, STA_SINE_2CH);

	//MIXER_4INS_4CH
	g_mixer = STA_AddModule(STA_DSP0, STA_MIXE_4INS_NCH); 	//2 channels by default. All linear gains = 0 by default
	STA_SetNumChannels(g_mixer, 4);  					    //add more output channels (sub-mixers)

	//BALANCE
	g_balanceF = STA_AddModule(STA_DSP0, STA_MIXE_BALANCE); //Front Balance
	g_balanceR = STA_AddModule(STA_DSP0, STA_MIXE_BALANCE); //Rear Balance
	STA_MixerBindTo(g_mixer, 0, g_balanceF, STA_LEFT);
	STA_MixerBindTo(g_mixer, 1, g_balanceR, STA_LEFT);
	STA_MixerBindTo(g_mixer, 2, g_balanceF, STA_RIGHT);
	STA_MixerBindTo(g_mixer, 3, g_balanceR, STA_RIGHT);

	//FADER
	g_faderL = STA_AddModule(STA_DSP0, STA_MIXE_FADER);		//Left Fader
	g_faderR = STA_AddModule(STA_DSP0, STA_MIXE_FADER);		//Right Fader
	STA_MixerBindTo(g_mixer, 0, g_faderL, STA_FRONT);
	STA_MixerBindTo(g_mixer, 1, g_faderL, STA_REAR);
	STA_MixerBindTo(g_mixer, 2, g_faderR, STA_FRONT);
	STA_MixerBindTo(g_mixer, 3, g_faderR, STA_REAR);

	//CONNECTIONS
	//INPUT
	STA_Connect(g_sineF, 0,  g_mixer,   0);
	STA_Connect(g_sineF, 1,  g_mixer,   4);

	STA_Connect(g_sineR, 0,  g_mixer,   1);
	STA_Connect(g_sineR, 1,  g_mixer,   5);

	STA_Connect(g_sine1, 0,  g_mixer,   2);
	STA_Connect(g_sine1, 1,  g_mixer,   6);

	STA_Connect(g_sine2, 0,  g_mixer,   3);
	STA_Connect(g_sine2, 1,  g_mixer,   7);

	//OUPUTS
	STA_Connect(g_mixer, 0,  STA_XOUT0, 0);
	STA_Connect(g_mixer, 1,  STA_XOUT0, 1);
	STA_Connect(g_mixer, 2,  STA_XOUT0, 2);
	STA_Connect(g_mixer, 3,  STA_XOUT0, 3);

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);

	//POST-BUILD initializations
	STA_MixerSetChannelInGains(g_mixer, 0, g_mixer_input_gains_ch0);
	STA_MixerSetChannelInGains(g_mixer, 1, g_mixer_input_gains_ch1);
	STA_MixerSetChannelInGains(g_mixer, 2, g_mixer_input_gains_ch2);
	STA_MixerSetChannelInGains(g_mixer, 3, g_mixer_input_gains_ch3);

	STA_MixerSetVolume(g_mixer, 0xFF, 0/*db*/);			//set all volumes to 0dB
	STA_MixerSetMute(g_mixer, 0xFF, FALSE, FALSE, 1/*don't update depth*/);

}


//---------------------------------------------------------
//Test Mixer Balance and Fader
//
static void Test1_BalanceFader(void)
{
	s32 balance = 0; //db
	s32 fader    = 0; //db

	//set balance
	STA_MixerSetBalance(g_balanceF, balance);
	STA_MixerSetBalance(g_balanceR, balance);

	//set fader
	STA_MixerSetBalance(g_faderL, fader);
	STA_MixerSetBalance(g_faderR, fader);

}
//---------------------------------------------------------

void TestMixer4x4(void)
{
	PRINTF("TestMixer4x4... ");

	STA_Reset();
	BuildFlow();

	//start
	STA_Play();
	SLEEP(100); 		//wait 100ms for DSP initialisation

	Test1_BalanceFader();

	//stop and clean
	STA_Reset();
	PRINT_OK_FAILED;
}


#endif //0
