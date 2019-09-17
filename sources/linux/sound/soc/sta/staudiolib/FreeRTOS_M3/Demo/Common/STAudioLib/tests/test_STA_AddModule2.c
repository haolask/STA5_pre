/*
 * STAudioLib - test_STA_AddModule2.c
 *
 * Created on 2017/06/20 by Christophe Quarre'
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	AUTO TEST
 */

#if 1
#include "staudio_test.h"

enum {
	GAIN_ID = 1,
	MUX_ID	= 2,
	EQ_ID	= 3,
	USER_ID = 4
};

const char* g_names[] = {
	"gain",
	"mux",
	"eq",
	"user module"
};

/*
typedef struct {
	STA_ModuleType	statype;
	STAModule		stamod;
	u32				id;
	char*			name;
} MODULES;
*/


static STA_UserModuleInfo g_info;

static u8 g_muxSels[2] = {0, 1};


static void testGetModuleName(void);

//---------------------------------------------------------
static void BuildFlow(void)
{
    STA_ErrorCode sta_err;
    int i;


    //add GAIN
	STA_AddModule2(STA_DSP0, STA_GAIN_STATIC_NCH, GAIN_ID, 0, "gain"); //2ch
	STA_SetNumChannels(GAIN_ID, 4);

	//add MUX
	STA_AddModule2(STA_DSP0, STA_MUX_4OUT, MUX_ID, 0, "mux");
	STA_MuxSet(MUX_ID, g_muxSels);

	//add EQ
	STA_AddModule2(STA_DSP0, STA_EQUA_STATIC_3BANDS_DP, EQ_ID, 0, NULL); //2ch
	STA_SetNumChannels(EQ_ID, 4);

    //add User module
    g_info.m_dspType = 0;
	STA_AddUserModule2(STA_DSP0, STA_USER_0+1, &g_info, USER_ID, 0, "user module");

	//test get module name now before to delete the USER module
	testGetModuleName();

    //test delete module by ID
    //IMPORTANT: need to delete this "dummy" user module to avoid an error in the BuildFlow
    STA_DeleteModule((STAModule*)USER_ID);


    //add connections
    FORi(2) {
    STA_Connect(STA_XIN0, i,	GAIN_ID,   i);
    STA_Connect(GAIN_ID,  i,	MUX_ID,    i);
    STA_Connect(MUX_ID,   i,	EQ_ID,     i);
    STA_Connect(EQ_ID,    i,	STA_XOUT0, i);
	}

    //build
    STA_BuildFlow();

    sta_err = STA_GetError();
    CHECK(sta_err==STA_NO_ERROR);

	//post-built init

}
//---------------------------------------------------------
/*
static void testGetModuleName(void)
{
	char name[STA_MAX_MODULE_NAME_SIZE];

	PRINTF("\nmodule names =\n");
	PRINTF("%s\n", STA_GetModuleName(GAIN_ID, 0));
	PRINTF("%s\n", STA_GetModuleName(MUX_ID, 0));

	STA_GetModuleName(EQ_ID, name);
	PRINTF("%s\n", name);

	STA_GetModuleName(USER_ID, name);
	PRINTF("%s\n", name);
}
*/
static void testGetModuleName(void)
{
	char name[STA_MAX_MODULE_NAME_SIZE];
	const char* pname;

	pname = STA_GetModuleName(GAIN_ID, 0);
	CHECK(strncmp(pname, g_names[GAIN_ID-1], STA_MAX_MODULE_NAME_SIZE) == 0);

	pname = STA_GetModuleName(MUX_ID, 0);
	CHECK(strncmp(pname, g_names[MUX_ID-1], STA_MAX_MODULE_NAME_SIZE) == 0);

	STA_GetModuleName(EQ_ID, name);
	CHECK(strncmp(name, "NONAME!", STA_MAX_MODULE_NAME_SIZE) == 0);

	STA_GetModuleName(USER_ID, name);
	CHECK(strncmp(name, g_names[USER_ID-1], STA_MAX_MODULE_NAME_SIZE) == 0);
}

//---------------------------------------------------------
static void testSet(void)
{
	u32 latency = 5;
	s32 dinL = 0xAAAA;
	s32 dinR = 0xBBBB;

	//set input data (both sides of DPort)
	XIN[0] = dinL;
	XIN[1] = dinR;
	SWAP_DP(1);
	XIN[0] = dinL;
	XIN[1] = dinR;

	//test that the audio data is going thru up to XOUT
	STA_GainSetGains(GAIN_ID, 0xFF, 0 /*dB*/);
	SWAP_DP(latency);
	CHECKQ23(XOUT[0], dinL);
	CHECKQ23(XOUT[1], dinR);

	//test set gain by ID
    STA_GainSetGain(GAIN_ID, 0, -1200/*dB*/); //MUTE left
    STA_GainSetGain(GAIN_ID, 1, 0/*dB*/);
	SWAP_DP(latency);
	CHECKQ23(XOUT[0], 0);
	CHECKQ23(XOUT[1], dinR);

	//test set gain by NAME
    STA_GainSetGain((STAModule)"gain", 0, 0/*dB*/);
    STA_GainSetGain((STAModule)"gain", 1, -1200/*dB*/); //MUTE right
	SWAP_DP(latency);
	CHECKQ23(XOUT[0], dinL);
	CHECKQ23(XOUT[1], 0);

	//test set mux by ID
    STA_GainSetGain(GAIN_ID, 0, 0/*dB*/);
    STA_GainSetGain(GAIN_ID, 1, 0/*dB*/);
    g_muxSels[0] = 1; //swap L and R
	g_muxSels[1] = 0;
	STA_MuxSet(MUX_ID, g_muxSels);
	SWAP_DP(latency);
	CHECKQ23(XOUT[0], dinR);
	CHECKQ23(XOUT[1], dinL);

	//test set mux by NAME
    g_muxSels[0] = 0; //swap L and R
	g_muxSels[1] = 1;
	STA_MuxSet((STAModule)"mux", g_muxSels);
	SWAP_DP(latency);
	CHECKQ23(XOUT[0], dinL);
	CHECKQ23(XOUT[1], dinR);
}
//---------------------------------------------------------
void TestSTA_AddModule2(void)
{
    PRINTF("TestAddModule2... ");

    STA_Reset();

    BuildFlow();

    //start
    STA_StartDSP(STA_DSP0);
    //STA_Play();				//DSPStart + DMABStart
    SLEEP(1);					//wait 1ms for DSP initialisation

//	testGetModuleName();
	testSet();

    //stop and clean
    STA_Reset();
    PRINT_OK_FAILED;
}


#endif //0
