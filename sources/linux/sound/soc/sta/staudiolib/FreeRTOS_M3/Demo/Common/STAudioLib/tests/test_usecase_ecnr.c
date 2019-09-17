/*
 * STAudioLib - test_usecase_ecnr.c
 *
 * Created on 2015/06/16 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	testing:

	- USE CASE 1: MSP2 48k 16b stereo

*/

#if 1
#include "staudio_test.h"


#define BYPASS_DSP			0

#define DSP_SEL				2

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


// DSP/AHB IO
#define NUM_DSP_IN_SAMPLES		2
#define NUM_DSP_OUT_SAMPLES		2


//---------------------------------------------------------
// DMA1 task (for "DSP AHB IO" workaround)
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

	//wait for the channels to complete the previous transfers <-- sometime blocking !!!
//	while ( (DMA1_C1CFG & 1) | (DMA1_C2CFG & 1));

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
// Test cases
//---------------------------------------------------------
/*
	                 DMABUS       DMA1C1     DMA1C2       DMABUS
	MSP2/SAI2 / SRC0 -----> FIFO1 -----> DSP -----> FIFO2 -----> SAI3/DAC
	48 Khz stereo
*/
void set_msp2_48k_to_48k()
{
	PRINTF("Playing MSP2(48k) > DSP(48k) > DAC... \n");

	//MSP2 > SAI2 > AIMUX > LPFSRC0 > AOMUX > SAI3
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, 2, 16, 48000);

	//DMABUS
	#if BYPASS_DSP
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,	STA_DMA_SAI3_TX1, 2);
	#else
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,	 STA_DMA_FIFO_IN, 2);
	STA_DMAAddTransfer(STA_DMA_FIFO_OUT, STA_DMA_SAI3_TX1, 2);
	#endif
}


/*
	                 DMABUS       DMA1C1     DMA1C2       DMABUS
	MSP2/SAI2 / SRC0 -----> FIFO1 -----> DSP -----> FIFO2 -----> SAI3/DAC
	16 Khz stereo
*/
void set_msp2_16k_to_48k()
{
	PRINTF("Playing MSP2(16k) > SRC0(UPx2) > DSP(48k) > DAC... \n");

	//MSP2 > SAI2 > AIMUX > LPFSRC0 > AOMUX > SAI3
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, 2, 16, 16000);

	//DMABUS
	#if BYPASS_DSP
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,	STA_DMA_SAI3_TX1, 2);
	#else
	STA_DMAAddTransfer(STA_DMA_SRC0_DO,	 STA_DMA_FIFO_IN, 2);
	STA_DMAAddTransfer(STA_DMA_FIFO_OUT, STA_DMA_SAI3_TX1, 2);
	#endif
}


/*
	          DMABUS       DMA1C1     DMA1C2       DMABUS
	MSP2/SAI2 -----> FIFO1 -----> DSP -----> FIFO2 -----> SRC0 / SAI3/DAC
	16 Khz stereo
*/
void set_msp2_16k_to_16k()
{
	PRINTF("Playing MSP2(16k) > DSP(16k) > SRC0(UPx2) > DAC... \n");

	//MSP2
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, 2, 16, 16000);


	//LPFSRC_0:  LPFDI > UPx2 > SAI3
	AIF->AIMUX.BIT.CH0SEL = AIMUX_LPFDI;
	AIF->AOMUX.BIT.CH0SEL = AOMUX_SAI3TX0;

//	AIF->LPF_CR.BIT.FS_0  = LPF0_UPx2;  // <-- NOT WORKING !?
	AIF->LPF_CR.BIT.FS_0  = LPF_FIR_0;

	AIF->SRC_CR[0].BIT.DRLL_THRES = 0x7;

	//FSYNCIN/FSYNCOUT_0
	#if 1
	AIF->FSYNCIN.BIT.CH0SEL  = FSYNCIO_CK1;
	AIF->FSYNCOUT.BIT.CH0SEL = FSYNCIO_FS;
	#else
	AIF->FSYNCIN.BIT.CH0SEL  = FSYNCIO_FS;
	AIF->FSYNCOUT.BIT.CH0SEL = FSYNCIO_FS;
	#endif

	//DMABUS
	#if BYPASS_DSP
	STA_DMAAddTransfer2(STA_DMA_SAI2_RX1,	STA_DMA_LPF0_DI, 2, STA_DMA_FSCK1_SLOT0);
