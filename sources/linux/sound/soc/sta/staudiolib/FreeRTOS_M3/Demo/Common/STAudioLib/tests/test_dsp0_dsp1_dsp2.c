/*
 * STAudioLib - test_dsp0_dsp1.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing 3 chained DSPs: DSP0 -> DSP1 -> DSP2 -> DAC

	              DSP0        DMABUS        DSP1        DMABUS        DSP2
	     +---------------------+   +---------------------+   +---------------------+
	L ==>|XIN[0]--DLY0--XOUT[0]|==>|XIN[0]--DLY1--XOUT[0]|==>|XIN[0]--DLX2--XOUT[0]|==> SAI3 -> DAC
	R ==>|XIN[1]--DLY0--XOUT[1]|==>|XIN[1]--DLY1--XOUT[1]|==>|XIN[1]--DLX2--XOUT[1]|==> SAI3 -> DAC
	     +---------------------+   +---------------------+   +---------------------+


 */

#if 1
#include "staudio_test.h"

#define DSP_SEL					0

extern RAWINFO g_song1;
extern RAWINFO g_song2;

static RAWINFO* g_song = &g_song1;
static MSP_TypeDef *g_MSP;

//---------
// MODULES
//---------
static STAModule	g_delay0;	//on DSP0
static STAModule	g_delay1;	//on DSP1
static STAModule	g_delay2;	//on DSP2


//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------
static void DSP_Flow0_1_2(void)
{
	int i;

	STA_Reset();

	//DELAY 0 -> DSP0
	g_delay0 = STA_AddModule(STA_DSP0, STA_DLAY_Y_2CH);
	STA_DelaySetLengths(g_delay0, 0x3, 1000);
	STA_DelaySetDelays( g_delay0, 0x3, 999);

	//DELAY 1 -> DSP1
	g_delay1 = STA_AddModule(STA_DSP1, STA_DLAY_Y_2CH);
	STA_DelaySetLengths(g_delay1, 0x3, 1000);
	STA_DelaySetDelays( g_delay1, 0x3, 999);

	//DELAY 2 -> DSP2
	g_delay2 = STA_AddModule(STA_DSP2, STA_DLAY_X_2CH);
	STA_DelaySetLengths(g_delay2, 0x3, 1500);
	STA_DelaySetDelays( g_delay2, 0x3, 1499);

	//CONNECTIONS
	FORi(2) {

		//DSP0
		STA_Connect(STA_XIN0,  i,  g_delay0,   i);
		STA_Connect(g_delay0,  i,  STA_XOUT0,  i);

		//DSP1
		STA_Connect(STA_XIN1,  i,  g_delay1,   i);
		STA_Connect(g_delay1,  i,  STA_XOUT1,  i);

		//DSP2
		STA_Connect(STA_XIN2,  i,  g_delay2,   i);
		STA_Connect(g_delay2,  i,  STA_XOUT2,  i);

		//TMP
//		STA_Connect(STA_XIN0,  i,  STA_XOUT0,  i);	//(shortcut all dsp0 effects) for debug
//		STA_Connect(STA_XIN1,  i,  STA_XOUT1,  i);	//(shortcut all dsp1 effects) for debug
	}

	//BUILD
	STA_BuildFlow();

	ASSERT(STA_GetError() == STA_NO_ERROR);


	//DMABUS
	STA_DMAReset();
#if 1
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	STA_DMA_DSP0_XIN, 2);		//SAI2 to DSP0
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,  	STA_DMA_DSP1_XIN, 2);		//DSP0 to DSP1
	STA_DMAAddTransfer(STA_DMA_DSP1_XOUT,  	STA_DMA_DSP2_XIN, 2);		//DSP1 to DSP2
	STA_DMAAddTransfer(STA_DMA_DSP2_XOUT,  	STA_DMA_SAI3_TX1, 2);		//DSP2 to SAI3 -> DAC

	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP0_XIN+127, 1);	//DSP0 swap
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP1_XIN+127, 1);	//DSP1 swap
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP2_XIN+127, 1);	//DSP2 swap

#else
	//TMP DEBUG: SAI2 -> DSP0 -> DAC
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	STA_DMA_DSP0_XIN, 2);		//SAI2 to DSP0
	STA_DMAAddTransfer(STA_DMA_DSP0_XOUT,  	STA_DMA_SAI3_TX1, 2);		//DSP0 to SAI3 -> DAC
	STA_DMAAddTransfer(STA_DMA_SAI2_RX1, 	STA_DMA_DSP0_XIN+127, 1);	//DSP0 swap
#endif
}

//---------------------------------------------------------
// play MSP
//---------------------------------------------------------
static void play_MSP(RAWINFO* song)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	PRINTF("Playing %s\n", song->filename);

	DMABUS_Set(g_MSP == MSP1 ? PLY_MSP1 : PLY_MSP2, song->nch, DSP_SEL);

	PLY_Set_pcm_source_MSP(g_MSP, song, PLY_NOFLAG);
	PLY_SetState_MSP(g_MSP, PLY_PLAY);
}


//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestDsp0Dsp1Dsp2(void)
{
	int nch = 2;

	PRINTF("TestDsp0Dsp1Dsp2... \n");

	g_MSP = MSP2;

	//audio init
	DSP_Flow0_1_2();

	//start dsp
	STA_Play();
	SLEEP(100);					//wait for DSP initialisation

	//post initialisation
	//---- mini player ----
	PLY_Init();
	play_MSP(g_song);


	//play (in loop)
	while (1)
	{
        SLEEP(10);
	}


	//stop and clean
	STA_Reset();
//	PRINT_OK_FAILED;
}

#endif //0
