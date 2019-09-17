/*
 * STAudioLib - test_miniplayer.c
 *
 * Created on 2015/06/16 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*


 */

#if 0
#include "staudio_test.h"

//REC
//	RAWINFO g_recInfo = {"0:/record_48k_16bit_stereo.pcm",		48000,   16, 2, (void*)0, 0};
	RAWINFO g_recInfo = {"0:/record_48k_3224bit_stereo.pcm",	48000, 3224, 2, (void*)0, 0};

//#define REC_DMA_ADDR		STA_DMA_SRC0_DO
#define REC_DMA_ADDR		STA_DMA_SAI2_RX1
#define REC_WRITE_TO_FILE	1



//DSP
extern void DSP_init(void);
extern void DSP_toggleMute(void);
extern void DSP_rtxCallback(int steps);

extern RAWINFO g_song1;
extern RAWINFO g_song2;

static RAWINFO* g_song = &g_song1;

static MSP_TypeDef *g_MSP;

//---------------------------------------------------------
// DSP
//---------------------------------------------------------
#define DSP_SEL				0

#if (DSP_SEL == 0)
#define DMA_XIN 			STA_DMA_DSP0_XIN
#define DMA_XOUT 			STA_DMA_DSP0_XOUT
#define MOD_XIN				STA_XIN0
#define MOD_XOUT			STA_XOUT0
#elif (DSP_SEL == 1)
#define DMA_XIN 			STA_DMA_DSP1_XIN
#define DMA_XOUT 			STA_DMA_DSP1_XOUT
#define MOD_XIN				STA_XIN1
#define MOD_XOUT			STA_XOUT1
#else
#define DMA_XIN 			STA_DMA_DSP2_XIN
#define DMA_XOUT 			STA_DMA_DSP2_XOUT
#define MOD_XIN				STA_XIN2
#define MOD_XOUT			STA_XOUT2
#endif


//---------------------------------------------------------
// DMABUS
//---------------------------------------------------------

void DMABUS_Set(PLY_SRC src, int nch, int dsp)
{
	u16 dmaSrc;
	u16 dmaDst	= STA_DMA_SAI3_TX1;
	u16 dmaXIN;
	u16 dmaXOUT;

	int bypassdsp = 0; //set if dsp != 0,1,2


	//DSP in/out
	switch (dsp)
	{
	case 0: dmaXIN  = STA_DMA_DSP0_XIN;		dmaXOUT = STA_DMA_DSP0_XOUT; break;
	case 1: dmaXIN  = STA_DMA_DSP1_XIN;		dmaXOUT = STA_DMA_DSP1_XOUT; break;
	case 2: dmaXIN  = STA_DMA_DSP2_XIN;		dmaXOUT = STA_DMA_DSP2_XOUT; break;

	default: bypassdsp = 1; break;
	}

	//SRC
	switch (src)
	{
	case PLY_MSP1:
	case PLY_MSP2:
		dmaSrc = STA_DMA_SRC0_DO; break;

	case PLY_ADCMIC_BUILTIN:
	case PLY_ADCMIC_EXT:
		dmaSrc = STA_DMA_SAI3_RX1; break;

	case PLY_ADCAUX_IN1:
	case PLY_ADCAUX_IN2:
		dmaSrc = STA_DMA_ADC_DATA; break;

	case PLY_FIFO2:
		dmaSrc = STA_DMA_FIFO_OUT; break;
	}

	STA_DMAReset();

	if (bypassdsp)
	{
		STA_DMAAddTransfer(dmaSrc,		dmaDst,  nch);
	}
	else
	{
		//DSP IO
		STA_DMAAddTransfer(dmaSrc,		dmaXIN,  nch);
		STA_DMAAddTransfer(dmaXOUT,  	dmaDst,  nch);

		//DSP IO swap
		STA_DMAAddTransfer(dmaSrc, 		dmaXIN+127, 1);
	}

	//FIFO1 REC
	STA_DMAAddTransfer(REC_DMA_ADDR,	STA_DMA_FIFO_IN,  nch); //dump after DSP

	STA_DMAStart();
}

//---------------------------------------------------------
// FIFO1 Recording
//---------------------------------------------------------
static int	g_record = 0;

static void toggleRecord(void)
{
	RAWINFO* rec 	= &g_recInfo;

	u32 numSamples	= -1; //max

	g_record ^= 1;

	if (g_record)
	{
		PRINTF("Recording...\n");
		PLY_start_dump_FIFO1(rec->data, numSamples, rec->nch); //max numsamples
	}
	else
	{
		PRINTF("Stop recording\n");
		PLY_stop_dump_FIFO1();

		rec->byteSize = g_player.fifo1.m_numBytes;

		switch (rec->bits) {
		case 16:
			PRINTF("converting to 16 bits... ");
			Convert_32to16(rec->data, rec->byteSize, rec->data, &rec->byteSize);
			PRINTF("done\n");
			break;
		case 24:
			PRINTF("converting to 24 bits... ");
			Convert_32to24(rec->data, rec->byteSize, rec->data, &rec->byteSize);
			PRINTF("done\n");
			break;
		default:
			rec->bits = 3224;
			PRINTF("(recorded as 32bit_24valid)\n");
		}
#if REC_WRITE_TO_FILE
		PRINTF("Copying to SD: %s... ", rec->filename);
		WriteFile(rec->filename, &rec->byteSize, (u32*)rec->data);
		PRINTF("done\n");
#endif
	}

	//then retrieve the dump with J-Link Commander:
	//J-Link> savebin <filename>,  <addr>     <numBytes>
	//J-Link> savebin savebin.raw, 0x61E00010, 0x5000
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
// toggle MSP: MSP2, MSP2
//---------------------------------------------------------
static void toggleMSP(void)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	g_MSP = (g_MSP == MSP1) ? MSP2 : MSP1;

	PRINTF((g_MSP == MSP1) ? "MSP1\n" : "MSP2\n");

	play_MSP(g_song);
}
//---------------------------------------------------------
// toggle Source: g_song1, g_song2
//---------------------------------------------------------