//	STA_DMAAddTransfer(STA_DMA_SAI2_RX1,	STA_DMA_LPF0_DI, 2);
	#else
	STA_DMAAddTransfer2(STA_DMA_SAI2_RX1,	STA_DMA_FIFO_IN, 2, STA_DMA_FSCK1_SLOT0);
	STA_DMAAddTransfer2(STA_DMA_FIFO_OUT, 	STA_DMA_LPF0_DI, 2, STA_DMA_FSCK1_SLOT0);
	#endif
}

void set_msp2_16k_to_16k_workaround_cascading_2_srcs()
{
	PRINTF("Playing MSP2(16k) > DSP(16k) > SRC0(BY) > SRC1(BY) > DAC... \n");

	//MSP2
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, 2, 16, 16000);

	//Workaround
	//SRC0 16 --> 24kHz (FsIN= FSCK1 = Fs/3 = 16kHz ; FsOUT = Fs 24)
	//SRC1 24 --> 48kHz (FsIN= FSCK2 = Fs/2 = 24kHz ; FsOUT = Fs 48)
	//all LPF in bypass

	//CK1, CK2
	AUDDMA->DMA_CR.BIT.FSYNC_CK1_SEL = STA_DMA_FSCK12_16KHZ;
	AUDDMA->DMA_CR.BIT.FSYNC_CK2_SEL = STA_DMA_FSCK12_24KHZ;


	//--- LPFSRC_0:  LPFDI_0 > Bypass > SRCDO_0 ---
	AIF->AIMUX.BIT.CH0SEL		= AIMUX_LPFDI;
	AIF->AOMUX.BIT.CH0SEL		= AOMUX_SRCDO;

	AIF->FSYNCIN.BIT.CH0SEL		= FSYNCIO_CK1; //16k
	AIF->FSYNCOUT.BIT.CH0SEL	= FSYNCIO_CK2; //24k

	AIF->LPF_CR.BIT.FS_0		= LPF_BYPASS; //LPF_BYPASS LPF_FIR_0, LPF0_UPx2
	AIF->SRC_CR[0].BIT.DRLL_THRES = 0x7;


	//--- LPFSRC_0:  LPFDI_1 > Bypass > SAI3 ---

	AIF->AIMUX.BIT.CH1SEL		= AIMUX_LPFDI;
	AIF->AOMUX.BIT.CH1SEL		= AOMUX_SAI3TX0;

	AIF->FSYNCIN.BIT.CH1SEL		= FSYNCIO_CK2; //24k
	AIF->FSYNCOUT.BIT.CH1SEL	= FSYNCIO_FS;  //48k

	AIF->LPF_CR.BIT.FS_1		= LPF_BYPASS; //LPF_BYPASS LPF_FIR_0, LPF0_UPx2
	AIF->SRC_CR[1].BIT.DRLL_THRES = 0x7;


	//--- DMABUS ---
	#if BYPASS_DSP
	STA_DMAAddTransfer2(STA_DMA_SAI2_RX1,	STA_DMA_LPF0_DI, 2, STA_DMA_FSCK1_SLOT0); //16k
	STA_DMAAddTransfer2(STA_DMA_SRC0_DO,	STA_DMA_LPF1_DI, 2, STA_DMA_FSCK2_SLOT1); //24k
	#else
	//DSP input
	STA_DMAAddTransfer2(STA_DMA_SAI2_RX1,	STA_DMA_FIFO_IN, 2, STA_DMA_FSCK1_SLOT0); //16k
	//DSP output
	STA_DMAAddTransfer2(STA_DMA_FIFO_OUT, 	STA_DMA_LPF0_DI, 2, STA_DMA_FSCK1_SLOT0); //16k
	STA_DMAAddTransfer2(STA_DMA_SRC0_DO, 	STA_DMA_LPF1_DI, 2, STA_DMA_FSCK2_SLOT1); //24k
	#endif
}

