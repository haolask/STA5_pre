/*
 * STAudioLib - staudio_test.h
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

#include "common.h"
#include "A2evb_frontpanel.h"
#include "miniplayer.h"

#include "staudio.h"
#include "audio.h"	//DSP audio header (ONLY to get the versions of MIXER, COMPANDER,...)

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

extern 	RAWINFO g_song1;
extern 	RAWINFO g_song2;


//--- Macros -------------

#ifndef FRAC
//#define FRAC(f)			((s32)((f) * 0x7FFFFF + 0.5f))	//float to fixed 1.23 bits
//#define FRAC(f)			((s32)((f) >= 0 ? (f) * 0x7FFFFF + 0.5f : (f) * 0x800000 - 0.5f))	//float to fixed 1.23 bits
//#define FRAC(f)			((s32)((f) == 1 ? 0x7FFFFF : (f) * 0x800000))
#define FRAC(f)				(MIN((fraction)((f)*0x1000000 + 1)/2, 0x7FFFFF))
#endif


//#define FLT(x)			((float)(x) / 0x7FFFFF)			//fixed to float. DOES NOT WORK for negative fixed 24 bits!!
#define _ABS24(x)			(~((x)|0xFF800000)+1)			//negative fixed 24 bits into a negative 32 bits
//#define FLT(x)			(((x) & 0x800000) ? -(float)((x)&0xFFFFFF)/0x800000 : (float)(x)/0x7FFFFF)
//#define FLT(x)			((((x) & 0x800000) ? -(float)_ABS24(x) : (float)(x)) / 0x7FFFFF)
//#define FLT(x)			(((x) & 0x800000) ? -(float)_ABS24(x) / 0x800000 : (float)(x) / 0x7FFFFF)
#define FLT(x)				(((x) & 0x800000) ? (float)(s32)(x|0xFF000000) / 0x800000 : (float)(x) / 0x800000)
#define DBL(x)				(((x) & 0x800000) ? (double)(s32)(x|0xFF000000) / 0x800000 : (double)(x) / 0x800000)


#define CHECK(cond)     	if (!(cond)) Error();
//#define CHECKF(val, ref)	if (ABSF((val) - (ref)) > 0.002f) Error();

#define PRINT_OK_FAILED		{PRINTF(g_sta_test_failed ? "FAILED\n" : "OK\n"); g_sta_test_failed = 0;} //reset flag

inline static float ABSF(float a)	 {return ((a) > 0) ? (a) : -(a); }
inline static float ABSD(double a)	 {return ((a) > 0) ? (a) : -(a); }
inline static float CLAMPF(float f)  {return ((f) < -1) ? -1 : ((f) > 1) ? 1 : (f); }
inline static float CLAMPD(double f) {return ((f) < -1) ? -1 : ((f) > 1) ? 1 : (f); }

//(Note this inline function produce much less instructions than the macro)
//inline static float FLT(s32 fx) 	{return ((fx) & 0x800000) ? -(f32)((fx)&0xFFFFFF)/0x800000 : (f32)(fx)/0x7FFFFF;} //NOT WORKING
//inline static float FLT(s32 fx) 	{return (((fx) & 0x800000) ? -(f32)_ABS24(fx) : (f32)(fx)) / 0x7FFFFF;}

void Error(void);

void CHECKF(float val, float ref);
void CHECKD(double val, double ref, double maxRatioError);
void CHECKDIFF(double val, double ref, double maxDiffError);
void CHECKQ23(q23 val, q23 ref);

void SWAP_DP(u32 numSwap);


//
void load_song(RAWINFO* song, void* dstaddress, int srcByteOffset);

//Player (old)
void Test_Play_PCM_48Khz_16bit(const u32* data, int size, int nch);
void Test_Play_PCM_48Khz_16bit_FIFO2(const u32* data, int size, int nch);
void Test_Play_PCM_48Khz_24of32bit(const u32* data, int size, int nch);

u32  Test_Dump_PCM_48Khz_16_to_24of32bit(const u32* src, u32* dst, u32 numSamples, u32 ch);


//test Audio Paths
void TestFrontPanel(void);
void TestMiniplayer(void);
void TestADCmic(void);
void TestDump(void);
void TestMSP(void);
void TestMSP2_16k(void);
void Test_ASRC(void);

void TestDspIRQ(void);
void TestDspAhbBuffering(void);
void TestDspAhbIO(void);
void Test_usecase_ecnr(void);

//test DSP
void TestGain(void);
void TestMixer(void);
void TestMixerBalance(void);
void TestBitShift(void);
void TestPeakDetector(void);
void TestLoudness(void);
void TestEqualizer(void);
void TestSineEqualizer(void);
void TestEqualizer_FIFO2(void);
void TestDelay(void);
void TestDelay0(void);
void TestSinewaveGen(void);
void TestMux(void);
void TestUserModule(void);
void TestSmoothGain(void);
void TestLimiter(void);
void TestLimiter2(void);
void TestClipLimiter(void);
void TestCompander(void);
void TestDsp0Dsp1Dsp2(void);
void TestDCO_4chmonozcf(void);
void Test_cddeemphasis(void);
void TestPCMChime(void);
void TestPolyChime(void);
void TestGainLowDB(void);
void Test_EQ_response(void);
void TestFixedMath(void);
void TestSquareSignal(void);
void TestDspCycles(void);
void TestSTA_AddModule2(void);

//--- Globals -------------

//extern PLAYER g_player;

extern int g_sta_test_failed;

//max DMA transfer size +1
//#define G_BUF_WSIZE			4096
#define G_BUF_WSIZE			1000
extern s32 g_buf[G_BUF_WSIZE];