static void toggleSource(void)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	g_song = (g_song == &g_song1) ? &g_song2 : &g_song1;

	play_MSP(g_song);
}

//---------------------------------------------------------
// toggle Rec Source: FILE(MSP2), REC(MSP2)
//---------------------------------------------------------
static int g_src_rec = 0;

static void toggleSourceRec(void)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	g_src_rec ^= 1;

	//play REC
	if (g_src_rec)
	{
		PRINTF("Playing REC\n");

		DMABUS_Set(g_MSP == MSP1 ? PLY_MSP1 : PLY_MSP2, g_recInfo.nch, DSP_SEL);

		PLY_Set_pcm_source_MSP(g_MSP, &g_recInfo, PLY_NOFLAG);
		PLY_SetState_MSP(g_MSP, PLY_PLAY);

	}
	else
		play_MSP(g_song);
}

//---------------------------------------------------------
// ADC AUX
//---------------------------------------------------------

static void toggleADCAUX(int chsel)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	PRINTF("ADCAUX_IN%d\n", chsel+1);

	ADCAUX_Init(chsel);

	DMABUS_Set((chsel == ADCAUX_IN1) ? PLY_ADCAUX_IN1 : PLY_ADCAUX_IN2, 2, DSP_SEL);
}

//---------------------------------------------------------
// ADC MIC
//---------------------------------------------------------

static void toggleADCMIC(int chsel)
{
	PLY_SetState_MSP(g_MSP, PLY_STOP);

	PRINTF("ADCMIC %s\n", (chsel == ADCMIC_EXTMIC) ? "(ext)" : "(builtin)");

	DMABUS_Set((chsel == ADCMIC_EXTMIC) ? PLY_ADCMIC_EXT : PLY_ADCMIC_BUILTIN, 2, DSP_SEL);
}

//---------------------------------------------------------
// KEY / ROTARY
//---------------------------------------------------------

static int g_key_pressed = 0;
static int g_rtx_steps   = 0;

static void keyCallback(int key, bool down)
{
	g_key_pressed = key;
}

static void rtxCallback(int steps)
{
	g_rtx_steps = steps;
}

//Process the key/rtx in main task instead of callbacks
static void process_key(void)
{
	/*
	key mapping:
	[MUTE]       [xxxx]	    [      ]  [   ]  [    ]  [      ]
							[SRC   ]  [   ]  [REC ]  [PlyRec]
	[PLAY/PAUS]  [STOP] 	[MSP1/2]  [   ]  [AUX2]  [MIC   ]
	*/

	int key = g_key_pressed;
	g_key_pressed = 0;  //key processed


	//always active keys
	switch(key) {

	//MUTE
#if DSP_SEL != -1
	case 0x11: 	DSP_toggleMute(); return;
#endif

	//PLAY/PAUSE
	case 0x13:  PLY_TogglePlayPause_MSP(g_MSP); return;

	//STOP
	case 0x14:  PLY_SetState_MSP(g_MSP, PLY_STOP); return;

	//SRC
	case 0x31:  toggleSource(); return;

	//REC
	case 0x33:	toggleRecord(); return;
	case 0x34:  toggleSourceRec(); return;

	//MSP
	case 0x41:  toggleMSP(); return;

	//ADCAUX
	case 0x43:  toggleADCAUX(ADCAUX_IN2); return;

	//ADCMIC
	case 0x44:  toggleADCMIC(ADCMIC_EXTMIC); return;

	}
}


static void process_rtx(void)
{
#if DSP_SEL != -1
	DSP_rtxCallback(g_rtx_steps);
#endif

	g_rtx_steps = 0; //rtx processed
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------

void TestMiniplayer(void)
{
	PRINTF("TestMiniplayer... \n");

	g_MSP = MSP2;
	g_recInfo.data = (void*)ALIGN_ADDR_TO_0((u32)g_song2.data + g_song2.byteSize + 32);


	//---- user control ----
	RTX_Init(rtxCallback);
	KPD_Init(keyCallback);

	//---- mini player ----
	PLY_Init();

	//---- DSP -------
#if DSP_SEL != -1
	DSP_init();
#else
	PRINTF("DSP%d bypassed\n", DSP_SEL);
#endif

	//----- play ------

	g_MSP  = MSP2;

	play_MSP(g_song);


	while (1)
	{
		if (g_key_pressed)
			process_key();

		if (g_rtx_steps)
			process_rtx();

		SLEEP(100);	//avoid sticky loop
	}

	//stop and clean
	STA_Reset();
}

#endif //0
