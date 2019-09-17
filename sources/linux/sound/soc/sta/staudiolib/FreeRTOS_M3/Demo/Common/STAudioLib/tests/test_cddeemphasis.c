/*
 * STAudioLib - test_cddeemphasis.c
 *
 * Created on 2015/06/16 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	testing:



*/

#if 0
#include "staudio_test.h"

//---------
// MODULES
//---------
static STAModule	g_deemph;

static int g_deemph_mode = STA_DEEMPH_ON;


//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void DSP_Flow(void)
{
	int i;

	STA_Reset();

	//CD-DEEMPH
	g_deemph = STA_AddModule(STA_DSP0, STA_CD_DEEMPH);
	STA_SetMode(g_deemph, g_deemph_mode);


	//CONNECTIONS
	FORi(2) {
		#if 1
		STA_Connect(STA_XIN0,  i,  g_deemph,  i);
		STA_Connect(g_deemph,  i,  STA_XOUT0, i);
		#else
		STA_Connect(STA_XIN0,  i,  STA_XOUT0,  i);  //TMP
		#endif
    }

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);

	//POST-BUILT INIT

	//START DSP
	STA_StartDSP(STA_DSP0);
	SLEEP(100);					//wait 100ms for DSP initialisation

	//POST-DSP INIT


	//DMABUS
	//DSP input
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,		STA_DMA_DSP0_XIN,	2);
	//DSP output
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,  	STA_DMA_SAI3_TX1,	2);		//XOUT to SAI3
	//DSP XIN/XOUT swap
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP0_XIN+127, 1);	//swap trigger


}


//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------

static void keyCallback(int key, bool down)
{
	char* deemphMode[] = {"OFF", "ON", "AUTO"};

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
	case 0x14:  PLY_SetState(PLY_STOP); return;

	//CD-DEEMPH
	case 0x21:
		g_deemph_mode ^= 1;
		STA_SetMode(g_deemph, g_deemph_mode);
		PRINTF("DEEMPH = %s\n", deemphMode[g_deemph_mode]);
		break;
	}
}

static void rtxCallback(int steps)
{
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void Test_cddeemphasis(void)
{

	PRINTF("Test_cddeemphasis... \n");


	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//mini player
	PLY_Init();
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, 2, 16, 16000);

	//STA / DMABUS
	DSP_Flow();


	//----- play (in loop)-------

	PLY_SetState(PLY_PLAY);	//starts MSP2/SAI2, SAI3,
	STA_DMAStart();


	while (1) {

		SLEEP(100);	//avoid sticky loop
	}

	//stop and clean
	STA_Reset();
}

#endif //0
