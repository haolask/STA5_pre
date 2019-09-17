/*
 * STAudioLib - test_ASRC.c
 *
 * Created on 2016/01/24 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	testing:



*/

#if 1
#include "staudio_test.h"




//---------------------------------------------------------
// Test cases
//---------------------------------------------------------


void set_msp2_DOWN_UP_to_dac()
{
	PRINTF("Playing MSP2(48k) > SRC1(DOWNx3) > DMABUS(16k) > SRC0(UPx2) > DAC... \n");

	//LPFSCR_1:  SAI2 > DOWNx3 > SRCDO
	AIF->AIMUX.BIT.CH1SEL    = AIMUX_SAI2RX0;
	AIF->AOMUX.BIT.CH1SEL    = AOMUX_DMABUS;
	AIF->LPF_CR.BIT.FS_1     = LPF1_DOWNx3;		//48k downto 16k


	//LPFSCR_0:  LPDI > UPx2 > SAI3
	AIF->AIMUX.BIT.CH0SEL    = AIMUX_DMABUS;
	AIF->AOMUX.BIT.CH0SEL    = AOMUX_SAI3TX0;
	AIF->LPF_CR.BIT.FS_0     = LPF0_UPx2;			// <-- NOT WORKING !?
//	AIF->LPF_CR.BIT.FS_0     = LPF_FIR_0;
	AIF->FSYNCIN.BIT.CH0SEL  = FSYNCIO_CK1;
	AIF->FSYNCOUT.BIT.CH0SEL = FSYNCIO_FS;


	//DMABUS
	STA_DMAAddTransfer2(STA_DMA_SRC1_DO,	STA_DMA_LPF0_DI, 2, STA_DMA_FSCK1_SLOT0);

}


//---------------------------------------------------------
// Main
//---------------------------------------------------------

void Test_ASRC(void)
{

	PRINTF("Test_ASRC... \n");

	//ADCMIC
	ADCMIC_Init(0);		//0:EXTMIC, 1:MICIN
	SLEEP(500);			//it seems that the ADCMIC needs some warmup-time....

	//user control
//	RTX_Init(rtxCallback);
//	KPD_Init(keyCallback);

	//mini player
	PLY_Init();
	PLY_Set_pcm_source_MSP(MSP2, &g_song1, PLY_NOFLAG);

	//STA / DMABUS
	STA_Reset();
	STA_DMAReset();
	STA_DMASetFSyncCK1(STA_DMA_FSCK12_16KHZ);
//	STA_DMASetFSyncCK2(STA_DMA_FSCK12_8KHZ);


	//reset all AIMUX/AOMUX
	AIF->AIMUX.BIT.CH0SEL = AIMUX_NOINPUT;
	AIF->AOMUX.BIT.CH0SEL = AOMUX_NOOUTPUT;
	AIF->AIMUX.BIT.CH1SEL = AIMUX_NOINPUT;
	AIF->AOMUX.BIT.CH1SEL = AOMUX_NOOUTPUT;


	//-- test cases ---

	set_msp2_DOWN_UP_to_dac();



	//----- play (in loop)-------

	PLY_SetState_MSP(MSP2, PLY_PLAY);	//starts MSP2/SAI2, SAI3,
	STA_DMAStart();


	while (1) {

		SLEEP(100);	//avoid sticky loop
	}

	//stop and clean
	STA_Reset();

}

#endif //0