void set_msp2_16k_to_16k_noUPtoDAC()
{
	PRINTF("Playing MSP2(16k) > DSP(16k) > DAC... \n");

	//MSP2
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, 2, 16, 16000);

	//AIMUX / AOMUX  --> LPFSRC0
	AIF->AIMUX.BIT.CH0SEL = AIMUX_NOINPUT;
	AIF->AOMUX.BIT.CH0SEL = AOMUX_NOOUTPUT;

	//DMABUS
	#if BYPASS_DSP
	STA_DMAAddTransfer2(STA_DMA_SAI2_RX1,	STA_DMA_SAI3_TX1, 2, STA_DMA_FSCK1_SLOT0);
	#else
	STA_DMAAddTransfer2(STA_DMA_SAI2_RX1,	STA_DMA_FIFO_IN,  2, STA_DMA_FSCK1_SLOT0);
	STA_DMAAddTransfer2(STA_DMA_FIFO_OUT, 	STA_DMA_SAI3_TX1, 2, STA_DMA_FSCK1_SLOT0);
	#endif
}


void set_msp2_48k_to_16k()
{
	PRINTF("Playing MSP2(48k) > SRC1(DOWNx3) > DSP(16k) > SRC0(UPx2) > DAC... \n");

	//MSP2/SAI2
	PLY_Set_PCM_MSP2(g_songAddr + 11, g_songSize - 11*4, 2, 16, 48000);


	//LPFSCR_1:  SAI2 > DOWNx3 > SRCDO
	AIF->AIMUX.BIT.CH1SEL = AIMUX_SAI2RX0;
	AIF->AOMUX.BIT.CH1SEL = AOMUX_SRCDO;
	AIF->LPF_CR.BIT.FS_1  = LPF1_DOWNx3;		//48k downto 16k


	//LPFSCR_0:  LPDI > UPx2 > SAI3
	AIF->AIMUX.BIT.CH0SEL = AIMUX_LPFDI;
	AIF->AOMUX.BIT.CH0SEL = AOMUX_SAI3TX0;
//	AIF->LPF_CR.BIT.FS_0  = LPF0_UPx2;			// <-- NOT WORKING !?
	AIF->LPF_CR.BIT.FS_0  = LPF_FIR_0;
	AIF->SRC_CR[0].BIT.DRLL_THRES = 0x7;

	//FSYNCIN/FSYNCOUT_0
	AIF->FSYNCIN.BIT.CH0SEL  = FSYNCIO_CK1;
	AIF->FSYNCOUT.BIT.CH0SEL = FSYNCIO_FS;


	//DMABUS
	#if BYPASS_DSP
	STA_DMAAddTransfer2(STA_DMA_SRC1_DO,	STA_DMA_LPF0_DI, 2, STA_DMA_FSCK1_SLOT0);
	#else
	STA_DMAAddTransfer2(STA_DMA_SRC1_DO,	STA_DMA_FIFO_IN, 2, STA_DMA_FSCK1_SLOT0);
	STA_DMAAddTransfer2(STA_DMA_FIFO_OUT, 	STA_DMA_LPF0_DI, 2, STA_DMA_FSCK1_SLOT0);
	#endif

}


void set_mic_48k()
{
	PRINTF("Playing MIC(48k) > DSP(48k) > DAC...\n");

	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_16VAL | SAI_EN;

	//DMABUS
	#if BYPASS_DSP
	STA_DMAAddTransfer2(STA_DMA_SAI3_RX1,	STA_DMA_SAI3_TX1,   2, STA_DMA_FSYNC);
	#else
	STA_DMAAddTransfer2(STA_DMA_SAI3_RX1,	STA_DMA_FIFO_IN,    2, STA_DMA_FSYNC);
	STA_DMAAddTransfer2(STA_DMA_FIFO_OUT, 	STA_DMA_SAI3_TX1,   2, STA_DMA_FSYNC);
	#endif

	STA_DMAStart();

	while (1) {
		SLEEP(100);
	}
}

