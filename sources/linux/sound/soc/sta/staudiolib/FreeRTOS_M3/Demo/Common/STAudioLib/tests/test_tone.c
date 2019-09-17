/*
 * STAudioLib - test_tone.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing Tone module (static stereo)

	          GAINS      TONES
	        +-------+  +-------+
	XIN[0]--| Gain0 |--| Tone0 |--XOUT[0]
	XIN[1]--| Gain1 |--| Tone1 |--XOUT[1]
	        +-------+  +-------+

 */

#if 0
#include "staudio_test.h"


static STAModule g_gains;
static STAModule g_tones;


//---------------------------------------------------------
static void BuildFlow(void)
{
	int i;

	//GAINS
	g_gains = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); 	//2 gains by default
//	STA_SetNumChannels(g_gains, 2);  							//add more channels
//	STA_GainSetGain(g_gains, 0, -60);				//-6dB (amplitude ~ x0.5) to channel 0
//	STA_GainSetGain(g_gains, 1, -60);				//-6dB (amplitude ~ x0.5) to channel 0

	//TONES
	g_tones = STA_AddModule(STA_DSP0, STA_TONE_STATIC_STEREO_DP);
	STA_SetMode(g_tones, STA_TONE_AUTO_BASS_Q);						//enable Tone
	//Set all the Tone params as default in STAudio and ADR3
/*	STA_ToneSetTypes(g_tones, STA_TONE_BANDBOOST_2, STA_TONE_BANDBOOST_2); //bass and trebble types
	STA_ToneSetAutoBassQLevels(g_tones, -96, 96, 1, 60);			// as default in ADR3
	STA_SetFilterParam(g_tones, STA_TONE_BASS, STA_GAIN, 0);		// 0 dB
	STA_SetFilterParam(g_tones, STA_TONE_BASS, STA_QUAL, 30);		// 3     <-- AUTO SET
	STA_SetFilterParam(g_tones, STA_TONE_BASS, STA_FREQ, 200);		// 200 Hz
	STA_SetFilterParam(g_tones, STA_TONE_MIDL, STA_GAIN, 0);		// 0 dB
	STA_SetFilterParam(g_tones, STA_TONE_MIDL, STA_QUAL, 30);		// 3
	STA_SetFilterParam(g_tones, STA_TONE_MIDL, STA_FREQ, 3000);		// 3000 Hz
	STA_SetFilterParam(g_tones, STA_TONE_TREB, STA_GAIN, 0);		// 0 dB
	STA_SetFilterParam(g_tones, STA_TONE_TREB, STA_QUAL, 30);		// 3
	STA_SetFilterParam(g_tones, STA_TONE_TREB, STA_FREQ, 16000);	// 16000 Hz
*/
	//CONNECTIONS
	FORi(2) {
		STA_Connect(STA_XIN0,  i,  g_gains,   i);
		STA_Connect(g_gains,   i,  g_tones,   i);
		STA_Connect(g_tones,   i,  STA_XOUT0, i);
	}

	//DMABUS
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_SRC0_DO, STA_DMA_DSP0_XIN+127, 1);	//swap trigger


	//BUILD
	STA_BuildFlow();
}
//---------------------------------------------------------
void TestTone(void)
{
	PRINTF("TestTone... ");

	STA_Reset();

	BuildFlow();

	//start
	STA_Play();
	SLEEP(5);					//wait 5ms for DSP initialisation

//	Test1_XINtoXOUT();

	//stop and clean
	STA_Reset();
	PRINT_OK_FAILED;
}

#endif //0
