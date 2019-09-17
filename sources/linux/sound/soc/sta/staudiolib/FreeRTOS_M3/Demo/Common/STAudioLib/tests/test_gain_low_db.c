/*
 * STAudioLib - test_gain_low_db.c
 *
 * Created on 2013/04/22 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing chained GAINS with chained attenuations (low dB)

 */

#if 1
#include "staudio_test.h"
#include "math.h"

#define DB2LIN(db)		pow(10, (double)(db) / 20)
#define LIN2DB(lin)		(20.0 * log10((double)(lin)))
#define DB2GAIN(db)		((int)round((db) * 10))


static STAModule g_srcScalar;
static STAModule g_fader;
static STAModule g_volume;
static STAModule g_postScalar1;
static STAModule g_postScalar2;


static void setDin(const double* din, int nch)
{
	int i;
	FORi(nch) XIN[i] = FRAC(din[i]);	//Face A
	XIN[127] = 1;	//swap dualport
	FORi(nch) XIN[i] = FRAC(din[i]);	//Face B
}


static void Test4_low_db(double Gin)
{
	//notations: G for dbGains, g for linGains
//	const double Gin  = -12.0;
	const double Gsrc = -20.4;		//gsrc = DB2LIN(Gsrc);	//-20.4
	const double Gfad = -13.0;		//gfad = DB2LIN(Gfad);	//-13
	const double Gpost=  48.0;		//gpost= DB2LIN(Gpost);	//+48

	const double Gvol[5] = {0, -80, -70, -65, -60}; 	const int numVol = 5;

	const double din0[2] = {1.0, DB2LIN(Gin)}; 			const int nch = 2;

	const double GtotRef = Gin + Gsrc + Gfad + Gpost;  //volume = 0dB

	double gvol, Gtot, Gout[2];
	int i, v;


	setDin(din0, nch);

	STA_GainSetGains(g_srcScalar,   0xFF, DB2GAIN(Gsrc));
	STA_GainSetGains(g_fader,       0xFF, DB2GAIN(Gfad));
	STA_GainSetGains(g_postScalar1, 0xFF, DB2GAIN(Gpost/2));
	STA_GainSetGains(g_postScalar2, 0xFF, DB2GAIN(Gpost/2));

	PRINTF("\nTEST with Gin = %d\n", DB2GAIN(Gin));

	for (v = 0; v < numVol; v++)
	{
		STA_GainSetGains(g_volume,  0xFF, DB2GAIN(Gvol[v]));
		SWAP_DP(15);

		Gtot = Gin + Gsrc + Gfad + Gvol[v] + Gpost;

		//check output
		FORi(nch) Gout[i] = LIN2DB(DBL(XOUT[i]));
		PRINTF("vol= %d (Gtot= %d), Gout= %d\n", DB2GAIN(Gvol[v]), DB2GAIN(Gtot), DB2GAIN(Gout[1] - GtotRef));

	}
}

//---------------------------------------------------------
static void BuildFlow(void)
{
	int i;

	//add modules
	g_srcScalar   = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); //2 gains by default
	g_fader       = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); //2 gains by default
	g_volume      = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); //2 gains by default
	g_postScalar1 = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); //2 gains by default
	g_postScalar2 = STA_AddModule(STA_DSP0, STA_GAIN_STATIC_NCH); //2 gains by default


	//connect
	FORi(2) {
	STA_Connect(STA_XIN0,      i,  g_srcScalar,   i);
	STA_Connect(g_srcScalar,   i,  g_volume,      i);
	STA_Connect(g_volume,      i,  g_fader,       i);
	STA_Connect(g_fader,       i,  g_postScalar1, i);
	STA_Connect(g_postScalar1, i,  g_postScalar2, i);
	STA_Connect(g_postScalar2, i,  STA_XOUT0,     i);
	}

	//build
	STA_BuildFlow();

	//Post init

}
//---------------------------------------------------------
void TestGainLowDB(void)
{
	PRINTF("TestGainLowDB... \n");

	STA_Reset();

	BuildFlow();

	//start
	STA_StartDSP(STA_DSP0);
	//STA_Play();				//DSPStart + DMABStart
	SLEEP(5);					//wait 5ms for DSP initialisation


	Test4_low_db(-12.0);
	Test4_low_db(-20.0);

	//stop and clean
	STA_Reset();
	PRINT_OK_FAILED;
}


#endif