void set_mic_16k()
{
	PRINTF("Playing MIC(48k) > SRC1(DOWNx3) > DSP(48k) > SRC0(UPx2) > DAC...\n");


	//SAI3 (master, enable)
	AIF->SAI3_CR.REG = SAI_I2S | SAI_MST | SAI_32_16VAL | SAI_EN;

	//LPFSCR_1:  SAI3 > DOWNx3 > SRCDO
	AIF->AIMUX.BIT.CH1SEL = AIMUX_SAI3RX0;
	AIF->AOMUX.BIT.CH1SEL = AOMUX_SRCDO;
	AIF->LPF_CR.BIT.FS_1  = LPF1_DOWNx3;		//48k downto 16k


	//LPFSCR_0:  LPDI > UPx2 > SAI3
	AIF->AIMUX.BIT.CH0SEL = AIMUX_LPFDI;
	AIF->AOMUX.BIT.CH0SEL = AOMUX_SAI3TX0;
//	AIF->LPF_CR.BIT.FS_0  = LPF0_UPx2;			// <-- NOT WORKING !?
	AIF->LPF_CR.BIT.FS_0  = LPF_FIR_0;
	AIF->SRC_CR[0].BIT.DRLL_THRES = 0x7;

	//FSYNCIN/FSYNCOUT_0
	AIF->FSYNCIN.BIT.CH0SEL  = FSYNCIO_CK1;
	AIF->FSYNCOUT.BIT.CH0SEL = FSYNCIO_FS;


	//DMABUS
	#if BYPASS_DSP
	STA_DMAAddTransfer2(STA_DMA_SRC1_DO,	STA_DMA_LPF0_DI, 2, STA_DMA_FSYNC);
	#else
	STA_DMAAddTransfer2(STA_DMA_SRC1_DO,	STA_DMA_FIFO_IN, 2, STA_DMA_FSYNC);
	STA_DMAAddTransfer2(STA_DMA_FIFO_OUT, 	STA_DMA_LPF0_DI, 2, STA_DMA_FSYNC);
	#endif

	STA_DMAStart();

	while (1) {
		SLEEP(100);
	}
}

void set_MSP1()
{
	PRINTF("Playing MIC(48k) > SRC1(DOWNx3) > DSP(48k) > SRC0(UPx2) > DAC...\n");


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

void Test_usecase_ecnr(void)
{

	PRINTF("Test_usecase_ecnr... \n");

	//ADCMIC
	ADCMIC_Init(0);		//0:EXTMIC, 1:MICIN
	SLEEP(500);			//it seems that the ADCMIC needs some warmup-time....

	//user control
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//mini player
	PLY_Init();

	//STA / DMABUS
	STA_Reset();
	STA_DMAReset();
	STA_DMASetFSyncCK1(STA_DMA_FSCK12_16KHZ);
	STA_DMASetFSyncCK2(STA_DMA_FSCK12_8KHZ);


#if BYPASS_DSP
	PRINTF("DSP bypassed\n");
#else
	//DSP
	//STA_StartDSP(DSP_SEL);
	DSP_Start(DSP_SEL);

	SLEEP(100);					//wait 100ms for DSP initialisation

	//////////////////////////////////////////////////////////////
	//DSP AHB IO workaround

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

	//DMA1C1/C2
	dma1_audio_io_isr(DMA1, 1);
	//////////////////////////////////////////////////////////////
#endif

	//reset all AIMUX/AOMUX
	AIF->AIMUX.BIT.CH0SEL = AIMUX_NOINPUT;
	AIF->AOMUX.BIT.CH0SEL = AOMUX_NOOUTPUT;
	AIF->AIMUX.BIT.CH1SEL = AIMUX_NOINPUT;
	AIF->AOMUX.BIT.CH1SEL = AOMUX_NOOUTPUT;


	//-- test cases ---

//	set_msp2_48k_to_48k();
//	set_msp2_16k_to_48k();
	set_msp2_16k_to_16k();				//<-- UPx2 NOT WORKING (using FIR_0)
//	set_msp2_16k_to_16k_workaround_cascading_2_srcs();
//	set_msp2_16k_to_16k_noUPtoDAC();
//	set_msp2_48k_to_16k();
//	set_mic_48k();
//	set_mic_16k();


	//----- play (in loop)-------

	PLY_SetState(PLY_PLAY);	//starts MSP2/SAI2, SAI3,
	STA_DMAStart();


	while (1) {

		SLEEP(100);	//avoid sticky loop
	}

	//stop and clean
	STA_Reset();
	STA_SetDspISR(DSP_SEL, 0);
}

#endif //0
