/*
 * STAudioLib - test_dsp_ahb_io.c
 *
 * Created on 2015/06/16 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	testing:


	                  DMABUS       DMA1C1    DSP      DMA1C2       DMABUS
	MSP2/SAI2 -> SRC0 -----> FIFO1 -----> XIN -> XOUT -----> FIFO2 -----> SAI3/DAC
	16 or 48 Khz stereo

*/

#if 0
#include "staudio_test.h"



#define DSP_SEL				0

#if (DSP_SEL == 0)
#define DMA_XIN 			STA_DMA_DSP0_XIN
#define DMA_XOUT 			STA_DMA_DSP0_XOUT
#define MOD_XIN				STA_XIN0
#define MOD_XOUT			STA_XOUT0
#define XIN_DSPAHB_ADDR		((volatile u32*)0x48984000)
#define XOUT_DSPAHB_ADDR	((volatile u32*)0x48984200)
#elif (DSP_SEL == 1)
#define DMA_XIN 			STA_DMA_DSP1_XIN
#define DMA_XOUT 			STA_DMA_DSP1_XOUT
#define MOD_XIN				STA_XIN1
#define MOD_XOUT			STA_XOUT1
#define XIN_DSPAHB_ADDR		((volatile u32*)0x48a84000)
#define XOUT_DSPAHB_ADDR	((volatile u32*)0x48a84200)
#else
#define DMA_XIN 			STA_DMA_DSP2_XIN
#define DMA_XOUT 			STA_DMA_DSP2_XOUT
#define MOD_XIN				STA_XIN2
#define MOD_XOUT			STA_XOUT2
#define XIN_DSPAHB_ADDR		((volatile u32*)0x48b8c000)
#define XOUT_DSPAHB_ADDR	((volatile u32*)0x48b8c200)
#endif

//-----------
// DSP/AHB IO
//-----------

#define NUM_DSP_IN_SAMPLES		2
#define NUM_DSP_OUT_SAMPLES		2



//---------------------------------------------------------
// BUILD FLOW
//---------------------------------------------------------

static void initDSP(void)
{
}


static void initDMABUS(int nch)
{
}

//---------------------------------------------------------
// DMA1 task
//---------------------------------------------------------

static void dma1_audio_io_isr(DMA_Typedef *dma, u32 ch)
{
	//PRINTF("dma1_isr(%d)\n", ch);

	//send IRQ to DSP
	//(irq cleared automatically in hw)
	switch (DSP_SEL) {
	case 0: AUDCR->ACO_EIR0.BIT.ARM2DSP0_INT1 = 0; break;
	case 1: AUDCR->ACO_EIR1.BIT.ARM2DSP1_INT1 = 0; break;
	case 2: AUDCR->ACO_EIR2.BIT.ARM2DSP2_INT1 = 0; break;
	}

	//wait for the channels to complete the previous transfers
	while ( (DMA1_C1CFG & 1) | (DMA1_C2CFG & 1));

	//prep new transfers for next frame:

	//DMA1C1: FIFO1 -> DSP-IN
	DMA_SetTransfer(DMA1, 1, (u32*)FIFO1_OUT, (u32*) XIN_DSPAHB_ADDR,
					DMA_DI | DMA_P2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_SBUSRT_1 | DMA_DBUSRT_1 | DMA_TCIEN, //CR
					DMA_P2P_DMACTL | DMA_SREQ(DMA1_DSPFIFO1)| DMA_ITC | DMA_ENABLE,				//CFG
					NUM_DSP_IN_SAMPLES); 	//numTransfer


	//DMA1C2: DSP-OUT -> FIFO2
	DMA_SetTransfer(DMA1, 2, (u32*)XOUT_DSPAHB_ADDR, (u32*) FIFO2_IN,
					DMA_SI | DMA_P2P | DMA_SBIT_32 | DMA_DBIT_32 | DMA_SBUSRT_1 | DMA_DBUSRT_1, // | DMA_TCIEN, 	//CR
					DMA_P2P_DMACTL | DMA_DREQ(DMA1_DSPFIFO2)| DMA_ENABLE, // | DMA_ITC,				//CFG
					NUM_DSP_OUT_SAMPLES); 	//numTransfer

}


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
	case 0x14:  PLY_SetState(PLY_STOP); return;

	}
}

static void rtxCallback(int steps)
{
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestDspAhbIO(void)
{
	int nch  = 2;
	int freq = 16000;
	int bits = 16;

	PRINTF("TestDspAhbIO (%d Khz)... \n", freq);


	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//mini player
	PLY_Init();
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, nch, bits, freq);

	//DSP
	STA_Reset();
	STA_StartDSP(DSP_SEL);
	SLEEP(100);					//wait 100ms for DSP initialisation

	//DMABUS
	STA_DMAReset();
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,		STA_DMA_FIFO_IN,	NUM_DSP_IN_SAMPLES);
	STA_DMAAddTransfer(STA_DMA_FIFO_OUT, 	STA_DMA_SAI3_TX1,	NUM_DSP_OUT_SAMPLES);


	//DMA1C1/C2
	DMA1_C1CFG  = 0;			//stop DMA1_C1 (FIFOs only works with DMA1)
	DMA1_C2CFG  = 0;			//stop DMA1_C2 (FIFOs only works with DMA1)
	DMA_SetTCIRQHandler(DMA1, 1, dma1_audio_io_isr);
#ifdef CORTEX_M3
	NVIC_EnableIRQChannel(EXT9_IRQChannel,  0, 0);	//enable DMA1 IRQ to M3
#else
	//TODO: set VIC for R4...
#endif


	//FIFOs
	FIFO_Reset(16, 16);			//reset FIFO
	FIFO_SetDMAReadRequest(TRUE);
	FIFO_SetDMAWriteRequest(TRUE);

	//----- play (in loop)-------

	PLY_SetState(PLY_PLAY);	//starts MSP2/SAI2, SAI3,

	//DMABUS
	STA_DMAStart();

	//DMA1C1/C2
	dma1_audio_io_isr(DMA1, 1);



	while (1) {

		SLEEP(100);	//avoid sticky loop
	}

	//stop and clean
	STA_Reset();
	STA_SetDspISR(DSP_SEL, 0);
}

#endif //0
