/*
 * STAudioLib - test_MSP2_16k.c
 *
 * Created on 2015/06/16 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	testing:

	                   DMABUS
	MSP2/SAI2 --> SRC0 -----> SAI3/DAC
	16 Khz, 16b stereo
	48 Khz, 16b stereo

*/

#if 1
#include "staudio_test.h"


//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------

static void keyCallback(int key, bool down)
{
	/*
	key mapping:
	[SRC]        [    ]	    [  ]  [  ]  [  ]  [  ]
							[  ]  [  ]  [  ]  [  ]
	[PLAY/PAUS]  [STOP] 	[  ]  [  ]  [  ]  [  ]
	*/

	switch(key) {

	//PLAY/PAUSE
	case 0x13:  PLY_TogglePlayPause(); return;

	//STOP
	case 0x14:  PLY_SetState(PL_STOP); return;

	}
}

static void rtxCallback(int steps)
{
}

//---------------------------------------------------------
// test MSP2/SAI2 48kz
//---------------------------------------------------------
/*
void test_msp2_48k()
{
	PRINTF("Playing MSP2 48k... \n");

	//MSP2 / AIF
//	PLY_Set_PCM_48Khz_16bit_MSP2( g_songAddr + 11, g_songSize - 11*4, 2);
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, 2, 16, 48000);

	//DMABUS
//	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	STA_DMA_SAI3_TX1,	2);
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,		STA_DMA_SAI3_TX1,	2);
}

void test_msp2_16k()
{
	PRINTF("Playing MSP2 16k... \n");

	//MSP2 / AIF
//	PLY_Set_PCM_16Khz_16bit_MSP2( g_songAddr + 11, g_songSize - 11*4, 2);
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, 2, 16, 16000);

	//DMABUS
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,		STA_DMA_SAI3_TX1,	2);
}
*/

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestMSP2_16k(void)
{
	int nch  = 2;
	int freq = 16000;
	int bits = 16;

	PRINTF("TestMSP2 (%d Khz)... \n", freq);


	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//mini player
	PLY_Init();
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, nch, bits, freq);

	//STA/DMABUS
	STA_Reset();
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,		STA_DMA_SAI3_TX1,	nch);


	//----- play (in loop)-------

	PLY_SetState(PL_PLAY);	//starts MSP2/SAI2, SAI3,
	STA_DMAStart();

	while (1) {

		SLEEP(100);	//avoid sticky loop
	}

	//stop and clean
	STA_Reset();
}

#endif //0
