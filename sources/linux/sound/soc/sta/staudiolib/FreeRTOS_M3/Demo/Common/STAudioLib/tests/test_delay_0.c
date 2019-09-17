/*
 * STAudioLib - test_delay_0.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing DELAY module (2ch by default)

	/!\ THIS TEST DOES NOT USE DSP @48Khz, instead triggers XIN/XOUT swaps manually

	           DELAY
	          +-------+
	L XIN[0]--|Delay0 |--XOUT[0]
	R XIN[1]--|Delay1 |--XOUT[1]
	          +-------+

 */

#if 1
#include "staudio_test.h"


static STAModule g_delay;

//---------------------------------------------------------
//test DELAY in asynchronous mode (not based on 48kH frames)
//note: an input data in XIN takes 3 frames to arrive to XOUT
//
static void Test1_delay(int delay)
{
	int i;

	STA_DelaySetDelays(g_delay, 0x1, delay);
	STA_DelaySetDelays(g_delay, 0x2, delay);

	for (i = 1; i < delay + 7/*min 5*/; i++)
	{
		XIN[0] = 0x00A000 | i;
		XIN[1] = 0x00B000 | i;

		//note: 2 dual ports + 1 modules imply a non compressible 3-sample delay
		if (i > delay + 3) {
			int outFrame = i - delay - 3;
			CHECK(XOUT[0] == (0x00A000 | outFrame));
			CHECK(XOUT[1] == (0x00B000 | outFrame));
		}

		SWAP_DP(1);
	}

}

//---------------------------------------------------------
static void BuildFlow(STA_ModuleType delayType)
{
	int i;

	STA_Reset();

	//add DELAY
	g_delay = STA_AddModule(STA_DSP0, delayType);
	STA_DelaySetLengths(g_delay, 0x1, 100);
	STA_DelaySetLengths(g_delay, 0x2, 100);

	//connect
	FORi(2) {
	STA_Connect(STA_XIN0,  i,  g_delay,   i);
	STA_Connect(g_delay,   i,  STA_XOUT0, i);
	}

	//build
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);
}
//---------------------------------------------------------
void TestDelay0(void)
{
	PRINTF("TestDelay0... ");

	//test DELAY Y ------------------------
	BuildFlow(STA_DLAY_Y_2CH);

	STA_Play();
	SLEEP(2);					//wait 5ms for DSP initialisation

	Test1_delay(0);
	Test1_delay(1);
	Test1_delay(24);

	//test DELAY Y -----------------------
	BuildFlow(STA_DLAY_X_2CH);

	STA_Play();
	SLEEP(2);					//wait 5ms for DSP initialisation

	Test1_delay(0);
	Test1_delay(1);
	Test1_delay(24);

	//-----------------------------------

	//stop and clean
	STA_Reset();
	PRINT_OK_FAILED;
}



#endif //0
